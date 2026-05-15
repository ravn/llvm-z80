# Session 73 — TruncInstCombine outside-user allowlist extensions (#165)

Date: 2026-05-15.  Continues session 72's trunc-narrowing push driven
by the AES-256 corpus in rc700-gensmedet.

## TL;DR

Closes `gf_log` 153 → **28 B** (−125 / 5.4×) and improves all 13 AES
corpus configs.  Runtime tstates drop 65M → 15M on baseline_Oz (4×
speedup).  Test-runner unchanged.  All 4 AES verifier cells PASS.

| llvm-z80 | Title | Class | Impact |
|---|---|---|---|
| **#165 (merged)** | Extend outside-user allowlist: icmp non-const + and-mask | Missed optimisation | `gf_log` 153 → 28 B; AES corpus all 13 improved (−26 to −129 B) |

## Background

End of session 72 left `gf_log` at 153 B vs ANSI's ~30 B (4.78×) as
the largest remaining AES residual addressable by trunc-narrowing.
The issue (`ravn/llvm-z80#165`) predicted that extending
`canNarrowIcmpThroughGraph` to non-constant Other operands would close
the gap.

Empirical finding partway through this session: the icmp extension
alone moves nothing.  `gf_log` has a **second** outside-graph blocker,
`%11 = and i16 %5, 128` — an and-mask user of the in-graph phi `%5`
that gates the bit-7 carry test in the GF(2^8) multiply-by-2
recurrence.  Until the and-mask outside-user is also allowlisted, the
phase-1 trunc root fails at `%5`'s outside-user check before even
reaching the icmp.

User picked "Extend #165 to also handle and-mask outside-user" via
AskUserQuestion after the empirical discovery.

## Implementation

Two parallel extensions to the outside-user check in
`getBestTruncatedType()`, with matching rewrite loops in
`ReduceExpressionGraph()`.

### Path A: ICmp non-const Other (companion of #160)

```cpp
// Shape (b): non-constant Other.  Require both narrowness witness AND
// single-use to avoid leaving the wide value live alongside a fresh
// trunc — the cost gate for #165 (v1, mirrors #164 boolean form).
if (!Other->hasOneUse())
  return false;
KnownBits Known = computeKnownBits(Other);
return Known.getMaxValue().getActiveBits() <= NarrowBits;
```

Rewrite: emit `Builder.CreateTrunc(OldOther, NewTy)` at the icmp site
when Other is non-constant.  InstCombine folds the trunc further when
Other is itself `(and W, mask)`.

`canNarrowIcmpThroughGraph` becomes a member function (needs
`computeKnownBits`).  Predicate-safety helper hoisted to
`isIcmpPredicateNarrowSafe`.

### Path B: And-mask outside-user

```cpp
if (auto *BO = dyn_cast<BinaryOperator>(UI)) {
  if (BO->getOpcode() == Instruction::And &&
      BO != AndMaskParentSkip) {
    ConstantInt *C = ...;  // match (and graph_value, Const)
    if (C && C->getValue().getActiveBits() <= NarrowBits) {
      PendingAndMasks.push_back(BO);
      continue;
    }
  }
}
```

Rewrite:

```cpp
NewAnd = Builder.CreateAnd(NewGraphOp, NewC);   // and NarrowTy
NewZext = Builder.CreateZExt(NewAnd, OrigTy);   // back to OrigTy
AndI->replaceAllUsesWith(NewZext);
AndI->eraseFromParent();
```

The zext keeps downstream consumers' types stable — they didn't
participate in narrowing, so they keep OrigTy.  InstCombine
canonicalises `(icmp eq (zext X), 0) → (icmp eq X, 0)` etc. later in
the pipeline.

### Two bugs caught during implementation

**Phase-2 parent And erased twice.**  Phase 2's synthetic trunc
root is created from a parent `And` that phase 2 itself replaces.
Without guarding, my outside-user check added that same `And` to
`PendingAndMasks`; phase 2 then RAUW+erased it before my rewrite
loop ran → dangling pointer.  Fixed by adding a transient member
`AndMaskParentSkip` set by phase 2 around its `getBestTruncatedType`
call; the check skips `BO == AndMaskParentSkip`.

**Phi-erase ordering vs outside-user rewrites.**  Pre-existing
ordering bug exposed by #165's first real phi+and-mask combo: the
phi-erase loop RAUW'd in-graph phis with `PoisonValue` BEFORE the
`PendingIcmps` / new `PendingAndMasks` rewrite loops, leaving
outside-graph users (icmp / and-mask) with poison operands.  My
`cast<ConstantInt>(poison)` then crashed.  Reordered so rewrite
loops run first, phi-erase last.

This bug was latent for #160 (the pre-existing icmp path) because
existing #160 tests don't combine in-graph phis with outside-graph
icmp users — but the reordering is correct for both paths and
sound by construction.

## Empirical: 13/13 configs improved

| Config | post-#162-p2 | post-#165 | Δ |
|---|---:|---:|---:|
| 01_baseline_Oz | 4330 | **4205** | **−125** |
| 02_Os | 4605 | **4480** | **−125** |
| 03_O3 | 12688 | **12559** | **−129** |
| 04_O2 | 8654 | **8529** | **−125** |
| 05_Oz_static_stack | 2911 | **2855** | **−56** |
| 06_Oz_no_licm_cse | 3867 | **3815** | **−52** |
| 07_Oz_no_lsr | 4696 | **4571** | **−125** |
| 08_Oz_gc_sections | 4310 | **4185** | **−125** |
| **09_Oz_prod_like** | 2721 | **2695** | **−26** |
| 10_Oz_no_licm_cse_lsr | 4223 | **4171** | **−52** |
| 11_Oz_no_licm_cse_gc | 3847 | **3795** | **−52** |
| 12_Oz_no_omit_fp | 3691 | **3606** | **−85** |
| 13_Oz_no_omit_fp_no_licm_cse_gc | 3373 | **3328** | **−45** |

Per-function (01_baseline_Oz):

| Function | post-#162-p2 | post-#165 | Δ |
|---|---:|---:|---:|
| `gf_log` | 153 | **28** | **−125 (5.4×)** |
| (rest of corpus unchanged) | | | 0 |

z80-utils test-runner: **685 / 42 / 56 / 207** — unchanged.

## Runtime tstates (big surprise)

The session-72 baseline reported the full 13-config sweep with tstates
ranging 22M (best, prod_like) to 65M (worst, baseline_Oz) for AES
encrypt+decrypt.  Post-#165, the entire sweep collapses to ~15M:

| Config | tstates post-#162-p2 | tstates post-#165 |
|---|---:|---:|
| 01_baseline_Oz | 65,832,659 | **15,742,481** (−76%) |
| 09_Oz_prod_like | 22,551,771 | **15,201,006** (−33%) |

Cause: `gf_log` is called from `gf_mulinv`, which is called for every
AES SubBytes lookup (16 per round × 14 rounds = 224 per block).  The
i16 phi loop iterated up to 256 times searching for the log, each
iteration doing 16-bit shifts and masking.  Narrowing the loop to i8
removes both the shift width and the post-shift masking — and removes
the 16-bit phi backedge entirely.

## What's still gap on the AES corpus

| Function | bytes | gap | class |
|---|---:|---:|---|
| `aes_mc_inv` | 460 | ~3× vs zsdcc | BSS-spill cluster (#89 / #27) |
| `aes_mixColumns` | 300 | ~2× | similar |
| `aes_subBytes` | 127 | small | residual |

None are trunc-narrowing problems; further AES gap closure goes
through the regalloc cluster.

## Issues touched

| # | Title | State at session start | State at session end |
|---|---|---|---|
| #165 | icmp outside-user → narrowable non-constant | OPEN (filed session 72) | **CLOSED** (path A + B landed) |

## Files changed (llvm-z80)

```
llvm/lib/Transforms/AggressiveInstCombine/AggressiveInstCombineInternal.h (+21)
llvm/lib/Transforms/AggressiveInstCombine/TruncInstCombine.cpp           (+218, -66)
llvm/test/Transforms/AggressiveInstCombine/trunc-narrow-icmp-narrowable-operand.ll (+87, NEW)
```

One commit on `main`:
- `c48824ce135f` — #165 outside-user extensions

Merged --no-ff.

## Files changed (rc700-gensmedet)

```
tasks/aes256-corpus/baselines.md          (+72 lines: post-#165 section)
tasks/aes256-corpus/clang-flag-sweep.md   (regenerated)
tasks/aes256-corpus/sdcc-flag-sweep.md    (regenerated)
tasks/timeline.md                          (+47 lines: session 73 entry)
```

## Lessons / patterns

1. **Issue predictions can over-fit one shape.**  #165 was filed with
   only the icmp blocker in mind.  The first real measurement showed
   the and-mask blocker dominates; the issue's "30 LOC, closes gf_log"
   prediction needed a second 30 LOC path to actually materialise.
   Re-measure baselines and pivot scope mid-implementation when the
   premise turns out incomplete.

2. **Latent ordering bugs surface when new combinations are added.**
   The phi-erase-before-rewrite-loops bug existed in #160's icmp path
   but no #160 test combined an in-graph phi with an outside-graph
   icmp user.  Adding a new path that hit the combination exposed it.
   Reorder rewrites first, then phi-erase — sound for both paths.

3. **Tstates count too.**  The headline win was bytes, but a 4×
   speedup on AES inner loop is the bigger production impact.  Always
   record tstates alongside byte deltas in sweep tables — they're
   already collected by the harness.

4. **Empirical bisect beats reasoning on bugs near use-after-free.**
   The first segfault on caller_trunc looked impossible from
   IR-level reasoning (PendingAndMasks should have been empty).
   Bisecting with a `(void)NarrowBits` early return + reproduce
   minimal isolated case quickly revealed: phase-2 was the
   trigger AND phi-cleanup was the failure site — two issues
   stacked.

## Cumulative gains (sessions 69 → 73) on AES corpus

| Config | session 69 start | session 73 end | Δ |
|---|---:|---:|---:|
| `01_baseline_Oz` | 5114 | **4205** | **−909 (−17.8%)** |
| `09_Oz_prod_like` | 3604 (zsdcc) | **2695** | clang beats zsdcc by **909 B** |

The `09_Oz_prod_like` knob, with `+static-stack`, now beats zsdcc by
909 B on AES-class C code.
