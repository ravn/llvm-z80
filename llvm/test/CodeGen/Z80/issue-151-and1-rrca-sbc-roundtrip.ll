; RUN: llc -mtriple=z80 -mattr=+static-frame < %s | FileCheck %s
;
; ravn/llvm-z80#151: After ravn/llvm-z80#144 the i16 `sext (icmp eq)`
; chain is:
;   <icmp prologue>
;   sbc  a, a       ; A = 0xFF iff equal, 0 iff not equal
;   and  1          ; A = 0x01 / 0x00      ← redundant
;   rrca            ; CF = bit 0, A rotated ← redundant
;   sbc  a, a       ; A = 0xFF / 0x00 (back to where it was)
;   ld   e, a
;   ld   d, a
;
; The `and 1; rrca; sbc a, a` triple round-trips A through {0,1}
; and back.  Net effect: same as the prior `sbc a, a` alone.
;
; Fix: post-RA peephole deletes the triple when it immediately
; follows another `sbc a, a`.

; C source:
;   typedef unsigned short uint16_t;
;   int16_t sext_eq(uint16_t a, uint16_t b) {
;       return (int16_t)((a == b) ? 0xFFFF : 0);  /* sext(icmp eq) */
;   }
; After #144: the AND 1; RRCA; SBC A,A triple round-trips A through {0,1}
; back to {0xFF,0}. The triple is eliminated as a no-op vs the preceding SBC A,A.

declare i16 @get()

define i16 @select_test() {
  %a = call i16 @get()
  %eq = icmp eq i16 %a, 1
  %res = sext i1 %eq to i16
  ret i16 %res
}
; CHECK-LABEL: select_test:
; CHECK:      	call	_get
; CHECK:      	ld	b,d
; CHECK:      	ld	a,e
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
