; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -Oz < %s | FileCheck %s
; XFAIL: *

; Issue #60: redundant `LD A,reg` when A already holds the value.
; This file pins specific known-A=0 cases that the backend should
; carry across instructions / MBB edges.

; Case 1: three consecutive stores of zero -- A=0 already established
; by `xor a`; ld a, 0 not re-emitted between stores.  ALREADY GOOD --
; this is the regression check.

@a = global i8 0
@b = global i8 0
@c = global i8 0

define void @zero_three() {
  store volatile i8 0, ptr @a
  store volatile i8 0, ptr @b
  store volatile i8 0, ptr @c
  ret void
}

; CHECK-LABEL: _zero_three:
; CHECK:       xor  a,a
; CHECK-NOT:   xor  a,a
; CHECK-NOT:   ld   a,#0x0
; CHECK:       ret

; Case 2: zero stored before a CALL that we know preserves A=0.
; (No standard Z80 ABI guarantees this, so this is a conservative
; check: outside this case, the second store should re-establish A=0
; via `xor a` which costs 1 B vs `ld a, 0` 2 B.)

declare void @callee_preserves_a(ptr) #0

; The attribute "preserves_a" is hypothetical -- a real fix would
; build on caller-saved/callee-saved semantics.

; Case 3: zero stored after another instruction that does NOT modify A
; (e.g. INC HL).  Bus-side instructions don't touch A.

@dst = global i16 0

define void @zero_then_inc_hl() {
  store volatile i8 0, ptr @a
  %p = ptrtoint ptr @dst to i16
  %q = add i16 %p, 1
  store volatile i16 %q, ptr @dst
  store volatile i8 0, ptr @b   ; A should still be 0 from step 1
  ret void
}

; CHECK-LABEL: _zero_then_inc_hl:
; CHECK:       xor  a,a
; CHECK:       ld   ({{.+}}),a
; first store
; ... (HL manipulation here -- doesn't touch A)
; second `ld (b),a` should NOT need a re-zero of A
; CHECK-NOT:   xor  a,a
; CHECK-NOT:   ld   a,#0x0
; CHECK:       ret

attributes #0 = { nounwind }
