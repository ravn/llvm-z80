# Phase A investigation: which IR passes consult which TTI hooks?

Date: 2026-05-21 (session 73p Phase 2).  Per the implementation plan,
Phase A is the prerequisite that validates the implementation
priority by mapping passes to hooks empirically.

## Method

1. Build AES corpus at production flags (`-Oz +static-stack
   -disable-lsr -ffunction-sections -fdata-sections`).
2. Run `opt -O2 -print-after-all` to enumerate IR passes that fire.
3. For each pass, grep its source in
   `llvm/lib/Transforms/` for `TTI.` / `TTI->` / direct hook calls.
4. Tabulate: pass → hook(s) called → in our TTI? → priority.

## Pass list (post-grouping)

88 unique passes fire across the AES corpus at -Oz.  Grouping by
purpose:

| Group | Passes | TTI relevance |
|---|---|---|
| Setup | Annotation2Metadata, ForceFunctionAttrs, etc. | none |
| Simplifications | InstCombine, SimplifyCFG, EarlyCSE, SROA | **high** (SimplifyCFG, EarlyCSE) |
| Inlining | InlinerPass, AlwaysInlinerPass, PostOrderFunctionAttrs | **high** |
| Loops | LICM, LoopRotate, LoopUnroll, LoopIdiom, IndVarSimplify, LoopVectorize | **high** (LICM, IndVarSimplify) |
| GVN | GVN, EarlyCSE, MergedLoadStoreMotion | medium |
| DCE | ADCE, BDCE, DeadStoreElim | low (no TTI use) |
| Vectorization | LoopVectorize, SLPVectorize, VectorCombine | **N/A** on Z80 (no SIMD) |
| Z80-specific | Z80IndexIV, Z80LoopIdiomFill, Z80LoopRotate, Z80NarrowIV | own logic; no TTI |
| Late | MergeICmps, ExpandMemCmp, Float2Int, LowerConstantIntrinsics | medium |
| Other | various | mostly low |

## TTI hook → calling-pass map

Hooks ranked by number of distinct callers in the IR pipeline:

| TTI hook | Used by | Priority for Z80 |
|---|---|---|
| **`getInstructionCost`** | LICMPass (line 1303), LoopUnroll (462, 558), SimplifyCFG (430), InlinerPass (3425) | **TIER 1** — highest leverage |
| `getPredictableBranchThreshold` | SimplifyCFG (3163, 3228, 3723, 3791, 3932) | **already handled** (returns 0/1) |
| `getArithmeticInstrCost` | IndVarSimplify (646), LICM, vectorizers | TIER 2 |
| `getCmpSelInstrCost` | SimplifyCFG (3111), vectorizers | TIER 2 |
| `getUnrollingPreferences` | LoopUnroll (226) | TIER 2 (Z80 should mostly disable unroll) |
| `isProfitableToHoist` | SimplifyCFG (1588) | TIER 2 |
| `isLoweredToCall` | LoopUnroll (577) | TIER 2 |
| `getCastInstrCost` | LICM, IndVarSimplify | TIER 2 |
| `enableMemCmpExpansion` | MergeICmps (892) | TIER 3 (Z80 should disable: use CPIR) |
| `hasBranchDivergence` | SimpleLoopUnswitch (3703) | TIER 3 (return false) |
| `getIntImmCostInst`, `getIntImmCostIntrin`, `preferToKeepConstantsAttached` | ConstantHoisting | TIER 3 |
| `getMemoryOpCost` | LoopVectorize, SLP, IROutliner, VectorCombine | **TIER 5** — vectorize-only, low leverage on Z80 |
| `getCFInstrCost` | LoopVectorize, IROutliner, ScalarEvolutionExpander | **TIER 5** — vectorize-mostly |

**Important revisions to the prior plan:**

1. `getMemoryOpCost` is **lower priority** than the original plan
   stated.  It's used almost exclusively by vectorizers, which Z80
   doesn't need.

2. `getCFInstrCost` similarly is **lower priority** — also vectorize-
   dominant.

3. `getInstructionCost` remains **highest priority** — it's the
   "swiss army knife" cost hook used by LICM, LoopUnroll,
   SimplifyCFG, and Inliner all together.

## Critical Phase A finding — MachineLICM / MachineCSE do NOT use TTI

**Empirical check:**

```
$ grep -nE "TTI\.|TTI->|TargetTransformInfo" \
    llvm/lib/CodeGen/MachineLICM.cpp \
    llvm/lib/CodeGen/MachineCSE.cpp
(no output)
```

MachineLICM and MachineCSE use `TargetInstrInfo`,
`MachineRegisterInfo`, and their own pressure-tracking — but NOT
TTI hooks.  Confirmed by reading both files.

**Implication for the plan:** Phase E (which proposed making
MachineLICM/MachineCSE respect optsize/minsize via TTI cost
hooks, enabling a revert of #128's `disablePass()` workaround) is
**NOT VIABLE via TTI alone**.

The MachineLICM/MachineCSE pessimization on Z80 can ONLY be
addressed by either:

1. Keeping #128's global `disablePass()` workaround (current).
2. Modifying MachineLICM/MachineCSE upstream-LLVM to consult
   function attributes (optsize/minsize) and skip when present
   in size-prioritized configs.
3. Implementing Z80-specific MachineLICM/MachineCSE subclasses
   that override the pressure-tracking heuristics with TII-based
   Z80-specific cost knowledge.

Path 1 is what we have.  Path 2 is upstream-LLVM-level work
(months, not weeks).  Path 3 is feasible but substantial.

**Recommendation:** retire Phase E from the plan.  Keep #128's
workaround; track the TTI-vs-MachineLICM gap as ravn/llvm-z80
follow-up #128B if a future MachineLICM modification becomes
warranted.

## Revised Phase B priorities

Based on Phase A findings, the Tier 1 implementation order shifts:

### Tier 1 (high leverage — implement first)

1. **`getInstructionCost`** (~all-leverage hook).  Used by 4 critical passes.
   Default: returns 1 for most ops (RISC-style).  Z80 should
   return realistic byte/cycle estimates.
2. **`getUnrollingPreferences`**.  Z80 unrolling rarely pays off
   (branches are cheap; unrolled bodies expand the BSS-spill traffic).
   Default UP allows substantial unrolling at -O2/-O3.  Z80 should
   tighten thresholds.
3. **`isProfitableToHoist`**.  Used by SimplifyCFG (line 1588).
   Default: returns true.  For Z80, hoisting non-A 8-bit operations
   often costs more than the redundant compute (BSS-spill cost).

### Tier 2 (medium leverage)

4. **`getArithmeticInstrCost`**.  IndVarSimplify (646), LICM, etc.
5. **`getCmpSelInstrCost`**.  SimplifyCFG (3111).
6. **`getCastInstrCost`**.  LICM, IndVarSimplify.
7. **`isLegalAddImmediate` / `isLegalICmpImmediate`**.  LSR
   (gated off by -disable-lsr), CSE.

### Tier 3 (low leverage / cleanup)

8. **`enableMemCmpExpansion`** → false.  Use CPIR-based memcmp.
9. **`hasBranchDivergence`** → false.
10. **`getIntImmCostInst`**, **`getIntImmCostIntrin`** — ConstantHoisting tuning.
11. **`isLoweredToCall`** — LoopUnroll: confirm that calls block unroll.

### Tier 4 (no-vectorization cluster — mechanical)

12. `getMaxInterleaveFactor` → 1.
13. `prefersVectorizedAddressing` → false.
14. `enableInterleavedAccessVectorization` → false.
15. `getMinimumVF` / `getMaximumVF` → 1.
16. `getMemoryOpCost` (vectorize-only) → if needed, conservative.
17. `getCFInstrCost` (vectorize-only) → if needed, conservative.

### Tier 5 (RETIRED — see Phase A finding)

~~Per-function optsize/minsize gating via TTI~~ — not viable;
MachineLICM/CSE don't use TTI.  Replaced by: track as a separate
upstream-LLVM gap (TBD whether to file as #128B).

## Revised effort estimate

- Phase A (this doc): ~30 min focused investigation
- Phase B (Tier 1 + 2): 1-2 weeks
- Phase C (was Tier 2, now subset): 3-5 days
- Phase D (Tier 3 + 4 cleanup): 1-2 days
- ~~Phase E~~ retired.
- Phase F (Tier 4 exploratory: `getUnrollingPreferences` partial):
  already moved into Phase B item 2.

**Revised total: 2-4 weeks** (down from 4-6 weeks).  Phase E
retirement removes ~1 week.

## Conservative target window (revised)

- Aggressive: 2026-06-04 (~2 weeks, Tier 1 + 2 + 3 + 4 done)
- Conservative: 2026-06-18 (~4 weeks, with iteration on tuning)

(Was: 2026-06-18 to 2026-07-02.  Phase E retirement pulls in by
~1 week.)

## What the implementation plan should NOT do anymore

- Don't promise that #128's global disablePass() will be reverted.
  It can't be reverted via TTI alone.  The expedient stays.
- Don't pursue `getMemoryOpCost` or `getCFInstrCost` as Tier 1.
  They're vectorize-dominant; Z80 doesn't vectorize.  Keep them
  in Tier 4 as no-vectorization-cluster cleanup with conservative
  default values.

## What the plan SHOULD prioritize

- `getInstructionCost` first.  Touches 4 critical passes.
  Empirically verify the AES corpus improves at -O2 / -Oz / -O3.
- `getUnrollingPreferences` second.  Z80 unrolling pessimizes;
  return conservative UP.
- `isProfitableToHoist` third.  SimplifyCFG hoist-decision-cost.

After Tier 1 (~1 week of focused work), pause to measure AES
corpus impact + cpnos PROM1 + test-runner before continuing.

## Memory rule violation check

This investigation invalidated the plan's prior premise that
Phase E was viable.  Per `feedback_no_commit_first_version` and the
project_z80_backend_unfinished framing: the lesson is "validate
the premise empirically before committing to multi-week work."

Phase A's investigation cost ~30 min and saved ~1 week of misdirected
Phase E work.  ROI of investigation: ~50× on time.

## Status update

- Phase A: **complete** (this doc).
- Phase B Tier 1: ready to start; new priority is
  `getInstructionCost` → `getUnrollingPreferences` →
  `isProfitableToHoist`.
- Work clock: revised target 2026-06-04 (aggressive) / 2026-06-18
  (conservative).
