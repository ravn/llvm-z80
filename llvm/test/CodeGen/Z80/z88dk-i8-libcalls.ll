; RUN: llc -mtriple=z80-unknown-none-z88dk -z80-asm-format=z88dk -O2 < %s | FileCheck %s --check-prefix=Z88DK
; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s --check-prefix=ELF
; For 200/7, the existing core returns L=28 and E=4 (not A).

define i8 @qdiv(i8 %a, i8 %b) minsize {
; Z88DK-LABEL: _qdiv:
; Z88DK: call l_fast_divu_8_8x8
; Z88DK-NEXT: ld a,l
; Z88DK-NEXT: ret
; ELF-LABEL: _qdiv:
; ELF: call ___udivqi3
  %q = udiv i8 %a, %b
  ret i8 %q
}

define i8 @qmod(i8 %a, i8 %b) minsize {
; Z88DK-LABEL: _qmod:
; Z88DK: call l_fast_divu_8_8x8
; Z88DK-NEXT: ld a,e
; Z88DK-NEXT: ret
; ELF-LABEL: _qmod:
; ELF: call ___umodqi3
  %r = urem i8 %a, %b
  ret i8 %r
}
