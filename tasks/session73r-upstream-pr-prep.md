# Session 73r — upstream-LLVM PR prep for #182 (deleteDeadLoop SSA-malforming)

**Date:** 2026-05-23
**Branch:** `session-73r` (off main at the session-73q closeout).
**Predecessor:** `session73q-issue182-fix.md` — root cause analysis and local fix.
**Outcome:** Target-agnostic unit test added.  PR draft ready below.  Submission gated on user authorization (per `feedback_no_pull_requests`).

## What landed in this session

- `llvm/unittests/Transforms/Utils/LoopUtilsTest.cpp`: new test `LoopUtils.DeleteDeadLoopExitIntoAnotherLoopHeader` that hand-crafts the offending CFG shape and calls `deleteDeadLoop` directly.  Test:
  - **PASSES** with the fix applied (the per-pred phi entries are preserved).
  - **FAILS** with the original code (phi has only 1 incoming for a block with 2 predecessors; control-tested by checking out the pre-fix file and rebuilding).

The unit test removes the need to construct a target-specific reproducer for upstream — `gtest` can exercise the utility directly on hand-crafted IR.

## Why a unit test, not a lit test

Upstream LLVM's only first-party caller of `deleteDeadLoop` is `LoopDeletionPass`, which runs within `LoopPassManager` and only ever sees LCSSA / loop-simplified IR.  In canonical form, every loop has its own dedicated preheader, so "loop1's exit block IS loop2's header" is structurally impossible.  Therefore the bug **does not** reproduce via any upstream-shipping pass pipeline — it only manifests when a custom downstream pass calls `deleteDeadLoop` on non-canonical IR (e.g. ravn/llvm-z80's `Z80LoopIdiomFill`).

A target-agnostic unit test calling `deleteDeadLoop` directly is the natural way to exercise the utility's API contract.  This matches the existing `LoopUtils.DeleteDeadLoopNest` test in the same file.

## Pre-submission checklist

- [x] Fix landed: `llvm/lib/Transforms/Utils/LoopUtils.cpp:556-588` (commit `6dc359f0c63c`).
- [x] Unit test added: `LoopUtils.DeleteDeadLoopExitIntoAnotherLoopHeader`.
- [x] Pre-fix test FAILS (control: phi has 1 incoming for a 2-pred block).
- [x] Post-fix test PASSES.
- [x] All other `LoopUtils.*` unit tests still PASS.
- [ ] **Confirm the bug is still present on upstream LLVM `main`** — not yet done; needs `git fetch llvm/llvm-project` and bisect.  Likely YES because the original code path in the fork-of-record is untouched by our recent llvm-z80 work, but should be empirically confirmed.
- [ ] **Write commit message + PR description per upstream conventions** — draft below.
- [ ] **User authorization to submit PR** — per `feedback_no_pull_requests` rule, the actual `gh pr create` requires explicit user request.

## Draft commit message (for upstream PR)

```
[Utils] Fix deleteDeadLoop SSA-malforming on exit blocks reachable from outside the loop

deleteDeadLoop's exit-block-phi update assumed every phi incoming in
the exit block came from one of the loop's exiting blocks.  This is
true for typical LCSSA-form exit blocks, but breaks when the exit block
is reachable from outside the loop too -- specifically when it's another
loop's header with its own backedge phi entries.

The original code:
  1) Set incoming block 0 to Preheader.
  2) Removed every other incoming entry.

The kept-and-remapped entry might be the OTHER loop's backedge entry,
not the exiting-block entry.  Result: malformed SSA -- the phi loses
entries for predecessors it still has, and the kept entry binds the
wrong predecessor.

Fix: iterate phi incoming values explicitly.  Only remap entries whose
source block is INSIDE the loop being deleted (i.e. real exiting-block
entries).  Keep entries from outside.  Remap exactly one from-loop
entry to Preheader; drop the rest.

Upstream's LoopDeletionPass runs within LoopPassManager and only ever
sees LCSSA / loop-simplified IR, so this bug doesn't manifest via any
shipping upstream pipeline.  It does manifest in downstream third-party
passes that call deleteDeadLoop on non-canonical IR.  Added unit test
LoopUtils.DeleteDeadLoopExitIntoAnotherLoopHeader exercises the API
directly with a hand-crafted shape.

This fix is target-agnostic.

Bug reported as ravn/llvm-z80#182 (Z80 downstream).
```

## Draft PR description

Title: `[Utils] Fix deleteDeadLoop SSA-malforming on non-LCSSA exit blocks`

Body:

```markdown
## Summary

`deleteDeadLoop` (`llvm/lib/Transforms/Utils/LoopUtils.cpp`) emits invalid SSA when the loop's exit block is reachable from outside the loop -- e.g. when the exit block is another loop's header with its own backedge phi entries.

The original code assumed every phi incoming in the exit block came from one of the loop's exiting blocks.  In canonical (LCSSA + loop-simplified) form this is always true: every loop has a dedicated preheader, so "loop1's exit is loop2's header" is structurally impossible after canonicalization.  But for downstream pass authors who call `deleteDeadLoop` directly on non-canonical input (e.g. immediately after a custom loop-idiom recognizer that hasn't run loop-simplify), the assumption breaks.

## Symptom

The original loop at `LoopUtils.cpp:556` set phi incoming-block 0 to Preheader and removed every other entry.  Block 0's identity is arbitrary -- it may be the OTHER loop's backedge entry, not the exiting-block entry.  Result: the kept entry binds the wrong predecessor, the dropped entries leave the phi with fewer incoming values than the block has predecessors, and downstream code that consumes the phi (RAUW chains, SCEV walks, etc.) hits malformed SSA.

In the original downstream report (ravn/llvm-z80#182), the corruption surfaced as `ScalarEvolution::createSCEVIter` walking a self-referencing add until its `SmallVector` overflowed at `UINT32_MAX` capacity -- a SCEV-side symptom of an upstream IR corruption.

## Fix

Iterate phi incoming values explicitly.  Only remap entries whose source block is INSIDE the loop being deleted; keep entries from outside.  Remap exactly one from-loop entry to Preheader; drop the rest.

## Test

New unit test: `LoopUtils.DeleteDeadLoopExitIntoAnotherLoopHeader` in `llvm/unittests/Transforms/Utils/LoopUtilsTest.cpp`.  Hand-crafts the offending CFG shape (two loops where loop1's exit IS loop2's header) and calls `deleteDeadLoop(loop1)` directly.  Verifies that loop2's phi has the correct number of incoming values matching its remaining predecessors after the deletion.

## No upstream user is affected today

Upstream LLVM's only first-party caller of `deleteDeadLoop` is `LoopDeletionPass`, which runs within `LoopPassManager` and only sees canonical IR.  This patch is purely defensive: it hardens the utility for downstream pass authors who call it from custom passes.  Target-agnostic.
```

## Mechanical next steps (for actual submission)

1. `git fetch upstream main` and rebase the fix + test onto upstream main HEAD.  Resolve any conflicts in `LoopUtils.cpp` (the line numbers might shift; the patch is small and locally contained, conflicts unlikely).
2. Run the full `check-llvm-unit` + `check-llvm-transforms` suites on upstream main + fix.
3. Submit as a GitHub PR to `llvm/llvm-project`.
4. Add link to the PR in #186.

These steps are **NOT** done in this session — requires user authorization per `feedback_no_pull_requests`.

## Risk assessment

- **Behavior change**: only for the path where the exit block has phi entries from outside the loop.  In canonical form, no upstream pass produces this input.  Zero behavior change for normal LoopDeletionPass usage.
- **Code-change size**: ~20 net lines (replace a 16-line block with a 32-line explicit-iteration version).
- **Maintenance burden**: low.  The new code is more verbose but more obviously correct.

## Files

- `llvm/lib/Transforms/Utils/LoopUtils.cpp` — fix (already committed).
- `llvm/unittests/Transforms/Utils/LoopUtilsTest.cpp` — unit test (new in this commit).
- `tasks/session73r-upstream-pr-prep.md` — this writeup.
