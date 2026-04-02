/* Test 94: BSS self-clear with memcpy must not clobber adjacent data.
 *
 * Reproduces issue #51: with +static-stack, relocate_bios() stores
 * a pointer to BSS, then zeros BSS (including the stored pointer),
 * causing memcpy to write to the wrong destination.
 *
 * This test mimics the pattern: a function does several memcpy calls
 * (creating register pressure), then zeros a region using the
 * memcpy(p+1, p, n-1) trick.  The sentinel before the region must
 * survive.
 *
 * NOTE: This test may only fail with +static-stack where function
 * locals are allocated in BSS.  Without it, locals go on the stack
 * and don't overlap the zeroed region.  The test is kept for
 * regression detection when +static-stack is the default. */

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

extern void *memcpy(void *dest, const void *src, uint16_t n);

/* Sentinel: initialized data that must survive the BSS clear below */
static volatile uint8_t sentinel[16] = {
    0x18, 0x42, 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE,
    0x55, 0xAA, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC
};

/* Region to be zeroed — must be AFTER sentinel in memory.
 * Make it large enough that the compiler must spill to service
 * the multiple memcpy calls. */
static uint8_t region_a[64];
static uint8_t region_b[64];
static uint8_t bss_region[128];

/* Heavy function: multiple memcpy calls create register pressure,
 * then BSS clear uses the memcpy(p+1, p, n-1) trick.
 * noinline to ensure it gets its own frame (BSS statics with +static-stack). */
void __attribute__((noinline)) do_work(void)
{
    /* Several memcpy calls to create register pressure */
    memcpy(region_a, sentinel, 16);
    memcpy(region_b, region_a, 64);
    memcpy(region_a + 16, region_b + 16, 32);

    /* Now zero bss_region using the problematic pattern */
    uint8_t *p = bss_region;
    uint16_t n = sizeof(bss_region);
    *p = 0;
    memcpy(p + 1, p, n - 1);
}

int main(void)
{
    uint16_t status = 0;
    uint8_t i;

    /* Fill bss_region with non-zero */
    for (i = 0; i < 128; i++)
        bss_region[i] = 0xFF;

    do_work();

    /* Bit 0: sentinel must be intact */
    {
        static const uint8_t expected[16] = {
            0x18, 0x42, 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE,
            0x55, 0xAA, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC
        };
        uint8_t ok = 1;
        for (i = 0; i < 16; i++) {
            if (sentinel[i] != expected[i])
                ok = 0;
        }
        if (ok) status |= (1 << 0);
    }

    /* Bit 1: bss_region must be all zeros */
    {
        uint8_t ok = 1;
        for (i = 0; i < 128; i++) {
            if (bss_region[i] != 0)
                ok = 0;
        }
        if (ok) status |= (1 << 1);
    }

    /* Bit 2: region_a[0..15] should have sentinel copy */
    {
        uint8_t ok = 1;
        for (i = 0; i < 16; i++) {
            if (region_a[i] != sentinel[i])
                ok = 0;
        }
        if (ok) status |= (1 << 2);
    }

    return status; /* expect 0x0007 */
}
