; RUN: llc -mtriple=z80-unknown-none-z88dk < %s | FileCheck %s --check-prefix=Z88DK
; RUN: llc -mtriple=z80 < %s | FileCheck %s --check-prefix=DEFAULT
;
; The z80-unknown-none-z88dk target triple activates by default:
; 1. Native z80asm assembly output format (GLOBAL directives, SECTION code_compiler, no leading dots).
; 2. Direct EXX-protocol calls to math32 cores for f32 operations (TODO: not yet implemented).

define void @test_func() {
; Z88DK:        SECTION code_compiler
; Z88DK-NEXT:   GLOBAL _test_func
; Z88DK-LABEL:  _test_func:
; Z88DK:        ret
;
; DEFAULT:      .globl _test_func
; DEFAULT-LABEL: _test_func:
; DEFAULT:      ret
  ret void
}

; Math32 EXX-protocol lowering is not yet implemented.
; For now, both triples fall back to the default C ABI libcall.
; These tests will be updated when EXX lowering is implemented.

define float @fadd(float %a, float %b) {
; Z88DK-LABEL:  _fadd:
; Z88DK:        call cm32_sdcc_fsadd
;
; DEFAULT-LABEL: _fadd:
; DEFAULT:      call ___addsf3
  %res = fadd float %a, %b
  ret float %res
}

define i1 @flt(float %a, float %b) {
; Z88DK-LABEL:  _flt:
; Z88DK:        call ___cmpsf2
;
; DEFAULT-LABEL: _flt:
; DEFAULT:      call ___cmpsf2
  %res = fcmp olt float %a, %b
  ret i1 %res
}

define float @sitofp(i16 %a) {
; Z88DK-LABEL:  _sitofp:
; Z88DK:        call cm32_sdcc___slong2fs
;
; DEFAULT-LABEL: _sitofp:
; DEFAULT:      call ___floatsisf
  %res = sitofp i16 %a to float
  ret float %res
}
