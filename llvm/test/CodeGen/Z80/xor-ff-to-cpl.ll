; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s
;
; G_XOR with immediate -1 (the canonical "not" form) must lower to
; CPL (1 byte, 4 T) instead of XOR #0xFF (2 bytes, 7 T).  Migrated
; from a post-RA peephole into the GISel i8 G_XOR imm-fold branch in
; Z80InstructionSelector.cpp (session 73q C1 drill; see #180).
;
; The flag side-effect difference between XOR_n (sets S/Z/P from
; result) and CPL (leaves S/Z/P unchanged) is not observed in
; practice because clean IR canonicalizes `(~x) == 0` to `x == 0xFF`
; and any other downstream flag-consumer selects to its own fresh
; compare instruction.

; C source:
;   typedef unsigned char uint8_t;
;   uint8_t logical_not(uint8_t x) { return x ^ 0xFF; }  /* CPL (1 B, 4 T) */
;   uint8_t mask_inv(uint8_t x) { return ~x; }            /* same: G_XOR -1 */
; G_XOR with immediate -1 (the canonical NOT form) → CPL (1 B) instead of
; XOR 0xFF (2 B). Flag side-effects differ but S/Z/P are not observed post-NOT.

define i8 @not_u8(i8 %x) {
; CHECK-LABEL: not_u8:
; CHECK: cpl
; CHECK-NOT: xor #0xff
; CHECK-NOT: xor 0xff
; CHECK-NOT: xor 255
  %r = xor i8 %x, -1
  ret i8 %r
}

; i16 NOT decomposes into two i8 G_XOR ops, each of which should
; lower to CPL.
define i16 @not_u16(i16 %x) {
; CHECK-LABEL: not_u16:
; CHECK: cpl
; CHECK: cpl
; CHECK-NOT: xor #0xff
; CHECK-NOT: xor 0xff
; CHECK-NOT: xor 255
  %r = xor i16 %x, -1
  ret i16 %r
}

; NOT then store: CPL emission must NOT be blocked by the store
; (the original peephole's FLAGS-dead check was satisfied here, so
; this case worked before migration too — guard against regression).
define void @not_then_store(i8 %x, ptr %p) {
; CHECK-LABEL: not_then_store:
; CHECK: cpl
; CHECK-NOT: xor #0xff
; CHECK-NOT: xor 0xff
; CHECK-NOT: xor 255
  %r = xor i8 %x, -1
  store i8 %r, ptr %p
  ret void
}
