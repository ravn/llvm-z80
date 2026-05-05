/* expect 0x0001 */
/* EXTRA-FLAGS: -Xclang -target-feature -Xclang +static-stack -Xclang -target-feature -Xclang +shadow-regs -mllvm -disable-lsr */
/*
 * BSS-spill peephole coverage #2: pointer-pressure cross-pair reload.
 *
 * Three buffer pointers + an index live across nested noinline CALLs.
 * Pointers are 16-bit so they compete directly for HL/DE/BC.  After
 * each CALL the pointers must be reloaded; with +static-stack the
 * spill/reload happens against BSS slots and is the BSS-spill peephole's
 * fire site.  Because three pointers can't simultaneously occupy three
 * registers AND be consumed by a CALL that needs HL for its return value,
 * the regalloc shuffles them between pairs across the CALL — which is
 * exactly the cross-pair shape the broken #74 extension would rewrite.
 *
 * Mimics the rcbios <-> autoload-in-c structural shape (multiple buffer
 * pointers + counters surviving multi-stage I/O calls) without the IRQ /
 * DMA / FDC dependencies.  A value miscompile shows up as wrong sums.
 *
 * Arithmetic is plain addition so the expected value is straightforward:
 *   sum of buf_a[0..3] + buf_b[0..3] + buf_c[0..3] = 10 + 100 + 394 = 504.
 */

typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define NOINLINE __attribute__((noinline))

NOINLINE
static uint16_t fetch_byte(const uint8_t *p, uint16_t i) {
    return (uint16_t)p[i];
}

NOINLINE
static uint16_t accumulate(uint16_t acc, uint16_t v) {
    return (uint16_t)(acc + v);
}

static const uint8_t buf_a[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
static const uint8_t buf_b[8] = { 10, 20, 30, 40, 50, 60, 70, 80 };
static const uint8_t buf_c[8] = { 100, 99, 98, 97, 96, 95, 94, 93 };

NOINLINE
static uint16_t walk_three_buffers(uint16_t n) {
    const uint8_t *pa = buf_a;
    const uint8_t *pb = buf_b;
    const uint8_t *pc = buf_c;

    uint16_t acc = 0;
    for (uint16_t i = 0; i < n; i++) {
        /* Three pointer-indexed CALLs in a row, each reloading its
         * pointer from BSS across the previous CALL. */
        uint16_t va = fetch_byte(pa, i);
        acc = accumulate(acc, va);
        uint16_t vb = fetch_byte(pb, i);
        acc = accumulate(acc, vb);
        uint16_t vc = fetch_byte(pc, i);
        acc = accumulate(acc, vc);
    }
    return acc;
}

int main(void) {
    volatile uint16_t n = 4;
    /* sums for indices 0..3:
     *   buf_a:   1 +  2 +  3 +  4 =  10
     *   buf_b:  10 + 20 + 30 + 40 = 100
     *   buf_c: 100 + 99 + 98 + 97 = 394
     *   total = 504 = 0x01F8
     */
    return walk_three_buffers(n) == 504u;
}
