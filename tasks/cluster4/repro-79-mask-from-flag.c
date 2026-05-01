/* #79: `(x != y) ? 0xFF : 0` should lower to:
 *   sub  a, e        ; A = x - y
 *   add  a, $ff      ; carry = (x != y)
 *   sbc  a, a        ; A = -carry
 * = 4 B.  Currently emits a 7-instruction mask chain. */
#include <stdint.h>

uint8_t mask_neq(uint8_t x, uint8_t y) {
    return (x != y) ? 0xFF : 0;
}
