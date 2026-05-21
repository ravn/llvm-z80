# Session 73p Phase 1 — clang beats SDCC on AES production target

Date: 2026-05-21.  Multi-hour focused session per user direction
"work concentrated on this on your own".

## Headline

**llvm-z80 clang now DOMINATES SDCC on AES corpus production target**:

| | clang `09_Oz_prod_like` | SDCC `01_baseline_prod` | Gap |
|---|---:|---:|---:|
| Before session 73p | 2 667 B / 14 887 472 ts | 3 323 B / 12 080 289 ts | clang **−20 % size, +23 % slower** |
| After session 73p | **2 574 B / 10 749 186 ts** | 3 323 B / 12 080 289 ts | clang **−23 % size, −11 % faster** |

**clang now wins both axes** on the production target.  Same pattern
holds for `05_Oz_static_stack`, `12_Oz_no_omit_fp`, `13_Oz_no_omit_fp_no_licm_cse_gc`.

All 13 AES corpus configs PASS verifier.  All 13 are now faster than
SDCC by 4.8–11 %.  4 of 13 are also smaller than SDCC.

## What landed in Phase 1

### Codegen changes (3 commits in llvm-z80)

**#179 P1** (commit `4f5562c99228`) — Z80ReorderTestDec pre-RA pass
recognizes the post-ISel `LD_A_R; DEC_A; LD_<r2>_A; LD_A_R; OR_A;
JR_Z` redundant-reload pattern and rewrites to
`LD_A_R; SUB_n 1; LD_<r2>_A; JR_C`.  Saves 2 instructions, ~5 ts
per match.  AES `09_Oz_prod_like`: −5.1 % ts.

**#179 P2** (commit `6820930cc156`) — extends the same pass to
recognize `LD_A_R; ADD_A_A; LD_<r2>_A; LD_A_R; RLCA; JR_C/NC`
(bit-7 test) and rewrite by dropping the redundant `LD_A_R; RLCA`
(ADD_A_A already produced the right carry).  Saves 2 instructions
per match.  **AES `09_Oz_prod_like`: additional −23.9 % ts.**
This is where the production target flipped to dominate SDCC.

**#128** (commit `7d5b4e5ea86c`) — `Z80PassConfig` disables
`EarlyMachineLICM`, `MachineLICM`, and `MachineCSE` globally via
`disablePass()` in its constructor.  These passes consistently
pessimize Z80 because the 3-pair register file can't hold the
hoisted/shared values; the hoisted-to-BSS-spilled traffic outweighs
the redundant-compute savings.  Closes ravn/llvm-z80#128.
Default `-Oz` AES: −281 B.

### Infrastructure

- New `rc700-gensmedet/tasks/compiler-comparison-corpus/` (wider
  oracle, mirrors AES corpus pattern).  3 benchmarks ported from
  z88dk's official compiler-comparison suite (sieve, fannkuch, pi).
- New `Z80ReorderTestDec.{h,cpp}` pre-RA MIR pass + lit test
  `issue-179-test-then-dec.ll`.
- Reset stub workaround for ravn/llvm-z80#182 (LLVM SCEV crash on
  init+reader loop pattern, filed this session).

### Issues filed this session

- **#173** — 8-bit BSS spill peephole opportunity (~3-4 h work
  remaining, ~50-100 B / 50 K ts estimated yield)
- **#174** — gf_log/gf_alog redundant reload (effectively closed
  by #179 P1+P2)
- **#175** — Missing 8-bit ALU with memory operand (revised: only
  the `(IX+d)`/`(IY+d)` forms are missing; `(HL)` forms exist)
- **#176** — Auto-infer +static-stack safety per-function
- **#177** — No Z80-specific TargetTransformInfo (meta-fix)
- **#178** — Pseudos with implicit physreg outputs break remat
- **#179** — GISel + scheduler don't reorder $a-chained ops
  (effectively closed by this session's Z80ReorderTestDec pass)
- **#180** — Z80LateOptimization peephole audit (tracker)
- **#181** — DAGISel vs GISel coexistence audit
- **#182** — LLVM ScalarEvolution capacity overflow on init+reader
  loop pattern (NEW high-priority correctness bug)

### Strategic documents

- `llvm-z80/tasks/aes-speed-gap-analysis.md` — initial speed-gap analysis
- `llvm-z80/tasks/all-modes-competitive-plan.md` — per-mode strategic plan
- `llvm-z80/tasks/structural-deficiency-survey.md` — 5 new structural issues
- `llvm-z80/tasks/issue174-implementation-plan.md` — #174 plan (executed)
- `llvm-z80/tasks/session73p-summary.md` — early-session summary
- `llvm-z80/tasks/session73p-phase1-summary.md` — this document

## Value oracle preserved across Phase 1

All gates green after the final commit:

| Oracle | Result |
|---|---|
| Z80 lit suite | 106 PASS + 3 XFAIL (was 104+3; +2 new tests this session) |
| AES corpus 13 configs | 13/13 PASS, byte-exact verifier |
| compiler-comparison-corpus | sieve + fannkuch + pi PASS llvm-z80 side |
| test-runner clang | 681/46/56/207 — exactly matches baseline (no regressions) |
| cpnos PROM1 size | 2030 / 2048 B (18 B free; unchanged from session start) |

## Honest reflection

**Estimates vs reality:**
- #179 plan estimated 1.5 M ts saved.  Actual P1+P2 combined: ~4.1 M ts.
  Off by ~3× HIGH — the bit-7 test pattern (P2) was more pervasive
  than my speed-gap analysis credited.
- #128 plan estimated −320 B at -Oz default.  Actual: −281 B.
  Off by ~12 % LOW.
- Wider oracle estimated 6 z88dk benchmarks; got 3 working + 3
  deferred to "libc" follow-up.

**What went well:**
- TDD discipline (Decision I): every fix had failing lit test first.
- Decision E full oracle gating caught issues before commit.
- Pre-RA MIR pass layer (vs late-opt peephole) per
  feedback_root_cause_over_peephole guided the right
  architectural choice for #179.
- The wider-oracle investment surfaced #182 (real LLVM SCEV crash)
  within minutes of building it.

**What was harder than expected:**
- The PROPER fix for #179 needed substantial pattern analysis (the
  6-instruction shape, the safety gate on I2's destination vreg)
  before implementation.  My initial "post-RA peephole" plan was
  the wrong layer; the pre-RA MIR pass mirrors Z80SplitDjnzCounters
  precedent and is the right home.
- #173 / #175 turned out smaller-impact than my speed-gap analysis
  estimated because clang already uses (HL) ALU fusion in
  +static-stack mode.

**Multi-week future work (out of scope for Phase 1):**
- **#176 auto-static-stack inference** — closes the remaining
  default-Oz size gap.  Callgraph SCC analysis + ISR isolation.
- **#177 Z80 TTI** — the meta-fix; subsumes #128's
  permanent solution + many other smaller issues.
- **#172 A-pin liveness** — would close additional residual ts
  gap (smaller now that gap is much narrower).
- **#178 pseudo SSA conversion** — unlocks rematerialization framework.

## Phase 2 next-best-steps (when resumed)

In yield-per-session-hour order:

1. **#173** — BSS spill peephole.  ~3-4 h.  Closes a Z80-ISA-
   specific gap.  ~50-100 B AES, possibly 5-10 B on cpnos PROM1.
2. **#175 (IX+d) form** — adds 16 missing ALU instructions to
   TableGen.  Helps IX-frame mode (configs 12/13) which are
   already close to SDCC.  Mechanical work, ~1 day.
3. **#176 auto-static-stack** — closes default-Oz size gap to
   under SDCC.  Substantial: per-function callgraph analysis.
   1-3 weeks.
4. **#177 Z80 TTI** — meta-fix that subsumes #128's permanent
   solution, plus #95, #27, #115.  1-2 weeks.
5. **#172 A-pin liveness-aware** — residual gap, now smaller
   since main gf_log/gf_alog is fixed.

## Upstream prep (Decision G)

Per project_z80_upstream_goal: do not engage llvm-z80/llvm-z80
until "something substantial is ready".  Three landed structural
fixes (Z80ReorderTestDec, #128 disable, plus the #182 crash bug
report) qualify as substantial.  When the user is ready to engage,
the upstream PR drafts should:

1. Summarize #179 P1+P2 as a single pre-RA MIR pass closing the
   "GISel + scheduler don't reorder $a-chained ops" structural
   gap.  Include AES corpus numbers + the lit test.

2. Submit #128 (LICM/CSE disable) as a TargetPassConfig change
   with the AES-corpus + cpnos-rom evidence that the passes
   pessimize on Z80.  Owner may prefer the gated-on-TTI version
   (#177); discuss in PR.

3. Submit #182 (SCEV crash) upstream-LLVM directly since it's an
   upstream LLVM bug, not Z80-specific.  Include the 7-line repro.

## Status of confirmed Decisions A–I

| Decision | Status |
|---|---|
| A — #179 fix layer (pre-RA MIR pass) | Used; correct choice |
| B — #178 pseudo conversion | Not started Phase 1 |
| C — #177 TTI hook order | Not started Phase 1 |
| D — Per-issue branch strategy | Used informally (session commits linear) |
| E — Validation cadence (full oracle pre-merge) | Used every commit |
| F — Failure handling (3-level rule) | Triggered once on #179 (I2 vreg safety gate) |
| G — Upstream-engagement timing | No engagement yet |
| H — Stopping / asking criteria | Triggered Phase 1 stop |
| I — Test-case philosophy (TDD) | Used every commit |

## Suggested closing point

This is a natural Phase 1 milestone.  Production target dominates
SDCC.  All Decision E gates green.  No outstanding work-in-progress.
Suggested: stop here, archive the session via timeline + workspace
bump, resume in a separate session for #173 / #175 / #176 / #177.
