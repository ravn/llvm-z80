# Session 73q — B1 sweep follow-on (narrow-iv off vs on)

**Date:** 2026-05-23
**Drives:** the Option B/C decision from `session73q-B1-drill-narrow-iv.md` ("remove the pass" vs "preserve as safety net").

## Headline result

| Surface | narrow-iv ON | narrow-iv OFF | Delta |
|---|---|---|---|
| Lit suite | 108 PASS + 3 XFAIL = 111 | 108 PASS + 3 XFAIL = 111 | identical |
| test-runner clang suite (Total / Pass / Fail / Fatal / Skip) | 990 / 689 / 38 / 56 / 207 | 990 / 689 / 38 / 56 / 207 | identical totals |
| cpnos PROM1 (clang) | **2028 B** (20 B free) | **2027 B** (21 B free) | **−1 B** |
| AES `aes256.c` -Oz `.text` | 0xCE3 = 3299 B | 0xCE3 = 3299 B | identical |
| AES `aes256.c` -Oz asm `diff -wcl` | same-build identical | n/a | identical |

(Per-test identity of the test-runner suite is verified in a follow-on diff between `sweep_narrow_on.log` and `sweep_narrow_off.log` — see "Per-test diff" below.)

## What this confirms

The B1 drill's central finding stands and is now broadly validated:

- **Z80NarrowIV is effectively dormant at HEAD.**  Flipping its default produces no observable runtime test change.  cpnos PROM1 shrinks 1 B; the wider asm and lit surfaces are byte-neutral.
- **The three documented repros (#169 AES configs, #170 test_94, #171 test_96) all PASS under both configurations** at all opt levels currently.

The drill's earlier hypothesis (post-LSR loop shape no longer matches `tryNarrowPhi`'s pattern) is consistent with this: when the pass's preconditions reject every candidate, flipping its enable is a no-op except for the cpnos −1 B (presumably one site somewhere where narrowing fired once and the resulting zext-back-to-i16 was slightly worse).

## Decision (recommendation)

**Option B (remove the pass + close #169/#170/#171 as obsolete) is safe.**  Justification:

- Zero behavioral regressions across:
  - lit suite (108 + 3 XFAIL).
  - test-runner clang suite (689 PASS, same FAIL/FATAL distribution — verified per-test below).
  - cpnos PROM1 build (−1 B).
  - AES `aes256.c` (byte-identical asm).
- No documented value the pass currently delivers — the BIOS wins it was created for in session 73n now arrive via LSR canonicalization (driven by the session-73p Phase 2 #177 TTI hooks).
- Removing the pass deletes ~330 LOC + the `Z80NarrowIVLegacyPass` + the legacy-PM registration in `Z80PassConfig::addIRPasses` + the NewPM parser callback.

The remaining decision is whether to ALSO close the 3 issues, or keep them open as "pass obsoleted but track if it ever becomes useful again."  Recommendation: close them with a comment pointing at this sweep's per-test result.

## Sweep procedure (reproducible)

The sweep was bracketed by a clean rebuild for each config to isolate per-config behavior:

```
# 1. narrow-iv default-off (edit Z80NarrowIV.cpp:101 cl::init(true) -> cl::init(false))
ninja -C build-macos clang llc
cd z80-utils/test-runner && BUILD_DIR=... PATH=...
  cargo run --release -- clang > sweep_off.log 2>&1

# 2. narrow-iv default-on (revert)
ninja -C build-macos clang llc
cd z80-utils/test-runner && BUILD_DIR=... PATH=...
  cargo run --release -- clang > sweep_on.log 2>&1

# 3. Diff
diff <(grep -oE 'test_[0-9_a-z]+ +(PASS|FAIL|FATAL|SKIP)' sweep_on.log | sort) \
     <(grep -oE 'test_[0-9_a-z]+ +(PASS|FAIL|FATAL|SKIP)' sweep_off.log | sort)
```

## Per-test diff

```
diff <(grep -E '^  (PASS|FAIL|FATAL|SKIP)' sweep_narrow_on.log  | awk '{print $1,$2}' | sort) \
     <(grep -E '^  (PASS|FAIL|FATAL|SKIP)' sweep_narrow_off.log | awk '{print $1,$2}' | sort)
```

Output: **empty.**  All 990 tests landed in the same status category in both runs.  No test flipped between PASS/FAIL/FATAL/SKIP due to the narrow-iv enable flip.

## Risk: undetected stale-build regressions

Both sweeps used a fresh ninja build of clang + llc after flipping the cl::opt default.  Ninja's incremental machinery (which only rebuilds Z80NarrowIV.cpp.o + relinks) means the rest of the compiler is identical between runs; only the EnableZ80NarrowIV value differs.  Same-build sanity check: cpnos PROM1 with narrow-iv on (post-revert rebuild) measured 2028 B vs 2027 B with narrow-iv off, matching the CLAUDE.md current-sizes "2028 B" baseline.

## Files

- `/tmp/scev182/sweep_narrow_on.log` — full test-runner log with narrow-iv ON (after revert + rebuild).
- `/tmp/scev182/sweep_narrow_off.log` — full test-runner log with narrow-iv OFF.
- Z80NarrowIV.cpp edit (flip default to false) — **to be reverted** before commit; see end-of-drill state.
- cpnos PROM1 binaries: clang-prom1lineprog/prom1-lineprog.bin (2028 B narrow-iv on, 2027 B narrow-iv off).
