; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; Narrowing: icmp eq/ne (add (zext i8), C), (zext i8) → 8-bit add+cp with carry guard
; Pattern from verify_seek_result: (drive_select + 0x20) != fdc_result.st0
@var1 = external global i8
@var2 = external global i8

; EQ branch: ADD A,C; JP C,fallthrough; CP r; JP Z,target
define i16 @narrow_add_cmp_eq() {
; CHECK-LABEL: _narrow_add_cmp_eq:
; CHECK:       add a,#32
; CHECK-NEXT:  jp c,
; CHECK-NEXT:  cp
; CHECK-NEXT:  jp z,
entry:
  %a = load i8, ptr @var1
  %ext_a = zext i8 %a to i16
  %sum = add nuw nsw i16 %ext_a, 32
  %b = load i8, ptr @var2
  %ext_b = zext i8 %b to i16
  %cmp = icmp eq i16 %sum, %ext_b
  br i1 %cmp, label %then, label %else
then:
  ret i16 1
else:
  ret i16 0
}

; NE branch: ADD A,C; JP C,target; CP r; JP NZ,target
define i16 @narrow_add_cmp_ne() {
; CHECK-LABEL: _narrow_add_cmp_ne:
; CHECK:       add a,#32
; CHECK-NEXT:  jp c,
; CHECK-NEXT:  cp
; CHECK-NEXT:  jp nz,
entry:
  %a = load i8, ptr @var1
  %ext_a = zext i8 %a to i16
  %sum = add nuw nsw i16 %ext_a, 32
  %b = load i8, ptr @var2
  %ext_b = zext i8 %b to i16
  %cmp = icmp ne i16 %sum, %ext_b
  br i1 %cmp, label %then, label %else
then:
  ret i16 1
else:
  ret i16 0
}

; Commuted: add on RHS instead of LHS
define i16 @narrow_add_cmp_commuted() {
; CHECK-LABEL: _narrow_add_cmp_commuted:
; CHECK:       add a,#32
; CHECK-NEXT:  jp c,
; CHECK-NEXT:  cp
; CHECK-NEXT:  jp nz,
entry:
  %a = load i8, ptr @var1
  %ext_a = zext i8 %a to i16
  %b = load i8, ptr @var2
  %ext_b = zext i8 %b to i16
  %sum = add nuw nsw i16 %ext_b, 32
  %cmp = icmp ne i16 %ext_a, %sum
  br i1 %cmp, label %then, label %else
then:
  ret i16 1
else:
  ret i16 0
}
