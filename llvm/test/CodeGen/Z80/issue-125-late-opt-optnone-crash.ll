; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; XFAIL: *
;
; Issue ravn/llvm-z80#125: Z80LateOptimization crashes on `optnone`-marked
; IR (the shape clang produces at -O0) when `+static-stack` is enabled.
; Originally observed when adding the test-runner BSS-spill peephole
; coverage in commit 0f3cd42101f9 — `test_99_bss_spill_lifo.c` triggers
; the crash at -O0 with `+static-stack +shadow-regs -disable-lsr`, but
; minimisation showed the trigger is `+static-stack` plus the optnone IR
; shape: a function with two allocas, store-then-load through one slot
; into another, and a CALL of the second slot's loaded value.  Crash
; reproduces at every llc opt level (default O2 included); shadow-regs
; and -disable-lsr are not required.
;
; Pass at the time of crash: "Z80 Late Optimizations" on @bug125.
;
; The c-test passes at O1..Oz because clang transforms the IR upstream
; of llc (mem2reg + alloca elimination) — the optnone IR shape only
; reaches llc at clang -O0.
;
; Closing this fully will let `test_99_bss_spill_lifo.c` drop its
; `SKIP-IF: O0` guard.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

declare zeroext i16 @callee(i16 noundef zeroext)

; CHECK-LABEL: bug125:
; CHECK: ret
define internal zeroext i16 @bug125(i16 noundef zeroext %0) #0 {
  %2 = alloca i16, align 1
  %3 = alloca i16, align 1
  store i16 %0, ptr %2, align 1
  %4 = load i16, ptr %2, align 1
  %5 = add i16 %4, 10
  store i16 %5, ptr %3, align 1
  %6 = load i16, ptr %3, align 1
  %7 = call zeroext i16 @callee(i16 noundef zeroext %6)
  ret i16 %7
}

attributes #0 = { noinline nounwind optnone }
