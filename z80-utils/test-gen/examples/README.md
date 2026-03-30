# Clang Z80 → CP/M .COM

## How to build CP/M binaries

Use the ELF → elf2rel → SDCC linker pipeline. This compiles through clang's
full optimization pipeline (including `copyPhysReg` expansion), then converts
the ELF object to SDCC `.rel` format for linking with `sdldz80`.

```bash
# 1. Compile to ELF object (runs all LLVM passes, no undocumented instructions)
clang --target=z80 -Os -nostdinc -ffunction-sections -fdata-sections \
  -c program.c -o program.o

# 2. Convert ELF → SDCC .rel
elf2rel program.o program.rel

# 3. Link with SDCC linker (selective: only pulls needed library members)
sdldz80 -m -i -b _CODE=0x0100 output \
  cpm_crt0_sdcc.rel \
  program.rel \
  -k /path/to/z80/lib/ -l z80_rt

# 4. Convert Intel HEX → CP/M .COM binary
makebin -s 65536 output.ihx output_full.bin
dd if=output_full.bin of=PROGRAM.COM bs=1 skip=256 count=<code_size>

# 5. Run
z88dk-ticks -iochar 1 PROGRAM.COM
```

### Why this pipeline

- **Correct code**: clang's full pass pipeline runs, including `ExpandPostRAPseudos`
  which expands register copies to documented instructions. The SDCC assembly path
  (`--target=z80-unknown-none-sdcc -c`) skips this and emits undocumented `LD A,IYH` (#37).
- **Selective linking**: `sdldz80 -l z80_rt` only pulls in library members that
  resolve undefined symbols (e.g. `__mulsi3`, `__modhi3` for arithmetic).
- **No z88dk dependency**: uses only clang + elf2rel + sdldz80 + makebin.

### Runtime library

`z80_rt.lib` provides:
- Arithmetic: `__mulsi3`, `__divmodsi3`, `__mulhi3`, etc.
- String/memory: `memcpy`, `memset`, `strlen`, `strcmp`, etc.
- Float (IEEE 754): `__addsf3`, `__mulsf3`, `__divsf3`, etc.
- I/O: `putchar` (port 1, for z88dk-ticks `-iochar 1`)

### CRT files

| File | Purpose |
|------|---------|
| `cpm_crt0_sdcc.asm` | CP/M CRT: BSS init, call main, JP 0 exit |
| `cpm.ld` | Linker script for ELF path (org 0x0100) |
| `putchar.asm` | Port-based putchar for z88dk-ticks |

### Code size from map file

After linking, check `output.map` for `_CODE` size:
```
_CODE ... size <hex>
```

## Examples

### mandelbrot.c — Fixed-point 8.8 (905 bytes)

80x25 ASCII Mandelbrot. 335M T-states (~84ms at 4MHz).

### mandelbrot_float.c — IEEE 754 float (3555 bytes)

Same output using `float`. 965M T-states (~241ms at 4MHz). ~3x slower.

### hello_cpm.c — Minimal hello world (81 bytes)

Uses `putchar()` only. No printf, no stdlib.

### Optimization comparison (mandelbrot.c)

| Flag | T-states | Notes |
|------|----------|-------|
| `-O2` | 335.5M | Fastest |
| `-O3` | 335.5M | Same as O2 |
| `-Os` | 335.5M | Same speed, smallest code |
| `-Oz` | 338.1M | 0.8% slower |
| `-O1` | 341.7M | 1.9% slower |

Inner loop dominated by runtime library calls — optimization level barely matters.

## Known Issues

- **#36**: `va_arg` broken — blocks native `printf`
- **#37**: `LD A,IYH` without `+undocumented` — the elf2rel pipeline avoids this

---

## TODO later

### Native ELF pipeline (no elf2rel/sdldz80)

```bash
clang --target=z80 -Os -nostdinc -c program.c -o program.o
ld.lld --gc-sections -T cpm.ld cpm_crt0.o program.o z80_rt.a -o program.elf
llvm-objcopy -O binary program.elf PROGRAM.COM
```

Simpler (no elf2rel step) and produces the smallest binaries (81B hello).
Blocked for programs using IY by #37 (ld.lld doesn't have the issue but
the ELF putchar.asm reads from L which is correct — this path works for
programs that don't trigger the IYH codegen bug).

### z88dk stdlib integration (printf via z88dk)

```bash
clang --target=z80 -Os -S program.c -o program.s
clang2z88dk.sh program.s program.asm
zcc +cpm -compiler=sdcc program.asm -o PROGRAM -create-app
```

Uses z88dk's full libc (printf, stdio). Requires:
- `z88dk_compat.h` to map calling convention macros
- `cmain` → `main` rename (clang ignores sdcccall(0) on main)
- `clang2z88dk.sh` to strip ELF directives and fix label names
- Blocked by #37 for programs using IY (assembly path, not elf2rel)

### Native libc with printf

Requires fixing #36 (va_arg codegen bug). Headers exist in
`compiler-rt/lib/builtins/z80/include/` and `printf.c` is written
but produces blank output due to va_arg returning wrong values.
