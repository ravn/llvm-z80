; RUN: llc -O2 -disable-lsr -mtriple=z80 -mattr=+static-stack \
; RUN:     -z80-loop-instr-form-prep < %s \
; RUN:   | FileCheck %s
; RUN: llc -O2 -disable-lsr -mtriple=z80 -mattr=+static-stack < %s \
; RUN:   | FileCheck %s --check-prefix=OFF

; ravn/llvm-z80#250, Phase-1a fix + cost gate.  A zero-trip-guarded byte-array
; loop (`if (c == 0) skip`) enters via a CONDITIONAL branch, so the loop has no
; dedicated preheader in the unsimplified (-disable-lsr) CFG.  Two things are
; exercised here:
;   (1) Z80LoopInstrFormPrep inserts a preheader on demand (it used to bail at
;       `!L->getLoopPreheader()`, leaving the #250 base reload in the loop);
;   (2) the profitability/cost gate (canEliminateOldIV) admits this loop --
;       its single relational exit test (`icmp ult %inext, %c`) lets the old
;       integer counter be eliminated, so the rewrite is a genuine win, not a
;       register-pressure regression.  A non-relational or compound exit would
;       be declined and left byte-identical.
; See tasks/session-2026-07-12-issue250-phase1a-spike.md.

@arr = dso_local global [256 x i8] zeroinitializer

; ---------------------------------------------------------------------------
; The Z80 loop body we are matching (concrete asm from this test's llc output).
;
; OFF (pass disabled, shipping default) -- the #250 bug.  The base pointer is
; reconstructed from @arr + i on EVERY iteration:
;
;   .LBB0_2:              ; %loop, Depth=1
;       ld  hl,_arr       ; \ base @arr reloaded ...
;       add hl,bc         ; / hl = &arr[i]   <-- #250: base rebuilt each iter
;       ld  a,7
;       ld  (hl),a        ; arr[i] = 7
;       inc bc            ; i++              (integer counter)
;       ld  a,c           ; \
;       sub e             ;  | i != c exit test on the integer counter
;       ld  a,b           ;  |
;       sbc a,d           ; /
;       jr  c,.LBB0_2
;
; CHECK / ON (pass enabled) -- the fix.  The base @arr is materialised once in
; the inserted preheader; the loop walks a running pointer in DE (`ld (de),a` /
; `inc de`), so there is NO `ld hl,_arr` / `add hl,bc` (no base reload) left in
; the body, and the old counter is gone -- the exit test compares the walking
; pointer against a precomputed end pointer:
;
;   ; %bb.1:              ; %loop.preheader   <-- inserted on demand
;       ld  de,_arr       ; walking pointer start = &arr[0]
;       ld  hl,_arr       ; \ end pointer = &arr[c] (loop-invariant)
;       add hl,bc         ; /
;   .LBB0_2:              ; %loop, Depth=1
;       ld  a,7
;       ld  (de),a        ; *p = 7           -- DE is the walking pointer
;       inc de            ; p++              <-- pointer walk, no base recompute
;       ld  a,e           ; \
;       sub l             ;  | p != end exit test (pointer compare)
;       ld  a,d           ;  |
;       sbc a,h           ; /
;       jr  c,.LBB0_2
;
; So the discriminating instruction is `ld hl,_arr` (the base reload) inside
; .LBB0_2: present with the pass OFF, absent with it ON.
; ---------------------------------------------------------------------------

; With the pass ON the base is materialised once in the (on-demand) preheader
; and the loop body walks a running pointer -- no per-iteration base reload.
; CHECK-LABEL: _guarded_fill:
; CHECK: .LBB0_2:
; CHECK-NOT: ld hl,_arr
; CHECK: jr {{n?c?z?,?}}.LBB0_2

; With the pass OFF (shipping default) the loop reconstructs the address every
; iteration -- the #250 pattern.  This is the control half of the red/green.
; OFF-LABEL: _guarded_fill:
; OFF: .LBB0_2:
; OFF: ld hl,_arr
; OFF: jr {{n?c?z?,?}}.LBB0_2

define void @guarded_fill(i16 %c) {
entry:
  ; Zero-trip guard -> conditional entry -> no dedicated preheader in the
  ; unsimplified CFG.  This is the shape the on-demand-preheader fix targets.
  %z = icmp eq i16 %c, 0
  br i1 %z, label %exit, label %loop

loop:
  %i = phi i16 [ 0, %entry ], [ %inext, %loop ]
  %addr = getelementptr inbounds i8, ptr @arr, i16 %i
  ; volatile so the fill is not dead-code-eliminated (@arr is never read).
  store volatile i8 7, ptr %addr, align 1
  %inext = add i16 %i, 1
  ; Single relational exit test -> cost gate admits (old counter eliminable).
  %cont = icmp ult i16 %inext, %c
  br i1 %cont, label %loop, label %exit

exit:
  ret void
}
