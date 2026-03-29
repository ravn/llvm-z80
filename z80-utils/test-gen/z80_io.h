/*
 * z80_io.h — Portable Z80 console I/O for z88dk-ticks
 *
 * Usage: z88dk-ticks -mz80 -iochar 1 program.bin
 *
 * Works with both clang (--target=z80) and zsdcc (zcc +z80 -compiler=sdcc).
 * Output goes to port 1 which z88dk-ticks maps to stdout.
 */
#ifndef Z80_IO_H
#define Z80_IO_H

#ifdef __SDCC
static void z80_putchar(char c) {
    c;  /* parameter in L (sdcccall) */
    __asm__("ld a,l\nout (0x01),a");
}
#else
static void z80_putchar(char c) {
    __asm volatile("out (1),a" : : "a"(c));
}
#endif

static void z80_print(const char *s) {
    while (*s) z80_putchar(*s++);
}

/* Print unsigned 16-bit decimal */
static void z80_print_u16(unsigned short n) {
    char buf[6];
    int i = 0;
    if (n == 0) { z80_putchar('0'); return; }
    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
    while (i > 0) z80_putchar(buf[--i]);
}

/* Print "FAIL at line N\n" */
static void z80_fail(unsigned short line) {
    z80_print("FAIL at ");
    z80_print_u16(line);
    z80_putchar('\n');
}

#endif /* Z80_IO_H */
