/* stdio.h — CP/M/Z80 minimal stdio */
#ifndef _CPM_STDIO_H
#define _CPM_STDIO_H

#include <stdarg.h>

typedef unsigned int size_t;
#define NULL ((void *)0)

/* FILE* — on CP/M there is only one output stream; stdout/stderr both go
   to the console.  FILE is a dummy struct; we only need the pointer value. */
typedef struct _FILE { int fd; } FILE;
extern FILE *stdout;
extern FILE *stderr;
#define stdin  ((FILE *)0)

int putchar(int c);
int puts(const char *s);
int printf(const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);
int snprintf(char *buf, size_t n, const char *fmt, ...);
int vprintf(const char *fmt, va_list ap);
int vsprintf(char *buf, const char *fmt, va_list ap);
int vsnprintf(char *buf, size_t n, const char *fmt, va_list ap);

int fprintf(FILE *f, const char *fmt, ...);
int fputs(const char *s, FILE *f);
int fputc(int c, FILE *f);
int fflush(FILE *f);   /* no-op on CP/M */

/* getchar is CP/M BDOS fn 1 (console in) */
int getchar(void);
int fgetc(FILE *f);

#endif
