; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s
;
; Regression guard for ravn/llvm-z80#335 (originally XFAILed with a stricter
; CHECK than needed; see the issue comments 2026-09-17). The canonical
; "shift left by one to zero-fill" idiom must not produce a self-copy at
; LDIR entry: HL must hold `_bss` and DE must hold `_bss+1`.
;
; The three companion fixtures cover the wider shape space:
;   - bss-self-clear-shift.ll        (global-array, no spill)
;   - bss-self-clear-arg-spill.ll    (pointer argument, spill across call)
;   - bss-self-clear-local-buf.ll    (stack-local buffer, spill across call)
; Runtime oracle: z80-utils/test-runner testcases test_94_bss_self_clear.c
; and test_bss_self_clear_335.c.

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
; DE must reach _bss+1 (either directly or via inc de). HL must be _bss.
; CHECK-DAG:   ld hl,_bss
; CHECK-DAG:   {{ld[[:space:]]+de,_bss\+1|inc[[:space:]]+de}}
; CHECK:       ldir
