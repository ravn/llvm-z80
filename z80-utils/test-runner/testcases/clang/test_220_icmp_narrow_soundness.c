/* expect 0x0063 */
/*
 * Soundness probe for the #160/#165 icmp-narrowing-through-graph extension:
 * t = x+1 is NOT provably narrow (x = 260 -> t = 261 needs 9 bits), so the
 * bound check t < (y & 0x0f) must be evaluated at 16 bits: 261 < 10 is false
 * -> return 99 (0x63).  If the icmp is unsoundly narrowed to i8, the compare
 * sees trunc(261) = 5 < 10 = true and returns 5 (0x05).
 */
typedef unsigned char u8;
volatile unsigned vx = 260;
volatile unsigned vy = 10;

static u8 pick(unsigned x, unsigned y) __attribute__((noinline));
static u8 pick(unsigned x, unsigned y) {
    unsigned t = x + 1;
    u8 r = (u8)t;
    return (t < (y & 0x0f)) ? r : 99;
}

int main(void) {
    return pick(vx, vy);
}
