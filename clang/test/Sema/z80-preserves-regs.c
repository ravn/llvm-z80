// RUN: %clang_cc1 -triple z80 -fsyntax-only -verify %s
//
// ravn/llvm-z80#131 — Sema validation of the z80_preserves_regs attribute.

// Valid usages (case-insensitive, single letter or pair).
extern void ok1(int) __attribute__((z80_preserves_regs("d", "e")));
extern void ok2(int) __attribute__((z80_preserves_regs("DE")));
extern void ok3(int) __attribute__((z80_preserves_regs("a", "bc", "hl", "ix", "iy")));
extern void ok4(void) __attribute__((z80_preserves_regs()));

// Unknown register name is rejected.
extern void bad1(int) __attribute__((z80_preserves_regs("r0"))); // expected-error{{does not recognise r0 as a Z80 register name}}
extern void bad2(int) __attribute__((z80_preserves_regs("d", "xyz"))); // expected-error{{does not recognise xyz as a Z80 register name}}
extern void bad3(int) __attribute__((z80_preserves_regs(""))); // expected-error{{does not recognise  as a Z80 register name}}

// Attribute is function-only — subject diagnostics enforced by SubjectList<[Function], ErrorDiag>.
int bad_global __attribute__((z80_preserves_regs("d"))); // expected-error{{'z80_preserves_regs' attribute only applies to functions}}
