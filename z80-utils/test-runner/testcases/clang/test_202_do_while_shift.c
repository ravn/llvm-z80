/* expect 0x0000 */
/* EXTRA-FLAGS: -Xclang -target-feature -Xclang +static-frame */
/*
 * ravn/llvm-z80#202: at -O0 with +static-frame, the cross-block BSS-spill
 * peephole dropped the store-back of the shift result.  The next iteration
 * reloaded the original value -> loop never progressed.
 * f(256): 256->128->64->32->16->8->4->2->1->0, returns 0.
 * With the bug: infinite loop -> ticks timeout.
 * Note: -O1+ constant-folds this loop away; the bug is -O0-only.
 */
typedef unsigned short uint16_t;

__attribute__((noinline))
uint16_t f(uint16_t v) {
    do { v >>= 1; } while (v > 0);
    return v;
}

int main(void) {
    return (int)f(256);   /* correct: 0; buggy at -O0: infinite loop */
}
