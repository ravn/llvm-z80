; ravn/llvm-z80 #254: at -O0 hasFP is true, so a static-stack function uses the
; frame-pointer path (IX == __sfrend_<fn>).  The callee-saved register saves
; (e.g. the caller's IX) live on the REAL stack via PUSH and are excluded from
; the BSS frame (Z80AsmPrinter: BSSSize = StackSize - CalleeSavedFrameSize).
; The frame-index -> __sfrend_<fn>+disp lowering used to skip only the saved-IX
; slot (+2) but NOT that callee-saved region, so the deepest spill slot was
; emitted at __sfrend_<fn>-8 while __sframe_<fn> sat at __sfrend_<fn>-6 -- a
; 2-byte underflow BELOW __sframe_<fn> into the adjacent global (silent data
; corruption).  The fix adds CalleeSavedFrameSize to the frame-pointer static
; displacement, mirroring the no-frame-pointer static branch.
;
; The invariant this test pins: no frame access may reach below __sframe_<fn>,
; i.e. for a 6-byte frame (.zero 6) the deepest displacement is __sfrend_f-6
; (== __sframe_f) and NEVER __sfrend_f-7 or __sfrend_f-8 (the pre-fix bug).
;
; RUN: llc -mtriple=z80 -O0 < %s | FileCheck %s

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

@k = dso_local global i8 2, align 1
@n = dso_local global i8 6, align 1
@g = dso_local global [10 x i8] zeroinitializer, align 1

; CHECK-LABEL: _f:
; The frame is 6 BSS bytes; the deepest slot must land AT __sframe_f
; (__sfrend_f-6), never below it.
; CHECK-NOT: __sfrend_f-7
; CHECK-NOT: __sfrend_f-8
; CHECK-NOT: __sfrend_f-9
; CHECK-NOT: __sfrend_f-1{{[0-9]}}
; CHECK: __sframe_f:
; CHECK-NEXT: .zero 6
define dso_local i16 @f() {
  %d = alloca ptr, align 1
  %s = alloca ptr, align 1
  store ptr @g, ptr %d, align 1
  %kv = load volatile i8, ptr @k, align 1
  %kz = zext i8 %kv to i16
  %sp = getelementptr inbounds i8, ptr @g, i16 %kz
  store ptr %sp, ptr %s, align 1
  %dl = load ptr, ptr %d, align 1
  %sl = load ptr, ptr %s, align 1
  %nv = load volatile i8, ptr @n, align 1
  %nz = zext i8 %nv to i16
  call void @llvm.memmove.p0.p0.i16(ptr align 1 %dl, ptr align 1 %sl, i16 %nz, i1 false)
  %g0 = load i8, ptr @g, align 1
  %g0z = zext i8 %g0 to i16
  %hi = shl i16 %g0z, 8
  %g1 = load i8, ptr getelementptr inbounds nuw (i8, ptr @g, i16 1), align 1
  %g1z = zext i8 %g1 to i16
  %r = or i16 %hi, %g1z
  ret i16 %r
}

declare void @llvm.memmove.p0.p0.i16(ptr writeonly captures(none), ptr readonly captures(none), i16, i1 immarg)
