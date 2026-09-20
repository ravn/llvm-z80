; RUN: llc -mtriple=z80 -O2 -mattr=+static-frame < %s -o - | FileCheck %s
;
; #335 investigation fixture #3 — the strongest form of the described bug:
; the buffer being cleared lives in the function's own static frame, so if
; the compiler spills %p1 into that same frame before *p=0, the zero store
; corrupts the spilled pointer and LDIR runs off the rails.
;
; This is the scenario the XFAILed bss-self-clear.ll comment describes but
; does not itself construct — its @bss is a global, and its function has
; no locals so no static frame is allocated.

declare void @clobber()
declare void @llvm.memcpy.p0.p0.i16(ptr noalias nocapture writeonly,
                                    ptr noalias nocapture readonly,
                                    i16, i1 immarg)

define void @bss_self_clear_local() {
entry:
  %buf = alloca [128 x i8], align 1
  %p   = getelementptr [128 x i8], ptr %buf, i16 0, i16 0
  %p1  = getelementptr i8, ptr %p, i16 1
  ; Force %p1 to live across a call, so if it is spilled it goes into the
  ; static frame that overlaps with %buf.
  call void @clobber()
  store i8 0, ptr %p
  call void @llvm.memcpy.p0.p0.i16(ptr %p1, ptr %p, i16 127, i1 false)
  ret void
}

; CHECK-LABEL: _bss_self_clear_local:
; CHECK:       ldir
