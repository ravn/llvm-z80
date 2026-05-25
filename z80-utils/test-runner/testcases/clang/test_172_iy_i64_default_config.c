/* EXTRA-FLAGS: -mllvm -z80-unreserve-iy */
/* Test 172: ravn/llvm-z80#189/#112 taxonomy -- wide (i64) loop-carried reduction
   under -z80-unreserve-iy in the DEFAULT (no +static-stack) config, the config
   where an IY byte-shuttle perturbs SP-relative spill slots and miscompiles.
   i64 arithmetic byte-decomposes into many 16-bit chunks and drives heavy IY use
   (the i128/i64 lit files show dozens of push/pop iy shuttles), so this exercises
   whether the residual shuttles left after the #189 getLargest + narrow fixes are
   value-correct (density only) or recur the corruption.  Expected value computed
   with the host compiler (0x7315), not by hand. */
typedef unsigned char uint8_t;
typedef unsigned long long uint64_t;

uint64_t f(uint64_t x) {
    for (uint8_t i = 0; i < 8; i++)
        x = (x >> 1) ^ ((x & 1) ? 0xC0C0C0C0C0C0C0C0ULL : 0);
    return x;
}

int main() {
    uint64_t r = f(0x0123456789ABCDEFULL);
    return (int)(r & 0xFFFFu); /* expect 0x7315 */
}
