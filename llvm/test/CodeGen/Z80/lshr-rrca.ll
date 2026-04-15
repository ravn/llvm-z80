; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; Issue #71: SRL A → RRCA when followed by AND mask
; RRCA is 1 byte (0F) vs SRL A's 2 bytes (CB 3F), saving 1B per shift.
; Safe when the AND mask clears the top N bits that RRCA contaminates.

; Shift right by 2, AND 3: (val >> 2) & 3
; Should emit: rrca; rrca; and #3 (4 bytes)
; Not:         srl a; srl a; and #3 (6 bytes)
define i8 @lshr2_and3(i8 zeroext %val) {
; CHECK-LABEL: _lshr2_and3:
; CHECK:       rrca
; CHECK-NEXT:  rrca
; CHECK-NEXT:  and	a,#3
  %shr = lshr i8 %val, 2
  %and = and i8 %shr, 3
  ret i8 %and
}

; Shift right by 3, AND 3: (val >> 3) & 3
; Should emit: rrca; rrca; rrca; and #3 (5 bytes)
; Not:         srl a; srl a; srl a; and #3 (8 bytes)
define i8 @lshr3_and3(i8 zeroext %val) {
; CHECK-LABEL: _lshr3_and3:
; CHECK:       rrca
; CHECK-NEXT:  rrca
; CHECK-NEXT:  rrca
; CHECK-NEXT:  and	a,#3
  %shr = lshr i8 %val, 3
  %and = and i8 %shr, 3
  ret i8 %and
}

; Shift right by 4, AND 3: (val >> 4) & 3
define i8 @lshr4_and3(i8 zeroext %val) {
; CHECK-LABEL: _lshr4_and3:
; CHECK:       rrca
; CHECK-NEXT:  rrca
; CHECK-NEXT:  rrca
; CHECK-NEXT:  rrca
; CHECK-NEXT:  and	a,#3
  %shr = lshr i8 %val, 4
  %and = and i8 %shr, 3
  ret i8 %and
}

; Shift right by 6, AND 3: (val >> 6) & 3
; LLVM folds the AND away (>> 6 already fits in 2 bits), so no fusion.
; The existing shift-by-7 RLCA+AND path doesn't apply either.
; Falls through to SRL×6.
define i8 @lshr6_no_and(i8 zeroext %val) {
; CHECK-LABEL: _lshr6_no_and:
; CHECK:       srl	a
; CHECK-NEXT:  srl	a
; CHECK-NEXT:  srl	a
; CHECK-NEXT:  srl	a
; CHECK-NEXT:  srl	a
; CHECK-NEXT:  srl	a
  %shr = lshr i8 %val, 6
  %and = and i8 %shr, 3
  ret i8 %and
}

; Shift right by 5, AND 7: (val >> 5) & 7
; LLVM also folds this AND away (>> 5 fits in 3 bits), no fusion.
; Shift right by 5, AND 3: mask IS narrower, fusion should fire.
define i8 @lshr5_and3(i8 zeroext %val) {
; CHECK-LABEL: _lshr5_and3:
; CHECK:       rrca
; CHECK-NEXT:  rrca
; CHECK-NEXT:  rrca
; CHECK-NEXT:  rrca
; CHECK-NEXT:  rrca
; CHECK-NEXT:  and	a,#3
  %shr = lshr i8 %val, 5
  %and = and i8 %shr, 3
  ret i8 %and
}

; Shift right by 1, AND 0x1F: (val >> 1) & 0x1F
define i8 @lshr1_and1f(i8 zeroext %val) {
; CHECK-LABEL: _lshr1_and1f:
; CHECK:       rrca
; CHECK-NEXT:  and	a,#31
  %shr = lshr i8 %val, 1
  %and = and i8 %shr, 31
  ret i8 %and
}

; Negative: shift right by 1 WITHOUT AND — must stay SRL
define i8 @lshr1_no_and(i8 zeroext %val) {
; CHECK-LABEL: _lshr1_no_and:
; CHECK:       srl	a
; CHECK-NOT:   rrca
  %shr = lshr i8 %val, 1
  ret i8 %shr
}

; Negative: AND mask too wide — top bits not cleared, must stay SRL
; (val >> 2) & 0xFF — mask doesn't clear top 2 bits
define i8 @lshr2_and_unsafe(i8 zeroext %val) {
; CHECK-LABEL: _lshr2_and_unsafe:
; CHECK:       srl	a
; CHECK-NEXT:  srl	a
  %shr = lshr i8 %val, 2
  %and = and i8 %shr, 255
  ret i8 %and
}
