; RUN: opt -S -passes=aggressive-instcombine -data-layout="e-m:o-p:16:8-i16:8-n8:16" < %s | FileCheck %s

; Regression test for ravn/llvm-z80#165.
;
; Companion of #160: there, the outside-graph icmp user was allowed when one
; operand was a ConstantInt fitting the narrow type.  This test covers icmps
; where the non-graph operand is NOT a constant but is still provably narrow
; via KnownBits (e.g., `(and W, 2^M - 1)`).  Motivating shape: `gf_log`
; phi-loop in aes256.c.
;
; Pre-fix: `icmp eq i16 %g, %y` where %g is in-graph and %y is a narrowable
; sibling value bails because `canNarrowIcmpThroughGraph` requires a
; ConstantInt on one side.  Post-fix: %y narrows via fresh trunc at the icmp
; site, %g via the existing reduce path; icmp drops to i8.

; --- Single-use guard fires: when Y has only one use (the icmp), narrowing
;     is profitable — Y becomes truncated and can fold further. ---
define i1 @single_use_other(i16 %a, i16 %b) {
; CHECK-LABEL: @single_use_other(
; CHECK: trunc i16 %y to i8
; CHECK: icmp eq i8
; CHECK-NOT: icmp eq i16
  %ma = and i16 %a, 255
  %y  = and i16 %b, 255              ; narrowable witness, only used by icmp
  %tr = trunc nuw i16 %ma to i8
  %use = xor i8 %tr, 1
  %dummy = add i8 %use, 0
  %cmp = icmp eq i16 %ma, %y
  ret i1 %cmp
}

; --- Multi-use Y is blocked by the cost gate: extra trunc would be live
;     alongside the wide Y. ---
define void @multi_use_other(i16 %a, i16 %b, ptr %out) {
; CHECK-LABEL: @multi_use_other(
; CHECK: icmp eq i16
  %ma = and i16 %a, 255
  %y  = and i16 %b, 255              ; multi-use: feeds icmp AND store below
  %tr = trunc nuw i16 %ma to i8
  %use = xor i8 %tr, 1
  %dummy = add i8 %use, 0
  %cmp = icmp eq i16 %ma, %y
  store i16 %y, ptr %out             ; second use of %y -> blocks narrowing
  store i1 %cmp, ptr %out
  ret void
}

; --- Non-narrowable Y must NOT narrow.  KnownBits would prove ~%b possibly
;     has high bits set; we cannot truncate. ---
define i1 @non_narrowable_other(i16 %a, i16 %b) {
; CHECK-LABEL: @non_narrowable_other(
; CHECK: icmp eq i16
  %ma = and i16 %a, 255
  %tr = trunc nuw i16 %ma to i8
  %use = xor i8 %tr, 1
  %dummy = add i8 %use, 0
  %cmp = icmp eq i16 %ma, %b         ; %b has no narrowing witness
  ret i1 %cmp
}

; --- Signed predicate without samesign: same gate as #160; don't narrow. ---
define i1 @signed_no_samesign_narrowable(i16 %a, i16 %b) {
; CHECK-LABEL: @signed_no_samesign_narrowable(
; CHECK: icmp slt i16
  %ma = and i16 %a, 255
  %y  = and i16 %b, 255
  %tr = trunc nuw i16 %ma to i8
  %use = xor i8 %tr, 1
  %dummy = add i8 %use, 0
  %cmp = icmp slt i16 %ma, %y
  ret i1 %cmp
}

; --- Signed predicate WITH samesign: narrow alongside #160 contract. ---
define i1 @signed_samesign_narrowable(i16 %a, i16 %b) {
; CHECK-LABEL: @signed_samesign_narrowable(
; CHECK: trunc i16 %y to i8
; CHECK: icmp samesign slt i8
; CHECK-NOT: icmp samesign slt i16
  %ma = and i16 %a, 255
  %y  = and i16 %b, 255
  %tr = trunc nuw i16 %ma to i8
  %use = xor i8 %tr, 1
  %dummy = add i8 %use, 0
  %cmp = icmp samesign slt i16 %ma, %y
  ret i1 %cmp
}
