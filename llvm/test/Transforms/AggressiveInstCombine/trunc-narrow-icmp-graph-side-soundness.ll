; RUN: opt -S -passes=aggressive-instcombine -data-layout="e-m:o-p:16:8-i16:8-n8:16" < %s | FileCheck %s
;
; Soundness gate for the #160/#165 icmp-narrowing-through-graph extensions:
; an icmp consumes the FULL wide value of the in-graph operand, so narrowing
; the icmp is sound only when the GRAPH-SIDE value is also provably narrow
; (KnownBits fits the destination type) — not just the "other" operand.
;
; Runtime witnesses: test-runner fixtures test_220/221/222 (e.g. t = 261,
; trunc8 = 5: `261 < 10` is false but `5 < 10` is true).
;
; This file pins POST-FIX behavior:
;   @unproven_*  — graph side NOT provably narrow: the icmp must NOT be
;                  narrowed, and (it being an inadmissible outside user)
;                  the whole graph must stay wide.
;   @proven_*    — graph side provably narrow: must STILL narrow (no
;                  over-restriction).
;   @andmask_*   — the and-mask outside-user path consumes only low bits,
;                  so it needs NO graph-side proof: must still narrow even
;                  with an unproven graph value.

; ===== Negatives: unproven graph side -> NO narrowing =====

; #160 constant-other path, ult, graph on LHS.
define i8 @unproven_ult_const(i16 %x) {
; CHECK-LABEL: @unproven_ult_const(
; CHECK: add i16
; CHECK: icmp ult i16
; CHECK-NOT: icmp ult i8
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %c = icmp ult i16 %t, 10
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; #160, ugt.
define i8 @unproven_ugt_const(i16 %x) {
; CHECK-LABEL: @unproven_ugt_const(
; CHECK: icmp ugt i16
; CHECK-NOT: icmp ugt i8
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %c = icmp ugt i16 %t, 10
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; #160, ule.
define i8 @unproven_ule_const(i16 %x) {
; CHECK-LABEL: @unproven_ule_const(
; CHECK: icmp ule i16
; CHECK-NOT: icmp ule i8
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %c = icmp ule i16 %t, 10
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; #160, uge.
define i8 @unproven_uge_const(i16 %x) {
; CHECK-LABEL: @unproven_uge_const(
; CHECK: icmp uge i16
; CHECK-NOT: icmp uge i8
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %c = icmp uge i16 %t, 10
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; #160, eq (261 == 5 is false wide, true narrowed).
define i8 @unproven_eq_const(i16 %x) {
; CHECK-LABEL: @unproven_eq_const(
; CHECK: icmp eq i16
; CHECK-NOT: icmp eq i8
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %c = icmp eq i16 %t, 5
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; #160, ne.
define i8 @unproven_ne_const(i16 %x) {
; CHECK-LABEL: @unproven_ne_const(
; CHECK: icmp ne i16
; CHECK-NOT: icmp ne i8
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %c = icmp ne i16 %t, 5
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; #160, graph operand on the RHS of the compare.
define i8 @unproven_ult_const_graph_rhs(i16 %x) {
; CHECK-LABEL: @unproven_ult_const_graph_rhs(
; CHECK: icmp ult i16
; CHECK-NOT: icmp ult i8
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %c = icmp ult i16 10, %t
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; #160, samesign signed predicate: the flag licenses the PREDICATE, not the
; graph-side range — still unsound when the graph value doesn't fit.
define i8 @unproven_slt_samesign_const(i16 %x) {
; CHECK-LABEL: @unproven_slt_samesign_const(
; CHECK: icmp samesign slt i16
; CHECK-NOT: icmp samesign slt i8
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %c = icmp samesign slt i16 %t, 10
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; #165 variable-other path, ult: other side proven narrow (and 255,
; single-use), graph side NOT — must not narrow.
define i8 @unproven_ult_var(i16 %x, i16 %y) {
; CHECK-LABEL: @unproven_ult_var(
; CHECK: add i16
; CHECK: icmp ult i16
; CHECK-NOT: icmp ult i8
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %ym = and i16 %y, 255
  %c = icmp ult i16 %t, %ym
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; #165, eq variant.
define i8 @unproven_eq_var(i16 %x, i16 %y) {
; CHECK-LABEL: @unproven_eq_var(
; CHECK: icmp eq i16
; CHECK-NOT: icmp eq i8
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %ym = and i16 %y, 255
  %c = icmp eq i16 %t, %ym
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; Boundary: graph side provably fits 9 bits (and 511) — one bit too many
; for i8; must not narrow.
define i8 @unproven_boundary_9bit(i16 %x) {
; CHECK-LABEL: @unproven_boundary_9bit(
; CHECK: icmp ult i16
; CHECK-NOT: icmp ult i8
  %m = and i16 %x, 511
  %r = trunc i16 %m to i8
  %c = icmp ult i16 %m, 10
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; ===== Positives: proven graph side -> narrowing must still fire =====

; #160: graph value = (x & 127) + 1, max 128, fits i8 -> narrow.
define i8 @proven_ult_const(i16 %x) {
; CHECK-LABEL: @proven_ult_const(
; CHECK: add i8
; CHECK: icmp ult i8
; CHECK-NOT: icmp ult i16
  %m = and i16 %x, 127
  %t = add i16 %m, 1
  %r = trunc i16 %t to i8
  %c = icmp ult i16 %t, 10
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; #160: exact-fit boundary, graph value = x & 255, max 255 = i8 max -> narrow.
define i8 @proven_exact_fit(i16 %x) {
; CHECK-LABEL: @proven_exact_fit(
; CHECK: icmp ult i8
; CHECK-NOT: icmp ult i16
  %m = and i16 %x, 255
  %r = trunc i16 %m to i8
  %c = icmp ult i16 %m, 10
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; #165: both sides proven (graph masked 127+1, other masked 255 single-use).
define i8 @proven_ult_var(i16 %x, i16 %y) {
; CHECK-LABEL: @proven_ult_var(
; CHECK: add i8
; CHECK: icmp ult i8
; CHECK-NOT: icmp ult i16
  %m = and i16 %x, 127
  %t = add i16 %m, 1
  %r = trunc i16 %t to i8
  %ym = and i16 %y, 255
  %c = icmp ult i16 %t, %ym
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; ===== And-mask path: low-bits consumer, needs NO graph-side proof =====

; Unproven graph value, and-mask outside user: must STILL narrow (the mask
; consumes only low bits — sound by the same argument as the trunc root).
define i8 @andmask_unproven_still_narrows(i16 %x, ptr %p) {
; CHECK-LABEL: @andmask_unproven_still_narrows(
; CHECK: add i8
; CHECK: zext i8
; CHECK: store i16
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %a = and i16 %t, 15
  store i16 %a, ptr %p
  ret i8 %r
}

; ===== Signed/samesign boundary: signed needs the SIGN BIT clear =====

; Graph value proven <= 255 (fits i8 unsigned) but NOT < 128: at i8 the
; value can read negative (200 -> -56), flipping signed compares even with
; samesign on the wide icmp.  Must NOT narrow.
define i8 @samesign_fits8_not7(i16 %x) {
; CHECK-LABEL: @samesign_fits8_not7(
; CHECK: icmp samesign slt i16
; CHECK-NOT: icmp samesign slt i8
  %m = and i16 %x, 255
  %r = trunc i16 %m to i8
  %c = icmp samesign slt i16 %m, 10
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; Graph value proven < 128 (sign bit clear at i8): samesign signed compare
; survives narrowing.  Must STILL narrow post-fix.
define i8 @samesign_fits7(i16 %x) {
; CHECK-LABEL: @samesign_fits7(
; CHECK: icmp samesign slt i8
; CHECK-NOT: icmp samesign slt i16
  %m = and i16 %x, 127
  %r = trunc i16 %m to i8
  %c = icmp samesign slt i16 %m, 10
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; ===== Multi-escape interactions =====

; Two escaping icmps, BOTH sides proven for one, graph side unproven for
; the analysis as a whole (t unproven): neither may narrow; graph wide.
define i8 @two_icmps_one_unsafe(i16 %x) {
; CHECK-LABEL: @two_icmps_one_unsafe(
; CHECK: add i16
; CHECK: icmp ult i16
; CHECK: icmp eq i16
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %c1 = icmp ult i16 %t, 10
  %c2 = icmp eq i16 %t, 5
  %or = or i1 %c1, %c2
  %s = select i1 %or, i8 %r, i8 99
  ret i8 %s
}

; Two escaping icmps, graph value proven: both narrow together.
define i8 @two_icmps_proven(i16 %x) {
; CHECK-LABEL: @two_icmps_proven(
; CHECK: icmp ult i8
; CHECK: icmp eq i8
; CHECK-NOT: icmp ult i16
; CHECK-NOT: icmp eq i16
  %m = and i16 %x, 127
  %t = add i16 %m, 1
  %r = trunc i16 %t to i8
  %c1 = icmp ult i16 %t, 10
  %c2 = icmp eq i16 %t, 5
  %or = or i1 %c1, %c2
  %s = select i1 %or, i8 %r, i8 99
  ret i8 %s
}

; Mixed escapes: unproven graph value with BOTH an and-mask escape (sound
; alone) and an icmp escape (unsound).  The icmp is inadmissible -> the
; whole graph must stay wide, and-mask notwithstanding.
define i8 @mixed_andmask_icmp_unproven(i16 %x, ptr %p) {
; CHECK-LABEL: @mixed_andmask_icmp_unproven(
; CHECK: add i16
; CHECK: icmp ult i16
  %t = add i16 %x, 1
  %r = trunc i16 %t to i8
  %a = and i16 %t, 15
  store i16 %a, ptr %p
  %c = icmp ult i16 %t, 10
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; ===== Both icmp operands in-graph (rewrite-site fallback path) =====

; Both compare operands are in-graph values; with both proven narrow the
; compare may narrow alongside the graph.
define i8 @both_operands_in_graph_proven(i16 %x) {
; CHECK-LABEL: @both_operands_in_graph_proven(
; CHECK: icmp ult i8
; CHECK-NOT: icmp ult i16
  %m = and i16 %x, 127
  %t1 = add i16 %m, 1
  %t2 = xor i16 %m, 3
  %sum = add i16 %t1, %t2
  %r = trunc i16 %sum to i8
  %c = icmp ult i16 %t1, %t2
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; Both operands in-graph but unproven: must NOT narrow.
define i8 @both_operands_in_graph_unproven(i16 %x) {
; CHECK-LABEL: @both_operands_in_graph_unproven(
; CHECK: icmp ult i16
; CHECK-NOT: icmp ult i8
  %t1 = add i16 %x, 1
  %t2 = xor i16 %x, 3
  %sum = add i16 %t1, %t2
  %r = trunc i16 %sum to i8
  %c = icmp ult i16 %t1, %t2
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}

; ===== Width generality: i32 -> i16 =====

; Unproven graph side at i32->i16: must NOT narrow.
define i16 @unproven_ult_const_i32(i32 %x) {
; CHECK-LABEL: @unproven_ult_const_i32(
; CHECK: add i32
; CHECK: icmp ult i32
; CHECK-NOT: icmp ult i16
  %t = add i32 %x, 1
  %r = trunc i32 %t to i16
  %c = icmp ult i32 %t, 10
  %s = select i1 %c, i16 %r, i16 99
  ret i16 %s
}

; Proven graph side at i32->i16 (and 65535 exact fit): must narrow.
define i16 @proven_ult_const_i32(i32 %x) {
; CHECK-LABEL: @proven_ult_const_i32(
; CHECK: icmp ult i16
; CHECK-NOT: icmp ult i32
  %m = and i32 %x, 65535
  %r = trunc i32 %m to i16
  %c = icmp ult i32 %m, 10
  %s = select i1 %c, i16 %r, i16 99
  ret i16 %s
}

; ===== Control: constant other that does NOT fit is already rejected =====

define i8 @const_other_too_wide(i16 %x) {
; CHECK-LABEL: @const_other_too_wide(
; CHECK: icmp ult i16
; CHECK-NOT: icmp ult i8
  %m = and i16 %x, 255
  %r = trunc i16 %m to i8
  %c = icmp ult i16 %m, 300
  %s = select i1 %c, i8 %r, i8 99
  ret i8 %s
}
