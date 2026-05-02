; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -disable-lsr -z80-asm-format=sdasz80 < %s | FileCheck %s
;
; XFAIL: *
;
; Issue ravn/llvm-z80#97 i16-counter sub-case: harder sibling of the
; main #97 issue.  The post-RA peephole (option 2) closed the i8-counter
; cases by dropping the BC ping-pong and keeping the pointer in HL — but
; here the counter is also i16 and competes with the pointer for HL.
;
; Today's MIR (before late-opt) for this shape spills the counter to
; BSS via `LD_nnind_HL` / `LD_HL_nnind` across the LD_L_C; LD_H_B reload
; (which the BSS->PUSH/POP peephole later rewrites to PUSH HL ; POP HL).
; The post-RA peephole guards bail because HL is defined inside the
; loop body window — the spill/reload counts as a non-pointer HL touch.
;
; Closing this would need either:
;   - a regalloc-level swap so the counter goes in BC and the pointer
;     stays in HL throughout (closes by allocation, not peephole), OR
;   - a more invasive peephole that rewrites the counter from HL to BC
;     by substituting all DEC_HL / LD_A_L / OR_H references.
;
; Practically the i16-counter rotated-loop shape doesn't appear in
; cpnos-rom or rcbios today (counters are i8 / DJNZ-eligible), so this
; sub-case is pinned but parked until a real-code instance shows up.

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
