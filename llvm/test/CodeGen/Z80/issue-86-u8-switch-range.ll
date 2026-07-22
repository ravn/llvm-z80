; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 < %s | FileCheck %s

; Issue #86: u8 switch range-check should use 8-bit CP, not 16-bit
; subtract.  GISel switch lowering widens the discriminator to i16
; for jump-table indexing BEFORE the bound check; the bound check
; then comes out as a 9-byte 16-bit subtract (LD DE,N; LD A,E; SUB L;
; LD A,D; SBC A,H; JR NC).  Z80LateOptimization rewrites it to
; CP N + JR NC (3 B), with the FLAGS-flip needed because CP and
; the 16-bit chain compute opposite-direction subtractions.

declare void @f01() ; declare void @f02() ; declare void @f04()
declare void @f02()
declare void @f04()
declare void @f05()
declare void @f08()
declare void @f09()
declare void @f0d()
declare void @f0e()
declare void @f1e()

define void @specc(i8 zeroext %c) {
entry:
  switch i8 %c, label %default [
    i8 1,  label %case01
    i8 2,  label %case02
    i8 4,  label %case04
    i8 5,  label %case05
    i8 8,  label %case08
    i8 9,  label %case09
    i8 13, label %case0d
    i8 14, label %case0e
    i8 30, label %case1e
  ]
case01: tail call void @f01() ret void
case02: tail call void @f02() ret void
case04: tail call void @f04() ret void
case05: tail call void @f05() ret void
case08: tail call void @f08() ret void
case09: tail call void @f09() ret void
case0d: tail call void @f0d() ret void
case0e: tail call void @f0e() ret void
case1e: tail call void @f1e() ret void
default: ret void
}
; CHECK-LABEL: _specc:
; The 16-bit subtract chain must NOT appear:
; CHECK-NOT:   sub l
; CHECK-NOT:   sbc a,h
; The bound check is `cp N` + a single carry branch (NC = out of range =
; take exit).  The switch range is 1..30 stride 1, so after `dec a` the
; offset is in [0,29] and the strict bound is `offset > 29 -> default`.
; That is `cp 30` (offset >= 30 -> NC -> default), NOT `cp 29`: the
; original peephole used `cp max_offset`, which wrongly sent offset 29
; (case 30) to the default block — the jump-table off-by-one fixed in
; Z80LateOptimization (CP_n Limit+1).  See bugs/switchbug.c.
; CHECK:       dec a
; CHECK:       cp 30
; CHECK:       {{ret|jr|jp}} nc
