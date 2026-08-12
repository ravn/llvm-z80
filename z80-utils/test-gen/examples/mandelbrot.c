/*
 * 80x25 ASCII Mandelbrot set
 *
 * Uses fixed-point arithmetic (16.16) to avoid floating point.
 * Compile with z88dk cross-link pipeline for printf/putchar.
 */
int putchar(int c);

/* Fixed-point 8.8: fits in 16-bit int, enough for [-2,2] range */
/* Scale: 1.0 = 256 */

#define FP_SHIFT 8
#define FP_ONE   (1 << FP_SHIFT)  /* 256 */
#define FP_MUL(a, b) ((int)((long)(a) * (b) >> FP_SHIFT))

int main(void) {
    int py, px;
    for (py = 0; py < 25; py++) {
        for (px = 0; px < 80; px++) {
            /* Map pixel to complex plane: x in [-2.0, 0.5], y in [-1.25, 1.25].
               cr uses px*8 (== px*640/80): the literal px*640 = 50560 at px=79
               overflows 16-bit int (>32767 from px=52), wrapping negative and
               corrupting the right third.  This restores the RC700 original's
               overflow-safe mapping (z88dk/examples/rc700/mandelbrot.c uses a
               (long) intermediate; this port had dropped it).  py*640 (<=15360)
               doesn't overflow and 640/25 isn't integral, so ci keeps divide. */
            int cr = -512 + px * 8;             /* -2.0 to +0.5 in 8.8 */
            int ci = -320 + (py * 640 / 25);    /* -1.25 to +1.25 in 8.8 */

            int zr = 0, zi = 0;
            int iter;
            for (iter = 0; iter < 30; iter++) {
                int zr2 = FP_MUL(zr, zr);
                int zi2 = FP_MUL(zi, zi);
                if (zr2 + zi2 > 4 * FP_ONE) break;
                int tmp = zr2 - zi2 + cr;
                zi = 2 * FP_MUL(zr, zi) + ci;
                zr = tmp;
            }
            putchar(iter >= 30 ? '#' : " .:-=+*%@#"[iter % 10]);
        }
        putchar('\n');
    }
    return 0;
}
