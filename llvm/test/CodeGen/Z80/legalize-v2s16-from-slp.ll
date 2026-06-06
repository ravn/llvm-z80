; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s

; Regression: SLPVectorizer can fold adjacent i16 multiplies into a
; <2 x i16> mul + 2 extractelement, even on a no-SIMD target like Z80.
; This used to crash GISel legalizer with:
;   fatal error: unable to legalize instruction:
;     %X:_(s16) = G_EXTRACT_VECTOR_ELT %V:_(<2 x s16>), %I:_(s16)
;     (in function: vec3_dot)
; surfaced post-upstream-merge 2b971123e3bd (2026-06-06) on the runtime
; test test_31_struct_ops.c::vec3_dot.  Z80LegalizerInfo now scalarizes
; G_LOAD/G_STORE/G_MUL on vector types and lowers G_EXTRACT_VECTOR_ELT
; (and G_INSERT_VECTOR_ELT for symmetry).  Result: three scalar i16
; multiplies via __mulhi3 + two adds, no vector instructions in the
; final asm.

target datalayout = "e-m:e-p:16:8-p1:8:8-p2:16:8-i16:8-i32:8-i64:8-f32:8-f64:8-a:8-n8:16-S8"
target triple = "z80"

%struct.Vec3 = type { i16, i16, i16 }

; CHECK-LABEL: _vec3_dot:
; CHECK: call{{.*}}___mulhi3
; CHECK: call{{.*}}___mulhi3
; CHECK: call{{.*}}___mulhi3
; CHECK-NOT: vector
; CHECK: ret
define dso_local zeroext i16 @vec3_dot(ptr noundef readonly captures(none) %a, ptr noundef readonly captures(none) %b) local_unnamed_addr {
entry:
  %0 = load i16, ptr %a, align 1
  %1 = load i16, ptr %b, align 1
  %mul = mul i16 %1, %0
  %y = getelementptr inbounds nuw i8, ptr %a, i16 2
  %y2 = getelementptr inbounds nuw i8, ptr %b, i16 2
  %2 = load <2 x i16>, ptr %y, align 1
  %3 = load <2 x i16>, ptr %y2, align 1
  %4 = mul <2 x i16> %3, %2
  %5 = extractelement <2 x i16> %4, i64 0
  %add = add i16 %5, %mul
  %6 = extractelement <2 x i16> %4, i64 1
  %add6 = add i16 %add, %6
  ret i16 %add6
}
