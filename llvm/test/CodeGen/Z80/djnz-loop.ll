; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs %s -o - | FileCheck %s --check-prefixes=CHECK,Z80
; RUN: llc -mtriple=sm83 -O2 -verify-machineinstrs %s -o - | FileCheck %s --check-prefixes=CHECK,SM83

target triple = "z80"

; Case 1: Simple countdown loop with counter in B: folds to DJNZ on Z80.
define zeroext i8 @sum_bytes(ptr %p, i8 zeroext %n) {
; CHECK-LABEL: _sum_bytes:
; Z80:         .LBB0_1:
; Z80-NOT:       dec b
; Z80-NOT:       jr nz
; Z80:           djnz .LBB0_1
; SM83:          dec {{[bc]}}
; SM83-NEXT:     jr nz,
entry:
  br label %do.body

do.body:
  %p.cur = phi ptr [ %p, %entry ], [ %p.next, %do.body ]
  %n.cur = phi i8 [ %n, %entry ], [ %n.dec, %do.body ]
  %sum.cur = phi i8 [ 0, %entry ], [ %sum.next, %do.body ]
  %val = load i8, ptr %p.cur, align 1
  %sum.next = add i8 %val, %sum.cur
  %p.next = getelementptr inbounds i8, ptr %p.cur, i16 1
  %n.dec = add i8 %n.cur, -1
  %cond = icmp eq i8 %n.dec, 0
  br i1 %cond, label %do.end, label %do.body

do.end:
  ret i8 %sum.next
}

; Case 2: Intervening instruction (inc hl) between dec b and branch:
; INC HL does not touch flags or B, so DEC B sinks and fuses into DJNZ.
define void @countdown_intervening(ptr %p, i8 zeroext %n) {
; CHECK-LABEL: _countdown_intervening:
; Z80:         .LBB1_1:
; Z80-DAG:       inc hl
; Z80-NOT:       dec b
; Z80-NOT:       jr nz
; Z80:           djnz .LBB1_1
; SM83:          dec b
entry:
  br label %loop

loop:
  %p.cur = phi ptr [ %p, %entry ], [ %p.next, %loop ]
  %i.cur = phi i8 [ %n, %entry ], [ %i.dec, %loop ]
  store i8 0, ptr %p.cur, align 1
  %p.next = getelementptr inbounds i8, ptr %p.cur, i16 1
  %i.dec = add i8 %i.cur, -1
  %c = icmp eq i8 %i.dec, 0
  br i1 %c, label %exit, label %loop

exit:
  ret void
}

; Case 3: Decrement of loop counter passed in another register (e.g. L):
; Z80SplitDjnzCounters inserts preheader copy to B, enabling DJNZ on Z80.
define zeroext i8 @dec_l_no_djnz(i8 zeroext %a, i8 zeroext %c_val) {
; CHECK-LABEL: _dec_l_no_djnz:
; Z80:         ld b,l
; Z80:         djnz
; SM83:        dec {{[e-l]}}
; SM83:        jr nz,
entry:
  br label %loop

loop:
  %cur = phi i8 [ %c_val, %entry ], [ %next, %loop ]
  %acc = phi i8 [ %a, %entry ], [ %acc.next, %loop ]
  %acc.next = add i8 %acc, 1
  %next = add i8 %cur, -1
  %cond = icmp eq i8 %next, 0
  br i1 %cond, label %exit, label %loop

exit:
  ret i8 %acc.next
}
