# Phase B0 investigation: where do Tier 1 hooks actually move codegen?

Date: 2026-05-22 (session 73p Phase 2, after Phase A retired Phase E).

## Method

Same discipline as Phase A: validate the premise empirically before
committing to multi-week implementation.  Cost: ~1 h focused reading
of the LICMPass, LoopUnrollPass, and IndVarSimplify source plus AES
corpus baseline interpretation.

## Findings

### 1. LICMPass uses `getInstructionCost` very narrowly

LICM only calls `getInstructionCost` once, inside `isFoldableInLoop`,
exclusively for GEPs.  The predicate: "if a GEP's cost is
`TCC_Free`, it can be kept in the loop (folded into addressing
mode) rather than hoisted."  This is a single decision, not a
broadly leveraged cost.

Implication: refining `getInstructionCost` won't move LICM behavior
on non-GEP IR.  For Z80, GEPs that index a global are already
folded via the `inttoptr+ptr_add` rule (issue #46), so the default
cost computation gives a correct answer.

**Lever size:** small.

### 2. LoopUnroll at `-Oz` already uses Threshold=0

`UnrollOptSizeThreshold` defaults to `0` (LoopUnrollPass.cpp:87).
At `-Oz` / `-Os` LoopUnroll bails on anything with positive cost.
So `getUnrollingPreferences` can't make LoopUnroll *more*
conservative than it already is at production opt levels.

At `-O2` / `-O3` LoopUnroll does fire and refined preferences
would matter — but `-O2` / `-O3` AES configs (`03_O3`, `04_O2`) are
3-5× the size of `-Oz` and not production-relevant.

**Lever size at `-Oz`:** essentially zero.

### 3. SimplifyCFG branch-folding is already cost-gated

`#168` landed `getPredictableBranchThreshold()` returning 0/1 to
cost-gate `foldTwoEntryPHINode`.  That's the SimplifyCFG branch
that pessimized AES via XOR-branchless materialization.  Further
SimplifyCFG cost refinement (via `getCmpSelInstrCost`) is
unlikely to move production codegen — the dominant SimplifyCFG
problem on Z80 is already solved.

**Lever size:** small.

### 4. Inliner already has Z80-specific `areInlineCompatible`

The existing `areInlineCompatible` overrides handle:
  - InlineHint attribute -> inline
  - <= 10 instructions -> inline
  - single-call-site internal -> inline

Refining `getInstructionCost` for the inliner would mostly affect
the multi-call-site case, which `areInlineCompatible` currently
blocks.  Refining `getInstructionCost` *and* relaxing
`areInlineCompatible` is a coupled change with hard-to-predict
spill consequences.

**Lever size:** small without also re-tuning `areInlineCompatible`.

### 5. IndVarSimplify

`getArithmeticInstrCost` and `getCastInstrCost` are queried by
IndVarSimplify to decide whether to widen IVs.  This *is* a
real Z80 lever — widening i8 to i16 doubles the cost on Z80.

But: `Z80NarrowIV` (session-73n, #77 fix path 1) already
narrows i16 IVs back to i8 when SCEV proves the range fits.
That pass operates AFTER LSR and AFTER `TargetPassConfig::addIRPasses`
— meaning it cleans up whatever IndVarSimplify decided.

If `Z80NarrowIV` is keeping the IV widths correct, refining
`getArithmeticInstrCost` so IndVarSimplify makes the right call
earlier is duplicate work.  If `Z80NarrowIV` misses cases, those
should be tracked in #169/#170/#171 (already open) and the fix
is in `Z80NarrowIV` itself, not the IR cost model.

**Lever size:** small — `Z80NarrowIV` covers this lane.

## Re-framing of the AES corpus picture

| Config | Bin | vs SDCC | Δ from `01_baseline_Oz` |
|---|---:|---:|---:|
| `01_baseline_Oz` (vanilla -Oz) | 3703 | +11.4% | — |
| `05_Oz_static_stack` (just +static-stack) | 2630 | −20.8% | **−1073** |
| `07_Oz_no_lsr` (just -disable-lsr) | 4036 | +21.5% | +333 |
| `09_Oz_prod_like` (production) | 2574 | −22.5% | −1129 |

SDCC `01_baseline_prod` = 3323 B.

**Key reading:**

1. The dominant lever between `01_baseline_Oz` (lose to SDCC by 380 B)
   and `09_Oz_prod_like` (beat SDCC by 749 B) is `+static-stack`,
   which alone saves 1073 B.
2. `-disable-lsr` standalone makes AES BIGGER (LSR is helping on AES;
   it's only harmful in the absence of `+static-stack` AND `Z80NarrowIV`).
3. `+static-stack` cannot be the default — it disables reentrancy.
   Production users opt in explicitly.

**Therefore:** the room remaining for IR-level TTI cost refinement
to close the `01_baseline_Oz` gap (3703 -> 3323) is bounded above
by the IR-level decisions that the current `+static-stack` /
`Z80NarrowIV` / `#128` / `#168` / `inttoptr+ptr_add` fold / `Z80
LoopIdiomFill` work already addresses.

That gap is largely a MIR-level / regalloc-level concern, not an IR
cost-model concern.

## Reprioritization recommendation

Phase B Tier 1 hooks (`getInstructionCost`, `getUnrollingPreferences`,
`isProfitableToHoist`) have **low expected impact on production AES
corpus**.  The same is likely true for Tier 2 (`getArithmeticInstrCost`,
`getCmpSelInstrCost`, `getCastInstrCost`).

The remaining production-relevant levers are MIR-level:

- **#173** — 8-bit BSS spill via A is 6 B per cycle.  Mixed-mode BSS +
  PUSH/POP-rr peephole estimated 100-200 B on AES.  **High yield.**
- **#172** — A-register pinning (parked default-off).  Resolving the
  liveness checks could pin the AES inner-loop A-shuttle, ~5 pp of
  the residual SDCC speed gap on AES `01_baseline_Oz` (post-Phase 1
  the speed gap is closed, but it's still 11-19 % on non-prod_like
  configs).
- **#27/#115/#95** — regalloc cost model for IX/IY (Phase 3 #38 gate).

## Recommended Phase B re-scope

Keep Phase B as documentation + best-effort target-truthful overrides,
not a multi-week implementation.  Specifically:

1. **Land target-truthful overrides where the default is wrong on Z80**
   but the override is unlikely to move codegen visibly.  Document each
   override's purpose so future passes that *might* consult them get
   the right answer.

   - `getArithmeticInstrCost`: i16 ops cost 2-4×, mul = libcall, div
     = libcall.
   - `getCastInstrCost`: native-width trunc free; i8->i16 zext free;
     i8->i16 sext = 2.
   - `getCmpSelInstrCost`: compare-with-zero = 1; select = 2-3
     (requires branch on Z80).
   - `getCFInstrCost`: PHI = 0; CondBr = 1.
   - `getMemoryOpCost`: i8/i16 load/store at known address = 1.

   Estimate: 2-3 days of careful overrides, lit + AES gating per
   commit.  Expected production codegen impact: **near-zero**, but
   the overrides are correct and future-proof the backend.

2. **Defer Phases C and D**.  They were predicated on Tier 1 paying
   off enough to justify pursuing Tier 2/3.  Phase B0's finding
   suggests they won't.

3. **Pivot remaining session-73p Phase 2 effort to #173** (BSS spill
   peephole).  Estimated 100-200 B production AES yield; concrete
   MIR pattern; full Decision E gating.

## Status update

- Phase A: complete (retired Phase E).
- Phase B0 (this doc): complete (re-scoped Phase B from
  multi-week implementation to 2-3 days of best-effort
  overrides).
- Phase B (re-scoped): ready to start, much smaller.
- Phase C, D: deferred / parked.

## When this investigation could be wrong

The lever-size estimates above are based on reading the LLVM source
and the AES corpus snapshot.  Two scenarios could reverse the
conclusion:

1. A future Z80-specific IR pass (not yet written) might consult
   TTI hooks and benefit from refined values.  If we anticipate such
   work, the overrides become enabling infrastructure rather than
   immediate-yield optimization.

2. cpnos PROM1 production code (which is not in the AES corpus)
   might be IR-cost-sensitive in ways AES isn't.  Worth a one-shot
   measurement with refined `getArithmeticInstrCost` to confirm
   no change.

Both scenarios suggest the "best-effort target-truthful overrides"
approach is the right hedge: low cost, low risk, future-proof,
and if a future pass needs them they're there.

## Cross-references

- Phase A finding: `issue177-phase-a-investigation.md`.
- Implementation plan: `issue177-implementation-plan.md` (needs
  update to reflect Phase B0 re-scope).
- Work clock: `/Users/ravn/z80/memory/project_issue177_work_clock.md`
  (needs target-completion revision).
- Phase 1 lessons: `session73p-phase1-lessons.md`.
