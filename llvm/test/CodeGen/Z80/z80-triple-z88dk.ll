; RUN: llc -mtriple=z80-unknown-none-z88dk < %s | FileCheck %s --check-prefix=Z88DK
; RUN: llc -mtriple=z80 < %s | FileCheck %s --check-prefix=DEFAULT
; RUN: llc -mtriple=z80-unknown-none-z88dk -z80-asm-format=sdasz80 -z80-float-sdcccall0=false < %s | FileCheck %s --check-prefix=OVERRIDE
;
; The z80-unknown-none-z88dk target triple activates by default:
; 1. Native z80asm assembly output format (GLOBAL directives, SECTION code_compiler, no leading dots).
; 2. sdcccall(0) calling convention for f32 operations (all arguments pushed on stack, matching math32).
;
; Command-line options (-z80-asm-format, -z80-float-sdcccall0) can explicitly override these defaults.

define void @test_func() {
; Z88DK:        SECTION code_compiler
; Z88DK-NEXT:   GLOBAL _test_func
; Z88DK-LABEL:  _test_func:
; Z88DK:        ret
;
; DEFAULT:      .globl _test_func
; DEFAULT-LABEL: _test_func:
; DEFAULT:      ret
;
; OVERRIDE:     .globl _test_func
; OVERRIDE-LABEL: _test_func:
; OVERRIDE:     ret
  ret void
}

define float @fadd(float %a, float %b) {
; Z88DK-LABEL:  _fadd:
; Z88DK:        push hl
; Z88DK:        call ___addsf3
;
; DEFAULT-LABEL: _fadd:
; DEFAULT:      call ___addsf3
;
; OVERRIDE-LABEL: _fadd:
; OVERRIDE:     call ___addsf3
  %res = fadd float %a, %b
  ret float %res
}

define i1 @flt(float %a, float %b) {
; Z88DK-LABEL:  _flt:
; Z88DK:        push hl
; Z88DK:        call ___cmpsf2
;
; DEFAULT-LABEL: _flt:
; DEFAULT:      call ___cmpsf2
  %res = fcmp olt float %a, %b
  ret i1 %res
}

define float @sitofp(i16 %a) {
; Z88DK-LABEL:  _sitofp:
; Z88DK:        push hl
; Z88DK:        call ___floatsisf
;
; DEFAULT-LABEL: _sitofp:
; DEFAULT-NOT:  push hl
; DEFAULT:      call ___floatsisf
  %res = sitofp i16 %a to float
  ret float %res
}
