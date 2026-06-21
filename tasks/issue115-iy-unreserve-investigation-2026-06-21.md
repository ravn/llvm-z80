# Issue #115 — IY-extraction overhead investigation 2026-06-21

## ⏸ PARKED 2026-06-21 (later, same day)

**Status**: PARKED.  Premise verified real (~21 B recoverable on the
production triplet); design sketch ready; implementation NOT started.

**Why parked**: a higher-priority direction surfaced -- **get the
usual 3-pair register set (BC, DE, HL) right so LDIR and DJNZ behave
correctly**.  #115's HLReg/DEReg approach would *extend* the
single-register-class machinery; before adding new exclusion classes
on top of BCReg / BReg / AReg / GR16NoIR, the 3-pair set's existing
behaviour must be solid for LDIR / LDDR / CPIR / CPDR (which need
HL+DE+BC) and DJNZ (which needs B).

## ⚠ STRUCTURAL CAVEAT discovered 2026-06-21 (later still)

An attempt to apply HLReg to the sister case #111 (i16 self-loop
pointer-arg) hit a structural conflict: when an HLReg-constrained
vreg is used as a **memory pointer** in the loop body, greedy
correctly **spills** it because the canonical Z80 memory access via
`(HL)` requires `INC_HL` (which physically clobbers `$hl`).  A vreg
can't be stably constrained to `$hl` AND fed into a sequence that
destroys `$hl`.

Full writeup: `3-pair-set-hlreg-structural-conflict-2026-06-21.md`.

### Impact on this issue's design sketch

The pickup runbook below assumes HLReg works universally on
constrained-to-HL vregs.  The 2026-06-21 attempt proved that
**partially false** -- it works for some use-site patterns but
not others.  Re-audit the 7 IY-extraction sites (autoload 2 +
rcbios 5) against this taxonomy before implementing:

| Post-extract use | HLReg applies? |
|------------------|----------------|
| `call` argument (HL passed, then dead) | ✓ YES -- value consumed, no in-loop $hl mutation |
| `SBC HL,rr` (16-bit compare, value-dead-after) | ✓ YES -- same |
| Memory access via `(HL)` then `INC_HL` (the rcbios e680 case) | ✗ NO -- physical $hl clobber kills the constraint |
| `ADD HL,rr` (HL modified) | ✗ NO -- same |

So the partial-#115 close is **even narrower** than the parked
design sketch claimed.  Roughly: HLReg works on the
"transfer-to-HL-then-consume" cases but not the
"use-HL-then-keep-using-it" cases.

The remaining cases (where HLReg doesn't apply) need a different
mechanism -- likely the ISel idiom recognition path discussed in
the #111 closing comment: recognise "store-then-advance" patterns
and lower so the post-store HL value IS the next-iteration
pointer (single vreg flowing through, no separate `INC16` pseudo).

Before any future implementation: triage each of the 7 sites
against this taxonomy, count how many fall into the ✓ bucket vs
the ✗ bucket, and decide whether the ✓-only subset is worth the
implementation effort.

### Update to "How to pick this up"

Add this step **before** the existing Step 3:

> **Step 2.5 -- triage the 7 sites against the structural caveat.**
> For each IY-extraction site, examine the post-extract use:
> - If it's a function arg / value-dead-after compare: HLReg
>   applies; site is fixable by the design sketch below.
> - If it's a memory access via (HL) followed by INC_HL / ADD HL,rr
>   / similar HL-mutation: HLReg is infeasible (greedy will spill).
>   Site needs a different fix (out of #115's scope).
> Count the ratio.  If the ✓ subset is < 50% of sites, reconsider
> whether implementing HLReg is worth it for that few extractions.


**Pickup**: see the "How to pick this up" section near the bottom of
this writeup.  The empirical numbers, design sketch, and risks are
already characterised; reading this file end-to-end gives full context
in ~10 minutes.

---

## Goal (original 2026-06-21 framing)

Examine whether #115's premise (greedy picks IY for HL/DE-
needed values, forcing `PUSH IY; POP HL` extractions) still applies on
current production builds, and if so, sketch the implementation.

**Method**: empirical scan of the production triplet (autoload + cpnos
PROM1 + rcbios BIOS) disassembly for IY-extraction patterns; locate
reference single-register-class mechanisms; sketch HLReg/DEReg design.

**Outcome**: #115's premise is **REAL AND CURRENT**.  ~21 B of
recoverable overhead across the production triplet from the specific
extraction pattern #115 describes.  Plus a correction to the 2026-06-21
inverse-analysis misclassification (#115 is not a cost-model question).

## Surprise correction — IY is already allocatable in production

`Z80RegisterInfo::getReservedRegs` doesn't unconditionally reserve IY:

```cpp
// Z80RegisterInfo.cpp:277
bool llvm::z80IsIYAllocatable(const MachineFunction &MF) {
  if (Z80UnreserveIY)
    return true;
  return MF.getFunction().hasOptSize() &&
         MF.getSubtarget<Z80Subtarget>().staticStack();
}

// Z80RegisterInfo.cpp:305
if (!z80IsIYAllocatable(MF))
  Reserved.set(Z80::IY);
```

The condition `hasOptSize() && staticStack()` is true for all three
production targets (`-Oz` + `+static-stack`).  **IY is allocatable in
production today.**  No flag-toggle experiment was needed.

The 2026-06-21 inverse-analysis claim that "IY reservation default-on"
was a sledgehammer was therefore wrong: it's already conditional, and
the condition matches production.  The remaining question is whether
the *regalloc quality* under IY-allocatable is good or has the #115
regression pattern.

## Empirical scan — production extractions today

Scanned the three production ELFs (autoload-baseline.elf, cpnos-
baseline.elf, rcbios-baseline.elf -- the ones from #232's investigation)
for `PUSH IY; POP rr` and `PUSH rr; POP IY` pairs.

| Target | iy→hl | iy→de | iy→bc | hl→iy | de→iy | bc→iy | Total | Bytes |
|--------|-------|-------|-------|-------|-------|-------|-------|-------|
| autoload | 1 | 0 | 1 | 3 | 0 | 0 | 5 | 15 B |
| cpnos PROM1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | **0 B** |
| rcbios | 5 | 0 | 0 | 6 | 0 | 0 | 11 | 33 B |
| **Aggregate** | **6** | 0 | 1 | **9** | 0 | 0 | 16 | **48 B** |

Each transfer is 3 B (`PUSH rr` = 1 B + `POP rr` = 1 B for non-IX/IY,
or 2 B + 1 B with the IX/IY prefix).

Two distinct buckets:

- **iy→hl/bc** (7 transfers, ~21 B): value extracted FROM IY.  This is
  the #115 pattern -- greedy picked IY for a value that LDIR / ADD-HL /
  memory-via-HL needed.
- **hl→iy** (9 transfers, ~27 B): value placed INTO IY.  Often
  intentional (to enable subsequent `IY+d` indexed access), so not
  necessarily a bug -- but if eliminated by closer pre-RA placement,
  bytes would be saved too.

#115 specifically addresses the first bucket.  Recoverable overhead by
that interpretation: **~21 B aggregate** across autoload + rcbios; cpnos
has zero IY-extractions and benefits nothing directly.

## Shape verification — does the pattern match #115's description?

Sample of the rcbios sites (raw disassembly excerpts):

```
e2af: e5         push hl
e2b0: fd e5      push iy   ; ← IY→HL extraction
e2b2: e1         pop  hl
e2b3: cd 6b da   call $da6b ; HL passed to callee

e466: eb         ex de,hl
e467: fd e5      push iy   ; ← IY→HL extraction
e469: e1         pop  hl
e46a: a7         and a
e46b: ed 42      sbc hl,bc ; 16-bit compare via HL

e680: fd e5      push iy   ; ← IY→HL extraction
e682: e1         pop  hl
e683: a6         and (hl)  ; memory access via HL
e684: fd 77 00   ld  (iy+$0),a
```

And autoload:

```
62eb: 11 08 00   ld   de,$8
62ee: fd e5      push iy   ; ← IY→HL extraction
62f0: e1         pop  hl
62f1: 19         add  hl,de ; ADD HL,rr operand needed in HL
```

All sampled sites confirm the #115 pattern: a value living in IY needs
to be in HL for the next instruction (call argument convention, 16-bit
compare via `SBC HL,rr`, memory access via `(HL)`, ADD HL,rr operand).
**Pattern matches #115's described regression.  Design is still
applicable.**

## Where the reference single-register-class pattern lives

`Z80RegisterInfo.td`:

```tablegen
// L 179: BReg -- B-only, for DJNZ counter via Z80SplitDjnzCounters.
def BReg : Z80Reg8Class<(add B)>;

// L 211: BCReg -- BC-only, sister of BReg for i16 DJNZ.
def BCReg : Z80Reg16Class<(add BC)>;

// L 186: AReg -- A-only, for ALU accumulator via Z80PinAluAccumulator.
def AReg : Z80Reg8Class<(add A)>;

// L 221: GR16NoIR -- GR16 minus IX/IY, for pseudo expansions whose
// 8-bit-half lowering needs documented SRL/SRA paths (no IXH/IXL/IYH/IYL).
def GR16NoIR : Z80Reg16Class<(add DE, HL, BC)>;
```

Two distinct application patterns:

1. **InstructionSelector-level constraint** (`GR16NoIR` is applied via
   `RBI.constrainGenericRegister(LhsLo, Z80::GR16NoIRRegClass, MRI)` in
   `Z80InstructionSelector.cpp:1543-1574`, `1571-1574`, `1696-1699`).
   Used when lowering a specific pseudo whose tied operand has a hard
   restriction.

2. **Pre-RA pass constraint** (`BCReg` is applied via
   `Z80SplitDjnzCounters`, see `Z80SplitDjnzCounters.cpp:201,258`).
   Used when the constraint is structural (loop counter shape) and
   needs a dedicated pass to identify the candidate vreg.

Either path can host the #115 fix.

## Design sketch -- HLReg / DEReg

**Not code; this is a sketch.**  Implementing requires the same kind of
test discipline (test-first, full lit + production triplet A/B) as the
2026-06-21 sweep.

### Step A — register classes

```tablegen
// Z80RegisterInfo.td, after BCReg:
def HLReg : Z80Reg16Class<(add HL)>;
def DEReg : Z80Reg16Class<(add DE)>;
```

Mechanically straightforward; mirrors BCReg.

### Step B — application

For each pseudo where the operand HAS to live in HL / DE / BC at lower
time, constrain the vreg that COPYs into it.

**LDIR / LDDR**: `Uses = [HL, DE, BC]`.  Each of the three preceding
COPYs (`HL = COPY %src`, `DE = COPY %dst`, `BC = COPY %count`) has a
vreg source that should be constrained to HLReg / DEReg / BCReg
respectively.

**CPIR / CPDR**: `Uses = [HL, A, BC]`.  HLReg + BCReg.

**ADD HL, rr family**: HL is implicitly the destination.  The operand
COPYing into HL is the vreg to constrain.

**Memory access via (HL)**: there's no specific pseudo to anchor here;
this is regalloc's general decision to use HL as a pointer.  #115's
extractions fed this case too (the `and (hl)` shape at e683).
Constraining for this would require a more general pass.

Cleanest first step: handle just LDIR/LDDR/CPIR/CPDR.  Those are 21 B
of the rcbios extractions today (5 sites, all of which involve a call
or compare that happens to land HL via convention).

### Step C — constraint mechanism

Two implementation choices:

1. **InstructionSelector-side** (analog of `GR16NoIR`): when selecting
   the LDIR pseudo lowering, `constrainGenericRegister` the source
   vregs of the three COPYs to HLReg/DEReg/BCReg.  Simple, runs at
   ISel time.

2. **Pre-RA pass** (analog of `Z80SplitDjnzCounters`): walk LDIR/LDDR
   pseudos in MachineInstr form, identify the source vregs, apply
   `MRI.setRegClass()`.  More flexible (can see GISel-lowered MIRs).

Choose (1) -- simpler and earlier; matches the GR16NoIR pattern that's
already in the codebase.

### Step D — risks

- **Over-constraint forcing extra COPYs.**  If the constrained vreg has
  uses where HL isn't the right physreg (say, ADD HL,rr where it's the
  rr operand, not the HL operand), forcing it to HL adds a COPY there.
  Net could be a regression if the vreg has many such uses.
- **Interaction with #110.**  Greedy's copy-elimination heuristic may
  still override the class constraint in some cases (the issue #110
  warns about).  Mitigation: GR16NoIR has worked, so the lever exists
  -- but each new class needs its own validation.
- **LDIR_GUARDED / LDDR_GUARDED / MEMSET_LDIR_GUARDED variants
  (`Z80InstrInfo.td:1760+`)** need the same treatment.
- **ADD HL,rr generalisation** is the bigger ticket -- many pseudos
  involve it implicitly via 16-bit add.  Doing LDIR/LDDR first is the
  fast-payoff path; ADD HL,rr is the long-tail.

### Step E — test discipline

- Lit test pinning the LDIR-via-vreg shape (no `push iy; pop hl`
  extraction; HL produced directly by class).
- Re-run full Z80 lit suite -- no regressions.
- Rebuild production triplet; verify extraction count drops from {2,
  0, 5} toward {0, 0, 0} for IY→HL extractions.
- Test-runner runtime fixture exercising LDIR with various source-vreg
  liveness patterns, to catch over-constraint regressions.
- Re-measure compressed ROM sizes (per #232's lesson: raw vs compressed
  can disagree).

### Step F — effort estimate

- Step A: 5 minutes (one TableGen edit).
- Step B: 30 minutes per pseudo (LDIR/LDDR/CPIR/CPDR, plus the _GUARDED
  variants) = ~2-3 hours.
- Step C: 1-2 hours integrating with the InstructionSelector.
- Step D-E: A/B + measurement = ~3-4 hours.

**Total: roughly 1-2 sessions of careful work** by someone familiar
with GISel + TableGen.  Not a "afternoon fix"; not a multi-week
project either.

Expected payoff: ~21 B saved on production (15 B autoload + 33 B rcbios
of which ~half is the IY→HL extraction half).  Per-target this is
0.3% / 0.3% / 0% relative.

## Reconciliation with the 2026-06-21 inverse analysis

The 2026-06-21 inverse-analysis section of
`session-2026-06-21-z80-tti-modelling-investigation.md` classified
#115 as the "only plausible cost-model retire-candidate" after
#232 was falsified.

That classification was wrong on two counts:

1. **#115's actual proposed fix is regalloc-class machinery, not a
   cost-model edit.**  HLReg/DEReg single-register classes are
   structurally identical to BCReg / GR16NoIR -- TableGen + RegisterInfo
   constraints, not TTI cost hooks.  These should be in the same
   "machinery is the right layer" bucket as the existing AReg/BReg/
   BCReg/GR16NoIR uses.
2. **The 2026-06-21 sweep verdict that "cost-model fixes are inert on
   GISel-Z80 because the downstream Z80-specific machinery already
   produces the right shape" does NOT apply to #115's design.**  The
   single-register-class mechanism doesn't fall under that verdict --
   it's the kind of machinery that the inertness pattern depends on
   producing the right shape.  Adding HLReg/DEReg extends that
   machinery; it doesn't compete with a cost-model edit.

**Net**: #115 is in the **"weak/no cost-model candidates -- the right
layer is machinery"** list, alongside `MaxStoresPerMemcpy`,
`isReMaterializable on LD_*`, etc.  My inverse-analysis cross-ref
comment on #115 was misframed; will be corrected with a follow-up
comment.

The session writeup's inverse-analysis section should also be
corrected.

## What to do with #115

Three options:

1. **Implement HLReg/DEReg per the sketch above** (~1-2 sessions of
   careful work).  Expected: −21 B aggregate across autoload + rcbios.
   Sound payoff for the effort; aligns with existing AReg/BReg/BCReg
   precedent.

2. **Document the empirical numbers + design sketch + reconciliation,
   leave #115 open for a future session** to implement.  Lower-effort
   immediate close; the work is well-scoped when picked up.

3. **Close #115 as low-priority "would be nice but production hard
   caps not binding"**.  The aggregate −21 B doesn't unlock anything
   specific today; cpnos's 31 B free under cap doesn't gate on this.
   Document and move on.

**Recommend option 2.**  Keep #115 open as a precisely-scoped tracker.
The investigation here makes it ready for implementation when someone
picks it up.

## Files produced under /tmp/issue232/ (reused from #232's investigation)

- `autoload-baseline.elf` / `cpnos-baseline.elf` / `rcbios-baseline.elf`
  -- the binaries scanned for IY-extraction patterns.

No new artefacts under /tmp; this investigation was a read-only scan
on the existing #232 artefacts.

## Cross-references

- ravn/llvm-z80#115 (this investigation closes the "examine" question).
- `llvm-z80/tasks/session-2026-06-21-z80-tti-modelling-investigation.md`
  -- the broader TTI sweep; inverse-analysis section needs correction.
- ravn/llvm-z80#110 -- greedy regalloc heuristic overrides target
  hints; #115's HLReg/DEReg is the workaround.
- ravn/llvm-z80#94 / #98 / #99 -- the BReg/BCReg precedents.
- ravn/llvm-z80#112 -- GR16NoIR precedent.
- ravn/llvm-z80#172 -- AReg / Z80PinAluAccumulator precedent.
- `llvm/lib/Target/Z80/Z80RegisterInfo.td:170-225` -- the existing
  single-register-class definitions.
- `llvm/lib/Target/Z80/Z80InstructionSelector.cpp:1543-1699` -- the
  GR16NoIR application pattern to mirror.
- `llvm/lib/Target/Z80/Z80SplitDjnzCounters.cpp:201,258` -- the BCReg
  pre-RA-pass application pattern.
- `llvm/lib/Target/Z80/Z80RegisterInfo.cpp:277` -- the
  `z80IsIYAllocatable` condition (production IY-allocatable in -Oz +
  static-stack).

---

## How to pick this up (future-me checklist)

When someone (probably future-me) returns to #115 after the
3-pair-set focus has been resolved, here's the runbook to re-derive
context and resume.

### Step 0 -- read these three things, in order, ~10 minutes

1. **This file** end-to-end (above and below).
2. **The empirical scan numbers** -- re-derive via Step 1 to confirm
   they still hold; the production binaries may have moved.
3. **The four precedent issues** in 15 seconds each: #94, #98, #99 set
   up BReg/BCReg; #112 set up GR16NoIR; #172 set up AReg/Z80PinAluAccumulator;
   #110 is the underlying greedy heuristic problem.  All have closing
   comments that explain mechanism; skim those.

### Step 1 -- re-derive the empirical numbers

```bash
cd /Users/ravn/z80/rc700-gensmedet
make -C autoload-in-c clean && make -C autoload-in-c prom
make -C cpnos-in-c clean && make -C cpnos-in-c prom1-lineprog
make -C rcbios-in-c clean && make -C rcbios-in-c bios

# Capture ELFs (paths approximate -- find the *.elf in each subdir).
AUTOLOAD=autoload-in-c/clang/prom.clang.elf
CPNOS=cpnos-in-c/clang-prom1lineprog/prom1-lineprog.elf
RCBIOS=rcbios-in-c/clang/bios.clang.elf

# Count IY-extractions (the #115 pattern, "iy→hl + iy→bc"):
for elf in $AUTOLOAD $CPNOS $RCBIOS; do
  hl=$(/Users/ravn/z80/llvm-z80/build-macos/bin/llvm-objdump -d --triple=z80 $elf \
       | awk '/push.*iy/{f=1; next} f && /pop.*hl/{c++} {f=0} END{print c+0}')
  bc=$(/Users/ravn/z80/llvm-z80/build-macos/bin/llvm-objdump -d --triple=z80 $elf \
       | awk '/push.*iy/{f=1; next} f && /pop.*bc/{c++} {f=0} END{print c+0}')
  echo "$(basename $elf): iy→hl=$hl  iy→bc=$bc"
done
```

Compare to the 2026-06-21 baseline: {autoload: 1+1=2}, {cpnos: 0},
{rcbios: 5+0=5}.  If counts moved significantly, the gap has shifted;
re-evaluate the design before implementing.

### Step 2 -- prerequisite check: 3-pair-set work is done

Before implementing HLReg/DEReg, confirm:

- LDIR/LDDR/CPIR/CPDR produce HL/DE/BC source vregs via greedy
  without unnecessary COPYs (the "3-pair-set right" work from the
  pivot).
- DJNZ counter pinning via BReg / BCReg is still firing (the
  Z80SplitDjnzCounters pass).
- No new precedents have landed that would replace the
  single-register-class pattern with something better.

If those are all OK, proceed to Step 3.

### Step 3 -- implement per the design sketch (above section)

Read the "Design sketch -- HLReg / DEReg" section above; follow Steps
A through F.  ~1-2 sessions of careful work for someone familiar with
GISel + TableGen.

### Step 4 -- validate

- Lit test pinning LDIR-via-vreg shape with no `push iy; pop hl`.
- Re-run full Z80 lit suite.
- Re-derive Step 1's numbers; the iy→hl/bc counts should drop toward
  zero on autoload + rcbios.
- Re-measure compressed ROM sizes (per #232's lesson, raw and
  compressed can disagree).
- Run the test-runner suite under +static-stack to catch
  over-constraint regressions.

### Step 5 -- close #115

Once Step 4 confirms the extraction count is at or near zero with no
regressions, close the issue with a comment referencing the final
measurements and a link to the implementation commit.

---

## Open questions discovered during investigation

Things that came up during the examine pass and were left
unanswered.  Resolve before or during implementation:

1. **Is `hl→iy` (the "intentional" direction) also bad?**  9 transfers
   across the triplet (3 autoload + 6 rcbios; cpnos none).  These look
   intentional (a value placed into IY for subsequent `(IY+d)` indexed
   access) but the count is large enough that some may be artifacts of
   regalloc indecision -- a vreg picked into IY only to be PUSH/POP'd
   in.  Worth a separate scan: at each `push hl; pop iy` site, does
   the IY value get used as `(IY+d)` indexing before being clobbered
   or copied out?  If not, the COPY into IY is wasted.

2. **ADD HL,rr generalisation -- how much does it actually save?**  The
   design sketch notes ADD HL,rr is the long tail.  Quantify it: walk
   the production triplet's disassembly counting cases where a vreg
   should have been in HL for an ADD HL,rr and wasn't.  If the count
   is small (~3-5), HLReg's LDIR-only application captures most of the
   recoverable bytes; if it's large (>20), the ADD HL,rr extension is
   important.

3. **Does the COPY-elimination heuristic (#110) still override
   target hints on current LLVM?**  The single-register-class pattern
   exists *specifically* because #110 makes target hints unreliable.
   But #110 is a generic LLVM regalloc concern -- upstream may have
   improved the heuristic since the precedent issues (#94/#98) landed.
   If #110's pain has been reduced, target hints might work today
   without needing exclusion classes -- which would make HLReg/DEReg
   redundant.  Check upstream `RegAllocGreedy` history for changes to
   copy-elimination heuristics since the BCReg precedent landed.

4. **What's the over-constraint cost on cpnos?**  cpnos has zero
   IY-extractions currently.  Adding HLReg might inadvertently force
   COPYs in cpnos where today it has none.  The empirical scan for
   cpnos showed it's clean; need to verify HLReg doesn't break that.

5. **Z80NarrowNoIndex interaction.**  `Z80NarrowNoIndex` (per
   `Z80NarrowNoIndex.cpp:131`) is gated on `z80IsIYAllocatable`; it
   eliminates IY-half emissions in a way that complements HLReg/DEReg.
   Need to confirm the two passes don't have an ordering or
   correctness interaction.

---

## Surrounding context (deeper background)

This section dumps everything else encountered during the
investigation that's worth knowing when picking up.

### Why the regalloc-class pattern exists (deeper than the precedent list)

The greedy regalloc's copy-elimination heuristic (#110) tries to
collapse a `vreg = COPY %src` followed by `... uses vreg ...` into a
single allocation: if `%src` is already in a physreg, the heuristic
prefers to keep `vreg` in the same physreg as `%src`, eliminating the
COPY.  This is correct for most targets where any GPR can hold any
value.

On Z80 it's catastrophic: many vregs MUST be in specific physregs
(LDIR needs HL/DE/BC; DJNZ needs B; ALU goes through A).  When the
heuristic ignores the target hint (`getRegAllocationHints`), the
backend has to COPY-out-then-COPY-back, costing bytes.

The single-register-class pattern (`BCReg`, `BReg`, `AReg`,
`GR16NoIR`) works around this by removing the choice entirely: a
vreg constrained to `BCReg` cannot be allocated to anything other
than BC, so the heuristic has no alternative to consider.

`HLReg` / `DEReg` extend the pattern to LDIR/LDDR/CPIR/CPDR (which
need HL/DE/BC) and to ADD HL,rr (which needs HL as the destination).

### The 2026-06-21 misclassification trail (don't repeat the mistake)

The 2026-06-21 inverse analysis classified #115 as a "plausible
cost-model retire-candidate" because the analysis author
(present-me at the time) confused two things:

1. "IY reservation default-on" -- which the analysis treated as a
   global toggle to retire via cost-model.  In reality:
   - IY is **already allocatable** on production (`-Oz +
     static-stack`).
   - The "reservation" is only on non-production builds (-O0,
     non-static-stack).
   - So there's no production sledgehammer to retire.

2. "Per-pair cost tradeoffs" -- which the analysis suggested could be
   modelled by a context-aware `CostPerUse` (IY=2 in cold blocks,
   higher in tight loops).  In reality:
   - #115's actual fix is regalloc-class machinery, not cost.
   - Cost-aware regalloc isn't the mechanism that prevents the
     extraction; class exclusion is.
   - Even if `CostPerUse` were context-aware, greedy's
     copy-elimination heuristic would still override it (#110).

When re-reading the 2026-06-21 inverse-analysis section, note this
correction.  Don't re-attempt cost-model framing for #115.

### IY-half (IXH/IXL/IYH/IYL) interaction

Under `+undocumented`, the half-index registers (IXH/IXL/IYH/IYL)
become allocatable.  These are 8-bit subregs of IX/IY accessible via
undocumented Z80 opcodes (`LD IXH, n` etc.).  The closed issues
#37/#112/#113/#189 dealt with the encoder + emission of these.

#115 is at the *16-bit pair* level (IY as a pair, not its halves).
The half-index work is orthogonal.  Don't conflate.

### CALL / RET / register conventions

`Z80_CSR_SaveList` and `Z80_AllReg_CSR_RegMask` define what's
preserved across calls.  HLReg-constrained vregs that are alive
across calls may force a callee-save spill, depending on the
convention.  Worth checking when implementing -- the constraint
mechanics may interact differently for live-across-CALL vregs vs
local-to-block vregs.

### The pivot to "3-pair set right"

The user's stated pivot is to focus on getting the usual 3-pair
register set (BC, DE, HL) right for LDIR and DJNZ.  Quoting:

> It is more important to get the usual 3 register set right so things
> turn out right for ldir and djnz

The implication: before extending the single-register-class machinery
(HLReg / DEReg for #115), the existing 3-pair behaviour for the
canonical Z80 idioms (LDIR / LDDR / DJNZ) should be solid.  If those
idioms are currently producing unnecessary COPYs even at the 3-pair
level, fixing that has higher leverage than extending the machinery.

When picking up #115, first confirm the pivot work is done -- otherwise
HLReg/DEReg may layer on top of an unstable base.

---

## What this writeup is NOT

- It is not an implementation.  Step C in the design sketch needs real
  code; this writeup contains pseudocode only.
- It is not a closed issue.  #115 stays OPEN at fork as a parked
  marker; the implementation work resumes here when picked up.
- It is not a cost-model edit proposal.  The 2026-06-21 inverse
  analysis classified #115 as cost-model; that was wrong.  This
  writeup explicitly corrects that and routes the fix to regalloc-
  class machinery.
