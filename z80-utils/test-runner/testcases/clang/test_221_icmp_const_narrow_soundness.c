/* expect 0x0063 */
/* Same soundness probe as test_220 but for the #160 CONSTANT-other path:
 * t = 261 compared against literal 10.  16-bit truth: 261 < 10 false -> 99.
 * Unsound i8 narrowing: trunc(261)=5 < 10 true -> 5. */
typedef unsigned char u8;
volatile unsigned vx = 260;

static u8 pick(unsigned x) __attribute__((noinline));
static u8 pick(unsigned x) {
    unsigned t = x + 1;
    u8 r = (u8)t;
    return (t < 10u) ? r : 99;
}

int main(void) {
    return pick(vx);
}
