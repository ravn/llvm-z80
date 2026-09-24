; ravn/llvm-z80 #244: at -O3 the i16 div/mod runtime calls are routed to the
; fully-unrolled _fast variants (djnz loop removed, ~14% faster on div-heavy
; code); every other opt level keeps the small default routines.
;
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O3 < %s | FileCheck %s --check-prefix=FAST
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s --check-prefix=SMALL
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s --check-prefix=SMALL

; C source:
;   // ravn/llvm-z80#244: at -O3 the i16 sdiv/udiv/srem/urem libcalls are
;   // routed to _fast variants (DJNZ loop removed, ~14% faster); all other
;   // opt levels keep the compact default routines.
;   //
;   int16_t  sdiv16(int16_t  a, int16_t  b) { return a / b; }
;   uint16_t udiv16(uint16_t a, uint16_t b) { return a / b; }
;   int16_t  smod16(int16_t  a, int16_t  b) { return a % b; }
;   uint16_t umod16(uint16_t a, uint16_t b) { return a % b; }
define i16 @sdiv16(i16 %a, i16 %b) {
; FAST-LABEL: _sdiv16:
; FAST:        {{call|jp}} ___divhi3_fast
; CHECK-LABEL: sdiv16:
; CHECK:      	call	___divhi3
; CHECK:      	ret
; SMALL-LABEL: _sdiv16:
; SMALL:       {{call|jp}} ___divhi3{{$}}
  %r = sdiv i16 %a, %b
  ret i16 %r
}

define i16 @udiv16(i16 %a, i16 %b) {
; FAST-LABEL: _udiv16:
; FAST:        {{call|jp}} ___udivhi3_fast
; SMALL-LABEL: _udiv16:
; SMALL:       {{call|jp}} ___udivhi3{{$}}
  %r = udiv i16 %a, %b
; CHECK-LABEL: udiv16:
; CHECK:      	call	___udivhi3
; CHECK:      	ret
  ret i16 %r
}

define i16 @srem16(i16 %a, i16 %b) {
; FAST-LABEL: _srem16:
; FAST:        {{call|jp}} ___modhi3_fast
; SMALL-LABEL: _srem16:
; SMALL:       {{call|jp}} ___modhi3{{$}}
  %r = srem i16 %a, %b
  ret i16 %r
}

; CHECK-LABEL: srem16:
; CHECK:      	call	___modhi3
; CHECK:      	ret
define i16 @urem16(i16 %a, i16 %b) {
; FAST-LABEL: _urem16:
; FAST:        {{call|jp}} ___umodhi3_fast
; SMALL-LABEL: _urem16:
; SMALL:       {{call|jp}} ___umodhi3{{$}}
  %r = urem i16 %a, %b
  ret i16 %r
}

; Fused signed div+rem (single __divhi3 call yields quot+rem): the path the `e`
; benchmark actually takes.  Must also flip to _fast at -O3.
define i16 @sdivrem16(i16 %a, i16 %b) {
; CHECK-LABEL: urem16:
; CHECK:      	call	___umodhi3
; CHECK:      	ret
; FAST-LABEL: _sdivrem16:
; FAST:        call ___divhi3_fast
; SMALL-LABEL: _sdivrem16:
; SMALL:       call ___divhi3{{$}}
  %q = sdiv i16 %a, %b
  %r = srem i16 %a, %b
  %s = add i16 %q, %r
  ret i16 %s
}

; A function marked optsize must keep the small routine even at -O3.
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O3 < %s | FileCheck %s --check-prefix=OPTSIZE
define i16 @sdiv16_optsize(i16 %a, i16 %b) optsize {
; OPTSIZE-LABEL: _sdiv16_optsize:
; OPTSIZE:       {{call|jp}} ___divhi3{{$}}
  %r = sdiv i16 %a, %b
  ret i16 %r
}
; CHECK-LABEL: sdivrem16:
; CHECK:      	call	___divhi3
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ex	de,hl
; CHECK:      	add	hl,bc
; CHECK:      	ex	de,hl
; CHECK:      	ret
; CHECK-LABEL: sdiv16_optsize:
; CHECK:      	call	___divhi3
; CHECK:      	ret
