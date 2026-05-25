; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 < %s | FileCheck %s

; ravn/llvm-z80#195/test_27: a +static-stack frame whose base address is
; materialized into a register (`LD HL,__sfrend_main`, here for a volatile
; m[3][3] indexed in a loop) can be read INDIRECTLY via pointer arithmetic.
; The BSS-spill->PUSH/POP peephole's orphan scan only sees DIRECT `LD A,(nn)`
; reads, so it converted m[0][0]'s store+reload to PUSH/POP and dropped the
; store -- the slot was then read (indirectly) as uninitialised, and the loop
; sum was off by one (44 vs 45).  Fix: refuse the conversion for slots whose
; frame symbol is address-taken.  The first element's value must be STORED to
; its BSS slot (not pushed and discarded).

; ModuleID = 't175.c'
source_filename = "t175.c"
target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

; Function Attrs: nofree norecurse nounwind optsize memory(inaccessiblemem: readwrite)
define dso_local i16 @main() local_unnamed_addr #0 {
  %1 = alloca [3 x [3 x i16]], align 1
  call void @llvm.lifetime.start.p0(ptr nonnull %1) #2
  store volatile i16 1, ptr %1, align 1, !tbaa !6
  %2 = getelementptr inbounds nuw i8, ptr %1, i16 2
  store volatile i16 2, ptr %2, align 1, !tbaa !6
  %3 = getelementptr inbounds nuw i8, ptr %1, i16 4
  store volatile i16 3, ptr %3, align 1, !tbaa !6
  %4 = getelementptr inbounds nuw i8, ptr %1, i16 6
  store volatile i16 4, ptr %4, align 1, !tbaa !6
  %5 = getelementptr inbounds nuw i8, ptr %1, i16 8
  store volatile i16 5, ptr %5, align 1, !tbaa !6
  %6 = getelementptr inbounds nuw i8, ptr %1, i16 10
  store volatile i16 6, ptr %6, align 1, !tbaa !6
  %7 = getelementptr inbounds nuw i8, ptr %1, i16 12
  store volatile i16 7, ptr %7, align 1, !tbaa !6
  %8 = getelementptr inbounds nuw i8, ptr %1, i16 14
  store volatile i16 8, ptr %8, align 1, !tbaa !6
  %9 = getelementptr inbounds nuw i8, ptr %1, i16 16
  store volatile i16 9, ptr %9, align 1, !tbaa !6
  %10 = load volatile i16, ptr %1, align 1, !tbaa !6
  %11 = load volatile i16, ptr %5, align 1, !tbaa !6
  %12 = load volatile i16, ptr %9, align 1, !tbaa !6
  br label %13

13:                                               ; preds = %0, %13
  %14 = phi i16 [ 0, %0 ], [ %25, %13 ]
  %15 = phi i16 [ 0, %0 ], [ %24, %13 ]
  %16 = getelementptr inbounds nuw [6 x i8], ptr %1, i16 %14
  %17 = load volatile i16, ptr %16, align 1, !tbaa !6
  %18 = add i16 %17, %15
  %19 = getelementptr inbounds nuw i8, ptr %16, i16 2
  %20 = load volatile i16, ptr %19, align 1, !tbaa !6
  %21 = add i16 %20, %18
  %22 = getelementptr inbounds nuw i8, ptr %16, i16 4
  %23 = load volatile i16, ptr %22, align 1, !tbaa !6
  %24 = add i16 %23, %21
  %25 = add nuw nsw i16 %14, 1
  %26 = icmp eq i16 %25, 3
  br i1 %26, label %27, label %13, !llvm.loop !8

27:                                               ; preds = %13
  call void @llvm.lifetime.end.p0(ptr nonnull %1) #2
  ret i16 %24
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(ptr captures(none)) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(ptr captures(none)) #1

attributes #0 = { nofree norecurse nounwind optsize memory(inaccessiblemem: readwrite) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+static-stack" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!llvm.errno.tbaa = !{!2}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{!"clang version 23.0.0git (git@github.com:ravn/llvm-z80.git 63661013ad212aa582477614572e942dd7b366b8)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"short", !4, i64 0}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}

; The m[0][0]=1 store must reach its BSS slot (not be PUSH'd and dropped).
; CHECK: ld de,1
; CHECK-NEXT: ld ({{[^)]*}}),de
