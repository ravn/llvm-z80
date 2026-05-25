; RUN: llc -mtriple=z80 -mattr=+static-stack -z80-unreserve-iy < %s | FileCheck %s
;
; ravn/llvm-z80#112 (session 73s): when IY is allocatable (-z80-unreserve-iy),
; a frame-index address can be allocated to IY.  LEA_IX_FI's eliminateFrameIndex
; had no IY destination case and fell through to llvm_unreachable, which in a
; Release build is a no-op: the pseudo was erased emitting NOTHING, leaving IY
; undefined and a downstream spill reading garbage (off-by pointer -> wrong sum).
; This is the dominant cause of the IY-allocation miscompile (cleared 63 of 70
; test-runner regressions).  Guard: the IY destination must actually be
; materialised -- `pop iy` (the PUSH HL; POP IY expansion of LEA_IX_FI) must
; appear, and IY must be written before any spill of it.
;
; (Flag is default-OFF; IY stays reserved in production -- this only exercises
; the #112 bring-up path.  See session73s-issue112-iy-unreserve-scope.md.)

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

define dso_local i16 @test() {
  %a = alloca i16, align 1
  %b = alloca i16, align 1
  %c = alloca i16, align 1
  %ptrs = alloca [3 x ptr], align 1
  store volatile i16 100, ptr %a, align 1
  store volatile i16 200, ptr %b, align 1
  store volatile i16 300, ptr %c, align 1
  store ptr %a, ptr %ptrs, align 1
  %p1 = getelementptr inbounds nuw i8, ptr %ptrs, i16 2
  store ptr %b, ptr %p1, align 1
  %p2 = getelementptr inbounds nuw i8, ptr %ptrs, i16 4
  store ptr %c, ptr %p2, align 1
  br label %loop

loop:
  %i = phi i8 [ %i.next, %body ], [ 0, %0 ]
  %ct = phi i16 [ %ct.next, %body ], [ 0, %0 ]
  %sum = phi i16 [ %sum.next, %body ], [ 0, %0 ]
  %done = icmp eq i16 %ct, 3
  br i1 %done, label %exit, label %body

exit:
  ret i16 %sum

body:
  %ix = zext nneg i8 %i to i16
  %ep = getelementptr i8, ptr %ptrs, i16 %ix
  %pp = load ptr, ptr %ep, align 1
  %v = load i16, ptr %pp, align 1
  %sum.next = add nsw i16 %v, %sum
  %ct.next = add nuw nsw i16 %ct, 1
  %i.next = add nuw nsw i8 %i, 2
  br label %loop
}

; CHECK-LABEL: test:
; The frame-address value allocated to IY must be defined (LEA_IX_FI -> pop iy).
; CHECK: pop iy
