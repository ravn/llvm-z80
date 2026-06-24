/* stdint.h — CP/M Z80 (16-bit int, 32-bit long) */
#ifndef _CPM_STDINT_H
#define _CPM_STDINT_H

typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef long               int32_t;
typedef unsigned long      uint32_t;

typedef int8_t             int_least8_t;
typedef uint8_t            uint_least8_t;
typedef int16_t            int_least16_t;
typedef uint16_t           uint_least16_t;
typedef int32_t            int_least32_t;
typedef uint32_t           uint_least32_t;

typedef int16_t            intptr_t;
typedef uint16_t           uintptr_t;
typedef int32_t            intmax_t;
typedef uint32_t           uintmax_t;

#define INT8_MIN    (-128)
#define INT8_MAX    127
#define UINT8_MAX   255U
#define INT16_MIN   (-32768)
#define INT16_MAX   32767
#define UINT16_MAX  65535U
#define INT32_MIN   (-2147483648L)
#define INT32_MAX   2147483647L
#define UINT32_MAX  4294967295UL

#endif
