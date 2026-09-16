; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Issue #60 (immediate form): redundant `LD A,imm` when A already
; holds the value.  Z80LateOptimization's known-immediate-A
; peephole carries A's value across non-A-clobbering instructions
; (LD HL,...; LD (nn),HL; arithmetic on HL/DE/BC; etc.) and
; deletes redundant subsequent `LD A,imm` of the same value.

@a = global i8 0
@b = global i8 0
@c = global i8 0
@dst = global i16 0

; CHECK-LABEL: zero_three:
; CHECK:      	xor	a
; CHECK:      	ld	(_a),a
; CHECK:      	ld	(_b),a
; CHECK:      	ld	(_c),a
; CHECK:      	ret
define void @zero_three() {
  store volatile i8 0, ptr @a
  store volatile i8 0, ptr @b
  store volatile i8 0, ptr @c
  ret void
}

; Three consecutive zero stores -- A=0 established once via xor a,
; carried across both inter-store branches.

; CHECK-LABEL: zero_then_inc_hl:
; CHECK:      	ld	bc,#_dst
; CHECK:      	inc	bc
; CHECK:      	xor	a
; CHECK:      	ld	(_a),a
; CHECK:      	ld	(_dst),bc
; CHECK:      	ld	(_b),a
; CHECK:      	ret
define void @zero_then_inc_hl() {
  store volatile i8 0, ptr @a
  %p = ptrtoint ptr @dst to i16
  %q = add i16 %p, 1
  store volatile i16 %q, ptr @dst
  store volatile i8 0, ptr @b   ; A should still be 0 from step 1
  ret void
}

; HL manipulation between two A=0 stores -- HL ops don't touch A,
; so no re-zero of A required.
