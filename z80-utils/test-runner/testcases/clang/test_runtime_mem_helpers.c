/* expect 0x003F */
/*
 * Runtime contract for the z80_rt.a mem helpers after the pop-iy tightening
 * (2026-07-08).  memcpy/memset/memchr were rewritten from an IX-frame prologue
 * to the tight `pop iy` / `jp (iy)` idiom (valid because IY is caller-saved:
 * Z80_CSR = CalleeSavedRegs<(add IX)>).  This fixture pins the two things that
 * rewrite must not break:
 *
 *   1. the copy/fill/search RESULT, and
 *   2. the RETURN VALUE (original dest for memcpy/memset, match ptr / NULL for
 *      memchr) -- the pop-iy version keeps `push .. / pop de` precisely to
 *      return the *original* dest, not dest+n.
 *
 * The helpers are called THROUGH a `volatile` function pointer so clang cannot
 * lower them to inline LDIR (which it does for a direct memcpy/memset even with
 * a runtime size); the indirect call forces a genuine `call _memcpy` /
 * `_memset` / `_memchr` into the archive, exercising the real pop-iy code.
 * rcbios itself references _memcpy/_memset/___umodqi3/__call_iy from the same
 * archive, so this guards its production runtime.  Buffers are file-scope
 * globals to avoid the unrelated O0 static-stack frame issue.
 */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

/* __SIZE_TYPE__ matches the library prototype exactly (no redeclaration warn). */
void *memcpy(void *, const void *, __SIZE_TYPE__);
void *memset(void *, int, __SIZE_TYPE__);
void *memchr(const void *, int, __SIZE_TYPE__);

void *(*volatile p_memcpy)(void *, const void *, __SIZE_TYPE__) = memcpy;
void *(*volatile p_memset)(void *, int, __SIZE_TYPE__) = memset;
void *(*volatile p_memchr)(const void *, int, __SIZE_TYPE__) = memchr;

uint8_t src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
uint8_t dst[8];
uint8_t fill[8];
volatile uint16_t n6 = 6;
volatile uint16_t n0 = 0;

int main(void) {
    uint16_t status = 0;
    uint8_t i;

    /* Bit 0: memcpy copies n bytes AND returns the ORIGINAL dst. */
    for (i = 0; i < 8; i++) dst[i] = 0xAA;
    if (p_memcpy(dst, src, n6) == dst &&
        dst[0] == 1 && dst[5] == 6 && dst[6] == 0xAA)
        status |= 1;

    /* Bit 1: memcpy with n == 0 is a no-op, still returns dst. */
    dst[0] = 0x55;
    if (p_memcpy(dst, src, n0) == dst && dst[0] == 0x55)
        status |= 2;

    /* Bit 2: memset fills n bytes AND returns the ORIGINAL ptr. */
    for (i = 0; i < 8; i++) fill[i] = 0;
    if (p_memset(fill, 0x7E, n6) == fill &&
        fill[0] == 0x7E && fill[5] == 0x7E && fill[6] == 0)
        status |= 4;

    /* Bit 3: memset with n == 0 is a no-op. */
    fill[0] = 0x11;
    if (p_memset(fill, 0x99, n0) == fill && fill[0] == 0x11)
        status |= 8;

    /* Bit 4: memchr finds the byte, returns &src[3]. */
    if (p_memchr(src, 4, n6) == &src[3])
        status |= 16;

    /* Bit 5: memchr misses, returns NULL. */
    if (p_memchr(src, 0x77, n6) == (void *)0)
        status |= 32;

    return status; /* 0x3F when all six pass */
}
