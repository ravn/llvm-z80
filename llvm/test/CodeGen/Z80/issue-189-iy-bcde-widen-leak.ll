; RUN: llc -mtriple=z80 -verify-machineinstrs < %s | FileCheck %s
;
; ravn/llvm-z80 #189 residual: when IY is allocatable (optsize + static-stack,
; the path that auto-static-stack makes the default), a byte-decomposed 16-bit
; value must never land in IY -- IX/IY have no documented 8-bit sub-register
; ops, so extracting its halves emits undocumented IYH/IYL.
;
; The leak here: an i16 EQ/NE compare keeps its operand in GR16_BCDE (= {BC,DE})
; and extracts sub_hi/sub_lo for the byte-wise compare.  getLargestLegalSuperClass
; (the grow step in recomputeRegClass / greedy live-range splitting) widened
; GR16_BCDE up to GR16 -- which re-introduces IX/IY -- so greedy parked the
; loop-carried compare operand in IY and emitted `ld a,iyh` / `ld a,iyl`.  The
; old guard only protected GR16NoIR; it now refuses to widen ANY IY-excluding
; 16-bit subclass when IY is allocatable.  (Reduced from test_96; the original
; surfaced via -verify-machineinstrs "isRenamable set on reserved register".)
;
; No undocumented IY half-register may appear:
; CHECK-NOT: iyh
; CHECK-NOT: iyl
; CHECK-LABEL: call_via_ptr:

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

declare i16 @sub2(i16, i16) #0

define internal fastcc i16 @call_via_ptr() unnamed_addr #0 {
  %1 = alloca [200 x i16], align 1
  call void @llvm.lifetime.start.p0(ptr nonnull %1)
  br label %2

2:                                                ; preds = %5, %0
  %3 = phi i16 [ 0, %0 ], [ %7, %5 ]
  %4 = icmp eq i16 %3, 200
  br i1 %4, label %8, label %5

5:                                                ; preds = %2
  %6 = getelementptr inbounds nuw [2 x i8], ptr %1, i16 %3
  store volatile i16 %3, ptr %6, align 1
  %7 = add nuw nsw i16 %3, 1
  br label %2

8:                                                ; preds = %2, %14
  %9 = phi i16 [ %17, %14 ], [ 0, %2 ]
  %10 = phi i16 [ %18, %14 ], [ 0, %2 ]
  %11 = icmp eq i16 %10, 200
  br i1 %11, label %12, label %14

12:                                               ; preds = %8
  %13 = icmp eq i16 %9, 19900
  br i1 %13, label %19, label %23

14:                                               ; preds = %8
  %15 = getelementptr inbounds nuw [2 x i8], ptr %1, i16 %10
  %16 = load volatile i16, ptr %15, align 1
  %17 = add i16 %16, %9
  %18 = add nuw nsw i16 %10, 1
  br label %8

19:                                               ; preds = %12
  %20 = tail call zeroext i16 @sub2(i16 noundef zeroext 99, i16 noundef zeroext 155)
  %21 = icmp eq i16 %20, -56
  %22 = zext i1 %21 to i16
  br label %23

23:                                               ; preds = %12, %19
  %24 = phi i16 [ %22, %19 ], [ 0, %12 ]
  call void @llvm.lifetime.end.p0(ptr nonnull %1)
  ret i16 %24
}

declare void @llvm.lifetime.start.p0(ptr captures(none)) #1
declare void @llvm.lifetime.end.p0(ptr captures(none)) #1

attributes #0 = { minsize optsize "target-features"="+z80,+static-stack" }
attributes #1 = { nounwind }
