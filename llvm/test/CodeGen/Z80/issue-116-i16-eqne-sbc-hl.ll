; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 -disable-lsr < %s | FileCheck %s

; ravn/llvm-z80#116 — i16 EQ/NE compare-and-branch: when one of the
; compared pairs is already in HL post-RA AND HL is dead-after-compare,
; replace the 6-byte byte-XOR sequence with AND A; SBC HL,rr (3 bytes,
; saves 3 B and 5 T-states per fire).
;
; Implemented as a post-RA peephole in Z80LateOptimization.cpp so the
; firing decision sees actual register placement and HL liveness — an
; earlier ISel-time gate (commit 33ceae174673) was reverted because
; forcing LHS into HL evicted long-lived values out of HL across loops
; and regressed rcbios bios.cim by +27 B.

@end_idx = external global i16

; ---- HL is loop-carried (held across iterations): peephole must NOT
;      fire, because SBC would clobber the loop-carried value.  Falls
;      back to the byte-XOR shape.
define i16 @count_to_end(i16 %start) {
entry:
  %end = load i16, ptr @end_idx, align 1
  br label %loop

loop:
; CHECK-LABEL: count_to_end:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	hl,#_end_idx
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	inc	bc
; CHECK:      	ld	a,b
; CHECK:      	xor	d
; CHECK:      	ld	h,a
; CHECK:      	ld	a,c
; CHECK:      	xor	e
; CHECK:      	or	h
; CHECK:      	jr	nz,.LBB0_1
; CHECK:      	ld	e,c
; CHECK:      	ld	d,b
; CHECK:      	ret
  %i = phi i16 [ %start, %entry ], [ %i.next, %loop ]
  %i.next = add i16 %i, 1
  %done = icmp eq i16 %i.next, %end
  br i1 %done, label %ret, label %loop

ret:
  ret i16 %i.next
}

; ---- HL is freshly loaded each iteration AND dead after the compare:
;      peephole fires.  Loop body shrinks from 12 B (XOR shape) to 9 B
;      (SBC shape).
define void @loop_dead_hl(i16 %v) {
entry:
  br label %loop

loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %i.next = add i16 %i, 1
  %end = load volatile i16, ptr @end_idx, align 1
  %done = icmp eq i16 %i.next, %end
  br i1 %done, label %ret, label %loop

ret:
  ret void
; CHECK-LABEL: loop_dead_hl:
; CHECK:      	ld	bc,#0
; CHECK:      	inc	bc
; CHECK:      	ld	hl,#_end_idx
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	a,b
; CHECK:      	xor	d
; CHECK:      	ld	d,a
; CHECK:      	ld	a,c
; CHECK:      	xor	e
; CHECK:      	or	d
; CHECK:      	jr	nz,.LBB1_1
; CHECK:      	ret
}
