; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

declare void @ext_yes()
declare void @ext_no()

; Case 1: val < 0 (slt 0) -> BIT 7, val; JR NZ
; CHECK-LABEL: _test_slt_zero:
; CHECK:      	bit	7,a
; CHECK-NEXT: 	jr	nz,[[YES:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_no
; CHECK:      [[YES]]:
; CHECK-NEXT: 	call	_ext_yes
define void @test_slt_zero(i8 zeroext %val) {
  %c = icmp slt i8 %val, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Case 2: val >= 0 (sge 0) -> BIT 7, val; JR Z
; CHECK-LABEL: _test_sge_zero:
; CHECK:      	bit	7,a
; CHECK-NEXT: 	jr	z,[[YES:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_no
; CHECK:      [[YES]]:
; CHECK-NEXT: 	call	_ext_yes
define void @test_sge_zero(i8 zeroext %val) {
  %c = icmp sge i8 %val, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Case 3: val > -1 (sgt -1) -> same as val >= 0 -> BIT 7, val; JR Z
; CHECK-LABEL: _test_sgt_minus_one:
; CHECK:      	bit	7,a
; CHECK-NEXT: 	jr	z,[[YES:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_no
; CHECK:      [[YES]]:
; CHECK-NEXT: 	call	_ext_yes
define void @test_sgt_minus_one(i8 zeroext %val) {
  %c = icmp sgt i8 %val, -1
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Case 4: val <= -1 (sle -1) -> same as val < 0 -> BIT 7, val; JR NZ
; CHECK-LABEL: _test_sle_minus_one:
; CHECK:      	bit	7,a
; CHECK-NEXT: 	jr	nz,[[YES:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_no
; CHECK:      [[YES]]:
; CHECK-NEXT: 	call	_ext_yes
define void @test_sle_minus_one(i8 zeroext %val) {
  %c = icmp sle i8 %val, -1
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Case 5: val < -64 (slt -64, repro from fdc_sense_interrupt)
; BiasC = (-64 ^ 0x80) & 0xFF = 64 = 0x40.
; CHECK-LABEL: _test_slt_const:
; CHECK:      	xor	#128
; CHECK-NEXT: 	cp	#64
; CHECK-NEXT: 	jr	c,[[YES:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_no
; CHECK:      [[YES]]:
; CHECK-NEXT: 	call	_ext_yes
define void @test_slt_const(i8 zeroext %val) {
  %c = icmp slt i8 %val, -64
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Case 6: val > 10 (sgt 10) -> val >= 11
; BiasC = (11 ^ 0x80) & 0xFF = 139 = 0x8B.
; CHECK-LABEL: _test_sgt_const:
; CHECK:      	xor	#128
; CHECK-NEXT: 	cp	#139
; CHECK-NEXT: 	jr	nc,[[YES:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_no
; CHECK:      [[YES]]:
; CHECK-NEXT: 	call	_ext_yes
define void @test_sgt_const(i8 zeroext %val) {
  %c = icmp sgt i8 %val, 10
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}
