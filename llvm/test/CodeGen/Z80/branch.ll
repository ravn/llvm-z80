; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; Test conditional branch based on 16-bit equality comparison
; CMP+BR fusion: G_ICMP EQ + G_BRCOND → XOR-based compare + JR Z
; CHECK-LABEL: branch_eq:
; CHECK:      	ld	a,h
; CHECK:      	xor	d
; CHECK:      	ld	b,a
; CHECK:      	ld	a,l
; CHECK:      	xor	e
; CHECK:      	or	b
; CHECK:      	jp	z,.LBB0_1
; CHECK:      	jp	.LBB0_2
; CHECK:      	ex	de,hl
; CHECK:      	ret
; CHECK:      	ret
define i16 @branch_eq(i16 %a, i16 %b) {
  %cond = icmp eq i16 %a, %b
  br i1 %cond, label %then, label %else
then:
  ret i16 %a
else:
  ret i16 %b
}

; Test 8-bit equality comparison
define i8 @icmp_eq8(i8 %a, i8 %b) {
  %c = icmp eq i8 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
; CHECK-LABEL: icmp_eq8:
; CHECK:      	sub	l
; CHECK:      	sub	#1
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ret
}

; Test 16-bit unsigned less-than (8-bit SUB/SBC chain)
define i8 @icmp_ult16(i16 %a, i16 %b) {
  %c = icmp ult i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}

; CHECK-LABEL: icmp_ult16:
; CHECK:      	ld	a,l
; CHECK:      	sub	e
; CHECK:      	ld	a,h
; CHECK:      	sbc	a,d
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ret
; Test conditional branch for SGT X, 0 (fused: non-negative AND non-zero)
define i16 @branch_sgt_zero(i16 %a) {
  %cond = icmp sgt i16 %a, 0
  br i1 %cond, label %then, label %else
then:
  ret i16 42
else:
  ret i16 0
}

; Test conditional branch for SLE X, 0 (fused: inverted SGT zero)
define i16 @branch_sle_zero(i16 %a) {
; CHECK-LABEL: branch_sgt_zero:
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
; CHECK:      	jp	nz,.LBB3_1
; CHECK:      	jp	.LBB3_2
; CHECK:      	ld	de,#42
; CHECK:      	ret
; CHECK:      	ld	de,#0
; CHECK:      	ret
  %cond = icmp sle i16 %a, 0
  br i1 %cond, label %then, label %else
then:
  ret i16 42
else:
  ret i16 0
}
; CHECK-LABEL: branch_sle_zero:
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
; CHECK:      	jp	nz,.LBB4_1
; CHECK:      	jp	.LBB4_2
; CHECK:      	ld	de,#42
; CHECK:      	ret
; CHECK:      	ld	de,#0
; CHECK:      	ret
