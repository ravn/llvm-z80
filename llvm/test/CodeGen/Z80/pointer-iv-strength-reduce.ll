; RUN: llc -O2 -mtriple=z80 -z80-loop-instr-form-prep < %s | FileCheck %s
; RUN: llc -O2 -mtriple=z80 < %s | FileCheck %s --check-prefix=OFF

; ravn/llvm-z80#250: byte-array loops with a non-constant stride used to
; recompute the base address every iteration (`ld hl,_flags; add hl, ...`)
; instead of walking a running pointer.  Z80LoopInstrFormPrep rewrites the
; offset-IV `gep(@flags, %k)` into a genuine pointer IV threaded through the
; loop, so the loop body only needs a single `add hl,de` per iteration.

@flags = dso_local global [8191 x i8] zeroinitializer

; CHECK-LABEL: kill:
; The base address must be materialised once, before the loop, NOT inside it.
; CHECK: ld hl,_flags
; CHECK-LABEL: .LBB0_1:
; CHECK-NOT: ld hl,_flags
; CHECK: add hl,de
; CHECK: jr

; With the pass OFF (the shipping default) the base is reloaded every
; iteration -- the #250 pattern this pass is designed to remove.
; OFF-LABEL: kill:
; OFF-LABEL: .LBB0_1:
; OFF: ld hl,_flags
; OFF: add hl,bc

define void @kill(i16 %start, i16 %prime) {
entry:
  br label %loop
loop:
  %k = phi i16 [ %start, %entry ], [ %kn, %loop ]
  %p = getelementptr inbounds i8, ptr @flags, i16 %k
  store i8 0, ptr %p, align 1
  %kn = add i16 %k, %prime
  %c = icmp ult i16 %kn, 8191
  br i1 %c, label %loop, label %exit
exit:
  ret void
}
