; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
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

declare i16 @get()

; CHECK-LABEL: select_test:
; CHECK:       call	_get
; CHECK:       sub	1
; CHECK:       sbc	a,a
; CHECK-NEXT:  ld	e,a
; CHECK-NEXT:  ld	d,a
; CHECK-NEXT:  ret
; CHECK-NOT:   and	1
; CHECK-NOT:   rrca
define i16 @select_test() {
  %a = call i16 @get()
  %eq = icmp eq i16 %a, 1
  %res = sext i1 %eq to i16
  ret i16 %res
}
