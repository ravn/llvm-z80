/* stdio.h — stub for CP/M CP/M builds with clang/llvm-z80
 * Declares only what cpm_io.c implements.  No FILE*, no scanf.
 */
#ifndef _CPM_STDIO_H
#define _CPM_STDIO_H

int putchar(int c);
int puts(const char *s);
int printf(const char *fmt, ...);

#define NULL ((void *)0)

#endif
