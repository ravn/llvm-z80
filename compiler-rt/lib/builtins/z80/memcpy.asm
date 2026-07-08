; SPDX-License-Identifier: Zlib OR Apache-2.0 WITH LLVM-exception OR MIT
	.area _CODE
	.globl _memcpy
	; _memcpy_done is a local label (no .globl) so `jr z` stays 2 bytes
	; rather than being promoted to a 3-byte relocated `jp z`.

;===------------------------------------------------------------------------===;
; _memcpy - Copy memory block
;
; Input:  HL = dest, DE = src, stack = size (i16)
; Output: DE = dest (original)
; Uses LDIR: copies (HL)->(DE), HL++, DE++, BC--, repeat until BC=0
; Note: LDIR source is HL, dest is DE, so we swap HL/DE from calling conv.
;
; The stack arg (size) is read via the `pop iy` idiom rather than an IX frame:
; IY is caller-saved (Z80CallingConv.td: Z80_CSR = CalleeSavedRegs<(add IX)> --
; only IX is callee-saved), so this trampoline may clobber it freely. This is
; ~17 B tighter than push ix / ld ix,0 / add ix,sp / ld b,5(ix).
;===------------------------------------------------------------------------===;
_memcpy:
	pop	iy		; return address (IY is caller-saved)
	pop	bc		; BC = size (callee-cleanup of the stack arg)
	ex	de, hl		; HL = src, DE = dest (LDIR format)
	push	de		; save original dest for return value
	ld	a, b
	or	c
	jr	z, _memcpy_done
	ldir
_memcpy_done:
	pop	de		; DE = original dest (return value)
	jp	(iy)
