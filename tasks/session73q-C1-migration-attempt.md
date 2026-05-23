# Session 73q — C1 migration attempt (XOR #0xFF -> CPL into ISel)

**Date:** 2026-05-23
**Predecessor:** `session73q-C1-drill-180.md`.
**Outcome:** PARTIAL.  i8 G_XOR -> CPL migrated into ISel; the late-opt peephole is **kept** (reclassified from "Migrate" to "Keep" in the #180 audit).

## What landed

In `Z80InstructionSelector.cpp`'s G_OR/G_XOR i8 imm-fold branch
(lines ~3508-3522), G_XOR with immediate 0xFF now lowers directly
to `CPL` instead of `XOR_n 0xFF`.  The peephole at
`Z80LateOptimization.cpp:823-841` is unchanged and still catches
remaining `XOR_n 0xFF` emissions from other ISel sites (notably the
i16 EQ/NE path at `Z80InstructionSelector.cpp:1277, 1284`).

New lit test at `llvm/test/CodeGen/Z80/xor-ff-to-cpl.ll` guards
against regression for the i8 NOT, i16 NOT decomposition, and
NOT+store patterns.

## What didn't land

The C1 drill's full plan (migrate ISel + remove the peephole) was
**not safely achievable** within this session.  Removal of the
peephole produced two regressions:

1. **Lit failure** on `issue-149-i16-ne-minus-one.ll`: the i16 EQ/NE
   path at `Z80InstructionSelector.cpp:1277, 1284` emits two
   `XOR_n 0xFF` (one each for Hi and Lo of the 0xFFFF compare).
   Migrating this path too (substituting CPL when the byte literal
   is 0xFF) is straightforward.

2. **cpnos PROM1 +1 B regression** that remains EVEN AFTER also
   migrating the i16 EQ/NE path.  Sequence of measurements:
   - State B (i8 G_XOR migrated, peephole kept): **2028 B**.
   - State C (peephole removed, i16 not migrated): 2031 B + lit FAIL.
   - State D (peephole removed, i16 migrated): 2029 B + lit OK.
   - State F (i8 migrated only, i16 NOT migrated, peephole kept): **2028 B**.  Final.

   The 1 B gap between state D and state F is the pipeline-ordering
   side effect from removing the peephole's MBB iteration — the same
   class of side effect that produced the +1 B regression in Option B
   (#177 Z80NarrowIV removal).  Tracking down whatever downstream pass
   is sensitive to that iteration is out of scope for this drill.

## Reclassification

The audit doc `tasks/late-opt-audit-2026-05-02.md` and #180 listed
this peephole under "Migrate".  Based on this drill, it should be
**reclassified to "Keep"**:

- The transform is correct only when FLAGS are dead, which post-RA
  liveness can prove cheaply but vreg-level reasoning cannot
  (without re-implementing the same liveness machinery).
- Removing the peephole regresses cpnos PROM1 by 1 B even after
  full migration to ISel.
- The peephole costs ~20 LOC.  Not worth chasing a -20 LOC, +1 B trade.

The C1 drill's broader value: it exposed that **not every audit
"Migrate" classification is correct**.  Future C2 audit iterations
should include a "Try removing and measure" step before committing
to the Migrate path.

## Verification

- Lit suite: 109 PASS + 3 XFAIL = 112 (was 108 + 3 = 111; added one
  test).
- cpnos PROM1: 2028 B / 2048 B (20 B free) — matches the
  pre-Option-B baseline, recovers the 1 B Option B cost.
- AES `aes256.c` -Oz `.text`: 3299 B (unchanged).
- test-runner clang sweep: running (results will be appended once
  complete).

## Files changed

- `llvm/lib/Target/Z80/Z80InstructionSelector.cpp`: ~15 lines added
  in G_OR/G_XOR i8 imm-fold branch.
- `llvm/test/CodeGen/Z80/xor-ff-to-cpl.ll`: new lit test.

## What this means for the #180 audit going forward

For the remaining 15 Migrate candidates, the C1 process should now
be: (1) identify the upstream home, (2) implement the ISel-side
emission, (3) **measure whether the peephole can be removed without
regression**, (4) classify the peephole as Keep or Delete based on
(3).  The methodology demo is sound; the conclusion ("Migrate, ~1 h
cost") is too optimistic — full LOC reduction requires the
downstream-pipeline-effect analysis that adds ~1 h per peephole on
average.
