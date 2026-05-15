# Session 72 — TruncInstCombine cost gate (#164) + per-callee body peek (#162 path 2)

Date: 2026-05-15.  Continues session 70/71 trunc-narrowing push driven
by the AES-256 corpus in rc700-gensmedet.

## TL;DR

Two AggressiveInstCombine extensions landed.  Together they shrink
`rj_sb_inv` K&R 156 → 36 B (matching the ANSI variant), move 11/13
AES corpus configs by 84–121 B, and bring the production
`09_Oz_prod_like` knob from 2806 → 2721 B.  Test-runner unchanged.

| llvm-z80 | Title | Class | Impact |
|---|---|---|---|
| **#163 + #164 phase 1 (merged)** | TTI cost gate + `(and X, MASK)` synthetic trunc root | Infrastructure | Z80-inert by design (boolean gate suppresses regressions); enables future cost-model work; lands the cost-model plumbing |
| **#162 path 2 (merged)** | Per-callee body peek for call-arg trunc-root injection | Missed optimisation | `rj_sb_inv` K&R 156 → 36 B (−120, 4.3×); AES corpus 11/13 improved 84–121 B |

## Background

End of session 71 ([session 71 summary](session71-truncinstcombine-cost-gate.md)
was never written; see `rc700-gensmedet/tasks/aes256-corpus/parked-162-context.md`),
both #163 (and-mask sink) and #162 Option 3 (call-boundary zeroext sink)
were attempted and ruled out:

- #163 without a use-count guard: corpus regressed +200–500 B / config
  because zext re-insertion at multi-use sites exceeded narrowing gain.
- #163 with `hasOneUse` guard: inert on the corpus.
- #162 Option 3 trusting `zeroext` attribute: **318 test-runner
  miscompiles** because `i16 zeroext` is an ABI signal, not a
  source-narrowness signal (`uint16_t` parameters carry zeroext too).

Three structural paths were left as future work, all multi-session:

1. Clang frontend K&R-narrow tag (new attribute).
2. **Per-callee body peek** (this session).
3. Tighter KnownBits for rotate-idiom DAGs.

Path 2 was empirically validated this session and turned out to be
exactly the right hammer for the `rj_sb_inv` and ~10 other AES corpus
chains.

## Phase 1: #164 cost gate + #163 and-mask sink

### Commit `d7c37aa6e928` ([merge `3d296f439645`](https://github.com/ravn/llvm-z80/pull/N/A))

`TargetTransformInfo` plumbed through `TruncInstCombine::TIC` ctor.
A new phase 2 of `run()` walks `(and X, MASK)` patterns where
`MASK = 2^M - 1` and `M` is a legal integer width, and for each
candidate synthesises a `trunc X to iM` root that the existing
narrowing engine consumes.

Cost gate (`#164` phase 1, boolean form):

```cpp
if (!And->hasOneUse() && !TTI.isZExtFree(NarrowTy, OrigTy))
  continue;
```

On Z80 (`isZExtFree=false`), only single-use ands fire — and those
were already measured inert in session 71.  On x86 family
(`isZExtFree=true` for narrow casts), the gate lets every match
through; upstream `trunc_multi_uses.ll` still PASSes.

### Side fix in this commit

`getMinBitWidth` had a latent UB: when the trunc's direct operand is
an Argument, line 258's `cast<Instruction>(Src)` reinterprets the
Argument pointer as an Instruction.  In release builds this stuffs a
garbage pointer into `InstInfoMap`, corrupting downstream narrowing
decisions.  Phase 1 callers never triggered this (their trunc operands
were always Instructions); phase 2's synthetic root reaches it when
the and's source is directly a function argument (`(and ARG, MASK)`).
Fixed by adding an Argument short-circuit mirroring the Constant
short-circuit at line 254.

### Empirical: Z80 inert by design

| Config | post-#157 | post-#164/163 | Δ |
|---|---:|---:|---:|
| All 13 | identical | identical | 0 |

z80-utils test-runner: 685 / 42 / 56 / 207 — unchanged.

## Phase 2: #162 path 2 (per-callee body peek)

### Commit `86eded565de7` ([merge `519aaaec4817`](https://github.com/ravn/llvm-z80/pull/N/A))

Mid-session experiment: manually inject `(zext (trunc i16 %arg to i8) to i16)`
at the `rj_sb_inv → gf_mulinv` call site.  Result: the existing
TruncInstCombine engine narrows the entire 17-op chain to i8 AND
InstCombine recognises 3× `llvm.fshl.i8`.  Exactly the ANSI form.

This proved that the missing piece wasn't tighter KnownBits — it was
getting the engine **engaged**.  The K&R chain ends in a call
argument (no trunc root); the existing engine never starts.

Implementation in TruncInstCombine.cpp phase 3: walk call sites; for
each direct, non-vararg, non-self callee, peek the callee's entry
block for either:

1. `trunc iW %argN to iM`, OR
2. `(and iW %argN, 2^M - 1)` (canonical form: InstCombine rewrites
   `(zext (trunc X to iM) to iW)` to this and).

Scan limited to the first 8 non-debug instructions (O(1) per call).
On a match, synthesise the trunc-zext bracket, swap the call's
argument to it, run the existing engine, rollback if narrowing fails.

### Critical ordering subtlety

The call's argument must be swapped to the synthetic `Zx` **before**
probing.  If we probe first, `ArgVal` has two users at probe time
(the original call + the synthetic Tr), and `getBestTruncatedType`
rejects the chain on the outside-graph multi-use check.  After the
swap, `ArgVal` is used only by Tr; the chain is single-rooted;
probing succeeds.  Rollback restores the original argument and erases
the bracket if probing fails.

Took one rebuild to notice (initial draft probed first, failed to
narrow).

### Empirical: 11/13 configs improved

| Config | post-#157 | post-path-2 | Δ |
|---|---:|---:|---:|
| 01_baseline_Oz | 4450 | 4330 | **−120** |
| 02_Os | 4725 | 4605 | **−120** |
| 03_O3 | 12688 | 12688 | 0 |
| 04_O2 | 8654 | 8654 | 0 |
| 05_Oz_static_stack | 2995 | 2911 | **−84** |
| 06_Oz_no_licm_cse | 3988 | 3867 | **−121** |
| 07_Oz_no_lsr | 4816 | 4696 | **−120** |
| 08_Oz_gc_sections | 4430 | 4310 | **−120** |
| 09_Oz_prod_like | **2806** | **2721** | **−85** |
| 10_Oz_no_licm_cse_lsr | 4344 | 4223 | **−121** |
| 11_Oz_no_licm_cse_gc | 3968 | 3847 | **−121** |
| 12_Oz_no_omit_fp | 3805 | 3691 | **−114** |
| 13_Oz_no_omit_fp_no_licm_cse_gc | **3488** | **3373** | **−115** |

Per-function (01_baseline_Oz):

| Function | pre | post | Δ |
|---|---:|---:|---:|
| `rj_sb_inv` | 156 | **36** | **−120** |
| `rj_xtime` | ~51 | 20 | −31 (estimate) |
| Everything else | unchanged | unchanged | 0 |

z80-utils test-runner: 685 / 42 / 56 / 207 — unchanged.

## What's still gap on the AES corpus

### gf_log (153 B vs ~30 B ANSI, 4.78×)

Path 2 narrows the chain feeding the call to `gf_log` in `gf_mulinv`,
but `gf_log`'s **own internal loop** (a phi-loop with `icmp eq i16 %6,
%2`) doesn't narrow.  The outside-graph user check requires the icmp's
non-graph operand to be a `ConstantInt`; here it's another narrowable
value (`%2 = and i16 %0, 255` — a sibling and-mask on the parameter).

Filed as **ravn/llvm-z80#165** ([AggressiveInstCombine] Extend icmp
outside-user to narrowable non-constant operands): extend
`canNarrowIcmpThroughGraph` to accept narrowable witnesses on the
non-graph operand.  Predicted impact: `gf_log` 153 → ~30 B.

### aes_mc_inv (460 B) + aes_mixColumns (236+ B)

BSS-spill cluster.  Not a call-arg-narrowing problem.  Tracked
separately (regalloc cluster #89 / #27).

## Issues touched

| # | Title | State at session start | State at session end |
|---|---|---|---|
| #162 | K&R u8 chain feeding K&R u8 call | OPEN (parked, 3 paths) | **CLOSED** (path 2 landed) |
| #163 | (and X, MASK) trunc-equivalent root | OPEN (Option 1 ruled out session 71) | **CLOSED** (infrastructure landed, inert-on-Z80 by design) |
| #164 | TruncInstCombine zext cost model | OPEN (filed session 71) | OPEN (phase 1 done; phase 2 byte-budget left) |
| #165 | icmp outside-user → narrowable non-constant | — | **NEW** (filed session 72; ~30 LOC; would close `gf_log` 4.78×) |

## Files changed (llvm-z80)

```
llvm/lib/Transforms/AggressiveInstCombine/AggressiveInstCombine.cpp     (+1, -1)
llvm/lib/Transforms/AggressiveInstCombine/AggressiveInstCombineInternal.h (+5, -1)
llvm/lib/Transforms/AggressiveInstCombine/TruncInstCombine.cpp           (+220, -3)
llvm/test/Transforms/AggressiveInstCombine/trunc-narrow-and-mask-root.ll (+64, NEW)
llvm/test/Transforms/AggressiveInstCombine/trunc-narrow-call-arg-via-callee-peek.ll (+125, NEW)
```

Two commits on `main`:
- `d7c37aa6e928` — #163 + #164 phase 1 (merge `3d296f439645`)
- `86eded565de7` — #162 path 2 (merge `519aaaec4817`)

## Files changed (rc700-gensmedet)

```
tasks/aes256-corpus/baselines.md          (+138 lines: 2 new HEAD rows)
tasks/aes256-corpus/parked-162-context.md (+33 lines: session 72 status)
```

Two commits on `main`:
- `94cba1a` — post-#164 baselines
- `3039699` — post-path-2 baselines + residual notes

## Lessons / patterns

1. **Pre-condition the IR before probing.** When synthesising a new
   value that the existing engine analyses, modify the IR users
   first; the engine's multi-use check counts current users at probe
   time, not just future users.  Cost: one explicit rollback path in
   the failure branch.

2. **Don't trust assumption-driven narrowing on multi-byte ABIs.**
   `zeroext` is an ABI signal, not a source-narrowness signal (session
   71 lesson, re-confirmed when designing path 2).  Sound narrowing
   needs a concrete witness: explicit trunc, explicit and-mask,
   range metadata, or KnownBits-via-computeKnownBits.  Path 2's
   callee-body peek is one such concrete witness.

3. **Empirical-first when picking between paths.**  Session 71's
   "path 3 (KnownBits) recommended" advice would have been wrong for
   this session — the chain never reaches the narrowing engine in
   the first place.  A 30-second manual-trunc-injection experiment
   immediately revealed that path 2 was the right hammer.  Multi-path
   issue branches should be re-evaluated with current state before
   re-engaging.

## Cumulative gains (sessions 69 → 72) on AES corpus

| Config | session 69 start | session 72 end | Δ |
|---|---:|---:|---:|
| `01_baseline_Oz` | 5114 | 4330 | **−784 (−15.3%)** |
| `09_Oz_prod_like` | 3604 (zsdcc) | 2721 | **clang beats zsdcc by 883 B** |

The `09_Oz_prod_like` knob, with `+static-stack`, brings clang ahead
of zsdcc by 883 B on AES-class C code.  Headline gap (`rj_sb_inv`
K&R 156 → 36 B) closed.
