# Session 73s — #180 peephole #2 retest (POP rr; PUSH rr elimination)

**Date:** 2026-05-24
**Predecessor:** `session73q-C2-audit-table-update.md` (#2 classified "Migrate" -- "stack-effect; legitimate MIR-DCE candidate. No FLAGS interaction.").
**Outcome:** Peephole #2 removed.  cpnos PROM1: 2028 -> **2027 B (-1 B)**; AES production target byte-identical.

## What the peephole did

`Z80LateOptimization.cpp:625-663` (now removed) matched a redundant `POP rr; PUSH rr` pair when the register pair was dead-after-PUSH.  The expected use case was SM83 boilerplate where each `LDHL SP,#` materialization needed a push/pop HL wrapper.

## Re-test

**Method:** Disable peephole via `if (false)`, rebuild clang+llc, measure cpnos PROM1 + AES `09_Oz_prod_like` `.text`.

| Configuration | Peephole ON | Peephole OFF / Removed |
|---|---|---|
| cpnos PROM1 | 2028 B | **2027 B (-1 B)** |
| AES `09_Oz_prod_like` .text sum | 2228 B | 2228 B |

cpnos *shrinks* by 1 B when the peephole is removed.  Same pipeline-ordering side effect as session 73p / 73r removals: removing a peephole that doesn't fire still shifts layout enough to recover 1 B elsewhere.

## Verification

- Lit: 111 PASS + 3 XFAIL = 114 (unchanged).
- AES production target byte-identical .text.
- cpnos PROM1 2028 -> 2027 B (-1 B).
- test-runner clang sweep: 990/690/37/56/207, zero per-test diff vs the #9-deletion baseline (sweep_p9_del.log).

## Methodology lesson

The Audit's "Migrate" classification for peephole #2 was already a signal that "removable, replace with generic MIR-DCE."  Re-test confirms the peephole's input shape doesn't reach the pass on Z80 production targets at HEAD -- so the "Migrate" step is not even needed: just delete.

This is the **sixth peephole this session series** identified as dead by C2 Re-test (Z80NarrowIV, #15, #11, #9, #2 + corpus #6's Re-test that confirmed Keep).

## Closes

Closes the #2 retest portion of #180.  Peephole gone, ~40 LOC removed.

## Files

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp` -- peephole removed, replaced with a one-line pointer comment.
- `tasks/session73s-issue2-retest.md` -- this writeup.
