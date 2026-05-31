; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -verify-machineinstrs < %s | FileCheck %s
;
; ravn/llvm-z80#197: a 16-bit i!=n loop-exit compare lowers to a byte-wise
; XOR/OR sequence, which the Z80LateOptimization peephole rewrites to
; `AND A; SBC HL,rr` (the AND A clears carry for the SBC).  Across the loop
; back-edge A is dead (the loop body's CALLs clobber it), so the carry-clear's
; $a read is a don't-care -- but the peephole built `AND A` without marking that
; read undef, so -verify-machineinstrs aborted with "Using an undefined physical
; register $a".  The peephole already proves A dead after the branch
; (isRegDeadAfter); it now marks the $a read undef.  This RUN line FAILS (verifier
; abort) without the fix.  Distilled from test_98 walk_three_buffers @ -O2
; +static-stack.
;
; CHECK-LABEL: _walk_three_buffers:
; CHECK:       and a
; CHECK-NEXT:  sbc hl,
; CHECK:       ret

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

@buf_a = internal constant [8 x i8] c"\01\02\03\04\05\06\07\08", align 1
@buf_b = internal constant [8 x i8] c"\0A\14\1E(2<FP", align 1
@buf_c = internal constant [8 x i8] c"dcba`_^]", align 1

define internal fastcc zeroext i16 @walk_three_buffers(i16 zeroext %0) unnamed_addr {
  %2 = icmp eq i16 %0, 0
  br i1 %2, label %3, label %5
3:
  %4 = phi i16 [ 0, %1 ], [ %13, %5 ]
  ret i16 %4
5:
  %6 = phi i16 [ %13, %5 ], [ 0, %1 ]
  %7 = phi i16 [ %14, %5 ], [ 0, %1 ]
  %8 = tail call fastcc zeroext i16 @fetch_byte(ptr nonnull @buf_a, i16 zeroext %7)
  %9 = tail call fastcc zeroext i16 @accumulate(i16 zeroext %6, i16 zeroext %8)
  %10 = tail call fastcc zeroext i16 @fetch_byte(ptr nonnull @buf_b, i16 zeroext %7)
  %11 = tail call fastcc zeroext i16 @accumulate(i16 zeroext %9, i16 zeroext %10)
  %12 = tail call fastcc zeroext i16 @fetch_byte(ptr nonnull @buf_c, i16 zeroext %7)
  %13 = tail call fastcc zeroext i16 @accumulate(i16 zeroext %11, i16 zeroext %12)
  %14 = add nuw i16 %7, 1
  %15 = icmp eq i16 %14, %0
  br i1 %15, label %3, label %5
}

define internal fastcc zeroext i16 @fetch_byte(ptr readonly captures(none) %0, i16 zeroext %1) unnamed_addr noinline {
  %3 = getelementptr inbounds nuw i8, ptr %0, i16 %1
  %4 = load i8, ptr %3, align 1
  %5 = zext i8 %4 to i16
  ret i16 %5
}

define internal fastcc zeroext i16 @accumulate(i16 zeroext %0, i16 zeroext %1) unnamed_addr noinline {
  %3 = add i16 %1, %0
  ret i16 %3
}
