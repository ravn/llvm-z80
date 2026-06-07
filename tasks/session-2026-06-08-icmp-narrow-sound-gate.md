# icmp-narrow sound gate (post-retraction redesign)

**Branch:** `icmp-narrow-sound-gate` (off `main` @ `05d44629e717`).
**Context:** session 2026-06-07 reverted the original #160/#165
icmp-narrow-through-graph extensions because they only checked the *non-graph*
operand's narrowness, not the in-graph operand's.  Runtime witnesses
test_220/221/222 returned 0x05 instead of the sound 0x63 — a real
miscompile.  Cost of the revert: AES `09_Oz_prod_like` went from
2695 B / 11 % faster than SDCC to 2866 B / 51 % slower than SDCC.

The user directive afterwards: *"please revert and then reconsider how
to do it properly"*.  This session is the reconsider phase.

## Diagnosis recap

Original `canNarrowIcmpThroughGraph` gate (now removed on `main`):

```cpp
// For an outside-graph ICmpInst using GraphValue, check Other-side narrowness.
if (auto *C = dyn_cast<ConstantInt>(Other))
  return C->getValue().getActiveBits() <= NarrowBits;
KnownBits Known = computeKnownBits(Other);
return Known.getMaxValue().getActiveBits() <= NarrowBits;
```

Soundness hole: the *graph-side* value (`GraphValue`) feeds the outside
icmp at full wide width.  Narrowing the icmp replaces a wide compare
with one over only the low NarrowBits — different result when high
bits are set.  The gate looked at the wrong operand.

## ROI check (precondition for the redesign)

The AES K&R `gf_log` pattern is what made #160/#165 worth keeping
(headline gain 153 → 28 B).  Inspected the unoptimized IR from
`clang --target=z80 -Os -Xclang -disable-llvm-passes` on
`/tmp/gflog_kr.c`:

- Every i16 value in the loop is sourced from `zext i8 to i16`
  (operands of the icmps: %9, %11, %17, %21, %22, %26, %27, %33, %34,
  %39 are all zext-from-i8).
- KnownBits on every operand proves the top 8 bits clear: a trivial
  `proven_*` shape from the failing-first matrix.

The test_220/221 witness, by contrast, has `%t = add i16 %x, 1` where
`%x` is an unconstrained i16 parameter (no zext lineage).  KnownBits
cannot prove `%t` fits in 8 bits.  `unproven_*` shape.

A sound gate that adds a KnownBits check on `GraphValue` distinguishes
the two perfectly: AES K&R narrows, soundness witnesses don't.  Sound
gate ROI: green.

## What changed

**`AggressiveInstCombineInternal.h`** —
- Forward-declare `ICmpInst`.
- Add `SmallVector<ICmpInst *, 4> PendingIcmps` member (cleared per
  TruncInst run).
- Declare `bool canNarrowIcmpThroughGraph(ICmpInst*, Value*, Type*)`.

**`AggressiveInstCombine/TruncInstCombine.cpp`** —
- New STATISTIC `NumIcmpsNarrowed`.
- New `narrowIcmpOperandFitBits(Pred, NarrowBits)` helper: returns
  `NarrowBits - 1` for signed predicates (so the sign bit stays clear
  at the narrow width, preserving the `samesign` invariant) and
  `NarrowBits` for unsigned/equality.
- New `isIcmpPredicateNarrowSafe`: admits eq/ne/unsigned, plus signed
  only with the `samesign` flag.
- New `canNarrowIcmpThroughGraph`: predicate gate, identifies graph-
  vs-other operand, checks **both** operands' KnownBits against
  `FitBits` (the soundness fix), variable-other path also requires
  `Other->hasOneUse()` to avoid leaving a wide value alive in parallel
  with the inserted trunc.
- `getBestTruncatedType` outside-user loop: when a non-Ext outside
  user is found, ask `canNarrowIcmpThroughGraph` before bailing.
  Admitted icmps go in `PendingIcmps` and `continue`.
- `ReduceExpressionGraph`: rewrites `PendingIcmps` *before* the
  phi-erase loop (otherwise the phi-RAUW-to-poison would corrupt the
  still-wide icmp operands).  Reuses `getReducedOperand` for the
  graph-side; emits `trunc` at the icmp site for variable Other, or
  `ConstantInt::trunc` for constant Other.  Preserves `samesign`.

**`llvm/test/Transforms/AggressiveInstCombine/trunc-narrow-icmp-graph-side-soundness.ll`** —
restored as the spec for the v1 sound gate.  21 functions:

- 12 `unproven_*` negatives (gate must reject): all six unsigned
  predicates, RHS-graph variant, samesign-signed without proven
  graph-side, variable Other with unproven graph, eq/ne, 9-bit
  boundary.
- 6 `proven_*` positives (gate must accept): K&R-shape constant-Other,
  exact 8-bit fit, variable Other proven, i32→i16 generality.
- 2 `samesign_*` signed-boundary: `fits8_not7` (mask 255, must NOT
  narrow at i8 because sign bit would be set), `fits7` (mask 127,
  must narrow).
- 2 `two_icmps_*` multi-escape: one-unsafe rejects whole graph; both
  proven narrows both.
- 1 `const_other_too_wide` control: constant 300 doesn't fit i8.

Two tests from the deleted v2 spec are NOT in v1
(`andmask_unproven_still_narrows`, `both_operands_in_graph_proven`) —
they cover the and-mask outside-user path and the both-in-graph
variable path, both deferred to a future increment.

## Verification

### Lit
- `trunc-narrow-icmp-graph-side-soundness.ll`: PASS (all 21 functions).
- Full `AggressiveInstCombine/` + `CodeGen/Z80/` suites: 190 PASS + 4
  XFAIL + 1 FAIL.  The single failure (`fold-split-ctlz.ll`) is
  pre-existing on `main` — verified by stashing, rebuilding,
  re-running (same output).

### Runtime fixtures (Z80, via test-runner)
- `test_220_icmp_narrow_soundness` × 6 opt levels: PASS, DE=0x0063.
- `test_221_icmp_const_narrow_soundness` × 6: PASS, DE=0x0063.
- `test_222_icmp_narrow_soundness_matrix` × 6: PASS, DE=0x0000.

Every level returns the SOUND value — 0x05 was the unsound result the
pre-revert pass returned.

### AVR cross-target soundness witness
Added `tasks/upstream-5bug/avr/sound_gate_220_soundness.c` and a
`sound-gate-220-test` Makefile target.  Compiled with
`clang --target=avr -mmcu=atmega328p -Os` and run under simavr:

```
pick_var(260,10)=0x63
pick_const(260)=0x63
VERDICT: SOUND
```

Confirms that the target-independent middle-end pass is sound under
AVR's codegen path too.  Useful as evidence the optimization is
target-agnostic.

### AES corpus
`make sweep_clang`, all 13 configs verifier PASS.  Headline
`09_Oz_prod_like` 2581 B / 18.21 M tstates (vs post-revert
2866 B / 18.28 M).  Recovered −285 B and the speed wins.

### Production byte/size compare
Built `autoload-in-c` and `cpnos-in-c` PROM1-line-program twice
(once on `main`, once on `icmp-narrow-sound-gate`):

|         | Main      | Sound-gate branch | Δ   |
|---------|-----------|-------------------|-----|
| autoload (compressed) | 1658 B    | 1658 B            | 0   |
| cpnos PROM1 (compressed) | 2029 B    | 2029 B            | 0   |

Size-neutral on production.  Binaries are not byte-identical (different
instruction sequences from the narrowing changes inside production C
that happen to net to zero size), but builds succeed and lit/runtime
gates are green.

## Sharing with other 8-bit platforms (user hint)

AggressiveInstCombine is target-independent middle-end.  The sound
gate is generic LLVM logic that any backend benefits from when its
narrow integer width is cheaper than its wide one.  The AVR
cross-target soundness witness — same C, same simavr verdict — is the
first concrete evidence the optimization travels.

This is upstream-eligible at `llvm/llvm-project` per
`feedback_upstream_routing_two_targets` (middle-end pass = generic
LLVM).  Per `feedback_explain_before_filing` and
`feedback_file_bugs_not_fixes`, do NOT file without the user
explaining the precondition + the soundness gate framing and giving
explicit go-ahead.  Suggested framing:

- **Title (RFC, not bug):** "Extend AggressiveInstCombine
  TruncInstCombine to admit narrowable icmp outside-graph users".
- **Evidence section:** Z80 AES K&R `gf_log` 153 → 28 B (5.4×); AVR
  cross-target sound on the runtime witness; lit matrix pinning the
  soundness boundary precisely (21 functions).
- **No fix included in the upstream filing** per the discipline
  rules; the rfc body describes the principle and the precondition,
  and the maintainer decides shape.

## Open follow-ups (not in v1)

1. **And-mask outside-user path** (the second half of the original
   #165).  Lit test `andmask_unproven_still_narrows` is the spec; not
   yet implemented in v1.
2. **Both-operands-in-graph variable path**.  Lit test
   `both_operands_in_graph_proven`; rejected by the `hasOneUse` gate
   on the variable Other.  Either lift the gate via a different
   safety argument, or treat the in-graph operand specially.

Both are size wins beyond what v1 captures, and both can be added on
top of v1 without further soundness concern as long as KnownBits is
checked on every observed operand.
