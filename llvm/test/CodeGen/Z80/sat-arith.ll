; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

declare i16 @llvm.uadd.sat.i16(i16, i16)
declare i16 @llvm.usub.sat.i16(i16, i16)
declare i16 @llvm.sadd.sat.i16(i16, i16)
declare i16 @llvm.ssub.sat.i16(i16, i16)
declare i8 @llvm.scmp.i8.i16(i16, i16)
declare i8 @llvm.ucmp.i8.i16(i16, i16)

; Test: unsigned add saturating
; CHECK-LABEL: test_uaddsat:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ex	de,hl
; CHECK:      	add	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	de,#65535
; CHECK:      	jr	nz,.LBB0_2
; CHECK:      	ex	de,hl
; CHECK:      	ret
define i16 @test_uaddsat(i16 %a, i16 %b) {
  %r = call i16 @llvm.uadd.sat.i16(i16 %a, i16 %b)
  ret i16 %r
}

; Test: unsigned sub saturating
define i16 @test_usubsat(i16 %a, i16 %b) {
  %r = call i16 @llvm.usub.sat.i16(i16 %a, i16 %b)
  ret i16 %r
}

; CHECK-LABEL: test_usubsat:
; CHECK:      	and	a
; CHECK:      	sbc	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	de,#0
; CHECK:      	jr	nz,.LBB1_2
; CHECK:      	ex	de,hl
; CHECK:      	ret
; Test: signed add saturating (uses P/V flag capture via CAPTURE_PV pseudo)
define i16 @test_saddsat(i16 %a, i16 %b) {
  %r = call i16 @llvm.sadd.sat.i16(i16 %a, i16 %b)
  ret i16 %r
}

; Test: signed sub saturating (uses P/V flag capture via CAPTURE_PV pseudo)
define i16 @test_ssubsat(i16 %a, i16 %b) {
  %r = call i16 @llvm.ssub.sat.i16(i16 %a, i16 %b)
  ret i16 %r
}
; CHECK-LABEL: test_saddsat:
; CHECK:      	and	a
; CHECK:      	adc	hl,de
; CHECK:      	ex	de,hl
; CHECK:      	push	af
; CHECK:      	pop	hl
; CHECK:      	ld	a,l
; CHECK:      	rrca
; CHECK:      	rrca
; CHECK:      	and	#1
; CHECK:      	ld	b,a
; CHECK:      	ld	(L_test_saddsat.frame),de
; CHECK:      	ld	a,d
; CHECK:      	add	a,a
; CHECK:      	sbc	a,a
; CHECK:      	ld	l,a
; CHECK:      	ld	h,a
; CHECK:      	ld	de,#32768
; CHECK:      	add	hl,de
; CHECK:      	ex	de,hl
; CHECK:      	ld	a,b
; CHECK:      	or	a
; CHECK:      	jr	nz,.LBB2_2
; CHECK:      	ld	de,(L_test_saddsat.frame)
; CHECK:      	ret

; Test: three-way signed comparison
define i8 @test_scmp(i16 %a, i16 %b) {
  %r = call i8 @llvm.scmp.i8.i16(i16 %a, i16 %b)
  ret i8 %r
}

; Test: three-way unsigned comparison
define i8 @test_ucmp(i16 %a, i16 %b) {
  %r = call i8 @llvm.ucmp.i8.i16(i16 %a, i16 %b)
  ret i8 %r
}
; CHECK-LABEL: test_ssubsat:
; CHECK:      	and	a
; CHECK:      	sbc	hl,de
; CHECK:      	ex	de,hl
; CHECK:      	push	af
; CHECK:      	pop	hl
; CHECK:      	ld	a,l
; CHECK:      	rrca
; CHECK:      	rrca
; CHECK:      	and	#1
; CHECK:      	ld	b,a
; CHECK:      	ld	(L_test_ssubsat.frame),de
; CHECK:      	ld	a,d
; CHECK:      	add	a,a
; CHECK:      	sbc	a,a
; CHECK:      	ld	l,a
; CHECK:      	ld	h,a
; CHECK:      	ld	de,#32768
; CHECK:      	add	hl,de
; CHECK:      	ex	de,hl
; CHECK:      	ld	a,b
; CHECK:      	or	a
; CHECK:      	jr	nz,.LBB3_2
; CHECK:      	ld	de,(L_test_ssubsat.frame)
; CHECK:      	ret
; CHECK-LABEL: test_scmp:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	a,d
; CHECK:      	xor	h
; CHECK:      	rlca
; CHECK:      	sbc	a,a
; CHECK:      	ld	(L_test_scmp.frame),a
; CHECK:      	ld	l,e
; CHECK:      	ld	h,d
; CHECK:      	and	a
; CHECK:      	sbc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	h,a
; CHECK:      	ld	a,(L_test_scmp.frame)
; CHECK:      	ld	l,a
; CHECK:      	cpl
; CHECK:      	and	h
; CHECK:      	ld	h,a
; CHECK:      	ld	a,d
; CHECK:      	rlca
; CHECK:      	and	#1
; CHECK:      	and	l
; CHECK:      	or	h
; CHECK:      	ld	(L_test_scmp.frame),a
; CHECK:      	ld	a,b
; CHECK:      	xor	d
; CHECK:      	rlca
; CHECK:      	sbc	a,a
; CHECK:      	ld	(L_test_scmp.frame+1),a
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	and	a
; CHECK:      	sbc	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	d,a
; CHECK:      	ld	a,(L_test_scmp.frame+1)
; CHECK:      	ld	c,a
; CHECK:      	cpl
; CHECK:      	and	d
; CHECK:      	ld	d,a
; CHECK:      	ld	a,b
; CHECK:      	rlca
; CHECK:      	and	#1
; CHECK:      	and	c
; CHECK:      	or	d
; CHECK:      	ld	c,a
; CHECK:      	ld	a,(L_test_scmp.frame)
; CHECK:      	sub	c
; CHECK:      	ret
; CHECK-LABEL: test_ucmp:
; CHECK:      	ld	a,e
; CHECK:      	sub	l
; CHECK:      	ld	a,d
; CHECK:      	sbc	a,h
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	b,a
; CHECK:      	ld	a,l
; CHECK:      	sub	e
; CHECK:      	ld	a,h
; CHECK:      	sbc	a,d
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	c,a
; CHECK:      	ld	a,b
; CHECK:      	sub	c
; CHECK:      	ret
