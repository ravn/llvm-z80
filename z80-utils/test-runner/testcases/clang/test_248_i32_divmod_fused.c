/* Test 248 (ravn/llvm-z80#248): fused i32 divrem.
 *
 * An adjacent x/y and x%y on identical i32 operands now lowers to one
 * __udivmodsi4 call (quotient returned, remainder via a caller pointer)
 * instead of two separate __udivsi3 + __umodsi3 calls.  This fixture
 * checks the fused path produces correct quotient AND remainder across a
 * range of shapes, including the pi-spigot "only the low 16 bits of the
 * remainder are used" pattern that motivated the change.  Signed i32
 * divmod (not fused) is checked too as a non-regression guard.
 *
 * Each check compares the runtime result (volatile operands -> real
 * __udivmodsi4 call) against the same expression on literal operands
 * (constant-folded at compile time = the reference), so the oracle never
 * relies on a hand-computed value.
 */
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef long           int32_t;

int main(void) {
    uint16_t status = 0;

    /* Bit 0: basic fused unsigned divmod, small operands. */
    {
        volatile uint32_t a = 1000003UL, b = 9973UL;
        if (a / b == (1000003UL / 9973UL) && a % b == (1000003UL % 9973UL))
            status |= 1;
    }

    /* Bit 1: full-width operands exercising the high words of both results. */
    {
        volatile uint32_t a = 4000000000UL, b = 123457UL;
        if (a / b == (4000000000UL / 123457UL) &&
            a % b == (4000000000UL % 123457UL))
            status |= 2;
    }

    /* Bit 2: pi-spigot shape -- remainder immediately narrowed to 16 bits
     * (the dead high half of the remainder may be optimized away). */
    {
        volatile uint32_t d = 24680056UL, b = 13UL;
        uint16_t rem16 = (uint16_t)(d % b);
        uint32_t quot  = d / b;
        if (rem16 == (uint16_t)(24680056UL % 13UL) &&
            quot == (24680056UL / 13UL))
            status |= 4;
    }

    /* Bit 3: divisor greater than dividend -> q == 0, r == dividend. */
    {
        volatile uint32_t a = 42UL, b = 1000000UL;
        if (a / b == 0UL && a % b == 42UL)
            status |= 8;
    }

    /* Bit 4: signed i32 divmod (separate __divsi3/__modsi3, not fused) --
     * quotient truncates toward zero, remainder follows the dividend sign. */
    {
        volatile int32_t a = -1000003L, b = 9973L;
        if (a / b == (-1000003L / 9973L) && a % b == (-1000003L % 9973L))
            status |= 16;
    }

    return status; /* expect 0x001F = 31 */
}
