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
