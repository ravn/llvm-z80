/* expect 0x0001 */
/*
 * ravn/llvm-z80#268: a function returning a struct via sret, whose return
 * value comes from another sret-returning call, copied the result to the wrong
 * destination when callee-saved registers (CSR > 0, i.e. IX saved) were
 * present.  eliminateFrameIndex added CalleeSavedFrameSize to fixed objects
 * (sret pointer and byval args), shifting the sret destination by 2.  The
 * memmove then wrote to the wrong address; on CP/M that address is often
 * 0x0000 (warm-boot vector) -> hang.
 *
 * bug() takes two struct-by-value args (forces IX frame -> CSR=2) and returns
 * g()'s result via sret.  With the fix, r.a == 1; with the bug, r.a gets
 * the value at the wrong address (garbage or 0).
 */
typedef unsigned short uint16_t;

typedef struct { uint16_t a, b, c, d; } F;   /* 8 bytes = i64 on Z80 */

__attribute__((noinline))
F g(void) {
    F r = {1, 2, 3, 4};
    return r;
}

__attribute__((noinline))
F bug(F a, F b) {
    (void)a; (void)b;
    return g();   /* sret chain; IX frame due to byval args -> CSR=2 */
}

int main(void) {
    F zero = {0, 0, 0, 0};
    F r = bug(zero, zero);
    return (int)(r.a == 1 && r.b == 2 && r.c == 3 && r.d == 4);
}
