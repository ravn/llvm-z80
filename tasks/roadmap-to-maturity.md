# Roadmap to maturity for llvm-z80/llvm-z80

**Date:** 2026-05-02 (session 36).
**Branch:** `session-36-code-density-plan` in `ravn/llvm-z80`.
**Author:** ravn (Thorbjørn) with Claude Code assistance.
**Status:** master plan; supersedes the framing in
`plan-code-density-2026-05-02.md` and unifies the views in
`fix-plan.md`, `backend-completion-roadmap.md`,
`phase-c-regalloc-investigation.md`, `peephole-vs-root-cause.md`,
`source-cleanup-vs-closed-issues.md`.

## Phase status (verified 2026-05-03)

**Update 2026-05-03 (post-triage + post-correctness sweep):** the
session-36 framing of "Phase 1 is next" is stale.  Current state:

| Phase | Status |
|---|---|
| Phase 1 — Foundation | **DONE.**  CI workflow `.github/workflows/z80-ci.yml`, size baseline `tasks/size-baseline.py`, late-opt audit (session 37), source-cleanup audit (session 34) all landed. |
| Phase 2 — Correctness | **4 of 5 closed.**  #28, #36, #63, #81 closed 2026-05-02/03.  #38 remains. |
| Phase 3 — Cluster A regalloc | **Next active workstream.**  See section 12.3. |
| Phase 4 — Cluster B spill mechanism | Mostly stale; only #100 live.  See section 12.4 update + `tasks/triage-2026-05-03-cluster-b.md`. |
| Phase 5+ | Not yet active. |

Engagement-mode (section 10.2) gate is two threads away: close #38
plus close one cluster (Phase 3 candidate).

Current tactical plan: `tasks/plan-2026-05-03-structural.md`.

## 0.  TL;DR

**Goal:** Bring `llvm-z80/llvm-z80` (the active fork-of-record at
github.com/llvm-z80/llvm-z80, owner @zlfn) to a mature state by
collaborating with @zlfn.  "Mature" means: correct, testable,
upstream-LLVM-quality where reasonable, and scoped so that
eventual submission to `llvm/llvm-project` as a new experimental
target becomes feasible.  Submission itself is a long-term
aspiration, not a near-term action.

**Working model:** ravn/llvm-z80 is the working repo.  Each fix
follows the established protocol with @zlfn:
  1. Discover and stabilize the change locally on
     `ravn/llvm-z80`.
  2. Open an issue on `llvm-z80/llvm-z80` describing the problem.
  3. Submit a PR on `llvm-z80/llvm-z80` once the change is review-
     ready.
  4. Track in our own `ravn/llvm-z80` issue tracker the workspace-
     specific work.

**Quality bar:** LLVM coding standards (clang-format, license
headers, lit tests with FileCheck on `*.ll` IR fixtures, no
BIOS/cpnos-rom dependence in upstream-bound tests).  AI-assisted
contributions are accepted by @zlfn but human review is required
("contributors must remain responsible for the code they submit").

**Sequence:** correctness first → cluster pessimizations →
late-opt audit → CI/test infrastructure → core-LLVM-diff
reduction.

## 1.  Why this plan exists

The work in this workspace has progressed through 35+ sessions of
incremental Z80 backend improvements: peepholes, regalloc hints,
target-specific passes (`Z80LoopRotate`, `Z80LoopIdiomFill`),
TableGen patterns, calling conventions.  Sizes have improved
substantially (BIOS now 5920 B clang vs 6123 B SDCC; cpnos-rom
1708 B), but the work has been done in fork-only mode without a
coherent strategy for upstream submission.

The user clarified the strategic goal 2026-05-02:

> "I want to collaborate with the owner of the llvm-z80/llvm-z80
>  project to reach maturity and then see if it can be submitted
>  to the official project."

The plan responds to that direct ive by:
  - Identifying who/what to collaborate with (Section 2).
  - Defining "mature" concretely (Section 3).
  - Mapping every open issue to a workstream (Section 4-9).
  - Specifying the collaboration protocol (Section 10).
  - Specifying the quality bar (Section 11).
  - Sequencing the work realistically (Section 12).
  - Documenting risks and contingencies (Section 13).
  - Treating upstream LLVM as a long-term aspiration with a
    different acceptance bar (Section 14).

## 2.  Collaboration target: llvm-z80/llvm-z80 (@zlfn)

### 2.1  Repository facts

  - URL: https://github.com/llvm-z80/llvm-z80
  - Description: "An LLVM backend implementation targeting the
    Zilog Z80 processor"
  - Owner: GitHub org `llvm-z80`, sole member @zlfn
  - Maintainer identity: Kiroo (Chanung), POSTECH, Korea.  Bio:
    "I don't know what i'm doing"; 88 followers; 80 public repos;
    GitHub since 2017.
  - License: "Other" (almost certainly Apache 2 with LLVM
    exceptions, inherited).
  - Activity: pushed 2026-04-25 (recent maintenance commit: `[Z80]
    Fix build after Combiner API change`).
  - 35 stars, 3 forks, 1 org member, 16 issues + 9 PRs (as of
    2026-05-02), all merged.
  - Default branch: `main`.

### 2.2  Predecessor history (for reference)

  - `jacobly0/llvm-z80` (the original Z80 LLVM project) was archived
    in 2017-11.  Last substantive Z80 work happened on
    `jacobly0/llvm-project:z80` and went dormant 2023-11.
  - `llvm-z80/llvm-z80` was started by @zlfn building on the
    inherited tree.  Per their issue #13 ("Track and reduce core
    LLVM modifications"), some of the inherited core-LLVM diff is
    being audited for upstream eligibility.
  - `ravn/llvm-z80` is a fork of `llvm-z80/llvm-z80` (verified via
    GitHub parent metadata).

### 2.3  Established collaboration channel

@zlfn and ravn have an open dialogue (llvm-z80/llvm-z80#8
"Possible contribution.", opened 2026-03-24).  Key statements from
that thread:

@zlfn:
  > "All contributions are more than welcome! ... contributions
  >  using AI are perfectly fine as well — provided there are
  >  sufficient test cases and at least a basic human review."
  > "this project's standards may be slightly less stringent than
  >  those for upstream LLVM contributions, [but] it is essential
  >  for contributors to have a understanding of the code they and
  >  the AI have produced."
  > "if you're joining the effort, I agree it would be best to
  >  transition to a more standard routine: raising issues first
  >  and then submitting Pull Requests (PRs)."

@zlfn's own AI workflow (paraphrased from the same thread):
  1. Initial implementation: AI-written test suites + high-level
     review before commit; periodic re-audit.
  2. Optimization: compare LLVM vs SDCC output, AI-research, AI
     implementation, full regression tests + benchmark before
     manual review and commit.
  3. Recursive review: occasional AI sweep of the entire backend;
     most flags are false positives but some real issues found.

ravn has already had **6+ closed issues and PRs** in
llvm-z80/llvm-z80 (issues #10, #11, #12, #14, #15, #16) and the
relationship is established.

### 2.4  @zlfn's own roadmap (issue #13)

@zlfn has a tracking issue ("Track and reduce core LLVM
modifications") that classifies the inherited core-LLVM diff into:

  1. **LSR + SCEV changes** — illegal-integer cost penalties, narrow-
     IV preference, narrow-type expansion in `SCEVExpander`.
     "Could potentially be upstreamed."
  2. **TTI hooks** — `preferNarrowTypes()`, `isZExtFree()`.
  3. **TargetLowering** — `isSuitableForBitTests` SHL check,
     virtual `getRegisterType(MVT)`, `preferNarrowTypes()` for
     CodeGen passes.
  4. **GlobalISel** — `CombinerHelper` legality guards,
     `LegalizationArtifactCombiner` fixes, `LegalizerHelper`
     i8/i16 libcall mapping.  "Strong upstream candidate."
  5. **RegAlloc** — `RS_LightSpill` stage, eviction heuristics,
     `canWiden` helper, post-spill not-spillable.  "Difficult to
     upstream due to sensitivity of RegAlloc changes."
  6. **Misc** — `getPredictableBranchThreshold` checks,
     `InlineSpiller` `setIsDead()`, `RegisterScavenging` 25-insn
     limit removal.

This issue is the **alignment document** between our work and
@zlfn's direction.  Our PRs should be designed to complement this
roadmap, not contradict it.  Where our work would add to the
core-LLVM diff, that's a discussion point with @zlfn first.

## 3.  What "mature" means

A definition is necessary before the plan can claim completion.
Maturity criteria, in priority order:

### 3.1  Correctness — non-negotiable

  - All open correctness issues closed: **#28, #36, #38, #63,
    #81**.  Five issues; mix of -O0 path bugs, va_arg ABI, IY
    sub-register handling, assembler lex.
  - All four `-O0..-Oz` opt levels produce correct code on the
    real-workload test corpus (rcbios, cpnos-rom, autoload-PROM if
    still present).
  - Lit suite remains 100% PASS (currently 76 PASS + 1 XFAIL for
    #99).  No silent XFAIL accumulation.
  - MAME smoke tests pass for rcbios and cpnos-rom builds at every
    PR-merge boundary.

### 3.2  Test coverage

  - Each backend area (16 per `backend-completion-roadmap.md`) has
    at least one regression-locking lit test.
  - Each closed issue has either a closed-form lit test (preferred)
    or a justification why it can't have one (e.g., requires
    multi-MBB analysis).
  - CI (GitHub Actions) gates PRs on the full Z80 lit suite.  Two
    historical regressions on `lshr-rrca.ll` and
    `switch-byte-field.ll` would have been caught by this; per
    `fix-plan.md`'s infrastructure follow-ups.

### 3.3  Late-opt peephole layer audited

Per `peephole-vs-root-cause.md` and confirmed by peer-target
precedent (ARM/M68k):  late-opt is acceptable for **Z80-ISA-
specific patterns with no IR/MIR representation** (`EX DE,HL`,
`BIT n,A`, `SBC A,A`, `JR` vs `JP`, `RRCA`, `EX (SP),IX/IY`).
Anything else is a candidate for migration upstream.

Audit deliverable: each of the ~50+ peepholes in
`Z80LateOptimization.cpp` (currently 5,272 LOC) classified as
**keep** / **migrate** / **delete** with rationale.  Migration
PRs follow.

### 3.4  Source workarounds verified

Per `source-cleanup-vs-closed-issues.md`: each closed issue's
source-side workaround in `rcbios-in-c`, `cpnos-rom`, etc., is
verified still needed or removed.  This catches "the bug was
fixed but I forgot the source workaround" rot.

### 3.5  Core-LLVM diff bounded

Per zlfn's issue #13:  every core-LLVM modification is either
upstreamed, replaced by a target hook, or documented with rationale.
We do NOT add new core-LLVM modifications without a discussion-first
approach with @zlfn.

### 3.6  Reproducible builds

  - Docker images documented and current (`llvm-z80-build`,
    `llvm-z80-test`).
  - Native macOS build path documented.
  - Test runner (Rust `z80-utils/test-runner`) green at each PR.

### 3.7  Documentation

  - `llvm-z80/llvm-z80` README accurately describes status.
  - Per-backend-area documentation (where missing) added.
  - Code-of-conduct, CONTRIBUTING.md (currently generic LLVM —
    consider adding Z80-specific notes via PR if @zlfn agrees).

### 3.8  Code-density target (informational, not a maturity gate)

Code density is a measurement of completeness, not a goal in
itself.  But for tracking, current state:

  - BIOS: clang 5920 B vs SDCC 6123 B — clang ahead by 203 B.
  - cpnos-rom payload: 1708 B.
  - PROM: clang 1756 B vs SDCC 1910 B (per CLAUDE.md historic).

These numbers should not regress during maturity work.  CI
baseline tracker (Section 5) makes the regression visible.

## 4.  Backend audit summary (from session-36 research)

Per the audit subagent's findings, the backend (~27K LOC across 79
files in `lib/Target/Z80/`) breaks down as:

| Area | LOC | Maturity | Notes |
| --- | --- | --- | --- |
| Top-level target description | ~244 | 5/5 complete | All CPU variants and features defined |
| Instruction definitions (TableGen) | ~3148 | 4/5 mostly | `Z80InstrGISel.td` minimal (73 LOC) |
| Register definitions | ~920 | 5/5 complete | RegisterInfo handles GR8/GR16/FP/shadow correctly |
| Calling conventions | ~1309 | 5/5 complete | Both sdcccall(0)/sdcccall(1) implemented |
| Legalization (GISel) | 1487 | 4/5 mostly | One `report_fatal_error` (line 571, likely float intrinsic) |
| Register Bank Info | 70 | 1/5 stub | Single AnyRegBank; acceptable for Z80 |
| Instruction Selector | 5488 | 4/5 mostly | 3× `llvm_unreachable` for i32+ merge/unmerge |
| Instruction Info | 2494 | 4/5 mostly | 9× `llvm_unreachable` for unsupported reg classes |
| Register Info | 1774 | 4/5 mostly | Frame elimination edge cases use `llvm_unreachable` |
| Frame Lowering | 490 | 4/5 mostly | Static-stack working; 2× `llvm_unreachable` |
| DAG Lowering (legacy) | 235 | 1/5 stub | GISel primary; DAG-only fallback unimplemented |
| TargetTransformInfo | 100 | 5/5 complete | 7 overrides incl. LSR cost weighting |
| Pseudo Expansion | 490 | 4/5 mostly | ~50 pseudo opcodes covered |
| Late Optimization Peephole | 5272 | 4/5 mostly | 50+ patterns; well-structured |
| Auxiliary IR-level passes | 1285 | 5/5 complete | LoopRotate, IndexIV, LowerSelect, etc. |
| MC Layer (asm/disasm/ELF) | 1184 | 5/5 complete | Full ELF object writer + assembler |
| Inline asm lowering | 55 | 4/5 mostly | Minimal; delegates to base class |

**Verdict:** ~80% production-ready.  Top concerns:

  - **Late-opt size** (5272 LOC).  Not bad per se — peer targets
    have similar — but each peephole needs the audit per Section
    3.3.
  - **`Z80InstrGISel.td` minimal** (73 LOC).  Most patterns done in
    `Z80InstructionSelector.cpp` C++ rather than TableGen.  Worth
    asking @zlfn about the strategic preference; some C++ ISel
    code may be migrate-able to TableGen for upstream readability.
  - **DAG layer stub** (`Z80ISelLowering.cpp` 235 LOC).  Acceptable
    for GISel-primary, but if upstream submission ever happens, the
    DAG fallback is conventionally provided.

## 5.  Issue inventory and clusters (from triage agent)

27 open issues in `ravn/llvm-z80`.  5 correctness, 22 pessimizations.

### 5.1  Correctness bugs (closes maturity criterion 3.1)

| # | Title | Area | Difficulty |
| --- | --- | --- | --- |
| #28 | -O0 codegen failures in large functions | RegAlloc -O0 path / FastRegAlloc | L (investigate first) |
| #36 | va_arg produces incorrect code — printf broken | CallLowering / Legalizer | M |
| #38 | Large fn codegen needs +undocumented (IY PUSH/POP) | InstrInfo / RegisterInfo / FrameLowering | L (layout-sensitive) |
| #63 | bench_string fails at -O0 only | Likely sibling of #28 | M-L |
| #81 | Integrated assembler rejects `ex af, af'` | MC parser | S |

### 5.2  Pessimization clusters

Six clusters per the triage agent:

**Cluster A: regalloc — counter/pointer hint quality** (~6 issues)
  - #98 (investigation, gates #94)
  - #94 (sequential DJNZ)
  - #89 (loop-invariant in DE clobbered)
  - #99 (i16-counter BC ping-pong)
  - #27 (per-pair 16-bit copy cost)
  - #16 (PUSH/POP vs IX-indexed across CALLs)

**Cluster B: spill-mechanism — PUSH/POP vs BSS** (~5 issues + 2 closed predecessors)
  - #20 (multi-value cross-CALL)
  - #100 (rotation-around-CALL — gates #77 default-on)
  - #96 (regalloc-level layer-3 — investigation)
  - #12 (hasFP=false re-evaluation; depends on B+C)

**Cluster C: -O0 correctness** (~2 issues)
  - #28, #63 — share root cause (FastRegAlloc / spill-slot bug)

**Cluster D: loop-shape / IV-form** (~5 issues)
  - #15 (index→pointer conversion)
  - #50 (LDIR unroll)
  - #77 (8-bit countdown DJNZ; gated by #100)
  - #95 (countdown→count-up IV rewrite — long-term TTI)
  - #7 (umbrella, mostly closed)

**Cluster E: CP/M-ABI ergonomics** (~5 issues)
  - #4 (`__critical` DI/EI wrapper)
  - #42 (intrinsics for DI/EI/HALT/IM2/LD I,A)
  - #43 (custom CC for BC/DE/HL params)
  - #36 (va_arg — also correctness)
  - #35 (no libc — runtime, not backend)

**Cluster F: MC / diagnostics** (~2 issues)
  - #70 (`-fverbose-asm` no source comments)
  - #81 (assembler `af'` lex — also correctness)

**Cluster G (umbrella, mostly informational)**
  - #7 (DJNZ/LDIR/CPIR/CP(HL) umbrella)
  - #18 (known-value reg copy)
  - #40 (per-function frame choice; depends on B+C)

### 5.3  Cluster ordering rationale

  - **Cluster C first**: -O0 correctness blocks confidence in the
    backend.  Investigate the shared root cause; one fix likely
    closes both.
  - **Cluster A second**: regalloc cluster is the most leveraged
    pessimization work.  #98 is the gating investigation; one good
    cost-model change can close 4-5 issues.
  - **Cluster B in parallel with A**: spill-mechanism work composes
    with regalloc.  #100 unblocks #77 (rotation default-on).
  - **Cluster D after A+B**: loop/IV form depends on regalloc being
    behaving correctly.
  - **Cluster E last (mostly)**: CP/M-ABI is user-visible feature
    work, not codegen quality.  #36 (va_arg) is correctness so
    pulled into Section 3.1.
  - **Cluster F any time**: small wins; fit in gaps.

## 6.  Per-issue maturity-relevance assessment

Of the 27 open issues, classify each by relevance to maturity
criteria 3.1-3.8:

  - **Maturity-critical (must close):** #28, #36, #38, #63, #81
    (correctness 3.1).
  - **High-leverage (closing multiple via cluster):** #94, #98,
    #99, #100 (regalloc cluster A).
  - **Medium-leverage (closing one each):** #4, #15, #16, #18, #20,
    #27, #42, #43, #50, #70, #77, #89, #96.
  - **Lower priority (defer until others close):** #7 (umbrella),
    #12 (depends on B+C), #35 (runtime), #40 (depends on B+C+#38),
    #95 (long-term TTI).

## 7.  Backend-area completion priorities

Mapping issues × areas, the highest-impact areas to invest in:

  1. **RegisterInfo + RegAlloc cost model** (Cluster A + B + C):
     8-9 issues touch here.  This is the single highest-leverage
     area.
  2. **CallLowering + Legalizer** (Cluster E correctness): #36 + #38.
  3. **TTI** (Cluster D long-term): #15, #95.  TTI exists per audit
     (100 LOC, 7 overrides); needs new hooks per zlfn's #13.
  4. **MC layer**: #70, #81.  Small focused fixes.
  5. **InstrInfo / TableGen**: incremental — `isReMaterializable`
     audit, intrinsics for #42.

## 8.  Specific design notes from peer-target research

Per session-36 peer-precedent agent:

### 8.1  DJNZ as a "primary" instruction

Originally I proposed (in `phase-c-regalloc-investigation.md`)
making DJNZ a primary ISel opcode.  Peer precedent says **no**:
ARM's CBZ/CBNZ analogue is selected via **post-RA fusion**
(`ARMConstantIslandPass.cpp:1906-1942`), not as a primary opcode.
M68k's DBcc is unimplemented in LLVM upstream (the Z80 backend
already does better).

The right Z80 design is a **DJNZ_LOOP pseudo emitted by ISel**
when the counter shape matches, plus a register-allocation hint
that prefers B for the counter, plus the existing post-RA
peephole that fires when B did get allocated.  This is what
@zlfn's existing pseudo-expansion approach already does for the
hand-written DJNZ pseudos (MUL8, DIV8, variable shift).

Cluster A.1 (DJNZ-as-primary) is therefore **not the right
framing**.  The correct framing is:
  - Improve `getRegAllocationHints` so the counter virtreg gets
    the prefer-B hint correctly (#92 already wired one case;
    extend for sequential loops #94 and i16-counter #99).
  - Audit the existing post-RA peephole at
    `Z80LateOptimization.cpp:760-803` — if its conditions are
    correct, it fires whenever B is allocated to the counter.
  - For #100 (rotation-around-CALL): the spill problem is in the
    same family — when B is the loop carrier and a CALL is in
    the loop body, the carrier gets BSS-spilled.  Fix at the
    spill-mechanism layer (Cluster B), not at DJNZ-as-primary.

This is a **plan correction** from earlier in the same session.

### 8.2  Rematerialization

ARM/X86/AVR pattern: `let isRematerializable = 1` in TableGen +
`isReMaterializableImpl` override in `XInstrInfo.cpp` for opcode-
specific logic.  Per audit, Z80 already has 4+ entries with the
TableGen flag.  Audit needed:

  - `LD r,nn` (8-bit immediate) — should be remat
  - `LD HL,nn` / `LD BC,nn` / `LD DE,nn` (16-bit immediate) — should
    be remat
  - `LD r,(nn)` for non-volatile globals — should be remat (cheaper
    than spill+reload by a factor of ~2.6 in bytes)
  - `LD HL,(nn)` similar

Cost model coordination: the regalloc compares `isRematerializable
&& isReMaterializableImpl(MI)` against spill cost.
`getInstSizeInBytes` must return correct values for both sides.

### 8.3  TTI for IV form

AVR has `isLSRCostLess` override (`AVRTargetTransformInfo.cpp:13-24`).
Per audit, Z80's TTI (`Z80TargetTransformInfo.h`) has 7 overrides
including LSR cost weighting.  Open question: does it cover the
specific countdown-vs-count-up case raised by #95?  Investigation
needed during Cluster D work.

### 8.4  Late-opt vs ISel boundary

Peer precedent confirms Z80's existing structure
(`Z80ExpandPseudo` for pseudos, `Z80LateOptimization` for
peepholes) **matches** ARM/M68k.  The 5272 LOC late-opt isn't
inherently a problem.  The audit per Section 3.3 determines which
peepholes belong vs which migrate.

## 9.  Test strategy

### 9.1  Lit tests (upstream-bound)

Per existing `fix-plan.md` "Per-issue test strategy":

  - One file per issue or one per cluster.  Path:
    `llvm/test/CodeGen/Z80/<topic>.ll`.
  - `; RUN: llc -mtriple=z80 -O2` (or `-Oz` / target-feature flags
    as appropriate).
  - `; CHECK-NOT:` the bad pattern, `; CHECK:` the expected good
    pattern.
  - `; XFAIL:` line allowed when landing test before the fix —
    flips PASS when the fix lands.
  - IR fixtures, not C source.  No BIOS/cpnos-rom dependence in
    upstream-bound tests.

### 9.2  Integration tests (workspace-only)

`z80-utils/test-runner` (Rust) covers the C-source test corpus
including bench_string, the Clang/SDCC cross-build suite, code-
size benchmarks.  Used for regression detection but not
upstreamed.

### 9.3  Real-workload tests (workspace-only)

  - rcbios-in-c BIOS build + MAME boot smoke.
  - cpnos-rom build + MAME launch + CCP banner verification.
  - z80-utils/test-runner bench mode for size deltas.

### 9.4  CI

Per `fix-plan.md` infrastructure follow-ups: GitHub Actions for
Z80 lit suite gating.  Expand to llvm-z80/llvm-z80 as well, gated
on path filters (`llvm/lib/Target/Z80/**`,
`llvm/test/CodeGen/Z80/**`, `clang/cmake/caches/Z80.cmake`).

## 10.  Collaboration protocol with @zlfn

**Timing constraint (user directive 2026-05-02):**
Do **not** engage `llvm-z80/llvm-z80` (file upstream issues, open
PRs, comment on @zlfn's tracking issues, etc.) until **something
substantial is ready**.  Premature engagement wastes maintainer
attention on speculative directions.

### 10.1  Phase split: workspace mode → engagement mode

Two distinct phases of operation:

**Workspace mode (current):**
  - All work on `ravn/llvm-z80` working branches.
  - Issue tracking on `ravn/llvm-z80` only.
  - No issues, PRs, or comments on `llvm-z80/llvm-z80`.
  - No comments on @zlfn's tracking issues (#13, etc.).
  - Quality bar still LLVM-grade so the work is presentable when
    engagement mode opens.

**Engagement mode (after substantial body of work is ready):**
  - Coordinated batch delivery to `llvm-z80/llvm-z80`.
  - Per @zlfn's preference (issue #8 thread): open issue first,
    then PR.
  - Multi-PR clusters: state the plan in the cluster tracking
    issue first so @zlfn can give early feedback.
  - AI-assisted contributions accepted with test cases + human
    review.

### 10.2  Definition of "substantial"

Engagement mode opens when **at minimum** the following are true:

  - All correctness bugs (#28, #36, #38, #63, #81) closed locally
    with lit tests proving each fix and no regressions on the lit
    suite or real-workload tests.
  - One coherent cluster of pessimization fixes (likely Cluster A
    regalloc or Cluster B spill mechanism) closed locally with the
    same standard.
  - Test infrastructure (CI, size baseline) ready to demonstrate
    the work doesn't regress.
  - A coordinated narrative for the upstream issues + PRs: each
    one's purpose, dependencies, expected reviewer effort.

In rough plan-phase terms: engagement mode is appropriate after
**Phase 2 (correctness sweep) plus at least Phase 3 substantially
progressed**, not before.

### 10.3  Engagement-mode protocol (for reference; do not act on
this until substantial)

When engagement mode opens, the per-fix protocol becomes:
  1. Discover, prototype, stabilize on `ravn/llvm-z80`.
  2. Update `ravn/llvm-z80` issue with the workspace view.
  3. Open `llvm-z80/llvm-z80` issue: minimal repro, observed vs
     expected, proposed fix outline.  Wait for @zlfn
     acknowledgement (or 1-2 days) before opening the PR.
  4. Submit `llvm-z80/llvm-z80` PR with: targeted commits, lit
     test, review-ready commit message.
  5. After merge, sync `ravn/llvm-z80` from
     `llvm-z80/llvm-z80:main`.

### 10.4  For core-LLVM modifications

Anything that touches files outside `llvm/lib/Target/Z80/` is a
core-LLVM modification.  Per zlfn's #13 (which we are NOT
commenting on yet — see 10.1):

  - Discuss with @zlfn (in engagement mode) before opening a PR.
  - Justify why the change is needed and why a target hook is
    insufficient.
  - Prefer adding a target hook to making generic-pass changes.

### 10.5  AI-assisted contribution etiquette

Per @zlfn's stance (issue #8):
  - AI assistance is welcome.  Test cases required.  Human review
    is essential.
  - Don't tag PRs as "AI-generated" — the standard is "well-
    reviewed code", not "human-only code".
  - `Co-Authored-By: Claude ...` trailers in commit messages are
    fine per existing project convention.

### 10.6  Collaboration cadence (engagement mode)

  - Don't surprise @zlfn with large PRs.
  - For multi-PR clusters, state the plan up front.
  - Be patient — @zlfn is a solo maintainer.  Response cadence in
    #8 was 1-3 days.

## 11.  Quality bar

### 11.1  Code

  - LLVM Coding Standards
    (https://llvm.org/docs/CodingStandards.html).
  - clang-format clean per LLVM `.clang-format`.
  - License headers (Apache 2.0 with LLVM exceptions, matching
    existing files).
  - Naming: PascalCase for types, camelCase for functions, etc.
  - Comments: sparingly, doxygen for new public functions.
  - No `// AI-generated` markers, no fork-only shortcuts.

### 11.2  Architecture

  - Prefer TableGen patterns for instruction selection where
    feasible.  C++ ISel code is acceptable but increases review
    surface (per zlfn's preference; verify in flight).
  - GISel combiners for IR-level patterns where the pattern is
    expressible in GISel terms.
  - Target hooks (TTI, TargetLowering, RegisterInfo) for
    generic-pass cost-model influence.
  - Late-opt peepholes for Z80-ISA-only patterns with no IR
    representation (audit per Section 3.3).

### 11.3  Testing

  - Lit tests required for every codegen change.
  - Tests use IR fixtures, FileCheck directives.
  - XFAIL only when test is landed before fix.
  - Real-workload regressions caught by `test-runner` and CI.

### 11.4  Reverting

Per user's 2026-05-02 directive:

  > "If this means reverting previously work, that is fine."

When a prior fork-only fix wouldn't survive review, reverting and
rewriting is acceptable.  Examples likely to face this:
  - Some of the more pattern-specific peepholes in
    `Z80LateOptimization.cpp` that target shapes only seen in
    rcbios/cpnos-rom.
  - Workarounds that contort source to avoid backend bugs (per
    `source-cleanup-vs-closed-issues.md`).

## 12.  Sequenced execution

Estimated effort assumes 1-2 hours/day of focused work, AI-
assisted, with weekend bursts.  Calendar weeks not full-time
weeks.

### 12.1  Phase 1 — Foundation (1-2 weeks)

  - **CI for Z80 lit suite** — GitHub Actions workflow for
    `llvm-z80/llvm-z80` and `ravn/llvm-z80` (per `fix-plan.md`
    infrastructure follow-ups).  PR to llvm-z80/llvm-z80.
  - **Per-function size baseline tracker** — script to extract
    function sizes from BIOS / cpnos-rom / PROM and report
    regressions.  Workspace-only (does not need upstreaming).
  - **Late-opt audit** — read `Z80LateOptimization.cpp` end-to-end,
    classify each peephole as keep/migrate/delete per Section 3.3.
    Output: `tasks/late-opt-audit-2026-05-02.md`.
  - **Source-cleanup audit** — per existing
    `source-cleanup-vs-closed-issues.md`; verify each closed-issue
    workaround.  Workspace-only.

Exit: CI green; size baseline locked; audit docs landed.

### 12.2  Phase 2 — Correctness sweep (2-4 weeks)

Issues: #28, #36, #38, #63, #81.

Order:
  1. **#81 first** (S, MC parser).  Quick win; useful as
     warm-up for the @zlfn collaboration cadence.
  2. **#36 second** (M, va_arg).  Focused ABI bug; narrow scope.
  3. **#28 + #63 third** (L investigation).  Likely shared root
     cause in FastRegAlloc / spill-slot at -O0.  One investigation,
     one fix-set.
  4. **#38 last** (L, layout-sensitive IY bug).  Highest risk; the
     longest individual fix; allows IY/IX to be re-enabled as
     allocatable per CLAUDE.md.

Exit: zero correctness issues open; lit tests + MAME smoke green
on all four opt levels.

### 12.3  Phase 3 — Cluster A (regalloc) (3-5 weeks)

Order:
  1. **#98 investigation first** — answer "why doesn't regalloc
     model B as dead between sequential DJNZ-eligible loops?"
     Output: design doc.
  2. **#94 fix** — based on #98 findings.  Likely
     `getRegAllocationHints` extension or live-range change.
  3. **#89 fix** — likely a sibling of #94 in the regalloc cost
     model (don't spill counter into D when D holds loop-invariant
     constant).
  4. **#99 fix** — i16-counter BC ping-pong.  Reg-class swap-hint
     or peephole rewrite.
  5. **#27 evaluation** — per-pair 16-bit cost.  May be subsumed
     by 1-4 above.

Exit: 4-5 issues closed; rcbios + cpnos-rom sizes improved or
unchanged (no regression).

### 12.4  Phase 4 — Cluster B (spill mechanism) — overlaps Phase 3

**Updated 2026-05-03 post-triage** (see `tasks/triage-2026-05-03-cluster-b.md`):
the original ordering below included two owner-downgraded issues
and one investigation-only issue, and miscategorised #16 (which
section 5.2 places in Cluster A).  Replaced with the post-triage
membership:

  1. **#100 fix** — rotation-around-CALL spill.  Extend BSS-spill
     ->PUSH/POP peephole for cross-back-edge.  Closes the gate on
     #77 default-on.  **The only live, implementation-ready Cluster
     B item.**
  2. **#12 evaluation** (the actual Cluster B item per section 5.2;
     was missing from this list).  hasFP=false re-evaluation
     blocked on Cluster A + B progress.  Likely subsumed by remat
     + cost-model changes from Cluster A; may close as wontfix.
  3. **#96 investigation** — regalloc-level layer-3 PUSH/POP
     spilling.  Filed as exploration only ("no deadline; lower
     priority than #77 and the active regalloc cluster").  May
     land later as Cluster B.5 if (a) Cluster A doesn't subsume
     and (b) cross-BB LIFO bracketing investigation produces a
     workable design.

Retired from this phase (still open, but reclassified):

  - **#20** — owner-downgraded April 2026 ("deprioritize in favor
    of #43 or wait for regalloc upstream"); 200+ LOC dataflow for
    ~12 B with stack-corruption risk class (cf. closed #41).
    Track as parked; revisit only if #43 (custom CC) lands and
    changes the spill landscape.
  - **#16** — moved to Cluster A per section 5.2 (the original
    placement; the prior version of this section listed it here
    in error).  Owner-downgraded March 2026 from ~40 B to ~6-8 B;
    "no peephole fix is practical".  Subsumed by Cluster A
    regalloc work.

Exit: #100 fixed; rotation-around-CALL no longer regresses sizes;
#77a default-on can flip if Cluster D (#95) also progresses.

### 12.5  Phase 5 — Cluster D (loop/IV form) (2-4 weeks)

Order:
  1. **#15 fix** — loop index→pointer conversion via TTI hook or
     IR-level pass.  ~90 B PROM impact.
  2. **#95 fix** — countdown-vs-count-up TTI override.  Closes a
     long-running issue and removes the corresponding post-RA
     peephole (#93's path-b).
  3. **#77 default-on flip** — `Z80LoopRotate` `cl::init(true)` if
     #100 + #95 cooperated.
  4. **#50 fix** — LDIR unroll for speed-critical paths.

Exit: rotation default-on; loop-shape pessimizations closed.

### 12.6  Phase 6 — Cluster E (CP/M ABI) (1-2 weeks)

Order:
  1. **#42 fix** — DI/EI/HALT/IM2/LD I,A intrinsics.  S work; opens
     up #4.
  2. **#4 fix** — `__critical` attribute building on #42.
  3. **#43 design + RFC discussion** — custom CC for BC/DE/HL is
     larger; coordinate with @zlfn first.
  4. **#35 deferred** — runtime/sysroot work; may be out of
     project scope until everything else lands.

### 12.7  Phase 7 — Cluster F + remaining (1 week)

  - **#70** verbose-asm source comments.
  - **#18** known-value reg copy (already has session-32 imm-form
    closed; reg-form remains).
  - **#7 umbrella** close as subsumed.

### 12.8  Phase 8 — Late-opt migration

By this phase, engagement mode (Section 10.2) has likely opened.

For each peephole classified "migrate" in the Phase 1 audit:
  - In engagement mode: open `llvm-z80/llvm-z80` issue describing
    the upstream-area target.  In workspace mode (if engagement
    hasn't opened yet): track on `ravn/llvm-z80` only.
  - Implement the upstream-area fix.
  - Land the upstream-area fix (in `ravn/llvm-z80` first; promote
    to `llvm-z80/llvm-z80` per engagement-mode protocol).
  - Remove the peephole.
  - Verify no regression via CI + size baseline.

Exit: late-opt contains only Z80-ISA-specific patterns.  Late-opt
LOC drops materially (target: from 5272 to under ~3000 LOC).

### 12.9  Phase 9 — Documentation + maturity declaration

  - README updated.
  - Per-area documentation written.
  - Maturity criteria 3.1-3.8 audit confirms all met.
  - PR to `llvm-z80/llvm-z80` declaring "Z80 backend is mature for
    Z80 + SM83 targets" — opens the door for the long-term LLVM
    upstream conversation.

## 13.  Risks and contingencies

### 13.1  @zlfn unresponsive or disagrees on direction

  - Mitigation: don't speculate-build large PRs.  Open issues and
    wait for direction.  If @zlfn goes silent for >2 weeks, the
    workspace continues on `ravn/llvm-z80` and waits.
  - Fallback: if @zlfn permanently goes silent, the project
    fork-of-record could shift to `ravn/llvm-z80` — but this is
    far from current reality.

### 13.2  Phase 2 correctness fixes turn out interdependent

  - Mitigation: investigate #28 and #63 jointly first (they likely
    share a root cause).  #38 may interact with IY allocatability
    decisions in #12 / #40.  Keep each fix isolated until ground
    truth is known.

### 13.3  Regalloc cluster (Phase 3) is harder than estimated

  - Mitigation: #98 investigation is the gate.  If the
    investigation reveals the problem requires upstream-LLVM
    changes (per zlfn's #13 RegAlloc section noting "difficult to
    upstream"), pivot to peephole-style fixes for the immediate
    case while the core change waits.

### 13.4  Late-opt migration regresses other targets

  - Mitigation: each migration ships with both directions: the
    new upstream-area fix AND verification that the old peephole
    is no longer needed.  Don't delete the peephole until the
    upstream fix lands and CI is green for 2-3 days.

### 13.5  CI infrastructure gets stuck

  - Mitigation: Phase 1 includes CI setup, but if it stalls (e.g.,
    GitHub Actions billing, secret access), Phases 2+ proceed with
    manual lit-suite runs.  CI is helpful but not critical.

### 13.6  Source-cleanup audit reveals churn

  - Mitigation: each cleanup is opt-in (revert workaround → run
    tests → if green, keep cleanup).  Don't bulk-revert.  Per
    `feedback_dual_compiler_test.md`, both clang AND SDCC must
    still build the rcbios sources.

### 13.7  Time budget overrun

  - Mitigation: phases are independent.  If Phase 3 takes 5 weeks
    instead of 3, Phases 5+ can defer.  The maturity declaration
    (Phase 9) is the only hard gate.

## 14.  Long-term aspiration: llvm/llvm-project

This is **not in scope for the maturity work**.  Recording the
context for future reference:

### 14.1  Bar for new experimental targets

Per `llvm.org/docs/DeveloperPolicy.html#adding-a-new-target`:

  - Named maintainer + active community.
  - RFC on LLVM Discourse describing how the target meets
    requirements.
  - Multi-patch series, all LGTM'd.
  - 3-month stability + check-all passing + public buildbot for
    graduation from experimental.
  - Code free of contentious legal issues.

### 14.2  Recent precedents

  - **M68k**: RFC 2020 → experimental 2021 → multiple authors,
    multi-month review.
  - **CSKY**: T-HEAD/Alibaba-backed, RFC + D86269 + patch series.
  - **Xtensa**: Espressif-backed RFC 2021 → shipped Clang 20.

All three had organizational backing.  A solo fork without
multi-author review presence has no precedent for acceptance.

### 14.3  Pre-submission requirements (post-maturity)

If the eventual goal is reached:

  - @zlfn is the natural maintainer-of-record.  Submission would
    be from llvm-z80/llvm-z80, not from a fork.
  - RFC drafts should reference: target description, ISA reference,
    motivating use cases (CP/M, Game Boy, retrocomputing), test
    coverage, current users.
  - Pre-RFC: contact LLVM target maintainers via Discourse to
    gauge interest and identify reviewers.
  - Patch series likely 10-20 patches: target plumbing, minimal
    ISel, MC layer, calling convention, basic ALU+control flow,
    GISel, Z80 specifics, SM83 specifics, tests.

### 14.4  Why this is deferred

  - Maturity work is the precondition.  An immature backend
    cannot be upstreamed.
  - Organizational backing is missing today.  Solo maintainer +
    occasional contributors is below the typical bar.
  - The patch series for a new experimental target requires
    maintainer commitment and community review.  Both take time
    that's better spent on maturity first.

## 15.  Immediate next actions (week 1)

Per Section 10.1, all of these stay within `ravn/llvm-z80`.  Do
not file or comment on `llvm-z80/llvm-z80` until engagement mode
opens (Section 10.2).

1. **Review this plan with the user.**  Get explicit go/no-go on
   each phase before starting.
2. **Update `tasks/fix-plan.md`** to reference this roadmap as the
   master and itself as the per-cluster engineering doc.
3. **Phase 1 prep on `ravn/llvm-z80`**:
   - Begin late-opt audit reading
     (`Z80LateOptimization.cpp`).  Output: per-peephole
     classification keep/migrate/delete.
   - Sketch CI workflow YAML for the `ravn/llvm-z80` fork (private
     testbed).  Run it on the workspace.  Once stable, it can be
     proposed to `llvm-z80/llvm-z80` in engagement mode.
   - Stand up per-function size baseline tracker.
4. **Phase 2 dispatch**: pick the first correctness bug (#81 is
   smallest; recommended starting move).  Investigate and fix on a
   `ravn/llvm-z80` working branch.  Lit test in
   `llvm/test/CodeGen/Z80/`.  Do NOT open any
   `llvm-z80/llvm-z80` issue or PR for this yet — accumulate fixes
   locally until substantial.

## 16.  Out of scope (explicit)

  - HiTech compiler integration (per memory
    `project_hitech_third_compiler.md`): pending TODO; not part of
    Z80 backend completion.
  - rcbios/cpnos-rom source-level optimizations: orthogonal.
  - MAME / hardware-side work: unrelated.
  - LLVM upstream submission action plan (see Section 14).

## 17.  References

  - `llvm-z80/llvm-z80/CLAUDE.md` — project design goals.
  - `tasks/fix-plan.md` — engineering plan with cluster framing.
  - `tasks/backend-completion-roadmap.md` — backend-area lens.
  - `tasks/phase-c-regalloc-investigation.md` — Cluster A design.
  - `tasks/phase-a-findings.md` — null-result diagnostic.
  - `tasks/peephole-vs-root-cause.md` — late-opt audit framing.
  - `tasks/source-cleanup-vs-closed-issues.md` — source-side audit.
  - `tasks/issue-status-2026-05-02.md` — issue state at end of
    session 32.
  - `tasks/session{33,34,35}-summary.md` — recent session summaries.
  - llvm-z80/llvm-z80 issue #8 — collaboration channel.
  - llvm-z80/llvm-z80 issue #13 — @zlfn's core-LLVM-diff tracking.
  - https://llvm.org/docs/CodingStandards.html — code quality bar.
  - https://llvm.org/docs/Contributing.html — LLVM contribution
    process.

---

This document is the canonical plan as of 2026-05-02 evening
(session 36).  It will be revised as the work proceeds and as
@zlfn provides direction.
