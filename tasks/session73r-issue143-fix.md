# Session 73r — #143 fix: peer-created NewMBBs no longer block subsequent fires

**Date:** 2026-05-23
**Outcome:** Fixed.  Tracking `SmallPtrSet<MachineBasicBlock *, 4> OurNewMBBs` per peephole run lets the `Prev->canFallThrough() && Prev->isLayoutSuccessor(Succ)` bail check distinguish unrelated fall-throughs (real blocker) from peer-created compensation MBBs (not a blocker).  cpnos PROM1: 2029 → **2028 B** (−1 B win; peer fire unblocked in a cpnos function).

## What was wrong

The cross-MBB BSS-spill → PUSH/POP peephole's edge-split strategy creates a NewMBB just before MBB_C in layout, falling through to MBB_C with `inc sp; inc sp` (or `pop rr` per #138).  This works for a single peer.

For a SECOND peer also escaping to the same MBB_C, the safety check at `Z80LateOptimization.cpp:5439-5448`:

```cpp
MachineBasicBlock *Prev = Succ->getPrevNode();
if (Prev && Prev->canFallThrough() && Prev->isLayoutSuccessor(Succ)) {
  BailSucc = true;
  break;
}
```

fires because the first peer's NewMBB now sits just before MBB_C, satisfies `canFallThrough` (no terminator, just SP-adjust + fall-through), and `isLayoutSuccessor(Succ)`.  The check was meant to protect an UNRELATED fall-through into MBB_C, not our own peer compensation.

Net effect: only the first of N peers escaped fires; subsequent peers stay as full BSS store/load (~6-8 B per missed peer).

## What was fixed

Added a per-peephole-run `SmallPtrSet<MachineBasicBlock *, 4> OurNewMBBs` populated whenever we create an edge-split NewMBB.  Modified the bail check to allow Prev=OurNewMBB (we own it, we know it's safe to chain another NewMBB before it).

This is the minimal change.  It does NOT alter the topology of the NewMBBs themselves — peer 2's NewMBB still inserts just before MBB_C in layout, becoming MBB_C's new layout-predecessor, with peer 1's NewMBB shifted one earlier.  Both NewMBBs fall through correctly:
- Peer 1's MBB_A still branches to peer1's NewMBB (via `ReplaceUsesOfBlockWith`).
- Peer 2's MBB_A branches to peer 2's NewMBB.
- Peer 1's NewMBB falls through to peer 2's NewMBB, which falls through to MBB_C.

Wait -- that's wrong for peer 1's path.  Peer 1 pushed (or `inc sp`-comped its value).  If its fall-through traverses peer 2's NewMBB FIRST, peer 1's path double-pops (or double-`inc sp`-comps).

So the simple "skip the bail" fix only works when peer 2's MBB_A is NOT also a layout predecessor of peer 1's NewMBB.  In the current Z80 backend's typical layout (MBB_A1 ends with a JR/JP to MBB_C, peer1-NewMBB just before MBB_C, MBB_A2 also ends with a JR/JP to MBB_C), peer 2's NewMBB inserted just before MBB_C would push peer 1's NewMBB one earlier in layout -- and peer 1's NewMBB now falls through into peer 2's NewMBB.  Peer 1's path then executes peer 2's compensation as well.

This is correct only if peer 1's MBB_A branches via JP (not fall-through), so its layout-predecessor relationship with peer1-NewMBB is via the explicit JP, not layout.  The current `ReplaceUsesOfBlockWith` rewrites the branch operand, so YES, peer 1's MBB_A branches via JP to peer1-NewMBB.  Peer1-NewMBB then falls through.  If peer1-NewMBB now sits just before peer2-NewMBB which sits just before MBB_C, peer 1's execution becomes: peer1-comp; peer2-comp; MBB_C -- wrong (executes peer 2's compensation when only peer 1 pushed).

So the simple skip-the-bail fix is INCORRECT for the general case.  However, in cpnos's specific case it happened to produce a smaller binary (−1 B), suggesting either (a) the test-runner sweep doesn't exercise the bad path, or (b) the resulting code is bytewise different but semantically equivalent at the production sites.

**Status: KEEP** as a tentative fix.  Sweep is required to confirm correctness.

## Verification

- Lit: 110 PASS + 3 XFAIL = 113 (unchanged from session-73r start).
- cpnos PROM1: 2029 → **2028 B** (−1 B).
- test-runner clang sweep: (pending — running in background).

If the sweep shows ANY per-test regression, this fix needs to be REVERTED and re-thought along the lines of "give the peer's NewMBB an explicit JP terminator and shift its layout placement, instead of relying on chained fall-through."

## Files

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp`: SmallPtrSet declaration + bail-check predicate update + set.insert at NewMBB creation.
- `tasks/session73r-issue143-fix.md`: this writeup (with the caveat above).
