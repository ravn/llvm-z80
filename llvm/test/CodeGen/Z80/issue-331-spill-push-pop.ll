; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Issue #331: use direct PUSH/POP instead of a dynamic SP-relative stack frame
; for a callee-saved scratch value across a call.
;
; STATUS: the SP-relative-frame -> PUSH/POP peephole is PARKED as UNSOUND
; (2026-09-16).  A naive implementation converts a spill and its *first* reload
; to PUSH/POP but leaves any *later* SP-relative reload of the same frame slot
; reading a slot that was never written (PUSH allocates its own 2 bytes rather
; than writing the prologue-allocated slot), corrupting the value.  This hangs
; recursive code where a value is reloaded both before and after a call
; (ackermann).  Root-cause writeup:
;   tasks/issue331-sprelative-pushpop-unsound-2026-09-16.md
; Runtime detector (the oracle that would have caught it):
;   z80-utils/test-runner/testcases/clang/test_22_recursion.c  (hangs if a naive
;   #331 is reintroduced).
;
; This lit test pins the codegen that is already correct today: a 16-bit
; callee-saved value preserved across a call in a loop is spilled with PUSH/POP
; by the register allocator natively (no SP-relative frame slot needed), which
; is the shape #331 was ultimately meant to guarantee for the loop-counter case.

declare zeroext i8 @callee()

; Loop counter preserved across a call in the loop body (mirrors
; _fdc_read_result in autoload-in-c): the register allocator already keeps the
; 16-bit counter callee-saved via PUSH/POP, with no `add hl,sp` frame access.
; CHECK-LABEL: spill_bc_across_call:
; CHECK-NOT:   add	hl,sp
; CHECK:       push	bc
; CHECK:       call	_callee
; CHECK:       pop	bc
define void @spill_bc_across_call() {
entry:
  br label %loop

loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %v = call zeroext i8 @callee()
  %i.next = add i16 %i, 1
  %done = icmp eq i16 %i.next, 7
  br i1 %done, label %exit, label %loop

exit:
  ret void
}
