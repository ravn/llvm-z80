; SPDX-License-Identifier: Zlib OR Apache-2.0 WITH LLVM-exception OR MIT
	.area _CODE
	.globl _memchr
	; _memchr_loop/_found/_notfound are local (no .globl) so the intra-function
	; `jr` jumps stay 2 bytes rather than 3-byte relocated `jp`.

; Input:  HL = ptr, DE = search byte (E), stack = size (i16)
; Output: DE = pointer to match, or 0 (NULL) if not found
;
; The stack arg (size) is read via the `pop iy` idiom rather than an IX frame:
; IY is caller-saved (Z80_CSR = CalleeSavedRegs<(add IX)>), so this trampoline
; may clobber it freely.
_memchr:
	pop	iy		; return address (IY is caller-saved)
	pop	bc		; BC = size (callee-cleanup of the stack arg)
_memchr_loop:
	ld	a, b
	or	c
	jr	z, _memchr_notfound
	ld	a, (hl)
	cp	e
	jr	z, _memchr_found
	inc	hl
	dec	bc
	jr	_memchr_loop
_memchr_found:
	ex	de, hl		; DE = pointer to match
	jp	(iy)
_memchr_notfound:
	ld	de, #0		; DE = NULL
	jp	(iy)

;===------------------------------------------------------------------------===;
; _bzero - Zero out memory block
;
; Input:  HL = ptr, DE = size
; Output: DE = ptr
