# Research: a standard whole-program (CP/M) benchmark suite for sweeping all compilers

**User question (2026-08-04):** the previous compiler sweep ran *without the
standard runtime library* (freestanding), so it cannot measure the efficiency
of *full CP/M programs*. Is there a **standard test suite of whole C programs
(preferably under CP/M)** that makes sense to sweep across all compilers?

**Short answer:** yes — **stdcbench** is the best-fit single standard, and the
workspace already has most of the surrounding infrastructure (a whole-program
benchmark tree with a working multi-compiler Dhrystone harness). Recommendation
and evidence below.

---

## 1. What the prior sweep was vs. what is needed

- **Prior sweep = freestanding, no clib.** The AES corpus / DCC quad
  (`sieve`, `e`, `ttt`, `tm`) and the `compiler-comparison-corpus` were compiled
  either freestanding (llc + no runtime) or as tiny kernels. They measure
  *codegen* but not the cost of a real CP/M program that links `printf`,
  `malloc`, string/`math` — i.e. the standard C runtime library, whose ABI and
  implementation differ per compiler and dominate real .COM size/speed.
  ("dcc" = David Lee's DCC C compiler, https://davidly.github.io/dcc/ — the
  `sieve/e/ttt/tm` programs are its sample set.)
- **Needed = whole ANSI C programs that exercise the stdlib, built the same way
  an end user builds** (`zcc +cpm` → real `.COM`), timed cycle-accurately
  (`z88dk-ticks`) and/or sized, then swept across every compiler.

## 2. What ALREADY exists in the workspace (don't rebuild it)

`z88dk/support/benchmarks/` is already a whole-program benchmark tree, built
with the real z88dk clib and timed with `z88dk-ticks`:

- `dhrystone21/` — **Dhrystone 2.1** with a working **multi-compiler sweep**
  `compare.sh` (3 lanes: llvmz80 `-O2`, `sdcc --sdcccall 1 -SO3`,
  `sdcc --sdcccall 0 -SO3`; `make verify` correctness-checks via the 20
  self-validation values). This is the template harness to generalize.
- `coremark10/` — **CoreMark 1.0** — README only (see licensing caveat below).
- `whetstone/`, plus the **Computer Language Benchmarks Game** whole programs:
  `binary-trees`, `fannkuch`, `fasta`, `mandelbrot`, `n-body`, `spectral-norm`,
  `pi`, `sieve`, `sorting`, `paranoia`, `sprintf`, `sscanf`.

So the *harness pattern* (per-lane Makefile → `z88dk-ticks` → markdown table)
and a set of CLBG whole programs are in place; what is missing is a single,
recognised, cross-compiler-fair **standard** score, and lanes for the other
compilers (sccz80, dcc).

## 3. Standard suites surveyed (verified 2026-08-04)

| Suite | Fit for "whole CP/M program, all compilers" | Notes |
|-------|---------------------------------------------|-------|
| **stdcbench** | **Best** | "A Benchmark for Small Systems" (ACM DOI 10.1145/3207719.3207726). Self-contained, single-file portable ANSI C; self-checking (checksum → cannot be gamed by dead-code elimination the way Dhrystone can); self-timing; exercises a broad slice of the C standard library. Authored/maintained by **Philipp Klaus Krause — the SDCC maintainer**, so it is the de-facto standard in the Z80/SDCC world and is designed to run under CP/M. SourceForge: https://sourceforge.net/projects/stdcbench/ (open). |
| **CoreMark** (EEMBC) | Strong, but licensing friction | Modern industry standard; single portable C; explicitly anti-gaming. **License forbids redistribution** — must be downloaded from EEMBC after agreeing to terms, so it can be used internally but not committed to the repo (workspace has only a placeholder README). Keep any local copy gitignored. |
| **Dhrystone 2.1** | Good for continuity | Already present AND already sweeping 3 lanes here. Small and partly gameable — LLVM already deletes its trivial one-trip loop / inlines `Func_1` (see `dhrystone21/readme.md`), which flatters clang. Keep as a legacy reference, not the sole metric. |
| **ansibench** (github nfinit/ansibench) | Convenience bundle | One repo packaging CoreMark + Dhrystone + Whetstone + LINPACK in portable ANSI C. Useful if we want a single upstream to vendor from. LINPACK/Whetstone are FP-heavy → they mainly stress the soft-float library (relevant now that llvmz80 uses IEEE-754 soft-float, but a different axis than integer codegen). |
| **CLBG programs** (already vendored) | Good breadth for **code size** | binary-trees/fannkuch/fasta/mandelbrot/n-body/spectral-norm exercise printf/malloc/float in realistic shapes; best complement for a *size* sweep of full programs. |

## 4. Recommendation

1. **Adopt `stdcbench` as the primary cross-compiler standard.** It is the one
   suite that is simultaneously: whole-program, stdlib-exercising, CP/M-targeted,
   self-validating, self-timing, single-file (so every compiler links its own
   clib the identical way), and already the accepted yardstick in the SDCC/Z80
   community. It directly answers "efficiency of full CP/M programs" and yields a
   single comparable number per compiler.
2. **Keep Dhrystone 2.1 as a legacy continuity metric** (harness already exists;
   just add lanes). Note its gameability caveat when reporting.
3. **Use CoreMark for an industry-recognised cross-check** *if* the EEMBC license
   terms are acceptable for our (internal, non-redistributed) use — keep the
   downloaded source gitignored, never committed.
4. **For a code-SIZE sweep of realistic programs, reuse the existing CLBG
   corpus** in `z88dk/support/benchmarks/`.

## 5. Compiler lanes to sweep (generalise `dhrystone21/compare.sh`)

- `zcc +cpm -compiler=llvmz80` (ravn/llvm-z80 clang) — `-O2` and `-Os`.
- `zsdcc --sdcccall 1` and `--sdcccall 0` (already lanes here).
- `sccz80` (z88dk classic front end) — add a lane.
- `dcc` (davidly) — add a lane for parity with the prior sweep.
- (native HI-TECH C v3.09 under an emulator is possible but off our toolchain;
  optional.)

Each lane: build the `.COM` the same way, correctness-gate on stdcbench's own
checksum, time with `z88dk-ticks`, and record BOTH cycles and `.COM` size.

## 6. Open items / caveats to verify before committing to it

- **Does stdcbench fit and link under `zcc +cpm` for each compiler?** It targets
  "small systems" but Z80 has 64 KB; confirm the single .c builds and links the
  full clib on llvmz80, sccz80 and zsdcc (it needs a working `printf`/`malloc`;
  llvmz80's newlib FILE\* + classic paths are green as of this session, and its
  IEEE soft-float closure is packaged — but stdcbench's FP sections will exercise
  soft-float, so size/speed there reflects the soft-float lib, by design).
- **stdcbench license** — SourceForge project; confirm the license permits
  vendoring the source into our tree before committing it (CoreMark definitely
  does not).
- This note is a **plan/recommendation only**; wiring the harness + lanes is a
  follow-up that needs an explicit go-ahead (new corpus/vendored dependency).
