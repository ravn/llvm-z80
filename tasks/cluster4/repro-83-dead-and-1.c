/* #83: storing a known-1 _Bool emits `ld a,$1; and $1` -- the AND is
 * dead because A is already 1, so 1 & 1 = 1.  Saves 2 B per site. */
#include <stdint.h>
#include <stdbool.h>

static volatile bool flag;

void set_flag_true(void) {
    flag = true;
}
