/*
 * 80x25 ASCII Mandelbrot set — floating-point version
 *
 * Uses IEEE 754 single-precision float (compiler-rt soft-float).
 * Build: clang --target=z80 -Os → elf2rel → sdldz80 -l z80_rt
 */
int putchar(int c);

int main(void) {
    int py, px;
    for (py = 0; py < 25; py++) {
        for (px = 0; px < 80; px++) {
            float cr = -2.0f + (float)px * 2.5f / 80.0f;
            float ci = -1.25f + (float)py * 2.5f / 25.0f;

            float zr = 0.0f, zi = 0.0f;
            int iter;
            for (iter = 0; iter < 30; iter++) {
                float zr2 = zr * zr;
                float zi2 = zi * zi;
                if (zr2 + zi2 > 4.0f) break;
                float tmp = zr2 - zi2 + cr;
                zi = 2.0f * zr * zi + ci;
                zr = tmp;
            }
            putchar(iter >= 30 ? '#' : " .:-=+*%@#"[iter % 10]);
        }
        putchar('\n');
    }
    return 0;
}
