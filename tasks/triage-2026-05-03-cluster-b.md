# Triage 2026-05-03: Cluster B (BSS-spill family) is mostly stale

**Context:** Session 41 closing carry-forward (`session41-summary.md`)
and `/Users/ravn/z80/CLAUDE.md` both name "Phase 4 Cluster B
(BSS-spill family — #100, #20, #96, #16)" as the next-session entry
point.  Triage on 2026-05-03 against current open-issue state shows
this list mixes clusters and includes three issues that have been
owner-downgraded since the roadmap was written (session 36).

## Per-issue status

### #100 — Loop rotation forces BSS-spill of loop carrier across CALL

**LIVE.**  Measured impact (2026-05-02): rcbios +33 B and cpnos-rom
+4 B if `Z80LoopRotate` flips to `cl::init(true)`.  Currently gates
#77a default-on.  Four fix options documented in the issue body:

  1. Extend BSS-spill -> PUSH/POP peephole to cross-back-edge form.
  2. Regalloc cost-model tweak (rematerialize across CALL).
  3. Pre-rotation regalloc hint.
  4. Skip rotation when loop body contains a CALL.

Option 4 is trivial but defers the win; option 1 is the targeted
fix with real return.  This is the only live, implementation-ready
issue in the named cluster.

### #20 — BSS spill across CALL: multi-value pattern

**OWNER-DOWNGRADED (April 2026).**  Comment by @ravn:

> Recommendation: deprioritize #20 in favor of #43 (broader payoff)
> or wait for regalloc improvements upstream.

Implementation cost: 200+ lines of dataflow + safety checks for
~12 B PROM win, with a documented stack-corruption risk class
(LIFO depth tracking, cf. closed #41).  Cross-block PHI-via-memory
sub-cases require path-balance analysis at every join point.

### #96 — Investigation: regalloc-level PUSH/POP spilling (layer 3)

**INVESTIGATION ISSUE, NOT IMPLEMENTATION.**  Issue body explicitly
says "no deadline; this is exploratory work" and "lower priority
than #77 ... and the active regalloc cluster (#94, #89, #95)."
Layer-3 PUSH/POP spilling needs a new `MachineFrameInfo`
StackObject flavor (LIFO-only) plus cross-BB stack-depth
bracketing.  Not a single-session target.

### #16 — PUSH/POP instead of IX-indexed spills across CALLs

**OWNER-DOWNGRADED (March 2026, latest 2026-04-07).**  Original
~40 B estimate was already revised to ~6-8 B after IX constant
propagation and direct-BSS spill landed (sessions 15/17).  Comment
by @ravn:

> No peephole fix is practical — the multi-extraction pattern in
> check_sysfile requires the allocator to make a different
> register choice upstream.  Overlaps with #27 (per-pair copy
> cost).

Per `roadmap-to-maturity.md` section 5.2, **#16 is in Cluster A
(regalloc), not Cluster B (spill mechanism).**  CLAUDE.md's listing
of #16 under Cluster B was an error — the canonical clustering
puts it next to #27, #94, #89, #99.

## Roadmap-vs-CLAUDE.md drift

`roadmap-to-maturity.md` (session 36, 2026-05-02) says:

  - **Phase 1 (Foundation) is next** — CI workflow, size baseline
    tracker, late-opt audit, source-cleanup audit.
  - Phase 2 — correctness sweep (#28, #36, #38, #63, #81).
  - Phase 3 — Cluster A regalloc (#98, #94, **#89**, #99, #27).
  - Phase 4 — Cluster B spill mechanism (#100, #20, #96, **#12**).
    Note **#12, not #16**.
  - Cluster ordering rationale (line 345): "Cluster B in parallel
    with A".

`/Users/ravn/z80/CLAUDE.md` and `session41-summary.md` both
recommend "Phase 4 Cluster B (BSS-spill family — #100, #20, #96,
#16)" as the next entry.  This jumps Phases 1-3 and substitutes
#16 (a Cluster A item) for #12 (the actual Cluster B item).

## Lens: structural fixes over peephole accumulation

User principle (restated 2026-05-03, persistent across memory
entries `feedback_root_cause_over_peephole.md` and
`feedback_proper_fixes_immature_backend.md`):

> "I want the underlying datastructures to reflect the z80
> properties as well as possible, instead of fixing bad
> modelling with peephole optimizations."

This is not a per-fix preference; it is a **prioritization lens**
for picking the next entry.  Z80 backend is preliminary; the goal
is to *finish modelling the architecture correctly*, not to
accumulate post-RA fixups for shapes the modelling layer should
never have produced.

Each candidate is therefore reclassified by where the fix lives:

| Layer (most -> least structural) | Examples |
|---|---|
| TableGen / register classes | new register class, instruction predicate, isReMaterializable flag |
| Target hooks (TTI, RegisterInfo, CallLowering) | hint logic, cost model, calling conv |
| ISel patterns / GISel combiners | pre-RA pattern matching, combiner rules |
| Pseudo expansion / Z80ExpandPseudo | post-RA but pre-final-emit; declarative |
| Late-opt peephole | post-RA pattern rewrite; reactive |

A fix at a higher tier closes the issue *and* a class of similar
shapes that would otherwise need their own peepholes later.  A
fix at the lowest tier closes one shape and adds permanent
maintenance surface.

## Better entry candidates, ranked under the structural lens

Each entry is annotated with **layer** (where the fix lands) and
**closes** (the class of shapes it eliminates, not just the
single repro).

1. **#113** — class restriction for undocumented IXH/IXL/IYH/IYL
   in pseudo expansions.
   *Layer:* TableGen / register classes (option 1 in the issue
   body: switch affected pseudo operands to `GR16NoIR`).
   *Closes:* the entire class of "pseudo emits undoc op when IY
   un-reserved" pessimizations.  Gates IY un-reserve (#38) along
   with #115.  ~50 LOC, declarative, low risk.

2. **#98 + #94** — regalloc hint cluster (per roadmap section
   12.3).  #98 is the gating investigation: "why doesn't regalloc
   model B as dead between sequential DJNZ-eligible loops?"
   Output is a design doc; #94's fix follows.
   *Layer:* `Z80RegisterInfo::getRegAllocationHints` +
   `MachineLoopInfo` integration + cost-model tuning.
   *Closes:* #92 (already done), #94 (sequential DJNZ), #89
   (loop-invariant in DE clobbered by counter spill), #99
   (i16-counter ping-pong), #27 (per-pair copy cost).
   Per the roadmap: "one good cost-model change can close 4-5
   issues."

3. **#89 alone** (if standalone session preferred over #98+#94
   investigation).
   *Layer:* regalloc cost model — don't spill counter into D when
   D holds loop-invariant constant.  The issue body's analysis
   names the structural fix directly; concrete reproducer with
   `setup_ivt` 25 B -> ~17 B.

4. **#100 — option 2 or 3 only**, NOT option 1.
   *Layer:* option 2 (regalloc cost-model: rematerialize across
   CALL) or option 3 (pre-rotation regalloc hint).  Option 1
   (extend the BSS-spill peephole to cross-back-edge) is exactly
   the kind of peephole accumulation this lens deprioritises.
   Option 4 (skip rotation when CALL in body) defers the work
   cheaply but doesn't fix the modelling gap either; acceptable
   only as a temporary brake.
   *Closes:* the rotation-around-CALL spill class plus any future
   CALL-in-loop pattern that triggers the same regalloc choice.

5. **roadmap Phase 1 Foundation** — CI workflow + per-function
   size baseline tracker.  Not a codegen change at all, but
   structural in a different sense: it makes regalloc-level work
   *measurable* without per-session manual size-comparison.  Per
   roadmap section 12.1.

### Demoted under this lens

- **#109** — hardens an existing post-RA peephole
  (`Z80LateOptimization.cpp` ADD HL,rr commutativity).  The
  underlying question is whether the codegen should ever produce
  the `LD C,L; LD B,H; EX DE,HL; ADD HL,BC` shape that the peephole
  rewrites — proper fix is at ISel / regalloc, not the peephole.
  The audit is still worth landing (a peephole that exists must
  be safe), but it doesn't advance modelling.  Demoted from #1
  to "do alongside, not instead of, structural work".

- **#108** — same shape as #109; latent-bug audit on six existing
  peepholes.  Useful safety hardening; structural-zero.  Demoted.

- **#100 option 1** (extend BSS-spill peephole to cross-back-edge)
  — explicit peephole accumulation.  Demoted out of the live list;
  treat as fallback only if structural fixes (option 2/3) prove
  infeasible.

## Recommendation under the structural lens

Drop "Phase 4 Cluster B (BSS-spill family — #100, #20, #96, #16)".
Replace with **roadmap-aligned structural work**:

  - **Single-session structural pick:** #113 (TableGen class
    restriction; cleanest landing; advances IY un-reserve).
  - **Multi-session structural pick:** Phase 3 Cluster A starting
    with #98 investigation, then #94 + #89 fix.  Per roadmap:
    one regalloc cost-model change closes 4-5 issues.
  - **Infra pick:** Phase 1 Foundation (CI + size baseline) so
    structural work has measurable feedback.
  - **Avoid:** new post-RA peepholes; #100 option 1; #109/#108
    hardening as the *primary* session goal.

The previous "tactical thread (#109 -> #89)" recommendation is
withdrawn — #109's primacy was a peephole-first ranking and
contradicts the user's structural-first principle.
