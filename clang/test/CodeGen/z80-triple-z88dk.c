// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -O1 -S -o - %s | FileCheck %s --check-prefix=Z88DK
// RUN: %clang_cc1 -triple z80 -O1 -S -o - %s | FileCheck %s --check-prefix=DEFAULT
// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -mdouble=64 -O1 -S -o - %s | FileCheck %s --check-prefix=OVERRIDE-64

// 1. Target triple defaults to native z80asm format (GLOBAL, SECTION code_compiler, no leading dots)
void external_call(void);
void test_func(char c) {
  if (c)
    external_call();
}
// Z88DK:        SECTION code_compiler
// Z88DK-NEXT:   GLOBAL _test_func
// Z88DK-LABEL:  _test_func:
// Z88DK:        call _external_call
// Z88DK:        ret
//
// DEFAULT:      .globl _test_func
// DEFAULT-LABEL: _test_func:
// DEFAULT:      call _external_call
// DEFAULT:      ret

// 2. Target triple uses z88dk's l_f32_* float runtime (default C = __smallc)
float fadd(float a, float b) { return a + b; }
// Z88DK-LABEL:   _fadd:
// Z88DK:         push hl
// Z88DK:         call l_f32_add
//
// DEFAULT-LABEL: _fadd:
// DEFAULT:       call ___addsf3

int flt(float a, float b) { return a < b; }
// Z88DK-LABEL:   _flt:
// Z88DK:         push hl
// Z88DK:         call l_f32_lt
//
// DEFAULT-LABEL: _flt:
// DEFAULT:       call ___ltsf2

int to_int(float a) { return (int)a; }
// Z88DK-LABEL:   _to_int:
// Z88DK:         push hl
// Z88DK:         call l_f32_f2slong
//
// DEFAULT-LABEL: _to_int:
// DEFAULT-NOT:   push hl
// DEFAULT:       call ___fixsfsi

float to_float(int a) { return (float)a; }
// Z88DK-LABEL:   _to_float:
// Z88DK:         call l_f32_slong2f
//
// DEFAULT-LABEL: _to_float:
// DEFAULT-NOT:   push hl
// DEFAULT:       call ___floatsisf

// 3. Target triple defaults to 32-bit double and long double (matching z88dk math32)
double dadd(double a, double b) { return a + b; }
// Z88DK-LABEL:   _dadd:
// Z88DK:         push hl
// Z88DK:         call l_f32_add
//
// DEFAULT-LABEL: _dadd:
// DEFAULT:       call ___addsf3

int dsize(void) { return (int)sizeof(double); }
// Z88DK-LABEL:   _dsize:
// Z88DK:         ld hl,{{ ?}}4
// Z88DK:         ret
//
// DEFAULT-LABEL: _dsize:
// DEFAULT:       ld de,{{ ?}}4
// DEFAULT:       ret
//
// OVERRIDE-64-LABEL: _dsize:
// OVERRIDE-64:        ld hl,{{ ?}}8

int ldsize(void) { return (int)sizeof(long double); }
// Z88DK-LABEL:   _ldsize:
// Z88DK:         ld hl,{{ ?}}4
// Z88DK:         ret
//
// DEFAULT-LABEL: _ldsize:
// DEFAULT:       ld de,{{ ?}}4
// DEFAULT:       ret
