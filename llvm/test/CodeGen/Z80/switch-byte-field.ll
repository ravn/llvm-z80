; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; ==========================================================================
; Switch on a 2-bit field extracted from a byte (IOBYTE-style dispatch).
;
; Regression test for issue #69: the comparison reversal peephole in
; Z80LateOptimization erased LD r,A (saving the discriminant) without
; checking if r was live-out to a successor block.  The second case
; comparison then loaded a stale caller register instead of the
; discriminant.
;
; The test verifies that the second case comparison uses the correct
; value, not a stale register.
; ==========================================================================

@iobyte = external global i8

declare void @case_low()
declare void @case_two()
declare void @case_default()

; Extract bits 3:2 from a global byte and dispatch on value 0-3.
; case 0,1 → case_low()
; case 2   → case_two()
; case 3   → case_default()
define void @switch_byte_field() {
; CHECK-LABEL: _switch_byte_field:
; CHECK:       ld	a, (_iobyte)
; CHECK:       srl	a
; CHECK:       srl	a
; CHECK:       and	#3
;
; The comparison reversal peephole folds LD r,A + LD A,#imm + CP r
; into CP #(imm+1).  When the saved register is live-out, the LD r,A
; must be preserved.
; CHECK:       cp	#2
;
; The second comparison must NOT use a stale register.
; CHECK-NOT:   ld	a, d
; CHECK:       cp	#2
  %raw = load volatile i8, ptr @iobyte
  %shifted = lshr i8 %raw, 2
  %field = and i8 %shifted, 3
  switch i8 %field, label %bb.default [
    i8 0, label %bb.low
    i8 1, label %bb.low
    i8 2, label %bb.two
  ]

bb.low:
  call void @case_low()
  ret void

bb.two:
  call void @case_two()
  ret void

bb.default:
  call void @case_default()
  ret void
}
