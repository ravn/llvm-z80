; RUN: llc -mtriple=z80 -O2 -disable-lsr -z80-enable-cse < %s | FileCheck %s
;
; MITIGATED 2026-06-30 by ravn/llvm-z80#248 (i32 divrem fusion) -- XFAIL
; removed, now a regression guard.  #248 lowers the adjacent i32 udiv/urem
; pair to one __udivmodsi4 whose remainder is loaded from a stack slot, so
; the remainder no longer arrives in $de from a trailing __umodsi3 call.
; That removes the "two consecutive DE stores sourced from constant loads"
; MIR shape that Branch Folder unsoundly hoisted, so this pi-spigot witness
; no longer miscompiles with CSE on.  The underlying Branch Folder
; unsoundness (known-suboptimal B15) is NOT root-fixed -- only this
; witness's trigger shape is broken -- so the test now guards that the
; trigger does not reappear.
;
; Branch Folder unsound cross-block hoist exposed by MachineCSE -- pi spigot
; miscompile.  Original analysis PARKED 2026-06-09 (production unaffected;
; CSE off by default).
;
; Root cause (full writeup in
; tasks/session-2026-06-09-pi-cse-miscompile-investigation.md):
;
; With -z80-enable-cse, MachineCSE collapses three constant-load PHI sources
; in the outer-loop prelude, eliminating a forward-only intermediate block.
; Branch Folder ("Control Flow Optimizer", llvm/lib/CodeGen/BranchFolding.cpp)
; then sees bb.0 ending with two consecutive `LD_nnind_DE` stores (both
; sourced from `LD_r16_nn 0`) and unsoundly moves one into the head of bb.1
; (the outer-loop header).  bb.1 has TWO predecessors: bb.0 (entry, $de = 0
; via the immediately-prior init) AND bb.4 (outer back-edge, $de = `m` from
; `__umodsi3`'s return).  The hoisted store fires on the back-edge path with
; stale $de = m, overwriting the freshly-computed new checksum that bb.4 had
; just stored to the same BSS slot.  Per-block trace: each outer iteration's
; checksum becomes only that block's contribution (3141, 5926, 5358, ...)
; instead of the accumulated total (3141, 9067, 14425, ..., 28116).
;
; CHECK below asserts the FIXED behavior: the outer-loop header (".LBB0_1:")
; must not begin with a store of $de to memory before $de has been initialized
; on the current iteration.  Specifically, the first non-label instruction
; in the loop header must NOT be `ld (__sfrend_bench_run-NN), de`, because
; on the back-edge entry $de holds `m` (the new c), not the new checksum.
;
; Before #248 (pre 2026-06-30) this CHECK FAILED because Branch Folder
; hoisted the store into that slot (the predicted "Z80-specific mitigation
; breaks the trigger MIR shape" cleanup signal has now fired -- see the
; MITIGATED note at the top).
;
; Confirmed via `-mllvm -disable-branch-fold` (with CSE on) -> pi PASS at
; 884 B / 58.87M ts (vs default CSE-on -> pi FAIL at 880 / 58.87M).

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

@r = global [281 x i16] zeroinitializer

declare fastcc void @pi_init()

; Function Attrs: minsize
define i16 @bench_run() #0 {
  tail call fastcc void @pi_init()
  br label %1

1:                                                ; preds = %25, %0
  %2 = phi i16 [ 280, %0 ], [ %32, %25 ]
  %3 = phi i16 [ 0, %0 ], [ %31, %25 ]
  %4 = phi i16 [ 0, %0 ], [ %29, %25 ]
  %5 = icmp eq i16 %2, 0
  br i1 %5, label %33, label %6

6:                                                ; preds = %22, %1
  %7 = phi i16 [ %20, %22 ], [ 280, %1 ]
  %8 = phi i32 [ %24, %22 ], [ 0, %1 ]
  %9 = getelementptr [2 x i8], ptr @r, i16 %7
  %10 = load i16, ptr %9, align 1
  %11 = shl i16 %7, 1
  %12 = zext i16 %10 to i32
  %13 = mul i32 %12, 10000
  %14 = add i32 %13, %8
  %15 = add i16 %11, -1
  %16 = zext i16 %15 to i32
  %17 = urem i32 %14, %16
  %18 = trunc i32 %17 to i16
  store i16 %18, ptr %9, align 1
  %19 = udiv i32 %14, %16
  %20 = add i16 %7, -1
  %21 = icmp eq i16 %20, 0
  br i1 %21, label %25, label %22

22:                                               ; preds = %6
  %23 = zext i16 %20 to i32
  %24 = mul i32 %19, %23
  br label %6

25:                                               ; preds = %6
  %26 = udiv i32 %19, 10000
  %27 = trunc i32 %26 to i16
  %28 = add i16 %4, %3
  %29 = add i16 %28, %27
  %30 = urem i32 %19, 10000
  %31 = trunc i32 %30 to i16
  %32 = add i16 %2, -14
  br label %1

33:                                               ; preds = %1
  ret i16 %4
}

attributes #0 = { minsize }

; The outer-loop header must not begin with a store of DE to a BSS slot,
; because on the back-edge entry DE holds `m` (the new c), not the new
; checksum.  The fixed codegen tests DE/HL first or re-initialises DE
; before any store -- it does NOT start the loop body with an ld (mem),de.
;
; CHECK-LABEL: bench_run:
; CHECK: .LBB0_1:
; CHECK-NOT: ld {{[^,]*__sfrend_bench_run[^,]*}},de
; CHECK: ld a,l
