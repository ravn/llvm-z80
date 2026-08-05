; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 -z80-float-sdcccall0 %s -o - | FileCheck %s
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 %s -o - | FileCheck --check-prefix=DEFAULT %s
;
; ravn/llvm-z80 #277 (follow-up to issue-277-f32-libcall-sdcccall0.ll): the
; f32 compare libcalls (__cmpsf2/__gtsf2/__gesf2/__unordsf2) and the f32<->i32
; conversion libcalls (__fixsfsi/__fixunssfsi/__floatsisf/__floatunsisf) are
; ALSO emitted with CallingConv::Z80_SDCCCall0 under the same opt-in
; `-z80-float-sdcccall0` flag, so z88dk's math32 bridge (this time NOT a pure
; alias for compares -- see z88dk's libsrc/l/llvmz80/__cmpsf2.asm -- and a
; pure alias for conversions, libsrc/l/llvmz80/__floatsisf.asm) sees the
; sdcccall(0) stack-args-only / DE:HL(or plain HL for 16-bit results)
; convention consistently across the whole f32 libcall surface.
;
; Compares: BOTH 32-bit float operands are pushed to the stack (4 `push hl`
; total), matching the arithmetic convention exactly (same declared-operand-
; order construction in Z80LegalizerInfo.cpp's G_FCMP case). Conversions:
; ONE 32-bit operand (int, sign/zero-extended by minScalar before reaching
; the libcall, or float) is pushed (2 `push hl`).
;
; DEFAULT (no flag): compares pass only the second float on the stack (the
; first stays register-resident per the default C ABI, same shape as
; issue-277-f32-libcall-sdcccall0.ll's arithmetic DEFAULT case); conversions
; pass no stack argument at all (single register-resident arg fits directly
; in HL:DE under the default ABI, so the call is a plain `jp`/`call` with no
; `push` at all for a 16-bit source int, or an implicit sign-extend then
; direct call for the float source).

define i16 @cmp_lt(float %a, float %b) {
; CHECK-LABEL: _cmp_lt:
; CHECK: push hl
; CHECK: push hl
; CHECK: push hl
; CHECK: push hl
; CHECK: call ___cmpsf2
;
; DEFAULT-LABEL: _cmp_lt:
; DEFAULT: push hl
; DEFAULT: push hl
; DEFAULT-NOT: push hl
; DEFAULT: call ___cmpsf2
  %c = fcmp olt float %a, %b
  %r = zext i1 %c to i16
  ret i16 %r
}

define i16 @cmp_ueq(float %a, float %b) {
; CHECK-LABEL: _cmp_ueq:
; CHECK: call ___cmpsf2
; CHECK: call ___unordsf2
;
; DEFAULT-LABEL: _cmp_ueq:
; DEFAULT: call ___cmpsf2
; DEFAULT: call ___unordsf2
  %c = fcmp ueq float %a, %b
  %r = zext i1 %c to i16
  ret i16 %r
}

define i16 @f2i(float %a) {
; CHECK-LABEL: _f2i:
; CHECK: push hl
; CHECK: push hl
; CHECK-NOT: push hl
; CHECK: call ___fixsfsi
;
; DEFAULT-LABEL: _f2i:
; DEFAULT-NOT: push hl
; DEFAULT: jp ___fixsfsi
  %r = fptosi float %a to i16
  ret i16 %r
}

define i16 @f2u(float %a) {
; CHECK-LABEL: _f2u:
; CHECK: push hl
; CHECK: push hl
; CHECK-NOT: push hl
; CHECK: call ___fixunssfsi
;
; DEFAULT-LABEL: _f2u:
; DEFAULT-NOT: push hl
; DEFAULT: jp ___fixunssfsi
  %r = fptoui float %a to i16
  ret i16 %r
}

define float @i2f(i16 %a) {
; CHECK-LABEL: _i2f:
; CHECK: push hl
; CHECK: push hl
; CHECK-NOT: push hl
; CHECK: call ___floatsisf
;
; DEFAULT-LABEL: _i2f:
; DEFAULT-NOT: push hl
; DEFAULT: jp ___floatsisf
  %r = sitofp i16 %a to float
  ret float %r
}

define float @u2f(i16 %a) {
; CHECK-LABEL: _u2f:
; CHECK: push hl
; CHECK: push hl
; CHECK-NOT: push hl
; CHECK: call ___floatunsisf
;
; DEFAULT-LABEL: _u2f:
; DEFAULT-NOT: push hl
; DEFAULT: jp ___floatunsisf
  %r = uitofp i16 %a to float
  ret float %r
}
