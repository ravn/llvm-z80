; RUN: llc -verify-machineinstrs -mtriple=sm83 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

declare void @llvm.memset.p0.i16(ptr nocapture writeonly, i8, i16, i1 immarg)

define void @test_sm83_memset(ptr %dst, i8 %val, i16 %n) {
; CHECK-LABEL: _test_sm83_memset:
; CHECK:       call _memset
; CHECK-NOT:   ldir
  call void @llvm.memset.p0.i16(ptr %dst, i8 %val, i16 %n, i1 false)
  ret void
}
