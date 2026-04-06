; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; SHL 7: RRCA + AND $80 (3B) instead of 7× ADD A,A (7B)
define i8 @shl7(i8 %x) nounwind {
; CHECK-LABEL: _shl7:
; CHECK:       rrca
; CHECK-NEXT:  and #128
; CHECK-NOT:   add a,a
  %r = shl i8 %x, 7
  ret i8 %r
}

; SHL 6: 2× RRCA + AND $C0 (4B) instead of 6× ADD A,A (6B)
define i8 @shl6(i8 %x) nounwind {
; CHECK-LABEL: _shl6:
; CHECK:       rrca
; CHECK-NEXT:  rrca
; CHECK-NEXT:  and #192
; CHECK-NOT:   add a,a
  %r = shl i8 %x, 6
  ret i8 %r
}

; SHL 5: should still use 5× ADD A,A (not profitable to rotate)
define i8 @shl5(i8 %x) nounwind {
; CHECK-LABEL: _shl5:
; CHECK:       add a,a
; CHECK-NOT:   rrca
  %r = shl i8 %x, 5
  ret i8 %r
}

; SHL 4: should still use 4× ADD A,A
define i8 @shl4(i8 %x) nounwind {
; CHECK-LABEL: _shl4:
; CHECK:       add a,a
; CHECK-NOT:   rrca
  %r = shl i8 %x, 4
  ret i8 %r
}
