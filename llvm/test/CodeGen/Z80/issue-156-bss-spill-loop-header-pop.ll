; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; ravn/llvm-z80#156: the cross-MBB BSS-spill → PUSH/POP peephole
; (`STI.staticStack()` branch in Z80LateOptimization.cpp) rewrote a
; STORE in entry block to PUSH AND the matching LOAD in the loop
; header to POP, even though the loop header had a back-edge from
; inside the loop body.  Each loop iteration fell into the POP without
; a matching PUSH, leaking 2 bytes off SP per iteration; eventually
; SP wrapped through 0x0000 and the next RET popped garbage as the
; return address.  The full AES-256 decrypt under -Oz +static-stack
; escaped via this PC-corruption path; gf_log was the trigger.
;
; Fix: require MBB_B (the LOAD-bearing successor of MBB_A) to have
; MBB_A as its only predecessor, so the LOAD is reached exactly once
; per executed PUSH.
;
; This test reproduces the gf_log shape: entry stores a u8 parameter
; that survives across a loop; the loop header reloads the value;
; the loop has a back-edge.  Pre-fix codegen emits `push hl` in entry
; and `pop hl` in the loop header.  Post-fix the peephole bails and
; the BSS store/load pair stays.

; CHECK-LABEL: gf_log_repro:
; The peephole must NOT have rewritten the store/load pair into a
; bare push/pop, because the load-bearing block has a back-edge
; predecessor in addition to the entry.  Either the BSS store/load
; survives, or the spill is handled by some other mechanism — what
; we CANNOT see is a `push hl` in the entry block with the matching
; `pop hl` inside the loop body.
; CHECK-NOT: push	hl
; CHECK-NOT: push	bc
; CHECK-NOT: push	de

; This is the exact IR shape produced by clang -Oz from gf_log() in
; aes256.c (K&R-style declaration; i16 parameter).  The mix of i8/i16
; ops forces a spill across the back-edge.

define dso_local zeroext i8 @gf_log_repro(i16 noundef %0) {
  %2 = and i16 %0, 255
  br label %3

3:                                                ; preds = %8, %1
  %4 = phi i8 [ 0, %1 ], [ %17, %8 ]
  %5 = phi i16 [ 1, %1 ], [ %16, %8 ]
  %6 = and i16 %5, 255
  %7 = icmp eq i16 %6, %2
  br i1 %7, label %19, label %8

8:                                                ; preds = %3
  %9 = trunc i16 %5 to i8
  %10 = shl i8 %9, 1
  %11 = and i16 %5, 128
  %12 = icmp eq i16 %11, 0
  %13 = xor i8 %10, 27
  %14 = select i1 %12, i8 %10, i8 %13
  %15 = zext i8 %14 to i16
  %16 = xor i16 %6, %15
  %17 = add i8 %4, 1
  %18 = icmp eq i8 %17, 0
  br i1 %18, label %19, label %3

19:                                               ; preds = %3, %8
  %20 = phi i8 [ %4, %3 ], [ 0, %8 ]
  ret i8 %20
}
