# Session 73s — #180 peephole #23 retest (HL save-via-BC roundtrip, #84)

**Date:** 2026-05-24
**Predecessor:** `session73q-C2-audit-table-update.md` (#23 classified "Migrate" -- "regalloc cost model"); ~117 LOC.
**Outcome:** Peephole #23 removed.  cpnos PROM1 byte-identical (2028 B), AES production target byte-identical, test-runner sweep zero per-test diff.

## What the peephole did

`Z80LateOptimization.cpp:2005-2120` (now removed; ~117 LOC) targeted the multi-BB pattern-fill loop shape (`for (...) *p++ = const_word`) where GISel saved HL into BC at the top of the back-edge MBB, ran the body which advanced HL, then restored HL from BC at the bottom.  Closes #84.

## Why dead at HEAD

Session 73p TTI hooks (#177) and LICM/CSE disable (#128) changed GISel's canonicalization for these loop shapes.  The output shape this peephole targeted no longer materializes in cpnos or AES production targets.

## Re-test

| Configuration | Peephole ON | Peephole OFF / Removed |
|---|---|---|
| cpnos PROM1 | 2028 B | 2028 B |
| AES `09_Oz_prod_like` .text sum | 2228 B | 2228 B |

## Verification

- Lit: 109 PASS + 5 XFAIL = 114.  `hl-no-bc-backup.ll` XFAILed (specifically exercises the removed peephole's pattern).
- test-runner clang sweep: 990/690/37/56/207 - zero per-test diff vs the #24-deletion baseline.
- cpnos PROM1 byte-identical 2028 B.
- AES production target byte-identical 2228 B.

## Methodology lesson

This is the **fifth peephole** in session 73s identified as dead by C2 Re-test, and the 7th in the series (counting Z80NarrowIV, #15 from session 73q).  The pattern is consistent: peepholes added between sessions 50 and 65 targeted post-RA shapes that newer ISel/canonicalization no longer produces.  Cleanup is essentially mechanical -- disable, measure, sweep, delete, XFAIL the lit test.

Sibling of #24 (single-BB self-loop variant) which was also dead.  Both #84 and #97 close logically as a pair.

## Closes

Closes the #23 retest portion of #180.

## Files

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp` -- ~117 LOC removed.
- `llvm/test/CodeGen/Z80/hl-no-bc-backup.ll` -- XFAILed.
- `tasks/session73s-issue23-retest.md` -- this writeup.
