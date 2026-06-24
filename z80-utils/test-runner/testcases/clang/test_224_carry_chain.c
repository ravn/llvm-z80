/* Test 224: multi-byte (i32/i64) add/sub carry propagation.
 *
 * Value oracle for the Z80FuseCarryChain pass (B17): the inter-limb carry is
 * threaded in the carry FLAG instead of round-tripped through A.  These cases
 * are chosen so that a dropped or mis-threaded carry produces a WRONG result
 * (the high limb depends on the low limb's carry/borrow).  `volatile` operands
 * defeat constant folding so the real ADD/ADC/SBC chain executes at runtime.
 */
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned long long uint64_t;
typedef signed long long int64_t;

int main() {
    uint16_t status = 0;

    /* Bit 0: i32 add, carry crosses the low->high 16-bit boundary. */
    {
        volatile uint32_t a = 0x0000FFFFUL, b = 0x00000001UL;
        if (a + b == 0x00010000UL) status |= (1 << 0);
    }

    /* Bit 1: i32 add, NO carry across the boundary (control). */
    {
        volatile uint32_t a = 0x12340000UL, b = 0x00005678UL;
        if (a + b == 0x12345678UL) status |= (1 << 1);
    }

    /* Bit 2: i32 sub, borrow crosses the boundary. */
    {
        volatile uint32_t a = 0x00010000UL, b = 0x00000001UL;
        if (a - b == 0x0000FFFFUL) status |= (1 << 2);
    }

    /* Bit 3: signed i32 add crossing zero (carry + sign). */
    {
        volatile int32_t a = -1L, b = 1L;
        if (a + b == 0L) status |= (1 << 3);
    }

    /* Bit 4: i64 add, single carry into the second 16-bit limb. */
    {
        volatile uint64_t a = 0x000000000000FFFFULL, b = 1ULL;
        if (a + b == 0x0000000000010000ULL) status |= (1 << 4);
    }

    /* Bit 5: i64 add, carry CASCADES across all four limbs. */
    {
        volatile uint64_t a = 0x0000FFFFFFFFFFFFULL, b = 1ULL;
        if (a + b == 0x0001000000000000ULL) status |= (1 << 5);
    }

    /* Bit 6: i64 sub, borrow cascades across limbs. */
    {
        volatile uint64_t a = 0x0001000000000000ULL, b = 1ULL;
        if (a - b == 0x0000FFFFFFFFFFFFULL) status |= (1 << 6);
    }

    /* Bit 7: i64 add, full 64-bit wraparound (carry out of the top is dead). */
    {
        volatile uint64_t a = 0xFFFFFFFFFFFFFFFFULL, b = 1ULL;
        if (a + b == 0x0000000000000000ULL) status |= (1 << 7);
    }

    return status; /* expect 0x00FF */
}
