; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s
; RUN: llc -verify-machineinstrs -mtriple=sm83 -z80-asm-format=sdasz80 -O1 < %s \
; RUN:   | FileCheck %s --check-prefix=SM83 --implicit-check-not=ldir

; ravn/llvm-z80#357 (issue #315): memmove between provably non-overlapping
; objects (distinct globals or distinct stack allocas) does not need a runtime
; direction check or libcall and can lower directly to LDIR on Z80.

@buf1 = global [32 x i8] zeroinitializer, align 1
@buf2 = global [32 x i8] zeroinitializer, align 1

declare void @llvm.memmove.p0.p0.i16(ptr nocapture writeonly, ptr nocapture readonly, i16, i1 immarg)
declare void @touch(ptr)

; (a) Distinct global variables: cannot overlap -> LDIR, no libcall
define void @test_memmove_distinct_globals() {
; CHECK-LABEL: _test_memmove_distinct_globals:
; CHECK-NOT:   call
; CHECK:       ld bc,#16
; CHECK:       ldir
; CHECK-NOT:   call
; CHECK:       ret
; SM83-LABEL:  _test_memmove_distinct_globals:
; SM83:        call ___z80_memmove_builtin
  call void @llvm.memmove.p0.p0.i16(ptr @buf2, ptr @buf1, i16 16, i1 false)
  ret void
}

; (b) Distinct stack allocas: cannot overlap -> LDIR, no libcall
define void @test_memmove_distinct_allocas() {
; CHECK-LABEL: _test_memmove_distinct_allocas:
; CHECK-NOT:   call ___z80_memmove_builtin
; CHECK:       ld bc,#16
; CHECK:       ldir
; CHECK-NOT:   call ___z80_memmove_builtin
; CHECK:       ret
; SM83-LABEL:  _test_memmove_distinct_allocas:
; SM83:        call ___z80_memmove_builtin
  %a = alloca [16 x i8], align 1
  %b = alloca [16 x i8], align 1
  call void @touch(ptr %a)
  call void @llvm.memmove.p0.p0.i16(ptr %b, ptr %a, i16 16, i1 false)
  call void @touch(ptr %b)
  ret void
}

; (c) Global vs stack alloca: cannot overlap -> LDIR, no libcall
define void @test_memmove_global_and_alloca() {
; CHECK-LABEL: _test_memmove_global_and_alloca:
; CHECK-NOT:   call ___z80_memmove_builtin
; CHECK:       ld bc,#16
; CHECK:       ldir
; CHECK-NOT:   call ___z80_memmove_builtin
; CHECK:       ret
; SM83-LABEL:  _test_memmove_global_and_alloca:
; SM83:        call ___z80_memmove_builtin
  %local = alloca [16 x i8], align 1
  call void @touch(ptr %local)
  call void @llvm.memmove.p0.p0.i16(ptr @buf1, ptr %local, i16 16, i1 false)
  ret void
}
