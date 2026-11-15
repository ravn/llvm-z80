; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs %s -o - | FileCheck %s

target triple = "z80"

; Test that a secondary pointer can be allocated to IX when hasFP is false,
; allowing direct in-place modification via set 7, (ix+0) without spilling.
define zeroext i8 @parse_packet_ix(ptr %base, i16 %offset, i8 zeroext %len, i8 zeroext %delim, ptr %flags) {
; CHECK-LABEL: _parse_packet_ix:
; CHECK:       push ix
; CHECK:       set 7, (ix
; CHECK:       pop ix
; CHECK:       ret
entry:
  %cmp.not = icmp eq i8 %len, 0
  br i1 %cmp.not, label %return, label %for.body.lr.ph

for.body.lr.ph:
  %add.ptr = getelementptr inbounds i8, ptr %base, i16 %offset
  br label %for.body

for.body:
  %ptr = phi ptr [ %add.ptr, %for.body.lr.ph ], [ %ptr.next, %for.inc ]
  %i = phi i8 [ %len, %for.body.lr.ph ], [ %i.dec, %for.inc ]
  %matches = phi i8 [ 0, %for.body.lr.ph ], [ %matches.next, %for.inc ]
  %val = load i8, ptr %ptr, align 1
  %cmp = icmp eq i8 %val, %delim
  br i1 %cmp, label %if.then, label %for.inc

if.then:
  %matches.inc = add i8 %matches, 1
  %f = load i8, ptr %flags, align 1
  %f.or = or i8 %f, 128
  store i8 %f.or, ptr %flags, align 1
  br label %for.inc

for.inc:
  %matches.next = phi i8 [ %matches.inc, %if.then ], [ %matches, %for.body ]
  %ptr.next = getelementptr inbounds i8, ptr %ptr, i16 1
  %i.dec = add i8 %i, -1
  %loop.cond = icmp eq i8 %i.dec, 0
  br i1 %loop.cond, label %return, label %for.body

return:
  %retval = phi i8 [ 0, %entry ], [ %matches.next, %for.inc ]
  ret i8 %retval
}
