/*
 * z88dk_compat.h — compatibility header for using z88dk headers with clang
 *
 * Include this BEFORE any z88dk headers. It maps z88dk's calling convention
 * macros to clang's __attribute__((sdcccall(0))).
 *
 * Usage:
 *   clang --target=z80 -Os -nostdinc -include z88dk_compat.h \
 *         -isystem /opt/z88dk/include -fno-builtin ...
 */
#ifndef Z88DK_COMPAT_H
#define Z88DK_COMPAT_H

/* z88dk calling convention macros → clang sdcccall(0) */
#define __LIB__         __attribute__((sdcccall(0)))
#define __SAVEFRAME__
#define __smallc        __attribute__((sdcccall(0)))
#define __stdc          __attribute__((sdcccall(0)))
#define __vasmallc      __attribute__((sdcccall(0)))
#define __z88dk_callee  __attribute__((sdcccall(0)))
#define __z88dk_fastcall
#define __FASTCALL__
#define __CALLEE__      __attribute__((sdcccall(0)))
#define __naked

/* Compiler identification — __SDCC must be 1 (not empty) for #elif __SDCC */
#define __SDCC 1
#define __Z80 1
#undef __Z80__
#define __Z80__ 1

/* Suppress z88dk's _Float16 (not supported by clang Z80) */
#define _FLOAT16

/* SDCC-specific attributes not supported by clang */
#define __preserves_regs(...)
#define __banked

#endif /* Z88DK_COMPAT_H */
