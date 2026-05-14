// RUN: %clang_cc1 -triple z80 -emit-llvm -O0 -o - %s | FileCheck %s

// Regression test for ravn/llvm-z80#162.
//
// K&R-style typed-parameter declarations of narrow unsigned types are
// int-promoted at the ABI boundary.  On Z80 (int = 16 bits) a `uint8_t`
// K&R parameter becomes i16 in IR.  Per the C standard, the caller's
// default argument promotion zero-extends the u8 value into the i16 it
// passes — so the i16 parameter has provably zero high bits.
//
// Before fix: clang emits `define ... @f(i16 noundef %0)` with no zeroext.
// After fix:  clang emits `define ... @f(i16 noundef zeroext %0)`.
//
// Mid-end impact: with zeroext, TruncInstCombine can narrow chains
// rooted at the parameter (since `(zext (trunc %p))` proves to a no-op),
// which unblocks `fshl.i8` idiom recognition on K&R u8 rotate chains.

typedef unsigned char u8;
typedef signed char i8_t;

// K&R-style typed-parameter declaration with narrow unsigned type.
// CHECK-LABEL: @kr_u8(
// CHECK-SAME: i16 noundef zeroext %{{[0-9a-zA-Z_]+}}
u8 kr_u8(x) u8 x; { return x; }

// K&R-style typed-parameter declaration with narrow signed type.
// CHECK-LABEL: @kr_i8(
// CHECK-SAME: i16 noundef signext %{{[0-9a-zA-Z_]+}}
i8_t kr_i8(x) i8_t x; { return x; }

// ANSI prototype with narrow unsigned type — keeps existing zeroext.
// CHECK-LABEL: @ansi_u8(
// CHECK-SAME: i8 noundef zeroext %{{[0-9a-zA-Z_]+}}
u8 ansi_u8(u8 x) { return x; }

// K&R-style with `int` (already promoted width) — must NOT get zeroext.
// CHECK-LABEL: @kr_int(
// CHECK-SAME: i16 noundef %{{[0-9a-zA-Z_]+}}
// CHECK-NOT: zeroext
int kr_int(x) int x; { return x; }
