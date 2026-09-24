; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs < %s -o /dev/null
;
; ravn/llvm-z80#363: Z80FuseCarryChain emitted AND A (carry-clear head of SBC
; chain) without marking the A read as undef.  -verify-machineinstrs demanded a
; prior def of A; in sub32/sub64/sub128 A is never materialised, so the
; verifier aborted with "Using an undefined physical register".
;
; Fix: markUndefUse on the AND A operand, matching the same carry-clear idiom
; already used in Z80InstrInfo.cpp for SUB_HL_rr / SADD_HL_rr expansion.
; No emitted bytes change -- purely verifier/liveness metadata.
;
; C source:
;   // clang --target=z80 -O2 -mllvm -verify-machineinstrs -c repro.c
;   typedef unsigned long      uint32_t;
;   typedef unsigned long long uint64_t;
;   uint32_t sub32(uint32_t a, uint32_t b) { return a - b; }
;   uint64_t sub64(uint64_t a, uint64_t b) { return a - b; }

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-ve-n8:16"
target triple = "z80"

define dso_local noundef i32 @sub32(i32 noundef %0, i32 noundef %1) {
  %3 = sub i32 %0, %1
  ret i32 %3
}

define dso_local noundef i64 @sub64(i64 noundef %0, i64 noundef %1) {
  %3 = sub i64 %0, %1
  ret i64 %3
}

define dso_local noundef i128 @sub128(i128 noundef %0, i128 noundef %1) {
  %3 = sub i128 %0, %1
  ret i128 %3
}
