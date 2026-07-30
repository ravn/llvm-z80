; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s

; Regression: the jump-table upper-bound check must include the MAXIMUM case
; value (the last dense jump-table slot).  The Z80LateOptimization peephole that
; narrows the 16-bit switch-range subtract to an 8-bit `cp` used `cp Range`
; (offset >= Range -> default), which is `offset > Range-1` — off by one: it
; sent the last in-range index (the highest case value) to the default block.
; The correct bound is `offset > Range -> default`, i.e. `cp Range+1`.
;
; This switch has 10 dense cases (offsets [0,9]) so GISel builds a jump table
; and the narrowing peephole fires.  The bound must be `cp 10` (offset >= 10 ->
; default), NOT `cp 9`.  With `cp 9`, case 9 (@c9, the highest value) would be
; miscompiled into @def.  Repro origin: nanoprintf's conversion switch, where
; `%x` (the max char) silently failed.  See bugs/switchbug.c and
; tasks/bug-jumptable-upper-bound-offbyone.md.

declare void @c0()
declare void @c1()
declare void @c2()
declare void @c3()
declare void @c4()
declare void @c5()
declare void @c6()
declare void @c7()
declare void @c8()
declare void @c9()
declare void @def()

define void @dispatch(i8 zeroext %x) {
entry:
  switch i8 %x, label %L_def [
    i8 0, label %L0
    i8 1, label %L1
    i8 2, label %L2
    i8 3, label %L3
    i8 4, label %L4
    i8 5, label %L5
    i8 6, label %L6
    i8 7, label %L7
    i8 8, label %L8
    i8 9, label %L9
  ]
L0: tail call void @c0() ret void
L1: tail call void @c1() ret void
L2: tail call void @c2() ret void
L3: tail call void @c3() ret void
L4: tail call void @c4() ret void
L5: tail call void @c5() ret void
L6: tail call void @c6() ret void
L7: tail call void @c7() ret void
L8: tail call void @c8() ret void
L9: tail call void @c9() ret void
L_def: tail call void @def() ret void
}

; CHECK-LABEL: _dispatch:
; The narrowed bound must be cp 10 (Range+1), not cp 9.
; CHECK-NOT:   cp 9
; CHECK:       cp 10
