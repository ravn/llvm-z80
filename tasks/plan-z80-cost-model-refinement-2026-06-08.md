# Z80 cost-model refinement — multi-phase plan

**Drafted:** 2026-06-08, end of #23/#220 session.
**Status:** PROPOSAL — awaiting user decision on whether to start.

## Why this work exists

Over the session 2026-06-08 the Z80 backend's `disablePass(LICM/CSE)`
workaround was retired (#23) and three follow-up attempts to refine
the resulting trade-offs all failed in informative ways:

  - **Count-based `shouldHoist` heuristic** (#220, reverted): added
    measurement noise; the simpler "no override" baseline matched the
    intended outcome.
  - **`getRegPressureSetLimit` override** (GR16 -> 6, Anyi8 -> 6):
    helps AES (~18 B at -Oz) but cannot help autoload (the autoload
    hoists bypass the pressure check via `isTriviallyReMaterializable`).
  - **`isReMaterializableImpl` disable**: recovers ~30 B of autoload
    uncompressed regression but adds +140 B to rcbios (the bypass is
    useful most places, harmful in autoload's `define_sextants`).

The pattern across all three: the LLVM generic passes (MachineLICM,
MachineCSE, MachineSink, regalloc) each have their own cost model
and consult a piece of `TargetInstrInfo` / `TargetRegisterInfo`.
The pieces are individually correct but DO NOT COMPOSE into an
accurate picture of Z80's tiered register file with different cost
tiers (cheap pairs vs index regs vs spill targets) and asymmetric
calling-convention clobber.

The "right" fix is a richer target description that the generic
passes can consult uniformly.  This document plans how to get there.

## What we're tracking (NOT gates — observation checkpoints)

User direction 2026-06-08: this is an exploratory project, "see where
it goes" — no hard restrictions on success.  So instead of pass/fail
gates, we **measure-and-report** at each phase and decide qualitatively
whether the direction is paying off.

Per-target deltas to record at each phase (vs pre-#23 baseline AND vs
current shipping):

  - AES -Oz / -O2: text size + tstates
  - autoload PROM compressed + raw .text + PROM-cap-free
  - cpnos PROM1 compressed + raw .text + cap-free
  - rcbios BIOS bytes + MINI/MAXI cap-free
  - test-runner runtime suite: PASS / FAIL / FATAL count
  - lit: pass count

Correctness gates DO stay hard (runtime + lit clean) — any change
that breaks them is a bug, full stop.  Performance deltas are
exploratory; we record them and the user decides whether the
direction is still interesting.

Aspirational outcome (not a requirement):
  - AES wins preserved
  - autoload regression reduced or eliminated
  - cpnos win preserved
  - rcbios within a few bytes
  - upstream LLVM contribution for the generic pieces

## What stays out of scope

- IX-unreserve runtime bug (issue #12): orthogonal regalloc work.
- Z80 instruction selector improvements (#27, #28 ...): different layer.
- Anything that requires changes to ravn/* CI: separate work.

## Architectural target

The end-state target description should expose:

  - **Register tiers** as separate sub-register classes with explicit
    cost.  Z80: GR16Cheap (HL/DE/BC) / GR16Index (IX, IY) / GR16AF
    (restricted).  Each its own pressure set with realistic limits.
  - **Per-instruction byte size + tstate cost** via a queryable hook
    (currently `getInstSizeInBytes` exists; need a tstate analogue
    OR cycle/byte tuple).
  - **Rematerialization cost** via `getRematCost(MI)` returning a
    numeric estimate, not a boolean (LICM/CSE consult this when
    deciding to hoist a rematable; regalloc consults it when
    deciding to rematerialize vs spill).
  - **Spill cost menu** via `getSpillCost(RegClass)` returning
    (size, tstates) for each available spill mechanism (PUSH/POP,
    IX/IY-spill, BSS).  Regalloc picks lowest-cost; LICM accounts
    for it when computing hoist profitability.
  - **Calling-convention clobber** queryable per-call-site: which
    physical regs / register-pair-units will not survive across this
    call.  Lets LICM cost models predict survival probability for a
    hoisted invariant.

Each of these is a real LLVM concept (some have existing hooks; some
need new ones).  The work is in (a) defining the Z80-specific
content, (b) wiring it through the existing pass consultations, (c)
adding new hooks where existing ones don't suffice, (d) upstream
proposing the new hooks if generic.

## Phases

Each phase has: description, deliverables, success gate, rollback
trigger, effort estimate.  Phases are intended to be one or two
sessions each; user reviews at gate before next phase starts.

### Phase 0 — Infrastructure (~1 session, 2-4 h)

**Goal:** make the next phases cheap to iterate.

  - **Docker-snapshot workflow**: build `llvm-z80-build:{baseline,
    licm-on,heuristic-vN,...}` images.  Pick a clang state, tag,
    use from rc700-gensmedet builds via `BUILD_DIR=docker:tag` or
    similar.  Cheap A/B without rebuilds.
  - **Per-target measurement harness** that builds all four
    production targets (autoload, cpnos, rcbios) + AES corpus +
    test-runner runtime suite in ONE invocation and emits a
    machine-readable TSV with every metric.  Eliminates the
    "stale wc -c" failure mode that bit twice this session.
  - **No-op-control measurement protocol**: every cost-model
    change measured with a control cell that contains the new
    code but configures it to no-op.  Validates that the new
    code's PRESENCE doesn't change measurements (the #220 bug
    that took two cycles to find).

**Deliverables:**
  - `tasks/tools/snapshot-build.sh` — drives docker tagging
  - `tasks/tools/measure-all.sh` — emits the TSV
  - Memory note pinning the no-op-control protocol

**Checkpoint (observation, not gate):** reproduce the current shipping state's
measurements byte-identically via the harness; the TSV becomes
the source of truth for "did this change help."

**Reflection prompt (if it doesn't go where we hope):** none — infrastructure-only, no production
code touched.

### Phase 1 — Add cost-query hooks with conservative defaults (~1-2 sessions, 4-8 h)

**Goal:** introduce the cost queries without changing pass behavior.
The hooks return "approximately TableGen-default" values; later
phases tune them.

  - **`Z80InstrInfo::getRematCost(MI)`** — returns size-in-bytes
    of the instruction's literal form (e.g., `LD A,#imm` = 2 B,
    `LD HL,#imm` = 3 B).  Initially: this matches `getInstSizeInBytes`;
    later phases can adjust if context-aware reasoning is added.
    Add to TargetInstrInfo.h as a new virtual hook with default
    returning size or a stub.  Upstream candidate.
  - **`Z80InstrInfo::getSpillCost(RegClass, SlotKind)`** — returns
    (size, tstates) tuple for spilling RegClass to SlotKind
    (BSS / PUSH-POP / IX-spill).  Initial: just BSS = 3, others
    not available.  Wire through `storeRegToStackSlot` /
    `loadRegFromStackSlot` consultation later.
  - **`Z80InstrInfo::getCallerSavedClobber(CallSite)`** — returns
    bitvector of register units clobbered.  Initially: matches
    `getCallPreservedMask`'s inverse for sdcccall(1) = HL/DE/BC.
    Used by Phase 3's LICM cost-model extension.

**Deliverables:**
  - Three new virtual hooks in `Z80InstrInfo.{h,cpp}`.
  - cl::opt `-z80-use-tiered-cost-model` (default OFF) that gates
    consumers in later phases.
  - Lit tests confirming hooks compile and produce expected values
    on toy instructions.

**Checkpoint (observation, not gate):** lit + runtime + production matrix all unchanged
(hooks not yet consulted by any pass).  Hooks measurable via a
small unit test or `llc -print-after=...` dump.

**Reflection prompt (if it doesn't go where we hope):** any production target regresses on byte
count.  Should not happen since hooks are not wired.

### Phase 2 — Split GR16 register class into cost tiers (~2 sessions, 8-12 h)

**Goal:** make the existing pressure / cost-per-use mechanisms see
the tier gradient.

  - **TableGen split**: GR16 -> GR16Cheap (HL, DE, BC) +
    GR16Index (IX, IY) + maybe GR16Accum (AF).  Each its own
    pressure set with TableGen-computed limit.
  - **Class assignment fixes**: every instruction pattern,
    constraint, and copy/store/load routine that referenced
    GR16 may need updating to reference the right sub-class.
    Large mechanical change; expect breakage during
    transition.
  - **Pressure set name change**: tests that grep for "GR16" in
    asm output may break (cosmetic).
  - **CostPerUse review**: IX (=1) / IY (=2) are TableGen-level;
    confirm they still apply correctly when the class is sub-
    divided.
  - **regalloc hint updates**: `Z80RegisterInfo::getRegAllocationHints`
    may need re-tuning when class topology changes.

**Deliverables:**
  - TableGen reshuffle in `Z80RegisterInfo.td`.
  - Pattern updates wherever needed (likely 20-50 sites).
  - Lit tests for each sub-class's basic ops.

**Checkpoint (observation, not gate):** lit 149 PASS + 4 XFAIL preserved; runtime 854
PASS; production targets within +/-5 % of baseline.  At this
phase the new sub-classes exist but no cost-model consumer has
been wired yet — the change should be largely a refactor with
neutral codegen.

**Reflection prompt (if it doesn't go where we hope):** runtime suite fails (correctness); OR
production targets regress by more than 5 %.  The refactor is
substantial; failure here halts phase 2 and decides whether to
proceed differently (e.g., keep GR16 unified but add a separate
"effective use" pressure set).

### Phase 3 — Wire LICM / CSE / regalloc to consume the tiered cost (~2-3 sessions, 8-16 h)

**Goal:** the real production impact.

  - **MachineLICM.cpp** (upstream-eligible): when checking
    pressure, consult `TII->getRegPressureLimit(MF, SetIdx,
    /*CostKind=*/ForLICM)` so target can report a tighter limit
    for hoist decisions specifically.  Either a new hook or
    extending existing.  Likely needs an upstream patch.
  - **MachineLICM `isTriviallyReMaterializable` bypass**:
    consult `TII->isProfitableToRemat(MI, Loop)` which checks
    both the remat cost AND whether regalloc could plausibly
    rematerialize back at use sites.  Replaces the bypass with
    a cost-aware decision.
  - **Regalloc spill choice**: `Z80InstrInfo::storeRegToStackSlot`
    already exists; extend the chooser to ask `getSpillCost`
    and pick cheapest available mechanism (currently always BSS).
  - **CSE**: harder — CSE doesn't have a target hook in the
    same vein.  Could be done as a Z80-specific post-CSE peephole
    that undoes pessimistic CSE on call-heavy loops, OR a
    pre-CSE filter.  Open design question.

**Deliverables:**
  - Z80-side hook implementations consulted by the above
    integrations.
  - LLVM-side patches (one or more) for any new TTI hooks.
    Drafted but not necessarily filed.
  - Per-target measurement showing the trade-off across all
    four production targets + AES.

**Checkpoint (observation, not gate):**
  - AES: maintains current -118 B / -9.2 % at -O2.
  - autoload: at most +25 B compressed vs pre-#23 (current
    accepted ceiling) OR ideally back to baseline.
  - cpnos: at least -15 B (current win).
  - rcbios: at most +7 B (current).
  - test-runner: 854 PASS.

**Reflection prompt (if it doesn't go where we hope):** any production target regresses by more
than 20 B vs current shipping.  Phase 3 changes are landed
incrementally; rollback only the failing integration, not the
whole phase.

### Phase 4 — Measure, tune, document (~1 session, 2-4 h)

**Goal:** lock in the gains with discipline.

  - Re-run the measurement harness across all four targets +
    AES + test-runner.  Snapshot baseline + post-tuning.
  - Update `known-suboptimal-codegen.md`: B11 marked closed;
    new entries for any residual issues.
  - Update `CLAUDE.md` (workspace + llvm-z80) with the new
    cost-model facts.
  - Memory note for any new discipline emerged
    (e.g., "always probe rematerialization cost, not just
    isReMaterializable, on Z80").

**Deliverables:**
  - Final measurement table.
  - Documentation updates.
  - Session writeup `llvm-z80/tasks/session-YYYY-MM-DD-cost-model-refinement.md`.

**Checkpoint (observation, not gate):** documented state matches measured state;
nothing in CLAUDE.md or memory notes is stale.

**Reflection prompt (if it doesn't go where we hope):** none — documentation phase.

### Phase 5 — Upstream where generic (~1-2 sessions, optional)

**Goal:** contribute back the generic pieces.

  - File at llvm/llvm-project: MachineLICM cost-model extension
    proposing `getRegPressureSetLimit(MF, SetIdx, CostKind)` or
    equivalent.  Z80 is the witness; framing as "tiny-register-
    file target with mixed-cost registers" lets AVR/MSP430 maintainers
    join review.
  - File at llvm/llvm-project: `TargetInstrInfo::getRematCost`
    proposal, if the boolean isReMaterializable proves
    insufficient.
  - Per `feedback_explain_before_filing`: each filing needs root-
    cause explanation + user go-ahead.

**Deliverables:**
  - Drafted issues at `tasks/upstream-5bug/draft-cost-model-*.md`
    (one per upstream proposal).
  - User reviews drafts before filing.

**Checkpoint (observation, not gate):** the drafts get user verdict and (if approved)
file cleanly.

**Reflection prompt (if it doesn't go where we hope):** N/A — these are filings, not production
changes.  If maintainers reject, our fork-local implementations
stay; we just lose the generic-LLVM benefit.

## Risk register

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| TableGen reshuffle in Phase 2 breaks ISel patterns en masse | HIGH | High | Phase 2 has 5% baseline gate; break it = rollback |
| Phase 3's MachineLICM upstream change unacceptable to reviewers | MED | Low | Z80-side hooks still useful as fork-local; upstream is bonus |
| Iterating cost-model tuning chases noise (cf. session 2026-06-08 cascading-revalidation incidents) | HIGH | Med | Phase 0's no-op-control protocol; mandatory per change |
| Production target regression discovered late (in real PROM boot) | LOW | High | Each phase's gate runs MAME boot test before declaring success |
| Stale-build incidents recur | MED | Med | Phase 0's docker-snapshot workflow eliminates incremental-rebuild ambiguity |
| Scope creep — "make the model more precise" expands to "rewrite the backend" | HIGH | High | Each phase has a hard time-box; phases sized for one or two sessions |

## Natural pause points

Per user direction (2026-06-08): exploratory, no hard cap.  Some
natural moments to pause and decide whether to continue:

- After each phase's checkpoint measurements: the data tells us
  whether the direction is paying off.  User decides.
- When a phase reveals an unexpected structural fact (this session
  produced TWO of those: the heuristic side-effect bug, the
  cascading-revalidation cascade).  Pausing to absorb the lesson
  before continuing is usually wise.
- When higher-priority work surfaces (firmware finishing, real
  user-facing bug, upstream review feedback on something else).
- When the user just wants to stop and look at the result.

There's no project-wide budget cap.  Even Phase 0 + 1 alone leave
the tree better off (snapshot infrastructure + cost-query hooks
exist, no production code touched).

## Total effort estimate

| phase | hours | sessions |
|---|---|---|
| 0 | 2-4 | 1 |
| 1 | 4-8 | 1-2 |
| 2 | 8-12 | 2 |
| 3 | 8-16 | 2-3 |
| 4 | 2-4 | 1 |
| 5 (optional) | 4-8 | 1-2 |
| **total** | **28-52** | **8-11** |

That's a real project.  About 1-1.5 months of focused part-time
sessions.  Output: a Z80 cost model that the generic LLVM passes
can use without per-target workarounds; an upstream contribution
that benefits other tiny-register-file targets.

## Decision points the user owns

Before Phase 0 starts:
- Is the success criteria right?  Specifically: is autoload "back to
  pre-#23 baseline" achievable, or do we accept +25 B compressed as
  the floor?
- Phase 5 upstream: yes, no, or wait-and-see?

After Phase 1 completes:
- Phase 2's TableGen reshuffle is the riskiest.  Should we go
  straight to it, or try an interim "virtual sub-classes via
  pressure-set-membership tricks" first?

After Phase 3 completes:
- Any production-target trade-off remaining?  Acceptable, or iterate?
- Upstream timing — file now, or wait for stable measurements?

## What this plan is NOT

- It's not a commitment.  User reviews each phase's plan before
  next phase starts.
- It's not the only way.  Alternative: ignore the structural fix,
  just accept +25 B autoload, file the upstream framing as a known
  issue, and revisit when the autoload PROM cap gets tight.
- It's not blocking other work.  The current shipping state
  (LICM+CSE on, no overrides) is production-ready and PR-able as-is.

## Companion / alternative paths

1. **"Just file upstream"** path — don't do Phases 1-4; jump straight
   to Phase 5 with the current measurements as evidence.  Lets
   upstream maintainers shape the solution; we wait for their patch
   to land in our fork.  ~1 session for the filing; multi-month wait
   for upstream resolution.

2. **"Accept and move on"** path — document the cost-model gaps as
   known-suboptimal entries; treat as background facts; revisit
   only when a production cap becomes tight.  ~30 min for the
   documentation; no further engineering.

3. **"Surgical autoload fix"** path — instead of fixing the cost
   model, refactor `define_sextants` source to avoid the hoist
   trigger (e.g., move constants into the inner loop body, use
   `volatile` to defeat hoisting).  ~2 h; brittle; one-function-at-
   a-time; doesn't help if the pattern recurs elsewhere.

The full project plan above (Phases 0-5) is the structural answer.
Companion paths 1, 2, 3 are the tactical alternatives.

---

## User direction at draft time (2026-06-08)

User said: "I want to see where this goes, so no hard restrictions
for success."

So the plan operates as a research narrative — each phase is a
chapter, the measurements at each checkpoint are observations, the
reflection prompts are decision points (continue / pivot / pause)
that the user owns.  Phases land sequentially; no time-budget cap.

Next session: Phase 0 (infrastructure).  Likely deliverable: docker
snapshot workflow + per-target measurement TSV harness.

---

## Phase 2 VERDICT — PARKED (2026-07-14, data-driven)

Phase 2 (split GR16 into cheap/index cost tiers with separate pressure sets)
was re-examined before starting and **parked as not data-justified**.

**Premise falsified:** IX is always reserved; IY is reserved by default
(`-z80-unreserve-iy`, default off; also the size-opt+static-stack path). So in
the shipping config the only allocatable GR16 members are DE/HL/BC — a single
tier. The proposed index tier (IX/IY) is reserved out, i.e. inert in production.
The cheap/index split already exists in raw form (`GR16NoIR = DE,HL,BC` vs
`IR16 = IX,IY`), and `getSpillCost` already encodes HL(6) vs DE/BC(8).

**Measured ceiling of the index tier** (measure-all.sh, clang 96394df,
baseline vs `-mllvm -z80-unreserve-iy`; TSVs in `tasks/measurements/`):

| target        | Δ text | Δ tstates    | correctness      |
|---------------|--------|--------------|------------------|
| AES -Oz       |  0     | 0            | verify PASS      |
| AES -O2       | -123 B | +13119 (+0.08%) | verify PASS   |
| autoload PROM | -1 B   | —            | boot intact      |
| cpnos PROM1   |  0     | —            | —                |
| rcbios BIOS   |  0     | —            | —                |
| runtime       | —      | —            | 924 PASS/0 FAIL  |

So the index tier is worth ~**0 on every production target** and only -123 B on
AES-O2 (off the production critical path, at a small speed cost). The old
BIOS-23 / autoload-11 / cpnos-10 / AES-145 win (recorded ~2026-05 in
Z80RegisterInfo.cpp) has been absorbed by the tiered #23 cost model + backend
gains landed since. A large, risky TableGen reshuffle to expose a gradient
worth ~0 in production is the wrong prioritization.

Stale claims corrected the same day: `Z80RegisterInfo.cpp` comment and
workspace `CLAUDE.md` IX/IY note now carry the 2026-07-14 re-measurement.

**Consequence:** #23 Phases 2–5 are parked. The live Phase-1 hooks
(`getRematCost`/`getSpillCost`, `-z80-use-tiered-cost-model` default ON) stay.
This matches the CLAUDE.md standing assessment that cheap codegen + regalloc
levers are exhausted on production; the remaining high-value work is upstream
packaging, not further cost-model tuning. Unpark trigger: a production target
becomes cap-tight AND a specific tier decision is shown (by measurement) to
recover bytes there.
