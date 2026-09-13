; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 < %s | FileCheck %s

; ravn/llvm-z80#248: matching i32 div/rem operations must share the one
; compiler-rt division pass that returns both results.

define i32 @udivmod32(i32 %a, i32 %b) {
; CHECK-LABEL: _udivmod32:
; CHECK:       call ___udivmodsi4
; CHECK-NOT:   ___udivsi3
; CHECK-NOT:   ___umodsi3
  %q = udiv i32 %a, %b
  %r = urem i32 %a, %b
  %s = add i32 %q, %r
  ret i32 %s
}

; Retaining only the low 16 bits of the remainder must not prevent fusion.
define i16 @udivmod32_narrow_rem(i32 %a, i32 %b, ptr %out) {
; CHECK-LABEL: _udivmod32_narrow_rem:
; CHECK:       call ___udivmodsi4
; CHECK-NOT:   ___udivsi3
; CHECK-NOT:   ___umodsi3
  %q = udiv i32 %a, %b
  %r = urem i32 %a, %b
  store i32 %q, ptr %out
  %rt = trunc i32 %r to i16
  ret i16 %rt
}

define i32 @sdivmod32(i32 %a, i32 %b) {
; CHECK-LABEL: _sdivmod32:
; CHECK:       call ___divmodsi4
; CHECK-NOT:   ___divsi3
; CHECK-NOT:   ___modsi3
  %q = sdiv i32 %a, %b
  %r = srem i32 %a, %b
  %s = add i32 %q, %r
  ret i32 %s
}
