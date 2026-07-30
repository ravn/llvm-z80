; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 -z80-verify-inline-runtime-size < %s | FileCheck %s

; ravn/llvm-z80#105 — variable-size memcpy/memmove/memset on Z80
; lower to inline LDIR.  LDIR with BC=0 runs 65536 iterations (BC test
; happens AFTER decrement), which trashes 64 KB of memory.
;
; #63 closed the constant-size sub-cases (size==0 → erase, size==1
; memset → emit only the leading store).  This test covers the
; variable-size case: the legalizer emits `LDIR_GUARDED` (and the
; equivalent for LDDR/memset) which expands at post-RA into a
; runtime guard that skips the LDIR when BC == 0.
;
; Expected expansion of the guarded LDIR:
;   LD A, B
;   OR C            ; sets Z if BC == 0
;   JR Z, .skip
;   LDIR
; .skip:

declare void @llvm.memcpy.p0.p0.i16(ptr nocapture writeonly,
                                    ptr nocapture readonly,
                                    i16, i1 immarg)
declare void @llvm.memset.p0.i16(ptr nocapture writeonly, i8, i16,
                                 i1 immarg)

; CHECK-LABEL: _test_memcpy_var:
; The size-zero guard must precede LDIR.
; CHECK:       ld      a,b
; CHECK-NEXT:  or      c
; CHECK-NEXT:  jr      z,
; CHECK:       ldir

define void @test_memcpy_var(ptr %dst, ptr %src, i16 %n) {
entry:
  call void @llvm.memcpy.p0.p0.i16(ptr %dst, ptr %src, i16 %n, i1 false)
  ret void
}

; CHECK-LABEL: _test_memset_var:
; For memset, the guard must wrap the entire fill (including the
; leading byte store), because size==0 means "no bytes set".
; CHECK:       ld      a,b
; CHECK-NEXT:  or      c
; CHECK-NEXT:  jr      z,
; CHECK:       ldir

define void @test_memset_var(ptr %dst, i8 %v, i16 %n) {
entry:
  call void @llvm.memset.p0.i16(ptr %dst, i8 %v, i16 %n, i1 false)
  ret void
}
