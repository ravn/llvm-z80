; RUN: llc -mtriple=z80 -O2 -mattr=+static-stack < %s -o %t.s
; RUN: FileCheck %s < %t.s
;
; Test: BSS self-clear via memcpy(p+1, p, n-1) must not clobber the
; destination pointer when the function's static frame overlaps with
; the BSS region being cleared.
;
; With +static-stack, function locals are in BSS.  If the compiler
; stores p+1 to BSS before *p=0, the zero write corrupts the stored
; pointer, causing LDIR to write to the wrong address.  See issue #51.
;
; The correct codegen must either:
; (a) compute p+1 after the zero store, or
; (b) keep p+1 in a register, not in BSS

@bss = internal global [128 x i8] zeroinitializer

declare void @llvm.memcpy.p0.p0.i16(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i16, i1 immarg)

define void @bss_self_clear() {
entry:
  %p = getelementptr [128 x i8], ptr @bss, i16 0, i16 0
  store i8 0, ptr %p
  %p1 = getelementptr i8, ptr %p, i16 1
  call void @llvm.memcpy.p0.p0.i16(ptr %p1, ptr %p, i16 127, i1 false)
  ret void
}

; CHECK-LABEL: _bss_self_clear:
; DE must hold bss+1 (not a corrupted value) and the zero store
; must happen before LDIR.  Order of DE setup vs zero store may vary.
; CHECK-DAG: ld de,_bss+1
; CHECK-DAG: ld (_bss),a
; CHECK: ldir
