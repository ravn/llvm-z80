/* ctype.h — CP/M stub; all functions work on ASCII 0-127 */
#ifndef _CPM_CTYPE_H
#define _CPM_CTYPE_H

static inline int isdigit(int c)  { return c >= '0' && c <= '9'; }
static inline int isalpha(int c)  { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static inline int isalnum(int c)  { return isdigit(c) || isalpha(c); }
static inline int isspace(int c)  { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'; }
static inline int isupper(int c)  { return c >= 'A' && c <= 'Z'; }
static inline int islower(int c)  { return c >= 'a' && c <= 'z'; }
static inline int isprint(int c)  { return c >= 0x20 && c < 0x7F; }
static inline int ispunct(int c)  { return isprint(c) && !isalnum(c) && c != ' '; }
static inline int iscntrl(int c)  { return (unsigned)c < 0x20 || c == 0x7F; }
static inline int isxdigit(int c) { return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
static inline int toupper(int c)  { return islower(c) ? c - 'a' + 'A' : c; }
static inline int tolower(int c)  { return isupper(c) ? c - 'A' + 'a' : c; }

#endif
