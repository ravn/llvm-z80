; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Issue #83: storing `i1 true` (a known-1 _Bool) to a byte-wide
; location previously emitted `ld a,#1; and #1` -- but A is already 1,
; so the AND was provably dead.  The known-immediate-A peephole in
; Z80LateOptimization deletes the AND.  Saves 2 B per site.
;
; Triggered in real code when a `uint8_t` field gets value-range-
; narrowed to `_Bool` because clang sees only stores of 0 and 1
; (e.g. transport_pio.c::pio_b_dir, a uint8_t with only PIO_DIR_INPUT=0
; and PIO_DIR_OUTPUT=1).  IR has `store i1 true, ptr @flag` and the
; backend now knows the constant already satisfies the i1 invariant.

@flag = internal global i1 false, align 1

define void @set_flag_true() {
  store i1 true, ptr @flag, align 1
  ret void
}

; CHECK-LABEL: _set_flag_true:
; CHECK:      ld  a,#1
; CHECK-NOT:  and  #1
; CHECK:      ld  (_flag),a
; CHECK:      ret

define void @set_flag_false() {
  store i1 false, ptr @flag, align 1
  ret void
}

; CHECK-LABEL: _set_flag_false:
; CHECK:      xor  a
; CHECK-NOT:  and  #1
; CHECK:      ld  (_flag),a
; CHECK:      ret

; Companion: _Bool from runtime value -- here AND IS legitimate
; (the source might have any bits set after `sbc a,a` materializes
; 0xFF/0x00 from icmp ne), so the AND must remain to narrow to {0,1}.

define void @set_flag_runtime(i8 zeroext %v) {
  %b = icmp ne i8 %v, 0
  store i1 %b, ptr @flag, align 1
  ret void
}

; CHECK-LABEL: _set_flag_runtime:
; CHECK:      sbc  a,a
; CHECK:      and  #1
; CHECK:      ld  (_flag),a
; CHECK:      ret
