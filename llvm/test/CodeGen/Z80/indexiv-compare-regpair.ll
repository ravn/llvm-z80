; RUN: llc -mtriple=z80 -O2 -z80-static-frames -verify-machineinstrs < %s | FileCheck %s

; compare_6bytes-equivalent: walk two byte arrays and return 1 at first
; difference, 0 if equal.  With unit-stride Z80IndexIV skipped the loop body
; should keep both pointers in register pairs and not spill to an IX frame.

; CHECK-LABEL: _compare_unit_stride:
; CHECK-NOT:   push ix
; CHECK:       ret
define zeroext i8 @compare_unit_stride(ptr %a, ptr %b) {
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

!0 = !{!1, !1, i64 0}
!1 = !{!"omnipotent char", !2, i64 0}
!2 = !{!"Simple C/C++ TBAA"}
