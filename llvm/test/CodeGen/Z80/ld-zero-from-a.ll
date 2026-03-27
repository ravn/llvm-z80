; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; Test: OR A; LD r,0 → OR A; LD r,A peephole.
; When A is known zero after OR A, loading 0 into another register can
; use LD r,A (1 byte) instead of LD r,#0 (2 bytes).
; This occurs in select lowering when the selected value is used later
; (not immediately returned), producing a triangle with join point.

@flag = external global i8

declare void @use(i8 zeroext, i8 zeroext)

define void @select_zero_merge(i8 %x) {
; CHECK-LABEL: _select_zero_merge:
; The select of (flag ? 64 : 0) should use LD r,A after OR A in the
; zero case, not LD r,#0.
; CHECK:       or	a
; CHECK-NOT:   ld	d,#0x00
; CHECK-NOT:   ld	e,#0x00
entry:
  %v = load i8, ptr @flag
  %cmp = icmp ne i8 %v, 0
  %sel = select i1 %cmp, i8 64, i8 0
  call void @use(i8 %x, i8 %sel)
  ret void
}
