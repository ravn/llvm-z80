// RUN: %clang_cc1 -triple z80 -O1 -emit-llvm -o - %s | FileCheck %s
//
// The upstream-standard way to pin an inline-asm operand to a Z80 16-bit
// register PAIR is a GCC local register variable (`register T x asm("de")`),
// NOT a braced "{de}" constraint in the C string: clang has no single-token
// pair constraint letter, and validateAsmConstraint rejects '{' for every
// target (so "+{de}" in C source is an "invalid constraint" error, and a bare
// "de" is parsed as the two 8-bit constraints 'd'+'e'). The register NAMES
// (a/bc/de/hl/...) live in Z80's getGCCRegNames(), so asm("de") is accepted and
// lowers to the physical pair. The rcbios/cpnos LDIR/LDDR/`ld i,a` helpers use
// this form. Companion backend test: CodeGen/Z80/inline-asm-reg-pair.ll.

#include <stddef.h>

// CHECK-LABEL: define {{.*}}@mem_copy_backwards
// The three pair operands must appear as the IR-level physical-register
// constraints {de},{hl},{bc} (both as outputs and read-write inputs).
// CHECK: call { ptr, ptr, i16 } asm sideeffect "lddr", "={de},={hl},={bc},{de},{hl},{bc},~{memory}"
void mem_copy_backwards(void *de_end, const void *hl_end, size_t n) {
  register void *de asm("de") = de_end;
  register const void *hl asm("hl") = hl_end;
  register size_t bc asm("bc") = n;
  __asm__ volatile("lddr" : "+r"(de), "+r"(hl), "+r"(bc) :: "memory");
}

// CHECK-LABEL: define {{.*}}@intrinsic_ld_i_a
// CHECK: call void asm sideeffect "ld i, a", "{a}"(i8
void intrinsic_ld_i_a(unsigned char page) {
  register unsigned char a asm("a") = page;
  __asm__ volatile("ld i, a" :: "r"(a));
}
