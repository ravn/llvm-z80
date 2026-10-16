; RUN: llc -mtriple=z80 -O1 -z80-static-frames -verify-machineinstrs < %s | FileCheck %s

; Issue #418: External noreturn calls (such as jump_ccp in firmware, or exit/panic)
; must not be treated as creating a recursion cycle back to the caller frame.
; An external call that never returns cannot complete a recursive invocation of
; the current activation frame, so the caller remains non-reentrant and eligible
; for a static frame.

; External declarations
declare void @external_exit(i8) noreturn
declare void @external_normal()

; -----------------------------------------------------------------------------
; Shape (a) Exact pattern from bug report:
; A function calling an external noreturn function.
; Must receive a static frame, not dynamic stack frame allocation.
; -----------------------------------------------------------------------------
; CHECK-LABEL: test_external_noreturn:
; CHECK-NOT: push ix
; CHECK-NOT: dec sp
; CHECK: ld hl,L_test_external_noreturn.frame
; CHECK: call _external_exit
define void @test_external_noreturn() {
  %slot = alloca i8, align 1
  store volatile i8 42, ptr %slot, align 1
  call void @external_exit(i8 0)
  unreachable
}

; -----------------------------------------------------------------------------
; Shape (b) Structural variations:
; Variation 1: Caller of a function that calls external noreturn.
; Both the helper and its caller should receive static frames.
; -----------------------------------------------------------------------------
; CHECK-LABEL: helper_calls_noreturn:
; CHECK-NOT: push ix
; CHECK-NOT: dec sp
; CHECK: ld (L_helper_calls_noreturn.frame),a
; CHECK: call _external_exit
define internal void @helper_calls_noreturn(i8 %val) {
  %slot = alloca i8, align 1
  store volatile i8 %val, ptr %slot, align 1
  call void @external_exit(i8 %val)
  unreachable
}

; CHECK-LABEL: caller_of_helper:
; CHECK-NOT: push ix
; CHECK-NOT: dec sp
; CHECK: ld hl,L_caller_of_helper.frame
; CHECK: call _helper_calls_noreturn
define void @caller_of_helper() {
  %slot = alloca i8, align 1
  store volatile i8 10, ptr %slot, align 1
  call void @helper_calls_noreturn(i8 1)
  unreachable
}

; Variation 2: Indirect call with noreturn attribute.
; CHECK-LABEL: test_indirect_noreturn:
; CHECK-NOT: push ix
; CHECK-NOT: dec sp
; CHECK: ld hl,L_test_indirect_noreturn.frame
define void @test_indirect_noreturn(ptr %fn) {
  %slot = alloca i8, align 1
  store volatile i8 99, ptr %slot, align 1
  call void %fn() noreturn
  unreachable
}

; -----------------------------------------------------------------------------
; Shape (c) Positive controls:
; Control 1: Genuinely recursive function must keep stack frame.
; -----------------------------------------------------------------------------
; CHECK-LABEL: genuine_recursion:
; CHECK-NOT: L_genuine_recursion.frame
define void @genuine_recursion(i8 %n) {
entry:
  %slot = alloca i8, align 1
  store volatile i8 %n, ptr %slot, align 1
  %c = icmp eq i8 %n, 0
  br i1 %c, label %done, label %recurse
recurse:
  %m = sub i8 %n, 1
  call void @genuine_recursion(i8 %m)
  br label %done
done:
  ret void
}

; Control 2: External call to a normal (non-noreturn, non-leaf) function.
; Because external code could call back into entry points while the caller
; activation is still live, this must conservatively keep its stack frame.
; -----------------------------------------------------------------------------
; CHECK-LABEL: test_external_normal:
; CHECK-NOT: L_test_external_normal.frame
define void @test_external_normal() {
  %slot = alloca i8, align 1
  store volatile i8 77, ptr %slot, align 1
  call void @external_normal()
  ret void
}

; -----------------------------------------------------------------------------
; Shape (d) Safety check: static frames are consolidated into L___static_frames.
; -----------------------------------------------------------------------------
; CHECK: L___static_frames:
; CHECK-DAG: L_test_indirect_noreturn.frame = L___static_frames
; CHECK-DAG: L_helper_calls_noreturn.frame = L___static_frames+{{[0-9]+}}
; CHECK-DAG: L_caller_of_helper.frame = L___static_frames+{{[0-9]+}}
; CHECK-DAG: L_test_external_noreturn.frame = L___static_frames+{{[0-9]+}}
