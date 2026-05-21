; RUN: llc -mtriple=z80 -O2 -disable-lsr -mattr=+static-stack -z80-asm-format=sdasz80 < %s | FileCheck %s
;
; Issue ravn/llvm-z80#179: GISel ISel + MachineScheduler don't reorder
; register-independent operations chained through implicit-use $a.
;
; Symptom from #174 gf_log/gf_alog inner-loop analysis: when the loop
; body has TWO PHIs (counter + accumulator) plus a "(sub counter, 1) +
; (icmp eq counter, 0) + br" pattern, the backend emits:
;
;   LD_A_C        ; load counter
;   DEC_A         ; counter - 1 (clobbers A)
;   LD_E_A        ; save dec'd
;   LD_A_C        ; RELOAD counter -- redundant
;   OR_A          ; test for zero
;   JR_Z exit
;
; Where the SAME structure without the second PHI is correctly
; optimized to test-first-then-dec.  The redundancy emerges from
; the regalloc/scheduler not recognizing that the test could fire
; first when both PHIs compete for registers.
;
; Optimal output (the test should reach this with #179 fix):
;
;   LD_A_C        ; load counter
;   OR_A          ; test for zero (uses A's current value)
;   JR_Z exit     ; branch
;   DEC_A         ; dec (only on continue path)
;   LD_E_A        ; save dec'd
;   ...accumulator ops...

; ----------------------------------------------------------------------
; gf_alog inner loop -- exact shape from AES corpus's gf_alog().
; The TWO PHIs are %carrier (the accumulator, loop-carried) and %i
; (the counter, also loop-carried).  Both consume A in the body.
;
; Adapted from the IR clang emits for:
;   uint8_t y = 1, atb = 1;
;   while (x--) {
;     atb = (atb << 1) ^ ((atb & 0x80) ? 0x1b : 0);
;     y ^= atb;
;   }
;   return y;

define i8 @gf_alog(i16 noundef %0) nounwind {
; CHECK-LABEL: _gf_alog:
;
; Post-fix: the loop header uses SUB 1 + JR C instead of the
; 6-instruction LD/DEC/LD/LD/OR/JR_Z pattern that the GISel
; selector originally emits.  SUB 1 sets CARRY iff pre-value was
; 0 (borrow from 0 wraps to 0xFF); JR C branches on that.
;
; Required: the inner loop body contains a `sub #1` and a `jr c`.
; CHECK:      sub{{[ \t]+}}#1
; CHECK:      jr{{[ \t]+}}c
;
; Anti-pattern (P1): the OLD redundant-reload pattern.  After the
; fix we should NOT see `dec a` followed by `or a` (the test)
; followed by `jr z` -- that's the unfixed shape.
; CHECK-NOT:  dec{{[ \t]+}}a
; CHECK-NOT:  or{{[ \t]+}}a
;
; Anti-pattern (P2): the bit-7 test redundant-reload pattern.
; Original shape: `add a,a` (sets carry = bit 7) followed by
; `ld <r>, a` (save shifted) then `ld a, <reg>` (reload original)
; then `rlca` (re-derive same carry).  After P2 fix the second
; reload + rlca disappear -- only the add a,a remains, with the
; conditional branch using its carry directly.
; CHECK-NOT:  rlca

  %2 = trunc i16 %0 to i8
  br label %3

3:
  %4 = phi i8 [ 1, %1 ], [ %15, %13 ]
  %5 = phi i8 [ %2, %1 ], [ %6, %13 ]
  %6 = add i8 %5, -1
  %7 = icmp eq i8 %5, 0
  br i1 %7, label %16, label %8

8:
  %9 = shl i8 %4, 1
  %10 = icmp sgt i8 %4, -1
  br i1 %10, label %13, label %11

11:
  %12 = xor i8 %9, 27
  br label %13

13:
  %14 = phi i8 [ %12, %11 ], [ %9, %8 ]
  %15 = xor i8 %14, %4
  br label %3

16:
  ret i8 %4
}
