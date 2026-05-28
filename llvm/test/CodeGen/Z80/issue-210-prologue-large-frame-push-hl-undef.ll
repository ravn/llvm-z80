; RUN: llc -mtriple=z80 -O2 -stop-after=prologepilog -verify-machineinstrs %s -o - \
; RUN:   | FileCheck %s
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
