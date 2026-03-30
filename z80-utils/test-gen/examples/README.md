# Clang Z80 → CP/M .COM Examples

## hello_cpm.c — Minimal (84 bytes, no stdlib)

Uses custom `cpm_crt0.asm` + `cpm_putchar.asm`. No z88dk dependency.

```bash
clang --target=z80 -Os -nostdinc -c hello_cpm.c -o hello.o
ld.lld -Tcpm.ld cpm_crt0.o hello.o cpm_putchar.o z80_rt.a -o hello.elf
llvm-objcopy -O binary hello.elf HELLO.COM
z88dk-ticks HELLO.COM
```

## hello_cpm_stdlib.c — Full stdio via z88dk (7855 bytes)

Uses `#include <stdio.h>` with z88dk's standard library. Needs the
`clang2z88dk.sh` conversion and `zcc +cpm` for linking.

```bash
# Compile
clang --target=z80 -Os -nostdinc -fno-builtin \
  -include z88dk_compat.h -isystem <z88dk>/include/_DEVELOPMENT/sdcc \
  -S hello_cpm_stdlib.c -o hello.s

# Convert assembly + link
clang2z88dk.sh hello.s hello.asm
sed 's/_cmain/_main/g' hello.asm > hello_final.asm
zcc +cpm -compiler=sdcc hello_final.asm -o HELLO -create-app

# Run
z88dk-ticks HELLO.COM
```

### Why `cmain` → `main` rename?

Clang treats `main` specially: it ignores `__attribute__((sdcccall(0)))` on
`main` and always generates sdcccall(1) return convention (`ld de,0; ret`).
But z88dk's CRT calls `_main` with sdcccall(0) and expects return in HL
(`ld hl,0; ret`).

Workaround: name the function `cmain` in C (so clang respects the sdcccall(0)
attribute), then rename `_cmain` → `_main` in the assembly output. The
`clang2z88dk.sh` converter handles this.

This is a clang frontend limitation — it hardcodes `main`'s calling convention.
A proper fix would teach clang's Z80 target to respect sdcccall attributes on
main, or add a `-fdefault-calling-conv=sdcccall0` flag.

### z88dk_compat.h

Maps z88dk's calling convention macros (`__LIB__`, `__smallc`, `__vasmallc`)
to clang's `__attribute__((sdcccall(0)))`. Include before z88dk headers:

```c
clang ... -include z88dk_compat.h -isystem <z88dk_headers> ...
```

This makes z88dk's `printf`, `puts`, etc. use stack-based argument passing,
matching the z88dk library's expectations. Internal functions in your code
still use clang's default sdcccall(1) (register-based, more efficient).
