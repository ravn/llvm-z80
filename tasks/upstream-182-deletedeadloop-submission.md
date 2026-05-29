# Upstream-submission writeup — deleteDeadLoop malforms SSA on shared exit blocks (#182)

Status: fix landed in ravn/llvm-z80 main (`llvm/lib/Transforms/Utils/LoopUtils.cpp`).
This is the target-agnostic submission text for llvm/llvm-project.  Per project
policy the agent does NOT file/PR upstream — a human submits it.

## One-line

`llvm::deleteDeadLoop` rewrites the dead loop's exit-block phis assuming every
incoming value comes from one of the loop's exiting blocks.  When the exit
block is *also* reachable from outside the loop (e.g. it is another loop's
header, with its own backedge phi entries), it drops/mis-binds the non-loop
entries, producing malformed SSA (instructions that reference their own SSA
name).

## Where the bug is

`llvm/lib/Transforms/Utils/LoopUtils.cpp`, `deleteDeadLoop`, the
"Rewrite phis in the exit block" step.  Original code (paraphrased):

```cpp
for (PHINode &P : ExitBlock->phis()) {
  int PredIndex = 0;
  P.setIncomingBlock(PredIndex, Preheader);          // keep entry 0, point at preheader
  P.removeIncomingValueIf([](unsigned Idx) { return Idx != 0; }, false); // drop the rest
  assert(P.getNumIncomingValues() == 1 && ...);
}
```

The implicit assumption: *all* of `ExitBlock`'s phi incomings come from this
loop's exiting blocks, so collapsing to a single entry (remapped to the
preheader) is correct.  That holds for a dedicated LCSSA exit block reachable
only from the loop.

## Why it is wrong

When `ExitBlock` is reachable from outside the loop too, its phis have entries
from non-loop predecessors.  The code:
1. keeps entry index 0 and remaps its predecessor to `Preheader` — but entry 0
   may be a *non-loop* predecessor (e.g. another loop's backedge), so it binds
   the wrong value to the preheader edge; and
2. removes every other entry — including legitimate non-loop predecessors.

The phi loses real incoming edges and may bind the wrong one.  When such a phi
is later folded to single-entry form, RAUW replaces its uses with that (wrong)
value, yielding instructions that reference their own SSA name
(`%v = add %v, 1` outside any phi) — malformed SSA.

## Observed downstream crash (the witness)

In ravn/llvm-z80, `Z80LoopIdiomFill` recognizes a buffer-fill loop, rewrites it
to `store + memcpy`, and calls `deleteDeadLoop(L1)`.  For two sequential loops
over the same array, `L1`'s exit block *is* `L2`'s header.  `L2`'s header phis
had one from-`L1` entry (the init value) and one from-`L2`-backedge entry; the
buggy code kept the backedge entry, remapped it to `L1`'s preheader, and dropped
the init entry.  The resulting self-referencing phis, after folding, made
`ScalarEvolution::createSCEVIter` walk a self-cycle until `SmallVector`'s
`grow_pod` aborted at `UINT32_MAX` — surfacing as a ScalarEvolution
"SmallVector capacity overflow" crash (originally mis-filed as a SCEV bug).
The bug is in **generic** `deleteDeadLoop`; the Z80 backend merely exposes it
via `Z80LoopIdiomFill` (other targets lack that pass, so they don't reproduce —
but any caller of `deleteDeadLoop` with an externally-reachable exit block is
vulnerable).

## The fix

Only remap phi entries whose source block is *inside* the loop being deleted;
keep entries from outside; remap exactly one from-loop entry to the preheader
and drop any redundant from-loop entries:

```cpp
for (PHINode &P : ExitBlock->phis()) {
  int FirstLoopIdx = -1;
  SmallVector<unsigned, 4> ToRemove;
  for (unsigned I = 0, E = P.getNumIncomingValues(); I < E; ++I) {
    if (L->contains(P.getIncomingBlock(I))) {
      if (FirstLoopIdx < 0) FirstLoopIdx = I;
      else ToRemove.push_back(I);
    }
  }
  assert(FirstLoopIdx >= 0 && "ExitBlock unreachable from this loop");
  P.setIncomingBlock(FirstLoopIdx, Preheader);
  for (auto It = ToRemove.rbegin(); It != ToRemove.rend(); ++It)
    P.removeIncomingValue(*It, /*DeletePHIIfEmpty=*/false);
}
```

Strictly more correct: for the common single-loop-reachable exit block it is
identical to the old behavior (one from-loop entry → remapped); it only differs
when non-loop entries exist, which the old code corrupted.

## Test

A target-agnostic reduction: two adjacent loops where the first is deleted by a
pass that calls `deleteDeadLoop`, and the first loop's unique exit block is the
second loop's header (so that header's phis carry a from-first-loop entry plus a
second-loop backedge entry).  After deletion the second loop's header phis must
retain their backedge entries and bind the preheader edge to the *from-first-loop*
value.  The fork's concrete witness is the 7-line C repro in ravn/llvm-z80#182
(two sequential fills over one array, `-O1+`), which crashed pre-fix and
compiles cleanly post-fix.

## Risk

`deleteDeadLoop` is a widely-used generic utility, but the change only alters
behavior for exit blocks with non-loop phi predecessors (previously corrupted);
the dominant single-loop-reachable case is unchanged.
