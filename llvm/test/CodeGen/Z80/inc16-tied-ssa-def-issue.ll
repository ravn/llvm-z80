; RUN: llc -mtriple=z80 -mattr=+static-frame -O2 -verify-machineinstrs < %s | FileCheck %s
;
; ravn/llvm-z80: the i16 ==/!= -1 fast path (getMinusOneI16) builds the tied
; INC16 pseudo.  INC16 is tied ($dst = $src); in SSA the def must be a DISTINCT
; vreg (the two-address pass ties them back).  ISel previously emitted
; `%t = INC16 %t` (dst == src) -> "Multiple virtual register defs in SSA form"
; under -verify-machineinstrs, AND a real miscompile (test_38_sort_search at O1:
; 0x0007 vs 0x000F).  Fixed by giving INC16 a fresh dst vreg.
;
; This test passes only if the module verifies clean (no multiple-def) on the
; i16 !=/== -1 path.


define i16 @is_minus_one(i16 %x) {
  %c = icmp ne i16 %x, -1
; CHECK-LABEL: is_minus_one:
; CHECK:      	ld	a,h
; CHECK:      	cpl
; CHECK:      	ld	b,a
; CHECK:      	ld	a,l
; CHECK:      	cpl
; CHECK:      	or	b
; CHECK:      	jr	z,.LBB0_2
; CHECK:      	ld	de,1
; CHECK:      	ret
; CHECK:      	ld	de,0
; CHECK:      	ret
  br i1 %c, label %ne, label %eq
ne:
  ret i16 1
eq:
  ret i16 0
}
