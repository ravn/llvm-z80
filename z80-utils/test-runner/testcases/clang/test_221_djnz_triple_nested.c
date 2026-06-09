/* expect 0x0001 */
/*
 * ravn/llvm-z80#221: triple-nested countdown loop, runtime verification.
 *
 * This is the production-shape `delay()` family from
 * rc700-gensmedet/autoload-in-c/rom.c lines 102-115.  Three nested countdown
 * loops with an inline-asm body in the innermost so the loops survive Oz
 * dead-code elimination.  The peephole DEC A; LD B,A; [OR A;] JR NZ -> DJNZ
 * (Z80LateOptimization.cpp) must fire on the innermost loop so the innermost
 * is lowered to a single `djnz` (2 B) instead of the 6 B round-trip
 * `ld a,b; dec a; ld b,a; or a; jr nz`.
 *
 * Pre-#221-fix: under `-g`, the peephole's std::next(I) landed on DBG_VALUE
 * pseudos and the rewrite bailed, costing 4 B per innermost loop.
 *
 * Runtime check: total inner-body iterations = outer * mid * 256.  We use a
 * volatile counter inside the innermost so the optimizer cannot eliminate it.
 * The exit value is the actual iter count, which must equal
 * OUTER * MID * 256 = 3 * 2 * 256 = 1536 (= 0x600).
 */
typedef unsigned char  u8;
typedef unsigned short u16;

#define OUTER 3
#define MID   2

static volatile u16 iter_count;

static void triple_nest(u8 outer, u8 mid_init) {
    if (!outer) return;
    do {
        u8 mid = mid_init;
        do {
            u8 k = 0;
            do {
                __asm__ volatile("");
                iter_count++;
            } while (--k);
        } while (--mid);
    } while (--outer);
}

int main(void) {
    iter_count = 0;
    triple_nest(OUTER, MID);
    /* Expected: OUTER * MID * 256 = 1536.  If the counter doesn't match,
     * either the innermost loop iterates the wrong number of times, the
     * middle counter is off, or some peephole rewrote semantics
     * incorrectly. */
    return (iter_count == (u16)(OUTER * MID * 256)) ? 1 : 0;
}
