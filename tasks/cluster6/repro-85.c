/* #85: Sequential consecutive-address stores should compile to:
 *   ld hl, base; ld (hl), v0; inc hl; ld (hl), v1; inc hl; ...
 * vs current:
 *   ld a, v0; ld (base), a; ld a, v1; ld (base+1), a; ...
 *
 * Same total bytes for 2 stores; HL-walked is shorter starting at 3.
 *   2 stores: 2*(2+3) = 10 B   vs  3 + 2 + 1 + 2 = 8 B (-2)
 *   3 stores: 3*(2+3) = 15 B   vs  3 + 2 + 1 + 2 + 1 + 2 = 11 B (-4)
 *   4 stores: 4*(2+3) = 20 B   vs  3 + 4*(2+1) - 1 = 14 B (-6)
 */
#include <stdint.h>

extern uint8_t buf[8];

void seed_buf(void) {
    buf[0] = 0x10;
    buf[1] = 0x20;
    buf[2] = 0x30;
    buf[3] = 0x40;
}
