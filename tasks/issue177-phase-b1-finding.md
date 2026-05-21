# Phase B1 finding: Phase B0 was wrong; TTI overrides have major impact and miscompile risk

Date: 2026-05-22 (session 73p Phase 2, follow-up to Phase B0).

## What I tried

Implemented the Phase B "best-effort target-truthful overrides" as a
single bundled commit (held in working tree, not committed):

1. `getArithmeticInstrCost`: Mul -> TCC_Expensive; i16 ops cost 2;
   i32 cost 4; i64+ cost TCC_Expensive.
2. `getCastInstrCost`: i16->i8 trunc = 0; i8->i16 zext = 0;
   i8->i16 sext = 2.
3. `getCmpSelInstrCost`: Select cost = 2.

These follow target reality (Z80 has no multiplier, native pair-to-
byte trunc, free zero-extend via `LD H, 0`, branch-based select).

## AES sweep result (with overrides applied)

| Config | Baseline | With overrides | Δ | Verify |
|---|---:|---:|---:|:---:|
| 01_baseline_Oz | 3703 | 3658 | -45 | PASS (assumed; not in capture) |
| 02_Os | 4200 | 3975 | **-225** | PASS at **tstates=28** (suspicious) |
| 03_O3 | 12055 | 12057 | +2 | PASS |
| 04_O2 | 8057 | 8096 | +39 | PASS at **tstates=28** (suspicious) |
| **05_Oz_static_stack** | 2630 | 2684 | +54 | **FAIL (100M ts timeout)** |
| 06_Oz_no_licm_cse | 3703 | 3710 | +7 | PASS |
| 07_Oz_no_lsr | 4036 | 3674 | -362 | PASS |
| 08_Oz_gc_sections | 3683 | 3690 | +7 | PASS |
| **09_Oz_prod_like** | 2574 | 2618 | **+44** | PASS but **slower** (10.76M ts) |
| 10_Oz_no_licm_cse_lsr | 4036 | 3674 | -362 | PASS |
| 11_Oz_no_licm_cse_gc | 3683 | 3690 | +7 | PASS |
| 12_Oz_no_omit_fp | 3244 | 3254 | +10 | PASS |
| 13_Oz_no_omit_fp_no_licm_cse_gc | 3224 | 3234 | +10 | PASS |

Key reads:

- **05_Oz_static_stack: hard FAIL** (timeout at 100M ts).  The
  bundle introduced a miscompile in static-stack mode.
- **02_Os and 04_O2: tstates=28** is a verifier-pattern-matched-
  empty-BSS pass, not a real PASS.  Both miscompiled at byte level
  but happened to leave the BSS in a verifier-matching state after
  immediate halt.
- **09_Oz_prod_like (production target): +44 B regression** with
  no offsetting speed win.
- 07/10 (no-LSR variants): -362 B which is the "lying-flat by 8.9 %"
  size win Phase B0 predicted couldn't happen — but it came together
  with miscompiles elsewhere, so it's not a real win.

## What this falsifies

Phase B0 claimed (and I committed in `ecd143c52df3`):

> Phase B Tier 1 hooks have **low expected production impact**.
> [...] Production codegen impact: **near-zero**.

**This is wrong.**  Refined TTI cost hooks have:

- Major code size impact in both directions (-362 B / +54 B / -225 B / +44 B).
- Real miscompile risk (05_Oz_static_stack FAIL, 02_Os and 04_O2
  silently broken).
- Coupling with the existing `+static-stack` lowering path that I
  did not anticipate.

The "near-zero impact" prediction relied on these false premises
(now disproved):

1. "LICM only uses `getInstructionCost` for GEP foldability" — TRUE
   for LICM specifically, but the bundle changed `getArithmeticInstrCost`
   and `getCastInstrCost` which feed into IndVarSimplify, Inliner,
   and SimplifyCFG decisions that DO ripple to codegen.
2. "LoopUnroll at -Oz uses Threshold=0" — TRUE for UnrollOptSizeThreshold,
   but the bundle's effect probably wasn't via LoopUnroll anyway.
3. "Z80NarrowIV covers IndVarSimplify" — partially true, but IndVarSimplify
   makes other cost-driven decisions (IV widening, recurrence
   simplification) that `Z80NarrowIV` doesn't undo.

The right hypothesis (re-derived): refined cost hooks change *many*
IR passes' decisions simultaneously, and Z80's *MIR-level* invariants
(static-stack address layout, BSS allocation, regalloc cost model)
are not fully decoupled from those IR decisions.

## What likely caused the miscompile

Speculation (NOT investigated; flagged for follow-up):

`getCastInstrCost` returning 0 for `Trunc i16 -> i8` and `ZExt i8 ->
i16` may have caused InstCombine to push truncs/zexts past PHI nodes
or memory operations in a way that the static-stack lowering path
mishandles.  The static-stack failure is a strong hint that BSS
address layout assumed an IR shape that the new trunc-pushing rules
broke.

Alternative hypothesis: `getArithmeticInstrCost` charging i16 = 2 vs
i8 = 1 may have caused IndVarSimplify or LSR to prefer i16 IVs over
i8 (because the lower 8-bit limb cost wasn't reflected as cheap
enough to bias toward narrow IVs).  `Z80NarrowIV` may have failed to
undo this in some loops.

Either way, this is hours-to-days of focused investigation work to
narrow.  It is NOT "2-3 days of mechanical overrides" as Phase B0
re-scoped.

## Revised assessment

Phase B is **higher-stakes and higher-impact than Phase B0 claimed**.
Each individual hook override needs:

1. Full AES sweep gate (not skipped on "expected near-zero").
2. Likely a per-hook investigation if anything regresses or miscompiles.
3. Cross-validation against `+static-stack`, `-disable-lsr`, and
   default-Oz configs (each may amplify different bug classes).

Realistic effort: **1-2 weeks per hook**, not "2-3 days total".

That makes Phase B substantially more work than the current
session-73p Phase 2 window allows.

## Recommended action

**Defer Phase B in its entirety.**  The bundle change is reverted in
the working tree (commit `ecd143c52df3` plan-update stays, but no
hook-override commit lands).  Document this finding (`Phase B1`) and
mark #177 as parked on the work clock pending dedicated multi-week
session.

**Confirmed:** the existing Phase 1 (#179 P1+P2, #128) and prior
overrides (`hasDivRemOp`, `isLSRCostLess`, `getPredictableBranchThreshold`,
`isValidAddrSpaceCast`, `getNumberOfRegisters`, `getRegisterBitWidth`,
`areInlineCompatible`, `prefersVectorizedAddressing`) are not
load-bearing miscompile risks — they've all been through the
oracle.  The remaining unreserved TTI hooks (Arith/Cast/CmpSel/CF/Memory)
are.

**Pivot to #173** (8-bit BSS spill peephole) for the remaining
session-73p Phase 2 effort.  Concrete MIR-level pattern, no IR-pass
ripple risk.

## Methodological lesson

Phase B0 violated my own discipline rule (`feedback_state_certainty`):
I asserted "near-zero impact" based on plausible-but-unvalidated
reasoning about which pass-decisions would flip.  The premise was
not tested with the full oracle.  The oracle ran in **one minute**
once I applied the bundle and immediately falsified the prediction.

**The rule (for future me):** when predicting "near-zero impact" from
a change that touches IR cost hooks, RUN THE FULL ORACLE before
documenting the prediction.  Costs/benefits of cost-hook changes are
non-obvious and cross-pass.

## Cross-references

- Phase A: `issue177-phase-a-investigation.md`
- Phase B0 (the wrong prediction): `issue177-phase-b0-investigation.md`
- Phase 1 lessons: `session73p-phase1-lessons.md`
- Memory rule cross-referenced: `feedback_state_certainty.md`,
  `feedback_no_commit_first_version.md`
