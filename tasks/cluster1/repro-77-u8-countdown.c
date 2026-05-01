/* #77: 8-bit countdown loop should use DJNZ (or dec r; jr nz),
 * not `dec a; ld r, a; or a; jr nz`. */
#include <stdint.h>

extern volatile uint8_t * const port;

/* Simple delay loop -- countdown N times with a body that reads but
 * doesn't write A.  Should compile to: ld b, n; loop: ... ; djnz loop. */
void delay_loop(uint8_t n) {
    while (n--) {
        *port = 0;   /* something side-effecting that doesn't touch counter */
    }
}

/* Same shape, different counter usage -- ensure DJNZ fires when only
 * the loop test reads the counter. */
void countdown_to_zero(uint8_t n) {
    do {
        *port = n;
    } while (--n);
}
