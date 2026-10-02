; RUN: llc -mtriple=z80-unknown-none-z88dk -z80-asm-format=z88dk -O2 < %s | FileCheck %s --check-prefix=Z88DK
; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s --check-prefix=ELF
;
; z88dk triple: i32 arithmetic uses sdcccall(0) (stack args).
; Symbol names use \01 prefix to bypass Mach-O '_' mangling so the
; z88dk linker sees __mulsi3, __divsi3 etc. (two underscores, not three).
; ELF path uses CallingConv::C (unchanged).

define i32 @mul(i32 %a, i32 %b) {
; Z88DK-LABEL: _mul:
; Z88DK:         call __mulsi3
; Z88DK-NOT:     call ___mulsi3
;
; ELF-LABEL: _mul:
; ELF:           call ___mulsi3
  %r = mul i32 %a, %b
  ret i32 %r
}

define i32 @sdiv(i32 %a, i32 %b) {
; Z88DK-LABEL: _sdiv:
; Z88DK:         call __divsi3
;
; ELF-LABEL: _sdiv:
; ELF:           call ___divsi3
  %r = sdiv i32 %a, %b
  ret i32 %r
}

define i32 @udiv(i32 %a, i32 %b) {
; Z88DK-LABEL: _udiv:
; Z88DK:         call __udivsi3
;
; ELF-LABEL: _udiv:
; ELF:           call ___udivsi3
  %r = udiv i32 %a, %b
  ret i32 %r
}

define i32 @srem(i32 %a, i32 %b) {
; Z88DK-LABEL: _srem:
; Z88DK:         call __modsi3
;
; ELF-LABEL: _srem:
; ELF:           call ___modsi3
  %r = srem i32 %a, %b
  ret i32 %r
}

define i32 @urem(i32 %a, i32 %b) {
; Z88DK-LABEL: _urem:
; Z88DK:         call __umodsi3
;
; ELF-LABEL: _urem:
; ELF:           call ___umodsi3
  %r = urem i32 %a, %b
  ret i32 %r
}
