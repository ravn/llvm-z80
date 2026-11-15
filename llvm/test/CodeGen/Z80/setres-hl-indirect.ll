; RUN: llc -mtriple=z80 -verify-machineinstrs %s -o - | FileCheck %s

; SET (HL) / RES (HL) fold: load/single-bit-logic/store on the same pointer
; collapses to the 2-byte memory-direct form 0xCB 0xC6+8*b / 0xCB 0x86+8*b.

target triple = "z80"

@flags = external global i8

; Check bit 7 (0x80)
define void @set_bit_7(ptr %p) {
entry:
  %v = load i8, ptr %p, align 1
  %r = or i8 %v, 128
  store i8 %r, ptr %p, align 1
  ret void
}

; CHECK-LABEL: _set_bit_7:
; CHECK:       set 7,(hl)
; CHECK-NOT:   ld a,(hl)
; CHECK-NOT:   set 7,a

; Check bit 0 (0x01)
define void @set_bit_0(ptr %p) {
entry:
  %v = load i8, ptr %p, align 1
  %r = or i8 %v, 1
  store i8 %r, ptr %p, align 1
  ret void
}

; CHECK-LABEL: _set_bit_0:
; CHECK:       set 0,(hl)
; CHECK-NOT:   ld a,(hl)

; Commutative: add or 128, %v
define void @set_commuted(ptr %p) {
entry:
  %v = load i8, ptr %p, align 1
  %r = or i8 128, %v
  store i8 %r, ptr %p, align 1
  ret void
}

; CHECK-LABEL: _set_commuted:
; CHECK:       set 7,(hl)

; Check RES bit 3 (~8 = 0xF7 = 247)
define void @res_bit_3(ptr %p) {
entry:
  %v = load i8, ptr %p, align 1
  %r = and i8 %v, 247
  store i8 %r, ptr %p, align 1
  ret void
}

; CHECK-LABEL: _res_bit_3:
; CHECK:       res 3,(hl)
; CHECK-NOT:   ld a,(hl)

; Check global variable bit set
define void @set_global_bit_7() {
entry:
  %v = load i8, ptr @flags, align 1
  %r = or i8 %v, 128
  store i8 %r, ptr @flags, align 1
  ret void
}

; CHECK-LABEL: _set_global_bit_7:
; CHECK:       ld hl,_flags
; CHECK-NEXT:  set 7,(hl)

; Multi-bit OR must NOT fold to SET
define void @set_multi_bit(ptr %p) {
entry:
  %v = load i8, ptr %p, align 1
  %r = or i8 %v, 3
  store i8 %r, ptr %p, align 1
  ret void
}

; CHECK-LABEL: _set_multi_bit:
; CHECK-NOT:   set {{[0-7]}},
; CHECK:       or 3

; Volatile load must NOT fold
define void @volatile_load(ptr %p) {
entry:
  %v = load volatile i8, ptr %p, align 1
  %r = or i8 %v, 128
  store i8 %r, ptr %p, align 1
  ret void
}

; CHECK-LABEL: _volatile_load:
; CHECK-NOT:   set 7,(hl)

; Multi-use load must NOT fold
define i8 @multi_use_load(ptr %p) {
entry:
  %v = load i8, ptr %p, align 1
  %r = or i8 %v, 128
  store i8 %r, ptr %p, align 1
  ret i8 %v
}

; CHECK-LABEL: _multi_use_load:
; CHECK-NOT:   set 7,(hl)
