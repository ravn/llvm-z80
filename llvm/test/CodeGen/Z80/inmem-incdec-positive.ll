; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; Z80LateOptimization peephole: in-memory INC/DEC for global byte counters.
;
; Pattern (pre-peephole, 6B):
;     LD A, (addr)        ; load
;     INC A   / DEC A     ; increment / decrement
;     LD (addr), A        ; store
;
; Rewrite (post-peephole, 4B):
;     LD HL, addr
;     INC (HL) / DEC (HL)
;
; Safe when A is dead after the store and HL is not live across the
; window.  See Z80LateOptimization.cpp `In-memory INC/DEC`.
;
; This file covers the *positive* path — peephole fires on a clean
; global increment / decrement.  The companion test
; `issue-104-incmem-h-liveness.ll` covers the negative path (peephole
; correctly skips when H/L is live).

@counter = dso_local global i8 0, align 1

; Plain increment of a global byte; A is dead after the store.
define void @bump_counter() nounwind {
  %v = load i8, ptr @counter, align 1
  %v.inc = add i8 %v, 1
  store i8 %v.inc, ptr @counter, align 1
  ret void
}
; CHECK-LABEL: bump_counter:
; CHECK:      	ld	bc,#_counter
; CHECK:      	ld	a,(bc)
; CHECK:      	inc	a
; CHECK:      	ld	(bc),a
; CHECK:      	ret

; Decrement: same shape, DEC (HL) form.
define void @drop_counter() nounwind {
  %v = load i8, ptr @counter, align 1
  %v.dec = sub i8 %v, 1
  store i8 %v.dec, ptr @counter, align 1
  ret void
}

; Three independent counters in one function: peephole should fire on
; each of them because A is dead between sequences (the next sequence
; defines A with its own LD A,(addr2)).
@a = dso_local global i8 0, align 1
@b = dso_local global i8 0, align 1
; CHECK-LABEL: drop_counter:
; CHECK:      	ld	bc,#_counter
; CHECK:      	ld	a,(bc)
; CHECK:      	dec	a
; CHECK:      	ld	(bc),a
; CHECK:      	ret
@c = dso_local global i8 0, align 1

define void @bump_three() nounwind {
  %va = load i8, ptr @a, align 1
  %va.inc = add i8 %va, 1
  store i8 %va.inc, ptr @a, align 1
  %vb = load i8, ptr @b, align 1
  %vb.inc = add i8 %vb, 1
  store i8 %vb.inc, ptr @b, align 1
  %vc = load i8, ptr @c, align 1
  %vc.inc = add i8 %vc, 1
  store i8 %vc.inc, ptr @c, align 1
  ret void
}
; CHECK-LABEL: bump_three:
; CHECK:      	ld	bc,#_a
; CHECK:      	ld	a,(bc)
; CHECK:      	inc	a
; CHECK:      	ld	(bc),a
; CHECK:      	ld	bc,#_b
; CHECK:      	ld	a,(bc)
; CHECK:      	inc	a
; CHECK:      	ld	(bc),a
; CHECK:      	ld	bc,#_c
; CHECK:      	ld	a,(bc)
; CHECK:      	inc	a
; CHECK:      	ld	(bc),a
; CHECK:      	ret
