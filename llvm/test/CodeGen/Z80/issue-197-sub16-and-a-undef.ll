; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs %s -o - | FileCheck %s
;
; ravn/llvm-z80 #197: 16-bit subtract lowers to `AND A; SBC HL,rr` (the AND A
; clears carry before the 16-bit subtract).  AND A reads A only as a flag
; vehicle -- A is unchanged and, here, dead -- so its $a read is a don't-care.
; The expansion now marks that read undef when A is dead, so this verifies
; clean; previously -verify-machineinstrs aborted with "Using an undefined
; physical register" on the AND A.  The verify-clean run (llc aborts otherwise)
; is the assertion; metadata-only, the emitted code is unchanged.
;
; C source:
;   typedef unsigned short uint16_t;
;   uint16_t sub16(uint16_t a, uint16_t b) { return a - b; }
; i16 subtraction lowers to AND A; SBC HL,rr where A is dead.
; Before the fix: -verify-machineinstrs aborted "Using undefined $a".

define dso_local i16 @sub16(i16 %a, i16 %b) {
  %r = sub i16 %a, %b
  ret i16 %r
}

; CHECK-LABEL: _sub16:
; CHECK: and a
; CHECK: sbc hl,
