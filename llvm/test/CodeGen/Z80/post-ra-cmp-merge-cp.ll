; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; Z80PostRACompareMerge correctness regression.
;
; The pass removes a redundant OR A when the Z flag is already set by a
; preceding instruction whose Z reflects A's value (XOR r, AND n with
; result in A, etc.). It MUST NOT remove OR A after a CP instruction,
; because CP sets Z based on (A == operand), not (A == 0).
;
; Pre-existing bug exposed by issue #60 cross-block LD A,r removal:
; setsZForA() listed CP_n/CP_r/CP_HLind/CP_IXd as Z-for-A, but those
; aren't.

declare i8 @compute()

; Build a function whose blocks are arranged so that on one path the
; predecessor ends with CP imm and the successor's first flag-setter is
; OR A on a value distinct from `imm`. The OR A must NOT be removed.
;
; In the asm, after `cp #2`, there must be an explicit OR A (or another
; A==0 test) before the JR Z that branches to the "is zero" return.
define i8 @cp_then_or_a_must_remain() {
; CHECK-LABEL: _cp_then_or_a_must_remain:
; CHECK:       call _compute
; CHECK:       cp   #2
; CHECK:       or   a
; CHECK:       j{{[rp]}} z,
entry:
  %v = call i8 @compute()
  %is2 = icmp eq i8 %v, 2
  br i1 %is2, label %ret_two, label %not_two
ret_two:
  ret i8 2
not_two:
  %is0 = icmp eq i8 %v, 0
  br i1 %is0, label %ret_zero, label %ret_other
ret_zero:
  ret i8 0
ret_other:
  ret i8 %v
}
