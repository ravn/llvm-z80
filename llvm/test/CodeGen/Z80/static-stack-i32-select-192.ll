; RUN: llc -mtriple=z80 -mattr=+static-stack -O1 < %s | FileCheck %s

; ravn/llvm-z80#192: an i32 `select((crc&1)==0, 0, CONST)` reduction loop
; miscompiled under +static-stack at -O1/-Os.  The i32 `icmp eq` is selected as
; two XOR_CMP_EQ16 (one per 16-bit half) AND-combined.  The #173 peephole
; ("bare BSS store + 4-instr A-preserving reload -> LD r,A; PUSH/POP rr")
; relocated the first half's result (flag1) into register D and bracketed it
; with PUSH/POP DE -- but D is the SECOND half-compare's zero input, which it
; reads.  flag1 thus clobbered D, the second compare computed NOT(flag1), and
; the i32-== AND collapsed to `NOT(flag1) AND flag1` = 0, so the select always
; took the CONST branch (crc_one(0xFF) returned 0xB6662D3D, not 0x2D02EF8D).
;
; Fix: #173 bails when its destination register is READ in the interval.  The
; 4-instr A-preserving reload (PUSH AF; LD A,(slot); LD r,A; POP AF) must then
; survive instead of being folded into a PUSH/POP DE that clobbers D.

define dso_local i32 @crc_one(i32 noundef %0) {
  br label %3

2:
  ret i32 %10

3:
  %4 = phi i8 [ 0, %1 ], [ %11, %3 ]
  %5 = phi i32 [ %0, %1 ], [ %10, %3 ]
  %6 = lshr i32 %5, 1
  %7 = and i32 %5, 1
  %8 = icmp eq i32 %7, 0
  %9 = select i1 %8, i32 0, i32 -306674912
  %10 = xor i32 %9, %6
  %11 = add nuw nsw i8 %4, 1
  %12 = icmp eq i8 %11, 8
  br i1 %12, label %2, label %3
}

; The A-preserving 4-instr reload of the first compare's result must survive
; (#173 must NOT fold it into a D-clobbering PUSH/POP DE).
; CHECK-LABEL: _crc_one:
; CHECK:      push af
; CHECK-NEXT: ld a,(__sfrend{{[^)]*}})
; CHECK-NEXT: ld {{[a-l]}},a
; CHECK-NEXT: pop af
