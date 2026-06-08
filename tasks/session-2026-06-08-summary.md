# Session 2026-06-08 — summary

Continuation of the icmp-narrow soundness work started 2026-06-07 with
the user-directed revert.  Goal: reconsider how to redo the
optimization soundly.

## Outcomes (in order)

### v1 sound icmp-narrow gate — landed

Merged at llvm-z80 main `230ba09`.  Designed and shipped a sound
version of the #160/#165 outside-user icmp-narrowing through the
trunc-rooted graph:

- `KnownBits(GraphValue) <= NarrowBits` precondition added (the
  missing check that drove the 2026-06-07 revert).
- Signed-`samesign` predicates tightened to `NarrowBits − 1` so the
  sign bit stays clear at the narrow width.
- 21-function lit matrix `trunc-narrow-icmp-graph-side-soundness.ll`
  pins the boundary precisely: `unproven_*` rejected, `proven_*`
  narrowed, samesign `fits7` narrows but `fits8_not7` doesn't, etc.
- Runtime fixtures `test_220` / `test_221` / `test_222` PASS sound
  (DE = 0x0063) at every opt level.
- AVR cross-target soundness witness added at
  `tasks/upstream-5bug/avr/sound_gate_220_soundness.c`; simavr
  verdict SOUND.  Concrete generic-8-bit-platform evidence.

### v2 and-mask outside-user path — landed

Merged at llvm-z80 main `4a4b124`.  Added the and-mask half of the
original #165: `(and X, Const)` outside-graph users where Const fits
in the narrow width rewrite as `(zext (and Xnarrow, ConstTrunc) to
OrigTy)`.  Sound regardless of the in-graph operand's KnownBits.
`AndMaskParentSkip` mechanism added to avoid a crash where Phase 2
(#163 synthetic and-mask trunc root) and v2 (outside-user and-mask
gate) both tried to operate on the same AND.  Lit matrix grew by
4 functions.

### AES K&R speed regression investigation

The user asked "investigate why clang is slower than sdcc" — the
post-revert numbers showed clang `09_Oz_prod_like` at +51 % slower
than SDCC despite the size win.

First guess (v1's omission of the and-mask path) was disproved by v2
landing with AES byte-identical.

Second guess (cyclic-phi KnownBits limitation) was true but partial.

Real root cause pin-pointed via per-pass IR dump:

> **`CorrelatedValuePropagationPass` (CVP) strips the `(and i16 %atb, 255)`
> mask on Z80 but not on AVR.**  Z80 IR has the `if (z & 0x80)`
> as a branch; AVR as a select.  Branch form gives CVP/LVI the
> per-edge ranges to prove the cyclic phi narrow; CVP then folds the
> AND away as redundant.  Select form doesn't, so the mask survives.
> Downstream, `AggressiveInstCombine` Phase 2 needs the mask as its
> trigger pattern — Z80 has nothing to match on.

The select-vs-branch divergence is driven by
`Z80TTIImpl::getPredictableBranchThreshold = 0`
(`llvm/lib/Target/Z80/Z80TargetTransformInfo.cpp:62-64`, set since
zlfn's initial backend commit `31997a6`).

### Option (2) attempt — discarded

Per user direction "2", wired `LazyValueAnalysis` through
`AggressiveInstCombinePass` and extended the icmp gate with a
KnownBits-then-LVI fallback.  Built clean, lit clean, AES
byte-identical to main.  Instrumented stderr print confirmed LVI
returns full-set on the post-CVP `gf_log` phi — the constraint LVI
used to prove narrowness was the very mask CVP stripped.  Once
stripped, LVI can't recover the bound from loop structure alone.

Branch discarded.  Findings doc updated with the negative result and
the conclusion that the structural chain (CVP strips marker → Phase 2
misses → narrowing fails) cannot be fixed locally without either
frontend cooperation (option 4: `!range` metadata) or research-grade
middle-end work (option 5: stronger cyclic-phi range analysis).

## Net state

- **Soundness restored end-to-end** (test_220/221/222 PASS on Z80
  AND under simavr on AVR).
- **AES size win preserved**: clang −22 % smaller than SDCC.
- **AES speed regression accepted**: clang +51 % slower; off the
  critical path for the four finishing-firmware components.
- **v1 + v2 sound gate landed** and useful for future shapes even if
  it doesn't crack AES K&R.
- **Production firmware byte-identical** to pre-investigation main.

## Drafts parked (not filed)

- `tasks/upstream-5bug/rfc-icmp-narrow-outside-user.md` — RFC for the
  middle-end sound icmp-narrowing extension.  Four open questions
  flagged for review.
- `tasks/upstream-5bug/draft-cvp-strips-narrowness-marker.md` — issue
  describing the multi-pass interaction (Z80 narrowing miss caused
  by CVP stripping AggressiveInstCombine Phase 2's marker).  Both
  await per-filing user go-ahead per [[feedback_explain_before_filing]].

## Memory notes added

- `tasks/memory/feedback_multi_pass_marker_interactions.md` — HARD
  rule: when an optimization "should fire" but doesn't,
  `-print-after-all` and check if the trigger marker existed earlier
  and was stripped mid-pipeline.
- `tasks/memory/project_aes_kr_speed_gap_accepted.md` — pins the
  accepted regression + revisit triggers.

## Tasks raised

- `#29` — update CLAUDE.md AES headline.  **DONE this session.**
- `#30` — decide on filing the CVP-strips-marker draft.  HOLD.
- `#31` — decide on filing the icmp-narrow RFC.  HOLD.
- `#32` — cheap test of option (3) `getPredictableBranchThreshold`
  change.  HOLD; only if AES speed becomes a priority.

## Commits this session

llvm-z80 main:
- `fa1606f` icmp-narrow v1 (sound icmp gate)
- `230ba09` Merge v1
- `c4f52eb` icmp-narrow v2 (and-mask outside-user path)
- `4a4b124` Merge v2
- `fe2f894` investigation findings doc
- `2f5d49f` deeper findings (CVP + branch-form chain)
- `21ef058` option (2) negative result

rc700-gensmedet main:
- `b58f111` AES sweep refresh post-sound-gate

workspace main:
- `35936ce` icmp-narrow v1 bump
- `77808be` RFC draft
- `0dcf93b` v2 bump
- `81a9b6d` investigation findings bump
- `79a6f60` deeper findings bump
- `3393d0d` option (2) negative-result bump
- (this session's wrap commit follows)
