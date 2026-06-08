# Session 2026-06-08 — #23 LICM/CSE revalidation + disablePass retirement

## TL;DR

The historical `disablePass(MachineLICM + EarlyMachineLICM + MachineCSE)`
workaround in `Z80PassConfig`, justified by 2026-05 measurements showing
LICM/CSE pessimized Z80 and #198 showing -O2 CSE miscompiled AES, has
been **retired**.  Re-measurement on a clean rebuild today shows:

- AES (the project's correctness oracle): **-8.9% tstates (-Oz)**,
  **-9.2% tstates + -118 B text (-O2)**, PASS in both.
- The #198 -O2 miscompile **no longer reproduces** — full lit suite
  (149 PASS + 4 XFAIL) and test-runner runtime suite (854 PASS / 0 FAIL
  / 0 FATAL across O0..Oz) stay green with LICM+CSE forced on.
- autoload-in-c PROM: +25 B compressed / +64 B raw .text (accepted per
  user direction "don't let short-term size block structural fixes").
- cpnos PROM1: **-15 B** (improves).
- rcbios BIOS: +7 B (marginal).

Implementation: defaults of `-mllvm -z80-enable-licm` and
`-mllvm -z80-enable-cse` flipped from `false` to `true`; the cl::opt
flags remain as opt-out escape hatches for diagnosis without rebuilding
clang.  A new opt-in heuristic `Z80InstrInfo::shouldHoist` gated by
`-mllvm -z80-licm-block-on-call` (default OFF) refuses to hoist out
of loops whose body contains a CALL; landed as tunable infrastructure
for follow-up cost-model work.

## How the investigation actually went

1. **AES A/B was thought to be available** — `tasks/aes256-corpus/task3_licm_ab.sh`
   existed but its OFF cells used `-mllvm -disable-machine-licm/-cse`
   which were redundant with the in-tree disablePass.  Its ON cells were
   degenerate against the unconditional disable — the script had been
   producing identical OFF/ON cells for months.

2. **Added the toggle** — `-mllvm -z80-enable-licm` and `-mllvm -z80-enable-cse`
   cl::opt flags in `Z80TargetMachine.cpp` (default OFF initially) gate
   the disablePass calls per-flag.  Rewired task3 to use the new flags.

3. **First measurement was suspect** — historical claims said LICM at
   -Oz adds +34 B / +144 B / pessimizes.  My measurement showed LICM at
   -Oz saves 13 B AND 8.9% tstates.  The inversion was so striking that
   we did a triple-check via full clean rebuild.

4. **Clean rebuild was byte-identical to incremental** — confirmed the
   measurement was sound (user-flagged stale-build concern was valid as
   a general principle but did not apply to today's numbers).

5. **The change in cost equation has two plausible explanations**:
   - **Backend movement since 2026-05**: regalloc gained IY-allocatable,
     GR8 reorder for DJNZ, IX callee-saved, COPY16_PUSHPOP, etc.
     Same C in, different MIR out, different register-pressure profile.
   - **Stale-rebuild incident in the original measurement**: the 2026-05
     numbers could have been on a partially-rebuilt clang where the
     flag toggle wasn't actually changing pass behavior.
   - Either way, the historical claim no longer matches current HEAD.

6. **Workload-dependence then surfaced** — AES wins big, autoload loses
   (+64 B raw text via SNIOS-shape register-pressure-dense HW init),
   cpnos roughly neutral, rcbios +7 B.

7. **Built a `shouldHoist` heuristic** — "refuse to hoist out of any
   loop whose body contains a CALL".  Preserves AES win (gf_alog/gf_log
   are leaf loops; heuristic doesn't fire), eliminates autoload
   regression (HW init loops have CALLs; heuristic blocks hoists).
   But it un-does cpnos's win (cpnos had a call-loop where the hoist
   WAS beneficial — heuristic is too coarse).

8. **User direction inflected the decision** — "do not consider short
   term code increase for now, it will come back".  Removed the
   conservative middle ground; flipped enable-LICM/CSE defaults to ON,
   left the shouldHoist heuristic default OFF, accept the autoload
   temporary regression.

## Production code state landed

| File | Change |
|---|---|
| `Z80TargetMachine.cpp` | `EnableMachineLICM` / `EnableMachineCSE` cl::opt defaults flipped FALSE -> TRUE.  Comment block rewritten with the 2026-06-08 re-measurement narrative.  disablePass calls preserved but gated by the flags. |
| `Z80InstrInfo.h` | New `shouldHoist` virtual override declaration. |
| `Z80InstrInfo.cpp` | `shouldHoist` impl + `-mllvm -z80-licm-block-on-call` cl::opt (default OFF) + `#include` of `MachineLoopInfo.h` and `CommandLine.h`. |

## Production validation

- Lit suite: **149 PASS + 4 XFAIL** (no regression vs baseline).
- test-runner runtime: **854 PASS / 0 FAIL / 0 FATAL across O0..Oz**.
- autoload PROM: still fits 2 KB hard cap (1683 / 2048 = 365 B free).
- cpnos PROM1: still fits 2 KB hard cap (2014 / 2048 = 34 B free,
  improved from 19 B free).
- rcbios BIOS: 5915 B vs 5908 B baseline.  MINI still has 229 B to
  spare (was 236 B).

## Follow-up work (filed as separate tracking)

1. **Count-based `shouldHoist` refinement** — track # of values already
   chosen for hoisting that will be live across a call in this loop.
   Refuse only when count + candidate > 3 (caller-saved-pair count on
   Z80).  Would close the cpnos regression-from-heuristic gap (heuristic
   currently un-does cpnos's 11-15 B win) and likely close rcbios's +7 B
   gap.  ~1 hour of design + measurement.

2. **MachineCSE has no `shouldHoist` analog** — would need a different
   hook or a Z80-specific custom pass that gates CSE per-loop.  Not
   needed today (CSE on AES is pure-win); revisit if a workload shows
   CSE-specific pessimization.

3. **Per-function `optsize` respect in MachineLICM** — currently
   `Function::hasOptSize()` is consulted only weakly.  If autoload's
   HW init were marked `__attribute__((optsize))`, MachineLICM could
   hoist more conservatively per-function rather than per-loop.  This
   is the "source-level opt-out" alternative to the cost-model fix
   in (1).

4. **Upstream filing candidate** — generic LLVM missed-optimization
   bug: MachineLICM's `CanCauseHighRegPressure` cost model doesn't
   model call-clobbered live ranges, so on tiny-register-file targets
   the hoist decision is workload-shape-dependent.  AES vs SNIOS
   diverges by 17 % in opposite directions.  See known-suboptimal-codegen.md
   for the public-facing framing.  Filing per `feedback_explain_before_filing`
   needs user go-ahead.

## Infrastructure landed in compiler-comparison-corpus

(Separate but same-session work, not strictly #23 but adjacent.)

- ED-FE-trap protocol for z88dk-ticks termination (replaces HALT,
  which never terminated ticks under any condition we tested).
  See [[reference_ticks_canonical_exit_trap]].
- `EXPECTED_FAIL` set in sweep.sh — fannkuch zsdcc + pi zsdcc marked
  XFAIL.  See `rc700-gensmedet/tasks/zsdcc-bench-divergence-2026-06-08.md`.
- New benches: `bench_word_fill.c` (i16-counter + walking pointer,
  targets ravn/llvm-z80#99 XFAIL); `bench_licm_pessimize.c` (LICM-bait
  synthetic — turned out IR-level passes hoist first, so MIR LICM sees
  nothing; null result documented).
- `CLANG_EXTRA` env-var injection hook in sweep.sh + the three
  production Makefiles for clean A/B measurements.

## Memory rules added this session

- `feedback_check_memory_before_coding` — HARD: at task start, scan
  MEMORY.md for applicable sections, name the rules in your first
  response, then code.
- `feedback_revalidate_historical_compiler_claims` — HARD: re-validate
  on clean rebuild before acting on any historical compiler-perf claim.
  Pin example: this session.
- `reference_ticks_canonical_exit_trap` — pins the ED FE trap as the
  canonical termination mechanism (replacing prior HALT-based attempts).
