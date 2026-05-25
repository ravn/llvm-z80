; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -z80-unreserve-iy < %s | FileCheck %s

; ravn/llvm-z80#112 / #14: a loop-carried i32 value whose high half is allocated
; to IY must keep its per-iteration update.  The Z80LateOptimization "IX/IY
; transfer" peephole used to collapse the COPY16_PUSHPOP round-trip that writes
; the new high word back into IY (treating IY as dead scratch), silently
; dropping the loop-carried update so the loop hung / returned garbage.
;
; The fix added a liveness guard: the peephole only fires when IX/IY is dead
; after the closing copy.  Here IY is live-out via the back-edge, so the update
; (a `pop iy` inside the loop body) must survive.

; popcount via shift loop: while (v) { count += v & 1; v >>= 1; }
define dso_local zeroext i8 @popcount32(i32 noundef %0) {
  %2 = icmp eq i32 %0, 0
  br i1 %2, label %11, label %3

3:
  %4 = phi i8 [ %8, %3 ], [ 0, %1 ]
  %5 = phi i32 [ %9, %3 ], [ %0, %1 ]
  %6 = trunc i32 %5 to i8
  %7 = and i8 %6, 1
  %8 = add i8 %7, %4
  %9 = lshr i32 %5, 1
  %10 = icmp eq i32 %9, 0
  br i1 %10, label %11, label %3

11:
  %12 = phi i8 [ 0, %1 ], [ %8, %3 ]
  ret i8 %12
}

; The loop-carried high word lives in IY and must be rewritten each iteration.
; CHECK-LABEL: .LBB0_2:
; CHECK: pop iy
; CHECK: .LBB0_2
