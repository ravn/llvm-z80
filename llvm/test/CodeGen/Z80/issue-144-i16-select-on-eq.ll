; RUN: llc -mtriple=z80 -mattr=+static-frame < %s | FileCheck %s
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
; Saving: 7 B per occurrence (22 B → 15 B).  The follow-up
; ravn/llvm-z80#151 post-RA peephole removes the residual
; `and 1; rrca; sbc a, a` round-trip that lands immediately
; after the icmp's own SBC A, A, yielding the compact tail
; `sub 1; sbc a,a; ld e,a; ld d,a`.

; C source:
;   typedef unsigned short uint16_t;
;   typedef short int16_t;
;   uint16_t sel_eq(uint16_t a, uint16_t k) {
;       return (a == k) ? 0xFFFFu : 0u;   /* sext(icmp eq) */
;   }
; (a == k) ? -1 : 0 in 16 bits: RRCA; SBC A,A pattern (3 B, 15 T)
; instead of the 22-byte SBC+AND+RLCA+SBC chain.

declare i16 @get()

define i16 @select_test(i16 %a) {
  %eq = icmp eq i16 %a, 1
  %res = sext i1 %eq to i16
  ret i16 %res
}
; CHECK-LABEL: select_test:
; CHECK:      	ld	b,h
; CHECK:      	ld	a,l
; CHECK:      	xor	1
; CHECK:      	or	b
; CHECK:      	sub	1
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ld	l,a
; CHECK:      	rrca
; CHECK:      	and	128
; CHECK:      	add	a,a
; CHECK:      	sbc	a,a
; CHECK:      	ld	e,a
; CHECK:      	ld	d,a
; CHECK:      	ret
