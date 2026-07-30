; ravn/llvm-z80 #268: a function that returns an aggregate/double via sret,
; whose return value is produced by ANOTHER sret-returning call, copied the
; callee's result into the WRONG destination slot when the function has
; callee-saved registers (CSR > 0).
;
; Frame layout (static-stack + frame pointer, IX == saved-IX):
;   [ix+2,+3]  return address (pushed by CALL)
;   [ix+4,+5]  sret return pointer   (fixed object, ObjOff=2)   <-- @bug's own
;   [ix+6..13] incoming arg a        (fixed object, ObjOff=4)
;   [ix+14..21] incoming arg b       (fixed object, ObjOff=12)
; The CSR saves live BELOW IX on the real stack, NOT above it, so incoming
; args and the sret pointer (all fixed objects, Idx < 0) must NOT be shifted
; by CalleeSavedFrameSize.  The pre-fix code unconditionally added CSR in the
; static-stack + frame-pointer branch of eliminateFrameIndex, so with CSR=2 the
; sret pointer at [ix+4] was misread as [ix+6] (== arg a's low word).  The
; __memmove_rt that copies g()'s result into @bug's sret buffer then loaded its
; DESTINATION from [ix+6] instead of [ix+4]; with a==3.0 (low word 0x0000) the
; 8 bytes landed on CP/M's 0x0000 warm-boot vector -> hang.
;
; The fix (#268) guards the CSR term with Idx >= 0 (BSS locals only).  Control
; fn @ok has CSR=0, so it was correct either way and pins that the fix does not
; regress the CSR==0 path.
;
; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

%struct.F = type { i64 }

; @bug has CSR=2 (it saves IX).  The __memmove_rt that copies g()'s sret result
; into @bug's own sret buffer loads its DESTINATION (HL) from the sret pointer.
; That pointer lives at [ix+4]/[ix+5]; pre-fix it was misread as [ix+6]/[ix+7]
; (arg a's low word).  Pin the two dest loads immediately preceding the copy.
; (Note: @bug DOES legitimately read arg a at [ix+6] elsewhere to forward it to
; g byval, so a blanket CHECK-NOT (ix+6) would be wrong -- the invariant is
; specifically the HL dest feeding the memmove.)
; CHECK-LABEL: _bug:
; CHECK: ld l,(ix+4)
; CHECK-NEXT: ld h,(ix+5)
; CHECK-NEXT: ld e,(ix
; CHECK-NEXT: ld d,(ix
; CHECK-NEXT: ld bc,8
; CHECK-NEXT: call ___memmove_rt
define dso_local double @bug(double noundef %0, double noundef %1) {
  %3 = alloca %struct.F, align 1
  %4 = alloca %struct.F, align 1
  %5 = alloca %struct.F, align 1
  store double %0, ptr %4, align 1
  store double %1, ptr %5, align 1
  call void @g(ptr dead_on_unwind nonnull writable sret(%struct.F) align 1 %3, ptr noundef nonnull byval(%struct.F) align 1 %4, ptr noundef nonnull byval(%struct.F) align 1 %5)
  %6 = load double, ptr %3, align 1
  ret double %6
}

declare dso_local void @g(ptr dead_on_unwind writable sret(%struct.F) align 1, ptr noundef byval(%struct.F) align 1, ptr noundef byval(%struct.F) align 1)

; Control: returns a double straight from an argument, CSR=0.  Its sret pointer
; at [ix+4] must be read directly (regression guard for the CSR==0 path).
; CHECK-LABEL: _ok:
; CHECK: ld c,(ix+4)
; CHECK-NEXT: ld b,(ix+5)
define dso_local double @ok(double noundef %0, double noundef returned %1) {
  ret double %1
}
