# Session 73q — #182 root cause + fix: upstream deleteDeadLoop SSA-malforming bug

**Date:** 2026-05-23
**Predecessors:** `session73q-A1-drill-182.md` (initial reclassification from "SCEV bug" to "LoopRotate corruption"), `session73q-A1-patch-attempt.md` (defensive SCEV patch attempted, blocked).
**Outcome:** ROOT-CAUSED and FIXED.  The actual bug was in upstream LLVM's `deleteDeadLoop` utility, not in LoopRotate.  ~20-line patch lands the fix in `llvm/lib/Transforms/Utils/LoopUtils.cpp`.

## Root cause

`llvm::deleteDeadLoop` (at `llvm/lib/Transforms/Utils/LoopUtils.cpp:556-571` pre-fix) updates phi nodes in the loop's `ExitBlock` to redirect their incoming edges from the exiting block to the preheader.  The implementation:

```cpp
for (PHINode &P : ExitBlock->phis()) {
  int PredIndex = 0;
  P.setIncomingBlock(PredIndex, Preheader);
  P.removeIncomingValueIf([](unsigned Idx) { return Idx != 0; },
                          /* DeletePHIIfEmpty */ false);
  assert(P.getNumIncomingValues() == 1 && ...);
}
```

This assumes **every phi incoming in ExitBlock comes from this loop's exiting blocks** — which is true for typical LCSSA-form exit blocks.  But: when ExitBlock is reachable from OUTSIDE the loop too — e.g. when ExitBlock is the HEADER of another loop with its own backedge phi entries — the entries from those non-loop predecessors got mistakenly removed.  Worse, entry 0 (kept and remapped to Preheader) might be the OTHER loop's backedge entry, not the exiting-block entry.

The result is malformed SSA: the phi loses entries for predecessors it still has, and the kept-and-remapped entry might bind the wrong predecessor.  When that phi got later folded into a single-entry form, RAUW replaced its uses with the latch value, producing instructions that reference their own SSA name (`%v = add %v, 1` outside any phi).

## Trigger for #182

Two sequential loops over the same array.  `Z80LoopIdiomFill` recognized the first loop as a buffer-fill pattern and rewrote it to `store + memcpy` followed by `deleteDeadLoop(L1)`.  L1's "ExitBlock" was L2's header.  L2's header phis had:
- one entry from L1's exiting block (the first-iteration init: `[0, %1]`)
- one entry from L2's own backedge (the second-iteration update: `[%next, .preheader_self_pred]`)

`deleteDeadLoop` mistakenly took entry 0 (which happened to be L2's backedge), remapped its predecessor block to L1's preheader, and dropped the legitimate `[0, %1]` initialization entry.  Two phis at L2's header (`%z80-indexiv.iv9` i8 + `%.17` i16) both lost their backedge entries; L2's body kept using them; later DCE/cleanup folded the now-single-entry phis with RAUW, yielding self-referencing adds; SCEV's `createSCEVIter` then walked the self-cycle and the worklist grew until SmallVector's `grow_pod` aborted at `UINT32_MAX`.

This is reachable from clean C source — the 7-line repro in #182 is the minimal case.  It's NOT specific to Z80 datalayout per se; the trigger is Z80LoopIdiomFill + the dual-loop-over-same-array shape.  Other targets don't reproduce because they don't have Z80LoopIdiomFill.

## Fix

Iterate explicitly over phi incoming values and only remap entries whose source block is INSIDE the loop being deleted.  Keep entries from outside.  Remap exactly one from-loop entry to Preheader; drop the rest.

Patch in `llvm/lib/Transforms/Utils/LoopUtils.cpp`:

```cpp
for (PHINode &P : ExitBlock->phis()) {
  int FirstLoopIdx = -1;
  SmallVector<unsigned, 4> ToRemove;
  for (unsigned I = 0, E = P.getNumIncomingValues(); I < E; ++I) {
    if (L->contains(P.getIncomingBlock(I))) {
      if (FirstLoopIdx < 0)
        FirstLoopIdx = I;
      else
        ToRemove.push_back(I);
    }
  }
  assert(FirstLoopIdx >= 0 && "ExitBlock unreachable from this loop");
  P.setIncomingBlock(FirstLoopIdx, Preheader);
  for (auto It = ToRemove.rbegin(); It != ToRemove.rend(); ++It)
    P.removeIncomingValue(*It, /* DeletePHIIfEmpty */ false);
}
```

## Verification

- **#182 minimal repro** (`/tmp/scev182/repro.c`, 7 lines): clang -O1 now exits 0 (was SIGSEGV/SmallVector overflow).
- **opt -passes='default<O1>'** on the same IR: exits 0 (was SIGILL).
- **Lit suite**: 109 PASS + 3 XFAIL (unchanged) + 1 new test (`issue-182-deletedeadloop-phi.ll`) → 110 PASS + 3 XFAIL = 113.
- **AES `aes256.c -Oz` `.text`**: 3299 B (unchanged).
- **cpnos PROM1 (clang)**: 2029 B (unchanged from before this fix — the fix only affects the pathological IR shape, not common code).
- **test-runner clang sweep**: (pending).

## Upstream submission

This is an UPSTREAM LLVM bug.  The fix lives in generic `llvm/lib/Transforms/Utils/LoopUtils.cpp` with no Z80 dependencies.  Submit to `llvm/llvm-project`:
1. Create a minimal reduced repro (a target-agnostic loop pair that exercises `deleteDeadLoop`).
2. Filed as an LLVM issue + PR per Track A workflow.
3. Add to #186 (upstream submission queue).

The minimal upstream-target repro:
```c
// Compile with any LLVM target that runs deleteDeadLoop on this shape.
char a[100];
void g(void) {
  unsigned short i;
  for (i = 0; i < 100; ++i) a[i] = 0;
  for (i = 0; i < 100; ++i) ++a[i];
}
```
With a target that has a loop-idiom pass that recognizes the first loop as memset.

## Files

- `llvm/lib/Transforms/Utils/LoopUtils.cpp`: ~20 LOC replaced.
- `llvm/test/CodeGen/Z80/issue-182-deletedeadloop-phi.ll`: new lit test.
- `tasks/session73q-issue182-fix.md`: this writeup.

## Time cost

~50 minutes total drilling.  Surprising: the A1 drill estimate was "half-day for root cause," but the actual root-cause emerged quickly once I dumped the IR pass-by-pass.  Lesson: dump IR at every pass boundary BEFORE hypothesizing about which pass is at fault.

## A1 reclassification (correction)

The A1 drill named LoopRotate as the corruption source.  The actual corruptor is `deleteDeadLoop`, several passes earlier.  LoopRotate inherited the malformed IR; SCEV's `createSCEVIter` walked into it later via LoopDeletionPass.  The "Route 1" plan (datalayout-aware LoopRotate fix) was directionally wrong; the bug is target-agnostic upstream.

## Closing

Closes #182 once the test-runner sweep confirms no behavioral regression.  The upstream-LLVM submission stays as queued work for next session.
