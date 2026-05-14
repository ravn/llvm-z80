; RUN: opt -S -passes=aggressive-instcombine -data-layout="e-m:o-p:16:8-i16:8-n8:16" < %s | FileCheck %s

; Regression test for ravn/llvm-z80#160 (residual after #158).
;
; TruncInstCombine bails when an in-graph instruction has an outside-graph
; user, except when that user is a ZExt/SExt that can be eliminated.  An
; ICmpInst against a constant that fits in the narrow type is also safely
; rewritable alongside the graph — fix extends the outside-user allowlist.
;
; The motivating case is the inlined AES `mc_loop` body where shared zext'd
; operands feed multiple xor chains each terminating in `icmp samesign ult
; i16 X, 128`.  Before the fix, the multi-use bailout left all i16 icmps
; un-narrowed.  After: every chain narrows to i8 alongside the trunc users.

; --- Single-icmp single-use baseline (already narrowed pre-#160 via
;     InstCombine's icmp folding; included as a sanity anchor). ---
define i8 @single_icmp_unsigned(i16 %a, i16 %b) {
; CHECK-LABEL: @single_icmp_unsigned(
; CHECK: trunc i16 %a to i8
; CHECK: trunc i16 %b to i8
; CHECK: xor i8
; CHECK: icmp samesign ult i8
; CHECK-NOT: i16
  %ma = and i16 %a, 255
  %mb = and i16 %b, 255
  %x  = xor i16 %ma, %mb
  %tr = trunc nuw i16 %x to i8
  %cmp = icmp samesign ult i16 %x, 128
  %t1 = shl i8 %tr, 1
  %t2 = xor i8 %t1, 27
  %sel = select i1 %cmp, i8 %t1, i8 %t2
  ret i8 %sel
}

; --- The #160 multi-use shape: shared zext'd i8 operands feeding several xor
;     chains, each consumed by both a trunc and an icmp against a constant
;     fitting in i8.  This faithfully mirrors inlined `mc_loop` shape from
;     aes256.c (K&R variant, post-#158).
;
;     Pre-#160 fix: ALL four icmps survived as `icmp samesign ult i16 X, 128`
;     because the multi-use bailout in getBestTruncatedType rejected the
;     graph at the inner xor node (it has both an in-graph trunc user AND an
;     out-of-graph icmp user).  Post-fix: all four narrow to i8 via the
;     new icmp-sink path; shared zext leaves narrow via the existing
;     ext-exception path.
define void @many_uses(i8 %a, i8 %b, i8 %c, i8 %d, ptr %out) {
; CHECK-LABEL: @many_uses(
; CHECK: xor i8
; CHECK: icmp samesign ult i8
; CHECK: icmp samesign ult i8
; CHECK: icmp samesign ult i8
; CHECK: icmp samesign ult i8
; CHECK-NOT: icmp samesign ult i16
  %za = zext i8 %a to i16
  %zb = zext i8 %b to i16
  %zc = zext i8 %c to i16
  %zd = zext i8 %d to i16

  %xab = xor i16 %za, %zb
  %xcd = xor i16 %zc, %zd
  %xac = xor i16 %za, %zc
  %xbd = xor i16 %zb, %zd

  %tab = trunc nuw i16 %xab to i8
  %cab = icmp samesign ult i16 %xab, 128
  %tcd = trunc nuw i16 %xcd to i8
  %ccd = icmp samesign ult i16 %xcd, 128
  %tac = trunc nuw i16 %xac to i8
  %cac = icmp samesign ult i16 %xac, 128
  %tbd = trunc nuw i16 %xbd to i8
  %cbd = icmp samesign ult i16 %xbd, 128

  %s1 = select i1 %cab, i8 %tab, i8 %tcd
  %s2 = select i1 %ccd, i8 %tac, i8 %tbd
  %s3 = select i1 %cac, i8 %s1, i8 %s2
  %s4 = select i1 %cbd, i8 %s3, i8 %tab
  store i8 %s4, ptr %out
  ret void
}

; --- Predicate guards: signed comparisons WITHOUT samesign must NOT narrow
;     (could change result when high byte was non-zero).  The icmp stays i16.
define i1 @signed_no_samesign(i16 %a, i16 %b) {
; CHECK-LABEL: @signed_no_samesign(
; CHECK: icmp slt i16
  %ma = and i16 %a, 255
  %mb = and i16 %b, 255
  %x  = xor i16 %ma, %mb
  %tr = trunc nuw i16 %x to i8
  %use = xor i8 %tr, 1
  %dummy = add i8 %use, 0       ; force tr's chain to materialise
  %cmp = icmp slt i16 %x, 64     ; signed, no samesign -> not safe to narrow
  ret i1 %cmp
}

; --- Predicate guards: constant too large for narrow type must NOT narrow.
;     `icmp ult i16 X, 300` cannot become `icmp ult i8 trunc(X), 300` because
;     300 doesn't fit in i8.
define i1 @const_too_large(i16 %a, i16 %b) {
; CHECK-LABEL: @const_too_large(
; CHECK: icmp ult i16
  %ma = and i16 %a, 255
  %mb = and i16 %b, 255
  %x  = xor i16 %ma, %mb
  %tr = trunc nuw i16 %x to i8
  %use = xor i8 %tr, 1
  %dummy = add i8 %use, 0
  %cmp = icmp ult i16 %x, 300    ; 300 > 255 = max i8 unsigned -> not safe
  ret i1 %cmp
}

; --- Equality narrowing: eq/ne with fits-in-narrow constants are always safe.
define i1 @eq_narrowable(i16 %a, i16 %b) {
; CHECK-LABEL: @eq_narrowable(
; CHECK: icmp eq i8
; CHECK-NOT: icmp eq i16
  %ma = and i16 %a, 255
  %mb = and i16 %b, 255
  %x  = xor i16 %ma, %mb
  %tr = trunc nuw i16 %x to i8
  %use = xor i8 %tr, 1
  %dummy = add i8 %use, 0
  %cmp = icmp eq i16 %x, 42
  ret i1 %cmp
}
