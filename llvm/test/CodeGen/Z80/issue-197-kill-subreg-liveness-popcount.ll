; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs < %s | FileCheck %s
;
; ravn/llvm-z80 #197: in a 16-bit shift loop, the rewriter emits
;   dead $hl = KILL $hl, implicit-def $l
; ahead of the low-byte read (`val & 1`), reviving only $l -- but the same
; block later does `$hl = LSHR16 $hl` (the `val >>= 1`), which needs $h.  The
; dead $hl def kills $h, so LSHR16 -> SRL_H read an undefined $h and
; -verify-machineinstrs aborted ("Using an undefined physical register $h").
; The value is correct (the register is not actually cleared); only the
; sub-register liveness metadata was wrong.  Z80FixupImplicitDefs now revives
; a KILL-pair sibling sub-register that is read downstream in the block.
;
; CHECK-LABEL: popcount16:
; CHECK: srl h
; CHECK: rr l

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

define zeroext i8 @popcount16(i16 zeroext %0) {
  %2 = icmp eq i16 %0, 0
  br i1 %2, label %11, label %3

3:
  %4 = phi i8 [ %8, %3 ], [ 0, %1 ]
  %5 = phi i16 [ %9, %3 ], [ %0, %1 ]
  %6 = trunc i16 %5 to i8
  %7 = and i8 %6, 1
  %8 = add i8 %7, %4
  %9 = lshr i16 %5, 1
  %10 = icmp eq i16 %9, 0
  br i1 %10, label %11, label %3

11:
  %12 = phi i8 [ 0, %1 ], [ %8, %3 ]
  ret i8 %12
}
