; CP/M CRT0 for clang Z80
; Entry at 0x0100 (set by linker script cpm.ld)
; BDOS at address 5 (set up by CP/M or z88dk-ticks)
	.area _CODE
	.globl _start
	.globl __bss_start
	.globl __bss_size

_start:
	; BSS zero-fill
	ld	hl, #__bss_start
	ld	bc, #__bss_size
	ld	a, b
	or	c
	jr	z, _bss_done
	ld	(hl), #0
	dec	bc
	ld	a, b
	or	c
	jr	z, _bss_done
	ld	d, h
	ld	e, l
	inc	de
	ldir
_bss_done:
	call	_main
	; Exit: return to CP/M (or z88dk-ticks stops on JP 0)
	jp	0x0000
