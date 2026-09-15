; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

declare void @ext_yes()
declare void @ext_no()

; Case 1: val > 191 (ugt 191, repro from fdc_read_when_ready: (status & 0xC0) == 0xC0)
; Should fold to CP 192; JR NC (no register clobber, no ld b,a / ld a,191 / cp b).
; CHECK-LABEL: _test_ugt_const:
; CHECK:      	cp	#192
; CHECK-NEXT: 	jr	c,[[NO:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_yes
; CHECK:      [[NO]]:
; CHECK-NEXT: 	call	_ext_no
define void @test_ugt_const(i8 zeroext %val) {
  %c = icmp ugt i8 %val, 191
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Case 2: val <= 191 (ule 191)
; Should fold to CP 192; JR C (fallthrough to yes, or jr c to yes).
; CHECK-LABEL: _test_ule_const:
; CHECK:      	cp	#192
; CHECK-NEXT: 	jr	nc,[[NO:\.LBB[0-9_]+]]
; CHECK:      	call	_ext_yes
; CHECK:      [[NO]]:
; CHECK-NEXT: 	call	_ext_no
define void @test_ule_const(i8 zeroext %val) {
  %c = icmp ule i8 %val, 191
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}
