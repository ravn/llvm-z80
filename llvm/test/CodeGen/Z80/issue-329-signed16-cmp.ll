; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

declare void @ext_yes()
declare void @ext_no()

; Case 1: val > 0 (repro from fdc_read_data_from_current_location: remaining > 0)
; Should NOT emit the 21-instruction emitSigned16BitCompare sequence.
; CHECK-LABEL: _test_sgt_zero:
; CHECK-NOT:   cpl
; CHECK:       xor	#128
; CHECK:       sub	#1
; CHECK:       sbc	a,#128
; CHECK:       jr	c,[[NO:\.LBB[0-9_]+]]
; CHECK:       call	_ext_yes
; CHECK:       ret
; CHECK:       [[NO]]:
; CHECK:       call	_ext_no
; CHECK:       ret
define void @test_sgt_zero(i16 %val) {
  %c = icmp sgt i16 %val, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Case 2: val <= 0
; CHECK-LABEL: _test_sle_zero:
; CHECK-NOT:   cpl
; CHECK:       xor	#128
; CHECK:       sub	#1
; CHECK:       sbc	a,#128
; CHECK:       jr	c,[[YES:\.LBB[0-9_]+]]
; CHECK:       call	_ext_no
; CHECK:       ret
; CHECK:       [[YES]]:
; CHECK:       call	_ext_yes
; CHECK:       ret
define void @test_sle_zero(i16 %val) {
  %c = icmp sle i16 %val, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Case 3: val > -1 (equivalent to val >= 0: sign bit clear)
; CHECK-LABEL: _test_sgt_minus_one:
; CHECK:       add	a,a
; CHECK:       jr	c,[[NO:\.LBB[0-9_]+]]
; CHECK:       call	_ext_yes
; CHECK:       ret
; CHECK:       [[NO]]:
; CHECK:       call	_ext_no
; CHECK:       ret
define void @test_sgt_minus_one(i16 %val) {
  %c = icmp sgt i16 %val, -1
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Case 4: val <= -1 (equivalent to val < 0: sign bit set)
; CHECK-LABEL: _test_sle_minus_one:
; CHECK:       add	a,a
; CHECK:       jr	c,[[YES:\.LBB[0-9_]+]]
; CHECK:       call	_ext_no
; CHECK:       ret
; CHECK:       [[YES]]:
; CHECK:       call	_ext_yes
; CHECK:       ret
define void @test_sle_minus_one(i16 %val) {
  %c = icmp sle i16 %val, -1
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Case 5: val < 100
; 100 ^ 0x8000 = 0x8064 (lo: 100, hi: 128)
; CHECK-LABEL: _test_slt_100:
; CHECK-NOT:   cpl
; CHECK:       xor	#128
; CHECK:       sub	#100
; CHECK:       sbc	a,#128
; CHECK:       jr	nc,[[NO:\.LBB[0-9_]+]]
; CHECK:       call	_ext_yes
; CHECK:       ret
; CHECK:       [[NO]]:
; CHECK:       call	_ext_no
; CHECK:       ret
define void @test_slt_100(i16 %val) {
  %c = icmp slt i16 %val, 100
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Case 6: val >= -50
; -50 ^ 0x8000 = 0x7FCE (lo: 206, hi: 127)
; CHECK-LABEL: _test_sge_neg50:
; CHECK-NOT:   cpl
; CHECK:       xor	#128
; CHECK:       sub	#206
; CHECK:       sbc	a,#127
; CHECK:       jr	c,[[NO:\.LBB[0-9_]+]]
; CHECK:       call	_ext_yes
; CHECK:       ret
; CHECK:       [[NO]]:
; CHECK:       call	_ext_no
; CHECK:       ret
define void @test_sge_neg50(i16 %val) {
  %c = icmp sge i16 %val, -50
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

