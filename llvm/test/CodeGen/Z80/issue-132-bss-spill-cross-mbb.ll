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
;
; C source:
;   typedef unsigned short uint16_t;
;   typedef unsigned char  uint8_t;
;
;   extern uint16_t target(void);
;
;   uint16_t retry(uint8_t t) {
;       /* Retry loop shape from cpnos-rom _snios_sndmsg_force: an 8-bit
;          counter `t` lives across a CALL, so with +static-frame it is
;          spilled to BSS in MBB_A (before the call) and reloaded in
;          MBB_B (after the fallthrough) for the decrement + back-edge.
;          The escape edge to `ret1` has MBB_A as its sole predecessor
;          and does not use the counter -> the slot is dead there.
;
;          The BSS-spill -> PUSH/POP peephole rewrites STORE -> PUSH AF
;          in MBB_A and LOAD -> POP AF in MBB_B, then prepends a
;          compensating `pop af` to the escape block (ravn/llvm-z80#138:
;          1 B vs 2 B for `inc sp; inc sp`, exploiting that AF is dead
;          at the escape since ret1 returns a constant). */
;       do {
;           if (target() != 0) return 1;
;       } while (--t != 0);
;       return 0;
;   }

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
; CHECK-NEXT:  dec	a
; CHECK-NEXT:  ld	b,a
; CHECK-NEXT:  jr	nz,.LBB0_1
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
