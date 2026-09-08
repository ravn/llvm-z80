; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

declare void @llvm.z80.halt()
declare void @llvm.z80.di()
declare void @llvm.z80.ei()
declare void @llvm.z80.nop()
declare void @llvm.z80.im2()
declare void @llvm.z80.set.i(i8)

; Test: HALT intrinsic
; CHECK-LABEL: test_halt:
; CHECK:      	halt
; CHECK:      	ret
define void @test_halt() {
  call void @llvm.z80.halt()
  ret void
}

; Test: DI and EI intrinsics for interrupt control
define void @test_di_ei() {
  call void @llvm.z80.di()
  call void @llvm.z80.ei()
; CHECK-LABEL: test_di_ei:
; CHECK:      	di
; CHECK:      	ei
; CHECK:      	ret
  ret void
}

; Test: NOP intrinsic
define void @test_nop() {
  call void @llvm.z80.nop()
  ret void
}

; Test: IM 2 intrinsic (select interrupt mode 2)
define void @test_im2() {
; CHECK-LABEL: test_nop:
; CHECK:      	nop
; CHECK:      	ret
  call void @llvm.z80.im2()
  ret void
}

; Test: LD I,A intrinsic (set interrupt vector register from a value)
define void @test_set_i(i8 %v) {
  call void @llvm.z80.set.i(i8 %v)
  ret void
}
; CHECK-LABEL: test_im2:
; CHECK:      	call	_llvm.z80.im2
; CHECK:      	ret
; CHECK-LABEL: test_set_i:
; CHECK:      	call	_llvm.z80.set.i
; CHECK:      	ret
