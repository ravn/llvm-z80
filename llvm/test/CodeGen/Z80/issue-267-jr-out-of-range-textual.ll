; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s
;
; Regression test for #267: the textual asm printer under-relaxes long
; conditional/unconditional branches.  In @sf_fix the i32 expansion makes the
; function large enough that the branches to the overflow / negate / return
; blocks sit farther than a `jr`'s signed 8-bit displacement (+/-127 B) allows.
;
; The object emitter (-filetype=obj) relaxes these to `jp`, so the .o is
; correct, but the textual `.s` keeps the literal `jr`.  clang's own integrated
; assembler silently relaxes on reassembly, so `llvm-mc`/`-c` hide the defect --
; but an external Z80 assembler (z88dk z80asm, used by the
; `zcc +cpm -compiler=llvmz80` pipeline) rejects it:
;
;   error: integer range: $84   ; -> .LBB0_15  (132 B, > 127)
;   error: integer range: $8e   ; -> .LBB0_22  (142 B)
;   error: integer range: $84   ; -> .LBB0_21  (132 B)
;
; The fix must emit `jp` for these far branches in the textual output too.  This
; test is XFAIL until #267 is fixed; when fixed it XPASSes -- remove the XFAIL
; line then.  The .LBB labels below reflect llc at filing time; if they renumber
; when the bug is fixed, refresh them.
;
; XFAIL: *
;
; CHECK-LABEL: sf_fix:
; CHECK-NOT: jr nc,.LBB0_15
; CHECK-NOT: jr z,.LBB0_22
; CHECK-NOT: jr .LBB0_21

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

; A reduced clang -O2 lowering of a real SoftFloat function (the float->int
; convert / roundPack shape) that triggered the bug in the softfloat build.
define dso_local i32 @sf_fix(i32 noundef %0) local_unnamed_addr #0 {
  %2 = lshr i32 %0, 23
  %3 = trunc nuw nsw i32 %2 to i16
  %4 = and i16 %3, 255
  %5 = and i32 %0, 8388607
  %6 = trunc i32 %2 to i8
  switch i8 %6, label %10 [
    i8 0, label %33
    i8 -1, label %7
  ]

7:                                                ; preds = %1
  %8 = icmp sgt i32 %0, -1
  %9 = select i1 %8, i32 2147483647, i32 -2147483648
  br label %33

10:                                               ; preds = %1
  %11 = icmp samesign ult i16 %4, 127
  br i1 %11, label %33, label %12

12:                                               ; preds = %10
  %13 = icmp samesign ugt i16 %4, 157
  br i1 %13, label %14, label %17

14:                                               ; preds = %12
  %15 = icmp sgt i32 %0, -1
  %16 = select i1 %15, i32 2147483647, i32 -2147483648
  br label %33

17:                                               ; preds = %12
  %18 = or disjoint i32 %5, 8388608
  %19 = icmp samesign ugt i16 %4, 149
  br i1 %19, label %20, label %24

20:                                               ; preds = %17
  %21 = add nsw i16 %4, -150
  %22 = zext nneg i16 %21 to i32
  %23 = shl nuw nsw i32 %18, %22
  br label %28

24:                                               ; preds = %17
  %25 = sub nuw nsw i16 150, %4
  %26 = zext nneg i16 %25 to i32
  %27 = lshr i32 %18, %26
  br label %28

28:                                               ; preds = %24, %20
  %29 = phi i32 [ %23, %20 ], [ %27, %24 ]
  %30 = icmp sgt i32 %0, -1
  br i1 %30, label %33, label %31

31:                                               ; preds = %28
  %32 = sub nsw i32 0, %29
  br label %33

33:                                               ; preds = %14, %10, %28, %31, %1, %7
  %34 = phi i32 [ 0, %1 ], [ %9, %7 ], [ 0, %10 ], [ %16, %14 ], [ %32, %31 ], [ %29, %28 ]
  ret i32 %34
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind null_pointer_is_valid willreturn memory(none) "target-features"="+z80" }
