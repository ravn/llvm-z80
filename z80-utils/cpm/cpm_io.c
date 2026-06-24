/* cpm_io.c — minimal CP/M stdio for clang/llvm-z80
 *
 * putchar, puts, printf (%d %u %ld %lu %s %c %%) via BDOS fn 2.
 * cpm_conout takes int so the caller passes HL; assembly reads L.
 */

#include <stdarg.h>

/* Assembly stub in cpm_crt0.s.  Declared as int so caller uses HL;
   L contains the character (low byte of the int).                    */
void cpm_conout(int c);

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

static void print_str(const char *s) {
    if (!s) s = "(null)";
    while (*s)
        cpm_conout((unsigned char)*s++);
}

static void print_ulong(unsigned long v) {
    char buf[11];
    char *p = buf + 10;
    *p = '\0';
    if (v == 0) { *--p = '0'; print_str(p); return; }
    if (v <= 65535U) {
        unsigned int u = (unsigned int)v;
        while (u) { *--p = (char)('0' + (int)(u % 10)); u /= 10; }
    } else {
        while (v) { *--p = (char)('0' + (int)(v % 10)); v /= 10; }
    }
    print_str(p);
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    for (; *fmt; ++fmt) {
        if (*fmt != '%') { cpm_conout((unsigned char)*fmt); continue; }
        ++fmt;
        int lng = 0;
        if (*fmt == 'l') { lng = 1; ++fmt; }
        switch (*fmt) {
        case 'd': {
            long v = lng ? va_arg(ap, long) : (long)va_arg(ap, int);
            if (v < 0) { cpm_conout('-'); v = -v; }
            print_ulong((unsigned long)v);
            break;
        }
        case 'u': {
            unsigned long v = lng ? va_arg(ap, unsigned long)
                                  : (unsigned long)va_arg(ap, unsigned int);
            print_ulong(v);
            break;
        }
        case 's': print_str(va_arg(ap, const char *)); break;
        case 'c': cpm_conout(va_arg(ap, int)); break;
        case '%': cpm_conout('%'); break;
        default:
            cpm_conout('%');
            if (lng) cpm_conout('l');
            cpm_conout(*fmt);
            break;
        }
    }
    va_end(ap);
    return 0;
}
