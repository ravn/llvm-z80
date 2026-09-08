; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; Test: loop counter placed in B, late optimization converts to DJNZ.
; The GR8 allocation order puts B last (before A), keeping it available
; for the DJNZ register hint on loop counters.

; CHECK-LABEL: delay:
; CHECK:      	ld	b,a
; CHECK:      	dec	b
; CHECK:      	jr	nz,.LBB0_1
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
; CHECK:      	ld	c,(hl)
; CHECK:      	pop	hl
; CHECK:      	ld	b,#0
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	a,b
; CHECK:      	add	a,d
; CHECK:      	ld	b,a
; CHECK:      	inc	hl
; CHECK:      	dec	c
; CHECK:      	jr	nz,.LBB1_1
; CHECK:      	ld	a,b
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
