; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -mattr=+static-stack -O2 < %s | FileCheck %s

; Issue #84: pattern-fill loop bodies emitted by GISel back HL up
; via BC (LD C,L; LD B,H; INC BC × N) at the top of the loop, run
; the body which advances HL itself, then restore HL from BC at
; the bottom (LD L,C; LD H,B).  The save+restore costs 6 B per
; iteration; the body's own INC_HL count (M) plus a few more
; (N - M) trailing INC_HL is sufficient.
;
; For N=2, M=1 (the canonical "fill 16-bit slots with constant"
; loop): -5 B per loop body.

@target_fn = external dso_local global i8

define void @word_fill_18() {
entry:
  br label %loop
loop:
  %p = phi ptr [ inttoptr (i16 -2816 to ptr), %entry ], [ %p.next, %loop.body ]
  %n = phi i8 [ 18, %entry ], [ %n.next, %loop.body ]
  %done = icmp eq i8 %n, 0
  br i1 %done, label %exit, label %loop.body
loop.body:
  %p.next = getelementptr inbounds nuw i8, ptr %p, i16 2
  store volatile i16 ptrtoint (ptr @target_fn to i16), ptr %p, align 1
  %n.next = sub i8 %n, 1
  br label %loop
exit:
  ret void
}

; CHECK-LABEL: _word_fill_18:
; CHECK-NOT:  ld  c,l
; CHECK-NOT:  ld  b,h
; CHECK-NOT:  ld  l,c
; CHECK-NOT:  ld  h,b
; CHECK:      ld  (hl),e
; CHECK-NEXT: inc  hl
; CHECK-NEXT: ld  (hl),d
; The body's own INC HL (M=1) + the synthesized trailing INC HL
; (N - M = 1) advance HL by 2 = the original BC pre-increment count.
; CHECK:      inc  hl
; CHECK:      jr
