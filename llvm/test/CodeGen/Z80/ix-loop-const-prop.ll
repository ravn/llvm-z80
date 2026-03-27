; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 -mattr=+static-stack < %s | FileCheck %s

; Regression test for IX constant propagation bug (ravn/llvm-z80#17).
;
; When IX is allocatable (hasFP=false with static-stack), the register
; allocator may put a loop counter in IX.  The IX constant propagation
; peephole in Z80LateOptimization must NOT fold INC IX inside a loop
; body into the initial LD IX,0 — the delta is loop-variant, not constant.
;
; The bug replaced PUSH IX; POP HL (correct loop counter) with
; LD HL,1 (constant), creating an infinite loop.

declare void @use(i8 zeroext)

define void @ix_loop_counter(ptr %base, i8 %n) {
; CHECK-LABEL: _ix_loop_counter:
; The loop body must NOT contain 'ld hl,$0001' as the loop exit check.
; It should use push ix; pop hl or similar to get the actual counter.
; CHECK-NOT:   ld	hl,$0001
; CHECK:       ret
entry:
  %cmp = icmp eq i8 %n, 0
  br i1 %cmp, label %exit, label %loop

loop:
  %i = phi i8 [ 0, %entry ], [ %i.next, %loop ]
  %ptr = getelementptr i8, ptr %base, i8 %i
  %val = load i8, ptr %ptr
  call void @use(i8 %val)
  %i.next = add i8 %i, 1
  %done = icmp eq i8 %i.next, %n
  br i1 %done, label %exit, label %loop

exit:
  ret void
}

; Same test but with a fixed trip count (sizeof struct = 7, like fdc_cmd).
; This is the exact pattern from fdc_write_full_cmd.
@data = external global [7 x i8]

define void @ix_loop_fixed_7() {
; CHECK-LABEL: _ix_loop_fixed_7:
; Must not have ld hl,$0001 — that was the miscompile.
; CHECK-NOT:   ld	hl,$0001
; CHECK:       ret
entry:
  br label %loop

loop:
  %i = phi i8 [ 0, %entry ], [ %i.next, %loop ]
  %idx = zext i8 %i to i16
  %ptr = getelementptr i8, ptr @data, i16 %idx
  %val = load i8, ptr %ptr
  call void @use(i8 %val)
  %i.next = add i8 %i, 1
  %done = icmp eq i8 %i.next, 7
  br i1 %done, label %exit, label %loop

exit:
  ret void
}
