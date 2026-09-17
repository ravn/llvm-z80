; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs < %s | FileCheck %s
;
; ravn/llvm-z80#341: The in-memory INC/DEC peephole in Z80PreEmitPeephole
; (optimizeInMemoryIncDec, commit 354d14db1273) rewrites
;   ld a,(addr); inc a; ld (addr),a       ->   ld hl,addr; inc (hl)
; saving 3 bytes. In an __attribute__((interrupt)) handler this
; introduces a new HL clobber AFTER PEI has already frozen the
; callee-saved spill set — so HL is never pushed and the interrupted
; code's HL value is silently corrupted on RETI.
;
; The fix bails from the peephole when the function has the "interrupt"
; attribute unless HL is already in the prologue's saved-regs list.
; Result: emit the original `ld a,(addr); inc a; ld (addr),a` form,
; which uses only A (already saved by `push af`).

@g = global i8 0

define void @isr_incmem() #0 {
  %v = load volatile i8, ptr @g
  %w = add i8 %v, 1
  store volatile i8 %w, ptr @g
  ret void
}

; CHECK-LABEL: _isr_incmem:
; CHECK:       push af
; CHECK-NOT:   ld hl,_g
; CHECK-NOT:   inc (hl)
; Expected A-only fallback (safe because AF is pushed):
; CHECK:       ld a,(_g)
; CHECK-NEXT:  inc a
; CHECK-NEXT:  ld (_g),a
; CHECK-NEXT:  pop af
; CHECK-NEXT:  reti

; --- decrement variant: same shape ---
define void @isr_decmem() #0 {
  %v = load volatile i8, ptr @g
  %w = sub i8 %v, 1
  store volatile i8 %w, ptr @g
  ret void
}
; CHECK-LABEL: _isr_decmem:
; CHECK:       push af
; CHECK-NOT:   ld hl,_g
; CHECK-NOT:   dec (hl)
; CHECK:       ld a,(_g)
; CHECK-NEXT:  dec a
; CHECK-NEXT:  ld (_g),a
; CHECK-NEXT:  pop af
; CHECK-NEXT:  reti

; --- bit-set variant: same-class peephole optimizeInMemoryBitSetRes ---
define void @isr_setbit() #0 {
  %v = load volatile i8, ptr @g
  %w = or i8 %v, 4
  store volatile i8 %w, ptr @g
  ret void
}
; CHECK-LABEL: _isr_setbit:
; CHECK:       push af
; CHECK-NOT:   ld hl,_g
; CHECK-NOT:   set {{.}},(hl)
; CHECK:       ld a,(_g)
; CHECK-NEXT:  or 4
; CHECK-NEXT:  ld (_g),a
; CHECK-NEXT:  pop af
; CHECK-NEXT:  reti

; --- non-ISR control: peephole still fires (byte-shorter) ---
define void @nonisr_incmem() {
  %v = load volatile i8, ptr @g
  %w = add i8 %v, 1
  store volatile i8 %w, ptr @g
  ret void
}
; CHECK-LABEL: _nonisr_incmem:
; CHECK:       ld hl,_g
; CHECK-NEXT:  inc (hl)
; CHECK:       ret

attributes #0 = { "interrupt" }
