# Design: target hook for `llvm.experimental.memset.pattern`

**Status:** DRAFT 2026-06-09.  Awaiting user review before filing.
**Filing target:** `llvm-z80/llvm-z80` (fork-of-record), staged so that
acceptance there builds the case for eventual mainline `llvm/llvm-project`
submission.
**Author/defender:** user (Thorbjørn Ravn Andersen).  This doc captures
the design so it can be defended in the user's own words per HARD rule
`feedback_explain_before_filing` (post-session-77 PR-#17 retraction).

## 1. Problem statement

LLVM defines `llvm.experimental.memset.pattern(ptr dst, iN pattern, iM count, i1 isvolatile)`
as the canonical IR-level representation of a multi-byte pattern fill
(`for (i=0; i<count; i++) dst[i*sizeof(pattern)] = pattern`).

The upstream lowering path lives in `llvm/lib/CodeGen/PreISelIntrinsicLowering.cpp::lowerMemsetPattern` (line 411).  It does, in order:

1. If `getMemSetPattern16Value(...)` succeeds (the pattern fits in 16 B
   AND the target has a `memset_pattern16` libcall available, i.e.
   Darwin libc), emit a call to `memset_pattern16`.
2. Otherwise, `expandMemSetPatternAsLoop(Memset, TTI)` — emit a generic
   loop of stores at IR level.

There is **no target hook between these two cases**.  A backend that has
a *native* multi-byte fill instruction has no way to claim the intrinsic:
by the time GISel/SelectionDAG sees the IR, `PreISelIntrinsicLowering`
has already either emitted the libcall or expanded to a loop.

**Z80 has LDIR** — a single-instruction byte-copy loop with implicit
counter (BC), source (HL), destination (DE), and decrement-to-zero
termination.  A K-byte pattern fill on Z80 is two instructions: write
the K-byte seed to `dst[0..K-1]`, then `LDIR` with `HL=dst`, `DE=dst+K`,
`BC=K*(count-1)`.  This is irreducibly the smallest and fastest lowering.

The current workaround in our fork (ravn/llvm-z80#205, session 76):

- Define a target-specific intrinsic `llvm.z80.pattern.fill(ptr, iN,
  i16 K, i16 count)` in `IntrinsicsZ80.td`.
- Add a pre-ISel pass `Z80LoopIdiomFill` that pattern-matches the
  source loop and rewrites it to a call of `llvm.z80.pattern.fill`.
- Lower the custom intrinsic to seed-store + LDIR in `Z80LegalizerInfo`.

This works in production (cpnos IVT-init uses it) and is byte-identical
to the prior `volatile`-marked overlapping-memcpy lowering it replaced.
But it carries growing **upstream-debt**:

- **Duplicate idiom recognizer.**  `Z80LoopIdiomFill` mirrors logic that
  `LoopIdiomRecognize` already implements upstream, just with a different
  emit target.  Two recognizers will drift in shape and edge-case
  handling over time.
- **API-drift maintenance cost.**  ravn/llvm-z80#217 is the first
  concrete drift cost: a 2026-06 upstream merge re-imported a
  `hasDedicatedExits()` assert into `deleteDeadLoop()`, and our
  `Z80LoopIdiomFill::runOnLoop` does not satisfy that precondition.
  Custom passes touching upstream-shared loop APIs are now ours to
  maintain forever.
- **Unfiled smell.**  Per the U-LLVM coherence map
  (`tasks/upstream-coherence-map-2026-05-22.md` Tier I), every fork-local
  workaround should have a *corresponding upstream bug filed* so the
  workaround story is traceable.  This one has none.

## 2. Proposal

Add a TargetTransformInfo hook that PreISelIntrinsicLowering consults
before expanding `experimental_memset_pattern`:

```cpp
// llvm/include/llvm/Analysis/TargetTransformInfo.h
class TargetTransformInfo {
  ...
  /// Return true if PreISelIntrinsicLowering should expand
  /// `llvm.experimental.memset.pattern` (either to a libcall or to a
  /// loop).  Return false if the target's legalizer or a target-specific
  /// pass will lower the intrinsic itself.  Default: true.
  bool shouldExpandExperimentalMemSetPattern(MemSetPatternInst *I) const;
};
```

Default returns `true` — every existing backend keeps its current
behavior.  Z80 overrides to return `false`; the intrinsic survives
PreISelIntrinsicLowering and is claimed by `Z80LegalizerInfo` (which
already knows how to emit seed + LDIR — that logic moves from the
custom intrinsic arm to the experimental-memset-pattern arm).

### 2.1 Order of operations in `PreISelIntrinsicLowering`

```
case Intrinsic::experimental_memset_pattern: {
  PatternValue = getMemSetPattern16Value(Memset, TLI);
  if (PatternValue) {
    emit memset_pattern16 libcall;          // unchanged
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

The libcall path runs first so Darwin behavior is byte-identical (no
behavioral change for any in-tree target that uses the libcall).  The
hook is consulted only on the fallback path.

### 2.2 Backend obligations

A backend overriding the hook to `false` MUST handle
`Intrinsic::experimental_memset_pattern` at legalization time (GISel:
`LegalizerHelper::lowerIntrinsic`, or in `Z80LegalizerInfo`'s switch on
intrinsic ID; SelectionDAG: `INTRINSIC_VOID` ISD node handling).  If
the backend fails to lower it, codegen aborts — the same failure mode
as any unlowered intrinsic.

This is a documented contract on the hook.  Doxygen + a comment in
`PreISelIntrinsicLowering`.

## 3. Open design questions — answered

### 3.1 TTI vs TLI

**Chosen: TTI.**

`expandMemSetPatternAsLoop` already takes a `TargetTransformInfo*` for
the preferred loop-op type query (`getMemcpyLoopLoweringType`).  The
existing memset/memcpy expansion architecture is TTI-centric.
`shouldExpandExperimentalMemSetPattern` joining that family is the
consistent choice.

TLI would also work but is at a lower layer (TargetLowering is a
codegen-time abstraction; PreISelIntrinsicLowering runs before that
layer is constructed for the function).

### 3.2 `bool` vs enum

**Chosen: `bool`.**

Simplest possible surface: backend wants to claim it (false) or doesn't
(true).  A richer enum
(`Expand` / `LowerToLibcall` / `LeaveForBackend`) could absorb the
existing libcall decision too — but that's a bigger change with wider
review surface.  Keep this PR small and orthogonal; the enum
generalisation can be a follow-up if a future backend wants to override
the libcall decision specifically.

### 3.3 Backward compatibility with `memset_pattern16` libcall

**Preserved.**  The libcall path is consulted first; the hook only
applies when the libcall path doesn't fire.  Darwin / any target with
`memset_pattern16` in TLI is byte-identical.

## 4. Risks

| Risk | Mitigation |
|---|---|
| Other backends might silently start claiming the intrinsic when they shouldn't | Default `true`; no behavioral change for any backend that doesn't explicitly override. |
| Downstream LLVM passes may not handle `experimental_memset_pattern` post-PreISelIntrinsicLowering | Survey the pass pipeline: every pass between PreISelIntrinsicLowering and ISel that touches IR intrinsics needs to be checked for safe handling of `experimental_memset_pattern`.  Empirically, the intrinsic is `IntrFamilies` / `MemoryEffects` annotated, so most passes treat it conservatively (don't reorder, don't drop).  Verify with a lit test that exercises the path. |
| Out-of-tree-target motivation may not convince mainline LLVM reviewers | Land in `llvm-z80/llvm-z80` first.  Use that as evidence that the design is implementable, tested, and motivated by a real backend (even if currently out-of-tree).  Mainline submission becomes a follow-up once Z80 itself approaches mainline acceptance. |
| Future LLVM changes to `expandMemSetPatternAsLoop` signature or location | Hook is orthogonal to those changes; if `expandMemSetPatternAsLoop` moves, the hook signature is unaffected. |

## 5. Alternatives considered

### 5.1 Status quo (fork-local custom intrinsic + custom recognizer)
- **Pros:** ships now; no upstream coordination.
- **Cons:** duplicate recognizer; growing API-drift cost (already manifested as #217); not in `#186` U-LLVM queue, so the workaround story is invisible to outside reviewers.
- **Reject:** project direction is upstream-correctness over workaround-forever.

### 5.2 Add `LegalizeAction::Custom` in `Z80LegalizerInfo` for `experimental_memset_pattern`, no upstream change
- **Why it doesn't work:** `PreISelIntrinsicLowering` runs *before* legalization.  The intrinsic is already expanded (libcall or loop) by the time legalize sees it.  Without the upstream hook, the intrinsic never reaches the backend's legalizer in the first place.

### 5.3 Fix `expandMemSetPatternAsLoop` to produce IR that Z80 backend can recognize and combine post-ISel
- **Why it doesn't help architecturally:** even if the expanded loop legalized cleanly, the Z80 backend would need to re-recognize the pattern and rewrite to LDIR — a second pattern matcher with the same drift risk we're trying to eliminate.

### 5.4 Promote `llvm.experimental.memset.pattern` from experimental to stable, with stronger TTI cost model
- **Bigger scope; orthogonal.**  Worth doing eventually, but not the right unit of work for this PR.

## 6. Discussion approach for the maintainer

### 6.1 Maintainer context

`llvm-z80/llvm-z80` is shepherded by @zlfn.  Recent interaction
history (per session 77 / PR-#17 retraction):

- @zlfn closed PR-#17 (session 77's curated submission) with the
  message: "can't merge code contributions that contributors can't
  explain themselves."
- Five of the six XFAIL tests in that PR were target-agnostic
  generic-LLVM bugs that didn't belong at the fork at all (routing
  miss).
- The rule the team adopted in response is `feedback_explain_before_filing`:
  explain root cause + get explicit per-filing go-ahead before any
  upstream-direction post.
- @zlfn's standard, paraphrased: every line must be defensible by the
  contributor in their own words; quality bar over volume; one issue
  per underlying bug; tests-only PRs preferred over fix PRs.

### 6.2 Why this submission is different from session 77's retraction

- **Not a bug fix.**  This is a *feature addition* (target hook) +
  Z80 backend consumer that demonstrates the value.
- **Single coherent design.**  One TTI hook, one PreISelIntrinsicLowering
  consumer, one Z80 backend override.  No bundling of multiple
  unrelated changes.
- **Routed correctly.**  The change touches `llvm/lib/CodeGen/` generic
  code, but the *motivation* is Z80-specific (LDIR consumer).  The
  fork-of-record is the right first stop for a generic+Z80 paired
  change: mainline alone is harder to justify (no in-tree consumer),
  Z80-backend alone doesn't work (PreISelIntrinsicLowering is generic).
- **Defensible.**  The user can explain each design choice in their own
  words from this doc (sections 3.1-3.3 + 5).

### 6.3 Suggested message shape — issue-first, not PR-first

File an **issue** at `llvm-z80/llvm-z80` with title:

> Proposal: `TargetTransformInfo::shouldExpandExperimentalMemSetPattern` hook

Body covers:

1. Problem statement (section 1 above, condensed).
2. Proposed design summary (section 2.1 code block).
3. Alternatives considered (section 5, summarised).
4. **Questions for @zlfn before writing code:**
   - Does the fork accept generic-LLVM design hooks that demonstrate
     Z80-specific value but have no current in-tree consumer?
   - Preference for TTI vs TLI placement (section 3.1 leans TTI)?
   - Preference for staging: land here first (then mainline), or
     mainline-RFC first (then mirror here)?
   - Should the demonstrating Z80 backend consumer ship in the same
     PR or as a follow-up?
5. State that the implementation is ready to draft if the proposal is
   accepted; defer to @zlfn's preferred sequencing.

### 6.4 What NOT to do

- Do **not** open a PR before the issue gets a substantive response.
  Session 77's mistake was filing a tests-only PR + 6 fix issues
  simultaneously; the right sequence here is RFC-style issue,
  maintainer-accept, then PR.
- Do **not** attach speculative co-author lists ("Co-Authored-By:
  Claude").  The user is the contributor of record; the doc captures
  the design so the user can defend it.
- Do **not** reference internal session numbers / ravn/llvm-z80 issue
  numbers in the external-facing issue body without context.  Treat
  the audience as upstream maintainers who don't know the fork's
  history.
- Do **not** file simultaneously at `llvm/llvm-project`.  Mainline
  filing is the *outcome* of fork acceptance, not a parallel track.

### 6.5 If @zlfn declines

Possible outcomes and responses:

- **"No current in-tree consumer, mainline first."**  Acknowledge,
  draft RFC for `llvm/llvm-project` instead.  Less likely to land
  without Z80 in mainline, but the design doc is reusable.
- **"Design is fine, but defer until Z80 mainline lands."**  Park the
  PR; revisit when Z80 upstreaming progresses.  Document the parking
  here.
- **"Design needs rework."**  Iterate based on feedback; this doc is
  the starting point.
- **No response within 2 weeks.**  Ping politely; if still silent,
  escalate to a draft PR (gives more concrete context for review)
  with explicit "marking as draft until @zlfn weighs in" language.

## 7. Implementation plan once accepted

(See `tasks/plan-...` to be written when the issue is filed and
@zlfn's preferences are clear.  Sketch:)

| Stage | Files | LOC est. |
|---|---|---|
| 1. Add TTI hook (default true) | `TargetTransformInfo.h/.cpp`, `TargetTransformInfoImpl.h` | ~15 |
| 2. PreISelIntrinsicLowering consumes the hook | `PreISelIntrinsicLowering.cpp` | ~10 |
| 3. Z80 backend overrides the hook | `Z80TargetTransformInfo.h/.cpp` | ~5 |
| 4. Z80 legalizer claims the intrinsic | `Z80LegalizerInfo.cpp` | ~30 (mostly moved from existing custom-intrinsic arm) |
| 5. Z80LoopIdiomFill emits upstream intrinsic | `Z80LoopIdiomFill.cpp` | ~10 (signature swap on the IRBuilder call) |
| 6. Delete custom intrinsic + custom legalizer arm | `IntrinsicsZ80.td`, `Z80LegalizerInfo.cpp` | -50 |
| 7. New lit test | `llvm/test/CodeGen/Z80/experimental-memset-pattern.ll` | ~50 |
| 8. Update `issue-205-pattern-fill.ll` to track the new intrinsic name | existing test | ~10 |

Total net: roughly **+30 LOC of upstream change + ~0 LOC of Z80 change**
(the Z80 side mostly *moves* code from the custom intrinsic arm to the
upstream intrinsic arm).  Test coverage grows by ~60 LOC.

## 8. Verification plan

- Upstream lit suite green (`llvm/test`).
- Z80 lit suite green (`llvm/test/CodeGen/Z80`), incl. updated
  `issue-205-pattern-fill.ll` and new `experimental-memset-pattern.ll`.
- Z80 runtime test-runner green at all opt levels.
- Production binaries byte-identical (autoload 1673, cpnos 2028, BIOS
  5905 — compare disasm not just sizes).
- AES corpus byte-identical or measurably-improved.

## 9. Cross-references

- ravn/llvm-z80#205 — original custom-intrinsic fix.
- ravn/llvm-z80#217 — live regression demonstrating the custom-pass
  maintenance burden.
- `tasks/session76-issue205-pattern-fill-2026-05-31.md` — original
  fix writeup.
- `tasks/upstream-coherence-map-2026-05-22.md` Tier I — U-LLVM
  candidate framing.
- HARD rules: `feedback_explain_before_filing`, `feedback_upstream_routing_two_targets`,
  `feedback_no_pull_requests`, `feedback_no_upstream_issues`.
