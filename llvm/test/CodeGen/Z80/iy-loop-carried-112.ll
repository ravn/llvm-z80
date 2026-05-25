; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -z80-unreserve-iy < %s | FileCheck %s

; ravn/llvm-z80#112 / #14 / #189.  This function (a loop-carried i32 popcount)
; used to allocate the i32 high half to IY and update it each iteration via a
; `pop iy` inside the loop -- the #14 Z80LateOptimization peephole-liveness guard
; was added so that update would not be dropped.
;
; After the #189 / #112 work (getLargestLegalSuperClass no longer re-widens
; GR16NoIR to GR16 under -z80-unreserve-iy, plus the Z80NarrowNoIndex pre-RA
; pass), the byte-decomposed i32 halves are kept in GR16NoIR and no longer
; routed through IY at all -- so this function is now IY-free and the old
; `pop iy`-in-loop shuttle (and the undocumented `xor iyh` it had regressed into)
; are gone.  Lock that in.
;
; NOTE: the #14 peephole-liveness guard in Z80LateOptimization is still live code
; but is no longer exercised by this function; it needs a dedicated witness that
; still lands a loop-carried value in IY via push/pop.  Tracked in
; tasks/issue112-189-iy-leak-taxonomy-2026-05-25.md.

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

; The loop-carried i32 must no longer be shuttled through IX/IY.
; CHECK-LABEL: popcount32:
; CHECK-NOT: iy
