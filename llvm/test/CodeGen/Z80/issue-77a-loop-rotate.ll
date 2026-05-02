; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -disable-lsr -z80-loop-rotate < %s | FileCheck %s --check-prefix=ROT
; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -disable-lsr < %s | FileCheck %s --check-prefix=NOROT
;
; Issue #77a: head-test do-while-decrement loops shouldn't pay the
; cross-BB `or a` test that re-derives the Z flag the body's `dec` already
; produced.  The fix is loop rotation: move the test from the header into
; the latch, giving a single-BB self-loop where the back-edge branch reads
; the flags from the body's dec directly.
;
; LLVM core's LoopRotate is gated on Function::hasMinSize() at -Oz (see
; LoopRotation.cpp:72 — Threshold forced to 0 when minsize is set), so it
; refuses to rotate -Oz functions regardless of header size.  This file
; exercises the Z80 target-specific Z80LoopRotate pass that bypasses that
; gate by calling LoopRotation() directly with a non-zero threshold.
;
; The pass is gated behind `-z80-loop-rotate` (default off) until #97 (BC
; ping-pong in single-BB self-loops) is fixed — the rotated form exposes
; that coalescing failure and currently regresses size on cpnos-rom.  Once
; #97 closes the default should flip to true.

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

; With -z80-loop-rotate ON, the back-edge branch uses the Z flag from the
; body's `dec` directly — no re-test via `or a`.
; ROT-LABEL: countdown:
; ROT-NOT:   or{{[ \t]+}}a
; ROT:       jr{{[ \t]+}}nz,

; With -z80-loop-rotate OFF (default), the head-test shape survives: the
; loop header re-tests A with `or a` even though the body's dec already
; set Z.  This is the regression #77a will close once #97 unblocks the
; default flip.
; NOROT-LABEL: countdown:
; NOROT:       or{{[ \t]+}}a
