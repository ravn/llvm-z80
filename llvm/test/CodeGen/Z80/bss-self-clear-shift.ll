; RUN: llc -mtriple=z80 -O2 < %s -o - | FileCheck %s
; RUN: llc -mtriple=z80 -O2 --z80-static-frames < %s -o - | FileCheck %s
;
; #335 investigation fixture #1 — canonical "shift left by one to zero-fill"
; against a global array. %p and %p+1 are constant addresses; ISel folds
; them so no spill can happen. The correct codegen must have DE = _bss+1
; and HL = _bss at LDIR entry, whether that is achieved as `ld de,_bss+1`
; or `ld de,_bss; inc de`.

@bss = internal global [128 x i8] zeroinitializer

declare void @llvm.memcpy.p0.p0.i16(ptr noalias nocapture writeonly,
                                    ptr noalias nocapture readonly,
                                    i16, i1 immarg)

define void @bss_self_clear_global() {
entry:
  %p  = getelementptr [128 x i8], ptr @bss, i16 0, i16 0
  store i8 0, ptr %p
  %p1 = getelementptr i8, ptr %p, i16 1
  call void @llvm.memcpy.p0.p0.i16(ptr %p1, ptr %p, i16 127, i1 false)
  ret void
}

; CHECK-LABEL: _bss_self_clear_global:
; DE reaches _bss+1 (either directly or via inc de).
; CHECK-DAG:   ld hl,_bss
; CHECK-DAG:   {{ld[[:space:]]+de,_bss\+1|inc[[:space:]]+de}}
; CHECK:       ldir
