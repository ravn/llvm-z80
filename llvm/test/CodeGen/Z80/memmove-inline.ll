; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 < %s | FileCheck %s

; memmove with statically-determinable dst/src direction is inlined as
; LDIR (when dst <= src) or LDDR (when dst >= src) instead of falling
; back to a `call _memmove` runtime stub.  When both pointers come
; from a common base via constant GEPs, or when one is a constant GEP
; off the other, the legalizer can compute the direction and pick.

@buf = external dso_local global [256 x i8]


; --- dst = src + 4: dst > src → LDDR ---------------------------------
define void @memmove_dst_after_src(ptr %src) {
  %dst = getelementptr inbounds i8, ptr %src, i16 4
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 16, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_dst_after_src:
; CHECK-NOT:  call _memmove
; CHECK:      lddr
; CHECK:      ret


; --- dst = src - 4: dst < src → LDIR ---------------------------------
define void @memmove_dst_before_src(ptr %src) {
  %dst = getelementptr inbounds i8, ptr %src, i16 -4
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 16, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_dst_before_src:
; CHECK-NOT:  call _memmove
; CHECK-NOT:  lddr
; CHECK:      ldir
; CHECK:      ret


; --- src = dst + 4: dst < src → LDIR ---------------------------------
define void @memmove_src_after_dst(ptr %dst) {
  %src = getelementptr inbounds i8, ptr %dst, i16 4
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 16, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_src_after_dst:
; CHECK-NOT:  call _memmove
; CHECK-NOT:  lddr
; CHECK:      ldir
; CHECK:      ret


; --- common-base GEPs: dst=buf+8, src=buf+2 → dst>src → LDDR ---------
define void @memmove_global_offsets() {
  %dst = getelementptr inbounds i8, ptr @buf, i16 8
  %src = getelementptr inbounds i8, ptr @buf, i16 2
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 32, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_global_offsets:
; CHECK-NOT:  call _memmove
; CHECK:      lddr
; CHECK:      ret


; --- identical pointers: no-op ---------------------------------------
define void @memmove_noop(ptr %p) {
  call void @llvm.memmove.p0.p0.i16(ptr %p, ptr %p, i16 64, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_noop:
; CHECK-NOT:  call _memmove
; CHECK-NOT:  ldir
; CHECK-NOT:  lddr
; CHECK:      ret


; --- runtime pointers, unknown direction → libcall (regression lock) -
define void @memmove_runtime(ptr %dst, ptr %src) {
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 16, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_runtime:
; CHECK:      call _memmove
; CHECK:      ret


declare void @llvm.memmove.p0.p0.i16(ptr, ptr, i16, i1 immarg)
