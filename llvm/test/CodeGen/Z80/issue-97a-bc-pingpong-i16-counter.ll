; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -disable-lsr -z80-asm-format=sdasz80 < %s | FileCheck %s
;
; XFAIL: *
;
; Issue ravn/llvm-z80#99 (filed end of session 35): the i16-counter
; sub-case of #97 (closed via post-RA peephole in Z80LateOptimization.cpp).
; The peephole drops the BC ping-pong and keeps the pointer in HL, but
; here the counter is also i16 and competes with the pointer for HL.
;
; Session 39 status (Phase 3): Z80SplitDjnzCounters and the BCReg
; single-register class were added (i8 path closed #94, i16 path
; partial for #99).  The i16 path identifies the DEC16 self-back-edge
; counter and constrains it to BCReg, so greedy now puts the counter
; in BC.
;
; Remaining gap: the function-arg pointer arrives in $hl (sdcccall)
; and the coalescer copies it to BC at entry.  With the counter ALSO
; constrained to BC, regalloc spills the counter to BSS each
; iteration instead of evicting the pointer's BC allocation.  Net
; asm is different (BSS-spill via __sfrend slot instead of PUSH HL /
; POP HL ping-pong) but still doesn't satisfy the CHECK-NOT below.
;
; Closing this fully needs the sister HLReg constraint for the
; pointer vreg — pointer constrained to HL forces it to stay where
; sdcccall delivered it, freeing BC unambiguously for the counter.
; Filed as Phase 3 follow-up; the i16 case is rare enough in
; real-world Z80 code (counters are i8 / DJNZ-eligible) that the
; full HLReg coordination is not yet justified.

; --- i16 counter, i16 store: pointer + counter both want HL.
;
; CHECK-LABEL: countdown_i16_counter:
; CHECK-NOT: ld{{[ \t]+}}c,l
; CHECK-NOT: ld{{[ \t]+}}l,c
; CHECK-NOT: inc{{[ \t]+}}bc
define void @countdown_i16_counter(ptr %p) {
entry:
  br label %loop
loop:
  %i = phi i16 [ 256, %entry ], [ %i.next, %loop ]
  %p.cur = phi ptr [ %p, %entry ], [ %p.next, %loop ]
  store volatile i16 0, ptr %p.cur, align 1
  %p.next = getelementptr inbounds nuw i8, ptr %p.cur, i16 2
  %i.next = sub i16 %i, 1
  %done = icmp eq i16 %i.next, 0
  br i1 %done, label %exit, label %loop
exit:
  ret void
}
