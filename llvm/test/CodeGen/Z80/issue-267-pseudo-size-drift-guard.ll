; RUN: llc -mtriple=z80 -O2 -z80-verify-inline-runtime-size < %s | FileCheck %s
; RUN: llc -mtriple=sm83 -O2 -z80-verify-inline-runtime-size < %s | FileCheck %s
;
; #267 systemic drift guard for EVERY inline-runtime pseudo that expands AFTER
; BranchRelaxation (in Z80ExpandPseudo).  These pseudos are alive during
; BranchRelaxation, which sizes any branch jumping *over* them via
; getInstSizeInBytes; if a pseudo is sized 0 (or wrong) there, the pass
; under-counts and can leave a `jr` whose real post-expansion offset exceeds the
; +/-127 B limit -- accepted by the object emitter (silently relaxed to `jp`)
; but rejected by an external Z80 assembler (z88dk z80asm), which was the
; ravn/llvm-z80#267 defect.  The original #240 guard only covered the i16 MUL/
; DIV/MOD pseudos; this file extends the guard to the remaining expand-after-
; relaxation pseudos: MUL8, U/S DIV8/MOD8, the i8 saturating add/sub, and the
; variable-count 16-bit shifts.  (IDX8 needs -z80-idx-addr and is guarded in
; issue-27-iy-indexed-addr.ll; the GUARDED memcpy/memset variants are guarded in
; issue-105-ldir-guarded.ll via their own verify RUN lines.)
;
; -z80-verify-inline-runtime-size makes Z80ExpandPseudo sum the whole function's
; getInstSizeInBytes immediately before and after each such expansion and
; report_fatal_error if the reported pseudo size does not equal the real byte
; count of the expansion.  Change an expansion without updating
; getInstSizeInBytes and llc aborts here.

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

define i16 @shl16_var(i16 %a, i16 %b) {
; CHECK-LABEL: shl16_var:
  %r = shl i16 %a, %b
  ret i16 %r
}

define i16 @lshr16_var(i16 %a, i16 %b) {
; CHECK-LABEL: lshr16_var:
  %r = lshr i16 %a, %b
  ret i16 %r
}

define i16 @ashr16_var(i16 %a, i16 %b) {
; CHECK-LABEL: ashr16_var:
  %r = ashr i16 %a, %b
  ret i16 %r
}
