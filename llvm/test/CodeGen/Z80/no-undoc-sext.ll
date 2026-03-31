; RUN: llc -mtriple=z80 -O1 < %s | FileCheck %s
;
; Issue #37: SEXT/ZEXT pseudo expansion must not emit undocumented
; IXH/IXL/IYH/IYL instructions without +undocumented.

; Sign-extend i8 to i16 (SEXT_GR8_GR16 / SEXT16 pseudo).
define i16 @sext8(i8 %x) nounwind {
; CHECK-LABEL: sext8:
; CHECK-NOT:   ixh
; CHECK-NOT:   ixl
; CHECK-NOT:   iyh
; CHECK-NOT:   iyl
; CHECK:       ret
  %r = sext i8 %x to i16
  ret i16 %r
}

; Zero-extend i8 to i16 (ZEXT_GR8_GR16 pseudo).
define i16 @zext8(i8 %x) nounwind {
; CHECK-LABEL: zext8:
; CHECK-NOT:   ixh
; CHECK-NOT:   ixl
; CHECK-NOT:   iyh
; CHECK-NOT:   iyl
; CHECK:       ret
  %r = zext i8 %x to i16
  ret i16 %r
}

; Sign-extend i16 to i32 (SEXT16 pseudo).
define i32 @sext16(i16 %x) nounwind {
; CHECK-LABEL: sext16:
; CHECK-NOT:   ixh
; CHECK-NOT:   ixl
; CHECK-NOT:   iyh
; CHECK-NOT:   iyl
; CHECK:       ret
  %r = sext i16 %x to i32
  ret i32 %r
}

; Fixed-point multiply that triggered the original report.
define i16 @fp_mul(i16 %a, i16 %b) nounwind {
; CHECK-LABEL: fp_mul:
; CHECK-NOT:   ixh
; CHECK-NOT:   ixl
; CHECK-NOT:   iyh
; CHECK-NOT:   iyl
; CHECK:       ret
  %wa = sext i16 %a to i32
  %wb = sext i16 %b to i32
  %prod = mul i32 %wa, %wb
  %shifted = ashr i32 %prod, 8
  %r = trunc i32 %shifted to i16
  ret i16 %r
}
