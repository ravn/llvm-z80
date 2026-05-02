# Session 36 — strategic reframe + maturity roadmap + upstream sync

Date: 2026-05-02 evening (immediate continuation of session 35).
Branch: `session-36-code-density-plan` (off `main`).

## TL;DR

No codegen changes; this was a planning + research session.  Key
deliverables:

  - Strategic clarification from the user: target is
    **`llvm-z80/llvm-z80`** (active fork-of-record, owner @zlfn) for
    near-term collaboration, with **official `llvm/llvm-project`**
    as a long-term aspiration.  Engagement with @zlfn deferred until
    *substantial* work is ready.
  - Master plan: `tasks/roadmap-to-maturity.md` (872 lines, 17
    sections) supersedes the false-start
    `plan-code-density-2026-05-02.md`.
  - Upstream sync: merged `llvm-z80/llvm-z80:main` into ravn's
    `main` (~3782 commits, 1 Z80-source change: zlfn's Combiner API
    fix `a016d5bc`).  Build clean, lit 77 PASS + 1 XFAIL, sizes
    byte-exact (BIOS 5920, cpnos-rom 1708).
  - One issue filed: `ravn/llvm-z80#101` — missing `override`
    markers on `Z80TargetTransformInfo.h` post-merge.

## What this session did NOT do

- No closed compiler issues.
- No code changes in `lib/Target/Z80/`.
- No size deltas (intentionally; the goal was to plan).

## Reframing journey (chronological)

1. **Started** thinking the work was an "optimization sprint" against
   SDCC.  Built `plan-code-density-2026-05-02.md` with Phase A/B/C/D
   structure targeting BIOS bytes.
2. **Phase A failed**: the diagnostic load-after-store-same-addr in
   `_isr_crt` turned out to be C-volatile-mandated.  Phase B
   removed.  See `tasks/phase-a-findings.md`.
3. **User reframe #1**: backend is preliminary, not finished.  Goal
   is "finish it correctly", not "optimize a finished one".  Saved
   as memory `project_z80_backend_unfinished.md`.
4. **Built** `tasks/backend-completion-roadmap.md` with 16-area audit
   framing.
5. **User reframe #2**: feedback "prefer C over inline asm; root
   cause over peephole" became explicit.  Saved as memory
   `feedback_root_cause_over_peephole.md`.
6. **User reframe #3**: target is upstream LLVM submission.  Saved as
   memory `project_z80_upstream_goal.md` (later revised twice).
7. **User reframe #4**: collaborate with `llvm-z80/llvm-z80` owner to
   reach maturity, then submit to official LLVM.  Memory revised.
8. **Discovery**: official `llvm/llvm-project` has NO Z80 backend.
   `jacobly0/llvm-z80` is archived (2017).  `jacobly0/llvm-project:z80`
   dormant since 2023-11.  Active parent is `llvm-z80/llvm-z80`,
   owner @zlfn.
9. **Discovery**: ravn already has 6+ closed issues + PRs in
   `llvm-z80/llvm-z80`; collaboration channel established via
   issue #8 ("Possible contribution.").  @zlfn's #13 ("Track and
   reduce core LLVM modifications") is the alignment document.
10. **User reframe #5**: don't engage `llvm-z80/llvm-z80` until
    *substantial* work is ready.  Memory + roadmap updated to
    "workspace mode" / "engagement mode" split.

## Research conducted (4 parallel agents)

### Agent 1: Upstream LLVM Z80 status

  - llvm/llvm-project: 25 targets in `llvm/lib/Target/`, **no Z80, no
    eZ80**.  No formal RFC for Z80 backend on Discourse.
  - jacobly0/llvm-z80: archived 2017.  jacobly0/llvm-project:z80 last
    substantive commit 2023-11-16; effectively dormant.
  - llvm-z80/llvm-z80: active (pushed 2026-04-25), owner @zlfn (Kiroo
    Chanung, POSTECH, Korea), 35 stars, 16 issues, 9 PRs.
  - LLVM new-target bar: RFC + named maintainer + active community +
    multi-patch series + 3-month stability + public buildbot.  Recent
    precedents (M68k, CSKY, Xtensa) all had organizational backing.

### Agent 2: Backend area audit

~80% production-ready across 79 files / 27K LOC.  Per-area maturity:

  - 5/5 (complete): target description, register defs, calling convs,
    TTI, auxiliary IR passes, MC layer.  Sized 244-1309 LOC each.
  - 4/5 (mostly): TableGen instr defs (3148 LOC, GISel.td minimal),
    Legalizer (1487, one float fatal — verified intentional for SM83
    alloca), InstrSelector (5488, large but structured), InstrInfo
    (2494), RegisterInfo (1774), FrameLowering (490), ExpandPseudo
    (490), LateOptimization (5272, 50+ peepholes), inline-asm (55).
  - 1/5 (stub): RegisterBankInfo (70 LOC; acceptable for Z80's
    single-bank reality), DAG ISelLowering (235; acceptable since
    GISel is primary).

Top concerns:
  - LateOptimization 5272 LOC needs per-peephole audit per
    `peephole-vs-root-cause.md`.
  - `Z80InstrGISel.td` is 73 LOC; most ISel logic is in C++.

### Agent 3: Peer-target precedent

Four problems researched (DJNZ-as-primary, remat, IV-form TTI,
late-opt boundary).  Key findings:

  - **DJNZ-as-primary was wrong**.  ARM uses **post-RA fusion** (CBZ
    via `ARMConstantIslandPass.cpp:1906-1942`), not primary ISel.
    M68k's DBcc unimplemented in upstream LLVM.  Right design is
    DJNZ_LOOP pseudo + register hint + existing peephole — same
    approach Z80 uses today.  This **corrects** the
    `phase-c-regalloc-investigation.md` Cluster 8.1 framing.
  - **Rematerialization** is well-trodden: TableGen `let
    isRematerializable = 1` + `isReMaterializableImpl` override
    (ARM/X86/AVR pattern).  Z80 has 4+ entries already.
  - **TTI for IV form**: AVR has `isLSRCostLess` override
    (`AVRTargetTransformInfo.cpp:13-24`).  Z80 already has TTI with
    7 overrides per audit — needs per-hook check during Cluster D.
  - **Late-opt boundary**: Z80's `Z80ExpandPseudo` + `Z80LateOptimization`
    structure **matches** ARM/M68k upstream precedent.  5272 LOC isn't
    inherently a problem; the audit determines per-peephole fitness.

### Agent 4: Open-issue triage

27 open in ravn/llvm-z80.  5 correctness bugs (#28, #36, #38, #63,
#81), 22 pessimizations across 6 clusters:

  - **A regalloc** (#94, #98, #89, #99, #27, #16) — highest leverage
  - **B spill mechanism** (#20, #100, #96, #12) — overlaps A
  - **C -O0 correctness** (#28, #63) — investigate jointly
  - **D loop/IV form** (#15, #50, #77, #95)
  - **E CP/M ABI** (#4, #42, #43, #36, #35)
  - **F MC/diagnostics** (#70, #81)

## Master plan: `roadmap-to-maturity.md` (17 sections)

Synthesizes the four research outputs + collaboration channel
verification.  Key sections:

  - §3: Maturity criteria (correctness, tests, late-opt audit, source
    cleanup, core-LLVM diff bound, builds, docs, size baseline).
  - §10: Workspace mode (current) vs engagement mode (gated on
    "substantial" — defined as all 5 correctness bugs closed +
    one cluster + test infra + coordinated narrative).
  - §12: 9-phase execution (Foundation → Correctness → Regalloc →
    Spill → Loop/IV → CP/M ABI → MC → Late-opt migration → Docs).
  - §14: Long-term llvm/llvm-project aspiration with realistic bar.

Time estimate: 13-23 calendar weeks (3-6 months part-time) for
Phases 1-9; LLVM-upstream submission is years out and out of scope.

## Upstream sync verification

Merged `llvm-z80/llvm-z80:main` (commit `a016d5bc`) into ravn's
`main` (was `b0d029a5`).  Result: `f91102a4`.

  - **Conflicts:** none.  3782 upstream commits, 12252 files changed,
    1.8M+/377K- lines.  Mostly non-Z80 (LLVM upstream tracking).
  - **Z80-source changes:** 2 files (`Z80PostLegalizerCombiner.cpp`,
    `Z80PreLegalizerCombiner.cpp`), 18+/24- — Combiner API adaptation.
  - **Build:** clang-23 + llc linked.  Two `-Winconsistent-missing-
    override` warnings on `Z80TargetTransformInfo.h` (filed as #101).
  - **Lit suite:** 77 PASS + 1 XFAIL (matches baseline).
  - **Real-workload sizes:** rcbios BIOS 5920 B, cpnos-rom payload
    1708 B (byte-exact match to baseline).  PROM0 1777 non-padding B.

Then merged `origin/main` into ravn's `main` (`fdba532f`) to pick up
the orphan SIO-B build artifact and make the push fast-forwardable.

Pushed: `main` (`fdba532f`) and `session-36-code-density-plan` to
`origin`.

## Memories saved or revised this session

- `project_z80_backend_unfinished.md` (new) — backend is preliminary
- `project_z80_upstream_goal.md` (new, then revised twice; final
  version: staged collaboration model with engagement-mode gate)
- `project_hitech_third_compiler.md` (new) — TODO: add ravn/hitech
  as third compiler submodule
- `feedback_root_cause_over_peephole.md` (new) — favor upstream fixes
  over post-RA peepholes
- `feedback_no_compliments.md` (revised) — added "Good catch" to the
  forbidden phrases list

## Issues filed this session

- `ravn/llvm-z80#101`: Missing `override` markers on
  `Z80TargetTransformInfo.h` after upstream merge.  Trivial fix
  (~2 lines); naturally fits Phase 1 of roadmap as a workflow
  warm-up.

## Files touched

- `tasks/plan-code-density-2026-05-02.md` (created, then marked
  superseded)
- `tasks/phase-a-findings.md` (new)
- `tasks/phase-c-regalloc-investigation.md` (new)
- `tasks/backend-completion-roadmap.md` (new, then header revised to
  complement fix-plan.md)
- `tasks/fix-plan.md` (session-36 update + Cluster 8 added)
- `tasks/roadmap-to-maturity.md` (new, master plan)
- `tasks/session36-summary.md` (this file)
- `CLAUDE.md` (gap-analysis section remeasure)

No `lib/Target/Z80/` source touched.

## Pain points

- **First-pass diagnostic was wrong.**  Phase A's
  load-after-store-same-addr finding turned out to be volatile-
  mandated.  Spent ~30 min chasing ghosts before re-checking the
  C declarations.  Recorded as a methodological lesson in
  `phase-a-findings.md` ("high BSS-access ratio in an ISR is a
  *structural* signal of mandatory volatile access, not a defect
  signal").  **(Medium)**

- **Plan rewrites accumulated**.  Wrote three planning files
  (`plan-code-density-2026-05-02.md`,
  `backend-completion-roadmap.md`, `roadmap-to-maturity.md`) before
  realizing the existing `fix-plan.md` from session 33 was already
  the canonical plan.  Should have checked `tasks/` thoroughly
  before writing the first new file.  **(Painful)** — spent ~1 hour
  on duplicate planning.

- **Initial framing missed upstream context.**  Wrote the first
  plan assuming "submit upstream" meant llvm/llvm-project, then
  discovered there's no Z80 there.  User then clarified the staged
  collaboration model.  Should have done the upstream-status
  research BEFORE writing the first plan.  **(Painful)**

- **TaskCreate / TaskList state-tracking is patchy.**  Created tasks
  but TaskList returned empty at end-of-session.  Worth investigating
  on a future session.  **(Easy)**

## Still open

- All 27 open issues unchanged (this was a planning session).
- New issue #101 (override warnings) is the natural Phase 1 first
  fix candidate.
- Phase 1 (Foundation) of the maturity roadmap not yet started.
- Engagement mode with @zlfn not yet opened (per user directive).

## Next session entry point

Per `roadmap-to-maturity.md` §15:

  1. Phase 1 prep on `ravn/llvm-z80`:
     - Late-opt audit reading (`Z80LateOptimization.cpp` per-peephole
       keep/migrate/delete classification).
     - Sketch CI workflow YAML (workspace testbed only).
     - Per-function size baseline tracker.
  2. Phase 2 dispatch: pick #81 (smallest correctness bug, MC parser,
     recommended starting move) — or #101 (the override warnings) as
     even-lighter warm-up.

Working branch: `session-37-...` off `session-36-code-density-plan`
(or off main, same content modulo plan docs).

## Verification matrix at session end

| Check | Result |
| --- | --- |
| Build (`ninja clang llc`) | ✅ clean, 2 warnings (filed as #101) |
| Z80 lit suite | ✅ 77 PASS + 1 XFAIL |
| rcbios BIOS | ✅ 5920 B (byte-exact baseline) |
| cpnos-rom payload | ✅ 1708 B (byte-exact baseline) |
| Pushed to origin | ✅ `main` and `session-36-code-density-plan` |

End of session 36.
