; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs %s -o - | FileCheck %s
;
; ravn/llvm-z80 #197: the carry/borrow-materialize idiom `SBC A,A` (A = -carry)
; reads A only to satisfy the encoding -- its result is independent of A's
; value, and A is overwritten.  When A is dead the read tripped
; -verify-machineinstrs "Using an undefined physical register".  The expansion
; now marks every emitted SBC A,A $a read undef.  A 32-bit subtract lowers
; through the 16-bit borrow-out path and emits SBC A,A with A dead; this
; verifies clean (llc aborts otherwise -- the assertion).  Metadata-only.

define dso_local i32 @s32(i32 %a, i32 %b) {
  %r = sub i32 %a, %b
  ret i32 %r
}

; CHECK-LABEL: _s32:
; CHECK: sbc a,a
