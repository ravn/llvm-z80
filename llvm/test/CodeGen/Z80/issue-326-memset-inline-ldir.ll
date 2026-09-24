; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s
; RUN: llc -verify-machineinstrs -mtriple=sm83 -z80-asm-format=sdasz80 -O1 < %s \
; RUN:   | FileCheck %s --check-prefix=SM83 --implicit-check-not=ldir

; ravn/llvm-z80#326: G_MEMSET with a constant size operand should inline as
; a seed store followed by LDIR, rather than generating a runtime libcall to
; ___z80_memset_builtin.

; C source:
;   void test_memset_constant(void *dst, unsigned char val) {
;       __builtin_memset(dst, val, 64);
;   }
;   void test_memset_one(void *dst, unsigned char val) {
;       __builtin_memset(dst, val, 1);
;   }
; Constant size >= 2: inline as LD (HL),A + LDIR (no libcall).
; Size 1: single LD (HL),A (no LDIR at all — size-1 LDIR wraps to 65536).

declare void @llvm.memset.p0.i16(ptr nocapture writeonly, i8, i16, i1 immarg)

; (a) Constant size >= 2: should inline LD (HL),A followed by LDIR (no libcall)
define void @test_memset_constant(ptr %dst, i8 %val) {
; CHECK-LABEL: _test_memset_constant:
; CHECK-NOT:   call
; CHECK:       ld bc,#63
; CHECK:       ldir
; CHECK-NOT:   call
; CHECK:       ret
; SM83-LABEL:  _test_memset_constant:
; SM83:        call ___z80_memset_builtin
  call void @llvm.memset.p0.i16(ptr %dst, i8 %val, i16 64, i1 false)
  ret void
}

; (b) Boundary case size == 2: seed store + LDIR with BC=1
define void @test_memset_size_two(ptr %dst, i8 %val) {
; CHECK-LABEL: _test_memset_size_two:
; CHECK-NOT:   call
; CHECK:       ld bc,#1
; CHECK:       ldir
; CHECK-NOT:   call
; CHECK:       ret
  call void @llvm.memset.p0.i16(ptr %dst, i8 %val, i16 2, i1 false)
  ret void
}

; (c) Boundary case size == 1: only seed store, NO LDIR and NO call
define void @test_memset_size_one(ptr %dst, i8 %val) {
; CHECK-LABEL: _test_memset_size_one:
; CHECK-NOT:   call
; CHECK-NOT:   ldir
; CHECK:       ret
  call void @llvm.memset.p0.i16(ptr %dst, i8 %val, i16 1, i1 false)
  ret void
}

; (d) Boundary case size == 0: completely erased
define void @test_memset_size_zero(ptr %dst, i8 %val) {
; CHECK-LABEL: _test_memset_size_zero:
; CHECK-NOT:   call
; CHECK-NOT:   ldir
; CHECK:       ret
  call void @llvm.memset.p0.i16(ptr %dst, i8 %val, i16 0, i1 false)
  ret void
}

; (e) Variable size: guarded LDIR without call on Z80; libcall on SM83
define void @test_memset_variable(ptr %dst, i8 %val, i16 %n) {
; CHECK-LABEL: _test_memset_variable:
; CHECK-NOT:   call
; CHECK:       ldir
; CHECK-NOT:   call
; CHECK:       ret
; SM83-LABEL:  _test_memset_variable:
; SM83:        call ___z80_memset_builtin
  call void @llvm.memset.p0.i16(ptr %dst, i8 %val, i16 %n, i1 false)
  ret void
}
