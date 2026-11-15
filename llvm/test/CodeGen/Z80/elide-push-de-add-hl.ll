; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs %s -o - | FileCheck %s

target triple = "z80"

@rx_status = external global i8
@packet_errors = external global i8

; Test that saving DE across stack argument reloads when DE is only used for an
; ADD HL, DE that follows is eliminated by hoisting ADD HL, DE before the reload.
define zeroext i8 @test_elide_push_de_add_hl(ptr %base, i16 %offset, i8 zeroext %len, i8 zeroext %delim, ptr %flags) {
; CHECK-LABEL: _test_elide_push_de_add_hl:
; CHECK-NOT:   push de
; CHECK:       add hl,de
; CHECK-NOT:   pop de
entry:
  %0 = load volatile i8, ptr @rx_status, align 1
  %1 = and i8 %0, 1
  %tobool.not = icmp eq i8 %1, 0
  br i1 %tobool.not, label %if.end, label %if.then

if.then:
  %2 = load i8, ptr @packet_errors, align 1
  %inc = add i8 %2, 1
  store i8 %inc, ptr @packet_errors, align 1
  br label %return

if.end:
  %cmp.not17 = icmp eq i8 %len, 0
  br i1 %cmp.not17, label %return, label %for.body.lr.ph

for.body.lr.ph:
  %add.ptr = getelementptr inbounds nuw i8, ptr %base, i16 %offset
  %flags.promoted = load i8, ptr %flags, align 1
  br label %for.body

for.body:
  %storemerge21 = phi i8 [ %flags.promoted, %for.body.lr.ph ], [ %storemerge, %if.end12 ]
  %i.020 = phi i8 [ %len, %for.body.lr.ph ], [ %dec, %if.end12 ]
  %matches.019 = phi i8 [ 0, %for.body.lr.ph ], [ %matches.1, %if.end12 ]
  %ptr.018 = phi ptr [ %add.ptr, %for.body.lr.ph ], [ %uglygep, %if.end12 ]
  %4 = load i8, ptr %ptr.018, align 1
  %cmp5 = icmp eq i8 %4, %delim
  br i1 %cmp5, label %if.then7, label %if.else

if.then7:
  %inc8 = add i8 %matches.019, 1
  %5 = or i8 %storemerge21, -128
  br label %if.end12

if.else:
  %inc11 = add i8 %storemerge21, 1
  br label %if.end12

if.end12:
  %storemerge = phi i8 [ %inc11, %if.else ], [ %5, %if.then7 ]
  %matches.1 = phi i8 [ %matches.019, %if.else ], [ %inc8, %if.then7 ]
  store i8 %storemerge, ptr %flags, align 1
  %uglygep = getelementptr inbounds i8, ptr %ptr.018, i16 1
  %dec = add i8 %i.020, -1
  %cmp.not = icmp eq i8 %dec, 0
  br i1 %cmp.not, label %return, label %for.body

return:
  %retval.0 = phi i8 [ 0, %if.then ], [ 0, %if.end ], [ %matches.1, %if.end12 ]
  ret i8 %retval.0
}
