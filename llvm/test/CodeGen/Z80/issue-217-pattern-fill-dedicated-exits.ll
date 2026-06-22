; RUN: opt -mtriple=z80 -passes=z80-pattern-fill-recognize -S < %s | FileCheck %s
;
; ravn/llvm-z80#217 -- Z80PatternFillRecognize calls llvm::deleteDeadLoop
; which has an upstream caller contract: L->hasDedicatedExits() must hold.
; Two sequential loops over the same array violate the contract -- loop1's
; unique exit IS loop2's header, and loop2 has a self-backedge predecessor
; outside loop1.  On asserts builds, the deleteDeadLoop call aborts; on
; non-asserts builds the original #182 fork-local rewrite (now dead behind
; the upstream assert) was masking the layering bug.
;
; The fix is caller-side: Z80PatternFillRecognize must form dedicated exit
; blocks before invoking deleteDeadLoop.
;
; Note: the existing issue-182-deletedeadloop-phi.ll test runs `llc -O1`,
; which never invokes Z80PatternFillRecognize (it's a middle-end IR pass,
; not a codegen pass); this test runs the pass explicitly via opt.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

@a = dso_local global [100 x i8] zeroinitializer, align 1

; CHECK-LABEL: define dso_local void @g()
; The first loop must be rewritten to a memset.pattern call (the
; pattern-fill recognizer fired and deleteDeadLoop succeeded).
; CHECK: call void @llvm.experimental.memset.pattern
; The second loop must survive intact (it loads + increments, not a
; constant fill, so it doesn't match the recognizer).
; CHECK: load i8
; CHECK: add i8

define dso_local void @g() {
entry:
  br label %loop1

loop1:
  %i1 = phi i16 [ 0, %entry ], [ %i1.next, %loop1 ]
  %gep1 = getelementptr i8, ptr @a, i16 %i1
  store i8 0, ptr %gep1, align 1
  %i1.next = add nuw nsw i16 %i1, 1
  %done1 = icmp eq i16 %i1.next, 100
  br i1 %done1, label %loop2, label %loop1

loop2:
  %i2 = phi i16 [ 0, %loop1 ], [ %i2.next, %loop2 ]
  %gep2 = getelementptr i8, ptr @a, i16 %i2
  %v = load i8, ptr %gep2, align 1
  %v.inc = add i8 %v, 1
  store i8 %v.inc, ptr %gep2, align 1
  %i2.next = add nuw nsw i16 %i2, 1
  %done2 = icmp eq i16 %i2.next, 100
  br i1 %done2, label %exit, label %loop2

exit:
  ret void
}
