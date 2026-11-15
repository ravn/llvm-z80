; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs %s -o - | FileCheck %s

target triple = "z80"

; Test that a loop-invariant comparison operand is hoisted into A before the loop
; so the loop header does not execute redundant LD A, r copies before CP (HL).
define zeroext i8 @test_loop_hoist(ptr %buf, i8 zeroext %len, i8 zeroext %delim) {
; CHECK-LABEL: _test_loop_hoist:
; CHECK:       ld a,
; CHECK:       .LBB{{[0-9_]+}}: ; %for.body
; CHECK-NOT:   ld a,
; CHECK:       cp (hl)
entry:
  %cmp.not = icmp eq i8 %len, 0
  br i1 %cmp.not, label %return, label %for.body

for.body:
  %p = phi ptr [ %buf, %entry ], [ %p.next, %for.inc ]
  %i = phi i8 [ %len, %entry ], [ %i.dec, %for.inc ]
  %matches = phi i8 [ 0, %entry ], [ %matches.next, %for.inc ]
  %v = load i8, ptr %p, align 1
  %c = icmp eq i8 %v, %delim
  br i1 %c, label %if.then, label %for.inc

if.then:
  %m.inc = add i8 %matches, 1
  br label %for.inc

for.inc:
  %matches.next = phi i8 [ %m.inc, %if.then ], [ %matches, %for.body ]
  %p.next = getelementptr inbounds i8, ptr %p, i16 1
  %i.dec = add i8 %i, -1
  %loop.cond = icmp eq i8 %i.dec, 0
  br i1 %loop.cond, label %return, label %for.body

return:
  %res = phi i8 [ 0, %entry ], [ %matches.next, %for.inc ]
  ret i8 %res
}
