; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs %s -o - | FileCheck %s
;
; ravn/llvm-z80 #197: the block-splitting expansions of the 8-bit mul/div
; pseudos (MUL8, UDIV8, UMOD8) produced malformed MIR that -verify-machineinstrs
; rejected, two distinct ways:
;   1. The conditional body (ADD A,E / SUB E;INC D) was emitted inline after a
;      mid-block JR, with SkipMBB added as a successor twice -> duplicate
;      successor list entry (and naive dedup let branch-folding drop SkipMBB's
;      label).  Fixed by moving the body into its own AddMBB/SubMBB block so
;      LoopMBB ends with a real JR terminator and has two distinct successors.
;   2. The created loop blocks had no live-ins, so every loop-body instruction
;      read an undefined physreg.  Fixed by recomputing live-ins for the whole
;      function after expansion (Z80ExpandPseudo).
; Both are metadata-only -- the emitted instruction stream is unchanged.
;
; The test passes iff -verify-machineinstrs is clean (llc would abort otherwise)
; and each function emits its restoring/shift-add loop.

define dso_local zeroext i8 @mul8(i8 zeroext %a, i8 zeroext %b) {
  %r = mul i8 %a, %b
  ret i8 %r
}
; CHECK-LABEL: _mul8:
; CHECK: djnz

define dso_local zeroext i8 @udiv8(i8 zeroext %a, i8 zeroext %b) {
  %r = udiv i8 %a, %b
  ret i8 %r
}
; CHECK-LABEL: _udiv8:
; CHECK: djnz

define dso_local zeroext i8 @umod8(i8 zeroext %a, i8 zeroext %b) {
  %r = urem i8 %a, %b
  ret i8 %r
}
; CHECK-LABEL: _umod8:
; CHECK: djnz
