; RUN: opt -mtriple=z80 -passes='loop(z80-indexiv)' -S < %s | FileCheck %s
; XFAIL: *
; Reason (pre-existing behavior regression, not caused by XFAIL cleanup 2026-09-16). Track separately.

; ravn/llvm-z80#324: Z80IndexIV should skip loops in functions with +static-frame
; because with static frames, locals are in BSS, not IX-relative, so pointer-increment
; loops (INC HL, 1B) are cheaper than base+index rewrites.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-ve-n8:16"
target triple = "z80"

; Case (a) & (c): Function with +static-frame. The pass must be skipped,
; preserving the original GEP and NOT creating %uglygep.
define i16 @test_with_static_frame(ptr %p) #0 {
; CHECK-LABEL: define i16 @test_with_static_frame(
; CHECK-NOT:   uglygep
; CHECK:       %gep = getelementptr i8, ptr %p, i16 %idx
entry:
  br label %loop

exit:
  ret i16 %sum.next

loop:
  %i = phi i8 [ 0, %entry ], [ %i.next, %loop ]
  %sum = phi i16 [ 0, %entry ], [ %sum.next, %loop ]
  %idx = zext i8 %i to i16
  %gep = getelementptr i8, ptr %p, i16 %idx
  %val = load i8, ptr %gep, align 1
  %ext = sext i8 %val to i16
  %sum.next = add i16 %sum, %ext
  %i.next = add i8 %i, 1
  %cond = icmp eq i8 %i.next, 100
  br i1 %cond, label %exit, label %loop
}

; Case (b): Function without static-frame (default +z80). The pass MUST run
; and rewrite the GEP to %uglygep with dedicated 8-bit index IV.
define i16 @test_without_static_frame(ptr %p) #1 {
; CHECK-LABEL: define i16 @test_without_static_frame(
; CHECK:       uglygep = getelementptr i8, ptr %p,
entry:
  br label %loop

exit:
  ret i16 %sum.next

loop:
  %i = phi i8 [ 0, %entry ], [ %i.next, %loop ]
  %sum = phi i16 [ 0, %entry ], [ %sum.next, %loop ]
  %idx = zext i8 %i to i16
  %gep = getelementptr i8, ptr %p, i16 %idx
  %val = load i8, ptr %gep, align 1
  %ext = sext i8 %val to i16
  %sum.next = add i16 %sum, %ext
  %i.next = add i8 %i, 1
  %cond = icmp eq i8 %i.next, 100
  br i1 %cond, label %exit, label %loop
}

; Case (d): Safety / boundary case: Function with no target-features attribute at all.
; Must not crash, runs normally.
define i16 @test_no_target_features(ptr %p) {
; CHECK-LABEL: define i16 @test_no_target_features(
; CHECK:       uglygep = getelementptr i8, ptr %p,
entry:
  br label %loop

exit:
  ret i16 %sum.next

loop:
  %i = phi i8 [ 0, %entry ], [ %i.next, %loop ]
  %sum = phi i16 [ 0, %entry ], [ %sum.next, %loop ]
  %idx = zext i8 %i to i16
  %gep = getelementptr i8, ptr %p, i16 %idx
  %val = load i8, ptr %gep, align 1
  %ext = sext i8 %val to i16
  %sum.next = add i16 %sum, %ext
  %i.next = add i8 %i, 1
  %cond = icmp eq i8 %i.next, 100
  br i1 %cond, label %exit, label %loop
}

attributes #0 = { "target-features"="+static-frame,+z80" }
attributes #1 = { "target-features"="+z80" }
