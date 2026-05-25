; RUN: llc -mtriple=z80 -mattr=+static-stack -O1 < %s | FileCheck %s

; ravn/llvm-z80#193: the "BSS spill->PUSH/POP" peephole in Z80LateOptimization
; segfaulted (EXC_BAD_ACCESS) when a 16-bit BSS spill (LD (sfrend),DE) was
; immediately followed by its matching reload (LD DE,(sfrend)): after erasing
; the store the iterator pointed AT the reload, erasing the reload left it
; dangling, and the subsequent `--MII` dereferenced freed memory.  Fix anchors
; resumption to the inserted PUSH instead of decrementing the post-erase
; iterator.  This must compile without crashing.  Reduced from test_40 (the
; crash manifested in xorshift16 under +static-stack at all opt levels).

; ModuleID = 't40.c'
source_filename = "t40.c"
target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

; Function Attrs: noinline nounwind optnone
define dso_local zeroext i16 @crc16(ptr noundef %0, i16 noundef zeroext %1) #0 {
  %3 = alloca ptr, align 1
  %4 = alloca i16, align 1
  %5 = alloca i16, align 1
  %6 = alloca i16, align 1
  %7 = alloca i8, align 1
  store ptr %0, ptr %3, align 1
  store i16 %1, ptr %4, align 1
  store i16 -1, ptr %5, align 1
  store i16 0, ptr %6, align 1
  br label %8

8:                                                ; preds = %42, %2
  %9 = load i16, ptr %6, align 1
  %10 = load i16, ptr %4, align 1
  %11 = icmp ult i16 %9, %10
  br i1 %11, label %12, label %45

12:                                               ; preds = %8
  %13 = load ptr, ptr %3, align 1
  %14 = load i16, ptr %6, align 1
  %15 = getelementptr inbounds nuw i8, ptr %13, i16 %14
  %16 = load i8, ptr %15, align 1
  %17 = zext i8 %16 to i16
  %18 = shl i16 %17, 8
  %19 = load i16, ptr %5, align 1
  %20 = xor i16 %19, %18
  store i16 %20, ptr %5, align 1
  store i8 0, ptr %7, align 1
  br label %21

21:                                               ; preds = %38, %12
  %22 = load i8, ptr %7, align 1
  %23 = zext i8 %22 to i16
  %24 = icmp slt i16 %23, 8
  br i1 %24, label %25, label %41

25:                                               ; preds = %21
  %26 = load i16, ptr %5, align 1
  %27 = and i16 %26, -32768
  %28 = icmp ne i16 %27, 0
  br i1 %28, label %29, label %33

29:                                               ; preds = %25
  %30 = load i16, ptr %5, align 1
  %31 = shl i16 %30, 1
  %32 = xor i16 %31, 4129
  br label %36

33:                                               ; preds = %25
  %34 = load i16, ptr %5, align 1
  %35 = shl i16 %34, 1
  br label %36

36:                                               ; preds = %33, %29
  %37 = phi i16 [ %32, %29 ], [ %35, %33 ]
  store i16 %37, ptr %5, align 1
  br label %38

38:                                               ; preds = %36
  %39 = load i8, ptr %7, align 1
  %40 = add i8 %39, 1
  store i8 %40, ptr %7, align 1
  br label %21, !llvm.loop !3

41:                                               ; preds = %21
  br label %42

42:                                               ; preds = %41
  %43 = load i16, ptr %6, align 1
  %44 = add i16 %43, 1
  store i16 %44, ptr %6, align 1
  br label %8, !llvm.loop !5

45:                                               ; preds = %8
  %46 = load i16, ptr %5, align 1
  ret i16 %46
}

; Function Attrs: noinline nounwind optnone
define dso_local i32 @crc32(ptr noundef %0, i16 noundef zeroext %1) #0 {
  %3 = alloca ptr, align 1
  %4 = alloca i16, align 1
  %5 = alloca i32, align 1
  %6 = alloca i16, align 1
  %7 = alloca i8, align 1
  store ptr %0, ptr %3, align 1
  store i16 %1, ptr %4, align 1
  store i32 -1, ptr %5, align 1
  store i16 0, ptr %6, align 1
  br label %8

8:                                                ; preds = %37, %2
  %9 = load i16, ptr %6, align 1
  %10 = load i16, ptr %4, align 1
  %11 = icmp ult i16 %9, %10
  br i1 %11, label %12, label %40

12:                                               ; preds = %8
  %13 = load ptr, ptr %3, align 1
  %14 = load i16, ptr %6, align 1
  %15 = getelementptr inbounds nuw i8, ptr %13, i16 %14
  %16 = load i8, ptr %15, align 1
  %17 = zext i8 %16 to i32
  %18 = load i32, ptr %5, align 1
  %19 = xor i32 %18, %17
  store i32 %19, ptr %5, align 1
  store i8 0, ptr %7, align 1
  br label %20

20:                                               ; preds = %33, %12
  %21 = load i8, ptr %7, align 1
  %22 = zext i8 %21 to i16
  %23 = icmp slt i16 %22, 8
  br i1 %23, label %24, label %36

24:                                               ; preds = %20
  %25 = load i32, ptr %5, align 1
  %26 = lshr i32 %25, 1
  %27 = load i32, ptr %5, align 1
  %28 = and i32 %27, 1
  %29 = icmp ne i32 %28, 0
  %30 = zext i1 %29 to i64
  %31 = select i1 %29, i32 -306674912, i32 0
  %32 = xor i32 %26, %31
  store i32 %32, ptr %5, align 1
  br label %33

33:                                               ; preds = %24
  %34 = load i8, ptr %7, align 1
  %35 = add i8 %34, 1
  store i8 %35, ptr %7, align 1
  br label %20, !llvm.loop !6

36:                                               ; preds = %20
  br label %37

37:                                               ; preds = %36
  %38 = load i16, ptr %6, align 1
  %39 = add i16 %38, 1
  store i16 %39, ptr %6, align 1
  br label %8, !llvm.loop !7

40:                                               ; preds = %8
  %41 = load i32, ptr %5, align 1
  %42 = xor i32 %41, -1
  ret i32 %42
}

; Function Attrs: noinline nounwind optnone
define dso_local zeroext i16 @hash16(ptr noundef %0, i8 noundef zeroext %1) #0 {
  %3 = alloca ptr, align 1
  %4 = alloca i8, align 1
  %5 = alloca i16, align 1
  %6 = alloca i8, align 1
  store ptr %0, ptr %3, align 1
  store i8 %1, ptr %4, align 1
  store i16 5381, ptr %5, align 1
  store i8 0, ptr %6, align 1
  br label %7

7:                                                ; preds = %23, %2
  %8 = load i8, ptr %6, align 1
  %9 = zext i8 %8 to i16
  %10 = load i8, ptr %4, align 1
  %11 = zext i8 %10 to i16
  %12 = icmp slt i16 %9, %11
  br i1 %12, label %13, label %26

13:                                               ; preds = %7
  %14 = load i16, ptr %5, align 1
  %15 = mul i16 %14, 33
  %16 = load ptr, ptr %3, align 1
  %17 = load i8, ptr %6, align 1
  %18 = zext i8 %17 to i16
  %19 = getelementptr inbounds nuw i8, ptr %16, i16 %18
  %20 = load i8, ptr %19, align 1
  %21 = zext i8 %20 to i16
  %22 = add i16 %15, %21
  store i16 %22, ptr %5, align 1
  br label %23

23:                                               ; preds = %13
  %24 = load i8, ptr %6, align 1
  %25 = add i8 %24, 1
  store i8 %25, ptr %6, align 1
  br label %7, !llvm.loop !8

26:                                               ; preds = %7
  %27 = load i16, ptr %5, align 1
  ret i16 %27
}

; Function Attrs: noinline nounwind optnone
define dso_local zeroext i16 @xorshift16(i16 noundef zeroext %0) #0 {
  %2 = alloca i16, align 1
  store i16 %0, ptr %2, align 1
  %3 = load i16, ptr %2, align 1
  %4 = shl i16 %3, 7
  %5 = load i16, ptr %2, align 1
  %6 = xor i16 %5, %4
  store i16 %6, ptr %2, align 1
  %7 = load i16, ptr %2, align 1
  %8 = lshr i16 %7, 9
  %9 = load i16, ptr %2, align 1
  %10 = xor i16 %9, %8
  store i16 %10, ptr %2, align 1
  %11 = load i16, ptr %2, align 1
  %12 = shl i16 %11, 8
  %13 = load i16, ptr %2, align 1
  %14 = xor i16 %13, %12
  store i16 %14, ptr %2, align 1
  %15 = load i16, ptr %2, align 1
  ret i16 %15
}

; Function Attrs: noinline nounwind optnone
define dso_local i16 @main() #0 {
  %1 = alloca i16, align 1
  %2 = alloca [2 x i8], align 1
  %3 = alloca i16, align 1
  %4 = alloca [1 x i8], align 1
  %5 = alloca i32, align 1
  %6 = alloca [5 x i8], align 1
  %7 = alloca i16, align 1
  %8 = alloca i16, align 1
  store i16 0, ptr %1, align 1
  %9 = getelementptr inbounds [2 x i8], ptr %2, i16 0, i16 0
  store i8 65, ptr %9, align 1
  %10 = getelementptr inbounds [2 x i8], ptr %2, i16 0, i16 1
  store i8 66, ptr %10, align 1
  %11 = getelementptr inbounds [2 x i8], ptr %2, i16 0, i16 0
  %12 = call zeroext i16 @crc16(ptr noundef %11, i16 noundef zeroext 2) #1
  store i16 %12, ptr %3, align 1
  %13 = load i16, ptr %3, align 1
  %14 = icmp eq i16 %13, 19316
  br i1 %14, label %15, label %18

15:                                               ; preds = %0
  %16 = load i16, ptr %1, align 1
  %17 = or i16 %16, 1
  store i16 %17, ptr %1, align 1
  br label %18

18:                                               ; preds = %15, %0
  %19 = getelementptr inbounds [1 x i8], ptr %4, i16 0, i16 0
  store i8 65, ptr %19, align 1
  %20 = getelementptr inbounds [1 x i8], ptr %4, i16 0, i16 0
  %21 = call i32 @crc32(ptr noundef %20, i16 noundef zeroext 1) #1
  store i32 %21, ptr %5, align 1
  %22 = load i32, ptr %5, align 1
  %23 = icmp eq i32 %22, -740712821
  br i1 %23, label %24, label %27

24:                                               ; preds = %18
  %25 = load i16, ptr %1, align 1
  %26 = or i16 %25, 2
  store i16 %26, ptr %1, align 1
  br label %27

27:                                               ; preds = %24, %18
  %28 = getelementptr inbounds [5 x i8], ptr %6, i16 0, i16 0
  store i8 104, ptr %28, align 1
  %29 = getelementptr inbounds [5 x i8], ptr %6, i16 0, i16 1
  store i8 101, ptr %29, align 1
  %30 = getelementptr inbounds [5 x i8], ptr %6, i16 0, i16 2
  store i8 108, ptr %30, align 1
  %31 = getelementptr inbounds [5 x i8], ptr %6, i16 0, i16 3
  store i8 108, ptr %31, align 1
  %32 = getelementptr inbounds [5 x i8], ptr %6, i16 0, i16 4
  store i8 111, ptr %32, align 1
  %33 = getelementptr inbounds [5 x i8], ptr %6, i16 0, i16 0
  %34 = call zeroext i16 @hash16(ptr noundef %33, i8 noundef zeroext 5) #1
  store i16 %34, ptr %7, align 1
  %35 = load i16, ptr %7, align 1
  %36 = icmp eq i16 %35, 12441
  br i1 %36, label %37, label %40

37:                                               ; preds = %27
  %38 = load i16, ptr %1, align 1
  %39 = or i16 %38, 4
  store i16 %39, ptr %1, align 1
  br label %40

40:                                               ; preds = %37, %27
  store volatile i16 1234, ptr %8, align 1
  %41 = load volatile i16, ptr %8, align 1
  %42 = call zeroext i16 @xorshift16(i16 noundef zeroext %41) #1
  store volatile i16 %42, ptr %8, align 1
  %43 = load volatile i16, ptr %8, align 1
  %44 = call zeroext i16 @xorshift16(i16 noundef zeroext %43) #1
  store volatile i16 %44, ptr %8, align 1
  %45 = load volatile i16, ptr %8, align 1
  %46 = call zeroext i16 @xorshift16(i16 noundef zeroext %45) #1
  store volatile i16 %46, ptr %8, align 1
  %47 = load volatile i16, ptr %8, align 1
  %48 = icmp eq i16 %47, -4034
  br i1 %48, label %49, label %52

49:                                               ; preds = %40
  %50 = load i16, ptr %1, align 1
  %51 = or i16 %50, 8
  store i16 %51, ptr %1, align 1
  br label %52

52:                                               ; preds = %49, %40
  %53 = load i16, ptr %1, align 1
  ret i16 %53
}

attributes #0 = { noinline nounwind optnone "frame-pointer"="all" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+static-stack" }
attributes #1 = { nobuiltin "no-builtins" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 7, !"frame-pointer", i32 2}
!2 = !{!"clang version 23.0.0git (git@github.com:ravn/llvm-z80.git 4c21831c419ec1bc29eb4f39709f55e60834ed01)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = distinct !{!5, !4}
!6 = distinct !{!6, !4}
!7 = distinct !{!7, !4}
!8 = distinct !{!8, !4}

; CHECK: xorshift16:
