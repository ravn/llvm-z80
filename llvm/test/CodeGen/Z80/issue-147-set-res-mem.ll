; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; ravn/llvm-z80#147: single-bit memory updates `mem |= 1<<N` and
; `mem &= ~(1<<N)` lower to 8 bytes (load + OR/AND immediate + store)
; when Z80's CB-prefix `SET n,(HL)` / `RES n,(HL)` could do it in 5
; (load HL with address + single-bit op).
;
; The peephole pattern (post-RA):
;   LD_A_nnind <Sym>            ; load constant-address byte (3 B)
;   {OR,AND}_n K                ; bit-twiddle (2 B)
;   LD_nnind_A <Sym>            ; store back (3 B) — same Sym as load
;                              ; = 8 B
; Replace with:
;   LD_HL_nn <Sym>              ; (3 B)
;   {SET,RES}_b_(HL) × N        ; (2 B each, N = popcount)
;                              ; = 5 B for single-bit (saves 3 B)

@flag = external global i8

;
; Set bit 0: |= 1
;
; CHECK-LABEL: set_bit_0:
; CHECK:       ld	hl,_flag
; CHECK-NEXT:  set	0,(hl)
; CHECK-NOT:   or	$1
; CHECK-NOT:   ld	(_flag),a
define void @set_bit_0() {
  %v = load i8, ptr @flag
  %or = or i8 %v, 1
  store i8 %or, ptr @flag
  ret void
}

;
; Set bit 1: |= 2
;
; CHECK-LABEL: set_bit_1:
; CHECK:       ld	hl,_flag
; CHECK-NEXT:  set	1,(hl)
; CHECK-NOT:   or	$2
define void @set_bit_1() {
  %v = load i8, ptr @flag
  %or = or i8 %v, 2
  store i8 %or, ptr @flag
  ret void
}

;
; Clear bit 0: &= ~1
;
; CHECK-LABEL: clear_bit_0:
; CHECK:       ld	hl,_flag
; CHECK-NEXT:  res	0,(hl)
; CHECK-NOT:   and	{{254|\$fe}}
define void @clear_bit_0() {
  %v = load i8, ptr @flag
  %and = and i8 %v, -2          ; ~1 = 0xFE
  store i8 %and, ptr @flag
  ret void
}

;
; Two-bit set: |= 3 (bits 0 and 1) — fires twice (2 SETs).  Saves 1 B
; vs `or 3; store` (3+2+3 = 8 B → 3+2+2 = 7 B).
;
; CHECK-LABEL: set_bits_01:
; CHECK:       ld	hl,_flag
; CHECK-NEXT:  set	0,(hl)
; CHECK-NEXT:  set	1,(hl)
; CHECK-NOT:   or	$3
define void @set_bits_01() {
  %v = load i8, ptr @flag
  %or = or i8 %v, 3
  store i8 %or, ptr @flag
  ret void
}

;
; Negative: |= 7 (popcount 3) — too many bits, don't fire (would
; cost 3+2+2+2 = 9 B vs 3+2+3 = 8 B).
;
; CHECK-LABEL: set_three_bits:
; CHECK:       ld	a,(_flag)
; CHECK:       or	7
; CHECK:       ld	(_flag),a
; CHECK-NOT:   set	{{[0-7]}},(hl)
define void @set_three_bits() {
  %v = load i8, ptr @flag
  %or = or i8 %v, 7
  store i8 %or, ptr @flag
  ret void
}
