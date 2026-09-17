// REQUIRES: z80-registered-target
// RUN: %clang_cc1 -triple z80 -O2 -S %s -o - | FileCheck %s
//
// ravn/llvm-z80#341: The in-memory INC/DEC / BIT-SET-RES peepholes in
// Z80PreEmitPeephole rewrite `ld a,(addr); {inc a,or K,and K,...}; ld
// (addr),a` into HL-based (`ld hl,addr; inc (hl)` / `set n,(hl)`).
// In an __attribute__((interrupt)) handler this introduces a new HL
// clobber AFTER PEI has already frozen the callee-saved spill set --
// so HL is never pushed and the interrupted code's HL is silently
// corrupted on RETI.
//
// The fix bails from these peepholes when the function has the
// "interrupt" attribute unless HL is already in the prologue push
// list. Result: the plain A-only sequence is emitted, which is safe
// because AF is always pushed.
//
// Companion lit test (from IR): llvm/test/CodeGen/Z80/issue-341-isr-inmem-incdec-clobber.ll

volatile unsigned char g;

__attribute__((interrupt)) void isr_incmem(void) {
    g++;
}
// CHECK-LABEL: _isr_incmem:
// CHECK:       push af
// CHECK-NOT:   ld hl,_g
// CHECK-NOT:   inc (hl)
// CHECK:       pop af
// CHECK-NEXT:  reti

__attribute__((interrupt)) void isr_decmem(void) {
    g--;
}
// CHECK-LABEL: _isr_decmem:
// CHECK:       push af
// CHECK-NOT:   ld hl,_g
// CHECK-NOT:   dec (hl)
// CHECK:       pop af
// CHECK-NEXT:  reti

__attribute__((interrupt)) void isr_setbit(void) {
    g |= 0x04;
}
// CHECK-LABEL: _isr_setbit:
// CHECK:       push af
// CHECK-NOT:   ld hl,_g
// CHECK-NOT:   set {{.}},(hl)
// CHECK:       pop af
// CHECK-NEXT:  reti

// Non-ISR control: the peephole must still fire, saving 3 B.
void nonisr_incmem(void) {
    g++;
}
// CHECK-LABEL: _nonisr_incmem:
// CHECK:       ld hl,_g
// CHECK-NEXT:  inc (hl)
// CHECK:       ret
