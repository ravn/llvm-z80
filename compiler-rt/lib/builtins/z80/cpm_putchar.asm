; CP/M putchar via BDOS function 2 (C_WRITE)
; Input: A = character (sdcccall(1))
; Uses BDOS: C=2, E=char, CALL 5
	.area _CODE
	.globl _putchar

_putchar:
	; int putchar(int c) — sdcccall(1): i16 arg in HL, char in L
	ld	e, l		; E = character for BDOS
	ld	c, #2		; C_WRITE function
	call	0x0005		; BDOS entry
	ld	e, l		; return value in DE = input char
	ld	d, h
	ret
