/* #86: switch on u8 discriminant uses 16-bit SUB/SBC for range check
 * instead of 8-bit CP. */
#include <stdint.h>

extern void low(void), mid(void), high(void), other(void);

void switch_u8(uint8_t v) {
    switch (v) {
    case 0: low(); break;
    case 1: mid(); break;
    case 2: high(); break;
    default: other(); break;
    }
}
