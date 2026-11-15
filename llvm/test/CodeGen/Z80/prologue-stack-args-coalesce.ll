; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs %s -o - | FileCheck %s

target triple = "z80"

; Test that consecutive stack argument loads reuse HL and step with INC HL
; instead of recomputing ADD HL, SP for every slot.
define zeroext i8 @test_consecutive_stack_args(ptr %p1, i16 %p2, i8 zeroext %c, ptr %flags) {
; CHECK-LABEL: _test_consecutive_stack_args:
; CHECK:       ld hl,2
; CHECK-NEXT:  add hl,sp
; CHECK-NEXT:  ld a,(hl)
; CHECK-NEXT:  inc hl
; CHECK-NEXT:  ld c,(hl)
; CHECK-NEXT:  inc hl
; CHECK-NEXT:  ld b,(hl)
entry:
  %f = load i8, ptr %flags, align 1
  %res = add i8 %c, %f
  ret i8 %res
}
