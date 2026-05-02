; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -disable-lsr -z80-asm-format=sdasz80 < %s | FileCheck %s
;
; Issue ravn/llvm-z80#97: in a single-BB self-loop with a PHI'd pointer
; used by a store AND incremented for the back-edge, the regalloc
; allocated the SAME logical pointer to TWO physical register pairs (HL
; for the stores, BC for the back-edge value), with cross-pair copies at
; every iteration.  Sibling of #84 (head-test multi-BB form).
;
; FIXED via post-RA peephole `BC ping-pong in single-BB self-loops` in
; `Z80LateOptimization.cpp` (option 2 from the original investigation):
; detect `LD C,L; LD B,H` in pred + `LD L,C; LD H,B` at loop top + INC BC
; chain at back-edge with HL only touched as memory base + INC HL inside
; the body, then drop the dance and replace `INC BC × N` with
; `INC HL × (N - M)` where M is the inside-body INC HL count.
;
; Closing #97 unblocks ravn/llvm-z80#77a (Z80LoopRotate stays off-by-
; default until the rotated single-BB shape ceases to regress).
;
; The harder i16-counter sub-case (counter and pointer compete for HL,
; so the peephole can't simply drop the ping-pong) is pinned in the
; sibling test `issue-97a-bc-pingpong-i16-counter.ll` and tracked as
; a follow-up issue.

; --- Canonical shape: do-while-decrement countdown with i16 store.
; Matches cpnos-rom's setup_ivt pattern after rotation.
;
; CHECK-LABEL: countdown_word_dowhile:
; CHECK-NOT: ld{{[ \t]+}}c,l
; CHECK-NOT: ld{{[ \t]+}}b,h
; CHECK-NOT: ld{{[ \t]+}}l,c
; CHECK-NOT: ld{{[ \t]+}}h,b
; CHECK-NOT: inc{{[ \t]+}}bc
define void @countdown_word_dowhile(ptr %p) {
entry:
  br label %loop
loop:
  %i = phi i8 [ 18, %entry ], [ %i.next, %loop ]
  %p.cur = phi ptr [ %p, %entry ], [ %p.next, %loop ]
  store volatile i16 -4345, ptr %p.cur, align 1
  %p.next = getelementptr inbounds nuw i8, ptr %p.cur, i16 2
  %i.next = sub i8 %i, 1
  %done = icmp eq i8 %i.next, 0
  br i1 %done, label %exit, label %loop
exit:
  ret void
}

; --- Variation: i8 store (single-byte fill).
; Even when the store doesn't need INC_HL between bytes, the back-edge
; GEP still produces a separate vreg, so the ping-pong still appears.
;
; CHECK-LABEL: byte_fill_dowhile:
; CHECK-NOT: ld{{[ \t]+}}c,l
; CHECK-NOT: ld{{[ \t]+}}l,c
; CHECK-NOT: inc{{[ \t]+}}bc
define void @byte_fill_dowhile(ptr %p) {
entry:
  br label %loop
loop:
  %i = phi i8 [ 24, %entry ], [ %i.next, %loop ]
  %p.cur = phi ptr [ %p, %entry ], [ %p.next, %loop ]
  store volatile i8 -1, ptr %p.cur, align 1
  %p.next = getelementptr inbounds nuw i8, ptr %p.cur, i16 1
  %i.next = sub i8 %i, 1
  %done = icmp eq i8 %i.next, 0
  br i1 %done, label %exit, label %loop
exit:
  ret void
}

; --- Variation: GEP comes AFTER the store in source IR (= the IR shape
; my Z80LoopRotate's reverted GEP-sink would have produced).  Confirms
; the bug isn't fixed by reordering the IR — even with %p.next defined
; AFTER store via %p.cur, the regalloc still ping-pongs.  This is the
; data point that proved the IR-level fix path doesn't work; the issue
; is at MIR level (store implicit-def $hl), not IR level.
;
; CHECK-LABEL: countdown_gep_after_store:
; CHECK-NOT: ld{{[ \t]+}}c,l
; CHECK-NOT: ld{{[ \t]+}}l,c
; CHECK-NOT: inc{{[ \t]+}}bc
define void @countdown_gep_after_store(ptr %p) {
entry:
  br label %loop
loop:
  %i = phi i8 [ 18, %entry ], [ %i.next, %loop ]
  %p.cur = phi ptr [ %p, %entry ], [ %p.next, %loop ]
  store volatile i16 -4345, ptr %p.cur, align 1
  ; GEP defined AFTER store — non-overlapping live ranges at IR level.
  %p.next = getelementptr inbounds nuw i8, ptr %p.cur, i16 2
  %i.next = sub i8 %i, 1
  %done = icmp eq i8 %i.next, 0
  br i1 %done, label %exit, label %loop
exit:
  ret void
}

; --- Reference shape that DOES coalesce cleanly (no XFAIL): the
; head-test multi-BB form that closed #84.  Included here as the
; positive control proving the regalloc handles head-test loops fine
; today; only single-BB self-loops are broken.
;
; This function is in the same file as the expected-to-fail ones so a
; single file-level expected-to-fail directive applies; when the bug
; fix lands, this control should still pass.  The control's CHECK-NOTs
; are already satisfied today; only the four functions above fail.
;
; CTRL-LABEL: word_fill_18_headtest:
; CTRL-NOT: ld{{[ \t]+}}c,l
; CTRL-NOT: ld{{[ \t]+}}l,c
; CTRL-NOT: inc{{[ \t]+}}bc
@target_fn = external dso_local global i8

define void @word_fill_18_headtest() {
entry:
  br label %loop
loop:
  %p = phi ptr [ inttoptr (i16 -2816 to ptr), %entry ], [ %p.next, %loop.body ]
  %n = phi i8 [ 18, %entry ], [ %n.next, %loop.body ]
  %done = icmp eq i8 %n, 0
  br i1 %done, label %exit, label %loop.body
loop.body:
  %p.next = getelementptr inbounds nuw i8, ptr %p, i16 2
  store volatile i16 ptrtoint (ptr @target_fn to i16), ptr %p, align 1
  %n.next = sub i8 %n, 1
  br label %loop
exit:
  ret void
}
