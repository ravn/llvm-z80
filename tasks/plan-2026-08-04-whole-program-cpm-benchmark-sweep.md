# Plan: whole-program CP/M benchmark sweep across all compilers

**Goal (user):** replace the freestanding (no-clib) compiler sweep with one that
measures the efficiency of **full CP/M programs** — real `.COM` files that link
the standard runtime library — across **every** Z80/CP/M C compiler, using a
**standard** benchmark suite.

**Primary metric suite:** `stdcbench` (Philipp Klaus Krause / SDCC maintainer) —
see research note `research-2026-08-04-whole-program-cpm-benchmark-sweep.md` for
why. Complements: Dhrystone 2.1 (continuity), CoreMark (industry cross-check,
license-gated), CLBG corpus (code-size breadth).

**Harness model:** mirror the existing, working
`z88dk/support/benchmarks/dhrystone21/compare.sh` (per-lane Makefile →
`z88dk-ticks` cycle-accurate timing → markdown table, with a `make verify`
correctness gate). All builds use `zcc +cpm` so they are real CP/M programs
linking the full z88dk clib, exactly how an end user builds.

**Verification discipline (AGENTS.md):** capture the BASELINE (cycles + `.COM`
size) on the unmodified harness before any change; **correctness-gate every lane
on stdcbench's own self-check/checksum BEFORE trusting any timing** (a fast pass
with a wrong checksum is meaningless); record BOTH cycles and size; treat a
suspiciously fast score as a red flag (cross-check elapsed vs iteration count).

---

## Phase 0 — Decision gates (no code) — MUST clear before Phase 1

0.1 **stdcbench license.** VERIFY the actual license (likely GPL, NOT confirmed).
   - If it permits vendoring → commit the source under
     `z88dk/support/benchmarks/stdcbench/` with its own LICENSE file.
   - If not → download-on-demand + `.gitignore` the source (the CoreMark model
     already used in `coremark10/`), commit only the harness + a readme with the
     download URL. Either way the harness works; this only changes what is
     committed.
0.2 **Harness home** = `z88dk/support/benchmarks/stdcbench/` (sibling of
   `dhrystone21/`), so it inherits the same z88dk-ticks + zcc conventions.
0.3 **Metric definition.** Primary = **cycles over a FIXED iteration count**
   (deterministic, comparable across compilers via z88dk-ticks) — NOT wall-clock
   score, because the target has no real clock. Secondary = **`.COM` size**.
   Also record stdcbench's own per-module (`c90base`, `c90lib`) + final score for
   external comparability. Fixed iteration count chosen so the slowest lane
   finishes in a sane ticks budget.
0.4 **Lane set** (decide which compilers are in scope):
   - `llvmz80` (ravn/llvm-z80 clang) — `-O2` and `-Os`  [REQUIRED]
   - `zsdcc --sdcccall 1` and `--sdcccall 0`             [REQUIRED]
   - `sccz80`                                             [REQUIRED]
   - `dcc` (davidly) — parity with the prior sweep       [OPTIONAL, own toolchain]
   - native HI-TECH C v3.09 under emulator               [OPTIONAL, off-toolchain]

## Phase 1 — stdcbench, llvmz80 lane only (proves it builds + self-checks)

1.1 Obtain stdcbench source (per 0.1); drop into `stdcbench/` with a `readme.md`.
1.2 Build `stdcbench` as a real CP/M `.COM` via
   `zcc +cpm -compiler=llvmz80 -O2` (and `-Os`). RESOLVE open-item: does it link
   the full clib (`printf`/`malloc`/string) and fit in 64 KB? Disable any
   `c90lib` sub-benchmark only if the target genuinely lacks the feature (record
   which, so lanes stay comparable).
1.3 Run under `ntvcm`; confirm its **self-check passes** (correct module
   checksums) — this is the correctness gate, independent of timing.
1.4 **Baseline capture:** with a fixed iteration count, record cycles
   (`z88dk-ticks`) + `.COM` size for llvmz80 `-O2` and `-Os`. This is the
   "before" endpoint; no comparison is valid without it.

## Phase 2 — add the remaining compiler lanes

2.1 `zsdcc --sdcccall 1 -SO3` and `--sdcccall 0 -SO3` lanes — reuse the
   `dhrystone21/sdcccall1` PATH-shim trick (zcc filters `--sdcccall`).
2.2 `sccz80` lane.
2.3 `dcc` lane (optional) — separate toolchain; gate on its availability. If the
   toolchain is not present, SKIP with a documented reason (do NOT fake a
   result), same discipline as the test-suite skips.
2.4 Each lane: identical source + fixed iterations; **verify checksum first**,
   then record cycles + `.COM` size.

## Phase 3 — harness + reporting

3.1 `stdcbench/compare.sh` generalised from `dhrystone21/compare.sh`: builds every
   lane, gates correctness, prints a markdown table:
   `| compiler / conv | opt | c90base | c90lib | final | cycles | .COM bytes |`.
3.2 `stdcbench/Makefile` per-lane targets `verify` (checksum) + `benchmark`
   (emit cycles), mirroring dhrystone21.
3.3 Deterministic + reproducible: pin iteration count, z88dk rev, compiler revs,
   and clock assumption in the emitted table header.

## Phase 4 — complementary suites (breadth)

4.1 **Dhrystone 2.1** — already sweeps llvmz80 + 2 sdcc lanes; ADD `sccz80`
   (and `dcc`) lanes for parity. Keep, but annotate its gameability caveat.
4.2 **CoreMark** — document download-on-demand (EEMBC license forbids
   redistribution); `.gitignore` the source; add the same lanes. Report as the
   industry cross-check.
4.3 **CLBG code-size sweep** — reuse the existing whole programs
   (`binary-trees`, `fannkuch`, `fasta`, `mandelbrot`, `n-body`,
   `spectral-norm`, `pi`, `sieve`, `sorting`): build each per compiler, tabulate
   `.COM` size (and optionally cycles). This is the realistic full-program
   **size** axis that the freestanding sweep could not measure.

## Phase 5 — document + integrate

5.1 `z88dk/support/benchmarks/README.md`: what the sweep is, how to run
   (`sweep.sh`), how to read each metric, and the exact metric definitions.
5.2 A top-level `benchmarks/sweep.sh` that runs stdcbench + Dhrystone + CLBG-size
   and concatenates the tables into one report.
5.3 Record a results snapshot; update `llvm-z80` CLAUDE / `known-suboptimal-codegen`
   with any new whole-program findings. Update memory rule B25 (per-workload opt
   level) if stdcbench confirms/contradicts it.
5.4 CI: these are heavy — run **periodically/manually**, not per-commit. If any
   subset is cheap + deterministic, consider a size-only regression gate.

## Risks / open items (carried from research)

- **License (0.1)** — gates whether stdcbench source is committed or downloaded.
- **Timing without a clock** — mitigated by fixed-iteration + z88dk-ticks (0.3).
- **64 KB fit per compiler** — verify in 1.2; disable only genuinely-unsupported
  `c90lib` parts, and only symmetrically-documented.
- **FP sections** — stdcbench's (future/optional) FP work would exercise
  llvmz80's IEEE soft-float closure, a different axis than integer codegen; keep
  FP modules off for the first comparable run, add later as a separate table.
- **dcc / HI-TECH toolchains** — optional lanes; skip-with-reason if absent.

## Definition of done

- `stdcbench/compare.sh` produces a reproducible markdown table with ≥ the 4
  required lanes (llvmz80 -O2/-Os, zsdcc --sdcccall 0/1, sccz80), each
  checksum-verified, reporting cycles + `.COM` size.
- Dhrystone lanes extended; CLBG code-size table generated.
- `benchmarks/README.md` + top-level `sweep.sh` committed; results snapshot
  recorded. (Push only when asked.)
