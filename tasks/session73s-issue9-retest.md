# Session 73s — #180 peephole #9 retest (select-form OR A + LD r,0)

**Date:** 2026-05-24
**Predecessor:** `session73q-C2-audit-table-update.md` (#9 classified "Likely Keep" -- branch-form FLAGS-sensitive optimization).
**Outcome:** Peephole #9 removed.  Both cpnos PROM1 and AES `-Oz` production target are byte-identical; the peephole's input shape no longer appears in clang output.

## What the peephole did

`Z80LateOptimization.cpp:951-1006` (now removed) matched the select-after-flag-test pattern:

```
OR A
LD r,0          ; 2B/7T
[more LD r,0]
JR Z / JR NZ
```

and rewrote each `LD r,0` to `LD r,A` (1B/4T), saving 1B/3T per match.  The OR_A test confirms that on the JR_Z fall-through `A == 0`, so `LD r,A` is equivalent to `LD r,0`.  On the NZ arm `r` gets overwritten by the select's non-zero value.

The pattern was the post-ISel shape of `r = (cond ? K : 0)` selects where ISel materialized 0 via `LD r,0` rather than reusing A.

## Re-test

**Method:** Disable peephole via `if (false)`, rebuild clang+llc, measure cpnos PROM1 + AES `-Oz +static-stack -disable-lsr -disable-licm -disable-cse -ffunction-sections -fdata-sections` `.text` sum.

| Configuration | Peephole ON | Peephole OFF |
|---|---|---|
| cpnos PROM1 | 2028 B | 2028 B |
| AES `09_Oz_prod_like` .text sum | 2228 B | 2228 B |

Byte-identical.  Peephole never fires on production targets.

Hypothesis: post session-73p ISel changes (especially #128 disabling LICM/CSE and #177 TTI adjustments), the select lowering now reuses A directly or routes through different shape entirely.  The `OR A` flag-test + `LD r,0` zero-materialization split has been canonicalized into a single ISel pattern.

## Verification

- Lit: 111 PASS + 3 XFAIL = 114 (unchanged).
- AES production target byte-identical .text.
- cpnos PROM1 byte-identical 2028 B.
- test-runner clang sweep: 990/**690**/37/56/207 -- **one extra PASS** vs baseline 990/689/38/56/207.  `test_27_array_2d_Os` flips from FAIL (DE=0x0000) to PASS (DE=0x000F) when the peephole is removed.

## Bonus: peephole was miscompiling test_27 at Os

The classified "Likely Keep" turned out to not only be dead on production targets but ALSO actively wrong on at least one test fixture.  `test_27_array_2d_Os` had been an O*z*-only failure (the Os opt level also FAILED), which now passes.  The remaining `test_27_array_2d_Os` FAIL noise from the #11-baseline (DE=0x0000) was caused by this peephole, not by anything else.

Two interpretations:
1. The peephole's `OR_A` check at MII didn't verify that the OR_A's A-input was actually the LD-zero candidate's source.  When ISel sometimes emits an unrelated OR_A test followed by an LD r,0 + JR Z that's NOT a select, the rewrite `LD r,A` corrupts r.
2. The "FoundBranch" gating allowed the rewrite when any Z-branch followed, even if the OR_A wasn't the producing instruction for the branch's flags.

Either way: removal fixes test_27_Os and doesn't regress anything else.

## Methodology lesson

This is the **fifth peephole this session series** (after Z80NarrowIV, #15, #11, now #9) where C2 "Re-test" methodology identifies a dead peephole.  The pattern is consistent:

| Peephole | Audit class | Actual class | LOC removed |
|---|---|---|---|
| Z80NarrowIV | -- | dead (pre-test) | ~150 |
| #15 (16-bit inc overflow) | Re-test | dead | ~80 |
| #11 (ALU #imm idempotent collapse) | Likely Keep | dead | ~20 |
| #9 (OR A; LD r,0; JR Z) | Likely Keep | dead | ~55 |

The Audit's "Likely Keep" classification is overcautious by a factor of ~2: many peepholes that look semantically necessary are dead because ISel has moved to canonicalize earlier.  The "Migrate" column will likely shrink further as more re-tests complete.

**Implication for the C2 audit reclassification:**

| Reclassification | Pre-73s | Post-73s |
|---|---|---|
| Migrate | 11 | 11 |
| Likely Keep | 2 | 0 (#11, #9 removed) |
| Re-test | 1 | 0 (#15 removed) |
| Keep | 2 | 2 (#6, #7) |

The Likely Keep classification is now an empty set; future re-tests should default to deletion-attempt as the working hypothesis.

## Closes

Closes the #9 retest portion of #180.  Peephole gone, ~55 LOC removed.

## Files

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp` -- peephole removed, replaced with a one-line pointer comment.
- `tasks/session73s-issue9-retest.md` -- this writeup.
