; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; Narrowing: icmp eq/ne (add (zext i8), C), (zext i8) → 8-bit add+cp with carry guard
; Pattern from verify_seek_result: (drive_select + 0x20) != fdc_result.st0
@var1 = external global i8
@var2 = external global i8

; EQ branch: ADD A,C; JR C,fallthrough; CP r; JR Z,target
; CHECK-LABEL: narrow_add_cmp_eq:
; CHECK:      	ld	a,(_var1)
; CHECK:      	ld	l,a
; CHECK:      	ld	h,#0
; CHECK:      	ld	bc,#32
; CHECK:      	add	hl,bc
; CHECK:      	ld	a,(_var2)
; CHECK:      	ld	c,a
; CHECK:      	ld	b,#0
; CHECK:      	ld	a,h
; CHECK:      	xor	b
; CHECK:      	ld	b,a
; CHECK:      	ld	a,l
; CHECK:      	xor	c
; CHECK:      	or	b
; CHECK:      	jp	z,.LBB0_1
; CHECK:      	jp	.LBB0_2
; CHECK:      	ld	de,#1
; CHECK:      	ret
; CHECK:      	ld	de,#0
; CHECK:      	ret
define i16 @narrow_add_cmp_eq() {
entry:
  %a = load i8, ptr @var1
  %ext_a = zext i8 %a to i16
  %sum = add nuw nsw i16 %ext_a, 32
  %b = load i8, ptr @var2
  %ext_b = zext i8 %b to i16
  %cmp = icmp eq i16 %sum, %ext_b
  br i1 %cmp, label %then, label %else
then:
  ret i16 1
else:
  ret i16 0
}

; NE branch: ADD A,C; JR C,target; CP r; JR NZ,target
; CHECK-LABEL: narrow_add_cmp_ne:
; CHECK:      	ld	a,(_var1)
; CHECK:      	ld	l,a
; CHECK:      	ld	h,#0
; CHECK:      	ld	bc,#32
; CHECK:      	add	hl,bc
; CHECK:      	ld	a,(_var2)
; CHECK:      	ld	c,a
; CHECK:      	ld	b,#0
; CHECK:      	ld	a,h
; CHECK:      	xor	b
; CHECK:      	ld	b,a
; CHECK:      	ld	a,l
; CHECK:      	xor	c
; CHECK:      	or	b
; CHECK:      	jp	nz,.LBB1_1
; CHECK:      	jp	.LBB1_2
; CHECK:      	ld	de,#1
; CHECK:      	ret
; CHECK:      	ld	de,#0
; CHECK:      	ret
define i16 @narrow_add_cmp_ne() {
entry:
  %a = load i8, ptr @var1
  %ext_a = zext i8 %a to i16
  %sum = add nuw nsw i16 %ext_a, 32
  %b = load i8, ptr @var2
  %ext_b = zext i8 %b to i16
  %cmp = icmp ne i16 %sum, %ext_b
  br i1 %cmp, label %then, label %else
then:
  ret i16 1
else:
  ret i16 0
}

; Commuted: add on RHS instead of LHS
; CHECK-LABEL: narrow_add_cmp_commuted:
; CHECK:      	ld	a,(_var1)
; CHECK:      	ld	c,a
; CHECK:      	ld	b,#0
; CHECK:      	ld	a,(_var2)
; CHECK:      	ld	l,a
; CHECK:      	ld	h,#0
; CHECK:      	ld	de,#32
; CHECK:      	add	hl,de
; CHECK:      	ld	a,b
; CHECK:      	xor	h
; CHECK:      	ld	b,a
; CHECK:      	ld	a,c
; CHECK:      	xor	l
; CHECK:      	or	b
; CHECK:      	jp	nz,.LBB2_1
; CHECK:      	jp	.LBB2_2
; CHECK:      	ld	de,#1
; CHECK:      	ret
; CHECK:      	ld	de,#0
; CHECK:      	ret
define i16 @narrow_add_cmp_commuted() {
entry:
  %a = load i8, ptr @var1
  %ext_a = zext i8 %a to i16
  %b = load i8, ptr @var2
  %ext_b = zext i8 %b to i16
  %sum = add nuw nsw i16 %ext_b, 32
  %cmp = icmp ne i16 %ext_a, %sum
  br i1 %cmp, label %then, label %else
then:
  ret i16 1
else:
  ret i16 0
}
