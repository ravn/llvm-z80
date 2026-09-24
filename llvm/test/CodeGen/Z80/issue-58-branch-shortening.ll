; RUN: llc -mtriple=z80 -O1 < %s | FileCheck %s
;
; ravn/llvm-z80#58: JP -> JR branch shortening.
; Convert unconditional and conditional JP instructions to their 2-byte JR
; equivalents when target is within short jump range (+-127 bytes).
; Out-of-range jumps are subsequently relaxed back to JP by BranchRelaxation.

; CHECK-LABEL: halt:
; CHECK:       .LBB0_1:
; CHECK:       jr	.LBB0_1
; CHECK-NOT:   jp
; C source:
;   void halt(void) { while (1) {} }   /* JP → JR .self (2 B vs 3 B) */
;   void tight_loop(unsigned char *p, unsigned char n) {
;       do { p[0]++; } while (--n);    /* JR NZ */
;   }
; JP (3 B) → JR (2 B) when target is within ±127 bytes.
; Out-of-range jumps are relaxed back to JP by BranchRelaxation.

define void @halt() {
entry:
  br label %loop

loop:
  br label %loop
}
