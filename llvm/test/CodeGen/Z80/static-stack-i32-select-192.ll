; RUN: llc -mtriple=z80 -mattr=+static-frame -O1 < %s | FileCheck %s

; ravn/llvm-z80#192: an i32 `select((crc&1)==0, 0, CONST)` reduction loop
; miscompiled under +static-frame at -O1/-Os.  The i32 `icmp eq` is selected as
; two XOR_CMP_EQ16 (one per 16-bit half) AND-combined.  The #173 peephole
; ("bare BSS store + 4-instr A-preserving reload -> LD r,A; PUSH/POP rr")
; relocated the first half's result (flag1) into register D and bracketed it
; with PUSH/POP DE -- but D is the SECOND half-compare's zero input, which it
; reads.  flag1 thus clobbered D, the second compare computed NOT(flag1), and
; the i32-== AND collapsed to `NOT(flag1) AND flag1` = 0, so the select always
; took the CONST branch (crc_one(0xFF) returned 0xB6662D3D, not 0x2D02EF8D).
;
; Fix: #173 bails when its destination register is READ in the interval.  The
; 4-instr A-preserving reload (PUSH AF; LD A,(slot); LD r,A; POP AF) must then
; survive instead of being folded into a PUSH/POP DE that clobbers D.

; CHECK-LABEL: crc_one:
; CHECK:      	ld	b,8
; CHECK:      	jr	.LBB0_2
; CHECK:      	ld	a,l
; CHECK:      	xor	d
; CHECK:      	ld	e,a
; CHECK:      	ld	a,h
; CHECK:      	ld	hl,L_crc_one.frame+3
; CHECK:      	ld	d,(hl)
; CHECK:      	xor	d
; CHECK:      	ld	d,a
; CHECK:      	ld	a,c
; CHECK:      	ld	hl,(L_crc_one.frame)
; CHECK:      	xor	l
; CHECK:      	ld	l,a
; CHECK:      	ld	a,b
; CHECK:      	xor	h
; CHECK:      	ld	h,a
; CHECK:      	ld	a,(L_crc_one.frame+2)
; CHECK:      	dec	a
; CHECK:      	ld	b,a
; CHECK:      	jr	z,.LBB0_4
; CHECK:      	ld	(L_crc_one.frame),hl
; CHECK:      	ld	a,b
; CHECK:      	ld	(L_crc_one.frame+2),a
; CHECK:      	ld	c,e
; CHECK:      	ld	b,d
; CHECK:      	srl	b
; CHECK:      	rr	c
; CHECK:      	ld	a,l
; CHECK:      	rrca
; CHECK:      	and	128
; CHECK:      	ld	h,a
; CHECK:      	ld	a,c
; CHECK:      	ld	d,a
; CHECK:      	ld	a,b
; CHECK:      	or	h
; CHECK:      	ld	(L_crc_one.frame+3),a
; CHECK:      	ld	bc,(L_crc_one.frame)
; CHECK:      	srl	b
; CHECK:      	rr	c
; CHECK:      	ld	(L_crc_one.frame),bc
; CHECK:      	ld	a,e
; CHECK:      	and	1
; CHECK:      	ld	c,a
; CHECK:      	ld	b,0
; CHECK:      	ld	e,0
; CHECK:      	ld	l,e
; CHECK:      	ld	h,0
; CHECK:      	ld	(L_crc_one.frame+4),hl
; CHECK:      	ld	hl,0
; CHECK:      	ld	a,c
; CHECK:      	or	b
; CHECK:      	ld	e,a
; CHECK:      	ld	bc,(L_crc_one.frame+4)
; CHECK:      	ld	a,c
; CHECK:      	or	b
; CHECK:      	or	e
; CHECK:      	ld	bc,0
; CHECK:      	jr	z,.LBB0_1
; CHECK:      	ld	bc,60856
; CHECK:      	ld	hl,33568
; CHECK:      	jr	.LBB0_1
; CHECK:      	ret
define dso_local i32 @crc_one(i32 noundef %0) {
  br label %3

2:
  ret i32 %10

3:
  %4 = phi i8 [ 0, %1 ], [ %11, %3 ]
  %5 = phi i32 [ %0, %1 ], [ %10, %3 ]
  %6 = lshr i32 %5, 1
  %7 = and i32 %5, 1
  %8 = icmp eq i32 %7, 0
  %9 = select i1 %8, i32 0, i32 -306674912
  %10 = xor i32 %9, %6
  %11 = add nuw nsw i8 %4, 1
  %12 = icmp eq i8 %11, 8
  br i1 %12, label %2, label %3
}

; The A-preserving 4-instr reload of the first compare's result must survive
; (#173 must NOT fold it into a D-clobbering PUSH/POP DE).
