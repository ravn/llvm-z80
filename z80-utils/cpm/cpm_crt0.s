; cpm_crt0.s — CP/M 2.2 startup + BDOS interface for clang/llvm-z80
;
; Calling convention (sdcccall default):
;   8-bit arg  -> L;  16-bit arg -> HL;  2nd 16-bit arg -> DE
;   Return: 16-bit -> HL
;
; Z80 store limits worth remembering here: only LD (HL),r takes an arbitrary
; 8-bit register; LD (DE),A / LD (BC),A are A-only.  LD SP,rr exists only for
; SP<-HL.  LD (nn),SP / LD BC,(nn) / LD HL,(nn) are the wide indirect moves.

        .section .text.crt0,"ax",@progbits
        .global _start
        .global _cpm_exit
        .global _cpm_conout
        .global _getchar
        .global _setjmp
        .global _longjmp

_start:
        ld      sp, #0xFFFE

        ; Zero BSS via LDIR (symbols from cpm.ld)
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
        ; Parse the CP/M command tail (0x0080) into argc/argv and call main().
        ; _cpm_start_main (cpm_stdlib.c) does the tokenizing and calls main
        ; with the real argument vector, so programs that read argv[1] (e.g.
        ; an iteration count) behave like they do under dcc/zsdcc.
        call    _cpm_start_main

_cpm_exit:
        ld      c, #0
        call    0x0005
        halt

; -----------------------------------------------------------------------
; void cpm_conout(int c)  —  c in L
; -----------------------------------------------------------------------
_cpm_conout:
        ld      e, l
        ld      c, #2
        call    0x0005
        ret

; -----------------------------------------------------------------------
; int getchar(void)  —  BDOS fn 1, returns char zero-extended in DE
; (sdcccall returns 16-bit values in DE)
; -----------------------------------------------------------------------
_getchar:
        ld      c, #1
        call    0x0005
        ld      e, a
        ld      d, #0
        ret

; -----------------------------------------------------------------------
; setjmp / longjmp
;
; jmp_buf layout (8 bytes, little-endian words):
;   [0-1]  caller SP after setjmp returns
;   [2-3]  return PC
;   [4-5]  IX
;   (BC/DE/HL are scratch across the longjmp, like a normal call clobber)
;
; Non-reentrant scratch (single-threaded CP/M): _sj_sp / _sj_val.
; -----------------------------------------------------------------------
        .section .bss.crt0,"aw",@nobits
_sj_sp:         .space 2
_sj_val:        .space 2

        .section .text.crt0,"ax",@progbits
; int setjmp(jmp_buf env)  —  env in HL; returns 0 (direct) or val (via longjmp)
_setjmp:
        ld      (_sj_sp), sp            ; SP currently points at the return PC
        pop     de                      ; DE = return PC; SP = caller post-ret SP
        push    de                      ; put it back so our RET works

        ; env[0-1] = caller SP = saved SP + 2
        ld      bc, (_sj_sp)
        inc     bc
        inc     bc                      ; BC = caller SP
        ld      (hl), c
        inc     hl
        ld      (hl), b
        inc     hl
        ; env[2-3] = return PC (DE)
        ld      (hl), e
        inc     hl
        ld      (hl), d
        inc     hl
        ; env[4-5] = IX
        push    ix
        pop     bc                      ; BC = IX
        ld      (hl), c
        inc     hl
        ld      (hl), b

        ld      de, #0                  ; setjmp direct return = 0 (retval in DE)
        ret

; void longjmp(jmp_buf env, int val)  —  env in HL, val in DE
_longjmp:
        ld      a, d
        or      e
        jr      nz, .lj_ok
        ld      de, #1                  ; longjmp(env,0) must look like 1
.lj_ok:
        ld      (_sj_val), de           ; stash return value

        ; Restore IX from env[4-5]
        push    hl                      ; save env base
        ld      de, #4
        add     hl, de                  ; HL = &env[4]
        ld      a, (hl)                 ; IX lo
        inc     hl
        ld      h, (hl)                 ; IX hi
        ld      l, a                    ; HL = IX value
        push    hl
        pop     ix
        pop     hl                      ; HL = env base

        ; env[0-1] -> DE (saved SP), env[2-3] -> BC (return PC)
        ld      e, (hl)
        inc     hl
        ld      d, (hl)
        inc     hl
        ld      c, (hl)
        inc     hl
        ld      b, (hl)

        ex      de, hl                  ; HL = saved SP
        ld      sp, hl                  ; restore SP
        push    bc                      ; push return PC for RET
        ld      de, (_sj_val)           ; DE = return value (sdcccall returns in DE)
        ret
