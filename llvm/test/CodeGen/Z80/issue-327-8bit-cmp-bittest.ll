; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; C source:
;   typedef signed char int8_t;
;   void ext_yes(void); void ext_no(void);
;   void test_slt_zero(int8_t x) { if (x < 0) ext_yes(); else ext_no(); }
;   void test_sge_zero(int8_t x) { if (x >= 0) ext_yes(); else ext_no(); }
; val < 0 (signed) → BIT 7,A; JR NZ (4 B) instead of full signed compare.

declare void @ext_yes()
declare void @ext_no()

; Case 1: val < 0 (slt 0) -> BIT 7, val; JR NZ to yes
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

; Case 2: val >= 0 (sge 0) -> BIT 7, val; fallthrough to yes, JR NZ to no
; CHECK-LABEL: _test_sge_zero:
; CHECK:      	bit	7,a
; CHECK-NEXT: 	jr	nz,[[NO:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_yes
; CHECK:      [[NO]]:
; CHECK-NEXT: 	call	_ext_no
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

; Case 3: val > -1 (sgt -1) -> same as val >= 0 -> BIT 7, val; fallthrough to yes, JR NZ to no
; CHECK-LABEL: _test_sgt_minus_one:
; CHECK:      	bit	7,a
; CHECK-NEXT: 	jr	nz,[[NO:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_yes
; CHECK:      [[NO]]:
; CHECK-NEXT: 	call	_ext_no
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

; Case 4: val <= -1 (sle -1) -> same as val < 0 -> BIT 7, val; JR NZ to yes
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
; CHECK-NEXT: 	jr	nc,[[NO:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_yes
; CHECK:      [[NO]]:
; CHECK-NEXT: 	call	_ext_no
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
; CHECK-NEXT: 	jr	c,[[NO:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_yes
; CHECK:      [[NO]]:
; CHECK-NEXT: 	call	_ext_no
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

; Case 7: two variables (fallback to 2x XOR 0x80 + CP r)
; CHECK-LABEL: _test_slt_vars:
; CHECK:      	xor	#128
; CHECK:      	xor	#128
; CHECK:      	cp	{{[a-z]+}}
; CHECK:      	ret
define i8 @test_slt_vars(i8 zeroext %a, i8 zeroext %b) {
  %c = icmp slt i8 %a, %b
  br i1 %c, label %yes, label %no
yes:
  ret i8 1
no:
  ret i8 0
}
