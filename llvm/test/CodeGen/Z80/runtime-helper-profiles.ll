; RUN: llc -mtriple=z80 -verify-machineinstrs -O2 < %s | FileCheck %s --check-prefix=BASE
; RUN: llc -mtriple=sm83 -verify-machineinstrs -O2 < %s | FileCheck %s --check-prefix=BASE
; RUN: llc -mtriple=z80-unknown-none-z88dk -verify-machineinstrs -O2 < %s | FileCheck %s --check-prefix=MATH32
; RUN: llc -mtriple=sm83-unknown-none-z88dk -verify-machineinstrs -O2 < %s | FileCheck %s --check-prefix=MATH32
; RUN: llc -mtriple=z80-unknown-none-z88dk -verify-machineinstrs -O2 < %s | FileCheck %s --check-prefix=ABI
;
; Ordinary helpers keep their runtime ABI even in a smallc (cc129) or
; sdcccall(1) (C) caller. Fast flags select fast compiler-rt helpers, but
; z88dk uses the same math32 entries with or without those flags.
; C repro: float add(float a, float b) { return a + b; }
; C repro: long to_int(float a) { return (long)a; }

define float @add(float %a, float %b) {
; BASE-LABEL: _add:
; BASE: call ___addsf3{{$}}
; MATH32-LABEL: _add:
; MATH32: call cm32_sdcc_fsadd{{$}}
  %r = fadd float %a, %b
  ret float %r
}

define float @sub(float %a, float %b) {
; BASE-LABEL: _sub:
; BASE: call ___subsf3{{$}}
; MATH32-LABEL: _sub:
; MATH32: call cm32_sdcc_fssub{{$}}
  %r = fsub float %a, %b
  ret float %r
}

define float @mul(float %a, float %b) {
; BASE-LABEL: _mul:
; BASE: call ___mulsf3{{$}}
; MATH32-LABEL: _mul:
; MATH32: call cm32_sdcc_fsmul{{$}}
  %r = fmul float %a, %b
  ret float %r
}

define float @div(float %a, float %b) {
; BASE-LABEL: _div:
; BASE: call ___divsf3{{$}}
; MATH32-LABEL: _div:
; MATH32: call cm32_sdcc_fsdiv{{$}}
  %r = fdiv float %a, %b
  ret float %r
}

define float @fast_add(float %a, float %b) {
; BASE-LABEL: _fast_add:
; BASE: call ___addsf3_fast{{$}}
; MATH32-LABEL: _fast_add:
; MATH32: call cm32_sdcc_fsadd{{$}}
  %r = fadd nnan ninf nsz float %a, %b
  ret float %r
}

define float @fast_sub(float %a, float %b) {
; BASE-LABEL: _fast_sub:
; BASE: call ___subsf3_fast{{$}}
; MATH32-LABEL: _fast_sub:
; MATH32: call cm32_sdcc_fssub{{$}}
  %r = fsub nnan ninf nsz float %a, %b
  ret float %r
}

define float @fast_mul(float %a, float %b) {
; BASE-LABEL: _fast_mul:
; BASE: call ___mulsf3_fast{{$}}
; MATH32-LABEL: _fast_mul:
; MATH32: call cm32_sdcc_fsmul{{$}}
  %r = fmul nnan ninf nsz float %a, %b
  ret float %r
}

define float @fast_div(float %a, float %b) {
; BASE-LABEL: _fast_div:
; BASE: call ___divsf3_fast{{$}}
; MATH32-LABEL: _fast_div:
; MATH32: call cm32_sdcc_fsdiv{{$}}
  %r = fdiv nnan ninf nsz float %a, %b
  ret float %r
}

define float @partial_flags(float %a, float %b) {
; BASE-LABEL: _partial_flags:
; BASE: call ___addsf3{{$}}
; MATH32-LABEL: _partial_flags:
; MATH32: call cm32_sdcc_fsadd{{$}}
  %r = fadd nnan ninf float %a, %b
  ret float %r
}

; Scalarization and intrinsic lowering must still use the custom helper path.
define <2 x float> @vector_add(<2 x float> %a, <2 x float> %b) {
; BASE-LABEL: _vector_add:
; BASE: call ___addsf3{{$}}
; BASE: call ___addsf3{{$}}
; MATH32-LABEL: _vector_add:
; MATH32: call cm32_sdcc_fsadd{{$}}
; MATH32: call cm32_sdcc_fsadd{{$}}
  %r = fadd <2 x float> %a, %b
  ret <2 x float> %r
}

declare i32 @llvm.fptosi.sat.i32.f32(float)

define i32 @saturating_convert(float %a) {
; BASE-LABEL: _saturating_convert:
; BASE: call ___fixsfsi{{$}}
; MATH32-LABEL: _saturating_convert:
; MATH32: call cm32_sdcc___fs2sint{{$}}
  %r = call i32 @llvm.fptosi.sat.i32.f32(float %a)
  ret i32 %r
}

define i32 @to_int(float %a) {
; BASE-LABEL: _to_int:
; BASE: call ___fixsfsi{{$}}
; MATH32-LABEL: _to_int:
; MATH32: call cm32_sdcc___fs2sint{{$}}
  %r = fptosi float %a to i32
  ret i32 %r
}

define i32 @to_uint(float %a) {
; BASE-LABEL: _to_uint:
; BASE: call ___fixunssfsi{{$}}
; MATH32-LABEL: _to_uint:
; MATH32: call cm32_sdcc___fs2uint{{$}}
  %r = fptoui float %a to i32
  ret i32 %r
}

define float @from_int(i32 %a) {
; BASE-LABEL: _from_int:
; BASE: call ___floatsisf{{$}}
; MATH32-LABEL: _from_int:
; MATH32: call cm32_sdcc___slong2fs{{$}}
  %r = sitofp i32 %a to float
  ret float %r
}

define float @from_uint(i32 %a) {
; BASE-LABEL: _from_uint:
; BASE: call ___floatunsisf{{$}}
; MATH32-LABEL: _from_uint:
; MATH32: call cm32_sdcc___ulong2fs{{$}}
  %r = uitofp i32 %a to float
  ret float %r
}

; 1.0f = 0x3f800000; 2.0f = 0x40000000. The runtime, not the
; caller, determines which operand is on top of the stack.
@lhs = global float 1.0
@rhs = global float 2.0

define cc129 float @smallc_sub() {
; BASE-LABEL: _smallc_sub:
; BASE: call ___subsf3{{$}}
; MATH32-LABEL: _smallc_sub:
; MATH32: call cm32_sdcc_fssub{{$}}
; ABI-LABEL: _smallc_sub:
; ABI: ld bc,(_lhs)
; ABI: ld de,(_lhs+2)
; ABI-NEXT: ld bc,(_rhs)
; ABI-NEXT: ld hl,(_rhs+2)
; ABI-NEXT: push hl
; ABI-NEXT: ld l,c
; ABI-NEXT: ld h,b
; ABI-NEXT: push hl
; ABI-NEXT: ex de,hl
; ABI-NEXT: push hl
; ABI-NEXT: ld hl,({{.*}}_frame)
; ABI-NEXT: push hl
; ABI-NEXT: call cm32_sdcc_fssub
; ABI-NEXT: pop af
; ABI-NEXT: pop af
; ABI-NEXT: pop af
; ABI-NEXT: pop af
; ABI-NEXT: ret
  %a = load volatile float, ptr @lhs
  %b = load volatile float, ptr @rhs
  %r = fsub float %a, %b
  ret float %r
}

define float @sdcccall1_sub() {
; BASE-LABEL: _sdcccall1_sub:
; BASE: call ___subsf3{{$}}
; MATH32-LABEL: _sdcccall1_sub:
; MATH32: call cm32_sdcc_fssub{{$}}
; ABI-LABEL: _sdcccall1_sub:
; ABI: call cm32_sdcc_fssub
; ABI-NEXT: ld c,e
; ABI-NEXT: ld b,d
; ABI-NEXT: pop af
; ABI-NEXT: pop af
; ABI-NEXT: pop af
; ABI-NEXT: pop af
; ABI-NEXT: ex de,hl
; ABI-NEXT: ld l,c
; ABI-NEXT: ld h,b
; ABI-NEXT: ret
  %a = load volatile float, ptr @lhs
  %b = load volatile float, ptr @rhs
  %r = fsub float %a, %b
  ret float %r
}
