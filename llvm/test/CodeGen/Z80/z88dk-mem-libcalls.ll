; RUN: llc -mtriple=z80-unknown-none-z88dk -z80-asm-format=z88dk -O2 < %s | FileCheck %s --check-prefix=Z88DK
; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s --check-prefix=ELF
;
; z88dk triple: memmove with unknown direction calls asm_memmove (HL=src, DE=dst, BC=count).
; z88dk triple: memset calls asm_memset (HL=dst, DE=val, BC=count).
; ELF path uses standard compiler-rt memmove/memset libcalls.

declare void @llvm.memmove.p0.p0.i16(ptr, ptr, i16, i1)
declare void @llvm.memset.p0.i16(ptr, i8, i16, i1)

define void @test_memmove(ptr %dst, ptr %src, i16 %n) {
; Z88DK-LABEL: _test_memmove:
; Z88DK:         call asm_memmove
; Z88DK-NOT:     call _memmove
;
; ELF-LABEL: _test_memmove:
; ELF:           call ___z80_memmove_builtin
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 %n, i1 false)
  ret void
}

define void @test_memset(ptr %dst, i8 %val, i16 %n) {
; Z88DK-LABEL: _test_memset:
; Z88DK:         call asm_memset
; Z88DK-NOT:     call _memset
;
; ELF-LABEL: _test_memset:
; ELF:           call ___z80_memset_builtin
  call void @llvm.memset.p0.i16(ptr %dst, i8 %val, i16 %n, i1 false)
  ret void
}
