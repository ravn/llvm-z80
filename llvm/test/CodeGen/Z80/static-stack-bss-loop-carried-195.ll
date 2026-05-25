; RUN: llc -mtriple=z80 -mattr=+static-stack -O1 < %s | FileCheck %s

; ravn/llvm-z80#195: a loop-carried i32 value whose high half is homed in a BSS
; slot under +static-stack.  The loop reads the slot at the top (`LD HL,(slot)`,
; the back-edge reload) and writes the shifted half back at the bottom
; (`LD (slot),BC`).  The BSS-spill->PUSH/POP peephole matched the bottom
; store + its same-block BC reload and converted them to PUSH/POP, *dropping the
; store* -- but its safety checks missed the top-of-loop read (the forward orphan
; scan only looks after the store; the "used elsewhere" check skips the current
; block and only matched the same register class).  Result: the slot was never
; written, the loop's high half never decreased, and popcount32(0xA5A5A5A5) hung
; (infinite loop) at O1/O2/O3/Os.
;
; Fix: also bail when the slot is accessed earlier in the same block, before the
; matched store (the loop-carried-reload signature).  The high-half write-back
; must survive in the loop body.

define dso_local zeroext i8 @popcount32(i32 noundef %0) {
  %2 = icmp eq i32 %0, 0
  br i1 %2, label %11, label %3

3:
  %4 = phi i8 [ %8, %3 ], [ 0, %1 ]
  %5 = phi i32 [ %9, %3 ], [ %0, %1 ]
  %6 = trunc i32 %5 to i8
  %7 = and i8 %6, 1
  %8 = add i8 %7, %4
  %9 = lshr i32 %5, 1
  %10 = icmp eq i32 %9, 0
  br i1 %10, label %11, label %3

11:
  %12 = phi i8 [ 0, %1 ], [ %8, %3 ]
  ret i8 %12
}

; The loop-carried high half must be written back into its BSS slot inside the
; loop body (not dropped by a PUSH/POP conversion).
; CHECK-LABEL: .LBB0_2:
; CHECK: ld ({{[^)]*}}),{{bc|de|hl}}
; CHECK: .LBB0_2
