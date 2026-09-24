; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Issue #331: use direct PUSH/POP instead of a dynamic SP-relative stack frame
; for a caller-saved scratch value across a call.
;
; STATUS 2026-09-17: FIXED by the SP-relative spill -> PUSH/POP peephole in
; Z80PreEmitPeephole::optimizeSPRelativeSpillToPushPop.  The peephole matches
; a 5-MI SP-relative spill sequence (`ld hl,K; add hl,sp; ld (hl),lo; inc hl;
; ld (hl),hi`) followed by exactly one CALL followed by the mirror 5-MI
; reload, guarded by:
;   - single-reader:  exactly ONE spill store and ONE reload load reference
;     the frame slot in the whole function (rejects the ackermann multi-reload
;     shape that made the parked draft unsound);
;   - CALL exactly once between spill and reload;
;   - only balanced PUSH/POP allowed between spill-end and reload-start (this
;     lets nested spills convert iteratively);
;   - no explicit SP write in the interval.
; Runtime guard against reintroducing an unsound variant:
;   z80-utils/test-runner/testcases/clang/test_22_recursion.c  (hangs if a
;   naive #331 is reintroduced -- see tasks/issue331-sprelative-pushpop-
;   unsound-2026-09-16.md).

; C source:
;   // ravn/llvm-z80#331: PEI places a loop counter on a dynamic SP-relative
;   // frame slot (LD HL,K; ADD HL,SP; LD (HL),lo/hi ... reload mirror).
;   // The #331 peephole in Z80PreEmitPeephole rewrites the spill+reload pair
;   // to PUSH/POP when: single-reader, exactly one CALL between them, balanced
;   // PUSH/POP between spill-end and reload-start, no explicit SP write.
;   //
;   // Mirrors _fdc_read_result in autoload-in-c: loop counter preserved across
;   // a call inside the loop body.
;   extern uint8_t callee(void);
;   uint16_t spill_bc_across_call(uint16_t n) {
;       uint16_t sum = 0;
;       for (uint16_t i = n; i != 0; i--) sum += callee();
;       return sum;
;   }
declare zeroext i8 @callee()

; Loop counter preserved across a call in the loop body (mirrors
; _fdc_read_result in autoload-in-c): PEI initially places the counter on a
; dynamic SP-relative frame slot; the #331 peephole then rewrites the spill
; and its unique matching reload to PUSH/POP around the CALL.
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
