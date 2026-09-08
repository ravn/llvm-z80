; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

@global_var = global i16 0

; Test: load from global variable (direct addressing)
; CHECK-LABEL: load_global:
; CHECK:      	ld	hl,#_global_var
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ret
define i16 @load_global() {
  %v = load i16, ptr @global_var
  ret i16 %v
}

; Test: store to global variable (direct addressing)
define void @store_global(i16 %v) {
  store i16 %v, ptr @global_var
  ret void
}
; CHECK-LABEL: store_global:
; CHECK:      	ex	de,hl
; CHECK:      	ld	hl,#_global_var
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ret

; Test: zero-extend i8 to i16
define i16 @zext_i8_to_i16(i8 %a) {
  %w = zext i8 %a to i16
  ret i16 %w
}

; Test: truncate i16 to i8
define i8 @trunc_i16_to_i8(i16 %a) {
; CHECK-LABEL: zext_i8_to_i16:
; CHECK:      	ld	e,a
; CHECK:      	ld	d,#0
; CHECK:      	ret
  %t = trunc i16 %a to i8
  ret i8 %t
}

; Test: sign-extend i8 to i16
define i16 @sext_i8_to_i16(i8 %a) {
  %w = sext i8 %a to i16
  ret i16 %w
}

; CHECK-LABEL: trunc_i16_to_i8:
; CHECK:      	ld	a,l
; CHECK:      	ret
; Test: 16-bit multiply uses library call
define i16 @mul16(i16 %a, i16 %b) {
  %c = mul i16 %a, %b
  ret i16 %c
}
; CHECK-LABEL: sext_i8_to_i16:
; CHECK:      	ld	e,a
; CHECK:      	rlca
; CHECK:      	sbc	a,a
; CHECK:      	ld	d,a
; CHECK:      	ret
; CHECK-LABEL: mul16:
; CHECK:      	call	___mulhi3
; CHECK:      	ret
