; ravn/llvm-z80 #283: at -O3 the 32-bit multiply runtime call is routed to the
; signed-magnitude fast variant __mulsi3_fast.  That variant takes the operand
; magnitudes first, so when the operands fit 16 bits (the dominant
; (long)i16*i16 fixed-point case) it hits the 32->16x16 demote fast path in the
; z88dk classic core -- ~2x faster than the always-32x32 default.  The low 32
; bits of a signed vs unsigned product are identical, so this is a correct
; drop-in for every 32-bit multiply.  It costs a small abs/negate overhead on
; genuine-32-bit operands (no demote), so it is a speculative
; speed-over-predictability tradeoff and is gated to -O3 only; every other opt
; level (-O0/-O1/-O2/-Os/-Oz) keeps the default __mulsi3, so production firmware
; (built at -Os) is untouched.  Mirrors the #244 __*hi3_fast div/mod routing.
;
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O3 < %s | FileCheck %s --check-prefix=FAST
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s --check-prefix=SMALL
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s --check-prefix=SMALL

define i32 @mul32(i32 %a, i32 %b) {
; FAST-LABEL: _mul32:
; FAST:        call ___mulsi3_fast
; SMALL-LABEL: _mul32:
; SMALL:       call ___mulsi3{{$}}
  %r = mul i32 %a, %b
  ret i32 %r
}

; (long)i16 * i16 -- the fixed-point shape that motivates the routing.
define i32 @wmul(i16 %a, i16 %b) {
; FAST-LABEL: _wmul:
; FAST:        call ___mulsi3_fast
; SMALL-LABEL: _wmul:
; SMALL:       call ___mulsi3{{$}}
  %sa = sext i16 %a to i32
  %sb = sext i16 %b to i32
  %r = mul i32 %sa, %sb
  ret i32 %r
}
