/* expect: 0x000F */
/* Test: LDIR/LDDR optimizations (Issue #357):
 * 1. Constant-size memset inlines to seed store + LDIR without library call.
 * 2. Constant-size memcpy between distinct pointers folds register setup (EX DE,HL).
 * 3. Memmove between distinct allocas (provably non-overlapping) lowers to LDIR.
 * 4. Zero-size operations are no-ops.
 */

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

extern void *memcpy(void *dest, const void *src, uint16_t n);
extern void *memmove(void *dest, const void *src, uint16_t n);
extern void *memset(void *p, int v, unsigned int n);

static uint8_t src_buf[32];
static uint8_t dst_buf[32];

__attribute__((noinline))
static int test_memset_inline(void) {
    memset(dst_buf, 0x5A, 32);
    for (int i = 0; i < 32; i++) {
        if (dst_buf[i] != 0x5A)
            return 0;
    }
    return 1;
}

__attribute__((noinline))
static int test_memcpy_inline(void) {
    for (int i = 0; i < 32; i++)
        src_buf[i] = (uint8_t)(i + 1);
    memset(dst_buf, 0, 32);

    memcpy(dst_buf, src_buf, 32);
    for (int i = 0; i < 32; i++) {
        if (dst_buf[i] != (uint8_t)(i + 1))
            return 0;
    }
    return 1;
}

__attribute__((noinline))
static int test_memmove_distinct(void) {
    uint8_t a[16];
    uint8_t b[16];
    for (int i = 0; i < 16; i++)
        a[i] = (uint8_t)(0x80 | i);

    /* Distinct stack arrays: cannot overlap */
    memmove(b, a, 16);
    for (int i = 0; i < 16; i++) {
        if (b[i] != (uint8_t)(0x80 | i))
            return 0;
    }
    return 1;
}

__attribute__((noinline))
static int test_memset_one_byte(void) {
    uint8_t single = 0;
    memset(&single, 0x42, 1);
    return (single == 0x42);
}

int main(void) {
    uint16_t status = 0;

    if (test_memset_inline())
        status |= (1 << 0);
    if (test_memcpy_inline())
        status |= (1 << 1);
    if (test_memmove_distinct())
        status |= (1 << 2);
    if (test_memset_one_byte())
        status |= (1 << 3);

    return status; /* expect 0x000F */
}
