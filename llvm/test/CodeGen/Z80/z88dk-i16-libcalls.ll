; RUN: llc -mtriple=z80-unknown-none-z88dk -z80-asm-format=z88dk -O2 < %s | FileCheck %s --check-prefix=Z88DK
; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s --check-prefix=ELF
;
; z88dk triple calls z88dk l_* cores directly (register protocol):
;   mul:  l_mulu_16_16x16  HL×DE → HL
;   sdiv: l_divs_16_16x16  HL÷DE → DE=quot, HL=rem
;   udiv: l_divu_16_16x16  HL÷DE → DE=quot, HL=rem
;
; ELF uses standard __mulhi3/__divmodhi4 libcalls.

define i16 @mul(i16 %a, i16 %b) {
; Z88DK-LABEL: _mul:
; Z88DK:         call l_mulu_16_16x16
; Z88DK-NOT:     call ___mulhi3
;
; ELF-LABEL: _mul:
; ELF:           call ___mulhi3
  %r = mul i16 %a, %b
  ret i16 %r
}

define i16 @sdiv(i16 %a, i16 %b) {
; Z88DK-LABEL: _sdiv:
; Z88DK:         call l_divs_16_16x16
; Z88DK-NOT:     call ___divhi3
;
; ELF-LABEL: _sdiv:
; ELF:           call ___divhi3
  %r = sdiv i16 %a, %b
  ret i16 %r
}

define i16 @srem(i16 %a, i16 %b) {
; Z88DK-LABEL: _srem:
; Z88DK:         call l_divs_16_16x16
; Z88DK-NOT:     call ___modhi3
;
; ELF-LABEL: _srem:
; ELF:           call ___modhi3
  %r = srem i16 %a, %b
  ret i16 %r
}

define i16 @udiv(i16 %a, i16 %b) {
; Z88DK-LABEL: _udiv:
; Z88DK:         call l_divu_16_16x16
; Z88DK-NOT:     call ___udivhi3
;
; ELF-LABEL: _udiv:
; ELF:           call ___udivhi3
  %r = udiv i16 %a, %b
  ret i16 %r
}

define i16 @urem(i16 %a, i16 %b) {
; Z88DK-LABEL: _urem:
; Z88DK:         call l_divu_16_16x16
; Z88DK-NOT:     call ___umodhi3
;
; ELF-LABEL: _urem:
; ELF:           call ___umodhi3
  %r = urem i16 %a, %b
  ret i16 %r
}

define {i16, i16} @sdivrem(i16 %a, i16 %b) {
; Z88DK-LABEL: _sdivrem:
; Z88DK:         call l_divs_16_16x16
;
; ELF-LABEL: _sdivrem:
; ELF:           call ___divmodhi4
  %q = sdiv i16 %a, %b
  %r = srem i16 %a, %b
  %t0 = insertvalue {i16, i16} undef, i16 %q, 0
  %t1 = insertvalue {i16, i16} %t0, i16 %r, 1
  ret {i16, i16} %t1
}
