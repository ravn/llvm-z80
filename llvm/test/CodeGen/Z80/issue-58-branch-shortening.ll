; RUN: llc -mtriple=z80 -O1 < %s | FileCheck %s
;
; ravn/llvm-z80#58: JP -> JR branch shortening.
; Convert unconditional and conditional JP instructions to their 2-byte JR
; equivalents when target is within short jump range (+-127 bytes).
; Out-of-range jumps are subsequently relaxed back to JP by BranchRelaxation.

; CHECK-LABEL: halt:
; CHECK:       .LBB0_1:
; CHECK-NEXT:  jr	.LBB0_1
; CHECK-NOT:   jp
define void @halt() {
entry:
  br label %loop

loop:
  br label %loop
}
