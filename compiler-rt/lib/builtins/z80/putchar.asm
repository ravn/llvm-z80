; SPDX-License-Identifier: Zlib OR Apache-2.0 WITH LLVM-exception OR MIT
	.area _CODE
	.globl _putchar

;===------------------------------------------------------------------------===;
; _putchar - Write character to console
;
; Input:  A = character (sdcccall(1): first i8 arg in A)
; Output: DE = character (return value)
;
; Uses z88dk-ticks -iochar 1 for console output.
; Port 1 is mapped to stdout by z88dk-ticks.
;===------------------------------------------------------------------------===;
_putchar:
	out	(0x01), a	; write character to iochar port
	ld	e, a		; return value in DE
	ld	d, #0
	ret
