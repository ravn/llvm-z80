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
; CHECK-LABEL: _sge_zero:
; CHECK:       add a,a
; CHECK-NEXT:  jr {{n?}}c,
; CHECK-NOT:   sbc hl,
define dso_local i16 @sge_zero(i16 %x) {
  %c = icmp sgt i16 %x, -1
  br i1 %c, label %t, label %f
t:
  ret i16 100
f:
  ret i16 200
}

; x < 0  (sle x, -1 after canonicalisation): sign test via ADD A,A, no 16-bit SBC.
; CHECK-LABEL: _slt_zero:
; CHECK:       add a,a
; CHECK-NEXT:  jr {{n?}}c,
; CHECK-NOT:   sbc hl,
define dso_local i16 @slt_zero(i16 %x) {
  %c = icmp sle i16 %x, -1
  br i1 %c, label %t, label %f
t:
  ret i16 100
f:
  ret i16 200
}

; Full CRC-16 byte step: the natural `if (crc & 0x8000)` idiom.  The inner
; sign test must be ADD A,A (no 16-bit SBC), and the poly XOR uses immediates.
; CHECK-LABEL: _crc16_byte:
; CHECK:       add hl,hl
; CHECK-NOT:   sbc hl,
; CHECK:       add a,a
define dso_local i16 @crc16_byte(i16 %crc, i8 zeroext %b) {
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
