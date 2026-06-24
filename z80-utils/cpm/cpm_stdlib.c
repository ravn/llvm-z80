/* cpm_stdlib.c — stdlib + stdio extras for CP/M/Z80 clang runtime */

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ---- FILE* stubs ---- */
static FILE _stdout_obj = { .fd = 1 };
static FILE _stderr_obj = { .fd = 2 };
FILE *stdout = &_stdout_obj;
FILE *stderr = &_stderr_obj;

int errno = 0;

/* ---- BDOS console I/O ---- */
void cpm_conout(int c);

/* getchar implemented in cpm_crt0.s (needs register-level BDOS access) */
int getchar(void);

int fgetc(FILE *f) { (void)f; return getchar(); }
int fputc(int c, FILE *f) { (void)f; cpm_conout(c & 0xFF); return c; }
int fputs(const char *s, FILE *f) { (void)f; while (*s) cpm_conout((unsigned char)*s++); return 0; }
int fflush(FILE *f) { (void)f; return 0; }
int fprintf(FILE *f, const char *fmt, ...) {
    (void)f;
    va_list ap; va_start(ap, fmt); int r = vprintf(fmt, ap); va_end(ap); return r;
}

/* ---- sprintf / snprintf ---- */
typedef struct { char *p; int remaining; } _sbuf;

/* When b->p is NULL, output goes straight to the console (unbounded);
   otherwise it accumulates into the caller's buffer (snprintf-style). */
static void _sputc(int c, _sbuf *b) {
    if (b->p == 0) { cpm_conout(c); return; }
    if (b->remaining > 1) { *b->p++ = (char)c; b->remaining--; }
}

int vsprintf(char *buf, const char *fmt, va_list ap) {
    return vsnprintf(buf, 32767, fmt, ap);
}

int sprintf(char *buf, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); int r = vsprintf(buf, fmt, ap); va_end(ap); return r;
}

/* vsnprintf — separate buffer-writing printf; shares logic with vprintf */
static void _write_ulong_s(_sbuf *b, unsigned long v) {
    char tmp[11]; char *p = tmp + 10; *p = '\0';
    if (v == 0) { _sputc('0', b); return; }
    if (v <= 65535U) { unsigned u = (unsigned)v; while (u) { *--p = (char)('0' + u%10); u /= 10; } }
    else              { while (v) { *--p = (char)('0' + (int)(v%10)); v /= 10; } }
    while (*p) _sputc(*p++, b);
}

static int _vfmt(_sbuf *b, const char *fmt, va_list ap) {
    int n = 0;
    for (; *fmt; ++fmt) {
        if (*fmt != '%') { _sputc(*fmt, b); n++; continue; }
        ++fmt;
        /* flags */
        int left = 0;
        if (*fmt == '-') { left = 1; ++fmt; }
        /* width */
        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') { width = width * 10 + (*fmt - '0'); ++fmt; }
        /* length modifier */
        int lng = 0;
        if (*fmt == 'l') { lng = 1; ++fmt; }

        char tmp[24]; char *s = tmp; int len;

        switch (*fmt) {
        case 'd': case 'i': {
            long v = lng ? va_arg(ap, long) : (long)va_arg(ap, int);
            char *p = tmp + 22; *p = '\0';
            int neg = v < 0; if (neg) v = -v;
            unsigned long u = (unsigned long)v;
            if (u == 0) *--p = '0';
            else while (u) { *--p = (char)('0' + (int)(u%10)); u /= 10; }
            if (neg) *--p = '-';
            s = p; len = (int)(tmp + 22 - p); break;
        }
        case 'u': {
            unsigned long v = lng ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned);
            char *p = tmp + 22; *p = '\0';
            if (v == 0) *--p = '0';
            else while (v) { *--p = (char)('0' + (int)(v%10)); v /= 10; }
            s = p; len = (int)(tmp + 22 - p); break;
        }
        case 'x': case 'X': {
            unsigned long v = lng ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned);
            const char *hex = *fmt == 'x' ? "0123456789abcdef" : "0123456789ABCDEF";
            char *p = tmp + 22; *p = '\0';
            if (v == 0) *--p = '0';
            else while (v) { *--p = hex[v & 0xF]; v >>= 4; }
            s = p; len = (int)(tmp + 22 - p); break;
        }
        case 'o': {
            unsigned long v = lng ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned);
            char *p = tmp + 22; *p = '\0';
            if (v == 0) *--p = '0';
            else while (v) { *--p = (char)('0' + (int)(v & 7)); v >>= 3; }
            s = p; len = (int)(tmp + 22 - p); break;
        }
        case 's': {
            s = va_arg(ap, char *); if (!s) s = "(null)";
            len = (int)strlen(s); break;
        }
        case 'c': {
            tmp[0] = (char)va_arg(ap, int); tmp[1] = '\0';
            s = tmp; len = 1; break;
        }
        case 'p': {
            unsigned v = (unsigned)(unsigned int)va_arg(ap, void *);
            char *p = tmp + 22; *p = '\0';
            const char *hex = "0123456789abcdef";
            if (v == 0) *--p = '0';
            else while (v) { *--p = hex[v & 0xF]; v >>= 4; }
            s = p; len = (int)(tmp + 22 - p); break;
        }
        case '%': _sputc('%', b); n++; continue;
        default:
            _sputc('%', b); if (lng) _sputc('l', b); _sputc(*fmt, b); n += 2 + lng;
            continue;
        }

        int pad = width > len ? width - len : 0;
        if (!left) while (pad--) { _sputc(' ', b); n++; }
        while (*s) { _sputc(*s++, b); n++; }
        if (left)  while (pad--) { _sputc(' ', b); n++; }
    }
    if (b->p && b->remaining >= 1) *b->p = '\0';
    return n;
}

int vprintf(const char *fmt, va_list ap) {
    /* p==NULL selects direct-to-console output (no length limit). */
    _sbuf b = { 0, 0 };
    return _vfmt(&b, fmt, ap);
}

int vsnprintf(char *buf, size_t sz, const char *fmt, va_list ap) {
    _sbuf b = { buf, (int)sz };
    return _vfmt(&b, fmt, ap);
}

int snprintf(char *buf, size_t sz, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); int r = vsnprintf(buf, sz, fmt, ap); va_end(ap); return r;
}

/* Override printf to use the new width-aware vprintf */
int printf(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); int r = vprintf(fmt, ap); va_end(ap); return r;
}

/* ---- atoi / atol / strtol ---- */
static long _strtol_impl(const char *s, char **endp, int base, int unsig) {
    while (*s == ' ' || *s == '\t') s++;
    int neg = 0;
    if (!unsig && *s == '-') { neg = 1; s++; }
    else if (*s == '+') s++;
    if (base == 0) {
        if (*s == '0' && (s[1] == 'x' || s[1] == 'X')) { base = 16; s += 2; }
        else if (*s == '0') { base = 8; s++; }
        else base = 10;
    } else if (base == 16 && *s == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
    unsigned long v = 0;
    const char *start = s;
    for (; *s; s++) {
        int d;
        if (*s >= '0' && *s <= '9') d = *s - '0';
        else if (*s >= 'a' && *s <= 'z') d = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'Z') d = *s - 'A' + 10;
        else break;
        if (d >= base) break;
        v = v * (unsigned)base + (unsigned)d;
    }
    (void)start;
    if (endp) *endp = (char *)s;
    return neg ? -(long)v : (long)v;
}

long  strtol(const char *s, char **endp, int base) { return _strtol_impl(s, endp, base, 0); }
unsigned long strtoul(const char *s, char **endp, int base) { return (unsigned long)_strtol_impl(s, endp, base, 1); }
int  atoi(const char *s) { return (int)strtol(s, 0, 10); }
long atol(const char *s) { return strtol(s, 0, 10); }

/* ---- rand/srand ---- */
static unsigned long _rand_seed = 1;
int rand(void) {
    _rand_seed = _rand_seed * 1103515245UL + 12345UL;
    return (int)((_rand_seed >> 16) & 0x7FFF);
}
void srand(unsigned int seed) { _rand_seed = seed; }

/* ---- malloc (bump allocator over the real TPA heap) ----
   Heap region [__heap_start, __heap_end) is defined by cpm.ld: it spans the
   free TPA between end-of-BSS and a ceiling below the stack.  free() is a
   no-op (bump allocator). */
/* Linker symbols (cpm.ld) — underscore-free here so C mangling -> _heap_*. */
extern char heap_start[];
extern char heap_end[];
static char *_heap_cur = 0;

void *malloc(size_t n) {
    if (_heap_cur == 0) _heap_cur = heap_start;
    n = (n + 1U) & ~1U;                 /* align to 2 bytes */
    /* Overflow-safe bound check: compare remaining space, never add first. */
    unsigned int avail = (unsigned int)(heap_end - _heap_cur);
    if (n > avail) return NULL;
    void *p = _heap_cur;
    _heap_cur += n;
    return p;
}
void *calloc(size_t nmemb, size_t size) {
    size_t n = nmemb * size;
    void *p = malloc(n);
    if (p) memset(p, 0, n);
    return p;
}
void  free(void *ptr) { (void)ptr; }          /* bump allocator: no-op free */
void *realloc(void *old, size_t sz) {
    void *p = malloc(sz);
    (void)old;
    return p;
}

/* ---- qsort (insertion sort; stable, no stack recursion) ---- */
void qsort(void *base, size_t nmemb, size_t size,
           int (*cmp)(const void *, const void *)) {
    char *b = (char *)base;
    char tmp[64];   /* max element size we handle */
    if (size > sizeof(tmp)) return;
    for (size_t i = 1; i < nmemb; i++) {
        memcpy(tmp, b + i * size, size);
        size_t j = i;
        while (j > 0 && cmp(b + (j-1)*size, tmp) > 0) {
            memcpy(b + j*size, b + (j-1)*size, size);
            j--;
        }
        memcpy(b + j * size, tmp, size);
    }
}

/* ---- bsearch ---- */
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*cmp)(const void *, const void *)) {
    const char *b = (const char *)base;
    size_t lo = 0, hi = nmemb;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        int r = cmp(key, b + mid * size);
        if (r == 0) return (void *)(b + mid * size);
        if (r < 0) hi = mid; else lo = mid + 1;
    }
    return NULL;
}

/* ---- div / ldiv ---- */
div_t div(int num, int denom) {
    div_t r; r.quot = num / denom; r.rem = num % denom; return r;
}
ldiv_t ldiv(long num, long denom) {
    ldiv_t r; r.quot = num / denom; r.rem = num % denom; return r;
}

/* ---- strrchr / strstr (not in z80_rt.a) ---- */
char *strrchr(const char *s, int c) {
    const char *last = NULL;
    while (*s) { if (*s == (char)c) last = s; s++; }
    if ((char)c == '\0') last = s;
    return (char *)last;
}

char *strstr(const char *hay, const char *needle) {
    size_t nl = strlen(needle);
    if (!nl) return (char *)hay;
    for (; *hay; hay++)
        if (strncmp(hay, needle, nl) == 0) return (char *)hay;
    return NULL;
}

/* ---- strncat (not in z80_rt.a) ---- */
char *strncat(char *dst, const char *src, size_t n) {
    char *d = dst + strlen(dst);
    while (n-- && *src) *d++ = *src++;
    *d = '\0';
    return dst;
}
