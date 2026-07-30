# Dhrystone 2.1 three-way comparison + sdcccall(1) investigation (2026-07-11)

Adds a third lane to the z88dk Dhrystone 2.1 benchmark suite so we can compare
the two register-passing calling conventions on the same source, same clib:

- **llvmz80 -O2**        — ravn/llvm-z80 clang, z80 register ABI
- **sdcc --sdcccall 1**  — upstream SDCC z80 register ABI (via a PATH shim)
- **sdcc --sdcccall 0**  — z88dk default (stack frames via IX)

All paths use the **same** z88dk classic clib (both call `LDIR` for the struct
copy; libc entry conventions are header-pinned, see below), so differences are
codegen only, not library.

## Results (@ 4 MHz, 20000 runs, all self-validated 20/20)

- **llvmz80 -O2**: 8461 cyc/run, **0.2691 DMIPS**
- **sdcc --sdcccall 1 -SO3**: 11044 cyc/run, **0.2061 DMIPS**
- **sdcc --sdcccall 0 -SO3**: 12158 cyc/run, **0.1872 DMIPS**

Register-passing (sdcccall 1 vs 0) closes only ~1/3 of the gap
(12158 -> 11044). llvmz80 is still **30.5% faster** than sdcc-sdcccall(1).
So the llvmz80 win is NOT primarily the calling convention.

Suite lives in `support/benchmarks/dhrystone21/` (z88dk):
`llvmz80/`, `sdcccall1/`, `z88dk-classic/` lanes + `compare.sh` + `readme.md`.

## Why llvmz80 is faster (measured, not guessed)

Same clib (both LDIR). Two mechanisms found by diffing the emitted asm:
1. **Frames**: llvmz80 builds only 2 IX frames across dhry_2; the small
   per-iteration functions run frame-less with args in HL/DE and call libc via
   the register entry (`___strcpy`) instead of sdcc's stack `_strcpy_callee`.
2. **Inlining/loop deletion**: LLVM inlines `Func_1` into `Func_2` and deletes
   the trivial one-trip loop in `Func_2`. Static instruction count is
   essentially equal (686 lines each) — the win is **dynamic**, not size.

## sdcccall(1) mechanism (how to actually use it)

- Upstream SDCC's z80 **default is sdcccall(1)** (register args, return 8-bit in
  A / 16-bit in DE / 32-bit HLDE). z88dk overrides back to **0** because its
  precompiled clib + crt0 are built version 0
  (`src/sdcc-build/doc/sdccman.lyx:49862`).
- `z88dk-zsdcc` accepts `--sdcccall N` (SPACE form only; `--sdcccall=1` ->
  "error 194: Bad integer argument"). It warns "296: non-default sdcccall
  specified, but default stdlib or crt0" — the ABI-mismatch guard.
- **User code sdcccall(1) is ABI-safe against the version-0 clib**: z88dk headers
  pin each libc function's convention (`include/string.h`: `strcpy ... __smallc
  __z88dk_callee`), so cross-calls use the header-declared convention regardless
  of the compiler's `--sdcccall` default. crt0 ignores `main()`'s return.
  Confirmed by 20/20 Dhrystone self-validation on the sdcccall(1) build.
- Demo `int add(int,int)`: sdcccall(1) = `add hl,de / ex de,hl / ret` (register);
  sdcccall(0) = 13-instruction IX stack frame.

## The zcc `--sdcccall` forwarding gap (ravn/z88dk#24)

`zcc` does NOT forward `--sdcccall` to the underlying `zsdcc` codegen. Verified
(z88dk `bin/`): `-Cc--sdcccall=1`, `--sdcccall=1`, and split `-Cc` forms all
leave codegen stack-based; `-v` shows no `--sdcccall` on the zsdcc line. The only
way to get sdcccall(1) through the full zcc pipeline today is a PATH shim that
`exec`s the real `z88dk-zsdcc` with `--sdcccall 1` prepended — which is exactly
what the `sdcccall1/` lane Makefile generates at build time (from
`$(shell command -v z88dk-zsdcc)`, no hardcoded path committed).

Repro (`add.c` = `int add(int a,int b){return a+b;}`):
```
zcc +test -compiler=sdcc -SO3 -Cc--sdcccall=1 -a add.c -o f.asm
# _add stays IX/stack-based; identical to the no-flag build
```

## The dhry.h spurious %f float-pull (ravn/z88dk#25)

`support/benchmarks/dhrystone21/dhry.h:420` emits
`#pragma output CLIB_OPT_PRINTF = 0x04000601` under bare `#ifdef PRINTF`. Bit
`0x04000000` selects the **%f** printf converter, which drags the float-math lib
into the link. But `%f` is only ever printed under `#ifdef TIMEFUNC`
(`dhry_1.c:242,258,260`) — a `-DPRINTF`-without-`-DTIMEFUNC` build never calls
`%f`. On a compiler with no float-math lib (llvmz80) the link then fails:
`undefined symbol: asm_fpclassify / __dtoa_base10 / __dtoa_digits`.
Workaround (used by both the llvmz80 and sdcccall1 verify builds):
`-pragma-define:CLIB_OPT_PRINTF=0x00000601` (%d|%s|%c). Proper fix: gate the
`0x04000000` bit on `TIMEFUNC`, or drop it for the PRINTF-only path.

## llvmz80 codegen observations (hypotheses -> ravn/llvm-z80#257)

Not filed as bugs; candidate improvements only, unverified as wins:
- `Proc_7` builds an IX frame (`push ix / ld ix,0 / add ix,sp`) solely to read
  one incoming stack argument at `(ix+4)`, then tears it down (`ld sp,ix`). A
  leaf that only reads an overflow arg might address it SP-relative without a
  frame. The callee-clean epilogue itself (`pop hl / ex (sp),hl / ret`, 2-byte
  clean) is about as good as z80 gets and is NOT suboptimal.

## Environment (build artifacts only; not committed)

`PATH=.../z88dk/bin`, `ZCCCFG=.../z88dk/lib/config`, ntvcm `.../ntvcm/ntvcm`,
llvmz80 clang `.../llvm-z80/build-macos/bin/clang`. macOS bash 3.2 has no
`declare -A`; `compare.sh` uses an indexed `"cycles|label"` array. `-notemp` is
sdcc-only (invalid for the clang path).
