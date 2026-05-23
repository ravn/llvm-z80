# Session 73q — A1 patch attempt (#182 defensive cycle detection in createSCEVIter)

**Date:** 2026-05-23
**Predecessor:** `session73q-A1-drill-182.md`.
**Outcome:** NEGATIVE.  The "defensive cycle detection" approach the A1 drill proposed cannot be implemented as a self-contained patch.  Reverted to baseline.

## What I tried

Per the A1 drill's "Option 1 — Defensive" path: add a self-reference check in `ScalarEvolution::createSCEVIter` so that a malformed `%v = add %v, 1` (the post-LoopRotate corruption from #182) is treated as a `SCEVUnknown` rather than growing the worklist past `UINT32_MAX`.

Two variants tried:

1. **Direct self-reference check** (cheap): for each operand `Op` returned by `getOperandsToCreate`, check if `Op == CurV`.  If yes, call `insertValueToMap(CurV, getUnknown(CurV))` and stop pushing operands.

2. **Pending-set cycle detection** (general): a `SmallPtrSet<Value*, 16> Pending` tracking values whose operand-fetch step has been queued.  Re-encountering an `Op` already in `Pending` short-circuits via `getUnknown`.

Both variants successfully prevent the original `SmallVector::grow_pod` overflow.

## Why both failed

After `createSCEVIter` returns `SCEVUnknown` for the malformed value, the caller (`computeExitLimitFromICmp` -> `LoopDeletionPass::run`) does NOT handle the unknown gracefully.  Instead of computing a conservative "cannot delete" decision, a downstream code path asserts or hits a `__builtin_unreachable`, producing **SIGILL (exit 132)** with empty stderr (no LLVM stack trace because the trap fires before the signal handler's `report_fatal_error` machinery is invoked).

Curiously, the failure mode depends on whether opt's stdout is a TTY or redirected:
- `opt ... -S /tmp/r.ll` to terminal: exit 0, runs cleanly.
- `opt ... -S /tmp/r.ll > /tmp/out.ll` to file: exit 132.
- `opt ... -o /tmp/out.bc`: exit 132.

The TTY/redirect distinction is incidental — likely a memory-layout artifact that exposes the downstream bug in one mode and hides it in the other.  But it confirms the issue is downstream of `createSCEVIter`, not in the worklist fix itself.

## What this means for #182

The A1 drill's "Option 1 defensive patch ... small, low risk" estimate was wrong.  Making SCEV's worklist resilient to malformed SSA is straightforward (one of the two variants above); making the **rest of the LoopAnalysis / LoopDeletion stack** resilient to `SCEVUnknown` results on values where it expected AddRec or constant is a much wider patch with unclear scope.

The right fix is upstream of `createSCEVIter`: prevent `LoopRotate` from emitting the invalid SSA in the first place.  That's the A1 drill's "Option 2 — real fix."  Cost-of-upstream-LoopRotate-fix dominates this drill, not the cycle-detection.

## Files

- Patch attempts: reverted via direct revert (the file is back to the unmodified state).  `git diff --stat llvm/lib/Analysis/ScalarEvolution.cpp` is empty.

## Updated next-steps for #182

1. **Drop "Option 1 — Defensive" from the queue.**  The patch's own complexity is low; the surrounding-code resilience requirement is high.  Not a cheap drill.
2. **Pivot to Option 2 — root-cause LoopRotate.**  This was the A1 drill's secondary path.  Needs investigation of `llvm/lib/Transforms/Utils/LoopRotationUtils.cpp` for the phi-strip code path that produces the self-referential adds.  Estimated cost: half-day to identify + write fix, plus upstream-PR cycle.
3. **Z80 cross-check (cheap, independent):** the #182 trigger requires the Z80 16-bit-pointer datalayout (x86_64 datalayout doesn't reproduce).  Likely the LoopRotate code path that fails has implicit assumptions about pointer-vs-index width that only the Z80 datalayout violates.  If a one-line LoopRotate fix can be found via datalayout-awareness, it'd ship faster than a full SSA-corruption audit.
