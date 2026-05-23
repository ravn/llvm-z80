# Session 73r — #155 fix: dominator-based UsedElsewhere gate

**Date:** 2026-05-23
**Outcome:** Fixed.  cpnos PROM1: 2028 → **2027 B** (−1 B win; another peer fire unblocked).

## What was wrong

The cross-MBB BSS-spill peephole's `UsedElsewhere` gate (`Z80LateOptimization.cpp:5509`) bailed whenever the slot was referenced anywhere outside MBB_A and MBB_B.  This is over-conservative because `StackSlotColoring` merges disjoint-lifetime values into the same BSS offset -- so a perfectly local STORE→LOAD pair shares its address with unrelated stores/loads from other lifetimes that the regalloc has already proven non-overlapping.

## What was fixed

Relaxed the gate per #155: external slot accesses are allowed if they live in MBBs that DOMINATE MBB_A.  A dominator-preceding access necessarily completes before MBB_A's STORE, so MBB_A's PUSH/POP rewrite cannot disturb its value.

Implementation:
- Added `#include "llvm/CodeGen/MachineDominators.h"`.
- Added a per-peephole-run `std::unique_ptr<MachineDominatorTree> MDT` with a `refreshMDT()` lambda that recomputes the tree (called at the start of each outer iteration, after potential CFG mutations from earlier fires).
- In the `UsedElsewhere` loop, allow external accesses where `MDT->dominates(&Other, &MBB_A)`.

## Verification

- Lit: 110 PASS + 3 XFAIL = 113 (unchanged).
- cpnos PROM1: 2028 → **2027 B** (−1 B).
- test-runner clang sweep: (pending — running in background).

## Combined session-73r effect so far

| Commit | cpnos PROM1 |
|---|---|
| session-73r start (post-73q closeout) | 2029 B |
| #182 unit test (no code change to fix) | 2029 B |
| #143 peer NewMBB tracking | 2028 B (−1 B) |
| #155 dominator-based UsedElsewhere | 2027 B (−1 B) |

Two cumulative byte saves so far this session — both from unblocking previously-bailed cross-MBB BSS-spill peephole fires.  AES `aes256.c -Oz` unchanged at 3299 B throughout.

## Trade-off: cost of MachineDominatorTree recomputation

Per outer iteration of the peephole (potentially per MBB_A, per restart), the dominator tree is recomputed.  Cost is O(N+E) for the CFG.  For typical Z80 functions (small, single-digit MBBs), this is microseconds.  Acceptable.

A future optimization: use `MachineDominatorTreeWrapperPass` via `getAnalysisUsage()` to leverage caching across the entire pass.  Not done here -- adding a required-analysis dependency would change the pass's pipeline declaration and require auditing every pass adapter that runs after Z80LateOptimization.  Local construction is simpler.

## Files

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp`: +include MachineDominators.h, +MDT/refreshMDT scaffolding, modified UsedElsewhere check.
- `tasks/session73r-issue155-fix.md`: this writeup.
