# Runner-exposed regressions, 2026-05-03

After landing #103 the test-runner clang suite actually executes tests
end-to-end for the first time.  A full opt-level matrix (`cargo run -- clang
-full`, six opt levels) ran 960 tests:

```
Total: 960  Pass: 655  Fail: 42  Fatal: 57  Skip: 206
```

Pass rate among non-skipped: 655 / 754 = 86.9%.

This file records what the failure buckets are so they can be triaged
without re-running the whole matrix.

## FATAL "no register value in emulator output" (50 cases)

All 50 are sub-tests of `test_90_edge` (25) and `test_91_edge_prom` (25)
at **-O0 only**.  Root cause is **test-infrastructure**, not codegen:
these are auto-generated synthetic stress tests that exceed Z80's 64 KB
flat memory limit at -O0.  Verified: `test_90_edge_0000.c` at -O0 links
to a 117 KB binary with `__bss_start = 0x1ca6d` (decimal 117357); the
emulator can't run it sensibly.  At -Os the same test fits in ~16 KB.

Action: (a) skip `test_90_edge_*` / `test_91_edge_prom_*` at -O0, or
(b) gate the suite on a binary-size check.  Either is a suite hygiene
fix, not a backend fix.  No issue filed; treat as known-bad until a
runner pass adds the guard.

## FATAL `'alloca.h' file not found` (6 cases)

`test_48_dynamic_alloca.c` `#include <alloca.h>` — that header does not
ship for `--target=z80`.  Pre-existing test bug, fatal at every opt
level.  No issue filed; suite-side fix is to either provide a minimal
`alloca.h` shim under `compiler-rt/include/` or replace the test's use
of alloca with a hand-rolled fixed-size buffer.

## FATAL "emulator timeout/30s" (1 case)

A single timeout in the matrix.  Not yet identified — the grep above
shows only the count.  Likely an infinite loop in some opt-level cell.

## FAIL — real codegen regressions

| Stem                           | Failing opts        | Notes                                            |
| ------------------------------ | ------------------- | ------------------------------------------------ |
| `test_18_short_circuit_goto`   | O1, O2, O3, Os, Oz  | **#104** — short-circuit `&&` / `\|\|` semantics |
| `test_27_array_2d`             | one opt level       | not yet inspected                                |
| `test_90_edge_*`               | various             | per-cell, runs at -Os but result wrong            |
| `test_91_edge_prom_*`          | various             | same shape as test_90                            |

The `test_90` / `test_91` FAILs (as opposed to the -O0 FATALs above)
are real cases where the test runs but returns the wrong value.  These
need per-test bisection — they may collapse to a small number of root
causes given the auto-generated structure.  Triage as a Phase 2 fold-in
once #28 + #63 land.

## What this changes

Before #103 the runner reported 0 PASS (all FATAL `_halt symbol not
found`).  Today's matrix is the **first** real correctness baseline
across the opt-level cube, and it's the dataset Phase 2 should iterate
against — including #28 + #63 (FastRegAlloc / -O0 spill).
