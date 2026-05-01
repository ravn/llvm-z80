; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -Oz < %s | FileCheck %s
; XFAIL: *

; Issue #79: `(x != y) ? 0xFF : 0` should lower to the canonical Z80
; mask-from-flag idiom in 4 bytes:
;
;   sub  a, l       ; A = x - y     (where x is in A, y in L per sdcccall(1))
;   add  a, $ff     ; carry := (x != y)
;   sbc  a, a       ; A = -carry = 0xFF if x!=y else 0
;
; Currently emits a 7-instruction mask chain (~12 bytes) routing
; through `and 1; rrca; and 0x80; add a,a; sbc a,a` -- about 8 B
; wasted per call site.

define zeroext i8 @mask_neq(i8 zeroext %x, i8 zeroext %y) {
  %cmp = icmp ne i8 %x, %y
  %m   = sext i1 %cmp to i8
  ret i8 %m
}

; CHECK-LABEL: _mask_neq:
; CHECK:       sub  a,
; CHECK:       add  a,#0xff
; CHECK:       sbc  a,a
; CHECK-NOT:   rrca
; CHECK-NOT:   and  a,#0x80
; CHECK:       ret

; Companion: same shape but with `eq` -- complemented mask
;   sub  a, l       ; A = x - y
;   add  a, $ff     ; carry := (x != y)
;   sbc  a, a       ; A = -carry = 0xFF if x!=y, 0 if x==y
;   xor  a, $ff     ; flip: 0 if x!=y, 0xFF if x==y
; Current emission path via `(x == y) ? 0xFF : 0` should have analogous
; treatment.

define zeroext i8 @mask_eq(i8 zeroext %x, i8 zeroext %y) {
  %cmp = icmp eq i8 %x, %y
  %m   = sext i1 %cmp to i8
  ret i8 %m
}

; CHECK-LABEL: _mask_eq:
; CHECK:       sub  a,
; CHECK:       add  a,#0xff
; CHECK:       sbc  a,a
; CHECK:       xor  a,#0xff
; CHECK-NOT:   rrca
; CHECK:       ret
