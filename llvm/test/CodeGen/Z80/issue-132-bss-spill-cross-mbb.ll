; RUN: llc -mtriple=z80 -mattr=+static-frame < %s | FileCheck %s
;
; The "Freestanding" module flag (see end of file) drives Z80NonReentrant into
; the closed-world proof — the same flag clang emits under -ffreestanding.
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
; compensating `pop af` (ravn/llvm-z80#138) prepended to MBB_C to balance SP.

declare i16 @target()

; Per ravn/llvm-z80#138: comp uses `pop af` (1 B) when AF is dead at
; the escape, instead of `inc sp; inc sp` (2 B).  Here %ret1 returns
; a constant so A and FLAGS are unused; the fast comp form applies.
; CHECK-LABEL: retry:
; CHECK:       ld	b,a
; CHECK-LABEL: .LBB0_1:
; CHECK:       push	af
; CHECK-NEXT:  call	_target
; CHECK:       jr	nz,.LBB0_4
; CHECK-LABEL: ; %bb.2:
; CHECK:       pop	af
; CHECK-NEXT:  djnz	.LBB0_1
; CHECK:       ld	de,0
; CHECK-NEXT:  ret
; CHECK-LABEL: .LBB0_4:
; CHECK-NEXT:  pop	af
; CHECK-NEXT:  ld	de,1
; CHECK-NEXT:  ret
; CHECK-NOT:   (L_retry.frame)
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


!llvm.module.flags = !{!0}
!0 = !{i32 2, !"Freestanding", i32 1}
