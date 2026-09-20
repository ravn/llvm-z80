; RUN: llc -mtriple=z80 -O2 < %s -o - | FileCheck %s --check-prefix=DEFAULT
; RUN: llc -mtriple=z80 -O2 < %s -o - | FileCheck %s --check-prefix=STATIC
;
; #335 investigation fixture #2 — %p is a runtime pointer (function argument),
; %p+1 must live somewhere across the intervening call. This is the scenario
; the fixture comment in bss-self-clear.ll describes: if the pointer or the
; pointer+1 gets spilled and then reloaded, the reload must not equal the
; store pointer, or LDIR self-copies.

declare void @clobber()
declare void @llvm.memcpy.p0.p0.i16(ptr noalias nocapture writeonly,
                                    ptr noalias nocapture readonly,
                                    i16, i1 immarg)

define void @bss_self_clear_arg(ptr %p) {
entry:
  %p1 = getelementptr i8, ptr %p, i16 1
  call void @clobber()
  store i8 0, ptr %p
  call void @llvm.memcpy.p0.p0.i16(ptr %p1, ptr %p, i16 127, i1 false)
  ret void
}

; Both configurations: DE (memcpy dest) must not equal HL (memcpy src) at
; LDIR entry. Concretely %p+1 != %p.
; DEFAULT-LABEL: _bss_self_clear_arg:
; DEFAULT:       ldir
; STATIC-LABEL:  _bss_self_clear_arg:
; STATIC:        ldir
