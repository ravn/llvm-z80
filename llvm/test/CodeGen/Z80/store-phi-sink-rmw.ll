; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs %s -o - | FileCheck %s

target triple = "z80"

; Test that a merged store of a PHI node whose incoming values are RMW ops
; on the same pointer is sunk back into the predecessor blocks, emitting
; in-place SET and INC instructions instead of loading through accumulator A.
define void @test_store_phi_rmw(i1 %c, ptr %flags) {
; CHECK-LABEL: _test_store_phi_rmw:
; CHECK:       set 7,(hl)
; CHECK:       inc (hl)
; CHECK-NOT:   ld (hl), a
; CHECK:       ret
entry:
  %f = load i8, ptr %flags, align 1
  br i1 %c, label %if.then, label %if.else

if.then:
  %f.or = or i8 %f, 128
  br label %if.end

if.else:
  %f.inc = add i8 %f, 1
  br label %if.end

if.end:
  %storemerge = phi i8 [ %f.or, %if.then ], [ %f.inc, %if.else ]
  store i8 %storemerge, ptr %flags, align 1
  ret void
}

; Test that a loop-carried store of a PHI node is also sunk, eliminating
; the loop-carried register and emitting in-place operations on IX.
define void @test_store_phi_loop_rmw(ptr %buf, i8 zeroext %len, i8 zeroext %delim, ptr %flags) {
; CHECK-LABEL: _test_store_phi_loop_rmw:
; CHECK:       push ix
; CHECK:       set 7,(ix+0)
; CHECK:       inc (ix+0)
; CHECK-NOT:   ld (ix+0), a
; CHECK:       pop ix
; CHECK:       ret
entry:
  %cmp.not = icmp eq i8 %len, 0
  br i1 %cmp.not, label %return, label %for.body.lr.ph

for.body.lr.ph:
  %flags.promoted = load i8, ptr %flags, align 1
  br label %for.body

for.body:
  %ptr = phi ptr [ %buf, %for.body.lr.ph ], [ %ptr.next, %if.end ]
  %i = phi i8 [ %len, %for.body.lr.ph ], [ %i.dec, %if.end ]
  %f.carried = phi i8 [ %flags.promoted, %for.body.lr.ph ], [ %storemerge, %if.end ]
  %val = load i8, ptr %ptr, align 1
  %cmp = icmp eq i8 %val, %delim
  br i1 %cmp, label %if.then, label %if.else

if.then:
  %f.or = or i8 %f.carried, 128
  br label %if.end

if.else:
  %f.inc = add i8 %f.carried, 1
  br label %if.end

if.end:
  %storemerge = phi i8 [ %f.inc, %if.else ], [ %f.or, %if.then ]
  store i8 %storemerge, ptr %flags, align 1
  %ptr.next = getelementptr inbounds i8, ptr %ptr, i16 1
  %i.dec = add i8 %i, -1
  %loop.cond = icmp eq i8 %i.dec, 0
  br i1 %loop.cond, label %return, label %for.body

return:
  ret void
}
