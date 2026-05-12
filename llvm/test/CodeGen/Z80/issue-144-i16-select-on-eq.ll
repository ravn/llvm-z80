; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; ravn/llvm-z80#144: `(a == K) ? -1 : 0` for i16 was lowering via a
; 22-byte SBC + AND + RLCA + ADD-A-A + SBC chain because the i1
; result was materialised to {0,1} via the icmp then sign-extended
; via SHL/ASHR by 15.
;
; Fix: Z80LegalizerInfo no longer expands G_SEXT_INREG width=1 to
; SHL+ASHR.  Z80InstructionSelector now lowers G_SEXT_INREG i16
; width=1 directly as `COPY $a, src:sub_lo; RRCA; SBC A, A;
; REG_SEQUENCE` — A becomes 0xFF or 0 based on bit 0, then both
; halves of the destination get A.
;
; Saving: 7 B per occurrence (22 B → 15 B).  Residual `and 1;
; rrca; sbc a, a` chain is logically a no-op after the icmp's own
; SBC A, A produced 0xFF/0 — tracked in a follow-up issue for a
; post-RA peephole.

declare i16 @get()

; CHECK-LABEL: select_test:
; CHECK:       ld	a,l
; CHECK:       xor	1
; CHECK:       or	{{[bdh]}}
; CHECK:       sub	1
; CHECK:       sbc	a,a
; CHECK:       rrca
; CHECK-NEXT:  sbc	a,a
; CHECK-NEXT:  ld	e,a
; CHECK-NEXT:  ld	d,a
; CHECK-NOT:   add	a,a
; CHECK-NOT:   ld	h,a
; CHECK-NOT:   ld	l,0
define i16 @select_test(i16 %a) {
  %eq = icmp eq i16 %a, 1
  %res = sext i1 %eq to i16
  ret i16 %res
}
