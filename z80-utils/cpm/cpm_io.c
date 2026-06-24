/* cpm_io.c — low-level BDOS console output for CP/M/Z80 clang runtime.
 * Higher-level printf/sprintf/etc. live in cpm_stdlib.c.
 */

void cpm_conout(int c);   /* defined in cpm_crt0.s */

int putchar(int c) {
    cpm_conout(c & 0xFF);
    return c;
}

int puts(const char *s) {
    while (*s)
        cpm_conout((unsigned char)*s++);
    cpm_conout('\r');
    cpm_conout('\n');
    return 0;
}
