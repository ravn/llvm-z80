; RUN: llc -mtriple=z80-unknown-none-z88dk -stop-after=legalizer < %s | FileCheck %s
;
; Current test policy excludes NaNs. These checks cover finite-value
; predicates and ensure they use existing runtime entry points only.

define i1 @ordered_lt(float %lhs, float %rhs) {
; CHECK-LABEL: name: ordered_lt
; CHECK: CALL_nn &"\01cm32_sdcc___fslt"
  %cmp = fcmp olt float %lhs, %rhs
  ret i1 %cmp
}

define i1 @finite_ueq(float %lhs, float %rhs) {
; CHECK-LABEL: name: finite_ueq
; CHECK: CALL_nn &"\01cm32_sdcc___fseq"
  %cmp = fcmp ueq float %lhs, %rhs
  ret i1 %cmp
}

define i1 @finite_one(float %lhs, float %rhs) {
; CHECK-LABEL: name: finite_one
; CHECK: CALL_nn &"\01cm32_sdcc___fsneq"
  %cmp = fcmp one float %lhs, %rhs
  ret i1 %cmp
}

define i1 @finite_ogt(float %lhs, float %rhs) {
; CHECK-LABEL: name: finite_ogt
; CHECK: CALL_nn &"\01cm32_sdcc___fsgt"
  %cmp = fcmp ogt float %lhs, %rhs
  ret i1 %cmp
}

define i1 @ordered(float %lhs, float %rhs) {
; CHECK-LABEL: name: ordered
; CHECK: G_CONSTANT i8 1
  %cmp = fcmp ord float %lhs, %rhs
  ret i1 %cmp
}

define i1 @unordered(float %lhs, float %rhs) {
; CHECK-LABEL: name: unordered
; CHECK: G_CONSTANT i8 0
  %cmp = fcmp uno float %lhs, %rhs
  ret i1 %cmp
}
