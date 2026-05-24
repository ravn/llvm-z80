# Session 73s — #180 peephole #11 retest (idempotent ALU collapse)

**Date:** 2026-05-24
**Predecessors:** `session73q-C2-audit-table-update.md` flagged peephole #11 as "Likely Keep" pending profiling.
**Outcome:** Peephole #11 (consecutive `AND_n`/`OR_n` with same imm) removed.  cpnos PROM1: 2029 → **2027 B (−2 B)**.

## What was wrong (or rather, what was unnecessary)

Peephole #11 at `Z80LateOptimization.cpp:1058-1075` collapsed consecutive `AND_n M` or `OR_n M` instructions with the same immediate, on the assumption that the second is redundant for idempotent AND/OR ops.

The pattern was noted as "Most common case: AND #1; AND #1 after SBC A,A; AND #1 sequences" — i.e., the SBC-A-A carry materialization followed by an idempotent mask.  At HEAD, the SBC chain is rewritten earlier by ISel (carry-mask + SBC A,A) and doesn't leave consecutive identical AND/OR pairs.

## Re-test

Disabled the peephole via `if (false) { ... }`, rebuilt, measured.
- Lit: 111 PASS + 3 XFAIL = 114 (unchanged).
- cpnos PROM1: 2029 → **2028 B** (−1 B; pipeline-ordering side effect with the dead-block still present).
- AES `aes256.c -Oz` `.text`: 3299 B (byte-identical).

Then DELETED the peephole entirely.
- cpnos PROM1: 2029 → **2027 B** (−2 B vs baseline; −1 B more vs disabled-but-present).
- AES + lit unchanged.

## Sweep

test-runner clang sweep with peephole removed: 990/689/38/56/207, zero per-test diff vs the pre-session-73s baseline.

## Methodology lesson

This is the **fourth peephole this session series** (after Z80NarrowIV, #15, and now #11) where the C2 "Re-test" methodology -- "disable, measure, decide" -- successfully identifies a dead peephole.  Each removal:
- 0 behavioral regressions.
- 1-2 B cpnos PROM1 SHRINK from removing the pipeline-ordering overhead.

The audit's original LOC ceiling of ~2300 has now been revised THREE times based on cumulative findings:
- Original: 2300 (16 Migrate candidates).
- C2 first revision: 1100-1500 (some candidates obsolete).
- C2-after-#15: 1500-1800 (more obsolete than expected).
- C2-after-#11 (this commit): unchanged trend.

Pattern: when LSR / TTI / ISel canonicalization improves, peepholes that targeted older shapes lose their input.  Worth a periodic sweep of the "Migrate" column to re-classify obsolete candidates.

## Closes

Closes the #11 retest portion of #180.  Peephole gone, ~20 LOC removed.

## Files

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp` -- peephole #11 block removed, replaced with a one-line comment pointing here.
- `tasks/session73s-issue11-retest.md` -- this writeup.
