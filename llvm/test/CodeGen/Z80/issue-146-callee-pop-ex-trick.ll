; RUN: llc -mtriple=z80 -O2 -disable-lsr < %s | FileCheck %s
;
; ravn/llvm-z80#146: RET_CLEANUP expansion uses EX (SP),HL trick when HL is
; dead (i8 / void return), saving 2 B vs the POP BC sequence regardless of N.
; For i16 returns HL holds the return value -- fall back to POP BC.
;
; EX trick shape (N bytes of callee-pop args, HL dead):
;   pop hl                      ; HL = ret addr, SP -> args
;   inc sp x (N-2)              ; skip first N-2 arg bytes
;   ex (sp),hl                  ; (SP) = ret addr, HL = garbage
;   ret                         ; jump to ret addr, SP += 2 past all args
;
; BC fallback shape (HL holds return value):
;   pop bc                      ; BC = ret addr
;   inc sp x N  (or LD HL,N; ADD HL,SP; LD SP,HL for large N)
;   push bc                     ; (SP) = ret addr
;   ret

; --- i8 return: HL dead, EX trick fires ----------------------------------------
; Four i8 args spill to stack (sdcccall(1) exhausts regs after A,L,E,C).
; CHECK-LABEL: ret_i8:
; CHECK:      	push	hl
; CHECK:      	ld	hl,4
; CHECK:      	add	hl,sp
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	hl,5
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	ld	hl,6
; CHECK:      	add	hl,sp
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	hl,7
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	pop	hl
; CHECK:      	add	a,l
; CHECK:      	add	a,b
; CHECK:      	add	a,c
; CHECK:      	add	a,d
; CHECK:      	add	a,e
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
define zeroext i8 @ret_i8(i8 %a, i8 %b, i8 %c, i8 %d, i8 %e, i8 %f) {
  %sum = add i8 %a, %b
  %sum2 = add i8 %sum, %c
  %sum3 = add i8 %sum2, %d
  %sum4 = add i8 %sum3, %e
  %sum5 = add i8 %sum4, %f
  ret i8 %sum5
}

; EX trick: zero or more INC SP between pop hl and ex (sp),hl.
; BC fallback must not appear.

; --- i16 return: HL holds return value, BC fallback used -----------------------
define i16 @ret_i16(i8 %a, i8 %b, i8 %c, i8 %d, i8 %e, i8 %f) {
  %ae = zext i8 %a to i16
  %be = zext i8 %b to i16
  %ce = zext i8 %c to i16
  %de = zext i8 %d to i16
  %ee = zext i8 %e to i16
; CHECK-LABEL: ret_i16:
; CHECK:      	push	hl
; CHECK:      	ld	hl,4
; CHECK:      	add	hl,sp
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	c,b
; CHECK:      	ld	b,0
; CHECK:      	ld	(L_ret_i16.frame+6),bc
; CHECK:      	ld	hl,5
; CHECK:      	add	hl,sp
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	c,b
; CHECK:      	ld	b,0
; CHECK:      	ld	(L_ret_i16.frame+4),bc
; CHECK:      	ld	hl,6
; CHECK:      	add	hl,sp
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	c,b
; CHECK:      	ld	b,0
; CHECK:      	ld	(L_ret_i16.frame+2),bc
; CHECK:      	ld	hl,7
; CHECK:      	add	hl,sp
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	ld	c,b
; CHECK:      	ld	b,0
; CHECK:      	ld	(L_ret_i16.frame),bc
; CHECK:      	ld	c,a
; CHECK:      	ld	b,0
; CHECK:      	ld	e,l
; CHECK:      	ld	d,b
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	bc,(L_ret_i16.frame+6)
; CHECK:      	add	hl,bc
; CHECK:      	ld	bc,(L_ret_i16.frame+4)
; CHECK:      	add	hl,bc
; CHECK:      	ld	bc,(L_ret_i16.frame+2)
; CHECK:      	add	hl,bc
; CHECK:      	ld	bc,(L_ret_i16.frame)
; CHECK:      	add	hl,bc
; CHECK:      	ex	de,hl
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
  %fe = zext i8 %f to i16
  %sum = add i16 %ae, %be
  %sum2 = add i16 %sum, %ce
  %sum3 = add i16 %sum2, %de
  %sum4 = add i16 %sum3, %ee
  %sum5 = add i16 %sum4, %fe
  ret i16 %sum5
}

; EX trick must not appear for i16 returns.
