# z88dk-ticks I/O Reference

z88dk-ticks is a headless Z80 emulator for automated testing. Three I/O mechanisms:

## 1. Port I/O (`-iochar X`)

Simplest. `OUT (X),A` writes character to stdout. `IN A,(X)` reads from stdin.

```
z88dk-ticks -mz80 -iochar 1 program.bin
```

Used by test-gen for both clang and SDCC tests. Clang's `putchar()` in z80_rt.a uses port 1.

## 2. z88dk Trap (`$ED $FE`)

z88dk-native mechanism used by the `+test` target. Register A selects the command:

| Cmd | Name | Interface |
|-----|------|-----------|
| 0 | CMD_EXIT | Exit, L=return code |
| 1 | CMD_PRINTCHAR | Print char in HL |
| 2 | CMD_READKEY | Read key → HL |
| 3 | CMD_POLLKEY | Poll key → HL (-1 if none) |
| 4-11 | File I/O | open/close/read/write/seek/stat on host filesystem |
| 30-31 | Time | Unix time, millisecond clock |

Not standard Z80 — `$ED $FE` is an illegal opcode intercepted by ticks.

## 3. CP/M BDOS (address 5 → JP 7)

Partial CP/M 2.2 emulation. Programs `CALL 5` with function in C register:

| Fn | Name | Status |
|----|------|--------|
| 01 | C_READ (char + echo) | Yes |
| 02 | C_WRITE (E=char) | Yes |
| 06 | C_RAWIO | Yes |
| 09 | C_WRITESTR (DE=$ string) | Yes |
| 0B | C_STAT (console status) | Yes |
| 0C | S_BDOSVER (returns 0x22) | Yes |
| 0F | F_OPEN (FCB) | Yes (host filesystem) |
| 10 | F_CLOSE | Yes |
| 13 | F_DELETE | Yes |
| 16 | F_MAKE (create) | Yes |
| 17 | F_RENAME | Yes |
| 1A | F_DMAOFF (set DMA addr) | Yes |
| 21 | F_READRAND (128B records) | Yes |
| 22 | F_WRITERAND | Yes |
| 23 | F_SIZE | Yes |
| 14 | F_READ (sequential) | **No** |
| 15 | F_WRITE (sequential) | **No** |
| 11/12 | F_SFIRST/F_SNEXT (search) | **No** |

Enough for console I/O and random-access file operations. Not enough for
programs using sequential file I/O or directory searches.

Files are opened on the host filesystem in the current working directory.
Address 5 must contain `JP 7` for the hook to work.

## test-gen usage

test-gen uses mechanism 1 (`-iochar 1`) for both compilers:
- **Clang**: `putchar()` from z80_rt.a does `OUT (1),A`
- **SDCC**: generated code embeds `__asm__("out (0x01),a")`
- **T-states**: measured without `-iochar` (clean numeric output)
- **Correctness**: clang checks DE (`-end` at `_halt`), SDCC checks HL (`-end 3`)

Source: `z88dk/src/ticks/hook_cpm.c`, `z88dk/src/ticks/hook.c`, `z88dk/src/ticks/cmds.h`
