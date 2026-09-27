// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -O1 -S -o - %s | FileCheck %s --check-prefix=Z88DK
// RUN: %clang_cc1 -triple z80 -O1 -S -o - %s | FileCheck %s --check-prefix=DEFAULT
// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -E -dM /dev/null | FileCheck %s --check-prefix=MACROS
// RUN: %clang_cc1 -triple z80 -E -dM /dev/null | FileCheck %s --check-prefix=DEFAULT-MACROS

// MACROS-DAG: #define __LLVMZ80 1
// MACROS-DAG: #define __LLVMZ80__ 1
// MACROS-DAG: #define __Z88DK 1
// MACROS-DAG: #define __Z88DK__ 1
// MACROS-DAG: #define __z88dk 1
// MACROS-DAG: #define __z88dk__ 1

// DEFAULT-MACROS-NOT: __Z88DK
// DEFAULT-MACROS-NOT: __z88dk

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

// 2. Target triple defaults to sdcccall(0) for f32 operations (all args pushed on stack)
float fadd(float a, float b) { return a + b; }
// Z88DK-LABEL:   _fadd:
// Z88DK:         push hl
// Z88DK:         call ___addsf3
//
// DEFAULT-LABEL: _fadd:
// DEFAULT:       call ___addsf3

int flt(float a, float b) { return a < b; }
// Z88DK-LABEL:   _flt:
// Z88DK:         push hl
// Z88DK:         call ___cmpsf2
//
// DEFAULT-LABEL: _flt:
// DEFAULT:       call ___cmpsf2

int to_int(float a) { return (int)a; }
// Z88DK-LABEL:   _to_int:
// Z88DK:         push hl
// Z88DK:         call ___fixsfsi
//
// DEFAULT-LABEL: _to_int:
// DEFAULT-NOT:   push hl
// DEFAULT:       call ___fixsfsi

float to_float(int a) { return (float)a; }
// Z88DK-LABEL:   _to_float:
// Z88DK:         push hl
// Z88DK:         call ___floatsisf
//
// DEFAULT-LABEL: _to_float:
// DEFAULT-NOT:   push hl
// DEFAULT:       call ___floatsisf
