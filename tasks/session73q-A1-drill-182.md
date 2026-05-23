# Session 73q — Track A drill A1 (#182 SCEV crash)

**Date:** 2026-05-23
**Budget:** 30 min (per `execution-plan-2026-05-22.md`)
**Outcome:** GO. Root cause shifted from "SCEV bug" to "LoopRotate produces corrupt SSA"; upstream-LLVM, not fork-specific.

## TL;DR

The crash signature in #182 ("SCEV SmallVector unable to grow at 4294967296") is a **symptom**, not the root cause. The actual bug is upstream LLVM's `LoopRotatePass` emitting invalid SSA (instructions referencing their own SSA name as an operand) when given a specific loop shape under the Z80 data layout. SCEV then walks `%v -> %v -> %v ...` in its worklist-based `createSCEVIter` and grows until the SmallVector overflows.

## Repro (already minimal — 7 lines, opt-only, no Z80 backend needed)

```c
unsigned char a[100];
void g(void) {
   unsigned short i;
   for (i = 0; i < 100; ++i) a[i] = 0;
   for (i = 0; i < 100; ++i) ++a[i];
}
```

```bash
build-macos/bin/clang --target=z80 -O1 -Xclang -disable-llvm-passes -emit-llvm -S repro.c -o repro.ll
build-macos/bin/opt -passes='default<O1>' -S repro.ll   # crashes
```

`opt` does NOT run the Z80 target callbacks (Z80LoopIdiomFill, Z80LoopRotate), so the crash is reachable from purely upstream passes on this IR.

## Cross-target check

| Variant | Result | Note |
|---|---|---|
| Original (Z80 datalayout) | CRASH | as above |
| Same IR, x86_64 datalayout (`-m:o-i64:64-i128:128-f80:128-n8:16:32:64-S128`) | OK | proves Z80 16-bit-pointer layout is the trigger |
| Two unrelated arrays | CRASH | issue's open bisection bullet — confirmed |
| Two empty loops (volatile counter, no array) | OK | needs GEP / pointer arithmetic in body |
| One loop only | OK | needs two loops |
| `unsigned int` IV | CRASH | not IV-width-specific |
| `unsigned char` IV | TIMEOUT (20s) | likely same shape, longer build before overflow |
| `N=4096` instead of 100 | TIMEOUT | same |
| `N=2` | TIMEOUT | same |
| `N` non-constant (loaded from global) | OK | needs constant trip count |

So the trigger needs: (a) Z80 16-bit-pointer datalayout, (b) two loops in sequence, (c) constant trip count, (d) GEP/pointer-arithmetic in body.

## Root cause: LoopRotate corrupts SSA

`opt -passes='default<O1>' -print-after=loop-rotate -S` shows the corrupted IR coming OUT of `LoopRotatePass`:

```llvm
.preheader:                                       ; preds = %0, %.preheader
  %1 = zext nneg i8 %z80-indexiv.iv.next10 to i16
  %uglygep11 = getelementptr i8, ptr @a, i16 %1
  %2 = load i8, ptr %uglygep11, align 1, !tbaa !6
  %3 = add i8 %2, 1
  store i8 %3, ptr %uglygep11, align 1, !tbaa !6
  %4 = add nuw nsw i16 %4, 1                      ; <-- SELF-USE, no phi
  %exitcond8.not = icmp eq i16 %4, 100
  %z80-indexiv.iv.next10 = add nuw nsw i8 %z80-indexiv.iv.next10, 1  ; <-- SELF-USE
  br i1 %exitcond8.not, label %5, label %.preheader, !llvm.loop !7
```

The block `.preheader` has two predecessors (entry + own backedge) but no phi nodes. Two instructions are self-referential:
- `%4 = add nuw nsw i16 %4, 1`
- `%z80-indexiv.iv.next10 = add nuw nsw i8 %z80-indexiv.iv.next10, 1`

Pre-rotation (from `-print-before=loop-deletion` on the same loop), both values were the *phi outputs* of a properly formed rotated-into-canonical loop:

```llvm
.preheader:                                       ; preds = %.preheader.preheader, %.preheader
  %z80-indexiv.iv9 = phi i8 [ 0, %.preheader.preheader ], [ %z80-indexiv.iv.next10, %.preheader ]
  %.17 = phi i16 [ %7, %.preheader ], [ 0, %.preheader.preheader ]
  ...
  %7 = add nuw nsw i16 %.17, 1
  ...
  %z80-indexiv.iv.next10 = add nuw nsw i8 %z80-indexiv.iv9, 1
```

After LoopRotate's second invocation on the same loop, both phis are gone and the consumers point at themselves. The pass-state hypothesis: LoopRotate replaced the phi-result with the body-defined incoming value but kept the body-defined add un-renamed, leaving the add as its own operand.

## Why SCEV is the visible victim

`ScalarEvolution::createSCEVIter` (`llvm/lib/Analysis/ScalarEvolution.cpp:7692`) is a worklist replacement for the older recursive `createSCEV`. The worklist (`SmallVector<PointerIntPair<Value*,1,bool>>`) has no "currently-in-progress" cycle detection — it only checks `getExistingSCEV(CurV)`, which returns null until the SCEV is fully inserted. For valid SSA this is fine because the operand DAG is acyclic outside phi nodes. For the corrupt SSA emitted by LoopRotate above, `%4`'s only operand is `%4`, so the worklist re-pushes `%4` every iteration:

```
Pop (%4,false) -> Push (%4,true), (%4,false)
Pop (%4,false) -> Push (%4,true), (%4,false)
...
```

Net +1 entry per iteration until 2^31 -> SmallVector grow request 2^32 -> overflow -> `report_at_maximum_capacity`.

## Patch direction(s)

Two candidate fixes, of escalating ambition:

**1. Defensive: add cycle detection to `createSCEVIter`.**
A `SmallPtrSet<Value*, 16> InProgress` populated when pushing `(V, true)` and consulted when pushing operands would turn the infinite loop into a graceful `getUnknown(V)` return. This is a defense-in-depth fix that makes SCEV survive malformed IR but does NOT address the underlying corruption. Useful as a hardening patch independent of (2).

**2. Real fix: LoopRotate must not emit self-referential adds.**
Investigation needed: which path in `LoopRotation.cpp` strips the phi but leaves the body-defined consumer pointing at its old phi-result name? Suspect `RewriteLoopBodyWithConditionConstant` or the post-rotate cleanup that replaces phi-result uses with the matching predecessor incoming value — likely picks the wrong one when the loop already has its IV materialized in both i8 and i16 forms (the second IV `%z80-indexiv.iv` is the Z80-fork's narrowed-IV from `Z80NarrowIV`, see #77 / session 73n).

Hypothesis: `Z80NarrowIV` (commit `bbcc6f6047c3`) creates the dual-IV loop shape (`%z80-indexiv.iv` i8 + `%.17` i16). When LoopRotate rewrites the loop and inserts a memcpy in the preheader (via Z80LoopIdiomFill, no — but `default<O1>` reproduces, so this is upstream's own loop-idiom-recognize-equivalent), it strips the i16 phi but doesn't re-thread the i16 consumer.

**Probe to confirm hypothesis (next session):** disable `Z80NarrowIV` via `-mllvm -disable-z80-narrow-iv` (or whichever flag was added) and re-run the repro. If crash goes away, the LoopRotate bug is conditional on the dual-IV shape created by `Z80NarrowIV` and the fix can live in `Z80NarrowIV` (avoid the shape) OR upstream LoopRotate (handle the shape). Even if upstream LoopRotate is the proper fix, the Z80-side mitigation is shippable in a single session.

## Disconnect from issue body

The issue's bisection table claimed "two unrelated arrays" was untested — confirmed CRASH. The "two loops on same array" framing was wrong; the trigger is "two loops, either array, with constant trip count and GEP body, under Z80 datalayout."

The issue's stack-trace attribution to SCEV was correct in mechanism but misleading in root cause. Update the issue body to reflect that LoopRotate is the producer of the bad IR.

## Next-session work units

1. Confirm `Z80NarrowIV` involvement (10 min) — disable + retry.
2. Bisect which upstream pass converts pre-rotate IR to post-rotate IR with the self-use (5 min).
3. If LoopRotate: read `llvm/lib/Transforms/Utils/LoopRotationUtils.cpp` for the phi-strip codepath (20 min).
4. Decide: file upstream-LLVM bug + propose patch, OR work around in `Z80NarrowIV` (avoid the dual-IV shape on this input).
5. Independent: SCEV hardening patch (cycle detect in `createSCEVIter`) — small, low risk, fileable to upstream-LLVM whether or not LoopRotate is fixed.

## Drill cost vs estimate

Execution plan estimate: "fastest Track A win because the repro already exists." Actual time ~45 min. Single biggest accelerant: opt with the user's `-O0 -emit-llvm` IR reproduces upstream-only, eliminating the need to grep target-callback registration.

The bigger payoff: **#182 turned out to NOT be a SCEV bug** — that's a meaningful re-classification for the coherence map. SCEV's iterative cycle-detection gap is a real but secondary issue.

## Files

- `/tmp/scev182/repro.c` — original 7-line repro.
- `/tmp/scev182/repro.in.ll` — clang-emitted IR (LLVM-version-free, no optnone) for direct opt reproduction.
- `/tmp/scev182/two_arrays.c` — confirms two-arrays variant also crashes (closes issue's open bisection).
- `/tmp/scev182/probe_*.c` — bisection probes for IV width / trip-count / N.
- `/tmp/scev182/probes.sh` — script driver.
