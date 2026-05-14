; RUN: opt -S -passes=aggressive-instcombine -data-layout="e-m:o-p:16:8-i16:8-n8:16" < %s | FileCheck %s

; Regression test for ravn/llvm-z80#158 — TruncInstCombine refused to
; narrow expression graphs rooted at function arguments.  The walker
; bailed out at `if (!I) return false` when it reached a non-Instruction
; non-Constant operand (i.e., an Argument).  This blocked narrowing for
; chains whose only "root" was an i16 parameter — exactly the K&R-style
; u8 case on Z80 where `int = 16` so the ABI passes `uint8_t` parameters
; as `i16`.
;
; Fix: accept Argument as a leaf in buildTruncExpressionGraph and
; getMinBitWidth; emit an explicit trunc at the function entry in
; getReducedOperand.

; Simplest case: `(arg & 0xFF) << 1 | (arg & 0xFF) >> 7` then trunc to i8.
; This is a u8 ROTL by 1 in disguise.  After the fix, TruncInstCombine
; narrows the chain to i8; downstream InstCombine recognises fshl.i8.

define i8 @rotl1_through_arg(i16 %arg) {
; CHECK-LABEL: @rotl1_through_arg(
; CHECK: trunc i16 %arg to i8
; CHECK: shl i8
; CHECK: lshr i8
; CHECK: or i8
; CHECK-NOT: shl i16
; CHECK-NOT: lshr i16
  %m = and i16 %arg, 255
  %s = shl nuw nsw i16 %m, 1
  %r = lshr i16 %m, 7
  %o = or disjoint i16 %s, %r
  %t = trunc i16 %o to i8
  ret i8 %t
}

; Confirm the narrowed chain still works when the arg type is i32 (a more
; conventional "promoted" type on most targets — i16 is the Z80 case).
define i8 @rotl1_through_i32_arg(i32 %arg) {
; CHECK-LABEL: @rotl1_through_i32_arg(
; CHECK: trunc i32 %arg to i8
; CHECK: shl i8
; CHECK: lshr i8
; CHECK: or i8
; CHECK-NOT: shl i32
; CHECK-NOT: lshr i32
  %m = and i32 %arg, 255
  %s = shl nuw nsw i32 %m, 1
  %r = lshr i32 %m, 7
  %o = or disjoint i32 %s, %r
  %t = trunc i32 %o to i8
  ret i8 %t
}
