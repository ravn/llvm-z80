; RUN: llc -mtriple=z80 -O2 -stop-after=prolog-epilog -verify-machineinstrs \
; RUN:   -z80-enable-auto-static-frame=false %s -o - | FileCheck %s
;
; -z80-enable-auto-static-frame=false: this test exercises the dynamic IX-frame
; prologue, which auto-static-frame (#176, default on) would bypass by routing
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
;
; C source:
;   void sink(void *);
;   void f(void) {
;       unsigned char arr[48];  /* large → LD HL,-48; ADD HL,SP; LD SP,HL prologue */
;       sink(arr);
;   }
; No parameters → HL is dead at entry. The large-frame prologue's PUSH HL
; (saves HL across the SP adjustment) read undef $hl. Before fix:
; -verify-machineinstrs aborted "Using an undefined physical register $hl".

define dso_local void @f() {
  %p = alloca [48 x i8], align 1
  call void @sink(ptr nonnull %p)
  ret void
}
declare dso_local void @sink(ptr)

; CHECK-LABEL: name: f
; HL is dead at entry (no parameters) so the large-frame prologue elides
; the save entirely -- no PUSH_HL of undef $hl is emitted; LD_SP_HL runs
; directly on the clobbered HL. The verifier is clean either way.
; CHECK:      LD_SP_HL
; CHECK-NOT:  PUSH_HL implicit undef $hl
