; Stubs for ___z80_memcpy_builtin and ___memcpy on SM83, separate from _memcpy.
;
; Both are defined here so that sdcc's sm83.lib memcpy.rel (which provides
; ___memcpy and _memcpy in the same module) is never lazily loaded — avoiding
; a _memcpy multiple-definition conflict with sm83_rt.lib's eager _memcpy.
;
; ___z80_memcpy_builtin: CallingConv::Z80_Builtin (DE=dest, BC=src, HL=size)
; ___memcpy:             alias to _memcpy (same C calling convention)
	.area _CODE
	.globl ___z80_memcpy_builtin
	.globl ___memcpy
	.globl _memcpy
___z80_memcpy_builtin:
	ld	a, h
	or	l
	ret	z
	push	bc
	ld	b, h
	ld	c, l
	pop	hl
___z80_memcpy_loop:
	ld	a, (hl+)
	ld	(de), a
	inc	de
	dec	bc
	ld	a, b
	or	c
	jr	nz, ___z80_memcpy_loop
	ret
___memcpy:
	jp	_memcpy
