; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Test: icmp eq/ne of zext i8 vs sext i8 should narrow to 8-bit CP,
; avoiding 16-bit expansion and register spilling.

define void @cmp_eq_zext_sext(i8 %a, i8 %b) {
; CHECK-LABEL: _cmp_eq_zext_sext:
; CHECK:       cp
; CHECK-NOT:   xor
; CHECK:       ret
entry:
  %a16 = zext i8 %a to i16
  %b16 = sext i8 %b to i16
  %cmp = icmp eq i16 %a16, %b16
  br i1 %cmp, label %if_true, label %if_false

if_true:
  tail call void @foo()
  ret void

if_false:
  ret void
}

declare void @foo()
