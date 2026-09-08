; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; Test: signed 16-bit division via library call
; CHECK-LABEL: sdiv16:
; CHECK:      	call	___divhi3
; CHECK:      	ret
define i16 @sdiv16(i16 %a, i16 %b) {
  %r = sdiv i16 %a, %b
  ret i16 %r
}

; Test: unsigned 16-bit division via library call
define i16 @udiv16(i16 %a, i16 %b) {
  %r = udiv i16 %a, %b
; CHECK-LABEL: udiv16:
; CHECK:      	call	___udivhi3
; CHECK:      	ret
  ret i16 %r
}

; Test: signed 16-bit remainder via library call
define i16 @srem16(i16 %a, i16 %b) {
  %r = srem i16 %a, %b
  ret i16 %r
}
; CHECK-LABEL: srem16:
; CHECK:      	call	___modhi3
; CHECK:      	ret

; Test: unsigned 16-bit remainder via library call
define i16 @urem16(i16 %a, i16 %b) {
  %r = urem i16 %a, %b
  ret i16 %r
}
; CHECK-LABEL: urem16:
; CHECK:      	call	___umodhi3
; CHECK:      	ret
