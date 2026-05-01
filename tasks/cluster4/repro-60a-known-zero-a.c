/* #60 / 'ld a,$0' after xor a -- the OUT in between doesn't touch A,
 * so the second `ld a, $0` is redundant. */
#include <stdint.h>

static inline void out_n(uint8_t port, uint8_t v) {
    __asm__ volatile ("out (c),%0" : : "r"(v), "c"(port) : "memory");
}

void out_zero_zero(void) {
    /* hand-asm equivalent that ISR_CRT actually contains:
     *   xor a; out (0xFC),a; ld a, 0; out (0xF4),a
     * but as C we have to use inline asm to force the OUT. */
    out_n(0xfc, 0);
    out_n(0xf4, 0);
}
