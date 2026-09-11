; RUN: llc -mtriple=z80 -mattr=+static-frame -O2 -disable-lsr < %s | FileCheck %s
; XFAIL: *
;
; Miscompile: a function that resets SP itself (inline asm `ld sp, imm`, as the
; RC700 autoload `main_relocated` does via SET_SP(ROM_STACK)) must not keep any
; compiler-managed frame object, because the register allocator computes the
; frame slot address relative to the ENTRY SP (`add hl, sp`) and spills there in
; the prologue, then the inline `ld sp` moves SP, and the later reload reads the
; slot relative to the NEW SP -- a different, uninitialised address. Observed on
; autoload: `fdc_cmd.sector = 1` reloaded a spilled &fdc_cmd pointer of 0x0000
; (runtime write-tap), so the store went to null, sector stayed 0, and the boot
; Read Data used sector 0 (disk is 1-based) -> No Data -> DISKETTE ERROR.
; See ravn/rc700-gensmedet#128, ravn/llvm-z80#316 (static-frame regression forced
; the frame spills that make this fire; `__naked` is a no-op for clang so the
; frame is not suppressed).
;
; Correct behavior: no frame slot is accessed (`add hl, sp`) BEFORE the SP is
; reset. When fixed (frame suppressed, or established after the SP write), this
; XPASSes -- drop the XFAIL then.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

@g = dso_local global [7 x i8] zeroinitializer
@dbl = dso_local global i8 0

declare void @a()
declare void @b()
declare void @c()
declare void @d()
declare void @e()
declare i16 @detect()
declare void @rd()

define dso_local void @mainrel() {
; CHECK-LABEL: mainrel:
; No frame slot may be materialised before the manual SP reset.
; CHECK-NOT: add hl,sp
; CHECK: ld sp,
  tail call void asm sideeffect "ld sp, 0xbfff", ""()
  tail call void @a()
  tail call void @b()
  tail call void @c()
  tail call void @d()
  tail call void @e()
  store i8 1, ptr getelementptr inbounds nuw (i8, ptr @g, i16 1), align 1
  store i8 1, ptr getelementptr inbounds nuw (i8, ptr @g, i16 2), align 1
  %1 = tail call i16 @detect()
  %2 = icmp eq i16 %1, 0
  br i1 %2, label %3, label %4
3:
  store i8 1, ptr @dbl, align 1
  br label %4
4:
  store i8 0, ptr getelementptr inbounds nuw (i8, ptr @g, i16 1), align 1
  %5 = tail call i16 @detect()
  %6 = icmp eq i16 %5, 0
  br i1 %6, label %7, label %11
7:
  br label %8
8:
  tail call void @rd()
  %9 = load i8, ptr @g, align 1
  %10 = icmp eq i8 %9, 0
  br i1 %10, label %11, label %8
11:
  ret void
}
