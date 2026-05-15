; RUN: opt -S -passes=aggressive-instcombine -data-layout="e-m:o-p:16:8-i16:8-n8:16" < %s | FileCheck %s

; Regression test for ravn/llvm-z80#162 path 2 (per-callee body peek).
;
; When the callee's entry block begins with `trunc iW %arg to iM` or
; the canonicalised `and iW %arg, 2^M - 1`, the high (W-M) bits of the
; corresponding caller argument are observably discarded.  At the call
; site, inject a synthetic `(zext (trunc V to iM) to iW)` bracket so
; the established TruncInstCombine narrowing engine can shrink the
; chain feeding V.
;
; Mirrors the K&R-into-K&R-call shape that aes256.c's `rj_sb_inv ->
; gf_mulinv` chain produces.  Cost gate from ravn/llvm-z80#164 still
; applies; the test runs in the default no-mtriple config where
; isZExtFree=false, so single-use args fire and multi-use args don't.

; --- Callee whose entry block begins with `trunc i16 -> i8`. ---
define i8 @trunc_callee(i16 %p) {
  %tr = trunc i16 %p to i8
  %r = add i8 %tr, 1
  ret i8 %r
}

; --- Callee whose entry block begins with `and i16, 255`
;     (the canonical form after InstCombine). ---
define i8 @and_mask_callee(i16 %p) {
  %m = and i16 %p, 255
  %icmp = icmp eq i16 %m, 0
  %r = zext i1 %icmp to i8
  ret i8 %r
}

; --- Callee that genuinely uses the full i16 (compared against an
;     out-of-i8 constant).  No narrow signal should be detected. ---
define i8 @wide_callee(i16 %p) {
  %cmp = icmp ult i16 %p, 1000
  %r = zext i1 %cmp to i8
  ret i8 %r
}

; --- Caller against trunc_callee: chain should narrow to i8. ---
define i8 @caller_trunc(i16 %x, i16 %y) {
; CHECK-LABEL: @caller_trunc(
; CHECK-DAG: trunc i16 %x to i8
; CHECK-DAG: trunc i16 %y to i8
; CHECK: add i8
; CHECK: shl i8
; CHECK: zext i8
; CHECK: call i8 @trunc_callee(
  %sum = add i16 %x, %y
  %sh = shl i16 %sum, 1
  %m = and i16 %sh, 255
  %r = call i8 @trunc_callee(i16 %m)
  ret i8 %r
}

; --- Caller against and_mask_callee: same outcome via the canonical
;     pattern. ---
define i8 @caller_and_mask(i16 %x) {
; CHECK-LABEL: @caller_and_mask(
; CHECK: trunc i16 %x to i8
; CHECK: shl i8
; CHECK: zext i8
; CHECK: call i8 @and_mask_callee(
  %sh = shl i16 %x, 1
  %m = and i16 %sh, 255
  %r = call i8 @and_mask_callee(i16 %m)
  ret i8 %r
}

; --- Caller against wide_callee: no peek match, chain stays i16. ---
define i8 @caller_wide(i16 %x) {
; CHECK-LABEL: @caller_wide(
; CHECK: %sh = shl i16 %x, 1
; CHECK: call i8 @wide_callee(i16 %sh)
; CHECK-NOT: trunc i16 %sh to i8
  %sh = shl i16 %x, 1
  %r = call i8 @wide_callee(i16 %sh)
  ret i8 %r
}

; --- Caller with declaration-only callee: must not crash, no peek. ---
declare i8 @extern_callee(i16)

define i8 @caller_decl(i16 %x) {
; CHECK-LABEL: @caller_decl(
; CHECK: call i8 @extern_callee(i16
; CHECK-NOT: trunc i16 %sh to i8
  %sh = shl i16 %x, 1
  %r = call i8 @extern_callee(i16 %sh)
  ret i8 %r
}

; --- Self-recursion: must not crash, no peek (the function being
;     processed and the callee are the same; avoiding self-narrowing
;     simplifies the analysis). ---
define i8 @self_recursive(i16 %x) {
; CHECK-LABEL: @self_recursive(
; CHECK: %m = and i16 %x, 255
; CHECK: icmp eq i16
  %m = and i16 %x, 255
  %cmp = icmp eq i16 %m, 0
  br i1 %cmp, label %done, label %rec
rec:
  %dec = sub i16 %x, 1
  %r = call i8 @self_recursive(i16 %dec)
  br label %done
done:
  %p = phi i8 [ 0, %0 ], [ %r, %rec ]
  ret i8 %p
}

; --- Multi-use arg with !isZExtFree (default no-mtriple): cost gate
;     should SUPPRESS the synthetic bracket, leaving the chain i16. ---
define i16 @caller_multi_use_blocked(i16 %x) {
; CHECK-LABEL: @caller_multi_use_blocked(
; CHECK: %m = and i16 %x, 255
; CHECK: call i8 @trunc_callee(i16 %m)
; CHECK: add i16 %m
  %m = and i16 %x, 255
  %r = call i8 @trunc_callee(i16 %m)
  %rz = zext i8 %r to i16
  %s = add i16 %m, %rz
  ret i16 %s
}
