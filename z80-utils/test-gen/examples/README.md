# Clang Z80 Examples

## Build Pipelines

### Pipeline 1: ELF (native clang, no external tools)

```bash
clang --target=z80 -Os -nostdinc -c program.c -o program.o
ld.lld --gc-sections -T cpm.ld cpm_crt0.o program.o z80_rt.a -o program.elf
llvm-objcopy -O binary program.elf PROGRAM.COM
z88dk-ticks -iochar 1 PROGRAM.COM
```

- Selective linking via `--gc-sections` + `z80_rt.a`
- `putchar()` uses port 1 (`-iochar 1` in z88dk-ticks)
- Smallest binaries (81B hello world)
- CRT files: `cpm.ld`, `cpm_crt0.o` in `compiler-rt/lib/builtins/z80/`

### Pipeline 2: ELF → elf2rel → SDCC linker

```bash
clang --target=z80 -Os -nostdinc -c program.c -o program.o
elf2rel program.o program.rel
sdldz80 -m -i -b _CODE=0x0100 out cpm_crt0_sdcc.rel program.rel -k <libdir> -l z80_rt
makebin -s 65536 out.ihx out_full.bin
dd if=out_full.bin of=PROGRAM.COM bs=1 skip=256 count=<code_size>
```

- Bypasses sdasz80 (avoids undocumented instruction issue #37)
- Selective linking via SDCC `.lib` archive
- Works with all Z80 programs including those using IY

### Pipeline 3: Assembly conversion → z88dk stdlib

```bash
clang --target=z80 -Os -nostdinc -fno-builtin -S program.c -o program.s
clang2z88dk.sh program.s program.asm
sed 's/_cmain/_main/g' program.asm > final.asm
zcc +cpm -compiler=sdcc final.asm -o PROGRAM -create-app
z88dk-ticks PROGRAM.COM
```

- Uses z88dk's full libc (printf, stdio, file I/O)
- Declare z88dk functions with `__attribute__((sdcccall(0)))`
- Name entry point `cmain` (renamed to `main` in asm conversion)
- `z88dk_compat.h` maps z88dk macros for `#include <stdio.h>`

### Why `cmain` → `main` rename?

Clang ignores `__attribute__((sdcccall(0)))` on `main` — it always uses
sdcccall(1) return convention (DE). z88dk's CRT expects sdcccall(0) return
(HL). Workaround: name the function `cmain` so the attribute is respected,
then rename to `_main` in the assembly output.

## Examples

### hello_cpm.c — Minimal (81 bytes)

Pipeline 1. Custom putchar via port 1. No stdlib.

### hello_cpm_stdlib.c — Full stdio (7812 bytes)

Pipeline 3. `#include <stdio.h>` via z88dk. Uses z88dk's printf.

### mandelbrot.c — Fixed-point 8.8 (905 bytes)

Pipeline 2. 80x25 ASCII Mandelbrot using integer arithmetic.
335M T-states (~84ms at 4MHz).

### mandelbrot_float.c — IEEE 754 float (3555 bytes)

Pipeline 2. Same output using `float`. Soft-float from compiler-rt.
965M T-states (~241ms at 4MHz). ~3x slower than fixed-point.

## Optimization Level Comparison (mandelbrot.c)

| Flag | T-states | Notes |
|------|----------|-------|
| `-O2` | 335.5M | Fastest |
| `-O3` | 335.5M | Same as O2 |
| `-Os` | 335.5M | Same speed, smallest code |
| `-Oz` | 338.1M | 0.8% slower |
| `-O1` | 341.7M | 1.9% slower |

Inner loop is dominated by runtime library calls (`__mulsi3`, `__modhi3`),
so optimization level has minimal impact on execution speed.

## Known Issues

- **#36**: `va_arg` broken — blocks native `printf` (use z88dk pipeline 3)
- **#37**: `LD A,IYH` emitted without `+undocumented` — use `elf2rel` pipeline 2
- **putchar duplicate**: `z80_rt.lib` has both `putchar.rel` and `cpm_putchar.rel` (warning, harmless)

## z88dk_compat.h

Maps z88dk calling convention macros to clang attributes for `#include <stdio.h>`:

```c
#define __LIB__    __attribute__((sdcccall(0)))
#define __smallc   __attribute__((sdcccall(0)))
#define __preserves_regs(...)
```

## Files

| File | Purpose |
|------|---------|
| `cpm.ld` | CP/M linker script (org 0x0100) |
| `cpm_crt0.asm` | CP/M CRT for ELF path |
| `cpm_crt0_sdcc.asm` | CP/M CRT for SDCC path |
| `cpm_putchar.asm` | BDOS putchar (CALL 5) |
| `putchar.asm` | Port-based putchar (-iochar 1) |
| `clang2z88dk.sh` | Assembly converter (clang .s → z88dk .asm) |
| `z88dk_compat.h` | z88dk header compatibility |
