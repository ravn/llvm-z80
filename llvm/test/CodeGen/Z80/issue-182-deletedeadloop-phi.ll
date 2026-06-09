; RUN: llc -mtriple=z80 -O1 < %s 2>&1 | FileCheck %s
;
; ravn/llvm-z80#182.  Upstream `deleteDeadLoop` (in
; llvm/lib/Transforms/Utils/LoopUtils.cpp) was malforming SSA on the
; exit block when the exit block had phi entries from outside the
; deleted loop -- specifically when the deleted loop's exit was the
; header of ANOTHER loop with its own backedge phi entries.  The
; original code kept phi entry 0 and removed everything else, but
; entry 0 might be the OTHER loop's backedge entry, not the
; exiting-block entry.  Result: self-referencing instructions
; (`%v = add %v, 1` outside any phi), which SCEV's `createSCEVIter`
; then walked into an unbounded worklist growth at LoopDeletionPass.
;
; This is reachable from clean source code: two sequential loops over
; the same array, where the first loop is rewritten to memcpy by
; `Z80PatternFillRecognize` and then deleted via `deleteDeadLoop`.

@a = dso_local global [100 x i8] zeroinitializer, align 1

; CHECK-LABEL: g:
; CHECK-NOT: SmallVector unable to grow

define dso_local void @g() {
entry:
  br label %loop1

loop1:                                          ; preds = %entry, %loop1
  %i1 = phi i16 [ 0, %entry ], [ %i1.next, %loop1 ]
  %gep1 = getelementptr i8, ptr @a, i16 %i1
  store i8 0, ptr %gep1, align 1
  %i1.next = add nuw nsw i16 %i1, 1
  %done1 = icmp eq i16 %i1.next, 100
  br i1 %done1, label %loop2, label %loop1

loop2:                                          ; preds = %loop1, %loop2
  %i2 = phi i16 [ 0, %loop1 ], [ %i2.next, %loop2 ]
  %gep2 = getelementptr i8, ptr @a, i16 %i2
  %v = load i8, ptr %gep2, align 1
  %v.inc = add i8 %v, 1
  store i8 %v.inc, ptr %gep2, align 1
  %i2.next = add nuw nsw i16 %i2, 1
  %done2 = icmp eq i16 %i2.next, 100
  br i1 %done2, label %exit, label %loop2

exit:                                           ; preds = %loop2
  ret void
}
