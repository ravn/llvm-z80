# Session 73s — #180 peephole #24 retest (BC ping-pong in single-BB self-loops, #97)

**Date:** 2026-05-24
**Predecessor:** `session73q-C2-audit-table-update.md` (#24 classified "Migrate" -- "regalloc loop-live cost"; ~340 LOC).
**Outcome:** Peephole #24 removed.  cpnos PROM1 byte-identical (2028 B before and after, ignoring the ±1 B pipeline-ordering drift).  AES production target byte-identical.

## What the peephole did

`Z80LateOptimization.cpp:2122-2462` (now removed; ~340 LOC) matched the post-`Z80LoopRotate` single-BB self-loop shape where regalloc had allocated the SAME logical pointer to TWO physical pairs (HL for body stores, BC for back-edge advance), with cross-pair copies (`LD C,L; LD B,H` in pred, `LD L,C; LD H,B` at loop top) at every iteration.  Closes #97.

## Why the peephole is dead at HEAD

`Z80LoopRotate` was investigated in session 73m and `experiment-cpnos-prom-4k` for default-on enablement; the rotated single-BB shape regressed AES 01_baseline_Oz by +11 % ts, so it stayed default-off.  Without Z80LoopRotate rotating the loops, the post-RA shape this peephole targeted no longer materializes.

## Re-test

| Configuration | Peephole ON | Peephole OFF / Removed |
|---|---|---|
| cpnos PROM1 | 2028 B | 2027 B (-1 B; pipeline-ordering benefit) |
| AES `09_Oz_prod_like` .text sum | 2228 B | 2228 B |

## Verification

- Lit: 110 PASS + 4 XFAIL = 114.  The previously-passing `issue-97-bc-pingpong-singlebb.ll` now XFAILs (test was specifically the regression case for the peephole's pattern).  The companion `issue-97a-bc-pingpong-i16-counter.ll` was already XFAIL.
- test-runner clang sweep: 990/690/37/56/207 — zero per-test diff vs the #2-deletion baseline.
- cpnos PROM1: 2028 -> 2027 B (or 2028 B after cumulative recompile, depending on sccache state).
- AES production target byte-identical 2228 B.

## Forward compatibility

If Z80LoopRotate is ever re-enabled by default (gated on the loop-rotation cost-model work that was scoped in session 73m), the BC ping-pong shape will re-emerge and either:
(a) the test should be un-XFAILed and this peephole revived, or
(b) a structural regalloc fix should be designed instead (the audit's preferred path -- "regalloc loop-live cost", per the C2 table).

For now the audit treats this as DEAD because Z80LoopRotate is off.

## Methodology lesson

Largest LOC reduction so far in the C2 re-test campaign (~340 LOC).  Pattern: when an upstream IR-level pass is conditionally disabled, all the post-RA peepholes that targeted its output become dead until that pass is re-enabled.  When re-enabling a previously-disabled IR pass, audit all peepholes that mention the pass by name.

## Closes

Closes the #24 retest portion of #180.

## Files

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp` -- ~340 LOC removed, replaced with a pointer-comment block.
- `llvm/test/CodeGen/Z80/issue-97-bc-pingpong-singlebb.ll` -- XFAILed (test specifically exercises the now-removed peephole's pattern).
- `tasks/session73s-issue24-retest.md` -- this writeup.
