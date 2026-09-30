; SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
;
; f32 comparison helpers for the z80-unknown-none-z88dk target: GCC-named
; entry points (__cmpsf2/__gtsf2/__gesf2/__unordsf2/__cmpsf2_fast) with the
; sdcccall(0) ABI used by ravn/llvm-z80 when -z80-float-sdcccall0 is active.
;
; ABI (sdcccall(0)): both 32-bit float operands on the stack, in declared
; order (LHS pushed first/deepest, RHS pushed second/closest to the return
; address); caller cleans up (4 pops / add sp,8 after the call); result
; returned in HL (-1/0/+1 tri-state, GCC convention).
;
; Stack layout at entry (own retaddr at SP+0):
;   SP+2 .. SP+5  : LHS (byte0=LSB, byte3=MSB)
;   SP+6 .. SP+9  : RHS (byte0=LSB, byte3=MSB)
;
; __cmpsf2 / __eqsf2 / __nesf2 / __ltsf2 / __lesf2:
;   NaN → +1  (so ordered predicates comparing against 0 evaluate false)
; __gtsf2 / __gesf2:
;   NaN → -1  (so a>b / a>=b evaluate false)
; __unordsf2:
;   NaN in either → HL=1, else HL=0
; __cmpsf2_fast (fast-math, nnan guaranteed):
;   No NaN check; calls m32_compare directly.
;
; The actual magnitude comparison is delegated to m32_compare from z88dk's
; math32 library (always linked when this code is used).  m32_compare has no
; NaN awareness; the NaN short-circuit above ensures it is never called with
; a NaN operand.
;
; m32_compare entry: stack = RHS(4B) @ SP+2, LHS(4B) @ SP+6, own retaddr
; @ SP+0 (double-call-entered).  Exit: Z=equal, C=LHS<RHS, flags only,
; non-destructive on the stack; caller cleans up both stack slots after ret.

SECTION code_l_clang

PUBLIC ___cmpsf2
PUBLIC ___eqsf2
PUBLIC ___nesf2
PUBLIC ___ltsf2
PUBLIC ___lesf2
PUBLIC ___gtsf2
PUBLIC ___gesf2
PUBLIC ___unordsf2
PUBLIC ___cmpsf2_fast

EXTERN m32_compare

; ---------------------------------------------------------------------
; CheckNaN: is the 4-byte LSB-first IEEE-754 binary32 value at (HL) NaN?
;
; entry : HL = pointer to byte0 (LSB) of the float; stack untouched
; exit  : A = 1 if NaN, 0 otherwise.  HL/DE/BC clobbered.
; ---------------------------------------------------------------------
.CheckNaN
    ld b,(hl)
    inc hl
    ld c,(hl)              ; BC = mantissa[15:0]
    inc hl
    ld a,(hl)              ; A  = byte2
    inc hl
    ld d,(hl)              ; D  = byte3 (SEEEEEEE)
    ld e,a                 ; E  = byte2 (Emmmmmmm)
    sla e
    rl d                   ; D  = exponent[7:0]
    ld a,d
    cp $FF
    jr nz,CheckNaN_no
    ld a,e
    and $7F                ; mantissa[22:16]
    or c
    or b
    jr z,CheckNaN_no       ; exponent=$FF, mantissa=0 → Infinity
    ld a,1
    ret
.CheckNaN_no
    xor a
    ret

; ___cmpsf2 / ___eqsf2 / ___nesf2 / ___ltsf2 / ___lesf2
; NaN → +1
___cmpsf2:
___eqsf2:
___nesf2:
___ltsf2:
___lesf2:
    ld hl,2
    add hl,sp
    call CheckNaN
    or a
    jr nz,cmpsf2_nan_gt
    ld hl,6
    add hl,sp
    call CheckNaN
    or a
    jr nz,cmpsf2_nan_gt
    call m32_compare
    jr z,cmpsf2_eq
    jr c,cmpsf2_lt
    ld hl,1
    ret
.cmpsf2_lt
    ld hl,$FFFF
    ret
.cmpsf2_eq
    ld hl,0
    ret
.cmpsf2_nan_gt
    ld hl,1
    ret

; ___gtsf2 / ___gesf2
; NaN → -1
___gtsf2:
___gesf2:
    ld hl,2
    add hl,sp
    call CheckNaN
    or a
    jr nz,gtsf2_nan_lt
    ld hl,6
    add hl,sp
    call CheckNaN
    or a
    jr nz,gtsf2_nan_lt
    call m32_compare
    jr z,gtsf2_eq
    jr c,gtsf2_lt
    ld hl,1
    ret
.gtsf2_lt
    ld hl,$FFFF
    ret
.gtsf2_eq
    ld hl,0
    ret
.gtsf2_nan_lt
    ld hl,$FFFF
    ret

; ___unordsf2 — nonzero iff either operand is NaN
___unordsf2:
    ld hl,2
    add hl,sp
    call CheckNaN
    or a
    jr nz,unordsf2_yes
    ld hl,6
    add hl,sp
    call CheckNaN
    or a
    jr nz,unordsf2_yes
    ld hl,0
    ret
.unordsf2_yes
    ld hl,1
    ret

; ___cmpsf2_fast — fast-math (nnan), no NaN check
___cmpsf2_fast:
    call m32_compare
    jr z,cmpsf2_fast_eq
    jr c,cmpsf2_fast_lt
    ld hl,1
    ret
.cmpsf2_fast_lt
    ld hl,$FFFF
    ret
.cmpsf2_fast_eq
    ld hl,0
    ret
