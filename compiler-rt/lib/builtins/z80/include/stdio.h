/* stdio.h — minimal standard I/O for Z80
 *
 * Provides putchar, puts, and printf (%d, %u, %x, %s, %c, %%).
 * putchar is provided by the runtime (z80_rt.a / z80_rt.lib).
 * printf is implemented in printf.c (compiled into the program).
 *
 * Usage: clang --target=z80 -Os -ffreestanding
 *              -isystem <path-to-this-directory>
 */
#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

/* Provided by runtime library (putchar.asm) */
int putchar(int c);

/* Provided by printf.c */
int puts(const char *s);
int printf(const char *fmt, ...);

/* No buffering under CP/M (putchar writes directly via port I/O), so
 * FILE/stdout/fflush are trivial stand-ins just to satisfy source that
 * unconditionally calls fflush(stdout) (e.g. dcc/tests/ttt.c). */
typedef int FILE;
#define stdout ((FILE *)0)
static inline int fflush(FILE *f) { (void)f; return 0; }

#endif /* _STDIO_H */
