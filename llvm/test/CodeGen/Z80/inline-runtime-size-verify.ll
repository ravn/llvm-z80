; RUN: llc -mtriple=z80 -mattr=+inline-i16-runtime -z80-verify-inline-runtime-size < %s | FileCheck %s
;
; #240 drift guard for the inline-runtime pseudos (MUL16 / UDIV16 / UMOD16 /
; SDIV16 / SMOD16).  Their sizes in Z80InstrInfo::getInstSizeInBytes are
; load-bearing: the pseudos are alive during BranchRelaxation (which sizes any
; branch jumping *over* them) and are only expanded afterwards, in
; Z80ExpandPseudo.  -z80-verify-inline-runtime-size makes that pass sum the
; whole function's getInstSizeInBytes immediately before and after each such
; expansion and report_fatal_error if the reported pseudo size does not equal
; the real byte count of the expansion.  If any expansion here is changed
; without updating getInstSizeInBytes, llc aborts and this test fails.  The
; CHECK lines additionally pin the expansion shape.  +inline-i16-runtime is
; required to select the inline pseudos instead of the __*hi3 runtime calls.

define i16 @mul16(i16 %a, i16 %b) {
; CHECK-LABEL: mul16:
; CHECK:       add hl,hl
; CHECK:       djnz
; CHECK:       ex de,hl
  %r = mul i16 %a, %b
  ret i16 %r
}

define i16 @udiv16(i16 %a, i16 %b) {
; CHECK-LABEL: udiv16:
; CHECK:       adc hl,hl
; CHECK:       sbc hl,de
  %r = udiv i16 %a, %b
  ret i16 %r
}

define i16 @umod16(i16 %a, i16 %b) {
; CHECK-LABEL: umod16:
; CHECK:       adc hl,hl
; CHECK:       sbc hl,de
  %r = urem i16 %a, %b
  ret i16 %r
}

define i16 @sdiv16(i16 %a, i16 %b) {
; CHECK-LABEL: sdiv16:
; CHECK:       sbc hl,de
  %r = sdiv i16 %a, %b
  ret i16 %r
}

define i16 @smod16(i16 %a, i16 %b) {
; CHECK-LABEL: smod16:
; CHECK:       sbc hl,de
  %r = srem i16 %a, %b
  ret i16 %r
}
