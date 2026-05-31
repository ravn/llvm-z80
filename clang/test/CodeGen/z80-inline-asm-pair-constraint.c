// RUN: %clang_cc1 -triple z80 -emit-llvm -o - %s | FileCheck %s --check-prefix=IR
// RUN: %clang_cc1 -triple z80 -S -o - %s | FileCheck %s --check-prefix=ASM
//
// ravn/llvm-z80: bare two-letter register-pair inline-asm constraints
// (hl, bc, de, af, ix, iy, sp) used to fatally crash the backend.
// validateAsmConstraint accepts bare "hl", but LLVM's IR-level InlineAsm
// parser splits a multi-letter constraint into single-register *alternatives*
// (so "hl" became "register h OR register l", 8-bit), which cannot hold a
// 16-bit operand -> IRTranslator aborted ("unable to translate instruction:
// call").  Z80TargetInfo::convertConstraint now rewrites a bare pair name to
// the braced specific-register form ("hl" -> "{hl}"), which lowers as a single
// 16-bit register and works.  Braced and single-letter constraints are
// unchanged.

// The emitted IR constraint string must be the braced "{hl}" / "{de}" form,
// NOT the bare "hl" that would be re-split into alternatives.
// IR-LABEL: define{{.*}} i16 @add_hl_de(
// IR: call i16 asm "add hl,de", "={hl},{hl},{de}"
unsigned add_hl_de(unsigned a, unsigned b) {
  unsigned r;
  __asm__("add hl,de" : "=hl"(r) : "hl"(a), "de"(b));
  return r;
}

// Bare "bc" input lowers to "{bc}".
// IR-LABEL: define{{.*}} void @use_bc(
// IR: call void asm sideeffect "inc bc", "{bc}"
void use_bc(unsigned a) { __asm__ volatile("inc bc" ::"bc"(a)); }

// A braced constraint is passed through verbatim (not double-wrapped).
// IR-LABEL: define{{.*}} void @braced_hl(
// IR: call void asm sideeffect "inc hl", "{hl}"
// IR-NOT: {{[{][{]}}
void braced_hl(unsigned a) { __asm__ volatile("inc hl" ::"{hl}"(a)); }

// A single-letter register constraint is unaffected (stays bare "a").
// IR-LABEL: define{{.*}} void @single_a(
// IR: call void asm sideeffect "inc a", "a"
void single_a(unsigned char a) { __asm__ volatile("inc a" ::"a"(a)); }

// End-to-end: the pair operands land in HL/DE and the asm is emitted with no
// backend crash.
// ASM-LABEL: _add_hl_de:
// ASM: add hl,de
