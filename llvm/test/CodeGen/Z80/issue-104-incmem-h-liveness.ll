; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; ravn/llvm-z80#104 — the in-memory INC/DEC peephole rewrites
;   LD A,(addr); INC A; LD (addr),A    (3 insns, 6 bytes)
;     into
;   LD HL,addr; INC (HL)               (2 insns, 4 bytes)
;
; The rewrite clobbers H and L (the new LD HL,nn).  Previously the
; peephole skipped the H/L liveness check ("the pattern is specific
; enough"), so it would fire even when the surrounding code held a
; live value in H or L — corrupting that value.
;
; Reproducer: c1 is held in H across an inlined check_true body that
; expands to the inc-mem pattern.  Without the liveness fix, c1 is
; lost (test_18_short_circuit_goto_O1 returned 0x000C instead of
; 0x000F because H got the high byte of `_g`'s address instead of c1).

@g = dso_local global i8 0, align 1

; CHECK-LABEL: _hold_h_across_incmem:

; Two interleaved increments of @g.  The peephole could in principle
; fold each into LD HL,@g; INC (HL), but that destroys the value
; previously read from @g and held in a register across the second
; rewrite.  After the fix, the second INC must NOT use the
; LD HL,@g; INC (HL) form when the first read's value is still live.
; We assert that at least one of the increments stays in the
; LD A,(_g); INC A; LD (_g),A form so the held value isn't clobbered.
;
; CHECK:       ld	a,(_g)
; CHECK:       inc	a
; CHECK:       ld	(_g),a

define i16 @hold_h_across_incmem() {
entry:
  %v1 = load volatile i8, ptr @g, align 1
  %v1.inc = add i8 %v1, 1
  store volatile i8 %v1.inc, ptr @g, align 1

  ; Reload @g; the value should differ from %v1 because @g is volatile.
  %v2 = load volatile i8, ptr @g, align 1
  %v2.inc = add i8 %v2, 1
  store volatile i8 %v2.inc, ptr @g, align 1

  ; Combine v1 and v2 as a 16-bit return value, forcing both to
  ; stay live across the second store.
  %v1.ext = zext i8 %v1 to i16
  %v2.ext = zext i8 %v2 to i16
  %v1.shl = shl i16 %v1.ext, 8
  %r = or i16 %v1.shl, %v2.ext
  ret i16 %r
}
