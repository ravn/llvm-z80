; SPDX-License-Identifier: Zlib OR Apache-2.0 WITH LLVM-exception OR MIT
	.area _CODE
	.globl ___udivhi3_fast

;===------------------------------------------------------------------------===;
; ___udivhi3_fast - 16-bit unsigned division, speed variant (ravn/llvm-z80 #244)
;
; Identical result to ___udivhi3, but the common 8-bit-divisor path is FULLY
; UNROLLED (no `djnz`), selected by the backend only at -O3 on Z80.
;
; The 8-bit path splits on the divisor size.  The partial remainder satisfies
; P = 2*rem + bit < 2*E, so its 9th bit can only appear when E > 128.  For
; E <= 128 the tight loop (remainder in 8-bit A) is exact, so we take the fast
; fully-unrolled body.  For E in 129..255 we fall to a compact correct loop
; that honours the 9th bit (the carry out of `rla`) -- dropping it miscompiled
; ~38% of large-dividend divisions (e.g. 60000/200 -> 256 instead of 300).
;
; Input:  HL = dividend, DE = divisor
; Output: DE = quotient, HL = remainder
;===------------------------------------------------------------------------===;
___udivhi3_fast:
	ld	a, d
	or	a
	jp	nz, .L16bit
	bit	7, e		; divisor > 128?  then the 9th bit can appear
	jp	nz, .Lbig8
	;; --- 8-bit divisor <= 128 fast path, fully unrolled (exact) ---
	xor	a		; A = 0 (remainder), CF = 0
	add	hl, hl		; initial shift: dividend MSB -> carry
	rla
	sub	e
	jr	nc, .Lok0
	add	a, e
.Lok0:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok1
	add	a, e
.Lok1:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok2
	add	a, e
.Lok2:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok3
	add	a, e
.Lok3:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok4
	add	a, e
.Lok4:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok5
	add	a, e
.Lok5:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok6
	add	a, e
.Lok6:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok7
	add	a, e
.Lok7:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok8
	add	a, e
.Lok8:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok9
	add	a, e
.Lok9:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok10
	add	a, e
.Lok10:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok11
	add	a, e
.Lok11:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok12
	add	a, e
.Lok12:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok13
	add	a, e
.Lok13:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok14
	add	a, e
.Lok14:
	ccf
	adc	hl, hl
	rla
	sub	e
	jr	nc, .Lok15
	add	a, e
.Lok15:
	ccf
	adc	hl, hl
	ld	e, a
	ex	de, hl		; DE = quotient, HL = 0:remainder
	ret
	;; --- 8-bit divisor 129..255: compact correct loop (9th bit honoured) ---
.Lbig8:
	xor	a		; A = 0 (remainder), CF = 0
	ld	b, #16
	add	hl, hl		; initial shift: dividend MSB -> carry
.Lbigloop:
	rla			; A = 2*rem + carry; CF = 9th bit (P >= 256)
	jr	c, .Lbsub	; P >= 256 > E: subtract, quotient bit 1
	sub	e
	jr	nc, .Lbset	; A >= E: quotient bit 1
	add	a, e		; restore, quotient bit 0
	or	a		; CF = 0
	jr	.Lbshift
.Lbsub:
	sub	e		; 9-bit P - E (fits, P < 2*E)
.Lbset:
	scf			; quotient bit 1
.Lbshift:
	adc	hl, hl
	djnz	.Lbigloop
	ld	e, a
	ex	de, hl
	ret
	;; --- 16-bit divisor path ---
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
