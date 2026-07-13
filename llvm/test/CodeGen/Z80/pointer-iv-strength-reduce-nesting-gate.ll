; Force the nesting gate ON (nested rewriting is now auto-on at -O2 since
; sink-cold-iv frees the pressure; this test still pins the DECLINE behaviour,
; so it disables allow-nested) and isolate form-prep from pin/hbf/sink-cold-iv.
; RUN: llc -O2 -disable-lsr -mtriple=z80 -mattr=+static-stack \
; RUN:     -z80-loop-instr-form-prep-allow-nested=false \
; RUN:     -z80-enable-pin-loop-pointer=false -z80-enable-hbf-branch=false \
; RUN:     -z80-enable-sink-cold-loop-iv=false < %s | FileCheck %s

; ravn/llvm-z80#250 nesting gate + cost gate.  Rewriting a loop that is NESTED
; inside another loop adds a 3rd live 16-bit value (new pointer + enclosing
; loop's IV + stride) to a target with exactly BC/DE/HL and no spare, which
; empirically regresses the sieve KILL loop.  So the pass DECLINES nested loops
; and only strength-reduces flat / outermost ones.  This test pins BOTH sides:
;   * @nested  -- an inner (Depth=2) loop that is otherwise fully eliminable
;                 (single relational exit `icmp ult %kn, %n`); the ONLY reason
;                 it is declined is the nesting gate, so its base is still
;                 reloaded every iteration -- the #250 pattern left in place.
;   * @flat    -- the SAME loop body at Depth=1; the pass rewrites it into a
;                 walking pointer (positive control), so no base reload remains.
; Both run under prep only (no -z80-pin-loop-pointer): the prep pass is the
; cost-gated, production-safe half; pin is a separate opt-in machine pass that
; regresses -O2 code and is not exercised here.
; See tasks/session-2026-07-12-issue250-phase1a-spike.md.

@arr = external dso_local global [256 x i8]

; ---------------------------------------------------------------------------
; @nested: the inner loop is NESTED, so the gate declines it and the base @arr
; is reconstructed every iteration -- `ld hl,_arr; add hl,bc` inside Depth=2:
;
;   .LBB0_5:              ; %inner, Depth=2
;       ld  hl,_arr       ; \ base reloaded ...
;       add hl,bc         ; / hl = &arr[k]   <-- #250 reload kept (nested)
;       ld  (hl),d
;       ...
; ---------------------------------------------------------------------------

; CHECK-LABEL: _nested:
; CHECK: This Inner Loop Header: Depth=2
; CHECK: ld hl,_arr
; CHECK: add hl,bc

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
  ; Relational exit -> the inner loop is eliminable; only the nesting gate
  ; (not the cost gate) declines it.
  %kcont = icmp ult i16 %kn, %n
  br i1 %kcont, label %inner, label %latch
latch:
  %jn = add nuw i16 %j, 1
  %jdone = icmp eq i16 %jn, %m
  br i1 %jdone, label %exit, label %outer
exit:
  ret void
}

; ---------------------------------------------------------------------------
; @flat: identical loop body, NOT nested.  The pass rewrites it: the base @arr
; is materialised once in the preheader and the loop walks a running pointer in
; DE (`ld (de),a; inc de`), so NO `ld hl,_arr` reload appears inside the loop
; body -- the positive control proving the gate declines nesting, not this
; shape.
;
;   ; %bb.1:              ; %loop.preheader
;       ld  de,_arr       ; walking pointer start
;       ld  hl,_arr       ; \ end pointer (loop-invariant, in preheader ONLY)
;       add hl,bc         ; /
;   .LBB1_2:              ; %loop, Depth=1
;       ld  a,9
;       ld  (de),a        ; *p = 9
;       inc de            ; p++          <-- no base recompute in the body
;       ...
; ---------------------------------------------------------------------------

; CHECK-LABEL: _flat:
; CHECK: This Inner Loop Header
; CHECK-NOT: ld hl,_arr
; CHECK: ret

define dso_local void @flat(i16 %n) {
entry:
  %z = icmp eq i16 %n, 0
  br i1 %z, label %exit, label %loop
loop:
  %k = phi i16 [ 0, %entry ], [ %kn, %loop ]
  %addr = getelementptr inbounds nuw i8, ptr @arr, i16 %k
  store volatile i8 9, ptr %addr, align 1
  %kn = add nuw i16 %k, 1
  %kcont = icmp ult i16 %kn, %n
  br i1 %kcont, label %loop, label %exit
exit:
  ret void
}
