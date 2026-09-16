; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; Test: loop counter placed in B, late optimization converts to DJNZ.
; The GR8 allocation order puts B last (before A), keeping it available
; for the DJNZ register hint on loop counters.

; CHECK-LABEL: delay:
; CHECK:      	ld	b,a
; CHECK:      	djnz	.LBB0_1
; CHECK:      	ret
define void @delay(i8 %n) {
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

; Test: DJNZ with a loop body (memory access + accumulation)
define i8 @sum_array(ptr %p, i8 %n) {
entry:
  br label %loop
; CHECK-LABEL: sum_array:
; CHECK:      	push	hl
; CHECK:      	ld	hl,#4
; CHECK:      	add	hl,sp
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	ld	d,#0
; CHECK:      	ld	c,(hl)
; CHECK:      	ld	a,d
; CHECK:      	add	a,c
; CHECK:      	ld	d,a
; CHECK:      	inc	hl
; CHECK:      	djnz	.LBB1_1
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret

loop:
  %i = phi i8 [ %n, %entry ], [ %i.next, %loop ]
  %ptr = phi ptr [ %p, %entry ], [ %ptr.next, %loop ]
  %sum = phi i8 [ 0, %entry ], [ %sum.next, %loop ]
  %val = load i8, ptr %ptr
  %sum.next = add i8 %sum, %val
  %ptr.next = getelementptr i8, ptr %ptr, i8 1
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit

exit:
  ret i8 %sum.next
}
