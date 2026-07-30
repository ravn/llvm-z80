; SPDX-License-Identifier: Zlib OR Apache-2.0 WITH LLVM-exception OR MIT
	.area _CODE
	.globl _memset
	; _memset_done is local (no .globl) so `jr z` stays 2 bytes.

;===------------------------------------------------------------------------===;
; _memset - Fill memory block
;
; Input:  HL = dest, DE = value (E = byte), stack = size (i16)
; Output: DE = dest (original)
; Writes first byte, then uses LDIR to propagate to remaining bytes.
;
; The stack arg (size) is read via the `pop iy` idiom rather than an IX frame:
; IY is caller-saved (Z80_CSR = CalleeSavedRegs<(add IX)>), so this trampoline
; may clobber it freely. ~19 B tighter than the IX-frame prologue/epilogue.
;===------------------------------------------------------------------------===;
_memset:
	pop	iy		; return address (IY is caller-saved)
	pop	bc		; BC = size (callee-cleanup of the stack arg)
	push	hl		; save original dest for return value
	ld	a, b
	or	c
	jr	z, _memset_done
	ld	(hl), e		; write first byte
	dec	bc		; remaining = size - 1
	ld	a, b
	or	c
	jr	z, _memset_done	; size was 1
	ld	d, h		; DE = HL (points to first byte)
	ld	e, l
	inc	de		; DE = HL + 1 (next byte)
	ldir			; copy first byte to remaining
_memset_done:
	pop	de		; DE = original dest (return value)
	jp	(iy)
