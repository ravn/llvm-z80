; RUN: llc -mtriple=z80 -O2 -stop-after=prologepilog -verify-machineinstrs \
; RUN:   -z80-enable-auto-static-stack=false %s -o - | FileCheck %s
;
; -z80-enable-auto-static-stack=false: this test exercises the dynamic IX-frame
; prologue, which auto-static-stack (#176, default on) would bypass by routing
; @f's locals to BSS.  The IX-frame path is still reached for opted-out / ISR /
; address-taken / stack-arg functions, so we pin it here.
;
; ravn/llvm-z80 #210/#197: the IX-frame large-frame prologue saves HL across the
; SP adjustment (PUSH HL; LD HL,-size; ADD HL,SP; LD SP,HL; restore HL from the
; frame).  For a function with no HL-parameter, HL is undef at entry, so the
; PUSH_HL read an undefined $hl and -verify-machineinstrs aborted.  The fix
; marks the $hl read undef when HL is not a live-in (mirroring the no-FP
; large-frame path).  This is metadata-only -- the save/restore is unchanged.
;
; @f has a large address-taken stack array (forcing an IX frame larger than the
; 4-PUSH threshold) and no parameters, so HL is dead at entry.

define dso_local void @f() {
  %p = alloca [48 x i8], align 1
  call void @sink(ptr nonnull %p)
  ret void
}
declare dso_local void @sink(ptr)

; CHECK-LABEL: name: f
; The large-frame prologue saves HL with a don't-care (undef) $hl read.
; CHECK: PUSH_HL implicit undef $hl
; CHECK: LD_SP_HL
