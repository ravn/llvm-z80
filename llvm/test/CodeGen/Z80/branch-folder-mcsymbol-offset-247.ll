; RUN: llc -mtriple=z80 -O1 < %s | FileCheck %s
;
; ravn/llvm-z80#247: clang -O1/-O2 miscompiled fannkuch (wrong result), while
; -O0/-Oz were correct.  Root cause is generic, not Z80-specific codegen:
;
; The Z80 static-frame lowering encodes a BSS slot address as an MO_MCSymbol
; operand carrying a *nonzero offset* set via MachineOperand::setOffset()
; (Z80InstrInfo.cpp: "LD (__sfrend_f - N),reg").  But generic
; MachineOperand::isIdenticalTo()/getHashValue() for MO_MCSymbol compared ONLY
; the symbol and ignored the offset (unlike MO_GlobalAddress/MO_ExternalSymbol,
; which compare it).  So the Control Flow Optimizer (branch-folder) tail-merged
; two stores that look identical but target different slots --
;   pred A:  ld (__sfrend_bench_run-4),hl   ; %6 phi source on the %4 edge
;   pred B:  ld (__sfrend_bench_run-2),hl   ; %8 phi source on the %1 edge
; -- collapsing them into ONE store in the common successor, dropping the
; other slot's value -> wrong result.  Only -O1+ triggers it because
; branch-folder + block-placement create the mergeable common tail; -O0/-Oz
; leave the two predecessors distinct.
;
; Fix: MO_MCSymbol isIdenticalTo/hash now also compare getOffset(), so the two
; stores are no longer considered identical and are NOT merged.  Both
; distinct-offset value-1 stores must survive, one per predecessor.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

define i16 @bench_run() {
  br label %1

1:
  %2 = phi i16 [ 0, %0 ], [ %6, %5 ]
  %3 = icmp eq i16 %2, 1
  br i1 %3, label %5, label %4

4:
  br label %5

5:
  %6 = phi i16 [ 0, %1 ], [ 1, %4 ]
  %7 = phi i32 [ 1, %1 ], [ 0, %4 ]
  %8 = phi i16 [ 1, %1 ], [ 0, %4 ]
  switch i32 %7, label %10 [
    i32 0, label %1
    i32 18, label %9
  ]

9:
  ret i16 0

10:
  ret i16 %8
}

; The two value-1 phi-source stores must go to DIFFERENT static-frame slots
; and must both be present (pre-fix, branch-folder merged them and only the
; -2 store survived).
; CHECK-LABEL: bench_run:
; CHECK-DAG: ld (__sfrend_bench_run-2),hl
; CHECK-DAG: ld (__sfrend_bench_run-4),hl
