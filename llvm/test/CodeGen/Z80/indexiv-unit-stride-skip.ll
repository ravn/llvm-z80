; RUN: opt -mtriple=z80 -passes='loop(z80-indexiv)' -S < %s | FileCheck %s

; Unit-stride pointer loops must NOT be rewritten to Base+uglygep: plain
; INC HL / INC DE is at least as cheap and avoids an extra live 16-bit value
; that would force the register allocator to spill register-pair arguments.
; narrowIV still runs and narrows the loop counter from i16 to i8.

; --- Case 1: two pointer loop, step=1 ---
; compare_6bytes pattern: walk two byte arrays in lock-step.
; No uglygep should appear; the counter should narrow to i8.

; CHECK-LABEL: define i8 @compare_unit_stride(
; CHECK-NOT:   uglygep
; CHECK:       phi i8
define i8 @compare_unit_stride(ptr %a, ptr %b) {
entry:
  br label %loop
loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %cont ]
  %ap = getelementptr i8, ptr %a, i16 %i
  %bp = getelementptr i8, ptr %b, i16 %i
  %va = load i8, ptr %ap, !tbaa !0
  %vb = load i8, ptr %bp, !tbaa !0
  %ne = icmp ne i8 %va, %vb
  br i1 %ne, label %diff, label %cont
cont:
  %i.next = add nuw nsw i16 %i, 1
  %done = icmp eq i16 %i.next, 6
  br i1 %done, label %exit, label %loop
diff:
  ret i8 1
exit:
  ret i8 0
}

; --- Case 2: single pointer loop, step=1 ---
; Memset-like pattern: write to one array.  Still no uglygep.

; CHECK-LABEL: define void @fill_unit_stride(
; CHECK-NOT:   uglygep
define void @fill_unit_stride(ptr %dst, i8 %val) {
entry:
  br label %loop
loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %dp = getelementptr i8, ptr %dst, i16 %i
  store i8 %val, ptr %dp
  %i.next = add nuw nsw i16 %i, 1
  %done = icmp eq i16 %i.next, 16
  br i1 %done, label %exit, label %loop
exit:
  ret void
}

; --- Case 3: step=2, single pointer ---
; Non-unit stride IS profitable; uglygep should appear.

; CHECK-LABEL: define void @stride2(
; CHECK:       uglygep
define void @stride2(ptr %dst) {
entry:
  br label %loop
loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %dp = getelementptr i8, ptr %dst, i16 %i
  store volatile i8 0, ptr %dp
  %i.next = add nuw nsw i16 %i, 2
  %done = icmp eq i16 %i.next, 20
  br i1 %done, label %exit, label %loop
exit:
  ret void
}

!0 = !{!1, !1, i64 0}
!1 = !{!"omnipotent char", !2, i64 0}
!2 = !{!"Simple C/C++ TBAA"}
