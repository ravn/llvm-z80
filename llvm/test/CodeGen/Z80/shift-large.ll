; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; SHL 7: RRCA + AND $80 (3B) instead of 7× ADD A,A (7B)
; CHECK-LABEL: shl7:
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	ret
define i8 @shl7(i8 %x) nounwind {
  %r = shl i8 %x, 7
  ret i8 %r
}

; SHL 6: 2× RRCA + AND $C0 (4B) instead of 6× ADD A,A (6B)
define i8 @shl6(i8 %x) nounwind {
  %r = shl i8 %x, 6
  ret i8 %r
}
; CHECK-LABEL: shl6:
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	ret

; SHL 5: should still use 5× ADD A,A (not profitable to rotate)
define i8 @shl5(i8 %x) nounwind {
  %r = shl i8 %x, 5
  ret i8 %r
}

; SHL 4: should still use 4× ADD A,A
define i8 @shl4(i8 %x) nounwind {
  %r = shl i8 %x, 4
  ret i8 %r
; CHECK-LABEL: shl5:
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	ret
}
; CHECK-LABEL: shl4:
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	add	a,a
; CHECK:      	ret
