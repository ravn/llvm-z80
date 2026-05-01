/* #88: pattern-fill loop should lower to seed-and-LDIR.
 *
 *   for (uint8_t n = 18; n; --n) *p++ = (uint16_t)CONST_PTR;
 *
 * Currently emits a 25-byte loop with per-iter constant reload (#89
 * sibling) and HL-via-BC roundtrip (#84 sibling).  Optimal Z80:
 *
 *   ld hl, base         ; 3
 *   ld (hl), low(c)     ; 2
 *   inc hl              ; 1
 *   ld (hl), high(c)    ; 2
 *   dec hl              ; 1
 *   ld de, base+2       ; 3
 *   ld bc, 2*N-2        ; 3
 *   ldir                ; 2
 *                       ; total 17 bytes (-8)
 */
#include <stdint.h>
extern void target_fn(void);

void word_fill_18(void) {
    volatile uint16_t *p = (volatile uint16_t *)0xF500;
    for (uint8_t n = 18; n; --n) {
        *p++ = (uint16_t)(uintptr_t)&target_fn;
    }
}
