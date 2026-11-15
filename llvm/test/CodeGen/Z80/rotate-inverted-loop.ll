; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs %s -o - | FileCheck %s

target triple = "z80"

; Test that an inverted loop (where the preheader jumps into the header over the latch,
; and the latch falls through into the header with a conditional exit on zero)
; is rotated to natural order:
; 1. Preheader falls through into loop header (eliding the 2-byte preheader jump).
; 2. Latch branches back to header with JR NZ (or DJNZ), falling through to exit.

define zeroext i8 @parse_packet_loop(ptr %base, i16 %offset, i8 zeroext %len, i8 zeroext %delim, ptr %flags) {
; CHECK-LABEL: _parse_packet_loop:
; CHECK:       ; %for.body.lr.ph
; CHECK-NOT:     jr .LBB0_
; CHECK:       .LBB0_{{[0-9]+}}: ; %for.body
; CHECK:       ; %if.end12
; CHECK-NOT:     jr z,
; CHECK:         djnz
entry:
  %cmp.not17 = icmp eq i8 %len, 0
  br i1 %cmp.not17, label %return, label %for.body.lr.ph

for.body.lr.ph:
  %add.ptr = getelementptr inbounds i8, ptr %base, i16 %offset
  br label %for.body

for.body:
  %i.020 = phi i8 [ %len, %for.body.lr.ph ], [ %dec, %if.end12 ]
  %matches.019 = phi i8 [ 0, %for.body.lr.ph ], [ %matches.1, %if.end12 ]
  %ptr.018 = phi ptr [ %add.ptr, %for.body.lr.ph ], [ %ptr.next, %if.end12 ]
  %val = load i8, ptr %ptr.018, align 1
  %cmp5 = icmp eq i8 %val, %delim
  br i1 %cmp5, label %if.then7, label %if.else

if.then7:
  %inc8 = add i8 %matches.019, 1
  br label %if.end12

if.else:
  %flag.val = load i8, ptr %flags, align 1
  %inc.flag = add i8 %flag.val, 1
  store i8 %inc.flag, ptr %flags, align 1
  br label %if.end12

if.end12:
  %matches.1 = phi i8 [ %matches.019, %if.else ], [ %inc8, %if.then7 ]
  %ptr.next = getelementptr inbounds i8, ptr %ptr.018, i16 1
  %dec = add i8 %i.020, -1
  %cmp.not = icmp eq i8 %dec, 0
  br i1 %cmp.not, label %return, label %for.body

return:
  %retval = phi i8 [ 0, %entry ], [ %matches.1, %if.end12 ]
  ret i8 %retval
}
