; CP/M CRT0 for clang Z80 (SDCC toolchain variant)
; Entry at 0x0100 via .area _CODE org directive
; BDOS at address 5 (set up by CP/M or z88dk-ticks)
; Uses s__BSS/l__BSS from sdldz80 linker.

	.module cpm_crt0
	.area _CODE

	.globl _start
	.globl _main
	.globl _halt

_start:
	;; BSS zero-fill
	ld	hl,#s__BSS
	ld	bc,#l__BSS
	ld	a,b
	or	a,c
	jr	z,_bss_done
	ld	(hl),#0
	dec	bc
	ld	a,b
	or	a,c
	jr	z,_bss_done
	ld	d,h
	ld	e,l
	inc	de
	ldir
_bss_done:
	call	_main
	;; Exit via JP 0 (CP/M warm boot)
	jp	0x0000
_halt:
	halt

	.area _BSS
