; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs -z80-fuse-carry-chain=false %s -o - | FileCheck %s
;
; ravn/llvm-z80 #197: the carry/borrow-materialize idiom `SBC A,A` (A = -carry)
; reads A only to satisfy the encoding -- its result is independent of A's
; value, and A is overwritten.  When A is dead the read tripped
; -verify-machineinstrs "Using an undefined physical register".  The expansion
; now marks every emitted SBC A,A $a read undef.  A 32-bit subtract lowers
; through the 16-bit borrow-out path and emits SBC A,A with A dead; this
; verifies clean (llc aborts otherwise -- the assertion).  Metadata-only.
;
; NOTE: Z80FuseCarryChain is disabled here on purpose -- with it on, this s32
; shape threads the borrow in the flag and emits no SBC A,A at all (see
; fuse-carry-chain.ll).  This test still pins the undef-marking on the
; register-carry expansion path, which Z80FuseCarryChain leaves untouched
; whenever the terminal borrow is observed.

define dso_local i32 @s32(i32 %a, i32 %b) {
  %r = sub i32 %a, %b
  ret i32 %r
}

; CHECK-LABEL: _s32:
; CHECK: sbc a,a
