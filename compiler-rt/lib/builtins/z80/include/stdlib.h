/* stdlib.h — minimal standard library for Z80 */
#ifndef _STDLIB_H
#define _STDLIB_H

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned int size_t;

int abs(int n);
int atoi(const char *nptr);
void exit(int status);

/* Provided by heap.c (first-fit free-list allocator over a static arena;
 * compile heap.c alongside the program the same way printf.c is compiled
 * in — it is not part of z80_rt.lib). */
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);

#endif /* _STDLIB_H */
