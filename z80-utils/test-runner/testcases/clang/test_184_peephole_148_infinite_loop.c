/* expect 0x0078 */
/* EXTRA-FLAGS: -Xclang -target-feature -Xclang +static-frame */
/*
 * ravn/llvm-z80#184: peephole #148 rewrote `CP 0xFF; JR Z` to `INC A; JR Z`
 * without seeing the fall-through MBB's PUSH AF, leaving A = counter+1.
 * The post-pop `DEC A; LD counter,A` then subtracted 0 (not 1) from the
 * counter -> loop never reached 0 -> infinite loop.
 *
 * aes_sb_inv_like applies lookup() to each element of a 16-byte buffer
 * iterating i=16..1.  With identity lookup the values are unchanged.
 * sum(0..15) = 120 = 0x78.  With the bug: infinite loop -> ticks timeout.
 */
typedef unsigned char uint8_t;

__attribute__((noinline))
static uint8_t lookup(uint8_t x) { return x; }

__attribute__((noinline))
void aes_sb_inv_like(uint8_t *buf) {
    for (uint8_t i = 16; i != 0; i--)
        buf[i - 1] = lookup(buf[i - 1]);
}

int main(void) {
    uint8_t buf[16], sum = 0, idx;
    for (idx = 0; idx < 16; idx++) buf[idx] = idx;
    aes_sb_inv_like(buf);
    for (idx = 0; idx < 16; idx++) sum += buf[idx];
    return (int)sum;   /* 0+1+...+15 = 120 = 0x78 */
}
