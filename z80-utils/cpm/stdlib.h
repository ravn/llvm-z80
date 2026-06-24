/* stdlib.h — CP/M/Z80 minimal stdlib */
#ifndef _CPM_STDLIB_H
#define _CPM_STDLIB_H

typedef unsigned int size_t;

#define NULL ((void *)0)

/* --- exit --- (C name has no leading underscore so it mangles to the
   assembly symbol _cpm_exit in cpm_crt0.s) */
void cpm_exit(void);
#define exit(n) cpm_exit()

/* --- abs --- */
static inline int abs(int x) { return x < 0 ? -x : x; }
static inline long labs(long x) { return x < 0 ? -x : x; }

/* --- string -> number --- */
int   atoi(const char *s);
long  atol(const char *s);
long  strtol(const char *s, char **endp, int base);
unsigned long strtoul(const char *s, char **endp, int base);

/* --- pseudo-random --- */
int  rand(void);
void srand(unsigned int seed);
#define RAND_MAX 32767

/* --- memory --- */
void *malloc(size_t n);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void  free(void *ptr);

/* --- searching / sorting --- */
void  qsort(void *base, size_t nmemb, size_t size,
            int (*cmp)(const void *, const void *));
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*cmp)(const void *, const void *));

/* --- div --- */
typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;
div_t  div(int num, int denom);
ldiv_t ldiv(long num, long denom);

#endif
