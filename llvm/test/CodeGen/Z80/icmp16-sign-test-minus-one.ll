; RUN: llc --mtriple=z80 -O2 < %s | FileCheck %s
;
; (No -g RUN line: this is an instruction-selection change that resolves
;  operands via MRI def-use lookup, not a peephole walking adjacent MIs, so it
;  is unaffected by DBG_VALUE pseudos.)
;
; A signed 16-bit compare against -1 (`x sgt -1` ⇔ `x sge 0` ⇔ sign bit clear,
; and `x sle -1` ⇔ `x slt 0` ⇔ sign bit set) is a pure sign-bit test, identical
; to a compare against 0.  The 16-bit signed ISel used to handle only the
; compare-against-0 form and fell through to a full `LD HL,0xFFFF; SBC HL,rr`
; 16-bit subtraction for the -1 form.  The middle-end canonicalises the natural
; `if (x & 0x8000)` / `x >= 0` idiom (e.g. the CRC-16 inner loop) to `sgt x,-1`,
; so this hit real code.  After the fix it must emit the one-instruction sign
; test (COPY hi; ADD A,A; JR C/NC) and NOT a 16-bit compare.

; x >= 0  (sgt x, -1 after canonicalisation): sign test via ADD A,A, no 16-bit SBC.
; (Branch sense c/nc is a block-layout choice; the invariant is "sign test, not
;  a full LD HL,0xFFFF; SBC HL,rr compare".)
define dso_local i16 @sge_zero(i16 %x) {
  %c = icmp sgt i16 %x, -1
  br i1 %c, label %t, label %f
t:
; CHECK-LABEL: sge_zero:
; CHECK:      	ld	a,h
; CHECK:      	add	a,a
; CHECK:      	jr	c,.LBB0_2
; CHECK:      	ld	de,100
; CHECK:      	ret
; CHECK:      .LBB0_2:
; CHECK:      	ld	de,200
; CHECK:      	ret
  ret i16 100
f:
  ret i16 200
}

; x < 0  (sle x, -1 after canonicalisation): sign test via ADD A,A, no 16-bit SBC.
define dso_local i16 @slt_zero(i16 %x) {
  %c = icmp sle i16 %x, -1
  br i1 %c, label %t, label %f
t:
; CHECK-LABEL: slt_zero:
; CHECK:      	ld	a,h
; CHECK:      	add	a,a
; CHECK:      	jr	c,.LBB1_2
; CHECK:      	ld	de,200
; CHECK:      	ret
; CHECK:      .LBB1_2:
; CHECK:      	ld	de,100
; CHECK:      	ret
  ret i16 100
f:
  ret i16 200
}

; Real-world witness: CRC-16 loop from rcbios/cpnos.
; The inner loop has `if (c & 0x8000)` which clang canonicalises to
; `icmp sgt i16 %c, -1`.  This must NOT emit a 16-bit compare inside the loop.
define dso_local i16 @crc16_byte(i16 %crc, i8 %b) {
  %b16  = zext i8 %b to i16
  %init = xor i16 %crc, %b16
  br label %loop
loop:
  %c  = phi i16 [ %init, %0 ], [ %next, %cont ]
  %j  = phi i8  [ 0,     %0 ], [ %j1,   %cont ]
  %neg = icmp sgt i16 %c, -1          ; sign clear ⇔ (c & 0x8000)==0
  %sh  = shl i16 %c, 1
  br i1 %neg, label %noxor, label %doxor
doxor:
  %x = xor i16 %sh, 4129
; CHECK-LABEL: _crc16_byte:
; CHECK:      	ex	de,hl
; CHECK:      	ld	hl,2
; CHECK:      	add	hl,sp
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	a,e
; CHECK:      	xor	b
; CHECK:      	ld	e,a
; CHECK:      	ld	b,8
; CHECK:      	jr	.LBB2_4
; CHECK:      .LBB2_1:
; CHECK:      	ld	a,l
; CHECK:      	xor	33
; CHECK:      	ld	e,a
; CHECK:      	ld	a,h
; CHECK:      	xor	16
; CHECK:      	ld	d,a
; CHECK:      .LBB2_2:
; CHECK:      	djnz	.LBB2_4
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
; CHECK:      .LBB2_4:
; CHECK:      	ld	l,e
; CHECK:      	ld	h,d
; CHECK:      	add	hl,hl
; CHECK:      	ld	a,d
; CHECK:      	add	a,a
; CHECK:      	jr	c,.LBB2_1
; CHECK:      	ex	de,hl
; CHECK:      	jr	.LBB2_2
  br label %cont
noxor:
  br label %cont
cont:
  %next = phi i16 [ %x, %doxor ], [ %sh, %noxor ]
  %j1   = add nuw nsw i8 %j, 1
  %done = icmp eq i8 %j1, 8
  br i1 %done, label %exit, label %loop
exit:
  ret i16 %next
}
