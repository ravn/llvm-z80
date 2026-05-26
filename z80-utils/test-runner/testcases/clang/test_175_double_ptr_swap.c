/* Test 175: double-pointer swap of locals (regression for ravn/llvm-z80#204).
 *
 * At -O0 +static-stack, a BSS-spill->PUSH/POP peephole used to convert the
 * store of `x`'s initial value to a PUSH, even though `x`'s slot is
 * address-taken (&x stored into px) and read indirectly via *px inside
 * swap_pp.  The slot was then never written, so the swap saw stale data and
 * main returned 0 instead of 1 -- only at -O0 +static-stack.  Fixed by sharing
 * the address-taken guard across all four spill->PUSH/POP peepholes (#204/#203).
 *
 * UB-free: no volatile, valid pointers throughout.  Must return 1 at EVERY opt
 * level (the cross-opt-level differential check, -diff-opt, catches a relapse).
 */
typedef short int16_t;

static void swap_pp(int16_t **pa, int16_t **pb) {
    int16_t t = **pa;
    **pa = **pb;
    **pb = t;
}

int main() {
    int16_t x = 42, y = 99;
    int16_t *px = &x, *py = &y;
    swap_pp(&px, &py);
    return (x == 99 && y == 42) ? 1 : 0; /* expect 0x0001 */
}
