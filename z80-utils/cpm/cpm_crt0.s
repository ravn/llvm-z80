; cpm_crt0.s — CP/M 2.2 startup + BDOS interface for clang/llvm-z80
;
; CP/M loads .COM files to 0x0100 and jumps here.  We reset SP to 0xFFFE so
; the stack is at the top of the 64 KB address space regardless of CCP variant.
;
; BDOS calling convention (CP/M 2.2): C = function, DE = parameter.
; Clang/llvm-z80 ABI (sdcccall default): 8-bit arg in L, 16-bit arg in HL.

        .section .text.crt0,"ax",@progbits
        .global _start
        .global _cpm_exit
        .global _cpm_conout

_start:
        ld      sp, #0xFFFE             ; stack at top of address space

        ; Zero BSS via LDIR (symbols supplied by cpm.ld)
        ld      hl, #__bss_start
        ld      de, #(__bss_start + 1)
        ld      bc, #__bss_length
        ld      a, b
        or      c
        jr      z, .no_bss
        ld      (hl), #0
        dec     bc
        ldir
.no_bss:

        call    _main

_cpm_exit:
        ld      c, #0                   ; BDOS fn 0 = program terminate
        call    0x0005
        halt                            ; should not return

; void cpm_conout(int c)  [c arrives in L (low byte of HL) per sdcccall ABI]
; vcpm's BDOS fn 2 auto-inserts CR before LF, so we pass characters as-is.
_cpm_conout:
        ld      e, l                    ; E = character
        ld      c, #2                   ; BDOS fn 2 = CONOUT
        call    0x0005
        ret
