/* Test 255: jump-table upper-bound off-by-one (Z80LateOptimization).
 *
 * A dense switch builds a jump table; the range-check narrowing peephole used
 * `cp Range` (offset >= Range -> default) instead of `cp Range+1`
 * (offset > Range), silently routing the MAXIMUM case value (the last dense
 * jump-table slot) to the default block at -O1+.  Exposed by nanoprintf's `%x`.
 *
 * This exercises every case incl. the maximum, so a mis-lowered bound flips at
 * least one bit.  Correct -> 0x00FF.  See bugs/switchbug.c.
 */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

/* Dense 0..7 switch -> jump table (>= 8 entries).  Returns 100+c for a hit,
 * 0 for the default.  The bug sent c==7 (the max) to default (-> 0). */
static uint16_t dispatch(uint8_t c) {
    switch (c) {
        case 0: return 100;
        case 1: return 101;
        case 2: return 102;
        case 3: return 103;
        case 4: return 104;
        case 5: return 105;
        case 6: return 106;
        case 7: return 107;   /* the max case — the off-by-one victim */
        default: return 0;
    }
}

int main(void) {
    uint16_t status = 0;

    /* Bit i set iff dispatch(i) == 100+i for i in 0..7.  The bug clears bit 7. */
    for (uint8_t i = 0; i < 8; i++) {
        volatile uint8_t c = i;             /* volatile: real runtime dispatch */
        if (dispatch(c) == (uint16_t)(100 + i))
            status |= (1u << i);
    }

    return status; /* expect 0x00FF */
}
