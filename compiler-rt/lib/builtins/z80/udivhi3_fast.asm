; SPDX-License-Identifier: Zlib OR Apache-2.0 WITH LLVM-exception OR MIT
	.area _CODE
	.globl ___udivhi3_fast

;===------------------------------------------------------------------------===;
; ___udivhi3_fast - 16-bit unsigned division, speed variant (ravn/llvm-z80 #244)
;
; Identical result to ___udivhi3, selected by the backend only at -O3 on Z80.
;
; Small quotients dominate division-heavy code (e.g. the digits-of-e kernel,
; where each x/n has x < ~10*n so the quotient is < ~10).  For those we use
; REPEATED SUBTRACTION -- a handful of full 16-bit `sbc hl,de` steps -- which
; is both faster than 16 fixed bit-iterations and correct by construction (no
; 9-bit partial-remainder hazard, unlike the 8-bit `A`-remainder tight loop).
; A cap of 40 bounds the worst case; on cap we fall back to the general 16-step
; bit divider (.L16bit) on the original dividend.  This mirrors dcc's DRSU/D16U.
;
; Input:  HL = dividend, DE = divisor
; Output: DE = quotient, HL = remainder
;===------------------------------------------------------------------------===;
___udivhi3_fast:
	push	hl		; save dividend for the cap fallback
	ld	bc, #0x2800	; B = 40 (attempt cap), C = 0 (quotient counter)
.Lrs:
	or	a		; CF = 0 for a clean subtract
	sbc	hl, de		; HL -= divisor
	jr	c, .Lrsdone	; borrow: overshot, quotient found
	inc	c		; another whole divisor subtracted
	djnz	.Lrs
	;; cap reached: quotient >= 40, use the general bit divider
	pop	hl		; restore original dividend
	jr	.L16bit
.Lrsdone:
	add	hl, de		; undo the overshoot -> HL = remainder
	ld	d, #0
	ld	e, c		; DE = quotient (< 40)
	pop	bc		; discard saved dividend
	ret
	;; --- general 16-step bit divider (correct for any divisor) ---
.L16bit:
	ld	b, h
	ld	c, l		; BC = dividend (becomes quotient)
	ld	hl, #0		; remainder
	ld	a, #16		; bit counter
.L16loop:
	sla	c
	rl	b
	adc	hl, hl
	jr	c, .Loverflow
	sbc	hl, de
	jr	nc, .Lsetbit
	add	hl, de
	jr	.Lnext
.Loverflow:
	or	a
	sbc	hl, de
.Lsetbit:
	inc	c
.Lnext:
	dec	a
	jr	nz, .L16loop
	ld	d, b
	ld	e, c		; DE = quotient
	ret
