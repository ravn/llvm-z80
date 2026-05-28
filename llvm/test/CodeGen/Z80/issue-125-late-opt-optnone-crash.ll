; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; Regression test for ravn/llvm-z80#125 (RESOLVED 2026-05-28): Z80LateOptimization
; used to crash on `optnone`-marked IR (the shape clang produces at -O0) when
; `+static-stack` is enabled — a function with two allocas, store-then-load
; through one slot into another, and a CALL of the second slot's loaded value.
; The crash was fixed incidentally by the frame-lowering / liveness hardening
; in the #210/#197 series; verified gone across llc -O0..-O3 and +shadow-regs,
; and the original broad trigger (test_99_bss_spill_lifo.c at clang -O0 with
; +static-stack +shadow-regs -disable-lsr) now compiles cleanly.  This test
; pins the no-crash behavior and the expected codegen (BSS frame + tail call).

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

declare zeroext i16 @callee(i16 noundef zeroext)

; CHECK-LABEL: bug125:
; CHECK:      ld de,10
; CHECK:      add hl,de
; CHECK:      jp _callee
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
