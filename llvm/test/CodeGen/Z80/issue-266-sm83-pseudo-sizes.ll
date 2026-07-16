; RUN: llc -mtriple=z80 -mattr=+sm83 < %s | FileCheck %s --check-prefix=SM83
; RUN: llc -mtriple=z80              < %s | FileCheck %s --check-prefix=Z80
;
; Regression test for #266: getInstSizeInBytes returned the Z80 size (6/7/7)
; for SUB_HL_rr_BO / ADC_HL_rr_CIO / SBC_HL_rr_BIO on SM83, instead of the
; correct SM83 sizes (9/11/11). BranchRelaxation uses these sizes to decide
; between JR (±127) and JP; an underestimate can leave a JR whose real offset
; exceeds 127, which would be rejected at final assembly.
;
; This test verifies the SM83 expansion emits the byte-by-byte sequences
; (confirming the pseudo is 9/11/11 bytes, not 6/7/7), and that Z80 still
; uses the compact ED-prefixed 16-bit ops.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-f32:8-f64:8-n8:16"
target triple = "z80"

; SUB_HL_rr_BO (and SBC_HL_rr_BIO for the high pair): i32 subtract.
; SM83 must emit ld/sub/ld/ld/sbc/ld byte-by-byte; Z80 uses SBC HL,rr.
define i32 @sub32(i32 %a, i32 %b) {
  %r = sub i32 %a, %b
  ret i32 %r
}
; SM83-LABEL: _sub32:
; SM83: ld a,l
; SM83-NEXT: sub
; SM83-NEXT: ld l,a
; SM83-NEXT: ld a,h
; SM83-NEXT: sbc a,
; SM83-NEXT: ld h,a
; SM83-NEXT: sbc a,a
; SM83-NEXT: and 1
; Z80-LABEL: _sub32:
; Z80: sbc hl,

; ADC_HL_rr_CIO / SBC_HL_rr_BIO: i64 subtract exercises the carry-chain path.
define i64 @sub64(i64 %a, i64 %b) {
  %r = sub i64 %a, %b
  ret i64 %r
}
; SM83-LABEL: _sub64:
; SM83: rrca
; SM83: ld a,l
; SM83: sbc a,
; SM83: ld l,a
; SM83: ld a,h
; SM83: sbc a,
; SM83: ld h,a
; Z80-LABEL: _sub64:
; Z80: sbc hl,
