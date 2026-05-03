; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; ravn/llvm-z80#112 — pseudo expansion sites pass opcode 0 to BuildMI
; when GR16 sub-registers are IXH/IXL/IYH/IYL.
;
; Root cause (LSHR16 / ASHR16 expansion at Z80InstrInfo.cpp:1773-1774):
; the expansion calls `getSRLOpcode(getSubReg(reg, sub_hi))` which only
; knows the documented {A,B,C,D,E,H,L} 8-bit registers.  For an IY
; operand, `getSubReg(IY, sub_hi) == IYH`; `getSRLOpcode(IYH)` returns
; 0 (out of table); `BuildMI(..., TII.get(0))` produces a bare opcode-0
; MachineInstr (PHI=0) that the encoder rejects with "Unsupported
; instruction : <MCInst 0>".
;
; Fix: define `GR16NoIR` register class (BC, DE, HL only, no IX/IY)
; and switch LSHR16/ASHR16 pseudo operands to it.  When IY becomes
; un-reserved (#38) and regalloc could otherwise pick it, the class
; constraint forces BC/DE/HL by construction.
;
; This test exercises a normal LSHR via constant-shift IR.  With IY
; currently reserved, the codegen must emit `srl <hi>; rr <lo>` for
; some {BC,DE,HL} pair.  The test does not depend on which pair is
; chosen — only that the expansion produces well-formed instructions
; (i.e., the encoder doesn't crash).

; CHECK-LABEL: _lshr_chain:
; CHECK: srl
; CHECK-NEXT: rr
; CHECK: srl
; CHECK-NEXT: rr
; CHECK: srl
; CHECK-NEXT: rr
; CHECK: ret
define i16 @lshr_chain(i16 %x) {
  %r = lshr i16 %x, 3
  ret i16 %r
}

; CHECK-LABEL: _ashr_chain:
; CHECK: sra
; CHECK-NEXT: rr
; CHECK: sra
; CHECK-NEXT: rr
; CHECK: ret
define i16 @ashr_chain(i16 %x) {
  %r = ashr i16 %x, 2
  ret i16 %r
}
