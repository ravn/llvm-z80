/* printf.c — minimal printf for Z80
 *
 * Supports: %d, %u, %x, %s, %c, %%, %lu (unsigned long)
 * No width/precision/padding.
 */
#include <stdarg.h>

int putchar(int c);

int puts(const char *s) {
    int count = 0;
    while (*s) { putchar(*s++); count++; }
    putchar('\n');
    return count + 1;
}

static void pr_str(const char *s) {
    while (*s) putchar(*s++);
}

static void pr_uint(unsigned int n) {
    char buf[6];
    int i = 0;
    if (n == 0) { putchar('0'); return; }
    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
    while (i > 0) putchar(buf[--i]);
}

static void pr_int(int n) {
    if (n < 0) { putchar('-'); n = -n; }
    pr_uint((unsigned int)n);
}

/* Same digit-extraction loop as pr_uint, but on `unsigned long` (32-bit)
 * so e.g. dcc/tests/ttt.c's `printf("%lu moves\n", g_Moves)` (g_Moves is
 * uint32_t, can exceed 65535) prints correctly instead of truncating to
 * 16 bits. buf[10] covers ULONG_MAX = "4294967295" (10 digits). */
static void pr_ulong(unsigned long n) {
    char buf[10];
    int i = 0;
    if (n == 0) { putchar('0'); return; }
    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
    while (i > 0) putchar(buf[--i]);
}

static void pr_hex(unsigned int n) {
    char buf[5];
    int i = 0;
    if (n == 0) { putchar('0'); return; }
    while (n > 0) { buf[i++] = "0123456789abcdef"[n & 0xF]; n >>= 4; }
    while (i > 0) putchar(buf[--i]);
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int count = 0;
    char c;
    while ((c = *fmt++) != 0) {
        if (c != '%') { putchar(c); count++; continue; }
        c = *fmt++;
        if (c == 'd') pr_int(va_arg(ap, int));
        else if (c == 'u') pr_uint(va_arg(ap, unsigned int));
        else if (c == 'x' || c == 'X') pr_hex(va_arg(ap, unsigned int));
        else if (c == 's') pr_str(va_arg(ap, const char *));
        else if (c == 'c') putchar((char)va_arg(ap, int));
        else if (c == '%') putchar('%');
        else if (c == 'l') {
            /* only %lu is needed by the current benchmark set; %ld can be
             * added the same way (pr_long) if a caller needs it later. */
            c = *fmt++;
            if (c == 'u') pr_ulong(va_arg(ap, unsigned long));
            else { putchar('%'); putchar('l'); putchar(c); }
        }
        else { putchar('%'); putchar(c); }
        count++;
    }
    va_end(ap);
    return count;
}
