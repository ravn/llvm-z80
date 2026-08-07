// RUN: %clang_cc1 -triple z80 -fsyntax-only -verify %s

// The z88dk classic clib decorates functions with combinations such as
// __smallc __z88dk_callee.  z80_smallc / z80_callee / z80_fastcall / sdcccall /
// z80_allreg are DISTINCT calling conventions; applying two conflicting ones to
// one function must be diagnosed (like every other target's CC attributes), not
// silently collapsed to whichever is applied last.  ravn/llvm-z80#281.

void sc_callee(int a, int b)
    __attribute__((z80_smallc)) __attribute__((z80_callee)); // expected-error {{z80_callee and z80_smallc attributes are not compatible}}

void callee_fast(int a, int b)
    __attribute__((z80_callee)) __attribute__((z80_fastcall)); // expected-error {{are not compatible}}

void smallc_sdcc0(int a, int b)
    __attribute__((z80_smallc)) __attribute__((sdcccall(0))); // expected-error {{are not compatible}}

void allreg_callee(int a, int b)
    __attribute__((z80_allreg)) __attribute__((z80_callee)); // expected-error {{are not compatible}}

// A single CC attribute is fine.
void just_smallc(int a, int b) __attribute__((z80_smallc)); // ok
void just_callee(int a, int b) __attribute__((z80_callee)); // ok
void just_fast(int a) __attribute__((z80_fastcall));        // ok

// Repeating the SAME convention is not a conflict.
void same_twice(int a, int b)
    __attribute__((z80_callee)) __attribute__((z80_callee)); // ok
