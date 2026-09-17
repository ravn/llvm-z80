/* ravn/llvm-z80#335 runtime check.
 *
 * The "shift left by one to zero-fill" idiom: seed bss[0]=0, then
 * memcpy(bss+1, bss, N-1). If the compiler ever produced a self-copy
 * (DE=HL at LDIR entry), bss[1..N-1] would keep their pre-existing
 * values instead of becoming zero.
 *
 * This test seeds the array to 0xAA, then runs the idiom, and returns
 * a status word where each bit reports one aspect of correctness. All
 * bits should be set. */

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;

#define N 128

static uint8_t bss_global[N];

__attribute__((noinline))
static void shift_clear_global(void) {
    bss_global[0] = 0;
    __builtin_memcpy(&bss_global[1], &bss_global[0], N - 1);
}

__attribute__((noinline))
static void shift_clear_ptr(uint8_t *p) {
    p[0] = 0;
    __builtin_memcpy(p + 1, p, N - 1);
}

__attribute__((noinline))
static uint16_t check_all_zero(const uint8_t *p) {
    uint16_t bad = 0;
    for (uint16_t i = 0; i < N; i++) {
        if (p[i] != 0) bad++;
    }
    return bad;
}

int main(void) {
    uint16_t status = 0;
    uint16_t i;

    /* Bit 0: global-array flavour. Seed 0xAA, run idiom, expect all zero. */
    for (i = 0; i < N; i++) bss_global[i] = 0xAA;
    shift_clear_global();
    if (check_all_zero(bss_global) == 0) status |= (1u << 0);

    /* Bit 1: heap-like flavour via pointer argument (forces %p+1 to
     * live across the call to shift_clear_ptr). */
    static uint8_t bss_arg[N];
    for (i = 0; i < N; i++) bss_arg[i] = 0x55;
    shift_clear_ptr(bss_arg);
    if (check_all_zero(bss_arg) == 0) status |= (1u << 1);

    /* Bit 2: stack-local buffer flavour. */
    uint8_t local[N];
    for (i = 0; i < N; i++) local[i] = 0x77;
    shift_clear_ptr(local);
    if (check_all_zero(local) == 0) status |= (1u << 2);

    return status; /* expect 0x0007 */
}
