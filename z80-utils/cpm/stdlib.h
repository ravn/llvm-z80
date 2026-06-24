/* stdlib.h — CP/M stub (no heap: malloc/free not implemented) */
#ifndef _CPM_STDLIB_H
#define _CPM_STDLIB_H

#define NULL ((void *)0)

void _cpm_exit(void);
#define exit(n) _cpm_exit()

/* Minimal abs */
static inline int abs(int x) { return x < 0 ? -x : x; }

#endif
