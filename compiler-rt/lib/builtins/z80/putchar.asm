; SPDX-License-Identifier: Zlib OR Apache-2.0 WITH LLVM-exception OR MIT
	.area _CODE
	.globl _putchar

;===------------------------------------------------------------------------===;
; _putchar - Write character to console
;
; Input:  int putchar(int c) — sdcccall(1): i16 in HL, char in L
; Output: DE = character (return value)
;
; Uses z88dk-ticks -iochar 1 for console output.
; Port 1 is mapped to stdout by z88dk-ticks.
;===------------------------------------------------------------------------===;
_putchar:
	ld	a, l		; char from L (low byte of HL)
	out	(0x01), a	; write character to iochar port
	ld	e, l		; return value in DE
	ld	d, h
	ret
