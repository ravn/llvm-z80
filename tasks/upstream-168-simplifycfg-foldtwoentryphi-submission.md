# Upstream-submission writeup — SimplifyCFG foldTwoEntryPHINode cost gate for no-branch-prediction targets (#168)

Status: landed in ravn/llvm-z80 main (`SimplifyCFG.cpp`, commit `cd2a2ace8754`).
Target-agnostic submission text for llvm/llvm-project.  Agent does not PR
upstream (project policy); a human submits.

## One-line

`SimplifyCFG::foldTwoEntryPHINode` converts a two-entry PHI (a small if/else
diamond) into a `select`, speculatively executing **both** arms.  On a target
with no branch predictor whose `select` lowers back to a conditional branch
anyway, that is a net loss: the speculated work runs unconditionally every
iteration instead of being skipped half the time.  Gate the fold so such
targets only take it when the speculated cost is free.

## The improvement

In `foldTwoEntryPHINode`, after the existing cost computation, bail when the
target reports no useful branch prediction and the speculated cost isn't free:

```cpp
// Cost-gated bailout for targets with no branch prediction (e.g. Z80).
// select must be lowered to a branch anyway, so speculating both arms then
// picking via select runs the compute-both pattern unconditionally every
// iteration, whereas a conditional branch skips it half the time.
if (TTI.getPredictableBranchThreshold().isZero() &&
    Cost > TargetTransformInfo::TCC_Free)
  return Changed;
```

## Why it is safe / generic

- It keys off the existing `TargetTransformInfo::getPredictableBranchThreshold()`
  hook.  Only targets that return `isZero()` (explicitly "branches are not
  predictable / no predictor") change behavior; every other target is
  unaffected.
- For those targets it only *declines* a transform (keeps the branch) when the
  speculated cost is more than `TCC_Free`; free folds (e.g. a constant that
  materializes for free in the parent block) still happen.
- It is the SimplifyCFG analogue of the same principle already applied
  elsewhere via `getPredictableBranchThreshold` (e.g. the SpeculativelyExecute
  paths), extended to the two-entry-PHI→select fold.

## Motivation / witness

ravn/llvm-z80#167: the AES-256 `gf_alog`/`gf_log` hot loop contains
`x = cond ? a : b`-shaped diamonds whose arms are non-trivial.  Folding them to
`select` forced both arms to run every iteration on the Z80 (no predictor,
`select` → branch), measurably regressing the hot loop; keeping the branch
(skipping one arm half the time) is faster and smaller.  With this gate the AES
hot paths branch instead of compute-both.

## Note

The "compute-both vs branch" tradeoff is fundamentally a target cost-model
question; this gate lets a no-branch-prediction target opt out of an
SSA-cleanup transform that the generic heuristic assumes is beneficial.  A
more aggressive variant (lower the cost threshold further for `isZero()`
targets) is possible but unneeded for the witnessed cases.
