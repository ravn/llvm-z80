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
  `0x0044`, test_167 `_O2`/`_O3` FATAL hang. **TODO (next step):** reduce to a
  minimal `llc` lit XFAIL and file as a ravn/llvm-z80 issue (the real upstream gate).

## Test case

- `iy-loop-carried-112.ll` exists (proves the #14 fix). Extend post-fix with a
  `CHECK-NOT: push iy` in the loop body to lock in the density fix.
- Add the value-oracle cells (test_166/167/168 IY-on) once the correctness question
  is settled.

## Value-oracle protocol (binding before any codegen commit here)

`ninja clang llc` -> lit (108+3) -> `cargo run -- clang` (681/46/56/207) ->
AES 13/13 -> cpnos polypascal-test -> BIOS/cpnos size check. Per
`execution-plan-2026-05-22.md`.
