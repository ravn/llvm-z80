; RUN: llc -mtriple=z80 < %s | FileCheck %s
;
; Wide scalar load+store pairs must become block moves, not per-limb
; traffic.  Without the pre-legalizer combine, narrowScalar splits an i64
; copy into 4 x i16 loads + 4 x i16 stores, each re-deriving its address
; (no displacement mode on (HL)) — measured 65 instructions for @copy8.
; With the combine the pair becomes G_MEMMOVE (load-completes-before-store
; semantics == memmove), which lowers to LDIR/LDDR or a _memmove libcall.
;
; AVR analogy: AVR absorbs the same shape via ldd/std Z+q displacement
; addressing; Z80's idiomatic answer is the block move.

define void @copy8(ptr %dst, ptr %src) {
; CHECK-LABEL: copy8:
; CHECK-NOT: ld e, (hl)
; CHECK: memmove
  %v = load i64, ptr %src, align 1
  store i64 %v, ptr %dst, align 1
  ret void
}

define void @copy4(ptr %dst, ptr %src) {
; CHECK-LABEL: copy4:
; CHECK-NOT: ld e, (hl)
; CHECK: memmove
  %v = load i32, ptr %src, align 1
  store i32 %v, ptr %dst, align 1
  ret void
}

; i16 is native (register pair) — must NOT become a block move.
define void @keep_i16(ptr %dst, ptr %src) {
; CHECK-LABEL: keep_i16:
; CHECK-NOT: memmove
; CHECK: ret
  %v = load i16, ptr %src, align 1
  store i16 %v, ptr %dst, align 1
  ret void
}

; Volatile accesses keep their exact memory operations.
define void @keep_volatile(ptr %dst, ptr %src) {
; CHECK-LABEL: keep_volatile:
; CHECK-NOT: memmove
  %v = load volatile i64, ptr %src, align 1
  store volatile i64 %v, ptr %dst, align 1
  ret void
}

; Multi-use loaded value: the value is genuinely needed in registers.
define i8 @keep_multiuse(ptr %dst, ptr %src) {
; CHECK-LABEL: keep_multiuse:
; CHECK-NOT: memmove
  %v = load i64, ptr %src, align 1
  store i64 %v, ptr %dst, align 1
  %t = trunc i64 %v to i8
  ret i8 %t
}

; An intervening may-write between load and store blocks the rewrite
; (the combine moves the read down to the store point).
define void @keep_intervening_store(ptr %dst, ptr %src, ptr %other) {
; CHECK-LABEL: keep_intervening_store:
; CHECK-NOT: memmove
  %v = load i64, ptr %src, align 1
  store i8 7, ptr %other, align 1
  store i64 %v, ptr %dst, align 1
  ret void
}
