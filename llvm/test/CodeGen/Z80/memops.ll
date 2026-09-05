; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

declare void @llvm.memcpy.p0.p0.i16(ptr nocapture writeonly, ptr nocapture readonly, i16, i1 immarg)
declare void @llvm.memmove.p0.p0.i16(ptr nocapture, ptr nocapture readonly, i16, i1 immarg)
declare void @llvm.memset.p0.i16(ptr nocapture writeonly, i8, i16, i1 immarg)

; Test: memcpy lowers to inline LDIR on Z80, with a runtime BC==0
; guard for the variable-size case (#105 — LDIR with BC=0 runs 65536
; iterations).
define void @test_memcpy(ptr %dst, ptr %src, i16 %n) {
; CHECK-LABEL: _test_memcpy:
; CHECK:       ld a,b
; CHECK-NEXT:  or c
; CHECK-NEXT:  jr z,
; CHECK:       ldir
  call void @llvm.memcpy.p0.p0.i16(ptr %dst, ptr %src, i16 %n, i1 false)
  ret void
}

; Test: runtime-unknown-direction memmove lowers to the register-CC helper
; __memmove_rt (z80_allreg), not the heavy stack-ABI _memmove (ravn/llvm-z80#126).
define void @test_memmove(ptr %dst, ptr %src, i16 %n) {
; CHECK-LABEL: _test_memmove:
; CHECK:       call ___memmove_rt
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 %n, i1 false)
  ret void
}

; Test: memset lowers to inline LDIR fill on Z80, with runtime size==0
; and size==1 guards (#105).  val is held in E across the BC test so
; `ld a,b; or c` doesn't clobber it; the leading store therefore goes
; via `ld (hl),e`.
define void @test_memset(ptr %dst, i8 %val, i16 %n) {
; CHECK-LABEL: _test_memset:
; CHECK:       ld a,b
; CHECK-NEXT:  or c
; CHECK-NEXT:  jr z,
; CHECK:       ld (hl),e
; CHECK:       ldir
  call void @llvm.memset.p0.i16(ptr %dst, i8 %val, i16 %n, i1 false)
  ret void
}

; Test: memset with zero val also goes through the LDIR-fill guard.
define void @test_memset_zero(ptr %dst, i16 %n) {
; CHECK-LABEL: _test_memset_zero:
; CHECK:       ld a,b
; CHECK-NEXT:  or c
; CHECK-NEXT:  jr z,
; CHECK:       ldir
  call void @llvm.memset.p0.i16(ptr %dst, i8 0, i16 %n, i1 false)
  ret void
}
