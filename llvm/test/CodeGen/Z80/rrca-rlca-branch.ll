; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

declare void @ext_yes()
declare void @ext_no()
declare void @use_a(i8 zeroext %val)

; Pattern 1: AND 1 with ne 0 -> RRCA; JR C
; CHECK-LABEL: test_and1_jr_nz:
; CHECK:       rrca
; CHECK-NEXT:  jr	c,
define void @test_and1_jr_nz(i8 zeroext %val) {
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

; Pattern 2: AND 1 with eq 0 -> RRCA; JR NC
; CHECK-LABEL: test_and1_jr_z:
; CHECK:       rrca
; CHECK-NEXT:  jr	nc,
define void @test_and1_jr_z(i8 zeroext %val) {
  %t = and i8 %val, 1
  %c = icmp eq i8 %t, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @ext_no()
  ret void
}

; Pattern 3: Negative control - A is live on fall-through, must keep AND 1
; CHECK-LABEL: test_and1_a_live:
; CHECK:       and	#1
; CHECK-NOT:   rrca
; CHECK:       jr	{{nz|z}},
define void @test_and1_a_live(i8 zeroext %val) {
  %t = and i8 %val, 1
  %c = icmp ne i8 %t, 0
  br i1 %c, label %yes, label %no
yes:
  call void @ext_yes()
  ret void
no:
  call void @use_a(i8 zeroext %t)
  ret void
}
