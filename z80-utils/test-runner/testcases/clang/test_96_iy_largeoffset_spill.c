/* expect 0x0001 */
/* Test: indirect call via function pointer at -O0 with a stack frame
 * large enough that the function-pointer spill slot lives outside the
 * IX-indexed (-128..+127) range.
 *
 * Pre-#28-residual-fix the SPILL_GR16/RELOAD_GR16 large-offset helpers
 * silently mishandled IX/IY destinations (the dispatch fell through
 * to the DE encoding because of `(reg == BC) ? ... : DE-side`).  The
 * function-pointer reload into IY produced `LD E,(HL); INC HL; LD
 * D,(HL)` instead, leaving IY uninitialised and clobbering the second
 * argument reg.  At -O0, indirect calls in any function with > ~64
 * bytes of locals would silently miscompile.
 *
 * This test fits in the test-runner harness rather than lit because
 * the bug only surfaced once the address actually got loaded and
 * called at runtime; the lit version asserts the asm pattern.
 */

typedef unsigned short uint16_t;

#define NOINLINE __attribute__((noinline))

NOINLINE
uint16_t sub2(uint16_t a, uint16_t b) { return a - b; }

NOINLINE
static int call_via_ptr(void)
{
    /* Force a large stack frame so the local fnptr sits below IX-128.
     * 200 uint16_t = 400 bytes of locals, far past the IX+d range. */
    volatile uint16_t pad[200];
    for (uint16_t i = 0; i < 200; i++) pad[i] = i;

    uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
    uint16_t r = fn(99, 155);

    /* Touch pad after the call to keep it live across the call. */
    uint16_t sum = 0;
    for (uint16_t i = 0; i < 200; i++) sum = (uint16_t)(sum + pad[i]);
    if (sum != 19900) return 0;

    return r == 65480;  /* 99 - 155 = -56 = 0xFFC8 = 65480 */
}

int main(void)
{
    return call_via_ptr();
}
