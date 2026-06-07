/* expect 0x0000 */
/*
 * Runtime oracle for the wide-copy -> block-move combine (Fix A of the
 * AVR-absorption work): i64/i32 load+store pairs become G_MEMMOVE.
 *
 * Distinct-buffer copies only — overlapping assignment is UB in C
 * (C11 6.5.16.1p3); the overlap semantics are runtime-verified at the IR
 * level instead (llc suite test_27_wide_copy_overlap.ll).
 *
 * Expected values computed by a byte-loop reference (no wide load/store
 * pair, so the combine cannot touch it).  Returns fail count; expect 0.
 */
typedef unsigned char u8;
typedef unsigned long u32;
typedef unsigned long long u64;

volatile u8 seed[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                        0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x10};

static u8 buf[16], ref[16];

static void load_bufs(void) {
    for (u8 i = 0; i < 16; i++) { buf[i] = seed[i]; ref[i] = seed[i]; }
}

static void ref_copy(u8 *d, const u8 *s, u8 n) __attribute__((noinline));
static void ref_copy(u8 *d, const u8 *s, u8 n) {
    for (u8 i = 0; i < n; i++) d[i] = s[i];
}

static u8 cmp16(void) {
    u8 bad = 0;
    for (u8 i = 0; i < 16; i++)
        if (buf[i] != ref[i]) bad++;
    return bad;
}

/* the shapes under test — wide load+store pairs through pointers */
static void copy64(u64 *d, const u64 *s) __attribute__((noinline));
static void copy64(u64 *d, const u64 *s) { *d = *s; }
static void copy32(u32 *d, const u32 *s) __attribute__((noinline));
static void copy32(u32 *d, const u32 *s) { *d = *s; }

int main(void) {
    unsigned fails = 0;

    /* 1: distinct u64 */
    load_bufs();
    copy64((u64 *)(void *)&buf[8], (const u64 *)(void *)&buf[0]);
    ref_copy(&ref[8], &ref[0], 8);
    fails += cmp16();

    /* 2: distinct u32 */
    load_bufs();
    copy32((u32 *)(void *)&buf[12], (const u32 *)(void *)&buf[4]);
    ref_copy(&ref[12], &ref[4], 4);
    fails += cmp16();

    return (int)fails;
}
