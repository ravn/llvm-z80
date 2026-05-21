# Implementation plan: #177 Z80 TargetTransformInfo

Date: 2026-05-21 (session 73p Phase 2).  Triggered by user direction
to plan #177 thoroughly before starting code work.

## Work clock

**Phase 2 work clock starts 2026-05-21** per user direction.  Initial
estimate was "4-6 weeks of focused work"; **revised to 2-4 weeks
after Phase A investigation** (Phase E retired -- see Phase A
findings doc, summary below).

Realistic calendar window (revised):

- **Aggressive target: 2026-06-04** (~2 weeks, Tier 1+2+3 land).
- **Conservative target: 2026-06-18** (~4 weeks, all remaining phases).
- ~~Earliest viable #128 revert: 2026-06-04~~ — **retired**, see below.

Each phase commits to main via the existing `--no-ff` merge bubble
pattern (session-73p-phase2-issue177 → main).  Phase A's deliverable
unblocks Phase B; subsequent phases can land in parallel.

Status tracking:
- **Phase A complete: 2026-05-21** ✓ (this session; see
  `issue177-phase-a-investigation.md`)
- Phase B (Tier 1) commits landing: ___
- Phase C (Tier 2) commits landing: ___
- Phase D (Tier 3 + 4 cleanup) commits landing: ___
- ~~Phase E~~: **retired** (see Phase A findings)
- ~~Phase F~~: merged into Phase B
- #177 closed: ___

## Phase A findings (summary)

**1. MachineLICM/CSE do NOT use TTI.**  Empirically verified by
grepping `llvm/lib/CodeGen/MachineLICM.cpp` and `MachineCSE.cpp`
for any TTI references — zero matches.  These passes use
`TargetInstrInfo` + `MachineRegisterInfo` + their own pressure-
tracking heuristics.

**Implication**: Phase E (the proposed TTI-based per-function
optsize/minsize gating that would enable a revert of #128's
global `disablePass()` workaround) **is not viable via TTI**.
Phase E retired.  #128's workaround stays.

**2. `getMemoryOpCost` and `getCFInstrCost` are vectorize-dominant.**
Used almost exclusively by `LoopVectorize`, `SLPVectorize`,
`VectorCombine`, `IROutliner`.  Z80 doesn't vectorize.  Demoted
from prior plan's Tier 1 to current Tier 4 (no-vectorization
cleanup with conservative defaults).

**3. `getInstructionCost` is the single highest-leverage hook.**
Used by LICMPass, LoopUnroll, SimplifyCFG, and InlinerPass — four
critical IR-level passes simultaneously.  Tier 1 confirmed.

**4. New Tier 1 priorities (revised):**

1. `getInstructionCost` (touches 4 critical passes)
2. `getUnrollingPreferences` (Z80 should mostly disable unroll)
3. `isProfitableToHoist` (SimplifyCFG hoist-decision cost)

See `issue177-phase-a-investigation.md` for full per-hook
per-pass mapping.

## ROI on Phase A investigation

~30 minutes of focused investigation saved ~1 week of misdirected
Phase E work.  **50× ROI on Phase A.**  Lesson confirmed (per
`feedback_no_commit_first_version`): validate the premise
empirically before committing to multi-week work.

## What TTI is and why Z80 needs it

`TargetTransformInfo` (TTI) is LLVM's mechanism for backends to tell
target-independent IR passes about target-specific cost.  Many IR
passes consult TTI before making transformation decisions: how many
registers exist, how expensive a memory op is, whether a branch is
predictable, whether vectorization is worthwhile, etc.

**Without a target-specific TTI, the default values come from
`BasicTTIImplBase` which is RISC-biased**: assumes ~16 GP registers
of natural-word width, branch prediction, vectorization, low load/
store cost, etc.

The Z80 reality is drastically different:
- 3 allocatable 16-bit pairs (BC, DE, HL); 1 single 8-bit accumulator (A).
- IX/IY available conditionally; cost ~2× normal ops (DD/FD prefix).
- 8-bit native; 16-bit half-native (pair instructions); 32-bit + FP software-emulated.
- Loads: `LD A, (nn)` 3 B / 13 ts; `LD r, (HL)` 1 B / 7 ts; `LD r, (IX+d)` 3 B / 19 ts.
- Branches: `JR cc` 2 B / 7-12 ts; `JP cc` 3 B / 10-17 ts.  No branch predictor.
- No barrel shifter (multi-bit shift = N×1-bit shifts).
- No multiply, no divide (software emulation via compiler-rt).

These differences mean IR passes consistently make wrong decisions
without target-specific cost overrides.  Concretely, this session's
#128 (LICM/CSE disable) demonstrated the problem: at -Oz, those two
passes added bytes by hoisting/sharing values that then BSS-spilled
across the 3-pair register file.  The right answer was target-
specific cost; the current expedient is a global disablePass()
which loses opportunity in cases where LICM/CSE WOULD help.

## Current TTI state (pre-#177)

File: `llvm/lib/Target/Z80/Z80TargetTransformInfo.h` (99 lines, no .cpp).

Hooks already implemented:
- `hasDivRemOp` → true (everything's a libcall)
- `isLSRCostLess` → prioritize NumRegs over Insns (good)
- `getPredictableBranchThreshold` → 0/1 = "no branch is predictable" (good — Z80 has no branch predictor)
- `isValidAddrSpaceCast` → true (Z80's port-IO AS=2 ↔ memory AS=0)
- `getNumberOfRegisters` → 3 (pair count)
- `getRegisterBitWidth` → 8 (scalar width)
- `areInlineCompatible` → custom logic (inline hint / small / single-call-site)

Hooks NOT YET implemented that matter for #128 root-cause + much else:

| Hook | What it controls | Why Z80 needs it |
|---|---|---|
| `getInstructionCost` | per-instruction cost — used by LICM, CSE, GVN, vectorizer | Z80 cost ≠ RISC default |
| `getMemoryOpCost` | load/store cost — used by MemcpyOpt, LoopIdiom, LICM | Z80 stores via A are 3 B / 13 ts; via (HL) 1 B / 7 ts |
| `getCFInstrCost` | branch cost — used by SimplifyCFG, LSR | JR vs JP cost differ; branches with-predictor cheap, without expensive |
| `getCallCost` | call cost — used by inliner | Z80 CALL+RET = 4 B / 27 ts; cost dominates short functions |
| `getMaxInterleaveFactor` | vectorizer interleave factor | 1 (no SIMD) |
| `prefersVectorizedAddressing` | vectorizer | false |
| `enableInterleavedAccessVectorization` | vectorizer | false |
| `getMinimumVF` / `getMaximumVF` | vectorizer | 1 (scalar only) |
| `isLegalAddImmediate(int64_t)` | LSR/CSE | true only for small constants on Z80 |
| `isLegalICmpImmediate(int64_t)` | LSR/CSE | true only for 8-bit constants |
| `getArithmeticInstrCost` | LICM/CSE/SLP | needed for cost-aware hoisting |
| `getCmpSelInstrCost` | branch decision cost | Z80 compares are A-via-CP; non-A registers more expensive |
| `getCastInstrCost` | type conversion cost | Z80 i16→i8 truncate = 0 cost; i8→i16 zext = LD H, 0 |
| `getMemcpyLoopLoweringType` | LDIR threshold | Z80 LDIR makes memcpy ~free for size 8+ |
| `getMinPrefetchStride` | prefetcher | N/A on Z80 |
| `getMaxPrefetchIterationsAhead` | prefetcher | 0 (no prefetcher) |

## What #177 closing would unlock

Direct issues #177 subsumes or improves:
- **#128** (closed by global LICM/CSE disable): proper per-function cost-gated disable via TTI cost hooks.
- **#95** (closed; IV rewrite countdown→count-up at -Oz): TTI cost should naturally prefer countdown form on Z80.
- **#27** (per-pair 16-bit register copy cost): TTI's `getInstructionCost` for `Copy` should reflect Z80's 16-bit copy reality (varies 2-4 bytes depending on pair).
- **#115** (regalloc heuristics gap: greedy picks IY when un-reserved): TTI cost penalty on IY/IX usage via `getInstructionCost` for instructions with DD/FD prefix.
- **#73** (closed; 8-byte memcpy unrolls to ~28B inline): TTI's `getMemcpyLoopLoweringType` should pick LDIR earlier.
- **#87** (closed; 8-byte memcpy threshold): same — TTI memcpy cost.
- **#94, #98, #99** (closed; DJNZ counter regalloc): adjacent to #115; benefits from cost-model awareness.

Indirect benefits (passes that suddenly behave better with TTI):
- LSR (LoopStrengthReduce) — already partially via `isLSRCostLess`, but `getInstructionCost` would refine further.
- IndVarSimplify — countdown bias.
- MachineLICM (post-RA) — uses MachineTraceMetrics which DOES query TTI.  May resolve some of #128's pessimization without global disable.
- MachineCSE (post-RA) — same.
- Inliner — much better short-function inlining decisions.
- SimplifyCFG — branch cost-aware decisions.
- LoopUnroll — depends on `getUnrollingPreferences`.
- MemcpyOpt + LoopIdiomRecognize — LDIR threshold via memcpy cost.

## Hook implementation priority (yield ranking)

Order by likely cumulative impact on AES + production targets:

### Tier 1 — implement first (~highest leverage)

1. **`getInstructionCost`** (the central cost hook).  Many passes use this.  Implementation: switch on Opcode + operand types; return Z80-realistic byte+cycle estimates.
   - 8-bit ALU on A: 1 B / 4 ts.
   - 8-bit ALU on non-A: 2 B / 8 ts (transit through A often required).
   - 16-bit ADD HL,rr: 1 B / 11 ts.
   - 16-bit LD rr,nn: 3 B / 10 ts.
   - i32/i64 ops: libcall cost (high).
   - Floating point: libcall cost (very high).

2. **`getMemoryOpCost`**.  Differentiates A-via-direct (3 B / 13 ts) from r-via-(HL) (1 B / 7 ts) from r-via-(IX+d) (3 B / 19 ts).  Helps MemcpyOpt + LoopIdiom + LICM.

3. **`getCFInstrCost`**.  Branches on Z80 cost ~7-17 ts depending on JR vs JP and condition.  Helps SimplifyCFG + LSR.

### Tier 2 — implement when Tier 1 lands (~medium leverage)

4. **`getArithmeticInstrCost`**.  Refines per-Opcode cost beyond `getInstructionCost`'s default.  Helps LICM/CSE/SLP.

5. **`getCmpSelInstrCost`**.  Z80 compare on A is cheap; on non-A requires LD A, r first.

6. **`getCastInstrCost`**.  Truncates are free on Z80; zexts cost 1 byte (LD H, 0 or similar).

7. **`isLegalAddImmediate` + `isLegalICmpImmediate`**.  Tells LSR/CSE which constants are cheap to materialize.

### Tier 3 — implement together as "no-vectorization" cluster (~low leverage but cleanup)

8. `getMaxInterleaveFactor` → 1.
9. `prefersVectorizedAddressing` → false.
10. `enableInterleavedAccessVectorization` → false.
11. `getMinimumVF`, `getMaximumVF` → 1.
12. `getMinPrefetchStride`, `getMaxPrefetchIterationsAhead` → 0/N-A.

### Tier 4 — exploratory (~unknown leverage)

13. **`getUnrollingPreferences`**.  Z80 loop unrolling rarely helps (branches are 2-3 B; unrolled bodies expand quickly).  Should mostly disable.

14. **`getMemcpyLoopLoweringType`**.  Lower memcpy/memset loops to LDIR earlier on Z80.  Already mostly handled by `Z80LoopIdiomFill`; check for overlap.

15. **`enableMemCmpExpansion`**.  Z80 has CPIR for byte memcmp; expansion might be wrong.

### Tier 5 — opt-attribute-aware tuning (~unlocks #128 permanent solution)

16. **Per-function optsize/minsize gating**.  When the function has `optsize` or `minsize` attribute, return COST values that strongly favor smaller code:
    - Higher LICM hoist cost (don't hoist if hoisting costs > 0 bytes).
    - Higher CSE cost (don't CSE across spills).
    - Lower inliner threshold.
    - Stricter unroll cutoff.

    This is what would let us REPLACE the global `disablePass(MachineLICMID + MachineCSE)` from #128 with a per-function attribute-driven decision, recovering the LICM/CSE wins on the rare functions where they're actually profitable.

## Phased implementation

### Phase A — Investigation (4-6 h, this session if continued)

Goals:
- Trace which passes in the Z80 IR pipeline consult TTI.
- For each, identify the SPECIFIC hooks called.
- Measure AES corpus baseline at -O2 / -Oz / -O3 (we have post-Phase-1 numbers; capture them).
- Quick-grep test-suite for any z80-specific TTI overrides already in target-independent code.

Deliverable: investigation log + per-pass-per-hook inventory.

### Phase B — Tier 1 hooks (1-2 weeks)

Implement `getInstructionCost`, `getMemoryOpCost`, `getCFInstrCost`
as Z80TargetTransformInfo.cpp (new file).

Each hook lands as a separate commit with:
- TDD lit test (a generic IR pattern that the hook affects)
- AES corpus sweep before + after (catch regressions)
- test-runner clang full pass (correctness gate)
- Decision E full oracle gating

Expected outcome:
- LICM/CSE pessimization at -Oz reduces (may not fully eliminate; depends on MachineLICM's TTI usage path).
- LSR makes better IV decisions (some configs see -Os/-Oz improve, especially the count-up shapes that fired in -O3).
- Inliner makes better short-function decisions.

If LICM/CSE pessimization disappears, we can REVERT the #128 disablePass() call.  If it persists in MachineLICM (post-RA), need Phase E.

### Phase C — Tier 2 hooks (3-5 days)

`getArithmeticInstrCost`, `getCmpSelInstrCost`, `getCastInstrCost`,
`isLegalAddImmediate`, `isLegalICmpImmediate`.

Validate same way.  These typically refine existing decisions rather
than enabling new ones.

### Phase D — Tier 3 (no-vectorization cluster) (1-2 days)

Mechanical work.  Mostly returns 1 / false / 0 / N/A.  Eliminates any
spurious vectorizer activity for Z80.

### Phase E — Per-function opt-attribute gating (1 week)

Investigate whether MachineLICM/MachineCSE can be made to respect
`hasOptSize()` / `hasMinSize()` via TTI hooks alone, or whether they
need a target hook in their own logic.  If the latter, file as a
separate upstream-LLVM issue.  Otherwise, implement TTI-based gating
and revert #128's `disablePass()` calls.

### Phase F — Tier 4 exploratory (1 week, may be split)

`getUnrollingPreferences`, `getMemcpyLoopLoweringType`,
`enableMemCmpExpansion`.  Each needs careful empirical validation; may
or may not pay off.

## Validation strategy

For each TTI hook landed:

1. **TDD lit test**: a focused .ll showing the affected pattern produces the expected MIR/asm post-hook.
2. **AES corpus full sweep**: 13 configs × `-Oz`/`-Os`/`-O2`/`-O3`; require no regression (size or tstates) on any.
3. **compiler-comparison-corpus**: sieve/fannkuch/pi pass.
4. **test-runner clang**: 681/46/56/207 baseline maintained.
5. **Production targets**: cpnos PROM1 size + polypascal-test PASS; autoload PROM size; BIOS size.
6. **Cross-mode AES analysis**: track which configs the hook helps and which it doesn't move.

## Risks

### Risk 1: TTI affects many passes simultaneously
A single cost number can shift LSR + LICM + CSE + inliner + vectorizer
all at once.  Per-hook landings keep the blast radius bounded, but
interactions are hard to predict.

**Mitigation**: small hooks, frequent value-oracle gating, ability to
revert per commit.

### Risk 2: Cost numbers are wrong by 1-2× and produce regressions
Z80 cycle counts are exact for documented instructions, but the
"effective cost" depends on regalloc / scheduling outcomes I can't
predict from IR.

**Mitigation**: AES corpus serves as the empirical oracle.  When a
hook lands, compare numbers; tune iteratively.

### Risk 3: Existing #128 disablePass() becomes redundant but loop-rotate / other passes regress
With proper TTI, MachineLICM may again hoist things we DON'T want
hoisted in some configs.  AES corpus would catch most; cpnos PROM1
would catch others.

**Mitigation**: don't revert #128 until Phase E confirms the
replacement is at least as good across the AES corpus.

### Risk 4: Cross-target test regressions
Some LLVM target-independent tests use a default-TTI fallback when the
target's TTI is incomplete.  Adding hooks may flip those tests.

**Mitigation**: Run the full lit suite after each landing; expect
some target-independent test churn that's actually correct.

### Risk 5: Optimization may diverge between -Os and -Oz
Phase E's per-attribute gating must distinguish optsize from minsize;
clang sets BOTH for -Oz, only optsize for -Os.

**Mitigation**: use minsize for the strictest gating (no LICM/CSE/
inline), optsize for moderate (some LICM/CSE/inline but conservative
thresholds).  Document the distinction.

### Risk 6: Upstream LLVM API drift
Many TTI hooks have been renamed/refactored over LLVM versions.  Our
fork is on LLVM 23; some upstream LLVM 24/25 changes may break our
hooks.

**Mitigation**: pin to LLVM 23 (already done); accept the API drift
when we eventually merge upstream.

## Connection to other open issues

| Issue | Status | Impact of #177 |
|---|---|---|
| #27 (16-bit copy cost) | OPEN | Subsumed by Tier 1 (`getInstructionCost`) |
| #115 (regalloc IY misallocation) | OPEN | Helped by Tier 1 cost-of-IX/IY |
| #128 (LICM/CSE -Oz disable) | CLOSED (workaround) | Phase E enables per-function-attr replacement |
| #95 (closed; IV rewrite) | already closed | TTI provides permanent solution-shape |
| #176 (auto-static-stack) | OPEN | Orthogonal; TTI doesn't address frame mode |
| #172 (A-pin liveness) | OPEN | Orthogonal; regalloc surgery, not cost-model |
| #173 (BSS spill peephole) | OPEN | Orthogonal; ISA-specific peephole |
| #175 (8-bit ALU mem op) | OPEN | Orthogonal; missing .td primitives |
| #100 (loop-rotate spill) | OPEN | May be helped by Tier 1+2 cost refinements |
| #38 (IY un-reserve) | OPEN | Tier 1 IX/IY cost penalty would feed the un-reserve decision |

## Effort estimate

- Phase A (investigation): 4-6 h
- Phase B (Tier 1): 1-2 weeks
- Phase C (Tier 2): 3-5 days
- Phase D (Tier 3 cleanup): 1-2 days
- Phase E (opt-attr gating + #128 revert): 1 week
- Phase F (Tier 4 exploratory): 1 week

**Total: 4-6 weeks** of focused work, split across sessions.  Phase A
is the prerequisite for everything else and should be done first.
Phases B-F can land incrementally; each commit is independently
revertable.

## Deliverables per phase

- Phase A: investigation log + per-pass-per-hook inventory in
  `tasks/issue177-phase-a-investigation.md`.
- Phases B-F: one commit per hook, each with lit test + corpus
  sweep numbers in the commit message.
- Phase E: revert of #128's global `disablePass` calls IF AES corpus
  shows MachineLICM/CSE no longer pessimize with proper TTI.

## What this plan deliberately doesn't cover

- **Upstream submission of #177**.  Per `project_z80_upstream_goal`,
  upstream engagement is gated on "substantial work" being ready.
  The TTI implementation is substantial in itself; whether to
  upstream as one PR or in chunks is a future decision.
- **API churn from upstream LLVM 24/25 merges**.  We're on LLVM 23;
  the API is stable for our purposes.
- **MachineLICM/MachineCSE upstream changes**.  If those passes
  don't respect TTI cost hooks at all (which is possible — they're
  post-RA), Phase E may need a separate upstream LLVM fix instead.
- **Performance regression on x86 / ARM**.  N/A — TTI is per-target;
  changes here don't affect other backends.

## First concrete next-action

When this plan is approved and Phase A is started:

1. `Z80TargetTransformInfo.cpp` (new file) — initially empty body,
   wires up the build.
2. Investigation script: build the AES corpus and dump the LLVM IR
   pass log (`-mllvm -print-after-all`) for a representative function
   (e.g., `aes_mc_inv`).  Identify which passes are firing.
3. For each firing pass, grep its source for TTI hook calls.
4. Tabulate: pass → hook(s) called → current default → Z80 should
   override?
5. Sequence Tier 1 hooks based on the Phase A findings.

Phase A's investigation output drives Phase B's specific
implementation priorities, which may differ from this plan's initial
ranking if empirical evidence contradicts my predictions.
