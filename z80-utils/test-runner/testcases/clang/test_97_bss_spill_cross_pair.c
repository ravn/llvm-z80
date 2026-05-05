/* expect 0x0001 */
/* EXTRA-FLAGS: -Xclang -target-feature -Xclang +static-stack -Xclang -target-feature -Xclang +shadow-regs -mllvm -disable-lsr */
/*
 * BSS-spill peephole coverage #1: cross-register-pair reload.
 *
 * With +static-stack, locals are BSS-resident.  The BSS-spill peephole
 * (Z80LateOptimization, issues #74 + #82) converts a `LD (slot),pairA;
 * CALL ...; LD pairB,(slot)` sequence into `PUSH pairA; CALL ...; POP
 * pairB`.  When pairA != pairB the rewrite is the cross-pair extension
 * that was reverted in b843d94 (session 43) after it silently broke
 * autoload-in-c boot.
 *
 * This test forces 4 distinct i16 values to be live across a noinline
 * CALL, more than the 3 GP register pairs (HL/DE/BC) on Z80, so at
 * least one MUST spill.  Consuming them in a different order on the
 * reload side gives the allocator natural cause to pick a different
 * pair, exercising the cross-pair shape.  A value-corrupting peephole
 * here flips the final arithmetic and fails the expect check.
 */

typedef unsigned short uint16_t;

#define NOINLINE __attribute__((noinline))

NOINLINE
static uint16_t side_call(uint16_t x) {
    /* Forces caller-saved spills.  Multiplies by 3 in a way the inliner
     * can't fold even at -Oz. */
    uint16_t r = 0;
    for (uint16_t i = 0; i < 3; i++) r = (uint16_t)(r + x);
    return r;
}

NOINLINE
static uint16_t four_live_across_call(uint16_t seed) {
    volatile uint16_t a = (uint16_t)(seed + 1);   /* 101 */
    volatile uint16_t b = (uint16_t)(seed + 2);   /* 102 */
    volatile uint16_t c = (uint16_t)(seed + 3);   /* 103 */
    volatile uint16_t d = (uint16_t)(seed + 4);   /* 104 */

    /* Snapshot into non-volatile so the compiler can keep them in
     * registers across the call (and choose to spill). */
    uint16_t na = a, nb = b, nc = c, nd = d;

    uint16_t side = side_call(seed);              /* 300 */

    /* Reversed consumption order biases reload assignment. */
    uint16_t r = (uint16_t)(nd * 1u + nc * 2u + nb * 3u + na * 4u + side);
    /* 104 + 206 + 306 + 404 + 300 = 1320 */
    return r;
}

int main(void) {
    volatile uint16_t s = 100;
    return four_live_across_call(s) == 1320;
}
