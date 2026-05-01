; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Issue #79: `(x != y) ? 0xFF : 0` previously emitted a 12-byte mask
; chain even though the canonical 4-byte mask-from-flag idiom suffices:
;
;   sub  a, l       ; A = x - y (using L as second i8 arg per sdcccall(1))
;   add  a, $ff     ; carry := (x != y)
;   sbc  a, a       ; A = -carry = 0xFF if x!=y else 0
;
; GISel lowers `sext i1 → i8` of icmp ne via (shl 7; ashr 7), which
; on Z80 expands to a trailing 5-instruction mask-roundtrip:
;
;   and  $1; rrca; and $80; add a,a; sbc a,a
;
; That sequence is an identity on the value already in A after the
; canonical sbc a,a.  Z80LateOptimization detects the literal pattern
; and deletes it (-8 B per call site).

define zeroext i8 @mask_neq(i8 zeroext %x, i8 zeroext %y) {
  %cmp = icmp ne i8 %x, %y
  %m   = sext i1 %cmp to i8
  ret i8 %m
}

; CHECK-LABEL: _mask_neq:
; CHECK:       sub  l
; CHECK-NEXT:  add  a,#255
; CHECK-NEXT:  sbc  a,a
; CHECK-NEXT:  ret
; CHECK-NOT:   rrca
; CHECK-NOT:   and  #128
