; RUN: llc -mtriple=z80-unknown-none-z88dk -z80-asm-format=z88dk -verify-machineinstrs -O2 < %s | FileCheck %s --check-prefix=Z88DK
; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s --check-prefix=ELF
; RUN: llc -mtriple=sm83 -verify-machineinstrs -O2 < %s | FileCheck %s --check-prefix=ELF
; RUN: llc -mtriple=sm83-unknown-none-z88dk -verify-machineinstrs -O2 < %s | FileCheck %s --check-prefix=SM83
;
; z88dk triple: i32 arithmetic calls existing cores via EXX.
; C repro: long quotient(long a, long b) { return a / b; }
; The core returns DE:HL=quotient and alternate DE:HL=remainder.
; IX must survive the call, including in dynamic-frame functions.
; ELF path uses CallingConv::C (unchanged).

define i32 @mul(i32 %a, i32 %b) {
; Z88DK-LABEL: _mul:
; Z88DK:         exx
; Z88DK:         push ix
; Z88DK-NEXT:    call l_mulu_32_32x32
; Z88DK-NEXT:    pop ix
; Z88DK-NOT:     call ___mulsi3
;
; ELF-LABEL: _mul:
; ELF:           call ___mulsi3
; SM83-LABEL: _mul:
; SM83: call __mulsi3{{$}}
  %r = mul i32 %a, %b
  ret i32 %r
}

define i32 @sdiv(i32 %a, i32 %b) {
; Z88DK-LABEL: _sdiv:
; Z88DK:         ld c,e
; Z88DK-NEXT:    ld b,d
; Z88DK-NEXT:    ex de,hl
; Z88DK-NEXT:    ld l,c
; Z88DK-NEXT:    ld h,b
; Z88DK-NEXT:    exx
; Z88DK:         push ix
; Z88DK-NEXT:    call l_divs_32_32x32
; Z88DK-NEXT:    pop ix
; Z88DK-NEXT:    ld c,e
; Z88DK-NEXT:    ld b,d
; Z88DK-NEXT:    ex de,hl
; Z88DK-NEXT:    ld l,c
; Z88DK-NEXT:    ld h,b
;
; ELF-LABEL: _sdiv:
; ELF:           call ___divsi3
; SM83-LABEL: _sdiv:
; SM83: call __divsi3{{$}}
  %r = sdiv i32 %a, %b
  ret i32 %r
}

define i32 @fused(i32 %a, i32 %b, ptr %remainder) {
; Z88DK-LABEL: _fused:
; Z88DK:         exx
; Z88DK:         push ix
; Z88DK-NEXT:    call l_divs_32_32x32
; Z88DK-NEXT:    pop ix
; Z88DK:         exx
; ELF-LABEL: _fused:
; ELF:           call ___divmodsi4
; SM83-LABEL: _fused:
; SM83: call __divmodsi4{{$}}
  %q = sdiv i32 %a, %b
  %r = srem i32 %a, %b
  store i32 %r, ptr %remainder, align 1
  ret i32 %q
}

define i32 @udiv(i32 %a, i32 %b) {
; Z88DK-LABEL: _udiv:
; Z88DK:         exx
; Z88DK:         push ix
; Z88DK-NEXT:    call l_divu_32_32x32
; Z88DK-NEXT:    pop ix
;
; ELF-LABEL: _udiv:
; ELF:           call ___udivsi3
; SM83-LABEL: _udiv:
; SM83: call __udivsi3{{$}}
  %r = udiv i32 %a, %b
  ret i32 %r
}

define i32 @srem(i32 %a, i32 %b) {
; Z88DK-LABEL: _srem:
; Z88DK:         exx
; Z88DK:         push ix
; Z88DK-NEXT:    call l_divs_32_32x32
; Z88DK-NEXT:    pop ix
; Z88DK-NEXT:    exx
;
; ELF-LABEL: _srem:
; ELF:           call ___modsi3
; SM83-LABEL: _srem:
; SM83: call __modsi3{{$}}
  %r = srem i32 %a, %b
  ret i32 %r
}

define i32 @urem(i32 %a, i32 %b) {
; Z88DK-LABEL: _urem:
; Z88DK:         exx
; Z88DK:         push ix
; Z88DK-NEXT:    call l_divu_32_32x32
; Z88DK-NEXT:    pop ix
; Z88DK-NEXT:    exx
;
; ELF-LABEL: _urem:
; ELF:           call ___umodsi3
; SM83-LABEL: _urem:
; SM83: call __umodsi3{{$}}
  %r = urem i32 %a, %b
  ret i32 %r
}

define i32 @ufused(i32 %a, i32 %b, ptr %remainder) {
; Z88DK-LABEL: _ufused:
; Z88DK: call l_divu_32_32x32{{$}}
; ELF-LABEL: _ufused:
; ELF: call ___udivmodsi4{{$}}
; SM83-LABEL: _ufused:
; SM83: call __udivmodsi4{{$}}
  %q = udiv i32 %a, %b
  %r = urem i32 %a, %b
  store i32 %r, ptr %remainder, align 1
  ret i32 %q
}
