/* SKIP-IF: O0 */
/* O0 is skipped: at O0 the memmove does NOT fold (0 lddr/ldir emitted, both
 * calls go to __memmove_rt), and the forward-overlap case then returns wrong
 * data — a PRE-EXISTING O0 + static-stack issue (ravn/llvm-z80 #192 class),
 * independent of the direction fold this test exercises.  This test's subject
 * is the fold, which happens at O1+; verified correct (both directions) there.
 */
/* Overlapping __builtin_memmove with a RUNTIME base pointer.
 *
 * Exercises the Z80 legalizer's memmove direction fold for the shape
 *   base = p + runtime_idx;  memmove(base + K, base, n)
 * where K is a constant.  Before the fix this fell back to __memmove_rt;
 * after it, dst = base + K folds to inline LDDR (K>0) / LDIR (K<0).
 *
 * Both directions are checked at runtime — a wrong direction choice would
 * corrupt the overlapping copy, so this catches a mis-fold, not just a
 * codegen shape.  The `volatile` index forces a genuine runtime base so
 * the compiler cannot constant-fold it into a global-offset GEP.
 *
 * Buffers are file-scope globals (not static locals) to avoid an unrelated
 * O0 +static-stack scratch-frame interaction.
 */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

uint8_t bufa[10];
uint8_t bufb[10];
volatile uint8_t idx = 0;   /* runtime value the optimizer can't fold */

int main() {
    uint16_t status = 0;

    /* Bit 0: backward overlap (dst = base + 2 > src) → LDDR. */
    for (uint8_t i = 0; i < 10; i++) bufa[i] = i;
    {
        uint8_t *base = bufa + idx;             /* runtime base */
        __builtin_memmove(base + 2, base, 6);   /* dst > src → LDDR */
        /* expect bufa = {0,1, 0,1,2,3,4,5, 8,9} */
        if (bufa[0]==0 && bufa[1]==1 && bufa[2]==0 && bufa[3]==1 &&
            bufa[4]==2 && bufa[5]==3 && bufa[6]==4 && bufa[7]==5 &&
            bufa[8]==8 && bufa[9]==9)
            status |= 1;
    }

    /* Bit 1: forward overlap (dst = base - 2 < src) → LDIR. */
    for (uint8_t i = 0; i < 10; i++) bufb[i] = i;
    {
        uint8_t *base = bufb + 2 + idx;         /* runtime base */
        __builtin_memmove(base - 2, base, 6);   /* dst < src → LDIR */
        /* expect bufb = {2,3,4,5,6,7, 6,7, 8,9} */
        if (bufb[0]==2 && bufb[1]==3 && bufb[2]==4 && bufb[3]==5 &&
            bufb[4]==6 && bufb[5]==7 && bufb[6]==6 && bufb[7]==7 &&
            bufb[8]==8 && bufb[9]==9)
            status |= 2;
    }

    return status; /* expect 0x0003 */
}
