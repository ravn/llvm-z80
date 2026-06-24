/* string.h — CP/M/Z80; functions implemented in z80_rt.a */
#ifndef _CPM_STRING_H
#define _CPM_STRING_H

typedef unsigned int size_t;

void  *memcpy(void *dst, const void *src, size_t n);
void  *memmove(void *dst, const void *src, size_t n);
void  *memset(void *dst, int c, size_t n);
int    memcmp(const void *a, const void *b, size_t n);
void  *memchr(const void *s, int c, size_t n);

size_t strlen(const char *s);
char  *strcpy(char *dst, const char *src);
char  *strncpy(char *dst, const char *src, size_t n);
char  *strcat(char *dst, const char *src);
char  *strncat(char *dst, const char *src, size_t n);
int    strcmp(const char *a, const char *b);
int    strncmp(const char *a, const char *b, size_t n);
char  *strchr(const char *s, int c);
char  *strrchr(const char *s, int c);
char  *strstr(const char *haystack, const char *needle);
char  *stpcpy(char *dst, const char *src);
size_t strnlen(const char *s, size_t maxlen);

#define NULL ((void *)0)

#endif
