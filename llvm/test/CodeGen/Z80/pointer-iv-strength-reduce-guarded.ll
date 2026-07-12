; RUN: llc -O2 -disable-lsr -mtriple=z80 -mattr=+static-stack \
; RUN:     -z80-loop-instr-form-prep -z80-pin-loop-pointer < %s \
; RUN:   | FileCheck %s
; RUN: llc -O2 -disable-lsr -mtriple=z80 -mattr=+static-stack < %s \
; RUN:   | FileCheck %s --check-prefix=OFF

; ravn/llvm-z80#250, Phase-1a fix.  A zero-trip-guarded byte-array loop
; (`if (c == 0) skip`) enters the loop via a CONDITIONAL branch, so it has no
; dedicated preheader until LoopSimplify inserts one.  Z80LoopInstrFormPrep
; used to bail at its first guard (`!L->getLoopPreheader()`), leaving the #250
; base reload (`ld l,(base); ld h,(base); add hl,de`) in the loop.  It only bit
; the production `-disable-lsr` pipeline, because LSR was the pass that pulled
; LoopSimplify in; -disable-lsr removed both.  The fix makes the pass require
; LoopSimplify form (AU.addRequiredID(LoopSimplifyID)), so the preheader is
; inserted and the offset-IV is rewritten into a walking pointer regardless of
; whether LSR runs.  See tasks/session-2026-07-12-issue250-phase1a-spike.md.

@arr = dso_local global [256 x i8] zeroinitializer

; ---------------------------------------------------------------------------
; The Z80 loop body we are matching (concrete asm from this test's llc output).
;
; OFF (pass disabled, shipping default) -- the #250 bug.  The base pointer is
; reconstructed from p + i on EVERY iteration:
;
;   .LBB0_2:              ; %loop, Depth=1
;       ld  l,(ix+-2)     ; \ reload base p from its frame slot
;       ld  h,(ix+-1)     ; /
;       add hl,de         ; hl = p + i   <-- #250: base rebuilt each iteration
;       ld  a,(hl)        ; a = p[i]
;       ld  l,c           ; hl = &val (also reloaded)
;       ld  h,b
;       cp  (hl)          ; p[i] == val ?
;       jr  nz,.LBB0_4
;       inc de            ; i++
;       ... i != c exit test ...
;       jr  nz,.LBB0_2
;
; CHECK / ON (pass enabled) -- the fix.  The base is materialised once in the
; preheader; the loop just walks a running pointer in BC (inc bc), so there is
; NO `add hl,de` (and no base reload) left in the body:
;
;   .LBB0_2:              ; %loop, Depth=1
;       ld  a,(bc)        ; a = *pc     -- BC is the walking pointer
;       ld  l,(ix+-2)     ; hl = &val (loop-invariant, in a frame slot)
;       ld  h,(ix+-1)
;       cp  (hl)          ; *pc == val ?
;       jr  nz,.LBB0_4
;       inc de            ; i++         (counter kept only for the exit test)
;       inc bc            ; pc++        <-- pointer walk, no base recompute
;       ... i != c exit test ...
;       jr  nz,.LBB0_2
;
; So the single discriminating instruction is `add hl,de` inside .LBB0_2:
; present with the pass OFF, absent with it ON.
; ---------------------------------------------------------------------------

; With the pass ON the base is materialised once in the preheader and the loop
; body walks a running pointer -- no per-iteration base reconstruction.
; CHECK-LABEL: _guarded_scan:
; CHECK: .LBB0_2:
; CHECK-NOT: add hl,de
; CHECK: jr {{n?z?,?}}.LBB0_2

; With the pass OFF (shipping default) the loop reconstructs the address every
; iteration -- the #250 pattern.  This is the control half of the red/green.
; OFF-LABEL: _guarded_scan:
; OFF: .LBB0_2:
; OFF: add hl,de
; OFF: jr {{n?z?,?}}.LBB0_2

define i16 @guarded_scan(ptr %p, i8 zeroext %val, i16 %c) {
entry:
  ; Zero-trip guard -> conditional entry -> no dedicated preheader in the
  ; unsimplified CFG.  This is the shape the fix targets.
  %z = icmp eq i16 %c, 0
  br i1 %z, label %exit, label %loop

loop:
  %i = phi i16 [ 0, %entry ], [ %inext, %loop ]
  %addr = getelementptr inbounds i8, ptr %p, i16 %i
  %v = load i8, ptr %addr, align 1
  %ne = icmp ne i8 %v, %val
  %inext = add i16 %i, 1
  %done = icmp eq i16 %inext, %c
  %or = or i1 %ne, %done
  br i1 %or, label %exit, label %loop

exit:
  %r = phi i16 [ 0, %entry ], [ 1, %loop ]
  ret i16 %r
}
