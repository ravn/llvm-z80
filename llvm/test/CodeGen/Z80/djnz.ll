; RUN: llc -mtriple=z80 -O1 -stop-after=z80-late-opt < %s | FileCheck %s

; Test: loop counter placed in B, late optimization converts to DJNZ.
; The GR8 allocation order puts B last (before A), keeping it available
; for the DJNZ register hint on loop counters.
;
; Note: uses -stop-after because branch relaxation has a pre-existing
; crash with simple do-while loops (tracked separately).

define void @delay(i8 %n) {
; CHECK-LABEL: name: delay
; CHECK:       LD_B_A
; CHECK:       DJNZ_e
entry:
  br label %loop

loop:
  %i = phi i8 [ %n, %entry ], [ %i.next, %loop ]
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit

exit:
  ret void
}
