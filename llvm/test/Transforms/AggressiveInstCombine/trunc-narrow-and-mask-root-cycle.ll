; RUN: opt -S -passes=aggressive-instcombine -data-layout="e-m:o-p:16:8-i16:8-n8:16" < %s | FileCheck %s

; Regression test for the use-after-free crash in the synthetic
; `(and X, 2^M - 1)` trunc-root path (ravn/llvm-z80#163/#164/#165).
;
; When X's def-use chain CYCLES back through the very `and` that is being
; treated as the synthetic trunc root, getBestTruncatedType (walking
; operands out of X) records that `and` as an in-graph node in
; InstInfoMap.  The injection path then erased the `and` BEFORE running
; ReduceExpressionGraph, leaving a dangling pointer in InstInfoMap that
; the reduction loop dereferenced -> segfault.
;
; Found by the stdcbench c90lib `add` function (an i16 induction
; recurrence `p = (p & 0xff) + 1`) under `-O2`/`-Os`.  The fix bails out
; of the synthetic-root injection when `InstInfoMap.count(And)` is true.
;
; The pass must not crash and, since the `and` cannot be safely narrowed
; here, must leave the recurrence untouched.

; CHECK-LABEL: @add_cycle(
; CHECK: %[[P:.*]] = phi i16 [ 0, %entry ], [ %[[N:.*]], %loop ]
; CHECK: %[[M:.*]] = and i16 %[[P]], 255
; CHECK: %[[N]] = add i16 %[[M]], 1
; CHECK-NOT: trunc
define i8 @add_cycle() {
entry:
  br label %loop

loop:
  %p = phi i16 [ 0, %entry ], [ %n, %loop ]
  %m = and i16 %p, 255
  %n = add i16 %m, 1
  br label %loop
}
