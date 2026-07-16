; RUN: llc -mtriple=z80 -O1 < %s | FileCheck %s

; A function with NO formal arguments that returns a value larger than 4 bytes
; (double / i64 / large struct) must still set up the hidden sret pointer for
; return-value demotion.  Regression test: Z80CallLowering::lowerFormalArguments
; used to early-return on F.arg_empty() BEFORE the sret demotion block ran, which
; left FLI.DemoteRegister == $noreg and lowered the return store through a null
; base -> legalizer crash ("unable to legalize G_STORE s16 into unknown-address
; + 6") or a corrupt 8-byte return.
;
; The sret pointer is the first stack argument; with a frame pointer the callee
; reads it from (ix+4)/(ix+5) and stores the result through it.  For 5.0 the top
; 16-bit word is 0x4014 = 16404, so a correct little-endian layout stores
; "ld de,16404" into the high word of the sret buffer.

define double @ret_double_noarg() nounwind {
; CHECK-LABEL: ret_double_noarg:
; CHECK:       ld c,(ix+4)
; CHECK:       ld b,(ix+5)
; CHECK:       ld de,16404
; CHECK:       ret
  ret double 5.000000e+00
}

define i64 @ret_i64_noarg() nounwind {
; CHECK-LABEL: ret_i64_noarg:
; CHECK:       ld c,(ix+4)
; CHECK:       ld b,(ix+5)
; CHECK:       ld de,5
; CHECK:       ret
  ret i64 5
}

; Value from a computation (libcall result), not a constant -- same sret path.
define double @ret_double_call_noarg() nounwind {
; CHECK-LABEL: ret_double_call_noarg:
; CHECK:       ld c,(ix+4)
; CHECK:       ld b,(ix+5)
; CHECK:       ret
  %v = call double @extern_d()
  %r = fadd double %v, 1.000000e+00
  ret double %r
}

declare double @extern_d()
