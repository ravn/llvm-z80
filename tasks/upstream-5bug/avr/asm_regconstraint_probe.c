/* PR #40 fallout probe: inline-asm register binding on Z80.
 *
 * Findings (clang --target=z80, llvmz80-23.1.0-r1 / PR #40), verified 2026-09-10:
 *
 *   FORM                              RESULT
 *   "a" / "d" (single letter)         OK   -- 8-bit register class constraint
 *   "r"                               OK   -- any 8/16-bit register
 *   "{de}" / "{a}" (braced)           ERROR "invalid ... constraint"  <-- firmware uses this
 *   "de" (bare two-letter)            BACKEND CRASH (IRTranslator "unable to translate call")
 *   register T x __asm__("de"); "+r"(x)   OK -> emits `lddr` / `ldir`      <-- upstream-standard
 *
 * Root cause: clang's C-level constraint model has NO rule that "two letters =
 * 16-bit pair". Each letter is a separate single-letter (8-bit) constraint, so
 * "de" parses as constraints 'd' AND 'e' -> nonsense -> backend crash. There is
 * no single-token constraint letter for a 16-bit pair. The braced "{de}" is the
 * LLVM-IR register-name notation; clang's validateAsmConstraint never accepts
 * '{' for any target, so "{de}" at C level is rejected. The fork used to carry a
 * convertConstraint extension making "{de}"/bare-"de" work; PR #40 (taking
 * upstream's clang) dropped it.
 *
 * The upstream-blessed way to bind an operand to a specific Z80 pair is a GCC
 * local register variable + a plain "+r"/"r" constraint (bottom of file). The
 * register NAMES (a/bc/de/hl/af/ix/iy) are in getGCCRegNames(), so __asm__("de") is
 * validated and binds the value to the physical pair.
 */
#include <stddef.h>

/* ---- BROKEN today: the form the firmware currently uses (compat.h, string.h) */
#if 0
static inline void mem_copy_backwards_BROKEN(void *de, const void *hl, size_t n) {
    __asm__ volatile("lddr" : "+{de}"(de), "+{hl}"(hl), "+{bc}"(n) :: "memory");
}
static inline void ld_i_a_BROKEN(unsigned char page) {
    __asm__ volatile("ld i, a" :: "{a}"(page));
}
#endif

/* ---- WORKS on upstream-current clang: GCC local register variables --------- */
void mem_copy_backwards(void *de_end, const void *hl_end, size_t n) {
    register void *de       __asm__("de") = de_end;
    register const void *hl __asm__("hl") = hl_end;
    register size_t bc      __asm__("bc") = n;
    __asm__ volatile("lddr" : "+r"(de), "+r"(hl), "+r"(bc) :: "memory");
}
void intrinsic_ld_i_a(unsigned char page) {
    register unsigned char a __asm__("a") = page;
    __asm__ volatile("ld i, a" :: "r"(a));
}
