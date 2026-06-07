; RUN: opt -S -passes=aggressive-instcombine -data-layout="e-m:o-p:16:8-i16:8-n8:16" < %s | FileCheck %s
;
; Soundness matrix for the icmp-narrowing-through-graph extension
; (ravn/llvm-z80#160 sound version).  An outside-graph icmp observes the
; FULL wide value of the in-graph operand it consumes, so the narrowing
; is sound only when BOTH operands' KnownBits fit in the narrow width.
; Without that, narrowing replaces an i16 comparison with an i8 one that
; loses the high byte — semantically different when high bits are set.
;
; Runtime witnesses: test-runner fixtures test_220 / test_221 / test_222.
;
; This file pins POST-FIX behavior under v1 (icmp path only, and-mask
; outside-user path NOT yet implemented):
;   @unproven_*  — graph side not provably narrow: icmp must stay wide
;                  (and as the icmp is then an inadmissible outside user,
;                  the whole expression graph stays wide too).
;   @proven_*    — graph side provably narrow via KnownBits: must narrow.
;   @samesign_*  — signed predicates with `samesign` flag: fit threshold
;                  tightens to NarrowBits-1 so the sign bit stays clear
;                  at the narrow width.
;   @two_icmps_* — multi-escape interactions.
;   @const_other_too_wide — control: constant > NarrowBits rejected.

; ===== Negatives: unproven graph side, NO narrowing =====

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

; Critical runtime witness: 261 == 5 is FALSE wide, would be TRUE if narrowed
; (test_220 / test_221).
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

; Graph operand on the RHS of the compare — same rejection.
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

; samesign signed predicate: the flag licenses the PREDICATE but doesn't
; relax the graph-side range — still unsound when the graph value isn't
; proven narrow.
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

; Variable-other path: other-side proven (and 255, single-use), graph
; side not proven — reject.
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
; for i8.  Reject.
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

; ===== Positives: proven graph side, narrowing fires =====

; Graph value = (x & 127) + 1, max 128.  Fits i8.  Narrows.
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

; Exact-fit boundary: and 255 → max 255 = i8 max.  Narrows.
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

; Variable-other proven (and 255, single-use), graph proven (and 127 + 1).
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

; ===== Signed samesign boundary: sign bit must be clear at narrow width =====

; Graph value proven <= 255 (fits i8 unsigned) but NOT < 128: at i8 the
; value can read negative (200 → −56), flipping the signed comparison
; even with samesign on the wide icmp.  Must NOT narrow.
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
; survives narrowing.
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

; Two escaping icmps, BOTH sides proven for one, graph side unproven —
; neither narrows.
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

; ===== Width generality: i32 → i16 =====

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

; ===== Control: constant other that does NOT fit is rejected =====

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
