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
; #91: with constant Size, the legalizer constant-folds Size-1 and the
; chained G_PTR_ADD offsets so each end pointer becomes one
; G_PTR_ADD(@buf, k); the existing ISel fold then emits a direct
; `LD HL, _buf+const`.  Result: src+31 = _buf+33, dst+31 = _buf+39,
; LDDR -- no register juggling, no BSS spill.
define void @memmove_global_offsets() {
  %dst = getelementptr inbounds i8, ptr @buf, i16 8
  %src = getelementptr inbounds i8, ptr @buf, i16 2
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 32, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_global_offsets:
; CHECK-NOT:  call _memmove
; CHECK:      ld  hl,_buf+33
; CHECK-NEXT: ld  de,_buf+39
; CHECK-NEXT: ld  bc,32
; CHECK-NEXT: lddr
; CHECK-NEXT: ret


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


; --- src is a RUNTIME base (itself a G_PTR_ADD), dst = src + 4 → LDDR ----
; The common screen-scroll shape: base = p + runtime_idx, then
; memmove(base + K, base, n) with K a positive constant.  dst = base + 4,
; direction = sign(4) = LDDR, even though `base` is not a leaf pointer.
; Regression: the direction analysis used to require src to be a leaf
; (getPtrAddOff set SrcBase as a side effect, tripping a SrcBase==0 guard),
; so this fell back to __memmove_rt.
define void @memmove_dst_after_runtime_base(ptr %p, i16 %idx) {
  %base = getelementptr inbounds i8, ptr %p, i16 %idx
  %dst = getelementptr inbounds i8, ptr %base, i16 4
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %base, i16 16, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_dst_after_runtime_base:
; CHECK-NOT:  call _memmove
; CHECK-NOT:  ___memmove_rt
; CHECK:      lddr
; CHECK:      ret


; --- src is a RUNTIME base, dst = src - 4 → LDIR -------------------------
define void @memmove_dst_before_runtime_base(ptr %p, i16 %idx) {
  %base = getelementptr inbounds i8, ptr %p, i16 %idx
  %dst = getelementptr inbounds i8, ptr %base, i16 -4
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %base, i16 16, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_dst_before_runtime_base:
; CHECK-NOT:  call _memmove
; CHECK-NOT:  ___memmove_rt
; CHECK-NOT:  lddr
; CHECK:      ldir
; CHECK:      ret


; --- runtime pointers, unknown direction → register-CC __memmove_rt (#126) -
; dst/src already in HL/DE, size in BC -> just `ld bc,16; jp ___memmove_rt`
; (tail call), not the heavy stack-ABI _memmove libcall.
define void @memmove_runtime(ptr %dst, ptr %src) {
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 16, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_runtime:
; CHECK:      ld bc,16
; CHECK:      jp ___memmove_rt


@sbuf = external dso_local global [4096 x i8]

; --- runtime-count LDDR: cancel a common runtime term in the end pointer -----
; The screen-scroll idiom:  base = sbuf + i;  memmove(base + 80, base, 1920 - i).
; dst = src + 80 > src -> LDDR, whose end pointer is src + Size - 1 =
; (sbuf + i) + (1920 - i) - 1 = buf + 1919: the runtime i cancels between the
; pointer's +i and Size's -i, so both end pointers fold to buf+const
; (`LD HL, _sbuf+1919` / `LD DE, _sbuf+1999`) with no runtime `add`.
define void @memmove_cancel_runtime_term(i16 %i) {
  %size = sub i16 1920, %i
  %src = getelementptr [4096 x i8], ptr @sbuf, i16 0, i16 %i
  %dst = getelementptr i8, ptr %src, i16 80
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 %size, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_cancel_runtime_term:
; CHECK-NOT:  call _memmove
; CHECK-NOT:  ___memmove_rt
; CHECK:      ld hl,_sbuf+1919
; CHECK:      ld de,_sbuf+1999
; CHECK:      lddr
; CHECK:      ret


; --- negative control: unrelated runtime size -> NO cancel, runtime add ------
; Size (%n) is not `C - i`, so the end pointer stays src + (Size-1) computed at
; runtime; it must NOT fold to a buf+const end pointer.
define void @memmove_no_cancel(i16 %i, i16 %n) {
  %src = getelementptr [4096 x i8], ptr @sbuf, i16 0, i16 %i
  %dst = getelementptr i8, ptr %src, i16 80
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 %n, i1 false)
  ret void
}
; CHECK-LABEL: _memmove_no_cancel:
; CHECK-NOT:  ld hl,_sbuf+1919
; CHECK-NOT:  ld de,_sbuf+1999
; CHECK:      lddr
; CHECK:      ret


declare void @llvm.memmove.p0.p0.i16(ptr, ptr, i16, i1 immarg)
