; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; ==========================================================================
; BIT test via branching: single bit checks
; ==========================================================================

; Bit 0 branch: RRCA; JR C/NC (AND $1 narrowed to rotate by peephole)
define void @branch_bit0(i8 zeroext %val) {
; CHECK-LABEL: _branch_bit0:
; CHECK:       rrca
; CHECK-NEXT:  jr	{{n?c}},
  %t = and i8 %val, 1
  %c = icmp ne i8 %t, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Bit 7 branch via sign test: RLCA; JR C/NC (3 bytes, not 10!)
define void @branch_bit7_sign(i8 zeroext %val) {
; CHECK-LABEL: _branch_bit7_sign:
; CHECK:       rlca
; CHECK-NEXT:  jr	{{n?c}},
  %c = icmp slt i8 %val, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Bit 7 branch via mask (LLVM rewrites to sgt X, -1): should also use RLCA
define void @branch_bit7_mask(i8 zeroext %val) {
; CHECK-LABEL: _branch_bit7_mask:
; CHECK:       rlca
; CHECK-NEXT:  jr	{{n?c}},
  %t = and i8 %val, 128
  %c = icmp ne i8 %t, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Bit 4 branch: AND 16; JR
define void @branch_bit4(i8 zeroext %val) {
; CHECK-LABEL: _branch_bit4:
; CHECK:       and	#16
; CHECK-NEXT:  jr	{{nz|z}},
  %t = and i8 %val, 16
  %c = icmp ne i8 %t, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; ==========================================================================
; BIT test on memory: load + AND + branch
; ==========================================================================

@g_status = global i8 0
define void @branch_mem_bit3() {
; CHECK-LABEL: _branch_mem_bit3:
; CHECK:       ld	a,(_g_status)
; CHECK-NEXT:  and	#8
; CHECK-NEXT:  jr	{{nz|z}},
  %v = load volatile i8, ptr @g_status
  %t = and i8 %v, 8
  %c = icmp ne i8 %t, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; ==========================================================================
; Bit manipulation: SET, RES, toggle
; ==========================================================================

define i8 @set_bit3(i8 %val) {
; CHECK-LABEL: _set_bit3:
; CHECK:       or	#8
; CHECK-NEXT:  ret
  %r = or i8 %val, 8
  ret i8 %r
}

define i8 @res_bit3(i8 %val) {
; CHECK-LABEL: _res_bit3:
; CHECK:       and	#247
; CHECK-NEXT:  ret
  %r = and i8 %val, -9
  ret i8 %r
}

define i8 @toggle_bit3(i8 %val) {
; CHECK-LABEL: _toggle_bit3:
; CHECK:       xor	#8
; CHECK-NEXT:  ret
  %r = xor i8 %val, 8
  ret i8 %r
}

; ==========================================================================
; Materializing bit as 0/1 value
; ==========================================================================

define i8 @extract_bit0(i8 %val) {
; CHECK-LABEL: _extract_bit0:
; CHECK:       and	#1
; CHECK-NEXT:  ret
  %t = and i8 %val, 1
  ret i8 %t
}

define i8 @extract_bit7(i8 %val) {
; CHECK-LABEL: _extract_bit7:
; CHECK:       rlca
; CHECK-NEXT:  and	#1
; CHECK-NEXT:  ret
  %t = lshr i8 %val, 7
  ret i8 %t
}

; ==========================================================================
; Signed comparisons that should use bit 7 test (RLCA)
; ==========================================================================

; slt X, 0 → RLCA; JR C/NC
define void @slt_zero(i8 %val) {
; CHECK-LABEL: _slt_zero:
; CHECK:       rlca
; CHECK-NEXT:  jr	{{n?c}},
  %c = icmp slt i8 %val, 0
  br i1 %c, label %neg, label %pos
neg:
  call void @ext_yes()
  ret void
pos:
  call void @ext_no()
  ret void
}

; sge X, 0 → RLCA; JR NC/C
define void @sge_zero(i8 %val) {
; CHECK-LABEL: _sge_zero:
; CHECK:       rlca
; CHECK-NEXT:  jr	{{n?c}},
  %c = icmp sge i8 %val, 0
  br i1 %c, label %pos, label %neg
pos:
  call void @ext_yes()
  ret void
neg:
  call void @ext_no()
  ret void
}

; sgt X, -1 (same as X >= 0) → RLCA; JR C/NC (was 10 bytes, now 3)
define void @sgt_minus1(i8 %val) {
; CHECK-LABEL: _sgt_minus1:
; CHECK:       rlca
; CHECK-NEXT:  jr	{{n?c}},
  %c = icmp sgt i8 %val, -1
  br i1 %c, label %pos, label %neg
pos:
  call void @ext_yes()
  ret void
neg:
  call void @ext_no()
  ret void
}

; ==========================================================================
; Multi-bit AND for branching
; ==========================================================================

define void @branch_mask_0x41(i8 zeroext %val) {
; CHECK-LABEL: _branch_mask_0x41:
; CHECK:       and	#65
; CHECK-NEXT:  jr	{{nz|z}},
  %t = and i8 %val, 65
  %c = icmp ne i8 %t, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; ==========================================================================
; Ensure no signed comparison bloat for bit 7 tests
; These should NOT generate XOR 0x80; CP sequences
; ==========================================================================

define void @no_xor80_for_bit7(i8 zeroext %val) {
; CHECK-LABEL: _no_xor80_for_bit7:
; CHECK-NOT:   xor	#128
; CHECK:       rlca
  %c = icmp slt i8 %val, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

define void @no_xor80_for_sgt_m1(i8 zeroext %val) {
; CHECK-LABEL: _no_xor80_for_sgt_m1:
; CHECK-NOT:   xor	#128
; CHECK:       rlca
  %c = icmp sgt i8 %val, -1
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; ==========================================================================
; Known edge case: i1 return of bit test generates XOR/CP sequence
; GlobalISel's i1 legalization doesn't recognize the bit test pattern.
; This is rare in practice (C uses i8/int for booleans, not i1).
; TODO: teach ISel to recognize (and i8, pow2) + (icmp ne, 0) as bit test
; ==========================================================================

define i1 @bit7_i1_return(i8 %val) {
; CHECK-LABEL: _bit7_i1_return:
; CHECK:       xor
  %t = and i8 %val, 128
  %c = icmp ne i8 %t, 0
  ret i1 %c
}

declare void @ext_yes()
declare void @ext_no()
