/* EXTRA-FLAGS: -mllvm -z80-unreserve-iy */
/* Test 174: ravn/llvm-z80#189/#112 taxonomy Class-C correctness witness.
   Soft-float arithmetic + comparison under -z80-unreserve-iy in the DEFAULT
   config (no +static-stack).  Float compare/arith (fcmp.ll shows ~16 push/pop iy)
   exercises IY pressure through the compiler-rt soft-float path.  All values are
   integer-valued so the result is exact in IEEE single (no rounding ambiguity).
   Expected value (0x0007) computed with the host compiler, not by hand. */
int g(float a, float b) {
    float s = 0;
    for (int i = 0; i < 10; i++) {
        s += a;
        if (s > b)
            s -= b;
        a += 1.0f;
    }
    return (int)s;
}

int main() {
    return g(3.0f, 17.0f) & 0xFFFF; /* expect 0x0007 */
}
