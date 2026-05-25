# #189 / #27 — regalloc cost-model drill (2026-05-25)

**Outcome: GO.** Root cause identified and grounded in upstream's model; fix is a
register-class constraint at instruction selection, not cost tuning. This doc is the
draft root-cause description for discussion on `llvm-z80/llvm-z80`.

## Repro (the witness)

`llvm/test/CodeGen/Z80/iy-loop-carried-112.ll`, function `popcount32`
(`while (v) { count += v & 1; v >>= 1; }` on an `i32`):

```
llc -mtriple=z80 -mattr=+static-stack -O2 -z80-unreserve-iy
```

Loop body, **IY reserved (default, good):**
```
ld   hl,(__sfrend-2)     ; read the i32 half directly, 3 B
```
Loop body, **IY un-reserved (bad):** the same half lives in IY and every access is a
stack round-trip:
```
push iy ; pop hl         ; 3 B + stack traffic, just to read IY
...
push iy ; pop bc         ; again
push hl ; ... ; pop hl    ; spill HL to free it for the shuttle
```
test_168 (`crc_one`) does *not* trigger at `-Os/-O2` — it's small enough to fit in
3 pairs. The pressure to reach for IY needs a hotter/wider i32 loop; popcount32 is
the minimal witness.

## Root cause

1. `GR16 = (add DE, HL, BC, IX, IY)` — so `AllocationOrder(GR16) = [de hl bc iy]`
   (IX reserved as FP here). IY is a legal landing spot for **any** `GR16` vreg.
2. The vregs that land in IY here are plain `gr16` that get **sub-register
   decomposed**: `%28.sub_lo`, `LSHR16 %28(tied-def 0)`, `XOR_CMP_EQ16`,
   `%62:gr8 = COPY %44.sub_lo:gr16`. The byte halves `IYL`/`IYH` have no documented
   8-bit ops, so once such a value is in IY, `Z80ExpandPseudo`/`copyPhysReg` *must*
   shuttle it through a real pair via `push iy; pop hl` (3 B each, in the loop).
3. **The cost model cannot see this shuttle cost:**
   - `CostPerUse=2` on IY (Target.td: *"additional cost of instructions using this
     register… ARM Thumb / x86-64 / RISC-V use it where registers need larger
     encodings"*) correctly prices the **DD/FD prefix byte**, but the dominant cost
     here is the push/pop round-trip, which is not an "instruction using IY" — it's a
     copy forced by the absence of `IYL`/`IYH` ops. CostPerUse is also a small static
     nudge that loop frequency swamps.
   - `CopyCost=3` lives on `IR16` (the IX/IY-only class). Copy/spill cost is a
     property of the **vreg's register class**; a `GR16` vreg assigned IY is costed as
     a cheap `GR16` copy (CopyCost 1). The expensive case is structurally
     unmodelable by `CopyCost`.
4. `GR16NoIR = (add DE, HL, BC)` already exists for exactly this ("regalloc never
   picks IX/IY for sub-register-accessed values") — but the `LSHR16` / `sub_lo` /
   `XOR_CMP_EQ16` operands here are plain `GR16`, so the guarantee never reaches them.

## Why the fix is a class, not a cost (upstream grounding)

`llvm/docs/CodeGenerator.html`: *"Each virtual register can only be mapped to
physical registers of a particular class"*; operation-specific limits are expressed
via classes — *"in the X86 architecture, some virtuals can only be allocated to 8-bit
registers."* Register classes are the **hard legality constraint** assigned at
instruction selection; `CostPerUse`/`CopyCost` are orthogonal *preference*. "An i32
half that will be byte-decomposed must never occupy IX/IY" is a legality statement
about those operands -> it belongs in the register **class**, not in a cost.

This also explains why prior cost-knob attempts (CostPerUse=2, IR16 CopyCost=3) did
not stop it, and why a class-narrowing pre-RA pass tried in session-73o/p was
net-harmful when applied bluntly: the constraint must be precise (only the
sub-register-accessed operands), applied at ISel, not a blanket pin.

### "If we cannot model the Z80's cost, why is the fix simple?" (the key insight)

This looks like a contradiction — the prose above says LLVM's cost model is too weak
to express the Z80's register costs, yet the fix is small.  The resolution is that
**the fix does not use the cost model at all.**  LLVM has two separate machines:

- **Cost / preference** (`CostPerUse`, `CopyCost`, allocation order, hints): answers
  the *graded* question "is IX/IY worth it *here*?"  IX/IY cost an extra DD/FD prefix
  byte per use but add a 4th/5th register pair that may save a spill.  Whether that
  trade nets out depends on local pressure, loop frequency, and what else is live.
  This is the part the Z80 fits badly, and it stays unsolved.

- **Legality / class** (`TargetRegisterClass`): answers the *binary* question "may
  this vreg occupy IX/IY *at all*?"  This LLVM models **exactly** — the allocator
  never even enumerates an illegal physreg for a vreg.

The fix is simple because the failing subset is **not a graded trade — its cost is
degenerate.**  For a value that gets byte-decomposed, IX/IY is *unconditionally*
worse: `IYL`/`IYH` have no documented 8-bit ops, so every byte touch is a forced
`push iy; pop rr` shuttle (and, in the default config, a miscompile).  There is no
pressure level, loop shape, or spill count at which holding a byte-split value in IY
beats DE/HL/BC.  When the cost function collapses to "always worse," you do not need
to *price* the choice — you remove it.  A foregone-conclusion cost is exactly a
legality statement, and legality is the machine LLVM models precisely.

So the move is: **find the slice of the allocation problem where the weak (cost)
model degenerates, and lift that slice into the exact (legality) model.**  We did not
make LLVM understand the Z80's costs better; we carved off the one pattern that does
not need a cost model and expressed it as a class.

Two caveats keep this honest — it is not a free lunch:

1. **It is not a complete Z80 model.**  The general question ("weigh IY's prefix
   bytes against spills for values that are *not* byte-decomposed") is still
   unmodeled and still imperfect.  This fixes one well-characterized pattern, not the
   allocator's cost blindness in general.
2. **The exclusion has a second-order cost.**  Removing IX/IY from these vregs' pool
   shrinks the supply and *may* push a spill elsewhere.  That residual cost is real
   and measurable — which is why the commit gate (below) is a register-pressure
   histogram, not "the repro passes."  We did not escape cost reasoning; we reduced
   it from "model it in the allocator" (hard, can't) to "measure whether this one
   exclusion regresses real code" (easy, a before/after count).

## Proposed fix direction (next session, implementation drill)

Constrain to `GR16NoIR` exactly the `GR16` operands that are byte-decomposed:
the def/use of `LSHR16` (and the SRL/RR i32/i16 shift-chain pseudos), `XOR_CMP_EQ16`,
and any vreg feeding a `COPY %x.sub_lo/.sub_hi:gr16`. Open implementation questions:
- Where do these vregs acquire `GR16`? (ISel pattern operand class vs RegBankSelect
  vs legalizer.) Tighten at the earliest point that covers all the byte-access uses.
- Minimal constraint set: over-constraining costs the extra pairs IY was meant to
  provide; under-constraining leaves the shuttle. Needs a pressure histogram
  (per `lessons-2026-05-04-structural-fix-failures.md`: structural fixes need
  pressure evidence, not plausibility) before committing.

**This is very likely the shared root of #27 (per-pair 16-bit copy cost), #110
(greedy ignores target hints), and #115 (greedy picks IY for HL-tied / LDIR
operands).** One class-level fix may close the cluster.

## Resolved: #189 has TWO faces, and they split on `+static-stack`

Ran the five distilled repros (test_166/167/168/169/170, the ones extracted from the
AES/test failures) through the emulator with `-z80-unreserve-iy`, both configs, all
six opt levels:

- **`+static-stack` (production config): 36/36 PASS, correct values.** So under the
  production config IY-on is *value-correct*; the problem there is purely the
  push/pop **density** regression (Tier IV).
- **default config (no `+static-stack`, IX frame pointer): MISCOMPILES.**
  - `test_168` `crc_one` -> **`0x0044`** instead of `0xEF8D` at `-O1` and `-Os`.
  - `test_167` `crc32` -> **emulator timeout / hang** at `-O2` and `-O3`.
  - opt-level-dependent (regalloc-pressure dependent), config-dependent.

So #189 is **also a Tier II correctness bug**, but only in the default
(stack-frame) configuration — the gate for ever submitting IY-unreserve upstream
(the backend must be correct in *all* configs, not just `+static-stack`). My first
read ("density-only") was premature: it tested only `+static-stack`. This is the
"value oracle covers all configs" rule earning its keep.

### Mechanism of the default-config miscompile

`crc_one` -O1 IY-on (no static-stack) emits nested, interleaved stack traffic:
`push af; push af; ... push hl; push hl; ... pop hl; pop hl; ... push iy; pop hl;
... push hl; pop iy`. The IY value is shuttled via push/pop **through** the same
stack the non-static-stack config uses for spill/reload. The LIFO discipline breaks
(a value is pushed and the wrong thing popped, or a spill slot is read at the wrong
SP depth), corrupting the loop-carried i32 -> `0x0044` / infinite loop.
`+static-stack` spills to **fixed BSS addresses**, so the IY shuttle never collides
with stack spills -> correct. Same family as the session-73s push/pop-tracking bugs,
but here it lives in allocation/expansion, not a peephole.

### One fix for both faces

The `GR16NoIR` class constraint proposed above fixes **both**: if the
byte-decomposed i32 half never occupies IY, there is no shuttle — so no density
bloat under `+static-stack` and no stack collision in the default config. The
correctness gate and the density gap collapse into one register-class fix.

### Repro witnesses

- Green production-config coverage: test_166-170 now pinned to IY-on `+static-stack`
  via per-test `EXTRA-FLAGS` (36/36 PASS, all opt levels).
- Default-config miscompile (deterministic): from `z80-utils/test-runner`,
  `cargo run -- clang iy` (no `-static-stack`) -> test_168 `_O1`/`_Os` FAIL
  `0x0044`, test_167 `_O2`/`_O3` FATAL hang.
- **DONE:** minimal `llc` lit XFAIL `llvm/test/CodeGen/Z80/iy-no-static-stack-miscompile-189.ll`
  (`CHECK-NOT: iy`, flips to XPASS when GR16NoIR lands); findings posted to
  ravn/llvm-z80#189 (the existing issue already covers this — no dup filed).

## Empirical mechanism (2026-05-25, pre-RA MIR dump of `crc_one`)

The actual MIR right before greedy (`llc -print-after=z80-pin-alu-accumulator`,
`-z80-unreserve-iy`) collapses the perceived difficulty: **the backend already
constrains nearly everything correctly.**  `LSHR16`, `XOR_CMP_EQ16`, the shift-chain
pseudos — all carry their operands as `gr16noir` by TableGen class.  Exactly **one**
vreg leaks:

```
bb.0:  %17:gr16 = COPY $de            ; loop-carried i32 high half, arg in DE
bb.4:  $a = COPY %17.sub_lo:gr16      ; only ever consumed via byte COPYs
       undef %17.sub_lo:gr16 = COPY killed $a
       %17.sub_hi:gr16   = COPY killed $a
bb.1:  $de = COPY %17:gr16            ; returned in DE
```

`%17` is `gr16` (the IX/IY-including class), is byte-decomposed (`sub_lo`/`sub_hi`
read *and* write), yet **nothing on its use-chain forces `gr16noir`**: it is only
copied to/from physregs and accessed by byte.  Its sibling half `%18` came in as
`COPY $hl` and *is* `gr16noir` only because it feeds `LSHR16` (whose operand class is
`GR16NoIR`).  So the leak is precisely "a plain `GR16` vreg that is byte-decomposed
but never flows through a `GR16NoIR`-typed instruction."  Greedy is then free to put
`%17` in IY -> byte shuttle -> density bloat (`+static-stack`) / SP-relative-slot
miscompile (default config).

### Two standard APIs that do NOT solve it (ruled out by inspection)

- **`getSubClassWithSubReg(GR16, sub_lo)` returns `GR16`, not `GR16NoIR`.**  Decoded
  from the generated table (`Z80GenRegisterInfoTargetDesc.inc:987`, value 13 = ID 12
  = `GR16`, off-by-one because 0 means "none").  Reason: `IXL`/`IYL` *do* exist as
  registers in an 8-bit class, so `GR16` as a whole still "has" a `sub_lo` — the
  subreg-class machinery does not exclude IX/IY.  The exclusion must come from the
  *use's* class (`GR8`), not from the subreg index.
- **`MRI.recomputeRegClass(Reg)` cannot narrow here.**  It is built to *grow* a class
  to `getLargestLegalSuperClass` and returns `false` the instant that equals the
  current class (its purpose is de-constraining after coalescing).  For a `GR16` vreg
  it bails immediately and never narrows to `GR16NoIR`.  The earlier recipe's
  "`recomputeRegClass` intersects the subreg constraint" was wrong.

## Implementation session (2026-05-25) — what actually happened

The pre-RA pass was built and then **discarded as redundant**, and the real lever
turned out to be one line in `getLargestLegalSuperClass`.  The journey, in order,
because each step corrected the previous understanding:

### 1. The pre-RA narrowing pass was built — and narrows nothing

Wrote `Z80NarrowSubRegGR16` exactly as the recipe above specified (narrow plain-`GR16`
byte-decomposed vregs to `GR16NoIR`, pre-RA).  Built clean.  But the IY shuttle
*persisted* in the generated `crc_one` (`push iy; pop hl` ... `push iy; pop rr`), in
BOTH `+static-stack` and default config.  MIR dumps showed why: after the pass there
were **zero plain `:gr16` operands** — yet greedy still produced IY code.  The pass
was a no-op because, in the combined pipeline, the offending vregs were *already*
`gr16noir` before the pass even ran.

### 2. The real mechanism: `getLargestLegalSuperClass` re-widens GR16NoIR -> GR16

`Z80RegisterInfo::getLargestLegalSuperClass` returned `GR16` for any class with `GR16`
as a super-class — including `GR16NoIR`.  This function is the **grow step** used by
`recomputeRegClass` (during the register coalescer) and by greedy's live-range
splitting.  So every `GR16NoIR` value (the byte-decomposed halves, correctly created
by the TableGen instruction classes — `LSHR16`, `XOR_CMP_EQ16`, ...) was silently
**widened back to `GR16` during coalescing**, restoring IX/IY eligibility.  The
allocator/spiller then parked the value in IY and byte-accessed it via the
push/pop shuttle.  The `GR16NoIR` exclusion was real at ISel and thrown away before
allocation.  **This is the core bug.**  The pre-RA pass could not win against it:
whatever it narrowed, the coalescer re-widened.

### 3. The fix, and why it must be flag-gated

Making `getLargestLegalSuperClass` not re-widen `GR16NoIR` removed the IY shuttle
entirely (crc_one default config: **0 IY refs**, test_171 `0x0044 -> 0xEF8D` at all
opt levels; test_166-170 stay green).  **But applied unconditionally it regressed
production**: with IY *reserved* (the production default), `GR16` and `GR16NoIR` have
the same allocatable set `{DE,HL,BC}`, yet the *class* distinction still drives
coalescing — refusing to widen reduced coalescing freedom and added `push hl; ...;
add hl,sp; ...; pop hl` spill churn in several functions (proven by an all-lit-files
`-O2` asm diff: NOT byte-identical).  This is exactly the
`lessons-2026-05-04-structural-fix-failures.md` pressure cost.

Fix: gate on the `Z80UnreserveIY` flag.

```cpp
if (RC == &Z80::GR16NoIRRegClass && Z80UnreserveIY)
  return RC;            // keep the IY-exclusion only when IY can actually be chosen
```

When IY is reserved, the branch is never taken and the function returns exactly what
it did before **for all inputs** -> production codegen is bit-for-bit identical (a
*provable* no-op, confirmed by a byte-identical all-lit-files `-O2` diff; AES / cpnos
/ BIOS never pass `-z80-unreserve-iy`, so they are byte-identical by construction —
no rebuild needed to verify).  When IY is un-reserved, the exclusion survives
allocation and the shuttle/miscompile is gone.

Net change: **one conditional in `getLargestLegalSuperClass`**.  The pre-RA pass was
deleted.

### 4. A SECOND, independent bug the fix exposes (NOT introduced by it)

With the i32 halves correctly kept out of IY, the freed IY gets used by the allocator
for *other* 16-bit values — and `popcount32` (lit `iy-loop-carried-112.ll`, the #14
witness) then emits `ld iy,0; xor iyh; xor iyl`: **undocumented `IYH`/`IYL` byte ops
without `+undocumented`** (and a pointless `xor 0` at that).  So the un-reserve-IY
path has a *second* latent bug: 16-bit ALU ops on an IY-resident operand lower to
undocumented half-index ops instead of a documented sequence (or being blocked).
This is a `#13`-class issue, pre-existing, merely *exposed* by the reallocation — the
A/B (stash the fix) shows pre-fix `popcount32` used documented `push iy; pop iy`.

Consequence: `iy-loop-carried-112.ll` (which asserted the old `pop iy` survives) no
longer matches.  The #14 peephole-liveness scenario it guarded is simply not
exercised by `popcount32` anymore.

### Status and where this leaves un-reserve-IY

- **Bug #1 (this issue):** root-caused and fixed, production-safe.  crc_one
  default-config miscompile closed; IY byte-shuttle density bloat closed; lit
  `iy-no-static-stack-miscompile-189.ll` + runtime `test_171` green; production
  byte-identical.
- **Bug #2 (to file):** un-reserve-IY emits undocumented `IYH`/`IYL` for 16-bit ALU
  ops on IY-resident values.  Independent; gates un-reserve-IY together with bug #1.
- **`iy-loop-carried-112.ll`:** must be XFAIL'd referencing bug #2 (honest: my fix
  changed popcount32's allocation, exposing bug #2; the test's #14 scenario is no
  longer hit here) before any commit, since a green lit suite is invariant.

Conclusion confirmed: un-reserve-IY (#112) is not one bug deep.  Fixing the
`getLargestLegalSuperClass` re-widening is a correct, isolated, production-safe
increment that removes one of the interlocking blockers — but `xor iyh` shows at
least one more must fall before IY can be un-reserved by default.

## Value-oracle results (this session, flag-gated fix)

- Production (`-O2`, IY reserved): **byte-identical** to pre-fix across all Z80 lit
  files (provable no-op + empirical diff).  AES/cpnos/BIOS byte-identical by
  construction.
- Z80 lit: 116 PASS + 5 XFAIL, **1 FAIL = `iy-loop-carried-112.ll`** (bug #2 exposure;
  to be XFAIL'd).  New `iy-no-static-stack-miscompile-189.ll` passes.
- test-runner `clang` full: 726 / 37 / 56 / 207 (fatal count matches baseline; no
  iy-test failures; the flag-off no-op guarantees the non-iy delta is zero).
- IY cells: test_166-171 all PASS at all opt levels (`0x0010 / 0x9E8B / 0xEF8D /
  0x2D3D / 0x00FF / 0xEF8D`); test_171 is the new default-config (no `+static-stack`)
  runtime witness that was `0x0044` pre-fix.
