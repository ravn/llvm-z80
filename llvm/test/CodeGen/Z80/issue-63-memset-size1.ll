; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; ravn/llvm-z80#63 — G_MEMSET (and G_MEMCPY, G_MEMMOVE) lowered with a
; small constant size used to emit "store first byte; DE=HL+1; BC=size-1;
; LDIR".  When size==1, BC=0 fed into LDIR causes a 65536-byte runaway
; that trashes all 64 KB of memory; when size==0 in G_MEMCPY, the same
; trap fires.  At -O0 there is no DCE to mask the bug, so any 1-byte
; array initializer (e.g. `uint8_t s[] = "";`) corrupts memory.
;
; The fix special-cases constant-size 0 and 1 in the legalizer: size 0
; erases the op, size 1 emits a single byte store/load.  This test
; checks that no LDIR is emitted for these tiny constant sizes.

declare void @llvm.memset.p0.i16(ptr nocapture, i8, i16, i1)
declare void @llvm.memcpy.p0.p0.i16(ptr nocapture, ptr nocapture, i16, i1)
declare void @llvm.memmove.p0.p0.i16(ptr nocapture, ptr nocapture, i16, i1)

; The bug is LDIR/LDDR with BC=0 (which runs 65536 iterations).  Ensure
; that each degenerate constant size is handled without that pattern:
;   - memset/memcpy/memmove size 0 → erased entirely;
;   - memset size 1                → only the leading store, no LDIR;
;   - memcpy size 1                → LDIR is fine here (BC=1 runs once).

; CHECK-LABEL: _memset_size_one:
; CHECK-NOT:   ld{{[id]}}{{i?}}r
; CHECK:       ret
define void @memset_size_one(ptr %dst, i8 %v) {
  call void @llvm.memset.p0.i16(ptr %dst, i8 %v, i16 1, i1 false)
  ret void
}

; CHECK-LABEL: _memset_size_zero:
; CHECK-NOT:   ld{{[id]}}{{i?}}r
; CHECK:       ret
define void @memset_size_zero(ptr %dst, i8 %v) {
  call void @llvm.memset.p0.i16(ptr %dst, i8 %v, i16 0, i1 false)
  ret void
}

; CHECK-LABEL: _memcpy_size_zero:
; CHECK-NOT:   ld{{[id]}}{{i?}}r
; CHECK:       ret
define void @memcpy_size_zero(ptr %dst, ptr %src) {
  call void @llvm.memcpy.p0.p0.i16(ptr %dst, ptr %src, i16 0, i1 false)
  ret void
}

; CHECK-LABEL: _memmove_size_zero:
; CHECK-NOT:   ld{{[id]}}{{i?}}r
; CHECK:       ret
define void @memmove_size_zero(ptr %dst, ptr %src) {
  call void @llvm.memmove.p0.p0.i16(ptr %dst, ptr %src, i16 0, i1 false)
  ret void
}
