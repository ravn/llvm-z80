; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; ravn/llvm-z80#132: cross-MBB BSS-spill → PUSH/POP across CALLs,
; conservative single-predecessor-escape variant.
;
; Pattern (from cpnos-rom _snios_sndmsg_force retry loops):
;   MBB_A: STORE counter to sframe; CALL; conditional branch to escape;
;          fallthrough to MBB_B
;   MBB_B: LOAD counter from sframe; decrement; back-edge to MBB_A
;          (or fallthrough to MBB_exit)
;   MBB_C: escape target with MBB_A as sole predecessor; slot dead here
;
; Expected rewrite: STORE → PUSH AF in MBB_A, LOAD → POP AF in MBB_B,
; compensating `inc sp; inc sp` prepended to MBB_C to balance SP.

declare i16 @target()

; CHECK-LABEL: retry:
; CHECK:       push	af
; CHECK-NEXT:  call	_target
; CHECK:       pop	af
; CHECK:       dec	a
; CHECK-LABEL: %ret1
; CHECK-NEXT:  inc	sp
; CHECK-NEXT:  inc	sp
; CHECK-NEXT:  ld	de,1
; CHECK-NEXT:  ret
; CHECK-NOT:   ld	{{.*}}({{.*}}sfrend{{.*}})
; CHECK-NOT:   ld	({{.*}}sfrend{{.*}})
define i16 @retry(i8 %t) {
entry:
  br label %loop

loop:
  %t.phi = phi i8 [ %t, %entry ], [ %t.dec, %cont ]
  %r = call i16 @target()
  %nz = icmp ne i16 %r, 0
  br i1 %nz, label %ret1, label %cont

cont:
  %t.dec = add i8 %t.phi, -1
  %nzc = icmp ne i8 %t.dec, 0
  br i1 %nzc, label %loop, label %ret0

ret1:
  ret i16 1

ret0:
  ret i16 0
}
