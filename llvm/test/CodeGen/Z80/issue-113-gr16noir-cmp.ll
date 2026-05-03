; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; ravn/llvm-z80#113 — pseudo expansions emit undocumented IXH/IXL/IYH/
; IYL ops without +undocumented gating.
;
; Sibling of #112.  XOR_CMP_EQ16, XOR_CMP_NE16, XOR_CMP_Z16 (and
; SM83_CMP_Z16) call `getLD8RegOpcode(A, lhs_hi)` and `getXOROpcode(
; rhs_hi)` in the standard expansion path.  These lookup tables DO
; have IXH/IXL/IYH/IYL entries — so unlike LSHR16 / ASHR16 the
; failure mode is not opcode-0 PHI but valid-but-undocumented
; XOR IXH / LD A,IYL etc.  That is a policy violation when the
; subtarget does not have +undocumented enabled.
;
; Fix: switch the pseudo operands to `GR16NoIR` (BC, DE, HL only).
; Regalloc cannot pick IX/IY for these operands by construction;
; if the source value lives in IX/IY a COPY is inserted before the
; pseudo.
;
; With IX/IY currently reserved (`Z80RegisterInfo::getReservedRegs`)
; this is a no-op for the hot path — regalloc never tries to put a
; comparison operand in IX/IY anyway.  The test is a regression guard
; against widening the operand class back to GR16.  When IX/IY
; un-reservation lands (#38), this test would have caught the
; undocumented-opcode regression.

; CHECK-LABEL: _eq_i16:
; CHECK:     xor	{{[bcdehl]}}
; CHECK:     xor	{{[bcdehl]}}
; CHECK-NOT: xor	ix
; CHECK-NOT: xor	iy
; CHECK-NOT: xor	{{[ixy]}}{{[hl]}}
; CHECK:     ret
define zeroext i1 @eq_i16(i16 %a, i16 %b) {
  %r = icmp eq i16 %a, %b
  ret i1 %r
}

; CHECK-LABEL: _ne_i16:
; CHECK:     xor	{{[bcdehl]}}
; CHECK:     xor	{{[bcdehl]}}
; CHECK-NOT: xor	ix
; CHECK-NOT: xor	iy
; CHECK-NOT: xor	{{[ixy]}}{{[hl]}}
; CHECK:     ret
define zeroext i1 @ne_i16(i16 %a, i16 %b) {
  %r = icmp ne i16 %a, %b
  ret i1 %r
}

; Branch form — exercises XOR_CMP_Z16 (fused compare-and-branch).
; CHECK-LABEL: _br_eq_i16:
; CHECK:     xor	{{[bcdehl]}}
; CHECK:     xor	{{[bcdehl]}}
; CHECK-NOT: xor	ix
; CHECK-NOT: xor	iy
; CHECK-NOT: xor	{{[ixy]}}{{[hl]}}
; CHECK:     jr	nz,
define void @br_eq_i16(i16 %a, i16 %b, ptr %p) {
entry:
  %c = icmp eq i16 %a, %b
  br i1 %c, label %t, label %f
t:
  store i8 1, ptr %p
  ret void
f:
  store i8 0, ptr %p
  ret void
}
