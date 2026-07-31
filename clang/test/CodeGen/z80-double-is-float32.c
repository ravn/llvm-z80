// RUN: %clang_cc1 -triple z80  -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple sm83 -emit-llvm -o - %s | FileCheck %s
//
// ravn/llvm-z80: on this 8-bit target, `double` and `long double` are 32-bit
// IEEE-754 binary32 -- the same width and bit format as `float` -- rather
// than the usual 64-bit binary64. This is deliberate, not a bug: it is the
// only float format with a reusable host-side runtime (z88dk's `math32`,
// itself IEEE-754 binary32), whereas a real 64-bit binary64 would need an
// entirely new from-scratch soft-float library with nothing to bridge to.
// Baseline (before this change, base TargetInfo default of double=64/
// IEEEdouble): `double add(double,double)` emitted `call ___adddf3` and
// `sizeof(double) == 8`.
//
// This test pins the frontend/IR-level consequence: arithmetic on `double`
// lowers to the *same* 32-bit ("sf") compiler-rt libcalls as `float`, not the
// 64-bit ("df") ones, and `sizeof(double) == sizeof(float) == 4`.

// CHECK: define{{.*}} float @add(float noundef %a, float noundef %b)
// CHECK: fadd float
double add(double a, double b) {
  return a + b;
}

// CHECK: define{{.*}} i16 @dsize()
// CHECK: ret i16 4
int dsize(void) {
  return (int)sizeof(double);
}

// CHECK: define{{.*}} i16 @ldsize()
// CHECK: ret i16 4
int ldsize(void) {
  return (int)sizeof(long double);
}
