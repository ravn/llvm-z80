# Tightening z80_rt.a mem helpers to the pop-iy idiom (2026-07-08)

## Summary

Moved rcbios's (and cpnos's) hand-rolled runtime helpers into the compiler's
own `z80_rt.a` archive, and — the substantive compiler change — **tightened the
archive's `memcpy`/`memset`/`memchr` from an IX-frame prologue to the compact
`pop iy` / `jp (iy)` idiom**.  This closed most of the size gap that made the
archive worse than the hand-rolled copies, so the "helpers live in the compiler"
cleanup ships at a small, justified cost instead of +45 B.

## The change

`compiler-rt/lib/builtins/z80/{memcpy,memset,memchr}.asm` read their stack size
argument via a full IX frame:

```asm
push ix ; ld ix,#0 ; add ix,sp ; ld c,4(ix) ; ld b,5(ix) ...
pop ix ; pop bc ; inc sp ; inc sp ; push bc ; ret   ; explicit callee cleanup
```

This is needlessly conservative.  **IY is caller-saved** on Z80
(`Z80CallingConv.td`: `Z80_CSR = CalleeSavedRegs<(add IX)>` — only IX is
callee-saved), and the register allocator reserves IY by default, so a leaf
runtime trampoline may clobber it freely.  Rewritten to:

```asm
_memcpy:
    pop iy          ; return address (IY caller-saved)
    pop bc          ; size (callee-cleanup of the stack arg)
    ex de,hl        ; HL=src, DE=dest
    push de         ; save ORIGINAL dest for the return value
    ld a,b ; or c ; jr z,_memcpy_done
    ldir
_memcpy_done:
    pop de          ; DE = original dest
    jp (iy)
```

Also dropped the `.globl` on the internal labels (`_memcpy_done` etc.) so the
intra-function `jr` stays 2 bytes instead of being promoted to a 3-byte
relocated `jp`.

Sizes: **memcpy 31→14 B, memset 39→23 B, memchr ~41→~26 B.**

Correctness note: the new archive `memcpy` returns the *original* dest (via
`push de`/`pop de`).  The hand-rolled rcbios `runtime.s` version returned
`dest+n` — a latent bug (unused there).  So the archive version is not just
smaller-than-before, it is **more correct** than the hand-rolled one it
replaces.

`___umodqi3` was left untouched: its 15 B O(1) shift-divide is faster than a
hand-rolled O(n) subtract loop and the +4 B is a deliberate speed trade.

Both dialects assemble: `llvm-mc` (ELF `z80_rt.a`) and `sdasz80` (SDCC
`z80_rt.lib`) both accept `pop iy`/`jp (iy)`.

## Verification

- New runtime fixture `z80-utils/test-runner/testcases/clang/test_runtime_mem_helpers.c`:
  forces genuine `call _memcpy`/`_memset`/`_memchr` through a `volatile`
  function pointer (clang otherwise inlines memcpy/memset as LDIR even for a
  runtime size), and checks both the result AND the return value (the pop-iy
  contract).  **6/6 PASS at O0-Oz** (DE=0x003F).
- Full clang runtime suite: **906 pass / 0 fail / 258 skip** (1164 total).
- Lit `CodeGen/Z80`: **182 pass + 5 XFAIL** (unchanged).

## Downstream (rc700-gensmedet)

`make` link line adds `z80_rt.a` as the last input (archive pulls only
referenced members; `--gc-sections` drops the rest).  Hand-rolled
memcpy/memset/memchr/__call_iy deleted; only `lddr_copy` stays (no compiler-rt
equivalent).

Clean side-by-side sizes (MSIZE=56):

| build  | hand-rolled | archive (tightened) | delta |
|--------|-------------|---------------------|-------|
| rcbios | 5908 B      | 5918 B              | +10 B |
| cpnos  | 2014 B      | 2011 B              | −3 B  |

rcbios +10 B = ___umodqi3 +4 (faster shift-divide, kept) + memcpy +1 (correct
return) + ~5 archive/section overhead.  Verified: rcbios `make mame-test` boots
to `A>`, 77-track checksum sweep ERR=0; cpnos `cpnos-polypascal-test` PASS (PPAS
PRIMES to 29989 over CP/NET PIO, Q → E>).

## Aside: is the loop→modulo transform size-aware? (investigated, yes)

Prompted by "does clang account for -Oz when a loop can become `%`?".  Empirically:
- `while (a>=b) a-=b;` and an `idx`-wrap loop did **not** convert to `urem` at
  either -O2 or -Oz — clang kept the compact loop.
- Explicit `a % b` (runtime divisor) **is** size-gated: `-O2` inlines the
  shift-divide (fast, ~14 B, no call); `-Oz` emits `ld e,l; jp ___umodqi3`
  (3 B tail-call to the shared helper).

So the feared "loop replaced by a heavier modulo at -Oz" does not occur here;
the inline-vs-libcall decision for `urem` honors minsize.
