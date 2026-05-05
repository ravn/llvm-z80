/* expect 0x0001 */
/* EXTRA-FLAGS: -Xclang -target-feature -Xclang +static-stack -Xclang -target-feature -Xclang +shadow-regs -mllvm -disable-lsr */
/* SKIP-IF: O0 */
/* O0 crashes Z80LateOptimization::runOnMachineFunction; passes O1..Oz.
 * The crash is independent of this test's intent (BSS-spill peephole
 * value-oracle coverage); skipping O0 keeps the test live where the
 * peephole actually fires.  TODO: file the O0 crash with this file as
 * the repro. */
/*
 * BSS-spill peephole coverage #3: multiple spill/reload pairs in the
 * same basic block (LIFO collect-and-reverse-apply territory).
 *
 * The post-#74 BSS-spill peephole rewrites the block in two passes:
 * first it collects every store/CALL/load triple into a worklist, then
 * it applies them in reverse order so each PUSH/POP rewrite doesn't
 * invalidate the iterators of later candidates.  The conservative-fix
 * attempt in session 43 reverted the cross-pair feature but kept this
 * LIFO refactor — and it STILL hung autoload-in-c, implicating the
 * refactor itself or its interaction with #82's orphan-load handling.
 *
 * This test crams several CALL boundaries and multiple long-lived i16
 * values into one function body so the worklist collects multiple
 * candidates per block.  Each value's correctness is checked, so a
 * mis-ordered LIFO apply (e.g. one peephole stomping on a slot another
 * needs) corrupts the result and fails the expect check.
 *
 * Plain addition keeps the expected value trivially computable.
 */

typedef unsigned short uint16_t;

#define NOINLINE __attribute__((noinline))

NOINLINE
static uint16_t add1(uint16_t x) { return (uint16_t)(x + 1); }
NOINLINE
static uint16_t add2(uint16_t x) { return (uint16_t)(x + 2); }
NOINLINE
static uint16_t add3(uint16_t x) { return (uint16_t)(x + 3); }
NOINLINE
static uint16_t add4(uint16_t x) { return (uint16_t)(x + 4); }

NOINLINE
static uint16_t multi_call_block(uint16_t seed) {
    /* Six values must coexist across four CALLs in the same block.
     * Z80 has 3 GP pairs, so at least 3 of these MUST spill.  With
     * +static-stack the spill targets are BSS, so the peephole's
     * worklist will hold multiple (store, CALL, load) candidates at
     * once. */
    uint16_t v1 = (uint16_t)(seed + 10);
    uint16_t v2 = (uint16_t)(seed + 20);
    uint16_t v3 = (uint16_t)(seed + 30);
    uint16_t v4 = (uint16_t)(seed + 40);
    uint16_t v5 = (uint16_t)(seed + 50);
    uint16_t v6 = (uint16_t)(seed + 60);

    /* Four CALLs interleaved with reads of different values, forcing
     * spills on different slots between each pair of CALLs. */
    uint16_t r1 = add1(v1);                          /* needs v1 */
    uint16_t t1 = (uint16_t)(r1 + v2);               /* needs v2 reloaded */

    uint16_t r2 = add2(v3);                          /* needs v3 */
    uint16_t t2 = (uint16_t)(r2 + v4);               /* needs v4 reloaded */

    uint16_t r3 = add3(v5);                          /* needs v5 */
    uint16_t t3 = (uint16_t)(r3 + v6);               /* needs v6 reloaded */

    uint16_t r4 = add4((uint16_t)(t1 + t2 + t3));

    /* Final read of every original to ensure each survived the block. */
    uint16_t survival = (uint16_t)(v1 + v2 + v3 + v4 + v5 + v6);

    return (uint16_t)(r4 + survival);
}

int main(void) {
    volatile uint16_t s = 0;
    /* With s=0:
     *   v1..v6 = 10, 20, 30, 40, 50, 60   (sum = 210)
     *   r1 = 11, t1 = 11 + 20 = 31
     *   r2 = 32, t2 = 32 + 40 = 72
     *   r3 = 53, t3 = 53 + 60 = 113
     *   r4 = add4(31 + 72 + 113) = add4(216) = 220
     *   survival = 210
     *   final = 220 + 210 = 430 = 0x01AE
     */
    return multi_call_block(s) == 430u;
}
