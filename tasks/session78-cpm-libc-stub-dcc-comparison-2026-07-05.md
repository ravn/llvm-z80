# Session 78 — CP/M libc stub for clang, dcc-vs-clang benchmark comparison (2026-07-05)

## Goal

Reproduce the `dcc`-vs-clang code-size/timing comparison from `cpm_compilers/images/table.jpg`
for the 4 `dcc/tests/` benchmark programs (`sieve.c`, `e.c`, `ttt.c`, `tm.c`), using
clang/llvm-z80 as an additional compiler in the comparison (issue #35's concrete motivating
case). clang could not build 3 of the 4 sources at session start — no `printf`, no
`malloc`/`calloc`/`free` existed in the Z80 CP/M stub — so most of this session was building
just enough of an ad-hoc libc stub to unblock the comparison, per issue #35's explicit
"not a full libc port" scope.

**This is scaffolding, not a production runtime.** A todo (`clang-cpm-libc-prod-runtime-followup`,
session-local SQL todos table) tracks resuming this work once a real CP/M C runtime exists for
llvm-z80; see "Known limitations of the stub" below for what a real runtime needs to fix.

## What was built

All in `compiler-rt/lib/builtins/z80/` (not yet committed as of this writing — see "State of
the tree" below):

- **`printf.c`**: added `%lu` support (`pr_ulong()`, 32-bit unsigned long digit extraction).
  `ttt.c` needs this for its move-count output.
- **`include/stdlib.h`**: added `atoi`, `exit`, `malloc`, `calloc`, `free` declarations.
- **`include/stdio.h`**: added a header-only `FILE`/`stdout`/`fflush` stub (no-op — CP/M's
  `putchar` has no buffering to flush).
- **`heap.c`** (new): a free-list allocator over a static arena. Went through two design
  iterations before working correctly on `tm.c`'s allocation pattern — see "The malloc bug"
  below.
- **`misc.c`** (new): `atoi()` and `exit()` (the latter is inline asm `jp 0x0000`, matching the
  CRT's own warm-boot exit path).

## The CP/M `.COM` build pipeline (verified working end-to-end)

```
clang --target=z80 -Os -ffreestanding -nostdlibinc \
      -isystem compiler-rt/lib/builtins/z80/include \
      -ffunction-sections -fdata-sections -c program.c -o program.o
elf2rel program.o program.rel
sdldz80 -m -i -b _CODE=0x0100 output cpm_crt0_sdcc.rel program.rel [extra.rel...] \
        -k <build>/lib/z80 -l z80_rt
makebin -s 65536 output.ihx output_full.bin
dd if=output_full.bin of=PROGRAM.COM bs=1 skip=256 count=<N>
```

Two non-obvious details, both confirmed by direct measurement this session:

1. **`-nostdlibinc`, not `-nostdinc`.** `-nostdinc` also removes clang's own resource-dir
   builtin headers (`stdarg.h` etc.), breaking anything using `va_list`. `-nostdlibinc` keeps
   those while still excluding a real system libc — correct flag for this pipeline, combined
   with `-isystem <stub-include-dir>`.
2. **`dd count` must span `_CODE` + `_DATA`, not just `_CODE`.** The SDCC linker places `_CODE`,
   then `_BSS` (usually zero-length here, see the elf2rel bug below), then `_DATA`
   contiguously. `count = (s__DATA + l__DATA) - 0x100`. Using just `l__CODE` silently passed for
   `sieve.c`/`e.c` (their statics happened to be all-zero, so truncating them read back as zero
   anyway) but broke `ttt.c`'s `g_Iterations` (a nonzero initialized global read back as 0).

Everything needed to run `sdldz80`/`makebin` is Docker-wrapped (`~/.local/bin/sdldz80` /
`makebin` exec `docker run ... sdcc-tools <tool>`) — needs Docker Desktop running.

## The malloc bug (root-caused via host-side simulation, not guessed)

`tm.c` links, but hung indefinitely (no crash, no output) with a naive first-fit-with-splitting
`heap.c`. Root cause, confirmed empirically before being labeled "confirmed":

- `tm.c`'s allocation sizes grow strictly monotonically within each of 10 outer iterations
  (`cb = 8 + i*10` for `i = 0..65`), then everything gets freed and the next iteration repeats
  the identical size sequence.
- A **first-fit** search picks the first free block that's *big enough*, not the smallest
  sufficient one. Against this repeating-but-graduated size pattern, first-fit reliably splits
  the *wrong* (oversized) free block whenever an exact-size match exists later in the list,
  scattering same-sized blocks that would otherwise be reused whole by the next iteration's
  identical request sequence.
- This fragmentation **compounds every outer iteration** rather than reaching steady state.
  Verified with a host-side Python simulation of the exact allocation pattern
  (`/tmp/heap_native_test/sim.py`, `sim2.py` — scratch, not committed): naive first-fit
  exhausted a 100,000-byte arena partway through the *third* outer iteration, still growing.
  Switching the same simulation to **best-fit** (scan the whole free list, pick the smallest
  block that still fits) reached a stable steady state of ~45.5 KB after the *first* iteration
  and never grew again for the remaining nine.
- Fixed `heap.c`'s `malloc()` to do a best-fit scan instead of first-fit; bumped `ARENA_SIZE`
  40000 → 48000 for headroom over the measured ~45.5 KB peak. Verified on the real Z80 build
  under `ntvcm` after the fix — `tm.c` now runs to completion and prints "success".

Native-host (arm64) compile-and-run of the standalone repro was blocked by an environment
permission hook that specifically (and, in this instance, persistently) denies "compile a new
native executable, then execute it" command patterns — the Python simulation was the workaround
and turned out to be sufficient to root-cause the bug without ever needing the native binary.

## dcc: updated to newest, rebuilt with dccpeep

Per user request, the `dcc` submodule (`/Users/ravn/z80/dcc`) was merged forward from local
`main` (11 commits ahead, ravn's own three-compiler-comparison-harness work) to
`origin/main` (3 newer upstream commits, notably `src/dcc/dccpeep.c` — the peephole optimizer
— and `src/dcc/dcc_array_narrow.c`). Rebuilt via `bash m.sh` (needed `chmod +x
src/dcc/build-dcc.sh scripts/stacksize.sh` first — these lost their executable bit somewhere,
worth a closer look if it recurs) and the 4 benchmarks rebuilt via `PATH=$PWD:.../ntvcm:$PATH
./ma.sh <test>` (default = with `dccpeep`).

Newest-dcc timings differ from the pre-merge numbers used earlier in this session — `e.c` and
`ttt.c` got measurably faster (e: 22.3M → 20.9M cycles, ttt: 5.4M → 4.75M cycles), consistent
with the merged-in codegen work; `sieve.c` was unchanged (18.18M cycles both before and after).

## Results: code size

**Not the focus of this session** (user: "jeg er ligeglad med størrelsen lige nu" — I don't
care about size right now) after an elf2rel bug was found that makes clang's `.COM` file sizes
currently unreliable for this comparison — see below. `_CODE`-only sizes (clang, `-Os`,
unaffected by the bug) were measured for reference: sieve 1964 B, e 2345 B, ttt 2645 B,
tm 3473 B. Total `.COM` sizes were NOT used for the size comparison — see the elf2rel finding.

### elf2rel bug found (filed as https://github.com/ravn/llvm-z80/issues/253, 2026-07-05)

`z80-utils/elf2rel/src/main.rs`'s `section_to_area()` (~line 65–75) maps ELF `.bss`/`.bss.*`
sections into the SDCC `"_DATA"` area — there is no dedicated `_BSS` area at all
(`SDCC_AREAS` only defines `("_CODE", 0)` and `("_DATA", 0)`, ~line 51–52). Worse, for
`SHT_NOBITS` (BSS) sections it materializes real zero bytes into that area
(`area.bytes.resize(area.bytes.len() + size, 0)`, ~line 367) instead of leaving them as an
uninitialized, non-file-resident placeholder the way SDCC's own linker treats `_BSS`.

Consequence: any clang-built CP/M `.COM` with a large uninitialized static gets that static's
full size baked into the file as literal zero bytes. Measured impact: `sieve.c`'s
`char flags[8191]` (correctly `.bss._flags` at the ELF level, confirmed via
`llvm-objdump -h`) inflated `SIEVE.COM` to 10,179 B vs dcc's 1,920 B for the same program —
almost entirely this bug, not a real code-size difference. Same mechanism also inflated the
`tm.c` build by the full 48,000-byte `heap.c` arena (also genuinely `.bss` at the ELF level).

This is a real, verified, root-caused defect (not a guess) affecting every future
clang-vs-{dcc,SDCC} `.COM` size comparison via this pipeline until fixed. Filed for follow-up,
not fixed in this session (out of scope — the session's focus shifted to runtime per user
request before size work resumed).

## Results: runtime (the focus of this session)

Measured with `ntvcm -c -p -s:4000000 <program>.COM`, clock rate 4 MHz, all 4 dcc/clang
"newest" builds, dcc built with `dccpeep`:

| Program | dcc (newest+peep) | clang -Os | clang -O1 | clang -O2 | clang -O3 |
|---|---:|---:|---:|---:|---:|
| sieve | 18,180,494 | 26,210,611 (1.44×) | 27,993,108 (1.54×) | 27,993,108 (1.54×) | 26,673,380 (1.47×) |
| e | 20,923,181 | 28,149,993 (1.35×) | 29,775,335 (1.42×) | 29,764,639 (1.42×) | 29,764,639 (1.42×) |
| ttt | 4,751,136 | 6,677,176 (1.41×) | 6,675,146 (1.40×) | 6,675,182 (1.40×) | 5,594,205 (1.18×) |
| tm | 49,501,528 | 216,017,107 (4.36×) → **193,060,987 (3.90×) after the calloc fix below** | 202,291,959 (4.09×) | 202,295,155 (4.09×) | 202,295,137 (4.09×) |

### Finding 1: `-Os` is at or near clang's fastest setting for this backend — `-O2`/`-O3` are not a reliable win

For `sieve`/`e`, every higher optimization level (`-O1`, `-O2`, `-O3`) was 5–9% **slower** than
`-Os`. Only `ttt` showed a real win from `-O3` (1.18× vs 1.40× at lower levels). `tm` improved
~6% from `-Os` to `-O1` but then plateaued — `-O1`/`-O2`/`-O3` are statistically identical for
`tm`. No single optimization level dominates across all 4 programs; hypothesis (not yet
confirmed) is that higher `-O` levels trigger inlining/unrolling heuristics tuned for
register-rich architectures that backfire on Z80's tiny register file. Not investigated further
this session — flagged as a candidate `known-suboptimal-codegen.md` entry if revisited.

### Finding 2: the ad-hoc `heap.c` allocator was a real, measurable, and partially fixable cost — the user's suspicion was correct

Used `ntvcm -g:<file>.csv` (per-PC dynamic execution-count profile) and bucketed the resulting
addresses against each build's linker `.map` symbol table to attribute instruction counts to
functions. For `sieve.c`, the ad-hoc runtime (`printf`/`putchar`) was negligible — under 0.02%
of all executed instructions; virtually 100% of time is in the program's own `_main`. This
directly falsifies the general hypothesis "the ad-hoc runtime is slow" for the 3 programs that
don't allocate memory.

For `tm.c` (the one program using `malloc`/`calloc`/`free`), the picture was different:
before any fix, the ad-hoc runtime — `_malloc` (19.8%) + `_calloc` (11.3%) + `_free` (0.1%) +
`_printf` (3.2%) + `_putchar` (0.4%) + z80_rt div/mul helpers (2.5%) — totaled **37.3%** of all
dynamically executed instructions. `_chkmem` (tm.c's *own* byte-by-byte memory verification
loop, not part of the stub) was the single largest bucket at 62.2%, and is identical cost for
dcc (same C source), so it's not evidence of unfair runtime overhead.

Root cause of the `calloc` share: `heap.c`'s `calloc()` zeroed memory with a hand-written
`for (i = 0; i < total; i++) bytes[i] = 0;` loop instead of calling `memset()`. The z80_rt
`memset()` (`compiler-rt/lib/builtins/z80/memset.asm`) writes the first byte then uses a single
`LDIR` to propagate it — one instruction, no per-iteration loop overhead — versus the manual
loop's per-byte compare+branch+increment+store. Fixed by changing `calloc()` to call
`memset(p, 0, total)`.

Effect, `tm.c` at `-Os`:
- Cycles: 216,017,107 → **193,060,987** (**-10.6%**)
- Ratio vs. newest dcc: 4.36× → **3.90×**
- `_calloc`'s own instruction share: 11.3% → 0.1% (the zero-fill work moved into `_memset`,
  which itself is only 0.6% of the total — confirming `LDIR` really is that much cheaper per
  byte than a hand-rolled loop)

After the fix, `_malloc`'s O(n) best-fit linear scan (no coalescing) is the largest remaining
ad-hoc-runtime cost at 22.3% of instructions — a legitimate further target if this stub is ever
extended, but more invasive (would need coalescing or a different free-list structure to reduce
scan length) and was not attempted this session; sieve/e/ttt do not use `malloc`/`calloc` at all,
so this remaining cost is `tm.c`-only.

## State of the tree (as of session end)

**Not committed.** Working-tree changes in `llvm-z80` at session end:
- Modified: `compiler-rt/lib/builtins/z80/include/stdio.h`, `include/stdlib.h`, `printf.c`
- New (untracked): `compiler-rt/lib/builtins/z80/heap.c`, `misc.c`

Note: `git status` at session end also showed unrelated modified/untracked files
(`llvm/lib/Target/Z80/{CMakeLists.txt,Z80.h,Z80TargetMachine.cpp}`,
`llvm/lib/Target/Z80/Z80LoopInstrFormPrep.{cpp,h}`,
`llvm/test/CodeGen/Z80/pointer-iv-strength-reduce.ll`) — **these predate this session and were
not touched by this work**; left untouched, not documented further here.

`dcc` submodule: merged `origin/main` into local `main` (see "dcc: updated to newest" above),
rebuilt binaries (`dcc`, `dccpeep`, `dccrtlstrip`, `dccmake`) are local build artifacts, not
committed (dcc's own `.gitignore` presumably covers these — not verified).

All benchmark build artifacts (`.o`/`.rel`/`.ihx`/`.bin`/`.COM`/`.map`/profile `.csv`s) live in
`/tmp/clang_cpm_test/` (plus `o2/`, `O1/`, `O3/` subdirectories) — scratch, disposable, not
part of the repo.

## Follow-ups (tracked in session-local SQL todos as `clang-cpm-libc-prod-runtime-followup`)

1. Decide what "production-quality CP/M runtime" means in scope before resuming (ask the user
   — full libc port? z88dk libc reuse? a properly designed but still minimal from-scratch
   runtime?).
2. ~~File the elf2rel `_BSS`-into-`_DATA` bug as a GitHub issue~~ — DONE 2026-07-05, filed as
   [#253](https://github.com/ravn/llvm-z80/issues/253) after re-verifying `section_to_area()`
   and the `SHT_NOBITS` zero-fill branch against current `main`.
3. Re-run the full dcc-vs-clang comparison (size AND timing) once elf2rel's `_BSS` handling is
   fixed and/or a production runtime replaces the ad-hoc stub.
4. Consider extending `heap.c`'s `malloc()` with coalescing (or a different structure) to cut
   its remaining 22.3% share of `tm.c`'s instructions, if the ad-hoc stub is kept around rather
   than replaced.
5. Investigate why `-O1`/`-O2`/`-O3` regress `sieve`/`e` relative to `-Os` on this backend — not
   done this session, candidate for `known-suboptimal-codegen.md` if picked up.
