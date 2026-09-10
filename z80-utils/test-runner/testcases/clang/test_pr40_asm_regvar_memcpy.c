/* expect 0x0001 */
/*
 * PR #40 inline-asm migration guard.  The rcbios (string.h) and cpnos
 * (compat.h) block-move helpers moved from the braced "+{de}" C constraint
 * (rejected by post-PR#40 clang) to GCC local register variables + "+r".
 *
 * This fixture mirrors both helpers with the NEW form and checks they copy
 * correctly.  The load-bearing part is memcpy_z80's TWO-block shape: a
 * remainder LDIR followed by a 16xLDI loop, where the read-write DE/HL must be
 * threaded forward (dest = de; src = hl) between blocks.  Drop that threading
 * and the second block copies from/to the wrong address -> this fixture fails.
 * In-array sentinels catch a one-past overrun.  Companion lit tests pin the
 * lowering: CodeGen/Z80/inline-asm-reg-pair.ll + clang z80-inline-asm-reg-pair.c.
 */
#include <stddef.h>
typedef unsigned char  u8;
typedef unsigned short u16;

/* cpnos compat.h: backward block move (LDDR). de_end/hl_end point at the LAST
 * byte of the destination/source; copies n bytes descending. */
static inline void mem_copy_backwards(void *de_end, const void *hl_end, size_t n) {
    register void *de       __asm__("de") = de_end;
    register const void *hl __asm__("hl") = hl_end;
    register size_t bc      __asm__("bc") = n;
    __asm__ volatile("lddr" : "+r"(de), "+r"(hl), "+r"(bc) :: "memory");
}

/* rcbios string.h: forward memcpy. remainder = n % 16 (initial LDIR, may be 0),
 * blocks16 = (n / 16) * 16 (byte count for the 16xLDI loop). */
static inline void
memcpy_z80(void *dest, const void *src, u16 blocks16, u8 remainder) {
    if (remainder) {
        register void *de       __asm__("de") = dest;
        register const void *hl __asm__("hl") = src;
        register u16 bc         __asm__("bc") = remainder;
        __asm__ volatile("ldir" : "+r"(de), "+r"(hl), "+r"(bc) :: "memory");
        dest = de;              /* thread the advanced pointers into block 2 */
        src  = (const void *)hl;
    }
    if (blocks16) {
        register void *de       __asm__("de") = dest;
        register const void *hl __asm__("hl") = src;
        register u16 bc         __asm__("bc") = blocks16;
        __asm__ volatile(
            "1: ldi\n ldi\n ldi\n ldi\n ldi\n ldi\n ldi\n ldi\n"
            "   ldi\n ldi\n ldi\n ldi\n ldi\n ldi\n ldi\n ldi\n"
            "   jp pe, 1b"
            : "+r"(de), "+r"(hl), "+r"(bc) :: "memory");
    }
}

static u8 srcbuf[40];
static u8 dstbuf[40];

int main(void) {
    u8 i;

    /* ---- forward memcpy_z80 of 37 bytes: remainder 5 + blocks16 32 -------- */
    for (i = 0; i < 40; i++) { srcbuf[i] = (u8)(i + 1); dstbuf[i] = 0; }
    dstbuf[37] = 0xAA; dstbuf[38] = 0xBB; dstbuf[39] = 0xCC; /* sentinels */
    memcpy_z80(dstbuf, srcbuf, /*blocks16=*/32, /*remainder=*/5);
    for (i = 0; i < 37; i++)
        if (((volatile u8 *)dstbuf)[i] != (u8)(i + 1)) return 0;
    if (((volatile u8 *)dstbuf)[37] != 0xAA ||
        ((volatile u8 *)dstbuf)[38] != 0xBB ||
        ((volatile u8 *)dstbuf)[39] != 0xCC) return 0;   /* overrun sentinel */

    /* ---- backward mem_copy_backwards of 20 bytes ------------------------- */
    for (i = 0; i < 40; i++) { srcbuf[i] = (u8)(0x80 + i); dstbuf[i] = 0; }
    dstbuf[20] = 0x5A;   /* sentinel just past the copied region */
    mem_copy_backwards(&dstbuf[19], &srcbuf[19], 20);
    for (i = 0; i < 20; i++)
        if (((volatile u8 *)dstbuf)[i] != (u8)(0x80 + i)) return 0;
    if (((volatile u8 *)dstbuf)[20] != 0x5A) return 0;

    return 1;
}
