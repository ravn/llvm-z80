; RUN: llc -mtriple=z80 -mattr=+static-stack -O0 < %s | FileCheck %s
;
; ravn/llvm-z80#202 (FIXED): at -O0 with +static-stack, a store to a frame (BSS)
; slot in one block was dropped when the stored value is forwarded (via
; push/pop) to a load in a *successor* block while the same slot is reloaded
; across a loop back-edge.  The back-edge reload then read the stale initial
; value.  Fix: the cross-block spill->PUSH/POP peephole now bails when the slot
; is accessed earlier in the store's block (the loop-carried back-edge
; signature), mirroring the single-block peephole's #195 guard.
;
; Concretely this miscompiles `do { v >>= 1; } while (v > 0)` over a memory-
; resident `v`: the shift result is computed (`srl`/`rr`), pushed for the
; loop-condition compare in the cond block, but never written back to v's slot,
; so the next iteration reloads the original value -> wrong result / non-
; progressing loop (test_54_unsigned_compare returns 0x0080 vs 0x00FF at
; O0_ss; minimal value: v=256 do-while returns 0x0080 vs 9).
;
; Surfaced from test_54 by isolating the `>>=` countdown loop.  -O1+ folds the
; loop away so only -O0 is affected.  Hand-IR with the load/shift/store/compare
; in a *single* block stores back correctly; the two-block (body/cond) split
; with a back-edge is the trigger.
;
; The store-back below MUST reach v's frame slot before the back-edge reloads
; it (the conversion to PUSH/POP must be refused for this loop-carried slot).

define i8 @f() {
entry:
  %v = alloca i16
  %bits = alloca i8
  store i16 256, ptr %v
  store i8 0, ptr %bits
  br label %body

body:
  %0 = load i16, ptr %v
  %s = lshr i16 %0, 1
  store i16 %s, ptr %v          ; writeback (was dropped before the #202 fix)
  %b = load i8, ptr %bits
  %b1 = add i8 %b, 1
  store i8 %b1, ptr %bits
  br label %cond

cond:
  %1 = load i16, ptr %v
  %c = icmp ugt i16 %1, 0
  br i1 %c, label %body, label %end

end:
  %r = load i8, ptr %bits
  ret i8 %r
}

; CHECK-LABEL: _f:
; The 16-bit right shift of v:
; CHECK: srl
; ...and its result must be stored back to v's static-stack frame slot before
; the loop re-reads it (this store is the one currently missing):
; CHECK: ld ({{(__sframe_f|__sfrend_f)[-+0-9]*}}),de
