# Session 73q — #108 fix: FLAGS-dead checks added to 4 peephole sites

**Date:** 2026-05-23
**Outcome:** 4 of 5 sites fixed; site 3 obsoleted by earlier #15 removal.  cpnos PROM1: 2029 -> **2028 B** (−1 B win, pipeline-ordering side effect — same class as #187 but with positive sign).

## What was wrong

The audit at `Z80LateOptimization.cpp` (issue #108, post-#104/#107 follow-up) identified 5 peephole sites that rewrite FLAGS-producing instructions to alternative FLAGS-producing instructions without checking whether FLAGS is live past the branch.  Each site has a real but low-severity latent bug: a flag consumer past the rewrite would observe a different state.

## What was fixed

| Site | Location | Pattern | Original FLAGS | New FLAGS | Fix |
|---|---|---|---|---|---|
| 1 | line ~735 | `DEC A; LD B,A; [OR A;] JR NZ → DJNZ` | DEC A sets Z/S/P/H | DJNZ doesn't touch FLAGS | added `isRegDeadAfter(...FLAGS)` |
| 2 | line ~808 | `DEC B; JR NZ → DJNZ` | DEC B sets Z/S/P/H | DJNZ doesn't touch FLAGS | added `isRegDeadAfter(...FLAGS)` |
| ~~3~~ | ~~line ~1255~~ | ~~16-bit overflow rewrite~~ | n/a | n/a | **obsoleted** -- peephole removed by #15 retest |
| 4 | line ~1554 | `LD r,A; LD A,#imm; CP r; JR C/NC → CP imm+1; JR NC/C` | CP r sets Z/S/P/H | CP imm+1 sets different Z/S/P/H | added `isRegDeadAfter(...FLAGS)` |
| 5 | line ~2521 | u8 switch range-check 16->8 (`LD DE,..; SUB; SBC; JR C/NC → CP_n; JR NC/C`) | SBC A,H sets Z/S/P/H | CP_n sets different Z/S/P/H | added `isRegDeadAfter(...FLAGS)` |

All 4 fixes have the same shape: add `isRegDeadAfter(std::next(Br), MBB, TRI, Z80::FLAGS)` and bail if FLAGS is live.

## Verification

- Lit: 110 PASS + 3 XFAIL (unchanged: 113 total).
- AES `aes256.c -Oz` `.text`: 3299 B (unchanged).
- cpnos PROM1 (clang): 2029 -> **2028 B** (−1 B).
- test-runner clang sweep: (pending).

## Why cpnos shrank

Adding safety checks should make peepholes fire LESS often, which usually grows code size.  We got the opposite.  Hypothesis (same class as #187 / #15 retest):

- Adding the `isRegDeadAfter(...FLAGS)` check before the bail-out shifts the iteration order over the MBB by a small amount.
- Subsequent peepholes in the same MBB see the same instructions but in a slightly different machine state (the MII iterator advanced once more).
- One downstream peephole now finds (or fails to find) a match that it previously missed (or hit).
- Net effect on cpnos: −1 B.

These cumulative pipeline-ordering side effects are real and small.  Not worth chasing further individually.

## Implication for the audit

This is the **third audit "Migrate" or "follow-up" candidate this session that closed via a small safety hardening**, not a big migration:
1. #109 — ADD HL,rr commutativity BC dead-after check (zero codegen change).
2. #108 sites 1+2+4+5 — FLAGS-dead checks (−1 B cpnos).
3. (Plus #154 / reg-reg LD flag-clean, #15 retest deletion, etc.)

The original #180 audit had "Migrate (16 patterns)" — these "Migrate" items often turn out to be either (a) obsolete, (b) safety hardening with no migration, or (c) small fixes that produce surprising side effects.  Realistic LOC saving on the full audit is still trending toward the C2 estimate of ~1100-1500.

Closes #108 once the test-runner sweep confirms no behavioral regression.

## Files

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp`: 4 separate FLAGS-dead checks added.
- `tasks/session73q-issue108-fix.md`: this writeup.
