; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 -z80-verify-inline-runtime-size < %s | FileCheck %s --check-prefix=Z80
; RUN: llc -mtriple=z80 -mcpu=sm83 -z80-asm-format=sdasz80 -O1 -z80-verify-inline-runtime-size < %s | FileCheck %s --check-prefix=SM83
;
; Regression / drift guard for ravn/llvm-z80#267.
;
; Z80ExpandPseudo has an opt-in drift guard (-z80-verify-inline-runtime-size)
; that verifies Z80InstrInfo::getInstSizeInBytes() reports the correct byte
; count for every inline-runtime pseudo just before it is expanded. If the
; two sources of truth drift apart, the pass raises a fatal error, so llc
; would fail to produce output and FileCheck would fail with no matches.
;
; This test exercises one function per pseudo class covered by
; isInlineRuntimeSizedPseudo(). It doesn't pin the exact expansion bytes
; (those live in the dedicated per-pseudo tests) — it just ensures each
; pseudo is actually emitted and expanded under the guard on both z80 and
; sm83 without tripping the drift check.
;
; The Z80/SM83 CHECK prefixes both look for the same function label, which
; is a load-bearing signal that llc produced real output.

declare i8 @llvm.fshl.i8(i8, i8, i8)
declare i8 @llvm.fshr.i8(i8, i8, i8)
declare i8 @llvm.uadd.sat.i8(i8, i8)
declare i8 @llvm.usub.sat.i8(i8, i8)
declare i8 @llvm.sadd.sat.i8(i8, i8)
declare i8 @llvm.ssub.sat.i8(i8, i8)

; MUL8
; Z80-LABEL: _test_mul8:
; SM83-LABEL: _test_mul8:
define i8 @test_mul8(i8 %a, i8 %b) {
  %r = mul i8 %a, %b
  ret i8 %r
}

; UDIV8
; Z80-LABEL: _test_udiv8:
; SM83-LABEL: _test_udiv8:
define i8 @test_udiv8(i8 %a, i8 %b) {
  %r = udiv i8 %a, %b
  ret i8 %r
}

; UMOD8
; Z80-LABEL: _test_umod8:
; SM83-LABEL: _test_umod8:
define i8 @test_umod8(i8 %a, i8 %b) {
  %r = urem i8 %a, %b
  ret i8 %r
}

; SDIV8
; Z80-LABEL: _test_sdiv8:
; SM83-LABEL: _test_sdiv8:
define i8 @test_sdiv8(i8 %a, i8 %b) {
  %r = sdiv i8 %a, %b
  ret i8 %r
}

; SMOD8
; Z80-LABEL: _test_smod8:
; SM83-LABEL: _test_smod8:
define i8 @test_smod8(i8 %a, i8 %b) {
  %r = srem i8 %a, %b
  ret i8 %r
}

; SHL8_VAR
; Z80-LABEL: _test_shl8_var:
; SM83-LABEL: _test_shl8_var:
define i8 @test_shl8_var(i8 %a, i8 %n) {
  %r = shl i8 %a, %n
  ret i8 %r
}

; LSHR8_VAR
; Z80-LABEL: _test_lshr8_var:
; SM83-LABEL: _test_lshr8_var:
define i8 @test_lshr8_var(i8 %a, i8 %n) {
  %r = lshr i8 %a, %n
  ret i8 %r
}

; ASHR8_VAR
; Z80-LABEL: _test_ashr8_var:
; SM83-LABEL: _test_ashr8_var:
define i8 @test_ashr8_var(i8 %a, i8 %n) {
  %r = ashr i8 %a, %n
  ret i8 %r
}

; ROTL8_VAR
; Z80-LABEL: _test_rotl8_var:
; SM83-LABEL: _test_rotl8_var:
define i8 @test_rotl8_var(i8 %a, i8 %n) {
  %r = call i8 @llvm.fshl.i8(i8 %a, i8 %a, i8 %n)
  ret i8 %r
}

; ROTR8_VAR
; Z80-LABEL: _test_rotr8_var:
; SM83-LABEL: _test_rotr8_var:
define i8 @test_rotr8_var(i8 %a, i8 %n) {
  %r = call i8 @llvm.fshr.i8(i8 %a, i8 %a, i8 %n)
  ret i8 %r
}

; SHL16_VAR
; Z80-LABEL: _test_shl16_var:
; SM83-LABEL: _test_shl16_var:
define i16 @test_shl16_var(i16 %a, i16 %n) {
  %r = shl i16 %a, %n
  ret i16 %r
}

; LSHR16_VAR
; Z80-LABEL: _test_lshr16_var:
; SM83-LABEL: _test_lshr16_var:
define i16 @test_lshr16_var(i16 %a, i16 %n) {
  %r = lshr i16 %a, %n
  ret i16 %r
}

; ASHR16_VAR
; Z80-LABEL: _test_ashr16_var:
; SM83-LABEL: _test_ashr16_var:
define i16 @test_ashr16_var(i16 %a, i16 %n) {
  %r = ashr i16 %a, %n
  ret i16 %r
}

; UADDSAT8
; Z80-LABEL: _test_uaddsat8:
; SM83-LABEL: _test_uaddsat8:
define i8 @test_uaddsat8(i8 %a, i8 %b) {
  %r = call i8 @llvm.uadd.sat.i8(i8 %a, i8 %b)
  ret i8 %r
}

; USUBSAT8
; Z80-LABEL: _test_usubsat8:
; SM83-LABEL: _test_usubsat8:
define i8 @test_usubsat8(i8 %a, i8 %b) {
  %r = call i8 @llvm.usub.sat.i8(i8 %a, i8 %b)
  ret i8 %r
}

; SADDSAT8
; Z80-LABEL: _test_saddsat8:
; SM83-LABEL: _test_saddsat8:
define i8 @test_saddsat8(i8 %a, i8 %b) {
  %r = call i8 @llvm.sadd.sat.i8(i8 %a, i8 %b)
  ret i8 %r
}

; SSUBSAT8
; Z80-LABEL: _test_ssubsat8:
; SM83-LABEL: _test_ssubsat8:
define i8 @test_ssubsat8(i8 %a, i8 %b) {
  %r = call i8 @llvm.ssub.sat.i8(i8 %a, i8 %b)
  ret i8 %r
}
