/* EXTRA-FLAGS: -mllvm -z80-unreserve-iy */
/* Test 173: ravn/llvm-z80#189/#112 taxonomy Class-C correctness witness.
   i128 loop-carried reduction under -z80-unreserve-iy in the DEFAULT config
   (no +static-stack), where an IY byte-shuttle perturbs SP-relative spill slots.
   i128 byte-decomposes into the most 16-bit chunks of any type (i128-support.ll
   shows ~92 push/pop iy shuttles), so this is the heaviest IY-pressure case.
   Expected value (0x4761) computed with the host compiler, not by hand. */
typedef unsigned __int128 u128;
typedef unsigned long long u64;

u128 f(u128 x) {
    for (int i = 0; i < 12; i++)
        x = (x >> 1) ^ ((x & 1)
                ? ((u128)0xC0C0C0C0C0C0C0C0ULL << 64 | 0x0F0F0F0F0F0F0F0FULL)
                : 0);
    return x;
}

int main() {
    u128 r = f(((u128)0x0123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL);
    return (int)((u64)r & 0xFFFFu); /* expect 0x4761 */
}
