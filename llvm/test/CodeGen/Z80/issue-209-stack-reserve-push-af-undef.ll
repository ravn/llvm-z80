; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs -z80-enable-auto-static-stack=false < %s | FileCheck %s
; RUN: llc -mtriple=z80 -O2 -stop-after=prologepilog -z80-enable-auto-static-stack=false < %s | FileCheck %s --check-prefix=MIR
;
; -z80-enable-auto-static-stack=false: this test exercises the PUSH AF stack-space
; reservation in the dynamic frame prologue, which auto-static-stack (#176,
; default on) bypasses by routing @g's locals to BSS.  The reservation path is
; still reached for opted-out / ISR / address-taken / stack-arg functions.
;
; ravn/llvm-z80 #209 (verifier surface, #197): Z80FrameLowering reserves stack
; space with PUSH AF.  PUSH AF reads $a and $flags, but for stack reservation
; their values are don't-cares (only SP moves; PUSH does not modify them).  In
; a function whose argument arrives in HL, A is dead at the prologue, so
; -verify-machineinstrs reported "Using an undefined physical register" at the
; reservation PUSH AF.  The reservation now marks the implicit uses undef.
;
; @g has an address-taken local array (forces an IX frame + a 4-byte stack
; reservation) and its arg in HL, so A is dead at entry.  The first RUN line
; asserts the function passes -verify-machineinstrs; MIR asserts the undef.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

define dso_local i16 @g(i16 %0) {
  %2 = alloca [2 x i16], align 1
  store i16 %0, ptr %2, align 1
  %3 = add nsw i16 %0, 1
  %4 = getelementptr inbounds nuw i8, ptr %2, i16 2
  store i16 %3, ptr %4, align 1
  call void @sink(ptr nonnull %2)
  ret i16 0
}

declare void @sink(ptr)

; CHECK-LABEL: g:
; CHECK: push af

; MIR-LABEL: name: g
; MIR: PUSH_AF implicit undef $a, implicit undef $flags
