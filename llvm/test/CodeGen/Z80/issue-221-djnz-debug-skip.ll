; RUN: llc -O2 -mtriple=z80 < %s | FileCheck %s
;
; ravn/llvm-z80#221 -- the `LD A,r; DEC A; LD r,A; OR A; JR NZ` peephole
; (Z80LateOptimization.cpp:812) and the DJNZ peephole (884) use std::next(I)
; that lands on DBG_VALUE pseudo-instructions when `-g` is on, silently
; bailing the rewrite.  Each affected loop costs 3 B (LD A,r case) or
; 4 B (DJNZ case).
;
; This test asserts the peephole fires on a shape that explicitly has
; the LD A,r entry (counter in non-A register, requires reload).  Pattern
; mirrors the production autoload `_delay` mid-counter site.
;
; The simpler `do { } while (--n)` shape doesn't exercise the LD A,r
; entry because the counter stays in A across iterations (no reload).
; To force the entry, the loop body must use A for something else
; (here: passing %n as a call arg, which clobbers A).

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

; Mirrors the production _delay nested-loop shape from the issue body.
; The inner loop's DJNZ claims B; the middle counter (%mid) lives in a
; non-B reg (typically H), and updating it requires the LD A,H; DEC A;
; LD H,A; OR A; JR NZ five-MI pattern that the peephole rewrites to
; DEC H; JR NZ.  Production autoload `_delay` is the canonical
; consumer (verified to drop 9 B with this peephole firing).
define void @delay(i8 %outer, i8 %inner) {
entry:
  %nonzero = icmp ne i8 %outer, 0
  br i1 %nonzero, label %outer.loop, label %exit
outer.loop:
  %o = phi i8 [ %outer, %entry ], [ %o.next, %outer.end ]
  br label %mid.loop
mid.loop:
  %m = phi i8 [ %inner, %outer.loop ], [ %m.next, %mid.end ]
  br label %inner.loop
inner.loop:
  %k = phi i8 [ 0, %mid.loop ], [ %k.next, %inner.loop ]
  call void asm sideeffect "", ""()
  %k.next = sub i8 %k, 1
  %k.done = icmp eq i8 %k.next, 0
  br i1 %k.done, label %mid.end, label %inner.loop
mid.end:
  %m.next = sub i8 %m, 1
  %m.done = icmp eq i8 %m.next, 0
  br i1 %m.done, label %outer.end, label %mid.loop
outer.end:
  %o.next = sub i8 %o, 1
  %o.done = icmp eq i8 %o.next, 0
  br i1 %o.done, label %exit, label %outer.loop
exit:
  ret void
}

; CHECK-LABEL: delay:
; The mid loop's counter (LBB0_3 ... LBB0_5 region) MUST end with the
; post-fix `dec r; jr nz` shape, NOT the pre-fix 5-MI shape that
; included `or a` between `ld r, a` and `jr nz`.
;
; Post-fix asm (the peephole fired):
;   .LBB0_5: dec h ; jr nz, .LBB0_3   (2 ops, 3 B)
;
; Pre-fix asm (peephole bailed under -g):
;   ld a, h ; dec a ; ld h, a ; or a ; jr nz, .LBB0_3   (5 ops, 6 B)
;
; Assert the post-fix instruction at LBB0_5 is dec-then-jr, not the
; LD-DEC-LD-OR-JR chain.
; CHECK: %bb.5:
; CHECK: dec{{[ \t]+}}h
; CHECK: jr{{[ \t]+}}nz
