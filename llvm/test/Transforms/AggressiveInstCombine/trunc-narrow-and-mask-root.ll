; RUN: opt -S -passes=aggressive-instcombine -data-layout="e-m:o-p:16:8-i16:8-n8:16" < %s | FileCheck %s

; Regression test for ravn/llvm-z80#163 + #164.
;
; #163: TruncInstCombine should treat `(and X, 2^M - 1)` as a synthetic
; trunc root, since InstCombine canonicalises `(zext (trunc X to iM) to
; iW)` to that form and the trunc-root engine can no longer reach the
; chain feeding X.
;
; #164: Cost gate.  Re-extension at every use of the and is not free on
; some targets (Z80: each zext i8 -> i16 costs ~2 bytes).  Fire only
; when zext is free OR the and has a single user.
;
; The default (no-mtriple) data-layout-only configuration here has
; isZExtFree == false (TargetTransformInfoImplBase default).  Combined
; with multiple uses below, this exercises the gated-off path.

; --- Single use: synthetic trunc root injected, chain narrowed to i8. ---
define i8 @single_use_narrows(i16 %x) {
; CHECK-LABEL: @single_use_narrows(
; CHECK: trunc i16 %x to i8
; CHECK: shl i8
; CHECK-NOT: and i16
; CHECK-NOT: shl i16
  %m = and i16 %x, 255
  %s = shl i16 %m, 1
  %t = trunc i16 %s to i8
  ret i8 %t
}

; --- Multi use, zext not free: cost gate suppresses synthetic root.
;     The and-mask remains, no synthetic trunc/zext bracket appears. ---
define i16 @multi_use_blocked(i16 %x) {
; CHECK-LABEL: @multi_use_blocked(
; CHECK: %m = and i16 %x, 255
; CHECK: add i16 %m
; CHECK: sub i16 %m
; CHECK-NOT: trunc i16 %x to i8
  %m = and i16 %x, 255
  %a = add i16 %m, 1
  %b = sub i16 %m, 1
  %s = add i16 %a, %b
  ret i16 %s
}

; --- Single use, but the chain feeding X has nothing to narrow.
;     The synthetic trunc rolls back; the and-mask survives unchanged.
;     (No-op semantically; verifies the rollback path doesn't perturb IR.) ---
define i16 @single_use_no_narrowing(i16 %x) {
; CHECK-LABEL: @single_use_no_narrowing(
; CHECK: %m = and i16 %x, 255
; CHECK: ret i16 %m
  %m = and i16 %x, 255
  ret i16 %m
}

; --- Non-mask constant: not a synthetic trunc root candidate. ---
define i16 @non_mask_constant(i16 %x) {
; CHECK-LABEL: @non_mask_constant(
; CHECK: %m = and i16 %x, 254
; CHECK-NOT: trunc
  %m = and i16 %x, 254
  ret i16 %m
}
