/*
 * Hello World using clang Z80 + z88dk libc
 *
 * Compile: clang --target=z80 -Os -nostdinc -S hello_libc.c -o hello.s
 * Convert: sed script to strip ELF directives → z88dk .asm
 * Link:    zcc +test -compiler=sdcc hello.asm -o hello -create-app
 * Run:     z88dk-ticks hello.bin
 *
 * z88dk libc uses sdcccall(0) — declare extern functions with the attribute.
 * Internal functions use clang's default sdcccall(1) (register-based, faster).
 */

/* z88dk libc functions — sdcccall(0) for stack-based calling convention */
extern __attribute__((sdcccall(0))) int puts(const char *s);
extern __attribute__((sdcccall(0))) int printf(const char *fmt, ...);

int main(void) {
    puts("Hello World!");
    printf("1 + 2 = %d\n", 1 + 2);
    return 0;
}
