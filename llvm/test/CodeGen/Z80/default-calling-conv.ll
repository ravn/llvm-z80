; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s
;
; The Clang frontend option selects the existing SDCCCall0 backend convention.
; Keep both call sequences pinned here: cc 128 is stack-based, while the
; target's default C convention passes the first two i16 values in registers.

declare cc 128 i16 @stack_sum(i16, i16)
declare i16 @register_sum(i16, i16)

; CHECK-LABEL: _call_stack_sum:
; CHECK:       ld hl,#22136
; CHECK-NEXT:  push hl
; CHECK:       ld hl,#4660
; CHECK-NEXT:  push hl
; CHECK:       call _stack_sum
; CHECK-NEXT:  ex de,hl
; CHECK-NEXT:  pop af
; CHECK-NEXT:  pop af
; CHECK:       ret
define i16 @call_stack_sum() {
  %sum = call cc 128 i16 @stack_sum(i16 4660, i16 22136)
  ret i16 %sum
}

; CHECK-LABEL: _call_register_sum:
; CHECK-NOT:   push
; CHECK:       call _register_sum
; CHECK-NOT:   inc sp
; CHECK:       ret
define i16 @call_register_sum() {
  %sum = call i16 @register_sum(i16 4660, i16 22136)
  ret i16 %sum
}
