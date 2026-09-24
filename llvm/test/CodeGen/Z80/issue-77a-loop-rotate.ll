; RUN: llc -mtriple=z80 -O2 -disable-lsr -enable-z80-loop-rotate < %s | FileCheck %s --check-prefix=ROT
; RUN: llc -mtriple=z80 -O2 -disable-lsr < %s | FileCheck %s --check-prefix=NOROT
;
; Issue #77a: head-test do-while-decrement loops shouldn't pay the
; cross-BB `or a` test that re-derives the Z flag the body's `dec` already
; produced.  The fix is loop rotation: move the test from the header into
; the latch, giving a single-BB self-loop where the back-edge branch reads
; the flags from the body's dec directly.
;
; LLVM core's LoopRotate is gated on Function::hasMinSize() at -Oz (see
; LoopRotation.cpp:72 — Threshold forced to 0 when minsize is set), so it
; refuses to rotate -Oz functions regardless of header size.  The Z80
; target-specific Z80LoopRotate pass bypasses that gate by calling
; LoopRotation() directly with a non-zero threshold.
;
; Pass remains gated behind `-enable-z80-loop-rotate` (default off).
; #100 closed at the MIR layer via the post-RA BC-ping-pong peephole.
; Session 73m added CALL-skip + small-trip-count guards in this pass
; aiming to flip the default on; measurement showed cpnos -4 B and
; AES production-like -2.2% tstates, but AES -Oz baseline regressed
; +11% tstates (LICM-hoisted invariants in the duplicated header are
; suspected but not isolated).  Default stays off pending either a
; tighter guard or the peephole alternative for #77.  See the comment
; on `EnableZ80LoopRotate` in `Z80LoopRotate.cpp` for the numbers.

; C source:
;   typedef unsigned char uint8_t;
;   void f(uint8_t n) { while (n--) {} }
; Head-test loop: the `or a` re-derives Z from the counter after the loop body's
; `dec a` already set it. Loop rotation moves the test into the latch, eliminating
; the redundant `or a` and giving a self-looping single-BB structure.
; Pass gated behind -enable-z80-loop-rotate (default off pending measurement).

define void @countdown(ptr %p) {
entry:
  br label %loop

loop:
  %i = phi i8 [ 18, %entry ], [ %i.next, %loop.body ]
  %p.cur = phi ptr [ %p, %entry ], [ %p.next, %loop.body ]
  %done = icmp eq i8 %i, 0
  br i1 %done, label %exit, label %loop.body

loop.body:
  %p.next = getelementptr inbounds nuw i8, ptr %p.cur, i16 2
  store volatile i16 -4345, ptr %p.cur, align 1
  %i.next = sub i8 %i, 1
  br label %loop

exit:
  ret void
}

; CHECK-LABEL: countdown:
; CHECK:      	ld	d,18
; CHECK:      	ld	a,d
; CHECK:      	or	a
; CHECK:      	jr	z,.LBB0_3
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	inc	bc
; CHECK:      	inc	bc
; CHECK:      	ld	a,d
; CHECK:      	ld	de,61191
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	dec	a
; CHECK:      	ld	d,a
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	jr	.LBB0_1
; CHECK:      	ret
; With -enable-z80-loop-rotate ON, the back-edge branch uses the Z flag from the
; body's `dec` directly — no re-test via `or a`.
; ROT-LABEL: countdown:
; ROT-NOT:   or{{[ \t]+}}a
; ROT:       jr{{[ \t]+}}nz,

; With -enable-z80-loop-rotate OFF (default), the head-test shape survives: the
; loop header re-tests A with `or a` even though the body's dec already
; set Z.  This is the regression #77a will close once #97 unblocks the
; default flip.
; NOROT-LABEL: countdown:
; NOROT:       or{{[ \t]+}}a
