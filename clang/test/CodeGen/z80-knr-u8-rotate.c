// RUN: %clang_cc1 -triple z80 -Oz -S -Wno-deprecated-non-prototype -o - %s | FileCheck %s
//
// ravn/llvm-z80#158: a K&R-style u8 parameter is int-promoted to i16 at the C
// ABI boundary (int = 16 bits on Z80).  Before the TruncInstCombine
// "narrow through Argument" fix, the rotate expression stayed i16 and emitted a
// 16-bit shift/mask/or dance (`add hl, hl` + masks).  With the fix it narrows to
// native 8-bit ops and the rotate idiom is recognized, leaving a single `rlca`.
//
// This is the end-to-end (full clang pipeline) guarantee for the symptom the
// issue filed; the precise mid-end assertion lives in the target-agnostic
// llvm/test/Transforms/AggressiveInstCombine/narrow-through-argument.ll.

typedef unsigned char uint8_t;

// CHECK-LABEL: _rotl_u8:
// CHECK-NOT:   add hl
// CHECK:       rlca
// CHECK:       ret
uint8_t rotl_u8(x) uint8_t x;
{
    return (x << 1) | (x >> 7);
}
