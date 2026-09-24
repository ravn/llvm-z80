/* expect 0x0000 */
/* EXTRA-FLAGS: -Xclang -target-feature -Xclang +static-frame */
/*
 * ravn/llvm-z80#185: the DJNZ peephole fired when the loop body clobbered B.
 * The body used two adjacent pointer offsets forcing HL+BC register use,
 * writing B mid-body.  The peephole dropped the LD B,A restore at the end of
 * the body -> B was corrupt -> infinite loop / wild stores.
 *
 * aes_done_like zeros ctx[32..64] (p=ctx+32; p[i-1]=0; p[i]=0 for i=32..1).
 * After the call, ctx[32..64] must all be zero; sum should be 0.
 * With the bug: infinite loop -> ticks timeout.
 */
typedef unsigned char uint8_t;

__attribute__((noinline))
void aes_done_like(uint8_t *ctx) {
    uint8_t *p = ctx + 32;
    for (uint8_t i = 32; i != 0; i--) {
        p[i - 1] = 0;
        p[i]     = 0;
    }
}

int main(void) {
    uint8_t ctx[68], sum = 0, idx;
    for (idx = 0; idx < 68; idx++) ctx[idx] = (uint8_t)(idx + 1);
    aes_done_like(ctx);
    for (idx = 32; idx < 65; idx++) sum += ctx[idx];
    return (int)sum;   /* ctx[32..64] all zeroed; sum == 0 */
}
