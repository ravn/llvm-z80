/* #78: After __builtin_memcpy(dst, src, N), DE = dst+N (post-LDIR
 * state).  But if the C code follows up with dst += N, the compiler
 * re-derives dst+N from scratch instead of reading DE.
 *
 * Pattern from cpnos-rom netboot_mpm.c: the READ-SEQ loop does
 *   __builtin_memcpy(dma, &msg[DAT + 37], 128);
 *   dma += 128;
 * and the dma reload is observable in the disassembly per-iter. */
#include <stdint.h>

extern uint8_t buf[256];

uint8_t *append_block(uint8_t *dma, const uint8_t *src) {
    __builtin_memcpy(dma, src, 128);
    dma += 128;
    return dma;
}
