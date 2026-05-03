; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 -disable-lsr < %s | FileCheck %s

; ravn/llvm-z80#116 — i16 EQ/NE compare-and-branch with variable RHS:
; current shape is byte-level XOR (6 B compare-only, 8 B with branch).
;
; Regression-guard for the *current* shape so an attempt at the SBC HL,DE
; transform (3 B compare-only) doesn't slip in unnoticed and ship a
; pessimization.  An ISel-time gate keyed on hasMinSize() was tried and
; reverted on 2026-05-03: forcing LHS into HL via SUB_HL_rr's HL-Def
; evicts long-lived values out of HL and adds BSS reload traffic that
; outweighs the per-fire savings (rcbios bios.cim +27 B).
;
; The proper fix is a post-RA peephole that inspects actual register
; placement and HL liveness — fire only when LHS is already in HL (or
; cheaply movable) AND the surrounding HL value is dead-after-compare.
; When that lands, flip the CHECK lines below to expect:
;     and  a
;     sbc  hl,{de|bc}
;     jr   nz,
; and update the second function to demonstrate the SBC shape under
; minsize.

@end_idx = external global i16

; ---- Default: byte-XOR sequence ----------------------------------------
; CHECK-LABEL: _count_to_end:
; CHECK:       xor
; CHECK:       xor
; CHECK:       or
; CHECK:       jr  nz,
define i16 @count_to_end(i16 %start) {
entry:
  %end = load i16, ptr @end_idx, align 1
  br label %loop

loop:
  %i = phi i16 [ %start, %entry ], [ %i.next, %loop ]
  %i.next = add i16 %i, 1
  %done = icmp eq i16 %i.next, %end
  br i1 %done, label %ret, label %loop

ret:
  ret i16 %i.next
}

; ---- minsize: still byte-XOR for now (#116 not yet implemented) --------
; CHECK-LABEL: _count_to_end_minsize:
; CHECK:       xor
; CHECK:       xor
; CHECK:       or
; CHECK:       jr  nz,
define i16 @count_to_end_minsize(i16 %start) #0 {
entry:
  %end = load i16, ptr @end_idx, align 1
  br label %loop

loop:
  %i = phi i16 [ %start, %entry ], [ %i.next, %loop ]
  %i.next = add i16 %i, 1
  %done = icmp eq i16 %i.next, %end
  br i1 %done, label %ret, label %loop

ret:
  ret i16 %i.next
}

attributes #0 = { minsize }
