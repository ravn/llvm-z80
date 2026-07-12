; RUN: llc -O2 -disable-lsr -mtriple=z80 -mattr=+static-stack \
; RUN:     -z80-loop-instr-form-prep -z80-pin-loop-pointer < %s \
; RUN:   | FileCheck %s
; RUN: llc -O2 -disable-lsr -mtriple=z80 -mattr=+static-stack \
; RUN:     -z80-loop-instr-form-prep -z80-pin-loop-pointer \
; RUN:     -z80-loop-instr-form-prep-allow-nested < %s \
; RUN:   | FileCheck %s --check-prefix=NESTED

; ravn/llvm-z80#250 nesting gate.  Rewriting a loop that is NESTED inside
; another loop adds a 3rd live 16-bit value (new pointer + enclosing loop's
; IV + stride) to a target with exactly 3 GP pairs and no spare, which
; empirically regresses the sieve KILL loop (+1.31M T-states).  So by default
; the pass DECLINES nested loops and only rewrites flat / outermost ones.  The
; hidden -z80-loop-instr-form-prep-allow-nested hatch re-enables the rewrite
; for experiments.  See tasks/session-2026-07-12-issue250-phase1a-spike.md.

@arr = external dso_local global [256 x i8]

; Default: the inner (Depth=2) loop is NESTED, so the pass declines it and the
; base is reconstructed every iteration -- `ld hl,_arr; add hl,<idx>`:
;
;   .LBB0_?:              ; inner, Depth=2
;       ld  hl,_arr       ; reload base
;       add hl,bc         ; hl = arr + k   <-- reload kept (nested, declined)
;       ld  (hl),d
;       ...
;
; CHECK-LABEL: _nested:
; CHECK: Depth=2
; CHECK: ld hl,_arr
; CHECK: add hl,bc

; With the escape hatch the nested loop IS rewritten to a walking pointer, so
; no base is reloaded inside the inner loop (the store becomes `ld (bc),d`):
;
; NESTED-LABEL: _nested:
; NESTED: Depth=2
; NESTED-NOT: ld hl,_arr
; NESTED-NOT: add hl,bc

define dso_local void @nested(i16 %n, i16 %m) {
entry:
  %mz = icmp eq i16 %m, 0
  br i1 %mz, label %exit, label %oload

oload:
  %nz = icmp eq i16 %n, 0
  br label %outer

outer:
  %j = phi i16 [ 0, %oload ], [ %jn, %latch ]
  br i1 %nz, label %latch, label %ipre

ipre:
  %jb = trunc i16 %j to i8
  br label %inner

inner:
  %k = phi i16 [ 0, %ipre ], [ %kn, %inner ]
  %addr = getelementptr inbounds nuw i8, ptr @arr, i16 %k
  store i8 %jb, ptr %addr, align 1
  %kn = add nuw i16 %k, 1
  %kdone = icmp eq i16 %kn, %n
  br i1 %kdone, label %latch, label %inner

latch:
  %jn = add nuw i16 %j, 1
  %jdone = icmp eq i16 %jn, %m
  br i1 %jdone, label %exit, label %outer

exit:
  ret void
}
