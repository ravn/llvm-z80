; Stubs for ___z80_memcpy_builtin and ___memcpy, separate from _memcpy.
;
; Both are defined here so that sdcc's z80.lib memcpy.rel (which provides
; ___memcpy and _memcpy in the same module) is never lazily loaded — avoiding
; a _memcpy multiple-definition conflict with z80_rt.lib's eager _memcpy.
;
; ___z80_memcpy_builtin: CallingConv::Z80_Builtin (HL=dest, DE=src, BC=size)
; ___memcpy:             alias to _memcpy (same C calling convention)
	.area _CODE
	.globl ___z80_memcpy_builtin
	.globl ___memcpy
	.globl _memcpy
___z80_memcpy_builtin:
	ex	de, hl
	ld	a, b
	or	c
	ret	z
	ldir
	ret
___memcpy:
	jp	_memcpy
