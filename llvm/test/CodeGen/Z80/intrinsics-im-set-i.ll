; RUN: llc -mtriple=z80 -O2 -stop-after=instruction-select < %s | FileCheck %s --check-prefix=MIR
; RUN: llc -verify-machineinstrs -mtriple=z80 -O2 < %s | FileCheck %s
;
; llvm.z80.im0/im1/im2 select IM 0/1/2; llvm.z80.set.i selects LD I,A.
; LD_I_A reads A, so the copy of the value into A must survive: with a
; constant (or a value that lives outside A) the load of A must be emitted.
;
; C equivalent (through the intrinsics):
;   void modes(void)            { im0(); im1(); im2(); }
;   void vec_const(void)        { set_i(0x40); }
;   void vec_arg(u8 page)       { set_i(page); }
;   void vec_second(u8 a, u8 b) { set_i(b); }      /* b is not in A */

declare void @llvm.z80.im0()
declare void @llvm.z80.im1()
declare void @llvm.z80.im2()
declare void @llvm.z80.set.i(i8)

; MIR-LABEL: name: modes
; MIR:       IM_0
; MIR-NEXT:  IM_1
; MIR-NEXT:  IM_2
; CHECK-LABEL: _modes:
; CHECK:       im 0
; CHECK-NEXT:  im 1
; CHECK-NEXT:  im 2
define void @modes() {
  call void @llvm.z80.im0()
  call void @llvm.z80.im1()
  call void @llvm.z80.im2()
  ret void
}

; The constant must be loaded into A first; without Uses = [A] on LD_I_A the
; copy was removed as dead and LD I,A used whatever A happened to hold.
; MIR-LABEL: name: vec_const
; MIR:       LD_I_A implicit $a
; CHECK-LABEL: _vec_const:
; CHECK:       ld a,64
; CHECK-NEXT:  ld i,a
define void @vec_const() {
  call void @llvm.z80.set.i(i8 64)
  ret void
}

; The argument already arrives in A.
; CHECK-LABEL: _vec_arg:
; CHECK-NOT:   ld a,
; CHECK:       ld i,a
define void @vec_arg(i8 %page) {
  call void @llvm.z80.set.i(i8 %page)
  ret void
}

; The second argument is not in A, so it has to be moved there.
; CHECK-LABEL: _vec_second:
; CHECK:       ld a,
; CHECK-NEXT:  ld i,a
define void @vec_second(i8 %a, i8 %b) {
  call void @llvm.z80.set.i(i8 %b)
  ret void
}
