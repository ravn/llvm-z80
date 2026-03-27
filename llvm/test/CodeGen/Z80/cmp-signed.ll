; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; Test: signed less-than (SLT)
define i8 @icmp_slt16(i16 %a, i16 %b) {
; CHECK-LABEL: icmp_slt16:
; CHECK:       xor d
; CHECK:       rlca
; CHECK:       sbc hl,de
; CHECK:       sbc a,a
  %c = icmp slt i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test: signed greater-or-equal (SGE) - same as SLT but with xor #1
define i8 @icmp_sge16(i16 %a, i16 %b) {
; CHECK-LABEL: icmp_sge16:
; CHECK:       xor d
; CHECK:       rlca
; CHECK:       sbc hl,de
; CHECK:       xor #1
  %c = icmp sge i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test: signed less-or-equal (SLE) - swapped operands
define i8 @icmp_sle16(i16 %a, i16 %b) {
; CHECK-LABEL: icmp_sle16:
; CHECK:       sbc hl,bc
; CHECK:       xor #1
  %c = icmp sle i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test: signed greater-than (SGT) - swapped operands
define i8 @icmp_sgt16(i16 %a, i16 %b) {
; CHECK-LABEL: icmp_sgt16:
; CHECK:       sbc hl,bc
; CHECK:       sbc a,a
  %c = icmp sgt i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test: signed greater-than zero (SGT X, 0) - non-negative AND non-zero
define i8 @icmp_sgt_zero(i16 %a) {
; CHECK-LABEL: icmp_sgt_zero:
; CHECK:       rlca
; CHECK:       sbc a,a
; CHECK:       cpl
; CHECK:       or l
; CHECK:       and
; CHECK:       add a,#255
; CHECK:       sbc a,a
; CHECK:       and #1
; CHECK-NOT:   xor #1
  %c = icmp sgt i16 %a, 0
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test: signed less-or-equal zero (SLE X, 0) - inverted SGT X, 0
define i8 @icmp_sle_zero(i16 %a) {
; CHECK-LABEL: icmp_sle_zero:
; CHECK:       rlca
; CHECK:       sbc a,a
; CHECK:       cpl
; CHECK:       or l
; CHECK:       and
; CHECK:       add a,#255
; CHECK:       sbc a,a
; CHECK:       and #1
; CHECK:       xor #1
  %c = icmp sle i16 %a, 0
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test: unsigned less-or-equal (ULE) - swapped operands, 8-bit SUB/SBC chain
define i8 @icmp_ule16(i16 %a, i16 %b) {
; CHECK-LABEL: icmp_ule16:
; CHECK:       sub l
; CHECK:       sbc a,h
; CHECK:       ccf
; CHECK:       sbc a,a
; CHECK:       and #1
  %c = icmp ule i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test: unsigned greater-than (UGT) - swapped operands, 8-bit SUB/SBC chain
define i8 @icmp_ugt16(i16 %a, i16 %b) {
; CHECK-LABEL: icmp_ugt16:
; CHECK:       sub l
; CHECK:       sbc a,h
; CHECK:       sbc a,a
; CHECK:       and #1
  %c = icmp ugt i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}
