; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; Test conditional branch based on 16-bit equality comparison
; CMP+BR fusion: G_ICMP EQ + G_BRCOND → XOR-based compare + JR Z
define i16 @branch_eq(i16 %a, i16 %b) {
; CHECK-LABEL: _branch_eq:
; CHECK:       xor
; CHECK:       or
; CHECK:       j{{[rp]}} z,
  %cond = icmp eq i16 %a, %b
  br i1 %cond, label %then, label %else
then:
  ret i16 %a
else:
  ret i16 %b
}

; Test 8-bit equality comparison
define i8 @icmp_eq8(i8 %a, i8 %b) {
; CHECK-LABEL: _icmp_eq8:
; CHECK:       sub l
  %c = icmp eq i8 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test 16-bit unsigned less-than (8-bit SUB/SBC chain)
define i8 @icmp_ult16(i16 %a, i16 %b) {
; CHECK-LABEL: _icmp_ult16:
; CHECK:       sub e
; CHECK:       sbc a,d
; CHECK:       sbc a,a
; CHECK:       and #1
  %c = icmp ult i16 %a, %b
  %r = zext i1 %c to i8
  ret i8 %r
}

; Test conditional branch for SGT X, 0 (fused: non-negative AND non-zero)
define i16 @branch_sgt_zero(i16 %a) {
; CHECK-LABEL: _branch_sgt_zero:
; CHECK:       rlca
; CHECK:       sbc a,a
; CHECK:       cpl
; CHECK:       or l
; CHECK:       and
; CHECK:       j{{[rp]}} nz,
  %cond = icmp sgt i16 %a, 0
  br i1 %cond, label %then, label %else
then:
  ret i16 42
else:
  ret i16 0
}

; Test conditional branch for SLE X, 0 (fused: inverted SGT zero)
define i16 @branch_sle_zero(i16 %a) {
; CHECK-LABEL: _branch_sle_zero:
; CHECK:       rlca
; CHECK:       sbc a,a
; CHECK:       cpl
; CHECK:       or l
; CHECK:       and
; CHECK:       j{{[rp]}} z,
  %cond = icmp sle i16 %a, 0
  br i1 %cond, label %then, label %else
then:
  ret i16 42
else:
  ret i16 0
}
