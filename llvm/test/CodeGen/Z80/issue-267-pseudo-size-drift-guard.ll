; RUN: llc -mtriple=z80 -O2 -z80-verify-inline-runtime-size < %s | FileCheck %s
; RUN: llc -mtriple=sm83 -O2 -z80-verify-inline-runtime-size < %s | FileCheck %s
;
; Drift guard for MUL8/DIV8/MOD8 and saturating-i8 pseudos that expand AFTER
; BranchRelaxation (same class as ravn/llvm-z80#267 / #240).
;
; -z80-verify-inline-runtime-size aborts llc if getInstSizeInBytes disagrees
; with the real post-expansion byte count, keeping the sizes honest in CI.
;
; Variable-shift pseudo sizes (SHL8_VAR/LSHR16_VAR/…) are covered by upstream
; llvm-z80/llvm-z80 commit 841d84e5e1d7 and are tested there.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

declare i8 @llvm.uadd.sat.i8(i8, i8)
declare i8 @llvm.usub.sat.i8(i8, i8)
declare i8 @llvm.sadd.sat.i8(i8, i8)
declare i8 @llvm.ssub.sat.i8(i8, i8)

define zeroext i8 @mul8(i8 zeroext %a, i8 zeroext %b) {
; CHECK-LABEL: mul8:
  %r = mul i8 %a, %b
  ret i8 %r
}

define zeroext i8 @udiv8(i8 zeroext %a, i8 zeroext %b) {
; CHECK-LABEL: udiv8:
  %r = udiv i8 %a, %b
  ret i8 %r
}

define zeroext i8 @umod8(i8 zeroext %a, i8 zeroext %b) {
; CHECK-LABEL: umod8:
  %r = urem i8 %a, %b
  ret i8 %r
}

define signext i8 @sdiv8(i8 signext %a, i8 signext %b) {
; CHECK-LABEL: sdiv8:
  %r = sdiv i8 %a, %b
  ret i8 %r
}

define signext i8 @smod8(i8 signext %a, i8 signext %b) {
; CHECK-LABEL: smod8:
  %r = srem i8 %a, %b
  ret i8 %r
}

define i8 @uaddsat8(i8 %a, i8 %b) {
; CHECK-LABEL: uaddsat8:
  %r = call i8 @llvm.uadd.sat.i8(i8 %a, i8 %b)
  ret i8 %r
}

define i8 @usubsat8(i8 %a, i8 %b) {
; CHECK-LABEL: usubsat8:
  %r = call i8 @llvm.usub.sat.i8(i8 %a, i8 %b)
  ret i8 %r
}

define i8 @saddsat8(i8 %a, i8 %b) {
; CHECK-LABEL: saddsat8:
  %r = call i8 @llvm.sadd.sat.i8(i8 %a, i8 %b)
  ret i8 %r
}

define i8 @ssubsat8(i8 %a, i8 %b) {
; CHECK-LABEL: ssubsat8:
  %r = call i8 @llvm.ssub.sat.i8(i8 %a, i8 %b)
  ret i8 %r
}
