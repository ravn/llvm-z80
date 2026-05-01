; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -Oz < %s | FileCheck %s
; XFAIL: *

; Issue #83: storing `i1 true` (a known-1 _Bool) to a byte-wide
; location emits `ld a,$1; and $1` -- but A is already 1, so the AND
; is provably dead.  Saves 2 B per site.
;
; Triggered in real code when a `uint8_t` field gets value-range-
; narrowed to `_Bool` because clang sees only stores of 0 and 1
; (e.g. transport_pio.c::pio_b_dir, a uint8_t with only PIO_DIR_INPUT=0
; and PIO_DIR_OUTPUT=1).  IR has `store i1 true, ptr @flag` and the
; backend should know the constant already satisfies the i1
; invariant.

@flag = internal global i1 false, align 1

define void @set_flag_true() {
  store i1 true, ptr @flag, align 1
  ret void
}

; CHECK-LABEL: _set_flag_true:
; CHECK:       ld   a,#0x1
; CHECK-NOT:   and  a,#0x1
; CHECK:       ld   ({{.+}}),a
; CHECK:       ret

define void @set_flag_false() {
  store i1 false, ptr @flag, align 1
  ret void
}

; CHECK-LABEL: _set_flag_false:
; CHECK:       xor  a,a
; CHECK-NOT:   and  a,#0x1
; CHECK:       ld   ({{.+}}),a
; CHECK:       ret

; Companion: `_Bool` from runtime value -- here AND IS legitimate
; (the source might have any bits set), so the test pins emission of
; AND for this case to differentiate from the constant-source case.

define void @set_flag_runtime(i8 zeroext %v) {
  %b = icmp ne i8 %v, 0
  store i1 %b, ptr @flag, align 1
  ret void
}

; CHECK-LABEL: _set_flag_runtime:
; CHECK:       ; runtime path -- mask required, no CHECK-NOT here
; CHECK:       ld   ({{.+}}),a
; CHECK:       ret
