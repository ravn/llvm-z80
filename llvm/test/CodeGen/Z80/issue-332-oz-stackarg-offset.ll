; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs < %s | FileCheck %s
;
; ravn/llvm-z80#332: at -Oz the BSS-spill -> PUSH/POP peephole in
; Z80PreEmitPeephole could fire on a store whose forward scan included
; an ADD_HL_SP that read a fixed-stack-object offset baked in by PEI.
; Inserting an additional PUSH before that read shifted SP relative to
; the hard-coded offset in `ld hl,4; add hl,sp` and the incoming
; argument was read from the return-address slot instead.
;
; Reproducer: my_strrev takes a pointer and an i8 length; the length
; arrives on the stack (SP+4 after the compiler's own HL-save push).
; Before the fix, -Oz emitted TWO `push hl` in the prologue but still
; used `ld hl,4; add hl,sp`, yielding a self-corrupting reverse loop.
;
; Runtime oracle: z80-utils/test-runner/testcases/clang/test_33_string_ops.c
; (test_33_string_ops_Oz was the sole FAIL in the clang runtime suite
; before this fix; all six opt levels PASS after).
;
; C source:
;   typedef unsigned char uint8_t;
;   void my_strrev(char *s, uint8_t len) {
;       char *end = s + len;
;       char *fwd = s;
;       for (uint16_t i = len / 2; i != 0; i--) {
;           char t = *--end;
;           *end = *fwd;
;           *fwd++ = t;
;       }
;   }
; At -Oz the BSS-spill peephole inserted an extra PUSH HL in the prologue
; but PEI had already baked the SP offset for `len` as `ld hl,4; add hl,sp`.
; The extra PUSH shifted SP by 2: `len` was read from the return-address slot
; instead of the argument slot → reverse loop used wrong length → corruption.

define void @my_strrev(ptr nocapture %s, i8 zeroext %len) #0 {
entry:
  %len16 = zext i8 %len to i16
  %half = lshr i16 %len16, 1
  %endp = getelementptr i8, ptr %s, i16 %len16
  br label %loop

loop:
  %i     = phi i16 [ %half, %entry ], [ %i.dec, %body ]
  %backp = phi ptr [ %endp, %entry ], [ %back.next, %body ]
  %fwdp  = phi ptr [ %s,    %entry ], [ %fwd.next, %body ]
  %done  = icmp eq i16 %i, 0
  br i1 %done, label %exit, label %body

body:
  %back  = getelementptr i8, ptr %backp, i16 -1
  %vf    = load  i8, ptr %fwdp
  %vb    = load  i8, ptr %back
  store  i8 %vb, ptr %fwdp
  store  i8 %vf, ptr %back
  %fwd.next  = getelementptr i8, ptr %fwdp, i16 1
  %back.next = getelementptr i8, ptr %back, i16 0
  %i.dec = add nsw i16 %i, -1
  br label %loop

exit:
  ret void
}

; The critical invariant: there must be at most ONE `push hl` between the
; function entry and the `add hl,sp` that computes the address of the
; incoming `len` argument. Before the fix a second `push hl` was inserted
; by the BSS-spill peephole, shifting SP so that `ld hl,4; add hl,sp`
; addressed the return-address slot instead of `len`.
; CHECK-LABEL: _my_strrev:
; CHECK:       push hl
; CHECK-NOT:   push hl
; CHECK:       ld hl,4
; CHECK-NEXT:  add hl,sp

attributes #0 = { minsize optsize }
