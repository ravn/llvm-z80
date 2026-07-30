# dcc-vs-clang Comparison: Size + Timing (2026-07-09)

Clean re-run of the session 78 dcc-vs-clang benchmark comparison, using:
- Fixed `elf2rel` (issue #253 closed 2026-07-09): `.bss` now routes to a real `_BSS`
  area instead of materialising zero bytes into `_DATA`.
- Fixed `cpm_crt0_sdcc.asm`: added `.globl s__BSS`/`l__BSS` for native `sdasz80`
  compatibility; added `.area _DATA` before `.area _BSS` to force `CODE→DATA→BSS`
  section ordering (avoids BSS gap inflating .COM files for programs with initialized globals).
- Tightened `z80_rt.a` memcpy/memset/memchr (`adcb0c2e2e73`, 2026-07-09).
- Native `sdldz80`/`makebin` (no Docker, `~/.local/bin/` shims updated).

## Build

```
clang --target=z80 -$OPT -ffreestanding -nostdlibinc \
      -isystem compiler-rt/lib/builtins/z80/include \
      -ffunction-sections -fdata-sections \
      -c <src>.c -o <src>.o
elf2rel <src>.o <src>.rel
sdldz80 -m -i -b _CODE=0x0100 out \
    cpm_crt0_sdcc.rel <src>.rel [heap.rel misc.rel printf.rel] \
    -k build-macos/lib/z80 -l z80_rt
makebin -s 65536 out.ihx out_full.bin
dd ... bs=1 skip=256 count=$(python3 extract_com_size.py out.map)
```

`.COM` size extraction: `s__DATA + l__DATA - 0x100` if initialized globals exist
(DATA follows CODE in file); else `s__BSS - 0x100` (BSS not in file); else `l__CODE`.

dcc built with `dccpeep` (newest, post-2026-07-05 merge, see session 78).

## Size table (bytes)

| program | dcc  | clang -Os | clang -O1 | clang -O2 | clang -O3 |
|---------|------|-----------|-----------|-----------|-----------|
| sieve   | 1920 | 1964 (+2%)  | 1981      | 1981      | 2934      |
| e       | 2304 | 2345 (+2%)  | 2430      | 2423      | 2432      |
| ttt     | 3456 | 2792 (−19%) | 2806      | 2815      | 4105      |
| tm      | 4224 | 3455 (−18%) | 3490      | 3482      | 3496      |

Notes:
- `sieve`/`e`: clang is 2% larger at `-Os` (small but real; not a pipeline artifact —
  `sieve.c` has no initialized globals and the fix reduces its `.COM` from 10 179 B
  (inflated by the elf2rel bug) to 1964 B, matching the real `_CODE` size).
- `ttt`/`tm`: clang is 18–19% **smaller** than dcc at `-Os`/`-O1`/`-O2`. These two
  programs are branch-heavy (ttt = minimax evaluator, tm = allocator test with
  inner `chkmem` loop). Clang's branch-folding and dead-code elimination outpace dcc
  on this shape.
- `-O3` unrolls loops aggressively on Z80 (`sieve` +53%, `ttt` +47%) — counter-productive
  on a register-starved 8-bit target. `-Os` is the right default.

## Timing table (Z80 cycles, ntvcm -p at full speed)

| program | dcc        | clang -Os        | clang -O1        | clang -O2        | clang -O3        |
|---------|------------|------------------|------------------|------------------|------------------|
| sieve   | 18,180,494 | 26,251,719 (1.44×) | 28,034,236 (1.54×) | 28,034,236 (1.54×) | 26,714,498 (1.47×) |
| e       | 20,923,181 | 28,152,176 (1.35×) | 29,777,528 (1.42×) | 29,766,832 (1.42×) | 29,766,832 (1.42×) |
| ttt     |  4,751,136 |  6,677,394 (1.41×) |  6,675,384 (1.41×) |  6,675,415 (1.41×) |  5,594,438 (1.18×) |
| tm      | 49,501,528 |180,149,702 (3.64×) |161,909,354 (3.27×) |161,912,550 (3.27×) |161,912,559 (3.27×) |

### Speed findings

**Finding 1: `-Os` is at or near the fastest for integer-intensive benchmarks on Z80.**
For `sieve` and `e`, `-Os` (1.44× / 1.35× slower than dcc) is actually 5–9% FASTER
than `-O1`/`-O2` (1.54× / 1.42×). Higher optimization levels are counter-productive:
they trigger inlining/unrolling heuristics tuned for register-rich architectures that
backfire on Z80's tiny register file (causing additional BSS spills). `-O3` matches
`-Os` on `sieve` (unrolling accidentally reduces loop overhead) but is neutral-to-
worse on `e`.

This is a candidate for investigation and a `known-suboptimal-codegen.md` entry: the
root cause is guessed to be `-O1`'s aggressive inlining increasing register pressure
above the Z80 register file capacity → additional BSS spills that each require an
A-shuttle load/store. Not root-caused this session; deferred to item 2.

**Finding 2: clang -Os is 1.35–1.54× slower than dcc on integer loops (`sieve`/`e`/`ttt`).**
Consistent with session 78. The gap is not from the runtime library (profiling session 78
showed < 0.02% of instructions in `printf`/`putchar` for `sieve`); it's from the codegen
itself. Primary suspect: Z80 BSS-spill-via-A-shuttle pattern (M2 in known-suboptimal-codegen).

**Finding 3: `tm` (3.27–3.64×) is dominated by `malloc`'s O(n) best-fit scan.**
Session 78 profiled this: `_malloc` accounts for ~22% of instructions.
The `-Os` is 11% slower than `-O1`/`-O2`/`-O3` on `tm`, all of which are essentially
identical (3.27×). Suspected cause: `-Os` emits the best-fit scan loop differently
(possibly unoptimized inner loop in `heap.c`) — not investigated.

**Finding 4: `ttt` at `-O3` is only 1.18× slower than dcc** (vs 1.41× at lower opts).
This is the only case where a higher optimization level meaningfully closes the gap.
`ttt.c` is minimax search: `-O3` inlines small evaluation helpers that `-Os` calls,
reducing call/return overhead which is significant for a depth-limited search.

## Comparison vs session 78

Session 78 timing was identical for `sieve`/`e`/`ttt` (same code). `tm` improved:
- Session 78 (after calloc fix): 193,060,987 cycles
- This session: 180,149,702 cycles (-7%)
- Cause: the z80_rt.a `memset` was tightened to the `pop-iy`/`ldir` idiom (`adcb0c2e2e73`),
  making `calloc`'s memset call ~10% faster.

## Infrastructure fixes this session

1. **`cpm_crt0_sdcc.asm`**: added `.globl s__BSS`/`l__BSS` for native sdasz80 V02.00
   compatibility; added `.area _DATA` before `.area _BSS` to ensure `CODE→DATA→BSS`
   ordering. Without the area ordering, sdldz80 placed `_BSS` before `_DATA` (following
   the first declaration order from `cpm_crt0_sdcc.rel`), causing `tm.COM` to balloon
   by 48 000 B (the heap arena BSS gap) and `ttt.COM` to exclude its initialized globals.

2. **`~/.local/bin/sdldz80` and `makebin` shims**: repointed to native arm64 binaries
   at `/Users/ravn/z80/z88dk/src/sdcc-build/bin/`. No Docker needed for CP/M linking.

3. **`elf2rel` binary rebuilt** from current source (previous binary was from 2026-07-05,
   before the BSS fix commit `284afd1ab88b` on 2026-07-08). Binary installed to
   `~/.local/bin/elf2rel`.

## Remaining gaps

See `tasks/known-suboptimal-codegen.md` M2 entry (BSS-spill-via-A pattern).
The `-Os`-beats-`-O1`/`-O2` finding for sieve/e is a new candidate entry — root cause
investigation is item 2 (next session or later in this session).


