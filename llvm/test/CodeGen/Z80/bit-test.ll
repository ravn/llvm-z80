; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; ==========================================================================
; BIT test via branching: single bit checks
; ==========================================================================

; Bit 0 branch: RRCA; JR C/NC (AND $1 narrowed to rotate by peephole)
; CHECK-LABEL: branch_bit0:
; CHECK:      	and	#1
; CHECK:      	jr	nz,.LBB0_2
; CHECK:      	call	_ext_no
; CHECK:      	ret
; CHECK:      	call	_ext_yes
; CHECK:      	ret
define void @branch_bit0(i8 zeroext %val) {
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
; CHECK-LABEL: branch_bit7_sign:
; CHECK:      	ld	b,a
; CHECK:      	xor	a
; CHECK:      	xor	#128
; CHECK:      	ld	c,a
; CHECK:      	ld	a,b
; CHECK:      	xor	#128
; CHECK:      	cp	c
; CHECK:      	jr	c,.LBB1_2
; CHECK:      	call	_ext_no
; CHECK:      	ret
; CHECK:      	call	_ext_yes
; CHECK:      	ret
define void @branch_bit7_sign(i8 zeroext %val) {
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
; CHECK-LABEL: branch_bit7_mask:
; CHECK:      	xor	#128
; CHECK:      	ld	b,a
; CHECK:      	ld	a,#255
; CHECK:      	xor	#128
; CHECK:      	cp	b
; CHECK:      	jr	nc,.LBB2_2
; CHECK:      	call	_ext_no
; CHECK:      	ret
; CHECK:      	call	_ext_yes
; CHECK:      	ret
define void @branch_bit7_mask(i8 zeroext %val) {
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
; CHECK-LABEL: branch_bit4:
; CHECK:      	and	#16
; CHECK:      	jr	nz,.LBB3_2
; CHECK:      	call	_ext_no
; CHECK:      	ret
; CHECK:      	call	_ext_yes
; CHECK:      	ret
define void @branch_bit4(i8 zeroext %val) {
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
; CHECK-LABEL: branch_mem_bit3:
; CHECK:      	ld	a,(_g_status)
; CHECK:      	and	#8
; CHECK:      	jr	nz,.LBB4_2
; CHECK:      	call	_ext_no
; CHECK:      	ret
; CHECK:      	call	_ext_yes
; CHECK:      	ret
define void @branch_mem_bit3() {
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

; CHECK-LABEL: set_bit3:
; CHECK:      	or	#8
; CHECK:      	ret
define i8 @set_bit3(i8 %val) {
  %r = or i8 %val, 8
  ret i8 %r
}

; CHECK-LABEL: res_bit3:
; CHECK:      	and	#247
; CHECK:      	ret
define i8 @res_bit3(i8 %val) {
  %r = and i8 %val, -9
  ret i8 %r
}

; CHECK-LABEL: toggle_bit3:
; CHECK:      	xor	#8
; CHECK:      	ret
define i8 @toggle_bit3(i8 %val) {
  %r = xor i8 %val, 8
  ret i8 %r
}

; ==========================================================================
; Materializing bit as 0/1 value
; ==========================================================================

; CHECK-LABEL: extract_bit0:
; CHECK:      	and	#1
; CHECK:      	ret
define i8 @extract_bit0(i8 %val) {
  %t = and i8 %val, 1
  ret i8 %t
}

; CHECK-LABEL: extract_bit7:
; CHECK:      	rlca
; CHECK:      	and	#1
; CHECK:      	ret
define i8 @extract_bit7(i8 %val) {
  %t = lshr i8 %val, 7
  ret i8 %t
}

; ==========================================================================
; Signed comparisons that should use bit 7 test (RLCA)
; ==========================================================================

; slt X, 0 → RLCA; JR C/NC
; CHECK-LABEL: slt_zero:
; CHECK:      	ld	b,a
; CHECK:      	xor	a
; CHECK:      	xor	#128
; CHECK:      	ld	c,a
; CHECK:      	ld	a,b
; CHECK:      	xor	#128
; CHECK:      	cp	c
; CHECK:      	jr	c,.LBB10_2
; CHECK:      	call	_ext_no
; CHECK:      	ret
; CHECK:      	call	_ext_yes
; CHECK:      	ret
define void @slt_zero(i8 %val) {
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
; CHECK-LABEL: sge_zero:
; CHECK:      	xor	#128
; CHECK:      	ld	b,a
; CHECK:      	ld	a,#255
; CHECK:      	xor	#128
; CHECK:      	cp	b
; CHECK:      	jr	nc,.LBB11_2
; CHECK:      	call	_ext_yes
; CHECK:      	ret
; CHECK:      	call	_ext_no
; CHECK:      	ret
define void @sge_zero(i8 %val) {
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
; CHECK-LABEL: sgt_minus1:
; CHECK:      	xor	#128
; CHECK:      	ld	b,a
; CHECK:      	ld	a,#255
; CHECK:      	xor	#128
; CHECK:      	cp	b
; CHECK:      	jr	nc,.LBB12_2
; CHECK:      	call	_ext_yes
; CHECK:      	ret
; CHECK:      	call	_ext_no
; CHECK:      	ret
define void @sgt_minus1(i8 %val) {
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

; CHECK-LABEL: branch_mask_0x41:
; CHECK:      	and	#65
; CHECK:      	jr	z,.LBB13_2
; CHECK:      	call	_ext_yes
; CHECK:      	ret
; CHECK:      	call	_ext_no
; CHECK:      	ret
define void @branch_mask_0x41(i8 zeroext %val) {
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

; CHECK-LABEL: no_xor80_for_bit7:
; CHECK:      	ld	b,a
; CHECK:      	xor	a
; CHECK:      	xor	#128
; CHECK:      	ld	c,a
; CHECK:      	ld	a,b
; CHECK:      	xor	#128
; CHECK:      	cp	c
; CHECK:      	jr	c,.LBB14_2
; CHECK:      	call	_ext_no
; CHECK:      	ret
; CHECK:      	call	_ext_yes
; CHECK:      	ret
define void @no_xor80_for_bit7(i8 zeroext %val) {
  %c = icmp slt i8 %val, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; CHECK-LABEL: no_xor80_for_sgt_m1:
; CHECK:      	xor	#128
; CHECK:      	ld	b,a
; CHECK:      	ld	a,#255
; CHECK:      	xor	#128
; CHECK:      	cp	b
; CHECK:      	jr	nc,.LBB15_2
; CHECK:      	call	_ext_yes
; CHECK:      	ret
; CHECK:      	call	_ext_no
; CHECK:      	ret
define void @no_xor80_for_sgt_m1(i8 zeroext %val) {
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
; i1 bit test materialization: (val & 0x80) != 0 as i1
; Legalizer transforms this to icmp slt val, 0.  ISel uses RLCA; AND 1.
; ==========================================================================

; CHECK-LABEL: bit7_i1_return:
; CHECK:      	xor	#128
; CHECK:      	ld	b,a
; CHECK:      	xor	a
; CHECK:      	xor	#128
; CHECK:      	ld	c,a
; CHECK:      	ld	a,b
; CHECK:      	cp	c
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ret
define i1 @bit7_i1_return(i8 %val) {
  %t = and i8 %val, 128
  %c = icmp ne i8 %t, 0
  ret i1 %c
}

; i1 non-negative test: (val & 0x80) == 0 as i1
; CHECK-LABEL: bit7_i1_nonneg:
; CHECK:      	ld	b,a
; CHECK:      	ld	a,#255
; CHECK:      	xor	#128
; CHECK:      	ld	c,a
; CHECK:      	ld	a,b
; CHECK:      	xor	#128
; CHECK:      	ld	b,a
; CHECK:      	ld	a,c
; CHECK:      	cp	b
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ret
define i1 @bit7_i1_nonneg(i8 %val) {
  %t = and i8 %val, 128
  %c = icmp eq i8 %t, 0
  ret i1 %c
}

declare void @ext_yes()
declare void @ext_no()
