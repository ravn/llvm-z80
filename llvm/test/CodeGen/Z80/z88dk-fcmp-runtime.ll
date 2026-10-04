; RUN: llc -verify-machineinstrs -mtriple=z80-unknown-none-z88dk -stop-after=legalizer < %s | FileCheck %s
; RUN: llc -verify-machineinstrs -mtriple=z80-unknown-none-z88dk < %s | FileCheck %s --check-prefix=ASM
;
; math32 predicates order NaN bit patterns like numbers. Strict comparisons
; must classify both inputs; only explicit nnan permits skipping this.
; C runtime counterpart: z88dk/test/clang/runtime_fcmp.c.

define i1 @ordered_lt(float %lhs, float %rhs) {
; CHECK-LABEL: name: ordered_lt
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fslt"
; CHECK: G_AND
; ASM-LABEL: _ordered_lt:
; ASM: call cm32_sdcc_fpclassify
; ASM: call cm32_sdcc_fpclassify
; ASM: call cm32_sdcc___fslt
  %cmp = fcmp olt float %lhs, %rhs
  ret i1 %cmp
}

define i1 @finite_ueq(float %lhs, float %rhs) {
; CHECK-LABEL: name: finite_ueq
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fseq"
; CHECK: G_OR
  %cmp = fcmp ueq float %lhs, %rhs
  ret i1 %cmp
}

define i1 @finite_one(float %lhs, float %rhs) {
; CHECK-LABEL: name: finite_one
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fsneq"
; CHECK: G_AND
  %cmp = fcmp one float %lhs, %rhs
  ret i1 %cmp
}

define i1 @finite_ogt(float %lhs, float %rhs) {
; CHECK-LABEL: name: finite_ogt
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fsgt"
; CHECK: G_AND
  %cmp = fcmp ogt float %lhs, %rhs
  ret i1 %cmp
}

define i1 @ordered(float %lhs, float %rhs) {
; CHECK-LABEL: name: ordered
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK-NOT: cm32_sdcc___fs
; CHECK: G_XOR
  %cmp = fcmp ord float %lhs, %rhs
  ret i1 %cmp
}

define i1 @unordered(float %lhs, float %rhs) {
; CHECK-LABEL: name: unordered
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK-NOT: cm32_sdcc___fs
; CHECK: G_OR
  %cmp = fcmp uno float %lhs, %rhs
  ret i1 %cmp
}

define i1 @nnan_ordered(float %lhs, float %rhs) {
; CHECK-LABEL: name: nnan_ordered
; CHECK-NOT: CALL_nn
; CHECK: G_CONSTANT i8 1
; CHECK-NOT: CALL_nn
; ASM-LABEL: _nnan_ordered:
; ASM-NOT: call
; ASM: ret
  %cmp = fcmp nnan ord float %lhs, %rhs
  ret i1 %cmp
}

define i1 @nnan_unordered(float %lhs, float %rhs) {
; CHECK-LABEL: name: nnan_unordered
; CHECK-NOT: CALL_nn
; CHECK: G_CONSTANT i8 0
; CHECK-NOT: CALL_nn
  %cmp = fcmp nnan uno float %lhs, %rhs
  ret i1 %cmp
}

define i1 @nnan_lt(float %lhs, float %rhs) {
; CHECK-LABEL: name: nnan_lt
; CHECK-NOT: cm32_sdcc_fpclassify
; CHECK: CALL_nn &"\01cm32_sdcc___fslt"
; CHECK-NOT: cm32_sdcc_fpclassify
  %cmp = fcmp nnan olt float %lhs, %rhs
  ret i1 %cmp
}

define i1 @ninf_unordered(float %lhs, float %rhs) {
; CHECK-LABEL: name: ninf_unordered
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
  %cmp = fcmp ninf uno float %lhs, %rhs
  ret i1 %cmp
}

define i1 @ordered_eq(float %lhs, float %rhs) {
; CHECK-LABEL: name: ordered_eq
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fseq"
; CHECK: G_AND
  %cmp = fcmp oeq float %lhs, %rhs
  ret i1 %cmp
}

define i1 @ordered_le(float %lhs, float %rhs) {
; CHECK-LABEL: name: ordered_le
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fsgt"
; CHECK: G_XOR
; CHECK: G_AND
  %cmp = fcmp ole float %lhs, %rhs
  ret i1 %cmp
}

define i1 @ordered_ge(float %lhs, float %rhs) {
; CHECK-LABEL: name: ordered_ge
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fslt"
; CHECK: G_XOR
; CHECK: G_AND
  %cmp = fcmp oge float %lhs, %rhs
  ret i1 %cmp
}

define i1 @unordered_ne(float %lhs, float %rhs) {
; CHECK-LABEL: name: unordered_ne
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fsneq"
; CHECK: G_OR
  %cmp = fcmp une float %lhs, %rhs
  ret i1 %cmp
}

define i1 @unordered_lt(float %lhs, float %rhs) {
; CHECK-LABEL: name: unordered_lt
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fslt"
; CHECK: G_OR
  %cmp = fcmp ult float %lhs, %rhs
  ret i1 %cmp
}

define i1 @unordered_le(float %lhs, float %rhs) {
; CHECK-LABEL: name: unordered_le
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fsgt"
; CHECK: G_XOR
; CHECK: G_OR
  %cmp = fcmp ule float %lhs, %rhs
  ret i1 %cmp
}

define i1 @unordered_gt(float %lhs, float %rhs) {
; CHECK-LABEL: name: unordered_gt
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fsgt"
; CHECK: G_OR
  %cmp = fcmp ugt float %lhs, %rhs
  ret i1 %cmp
}

define i1 @unordered_ge(float %lhs, float %rhs) {
; CHECK-LABEL: name: unordered_ge
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc_fpclassify"
; CHECK: CALL_nn &"\01cm32_sdcc___fslt"
; CHECK: G_XOR
; CHECK: G_OR
  %cmp = fcmp uge float %lhs, %rhs
  ret i1 %cmp
}

define i1 @nnan_ueq(float %lhs, float %rhs) {
; CHECK-LABEL: name: nnan_ueq
; CHECK-NOT: cm32_sdcc_fpclassify
; CHECK: CALL_nn &"\01cm32_sdcc___fseq"
; CHECK-NOT: cm32_sdcc_fpclassify
  %cmp = fcmp nnan ueq float %lhs, %rhs
  ret i1 %cmp
}
