# Execution plan — clang upstream coherence (2026-05-22)

Companion to `unpark-2026-05-22.md` (active backlog) and
`upstream-coherence-map-2026-05-22.md` (full classification).
This doc answers: **in what order, with what gates, do we work
through everything?**

## Goal (restated)

Two end-states, pursued in parallel because they have different
audiences and gate independently:

1. **Clang ≥ SDCC on the corpus.**  AES is done.  Open: BIOS,
   cpnos, wider-oracle corpus (`compiler-comparison-corpus/`),
   future libc-dependent benchmarks (gated on #35).
2. **Upstream-submittable findings.**  Generic LLVM improvements
   PRable to `llvm/llvm-project` today; Z80-backend patch series
   PRable to `llvm-z80/llvm-z80` after the cleanup gates close.

The plan does NOT try to land everything.  It tries to make
progress measurable and coherent against both goals at every
checkpoint.

## Constraints (binding methodology rules)

These shape every decision below.  They are not aspirations.

1. **30-min drill before declaring multi-week.**  Session 73p had
   5 cases where surface "multi-week" estimates collapsed to
   5-line fixes once a drill was attempted.  Per
   `feedback_dig_deeper_before_parking`.  Each work-unit below
   starts with an instrumented drill.

2. **Value oracle preserved on every commit touching codegen.**
   Per `feedback_no_commit_first_version`.  AES 13/13 PASS, lit
   suite green, cpnos polypascal-test PASS, test-runner baseline
   (681/46/56/207).  No "lit + size passes" commits.

3. **Structural fixes need pressure evidence, not just plausibility.**
   Per `lessons-2026-05-04-structural-fix-failures.md`.  Three
   plausible structural fixes (drop `isAsCheapAsAMove`,
   loop-depth gate, ...) all regressed real workloads because
   the Z80's 3-pair register file makes pressure dominate the
   cost.  Instrument pressure histograms before committing.

4. **Compiler is not trusted** — when an item shows up as a
   "source bug", inspect generated Z80 asm first
   (`feedback_compiler_not_trusted`).

5. **Each compiler bug gets an XFAIL lit test before the fix**
   (`feedback_test_before_fix`, `feedback_compiler_bug_test`).

6. **Both compilers, both transports.**  Any Z80-backend change
   landing in production code must build with clang + SDCC and
   pass the {PIO, SIO} test cells (`feedback_dual_compiler_test`,
   `feedback_value_oracle_all_transport_cells`).

## Decision: four parallel tracks

Why parallel?  The four tracks below have **no inter-track
dependencies** at the work-unit level.  Track A doesn't share
files with Track B; Track C doesn't share files with Track D.
A session picks the highest-value unit from any track without
blocking the others.

Why not "do them all sequentially"?  Because Track A (generic
LLVM) is the fastest upstream ROI but is independent of the
backend work, so blocking it on Tracks B/C/D would be pure waste.

```
                Track A (Tier I, U-LLVM)    -- independent
                Track B (Tier II + IV)      -- BIOS / cpnos parity
                Track C (Tier VI gates)     -- #180 + #181 cleanup
                Track D (Tier III + V)      -- Z80 codegen wins packaging
```

Track D is the only one that DOES depend on others — it cannot
ship as a coherent patch series until Track B is zero and Track C
is closed.  Track D's prep work (writing up the 73p wins as
upstream-shaped commits with lit tests) is independent and starts
now.

### Track A — Generic LLVM upstreaming (Tier I)

**Audience:** `llvm/llvm-project` reviewers.
**Why first / independent:** doesn't depend on Z80 target
acceptance; fastest review path; demonstrates we have credible
upstream-ready work.

**Work units, ordered by submission-readiness:**

A1. **#182 ScalarEvolution crash** — minimal repro already exists
    (sequential loops on same array at -O1+).  First drill: reduce
    repro to <50 lines, identify the SmallVector growth site,
    propose patch.  Submit to `llvm/llvm-project` as bug + patch
    candidate.

A2. **#168 SimplifyCFG foldTwoEntryPHINode cost gate** — already
    landed locally as a 12-line patch (commit `cd2a2ace8754`).
    First drill: confirm the patch is target-agnostic (no Z80-
    specific assumptions), write the upstream test case, prepare
    Phabricator/GitHub PR.

A3. **#163 + #165 TruncInstCombine extensions** — landed locally
    (icmp non-const, and-mask).  First drill: write generic
    test cases (i.e. not Z80-target-specific), prep PR.

A4. **#164 TruncInstCombine zext re-insertion cost model** —
    active.  First drill: locate the re-insertion site in
    `AggressiveInstCombine`, sketch the cost model API call.

A5. **#128 underlying pessimization** — currently shipped as a
    fork-only `disablePass()`.  First drill: produce a target-
    agnostic repro showing MachineLICM creating long-live ranges
    on tiny register files; propose the cost-gate API change.
    **Highest design surface of Track A — likely multi-session.**

**Track A exit criteria:** at least 2 PRs in flight to
`llvm/llvm-project` with public review activity.  Queue tracked
in **ravn/llvm-z80#186** (`[meta] Upstream-submission queue`).

### Track B — Z80 correctness sweep + Cluster A residual (Tier II + IV)

**Audience:** us, primarily; result is risk reduction + closing
the BIOS gap.

**Why it must run:** Track D cannot ship as a coherent patch
series with 9 open correctness bugs in the same backend.  Also,
Cluster A (#27 + cluster-mates) is the dominant remaining BIOS
codegen gap.

**Tier II work units (ordered by upstream-blocking risk):**

B1. **NarrowIV trio (#169, #170, #171)** — all three are silent
    miscompiles or timeouts in `Z80NarrowIV` worked around by
    conservative guards.  First drill: re-enable the guards,
    capture the failing MIR for one repro, bisect.  Highest
    leverage because closing the underlying bug lets all three
    guards be lifted (more aggressive narrowing → more BIOS wins).

B2. **#159 silent miscompile (rj_sb_inv chained u8 rotates,
    uses uninit register)** — repro exists in compiler-comparison-
    corpus.  First drill: reduce repro to minimum, inspect MIR
    around the uninit-use site.

B3. **#150 i16 EQ/NE HighByteZero breaks polypascal-test** —
    direct sub_lo extraction path.  First drill: re-run
    polypascal-test with the path enabled, capture the failure
    site.

B4. **#136 pre-existing O1 miscompile (test_90/91 edge_*)** —
    38 failures.  First drill: confirm still reproduces, capture
    the failing fixture's MIR, look for the common pattern.

B5. **#125 Z80LateOptimization crash at -O0** — multi-CALL
    +static-stack +shadow-regs.  First drill: reduce repro, look
    at which peephole crashes.

B6. **#182 ScalarEvolution crash** — already in Track A (A1)
    because the fix lives in generic code.

B7. **#2 IRTranslator crash on "hl" inline-asm constraint** —
    long-standing.  First drill: minimal repro + look at
    IRTranslator's reg-class resolution path.

B8. **#184 i16=2 root cause 1** — still unexplained even after
    #185 fix.  Drill: capture the post-#185 MIR with i16=2
    enabled, find what's wrong.

**Tier IV (Cluster A residual) work units:**

B9. **#27 per-pair 16-bit copy cost drill** — first drill in
    `unpark-2026-05-22.md` Tier B.  Count IX/IY copy instances
    in BIOS + cpnos + autoload; histogram by source (PHI,
    callee-save, spill).  **#27 likely shares root cause with
    #100, #110, #115** — one cost-model fix may close four issues.

B10. **#100 loop-rotation BSS-spill** — gates #77a default-on.
    First drill: identify whether the loop-rotation pass is the
    proximate cause or whether regalloc is "spilling the wrong
    thing."

**Track B exit criteria:** Tier II issue count = 0; #27 cluster
either fixed or has a verified-zero-yield closure note.

### Track C — Cleanup gates for Track D (Tier VI)

**Audience:** future `llvm-z80/llvm-z80` reviewers (and ourselves —
the audit itself surfaces hidden bugs).

**Why it gates Track D:** reviewers will reject a 2300-LOC peephole
file with 16/38 entries that are stand-ins for missing upstream
infrastructure.  Audit + migrate first, then submit the codegen
wins on top of a cleaner base.

**Work units:**

C1. **#180 peephole audit drill** — single-peephole audit pattern.
    First drill (≤30 min): pick the single highest-T-state
    peephole in `Z80LateOptimization.cpp`, locate the upstream
    combiner / ISel pattern that would subsume it, write the
    migration MIR test.  Determine: independent migration, or
    chained dependency?

C2. **#180 audit table** — once C1 is done, expand the audit to
    all 38 peepholes: classify each as Migrate / Keep / Delete.
    Result is a small table + per-peephole notes.

C3. **#181 DAGISel/GISel coexistence audit drill** — confirm
    whether `Z80ISelLowering.cpp` is dead code (it's the DAGISel
    path; GISel is supposed to be the active one).  Result is
    either deletion (large LOC reduction) or a doc explaining
    why both must coexist.

C4. **Peephole migrations from C2** — work through Migrate
    candidates one at a time.  Each becomes a Track A patch
    candidate (because moving a peephole into a generic combiner
    IS a generic-LLVM PR).

**Track C exit criteria:** #180 audit table complete + at least
3 Migrate items shipped; #181 audit complete (either deletion
landed or coexistence rationale documented).

### Track D — Z80 codegen wins packaging (Tier III + V)

**Audience:** `llvm-z80/llvm-z80` review (and eventually
`llvm/llvm-project` once the Z80 target is accepted upstream).

**Why parallel-but-gated:** the work of writing up the 73p wins
as upstream-shaped commits with lit tests is independent of
Tracks B/C.  But submitting them must wait until B is zero and
C is closed.

**Work units:**

D1. **Write up `#179 P1` (`Z80ReorderTestDec`) as patch series.**
    Already committed locally.  First drill: write the lit
    test (already in place per CLAUDE.md), draft the commit
    message with the upstream-coherence rationale (why a new
    MIR pass is necessary; how it relates to MachineScheduler;
    what test cases prove correctness).

D2. **Write up `#179 P2` (ADD_A_A carry forwarding)** as
    addition to D1's series.

D3. **Write up `#185` DJNZ B-clobber safety check.**

D4. **Write up `#148` (peephole #148 fall-through MBB safety
    check from session-73p-issue184).**

D5. **Write up `#177` partial TTI ship (Phase 2).**

D6. **Tier III.b small peepholes** — #117, #122, #141, #146,
    #149, #151, #152.  Each has small individual yield; the
    upstream-coherence question is whether to batch them as one
    series or spread across the timeline.

**Track D exit criteria:** at least one patch series submitted
to `llvm-z80/llvm-z80` for review.

## Sequencing decisions

### What's the right "first move"?

The methodology rule + parallel-tracks structure suggests this
opening sequence:

**Session N+1 (next):**
- 30-min drill on **A1 (#182 ScalarEvolution crash)** — fastest
  Track A win because the repro already exists.
- 30-min drill on **B1 (NarrowIV trio)** — highest-leverage
  Track B item because one fix unblocks three issues.
- 30-min drill on **C1 (#180 single peephole audit)** — the
  Track C / D-gating drill.

Three 30-min drills in one session is realistic; each produces
either a "go" decision (continue work in a follow-up session)
or a revised estimate.  None blocks the others.

### When does Track D start submitting?

Trigger conditions (all must hold):
- Track B Tier II count ≤ 2 (down from 9)
- Track C C1 + C3 drills complete (audit direction known)
- A draft of the #179 patch series exists with lit tests passing

Once these hold, Track D opens its first PR.  Track B + C continue
in parallel after.

### When does this plan say "done"?

End state for goal 1 (clang ≥ SDCC on corpus):
- AES corpus 13/13 faster + smaller than SDCC (already true)
- BIOS clang < SDCC byte count (already true: 5922 vs 6091)
- cpnos clang < SDCC (already true: 2028 vs 2151)
- compiler-comparison-corpus: clang ≥ SDCC on each enabled
  benchmark (sieve, fannkuch, pi today; sorting, binary-trees,
  dhrystone21 after #35 ships)
- BIOS regalloc-residual gap closed (gate on #27 cluster work)

End state for goal 2 (upstream-submittable findings):
- ≥ 2 PRs in flight to `llvm/llvm-project` with positive review
  activity (Track A)
- ≥ 1 patch series submitted to `llvm-z80/llvm-z80` for the 73p
  codegen wins (Track D)
- #180 + #181 audit results documented
- Tier II open count = 0

Neither end-state has a deadline.  The plan is "make every
session move at least one tier-counter."

## Methodology checkpoints

Per the dig-deeper-before-parking rule, every drill produces one
of three outcomes:

- **Go** — drill found a tractable fix, continue work in follow-up
  session
- **Stop** — drill found the fix is blocked by deeper machinery
  (e.g. #178 remat blocker); document the blocker, move to next
  item
- **No-witness** — drill showed the issue doesn't reproduce on
  current binaries; close with re-survey trigger

The 73p Phase 3 closeout table is the canonical record-keeping
format for these outcomes.  Each session's drills get a similar
table in the session summary.

## Value-oracle protection

Every codegen-affecting commit runs (in this order, abort on
failure):

1. `ninja clang llc` — both, per `feedback_ninja_clang_llc_together`
2. `build/bin/llvm-lit llvm/test/CodeGen/Z80/` — currently 108+3
3. `cargo run -- clang` (z80-utils test-runner) — baseline
   681/46/56/207
4. AES corpus 13/13 PASS
5. cpnos PROM1 build + polypascal-test PASS at ~51 s
6. BIOS clang + SDCC builds size-checked (no regressions
   without value justification)

This is non-negotiable; running this loop is the cost of changing
codegen.

## What this plan does NOT cover

- Specific test-runner / MAME orchestration details
- Per-issue investigation methodology (lives in
  `tasks/issue<n>-*.md`)
- Timeline / calendar — none specified
- Resource allocation — single-developer workflow
- Tier VII (attributes/intrinsics/CCs) — listed in coherence
  map but not in active tracks; pick up if a corpus witness
  surfaces

## Decisions to revisit

After **3 sessions** of executing this plan, revisit:

- Are the parallel tracks actually advancing in parallel, or has
  one starved?
- Are the 30-min drills producing useful Go/Stop/No-witness
  decisions, or are they collapsing into multi-hour investigations
  with no exit?
- Have any new issues been filed that change the tier picture?
- Has anything in Tier X (zero-yield) re-surfaced as a witness?

If yes to any of these, refresh `upstream-coherence-map-*.md` and
this plan.

## Reprioritization 2026-05-25 (user directive)

**Primary focus is the Z80 backend -> `llvm-z80/llvm-z80`.** The
"upstream-upstream" `llvm/llvm-project` work is deferred until the
backend is submittable — *except* generic bugs that **interfere with
the Z80 backend**, which stay in scope because they block it.

This **demotes Track A**: the §"first move" recommendation to lead with
A1/A2/A3 (generic-LLVM PRs) is now wrong for this priority. Track A
shrinks to only its interfering subset:
- **#128** — MachineLICM long-live-ranges pessimization, currently masked
  by a fork-only `disablePass()`; directly degrades the Z80 backend.
- **#182** — ScalarEvolution crash that fires on the Z80 corpus build.

The rest of Track A (TruncInstCombine, SimplifyCFG cost gate) is pure
density help with no backend dependency -> wait. **New order of the day:
Tracks B (correctness) + C (cleanup gates) + D (packaging); A only as
needed to unblock B/C/D.**

### #189 reclassified (drill 2026-05-25)

`issue189-27-regalloc-cost-model-drill-2026-05-25.md` (GO). The
IX/IY-as-GPR regalloc gap has **two faces split on `+static-stack`**:
- **Tier IV density** under `+static-stack` (production): IY-on is
  value-correct (36/36 repros PASS all opt levels) but bloated by
  `push iy`/`pop rr` shuttles.
- **Tier II correctness** in the default (IX-frame) config: IY-on
  **miscompiles** (test_168 -> `0x0044` @ -O1/-Os) and **hangs**
  (test_167 @ -O2/-O3), because the IY push/pop shuttle collides with
  stack-based spill/reload.

Root cause (both): `GR16` allocation order includes IY, so a 16-bit
value that gets **byte-decomposed** (`LSHR16`/`.sub_lo`/`XOR_CMP_EQ16`)
can land in IY, forcing a shuttle the cost model can't price. **One fix
for both:** constrain those byte-accessed operands to `GR16NoIR` at
ISel (register classes express legality; costs only express preference —
confirmed against `CodeGenerator.html` + `Target.td`). Likely the shared
root of **#27 / #110 / #115** too. Next: reduce the default-config
miscompile to a minimal `llc` lit XFAIL + file the ravn/llvm-z80 issue,
then implement the `GR16NoIR` constraint with a pressure histogram.
