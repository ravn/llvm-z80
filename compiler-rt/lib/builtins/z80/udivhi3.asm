; SPDX-License-Identifier: Zlib OR Apache-2.0 WITH LLVM-exception OR MIT
	.area _CODE
	.globl ___udivhi3

;===------------------------------------------------------------------------===;
; ___udivhi3 - 16-bit unsigned division
;
; Input:  HL = dividend, DE = divisor
; Output: DE = quotient, HL = remainder
; Algorithm: restoring division with 8-bit divisor fast path
;===------------------------------------------------------------------------===;
___udivhi3:
	ld	a, d
	or	a
	jr	nz, ___udivhi3_16bit
	;; --- 8-bit divisor fast path (D == 0) ---
	;; HL serves as both dividend shift register and quotient accumulator.
	;; A holds the partial remainder.  It is < 256 at each step boundary
	;; (divisor < 256), but DURING a step it is doubled and a bit brought in,
	;; giving a 9-bit value P = 2*rem + bit with P < 2*divisor <= 510.  That
	;; 9th bit lives in the carry out of `rla`; it MUST be honoured (dropping
	;; it miscompiled ~38% of large-dividend divisions, e.g. 60000/200 -> 256
	;; instead of 300 -- ravn/llvm-z80 #244).  When CF9 is set P >= 256 > E,
	;; so we subtract unconditionally (quotient bit 1); otherwise compare A vs
	;; E as usual.  Since P < 2*E the single subtraction always leaves A < E.
	xor	a		; A = 0 (remainder), CF = 0
	ld	b, #16		; iteration counter
	add	hl, hl		; initial shift: dividend MSB -> carry
___udivhi3_8loop:
	rla			; A = 2*rem + carry; CF = 9th bit (P >= 256)
	jr	c, ___udivhi3_8sub	; P >= 256 > E: subtract, quotient bit 1
	sub	e		; trial subtract (P < 256)
	jr	nc, ___udivhi3_8set	; A >= E: quotient bit 1
	add	a, e		; A < E: restore remainder
	or	a		; CF = 0 (quotient bit 0)
	jr	___udivhi3_8shift
___udivhi3_8sub:
	sub	e		; 9-bit P - E (fits in 8 bits since P < 2*E)
___udivhi3_8set:
	scf			; CF = 1 (quotient bit 1)
___udivhi3_8shift:
	adc	hl, hl		; shift HL + quotient bit; MSB -> CF for next rla
	djnz	___udivhi3_8loop
	; HL = quotient, A = remainder, D = 0
	ld	e, a		; E = remainder
	ex	de, hl		; DE = quotient, HL = 0:remainder
	ret
	;; --- 16-bit divisor path ---
___udivhi3_16bit:
	ld	b, h
	ld	c, l		; BC = dividend (becomes quotient)
	ld	hl, #0		; remainder
	ld	a, #16		; bit counter
___udivhi3_16loop:
	sla	c		; shift BC left
	rl	b		; MSB -> carry
	adc	hl, hl		; remainder = remainder * 2 + carry
	jr	c, ___udivhi3_overflow	; 17-bit remainder, always >= divisor
	sbc	hl, de		; trial subtract (carry = 0 here)
	jr	nc, ___udivhi3_setbit
	add	hl, de		; restore remainder
	jr	___udivhi3_next
___udivhi3_overflow:
	or	a		; clear carry
	sbc	hl, de		; subtract (always fits)
___udivhi3_setbit:
	inc	c		; set quotient bit 0
___udivhi3_next:
	dec	a
	jr	nz, ___udivhi3_16loop
	ld	d, b
	ld	e, c		; DE = quotient
	ret
