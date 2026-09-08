; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; Test: signed less-than (SLT)
; CHECK-LABEL: icmp_slt16:
; CHECK:      	dec	sp
; CHECK:      	ld	b,h
; CHECK:      	ld	a,h
; CHECK:      	xor	d
; CHECK:      	rlca
; CHECK:      	sbc	a,a
; CHECK:      	push	hl
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),a
; CHECK:      	pop	hl
; CHECK:      	and	a
; CHECK:      	sbc	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	c,a
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	a,d
; CHECK:      	cpl
; CHECK:      	and	c
; CHECK:      	ld	c,a
; CHECK:      	ld	a,b
; CHECK:      	rlca
; CHECK:      	and	#1
; CHECK:      	and	d
; CHECK:      	or	c
; CHECK:      	inc	sp
; CHECK:      	ret
define i8 @icmp_slt16(i16 %a, i16 %b) {
  %c = icmp slt i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test: signed greater-or-equal (SGE) - same as SLT but with xor #1
define i8 @icmp_sge16(i16 %a, i16 %b) {
  %c = icmp sge i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}
; CHECK-LABEL: icmp_sge16:
; CHECK:      	dec	sp
; CHECK:      	ld	b,h
; CHECK:      	ld	a,h
; CHECK:      	xor	d
; CHECK:      	rlca
; CHECK:      	sbc	a,a
; CHECK:      	push	hl
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),a
; CHECK:      	pop	hl
; CHECK:      	and	a
; CHECK:      	sbc	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	c,a
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	a,d
; CHECK:      	cpl
; CHECK:      	and	c
; CHECK:      	ld	c,a
; CHECK:      	ld	a,b
; CHECK:      	rlca
; CHECK:      	and	#1
; CHECK:      	and	d
; CHECK:      	or	c
; CHECK:      	xor	#1
; CHECK:      	inc	sp
; CHECK:      	ret

; Test: signed less-or-equal (SLE) - swapped operands
define i8 @icmp_sle16(i16 %a, i16 %b) {
  %c = icmp sle i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test: signed greater-than (SGT) - swapped operands
define i8 @icmp_sgt16(i16 %a, i16 %b) {
  %c = icmp sgt i16 %a, %b
  %r = zext i1 %c to i8
; CHECK-LABEL: icmp_sle16:
; CHECK:      	dec	sp
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	a,d
; CHECK:      	xor	h
; CHECK:      	rlca
; CHECK:      	sbc	a,a
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),a
; CHECK:      	ld	l,e
; CHECK:      	ld	h,d
; CHECK:      	and	a
; CHECK:      	sbc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	c,a
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	a,b
; CHECK:      	cpl
; CHECK:      	and	c
; CHECK:      	ld	c,a
; CHECK:      	ld	a,d
; CHECK:      	rlca
; CHECK:      	and	#1
; CHECK:      	and	b
; CHECK:      	or	c
; CHECK:      	xor	#1
; CHECK:      	inc	sp
; CHECK:      	ret
  ret i8 %r
}

; Test: signed greater-than zero (SGT X, 0) - non-negative AND non-zero
define i8 @icmp_sgt_zero(i16 %a) {
  %c = icmp sgt i16 %a, 0
  %r = zext i1 %c to i8
  ret i8 %r
}

; CHECK-LABEL: icmp_sgt16:
; CHECK:      	dec	sp
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	a,d
; CHECK:      	xor	h
; CHECK:      	rlca
; CHECK:      	sbc	a,a
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),a
; CHECK:      	ld	l,e
; CHECK:      	ld	h,d
; CHECK:      	and	a
; CHECK:      	sbc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	c,a
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	a,b
; CHECK:      	cpl
; CHECK:      	and	c
; CHECK:      	ld	c,a
; CHECK:      	ld	a,d
; CHECK:      	rlca
; CHECK:      	and	#1
; CHECK:      	and	b
; CHECK:      	or	c
; CHECK:      	inc	sp
; CHECK:      	ret
; Test: signed less-or-equal zero (SLE X, 0) - inverted SGT X, 0
define i8 @icmp_sle_zero(i16 %a) {
  %c = icmp sle i16 %a, 0
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test: unsigned less-or-equal (ULE) - swapped operands, 8-bit SUB/SBC chain
define i8 @icmp_ule16(i16 %a, i16 %b) {
  %c = icmp ule i16 %a, %b
; CHECK-LABEL: icmp_sgt_zero:
; CHECK:      	ex	de,hl
; CHECK:      	ld	bc,#0
; CHECK:      	ld	a,d
; CHECK:      	rlca
; CHECK:      	sbc	a,a
; CHECK:      	ld	b,a
; CHECK:      	ld	hl,#0
; CHECK:      	and	a
; CHECK:      	sbc	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	c,a
; CHECK:      	ld	a,b
; CHECK:      	cpl
; CHECK:      	and	c
; CHECK:      	ld	c,a
; CHECK:      	ld	de,#0
; CHECK:      	ld	a,d
; CHECK:      	rlca
; CHECK:      	and	#1
; CHECK:      	and	b
; CHECK:      	or	c
; CHECK:      	ret
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test: unsigned greater-than (UGT) - swapped operands, 8-bit SUB/SBC chain
define i8 @icmp_ugt16(i16 %a, i16 %b) {
  %c = icmp ugt i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}
; CHECK-LABEL: icmp_sle_zero:
; CHECK:      	ex	de,hl
; CHECK:      	ld	bc,#0
; CHECK:      	ld	a,d
; CHECK:      	rlca
; CHECK:      	sbc	a,a
; CHECK:      	ld	b,a
; CHECK:      	ld	hl,#0
; CHECK:      	and	a
; CHECK:      	sbc	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	c,a
; CHECK:      	ld	a,b
; CHECK:      	cpl
; CHECK:      	and	c
; CHECK:      	ld	c,a
; CHECK:      	ld	de,#0
; CHECK:      	ld	a,d
; CHECK:      	rlca
; CHECK:      	and	#1
; CHECK:      	and	b
; CHECK:      	or	c
; CHECK:      	xor	#1
; CHECK:      	ret
; CHECK-LABEL: icmp_ule16:
; CHECK:      	ld	a,e
; CHECK:      	sub	l
; CHECK:      	ld	a,d
; CHECK:      	sbc	a,h
; CHECK:      	ccf
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ret
; CHECK-LABEL: icmp_ugt16:
; CHECK:      	ld	a,e
; CHECK:      	sub	l
; CHECK:      	ld	a,d
; CHECK:      	sbc	a,h
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ret
