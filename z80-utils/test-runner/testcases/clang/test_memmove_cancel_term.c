/* SKIP-IF: O0 */
/* O0 skipped: the cancellation (and the memmove fold) only fires at O1+; at O0
 * the call goes to __memmove_rt and trips the pre-existing O0 frame-size
 * miscompile (see test_memmove_runtime_base.c).  The subject here — the
 * runtime-term cancellation in the LDDR end pointer — is an O1+ codegen fold.
 */
/* Runtime-count LDDR with a cancelling runtime term:
 *   base = buf + i;  memmove(base + K, base, C - i)   (K,C constants, i runtime)
 * The end pointer src + (C-i) - 1 = buf + C - 1 folds to a constant offset (the
 * i cancels).  This test confirms the OPTIMIZED code still copies correctly —
 * a wrong cancellation would corrupt the overlapping backward copy.
 */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

uint8_t buf[16];
volatile uint8_t iv = 2;   /* runtime i the optimizer cannot fold */

int main() {
    for (uint8_t k = 0; k < 16; k++) buf[k] = k;
    uint8_t i = iv;                       /* i = 2 at runtime */
    uint8_t *base = buf + i;              /* runtime base */
    /* dst = base + 4 > src -> LDDR; size = 10 - i.  end = base + (10-i) - 1
     * = buf + 9 (i cancels).  Copies buf[2..9] -> buf[6..13] backward. */
    __builtin_memmove(base + 4, base, (uint16_t)(10 - i));
    /* expect buf = {0,1,2,3,4,5, 2,3,4,5,6,7,8,9, 14,15} */
    uint16_t status = 0;
    static const uint8_t want[16] =
        {0,1,2,3,4,5, 2,3,4,5,6,7,8,9, 14,15};
    uint8_t ok = 1;
    for (uint8_t k = 0; k < 16; k++)
        if (buf[k] != want[k]) ok = 0;
    if (ok) status = 0x00A5;   /* distinctive success value */
    return status;
}
/* expect 0x00A5 */
