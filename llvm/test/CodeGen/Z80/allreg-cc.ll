; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; Test: Z80_AllReg calling convention (cc 129) passes all args in registers.

; 1 x i16 arg → HL
; CHECK-LABEL: one_i16:
; CHECK:      	ret
define cc 129 void @one_i16(i16 %a) {
  ret void
}

; 2 x i16 args → HL, DE
define cc 129 i16 @add_two_i16(i16 %a, i16 %b) {
  %r = add i16 %a, %b
  ret i16 %r
; CHECK-LABEL: add_two_i16:
; CHECK:      	ld	hl,#4
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,#4
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	add	hl,bc
; CHECK:      	ret
}

; 3 x i16 args → HL, DE, BC
define cc 129 i16 @three_i16(i16 %a, i16 %b, i16 %c) {
  %ab = add i16 %a, %b
  %r = add i16 %ab, %c
  ret i16 %r
}
; CHECK-LABEL: three_i16:
; CHECK:      	ld	hl,#6
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,#6
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	hl,#4
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	add	hl,de
; CHECK:      	add	hl,bc
; CHECK:      	ret

; 1 x i8 arg → A
define cc 129 i8 @one_i8(i8 %a) {
  ret i8 %a
}

; i8 + i16 → A, HL
define cc 129 i16 @mixed_i8_i16(i8 %a, i16 %b) {
  ret i16 %b
}
; CHECK-LABEL: one_i8:
; No push hl needed: HL is dead across the reload, and the following
; ld l,a writes L (the pre-value of L is not needed). #210 sibling-half
; liveness fix.
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	a,(hl)
; CHECK:      	ld	l,a
; CHECK:      	ret

; 5 x i16 args → HL, DE, BC, IX, IY (all register pairs used)
define cc 129 i16 @five_i16(i16 %a, i16 %b, i16 %c, i16 %d, i16 %e) {
  %ab = add i16 %a, %b
  %abc = add i16 %ab, %c
  %abcd = add i16 %abc, %d
  %r = add i16 %abcd, %e
; CHECK-LABEL: mixed_i8_i16:
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	ret
  ret i16 %r
}
; CHECK-LABEL: five_i16:
; CHECK:      	ld	hl,#10
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,#10
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	hl,#8
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	pop	hl
; CHECK:      	add	hl,bc
; CHECK:      	add	hl,de
; CHECK:      	push	hl
; CHECK:      	ld	hl,#6
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	add	hl,bc
; CHECK:      	push	hl
; CHECK:      	ld	hl,#4
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	add	hl,bc
; CHECK:      	ret
