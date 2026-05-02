# Z80 backend completion roadmap

**Date:** 2026-05-02 (session 36).
**Branch:** `session-36-code-density-plan` in llvm-z80.
**Relationship to other plan docs:**
  - **`fix-plan.md`** is the canonical plan for the project.  It
    predates this file and covers Clusters 1-7 (8-bit primacy, DJNZ +
    LDIR, memcpy thresholds, known-value, direct-address, loop-body,
    calling conv).  This roadmap **complements** fix-plan.md by
    adding the regalloc-completion work (DJNZ-as-primary, remat
    framework) that emerged from sessions 33-35.
  - **`plan-code-density-2026-05-02.md`** was a false start by this
    session.  Marked superseded.

The frame-of-reference here ("16 backend areas to audit") is
orthogonal to fix-plan.md's cluster framing.  Both views are valid:
clusters group related symptoms; backend areas group related
infrastructure.  A given fix lands in one area but may close issues
across multiple clusters.

## 1.  Reframing

The Z80 backend in this repo is **preliminary and unfinished**.  The
goal is to **finish it correctly**, not to chase BIOS bytes against
SDCC.  Per project memory `project_z80_backend_unfinished.md`, this
framing affects every individual fix decision.

Concrete consequence: each open issue is a **gap in incomplete
backend infrastructure**, not a bug to patch.  Code-density numbers
(BIOS 5920 B, cpnos-rom 1708 B) are *measurements of completeness*,
not the goal in themselves.

This is also the strongest reading of the
`feedback_root_cause_over_peephole.md` rule: the peephole layer in
`Z80LateOptimization.cpp` should ideally contain **only Z80-ISA-
specific patterns with no IR/MIR representation** (e.g. `EX DE,HL`
register swap, `BIT n,A`, `SBC A,A` carry materialization).
Anything else in late-opt is a stand-in for unfinished backend
work and is a candidate for migration upstream.

## 2.  Backend areas (audit dimensions)

A "finished" LLVM backend has complete, target-correct
implementations of these areas.  The Z80 backend's status in each
must be audited:

| # | Area | Files | Status (preliminary) |
| -- | -- | -- | -- |
| 1 | TableGen instruction definitions | `Z80InstrInfo.td` (~1300 lines) | Mostly there; `isReMaterializable` partial |
| 2 | Instruction selection patterns | `Z80InstructionSelector.cpp` | Largest file; gaps cause peephole stand-ins |
| 3 | GISel legalization | `Z80LegalizerInfo.cpp` | Unknown coverage |
| 4 | GISel reg-bank info | `Z80RegisterBankInfo.cpp` | Unknown coverage |
| 5 | Calling conventions | `Z80CallLowering.cpp` | sdcccall(0)/(1) partly working |
| 6 | Frame lowering | `Z80FrameLowering.cpp` | hasFP=false runtime bug (#12 parked) |
| 7 | Register info | `Z80RegisterInfo.cpp` | `getRegAllocationHints` partial (#92) |
| 8 | Instr info | `Z80InstrInfo.cpp` | `isReallyTriviallyReMaterializable` partial |
| 9 | Target lowering | `Z80ISelLowering.cpp` (DAGISel) | Coexists with GISel; status unclear |
| 10 | TargetTransformInfo | `Z80TargetTransformInfo.{h,cpp}` | **No Z80 TTI** (#95 needs one) |
| 11 | Pseudo expansion | `Z80ExpandPseudo.cpp` | Working; possibly incomplete coverage |
| 12 | Peephole layer | `Z80LateOptimization.cpp` | **Bloated**; multiple stand-ins for upstream gaps |
| 13 | IR-level Z80 passes | `Z80LoopRotate.cpp`, `Z80LoopIdiomFill.cpp` | Working but gated off (#100); add as needed |
| 14 | Calling convention modeling | TableGen + CallLowering | Partial |
| 15 | Inline asm | `Z80AsmPrinter.cpp` + ISel | `"hl"` constraint crashes (Known Bugs) |
| 16 | Lit test coverage | `llvm/test/CodeGen/Z80/` | 76 PASS + 1 XFAIL — coverage spotty |

## 3.  Open-issue → backend-area mapping

Each open issue maps to one or more backend areas.  Issues that map
to area **#12 (peephole layer)** alone are candidates for migration
upstream when the upstream area is filled.

| Issue | Title | Maps to areas | Upstream target |
| --- | --- | --- | --- |
| #12 | hasFP=false IX-frame overhead | 6 | Frame lowering |
| #15 | Loop index→pointer conversion | 7, 8, 12 | Regalloc remat |
| #20 | BSS spill across CALL multi-value | 7, 8, 12 | Remat / spill placement |
| #27 | Per-pair 16-bit register copy cost | 7, 10 | Cost model |
| #28 | -O0 codegen failures | 2, 5 | ISel coverage |
| #36 | va_arg broken | 5, 9 | CallLowering / ISelLowering |
| #38 | Large fn codegen needs +undocumented | 7 | Reg class for IY (parked) |
| #40 | IX frame ptr vs static-stack per-fn | 6 | Frame lowering |
| #42 | Intrinsics for DI/EI/HALT/IM 2 | 1, 2 | TableGen + ISel |
| #43 | Custom CC for BIOS entry points | 5, 14 | CallLowering |
| #50 | Unroll memcpy/memmove → LDI | 13 | New IR-level Z80 pass |
| #63 | bench_string fails at -O0 | 2 | ISel coverage |
| #70 | -fverbose-asm no source comments | 15 | AsmPrinter |
| #77 | 8-bit countdown DJNZ vs dec/jr | 1, 7, 12 | DJNZ as primary |
| #81 | Integrated asm rejects ex af,af' | 15 | Asm parser |
| #89 | Loop-invariant 16-bit reload in DE | 7, 8 | Remat + reg-class hints |
| #94 | Sequential DJNZ: only one gets it | 1, 7, 12 | DJNZ as primary |
| #95 | IV rewrite countdown→count-up | 10, 13 | TTI + Z80LoopForm IR pass |
| #96 | Regalloc PUSH/POP for short 16-bit | 7, 8 | Spill cost model |
| #98 | B-dead between sequential DJNZ | 1, 7 | DJNZ as primary (B liveness) |
| #99 | i16-counter BC ping-pong | 7 | Reg-class swap hint |
| #100 | Rotation-around-CALL spill | 7, 8 | Remat for cheap forms |

## 4.  Sequenced completion plan

Order by leverage (issues closed per change) and by dependency
(later phases benefit from earlier ones being done).

### Phase 1 — Audit (no codegen changes)

Walk each of the 16 backend areas and produce a "what's missing"
checklist.  Output: `tasks/audit-2026-05-02.md` per area.

Effort: ~1 day reading.  Critical because the plan below is
**hypothesis-quality** until the audit confirms what's actually
incomplete.

### Phase 2 — Complete DJNZ (closes #77, #94, #98; touches #92)

**Backend areas:** 1 (TableGen), 7 (RegisterInfo), 12 (peephole removal).

The DJNZ instruction is currently selected at post-RA peephole time
from `DEC B; JR NZ`.  Its TableGen def at `Z80InstrInfo.td:160`
declares `Uses = [B]; Defs = [B, FLAGS]` but does not model B's dead
fall-through value.

Completion:
1. Add `isBranch`, `isTerminator`, `isBarrier` flags as appropriate.
2. Model the dead-on-fall-through B via `let isCall = 0; let
   hasSideEffects = 0; let mayLoad = 0; let mayStore = 0;` plus
   custom `Uses/Defs` reflecting the kill.
3. Add an early instruction-selection pattern matching the
   countdown loop shape directly to DJNZ instead of relying on the
   post-RA peephole.
4. Re-evaluate `getRegAllocationHints` (the #92 prefer-B hint).
   If DJNZ is now primary, the hint may be redundant.
5. Remove the post-RA peephole at `Z80LateOptimization.cpp:760-803`
   once the primary path covers all measured cases.
6. Lit tests for: single-loop DJNZ, sequential-loop DJNZ, nested
   DJNZ inside non-DJNZ outer, DJNZ with non-trivial body.

### Phase 3 — Complete rematerialization (closes #15, #89, #99, #100)

**Backend areas:** 1, 7, 8, 12.

Audit `isReMaterializable` flags in `Z80InstrInfo.td` and
`isReallyTriviallyReMaterializable` in `Z80InstrInfo.cpp`.
Z80's cheap forms (`LD r,nn` 3 B, `LD HL,nn` 3 B, `LD r,(nn)` for
non-volatile globals 3 B) should all be remat targets.

Completion:
1. List existing `isReMaterializable = true` entries and confirm
   they cover all cheap forms.
2. Add missing entries; verify `isReallyTriviallyReMaterializable`
   matches.
3. Cost model: when regalloc considers spill vs remat, the cost of
   `LD HL,nn` (3 B) must beat `LD (slot),HL; LD HL,(slot)` (8 B).
4. Specifically: extend remat to cover the rotation-around-CALL
   shape (#100) — when a cheap-remat-able value is needed across a
   CALL, prefer remat to BSS spill.
5. Lit tests: closed-form repros for #15, #89, #99, #100.
6. **At end of Phase 3**, flip `Z80LoopRotate` `cl::init(false)`
   → `cl::init(true)` (closes #77a).  Re-measure BIOS / cpnos-rom;
   should show negative bytes.

### Phase 4 — Complete TargetTransformInfo (closes #95)

**Backend areas:** 10.

Today there is no Z80-specific TTI.  Generic LSR / IndVarSimplify
makes count-up-with-carry decisions that are wrong for Z80.

Completion:
1. Create `Z80TargetTransformInfo.{h,cpp}` if it doesn't exist.
2. Override the cost hooks LSR and IndVarSimplify use to choose
   between count-up and count-down forms.
3. Add a `Z80LoopForm` IR-level pass (option 4 from #95) as a
   safety net that runs after IV simplification.
4. Lit tests: countdown loops with i8/i16 counters at -Oz.

### Phase 5 — Calling convention completion (closes #36, #43)

**Backend areas:** 5, 14.

`va_arg` doesn't work (#36).  Custom CCs for BIOS entry points (#43)
not supported.

Completion: read existing `Z80CallLowering.cpp`, identify what's
missing, fill in.

### Phase 6 — Coverage gaps (#28, #63, etc.)

**Backend areas:** 2.

-O0 codegen has known holes (#28, #63).  Fill instruction-selection
gaps so all `-O0..-Oz` levels work.

### Phase 7 — Inline asm + asm parser (closes #70, #81; partial fix #38)

**Backend areas:** 15.

`"hl"` constraint crash; integrated asm rejects `ex af, af'`;
`-fverbose-asm` doesn't annotate.  These are user-visible
correctness issues.

### Phase 8 — Late-opt peephole audit and migration

**Backend areas:** 12.

After Phases 2-7, audit `Z80LateOptimization.cpp` end-to-end and
classify each peephole:
- **Keep:** Z80-ISA-specific (no IR-level form).
- **Migrate:** stand-in for now-completed upstream area.
- **Delete:** subsumed by upstream completion.

This is the "spring cleaning" phase that finishes the work.

## 5.  Phase ordering rationale

Phases 2-4 are the **highest leverage** (close 9+ open issues
between them) and are also the most **structurally fundamental**:
DJNZ as primary, remat as primary, TTI as primary.  These define
how the regalloc and pre-RA pipeline see Z80.

Phase 5 (CCs) is high-leverage but mostly orthogonal — can run
in parallel.

Phase 6 (coverage) is necessary but lower-priority unless -O0 is
needed for debugging (it probably is).

Phase 7 (inline asm) is user-visible but small in scope.

Phase 8 (peephole audit) is the **finish line**: when the upstream
areas are complete, the peephole layer should shrink dramatically.
The size of `Z80LateOptimization.cpp` is a measurement of
completeness — finished backend = small late-opt.

## 6.  Risks

- **Phase 1 (audit) takes longer than estimated.**  Mitigation:
  hard-cap at 1 day; the plan can refine itself iteratively.

- **Phase 2 (DJNZ as primary) regresses non-DJNZ loops.**
  Mitigation: keep the post-RA peephole alive as a safety net
  during Phase 2 development; remove only after measurement.

- **Phase 3 (remat) interacts with Phase 2.**  Sequencing matters:
  Phase 2 first means the regalloc sees DJNZ-eligible loops with
  proper live-ranges, which simplifies Phase 3's cost model.

- **Phases 5-7 may surface unknown holes** in legalization or
  ISel.  These would be filed as new issues, not blockers for
  Phases 2-4.

## 7.  Out of scope

- **HiTech compiler integration** (per memory
  `project_hitech_third_compiler.md`): pending TODO; not part of
  Z80 backend completion.
- **rcbios/cpnos-rom source-level cleanup**: orthogonal.  Source
  changes can be made independently of backend completeness.
- **MAME-side / hardware-side work** (#6 PIO regression, etc.):
  unrelated to LLVM Z80 backend.

## 8.  Success criterion

The Z80 backend is "finished correctly" when:

1. All 24+ open issues mapping to backend areas (Section 3) are
   closed or have explicit reasons to defer.
2. `Z80LateOptimization.cpp` contains **only** Z80-ISA-specific
   patterns (audit per Phase 8).
3. Lit suite covers each completed backend area with regression
   tests (no XFAIL except for genuinely-parked items).
4. cpnos-rom / rcbios sizes are competitive with SDCC across all
   tested workloads (not just aggregate).
5. -O0..-Oz all produce correct code on the project workloads.

## 9.  Next concrete action

Phase 1 (audit) — read each of the 16 backend areas and produce a
gap checklist.  Output goes in `tasks/audit-2026-05-02.md` (one
file or one per area, decide during the audit).

This roadmap will be revised based on what the audit finds.  Phases
2-8 are **hypothesis-quality** until then.
