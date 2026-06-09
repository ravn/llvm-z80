<!--
  RFC issue body for filing at llvm-z80/llvm-z80.

  Purpose: get @zlfn's substantive response on direction BEFORE writing a PR
  (per the design doc section 6.4: "no PR before substantive response").

  Audience: upstream maintainers who do NOT know this fork's history.  No
  internal session numbers; no co-author trailers; user files in their own
  voice.

  Filing instructions:
    gh issue create --repo llvm-z80/llvm-z80 \
      --title 'RFC: TargetTransformInfo::shouldExpandExperimentalMemSetPattern hook' \
      --body-file tasks/upstream-memset-pattern-issue-body.md

  DO NOT file without explicit user go-ahead per HARD rule
  `feedback_explain_before_filing`.  DO NOT open a PR alongside the issue;
  wait for the maintainer's direction on sequencing.
-->

# RFC: `TargetTransformInfo::shouldExpandExperimentalMemSetPattern` hook

## Summary

I'd like to propose a small `TargetTransformInfo` hook that lets a backend
claim `llvm.experimental.memset.pattern` instead of having it expanded
(libcall or open-coded loop) by `PreISelIntrinsicLowering`.

Default: keep the current behaviour for every backend.  Override: a backend
that has a *native* multi-byte fill instruction (Z80's `LDIR` is the
motivating case) gets to lower the intrinsic itself.

Before writing a PR I'd like your view on the direction — especially on
whether a Z80-motivated generic hook is in scope for the fork.

## Problem

`llvm.experimental.memset.pattern(ptr dst, iN pattern, iM count, i1 isvolatile)`
is the canonical IR for a multi-byte pattern fill.  Today its lowering lives
in `llvm/lib/CodeGen/PreISelIntrinsicLowering.cpp::lowerMemsetPattern`:

1. If `getMemSetPattern16Value(...)` succeeds (pattern ≤ 16 B and the target
   advertises a `memset_pattern16` libcall, i.e. Darwin libc), emit the
   libcall.
2. Otherwise, `expandMemSetPatternAsLoop(...)` — emit a generic loop of
   stores at IR level.

There is **no target hook between these two cases**.  A backend that has a
single-instruction multi-byte fill has no way to claim the intrinsic: by
the time GISel / SelectionDAG sees the IR, `PreISelIntrinsicLowering` has
already either emitted the libcall or expanded to a loop.

The motivating case is Z80's `LDIR`: a 2-byte instruction that copies
`BC` bytes from `(HL)` to `(DE)` with implicit increment.  A K-byte
pattern fill on Z80 is irreducibly two instructions — write the K-byte
seed to `dst[0..K-1]`, then `LDIR` with `HL=dst`, `DE=dst+K`,
`BC=K*(count-1)` (the classic seed-and-propagate idiom).  No generic loop
expansion can match it.

## Proposed design

Add a TargetTransformInfo hook consulted between the libcall path and the
loop expansion:

```cpp
// llvm/include/llvm/Analysis/TargetTransformInfo.h
class TargetTransformInfo {
  ...
  /// Return true if PreISelIntrinsicLowering should expand
  /// `llvm.experimental.memset.pattern` (either to a libcall or to a
  /// loop).  Return false if the target's legalizer or a target-specific
  /// pass will lower the intrinsic itself.  Default: true.
  bool shouldExpandExperimentalMemSetPattern(const IntrinsicInst *II) const;
};
```

Default returns `true` — every existing backend keeps its current behaviour.
A backend overriding to `false` MUST handle
`Intrinsic::experimental_memset_pattern` at legalization time (GISel:
`LegalizerHelper::lowerIntrinsic` or the backend's legalizer info;
SelectionDAG: `INTRINSIC_VOID` ISD node).  If it fails to lower, codegen
aborts — the same failure mode as any unlowered intrinsic.  This obligation
goes in the hook's Doxygen plus a comment in `PreISelIntrinsicLowering`.

### Order of operations in `PreISelIntrinsicLowering`

```cpp
case Intrinsic::experimental_memset_pattern: {
  PatternValue = getMemSetPattern16Value(Memset, TLI);
  if (PatternValue) {
    // emit memset_pattern16 libcall (unchanged)
    break;
  }
  TTI = LookupTTI(*ParentFunc);
  if (!TTI.shouldExpandExperimentalMemSetPattern(Memset)) {
    break;                                  // leave intrinsic for backend
  }
  expandMemSetPatternAsLoop(Memset, TTI);   // unchanged
  break;
}
```

The libcall path runs first, so Darwin behaviour is byte-identical (no
behavioural change for any in-tree target that uses the libcall).  The
hook is consulted only on the fallback path.

## Design choices

**TTI vs TLI.**  `expandMemSetPatternAsLoop` already takes a
`TargetTransformInfo *` for its loop-op type query
(`getMemcpyLoopLoweringType`).  Joining that family is the consistent
choice.  TLI would also work but is a lower layer (TargetLowering is
codegen-time; `PreISelIntrinsicLowering` runs before that layer is
constructed for the function).

**`bool` vs richer enum.**  A 3-state
`Expand` / `LowerToLibcall` / `LeaveForBackend` could absorb the libcall
decision too — but that's a wider review surface and orthogonal to the
"give backends a way in" problem.  Boolean keeps the PR small and lets a
future backend that wants to override the libcall decision specifically
propose the enum generalisation on its own merits.

**Libcall path stays first.**  Darwin / any target with `memset_pattern16`
in TLI is byte-identical.

## Alternatives considered

1. **Add `LegalizeAction::Custom` for `experimental_memset_pattern` in the
   backend, no upstream change.**  Doesn't work: `PreISelIntrinsicLowering`
   runs *before* legalization.  Without the upstream hook, the intrinsic
   is already expanded by the time the backend's legalizer sees it.
2. **Have the backend re-recognise the post-expansion loop and rewrite to
   LDIR.**  Doesn't help architecturally: it requires a second pattern
   matcher (after the loop expansion) doing the same work the existing
   `LoopIdiomRecognize` did before expansion.  Same drift risk we'd be
   trying to eliminate.
3. **Promote `llvm.experimental.memset.pattern` from experimental to stable
   with a stronger TTI cost model.**  Bigger scope; orthogonal.  Worth
   doing eventually but not the right unit of work.

## Proof-of-concept

A working implementation lives on ravn/llvm-z80 main (commit
`6839ebc4bcbf`).  It implements all six pieces of the design:

1. New TTI hook (default `true`) in `TargetTransformInfo` / `*Impl` / `Z80TTIImpl`.
2. `PreISelIntrinsicLowering` consults the hook after the libcall path.
3. Z80 TTI overrides to `false` for i8/i16/i32 patterns (K∈{1,2,4}).
4. Z80 GISel legalizer claims `experimental_memset_pattern` and emits the
   same seed + `LDIR` shape used by the prior fork-local intrinsic.
5. The Z80 idiom-recognition pass emits the upstream intrinsic for
   K∈{1,2,4}.
6. A direct-exercise lit test (`experimental-memset-pattern.ll`) pins the
   K=1/2/4 lowering shapes (seed store + `LDIR` with `BC = K*(N-1)`)
   independently of idiom recognition.

K=3 stays on the fork-local intrinsic for now — it'd want either an i24
pattern type or a `K + container` operand split, which is a separate
design call.  Happy to follow up after this hook lands.

Verification on the POC:
- Z80 CodeGen lit suite: all PASS (151 PASS + 5 XFAIL) including the new
  test and the existing pattern-fill tests.
- `Transforms/PreISelIntrinsicLowering` lit subset (8 supported tests):
  PASS; default-true hook preserves behaviour.
- Z80 runtime test-runner (clang suite, all opt levels): green.
- Production-binary oracle on the only consumer of the new path
  (Z80 IVT-init / K=2 pattern fill): byte-identical to baseline.

## Questions for @zlfn

Before I write the actual PR, would value your call on:

1. Does the fork accept generic-LLVM design hooks motivated by Z80 even
   when there's no other in-tree consumer?  (`PreISelIntrinsicLowering`
   is generic code; the motivating backend is currently out-of-tree from
   mainline's perspective but in-tree here.)
2. TTI vs TLI placement (I've leaned TTI — see "Design choices" above)?
3. Preferred staging — land in `llvm-z80/llvm-z80` first then mirror to
   `llvm/llvm-project`, or open the RFC at mainline first and mirror here?
4. Should the demonstrating Z80 backend consumer ship in the same PR as
   the generic hook, or as a follow-up?
5. K=3 path — happy to keep the fork-local intrinsic for the i24 case
   indefinitely, or would you prefer I work the generalisation
   (i24 pattern type) before the first PR?

I have the implementation ready to draft as a PR once you've weighed in on
direction; deferring to your preferred sequencing.

## Cross-references

- Original fork-local fix (custom `llvm.z80.pattern.fill` intrinsic +
  `Z80PatternFillRecognize` recognizer): ravn/llvm-z80#205.
- Live regression demonstrating the custom-pass maintenance burden the
  hook would retire: ravn/llvm-z80#217 (a 2026-06 upstream merge
  re-imported a `hasDedicatedExits()` assert into `deleteDeadLoop()`;
  our pass doesn't satisfy that precondition).
- Proof-of-concept commit: ravn/llvm-z80@6839ebc4bcbf.
