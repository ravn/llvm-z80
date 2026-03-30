/* stdlib.h — minimal standard library for Z80 */
#ifndef _STDLIB_H
#define _STDLIB_H

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned int size_t;

int abs(int n);

#endif /* _STDLIB_H */
