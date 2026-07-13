; Isolate Z80LoopInstrFormPrep (the companion pin/hbf/sink-cold-iv passes are now
; auto-on at -O2; disable them so this test pins just the form-prep rewrite).
; RUN: llc -O2 -mtriple=z80 -z80-enable-pin-loop-pointer=false \
; RUN:     -z80-enable-hbf-branch=false -z80-enable-sink-cold-loop-iv=false < %s \
; RUN:   | FileCheck %s
; OFF control: force the whole stack off (it is otherwise auto-on at -O2).
; RUN: llc -O2 -mtriple=z80 -z80-enable-loop-instr-form-prep=false < %s \
; RUN:   | FileCheck %s --check-prefix=OFF

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

; With the pass forced OFF the base is reloaded every iteration -- the #250
; pattern this pass removes (auto-on at -O2 since it now beats dcc on sieve).
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
