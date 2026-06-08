# Session 2026-06-08 — cost-model refinement Phases 0 through 4

## TL;DR

Followed `tasks/plan-z80-cost-model-refinement-2026-06-08.md` through
seven sub-deliverables in one stretch.  Cumulative production
improvement (default ON):

  - AES -Oz: −70 B text / −8.9 % tstates
  - AES -O2: **−132 B text / −9.2 % tstates** (2.9 % size win)
  - autoload-in-c PROM: recovered 13 B of the post-#23 regression
    (was +64 B raw / +25 B compressed; now +51 B raw / +13 B
    compressed)
  - cpnos PROM1: unchanged from session-start
  - rcbios BIOS: unchanged
  - Correctness: lit 149 PASS + 4 XFAIL, runtime 854 PASS / 0 FAIL
    / 0 FATAL across O0..Oz

Master gate: `-mllvm -z80-use-tiered-cost-model` (default ON; set
false to restore pre-tiered behavior without rebuilding clang).

## Phase-by-phase

### Phase 0 — Measurement infrastructure

**Commits:**
  - `93f94560354f` — `tasks/tools/measure-all.sh` (long-format TSV
    harness, builds every production target from clean, records
    clang SHA + timestamp + CLANG_EXTRA in TSV header)
  - `8cdf0b0` (workspace) — new memory rule
    `feedback_no_op_control_measurement` mandating 3-cell discipline
    (baseline / no-op-control / feature-ON) for every cost-model
    change

The harness eliminated the wc-c-on-stale-file failure mode that bit
twice earlier in the day.  Validation: ran the harness twice with
the same inputs, got byte-identical data rows.

### Phase 1 — Cost-query hook signatures

**Commits:** `7c28d7447781` + workspace `39eec4a`

Added three Z80-specific cost queries with conservative defaults:
  - `Z80InstrInfo::getRematCost(MI)` → `getInstSizeInBytes(MI)`
  - `Z80InstrInfo::getSpillCost(RC, Kind)` where
    `Kind ∈ {BSS, PushPop, IXIYIndex}` → returns 6, 2, 6 respectively
  - `Z80InstrInfo::useTieredCostModel()` → convenience wrapper for
    the `-z80-use-tiered-cost-model` cl::opt (default OFF at this
    phase)

No consumers yet wired.  Three-cell verification: baseline / flag
OFF / flag ON all produced byte-identical TSV.  Phase 3 reads these.

### Phase 2 — Tiered GR16 pressure limit

**Commits:** `b081796b8c31` + workspace `4373b97`

`Z80RegisterInfo::getRegPressureSetLimit` override: report GR16 set
limit as 6 register units (3 cheap pairs HL/DE/BC) instead of
TableGen's auto-generated 12 (counts IX/IY/AF too).  Regalloc
empirically uses HL/DE/BC for short-lived values; reporting the
practical budget here aligns MachineLICM's pressure check with
regalloc's effective behavior.

Gated by `-z80-tiered-gr16-pressure` (default ON, since measurement
showed AES −18 B at -Oz with no regression on cpnos/rcbios).

Path not taken: full TableGen sub-class reshuffle (split GR16 into
GR16NoIR/IR16/GR16AF each its own pressure set).  TableGen merges
pressure sets when one class is a strict subset of another, and
GR16NoIR ⊂ GR16, so forcing separate sets requires breaking the
subset relationship — invasive change to ISel patterns.  The
override is the simpler route; revisit if measurement shows it
needed.

### Phase 3 chapter 1 — shouldHoist CALL-veto

**Commits:** `81165bdfcc5d` + workspace `3f178d4`

`Z80InstrInfo::shouldHoist(MI, Loop)` override: refuse to hoist
rematerializable instructions out of a loop whose body contains a
CALL.  Under sdcccall(1), CALL clobbers HL/DE/BC; a hoisted vreg's
live range crossing the CALL would spill to BSS (6 B per pair)
exceeding the natural remat cost.

Gated by Phase 1's `-z80-use-tiered-cost-model` (still default
OFF after this commit).  Three-cell verification: feature-OFF byte-
identical to pre-Phase-3 state.  Feature-ON gives extra AES −26 B
at -Oz and a small autoload win.

### Phase 3 chapter 2 — leaf-loop high-pressure veto

**Commits:** `4035d3cbd221` + workspace `2b32dae`

Extends `shouldHoist` with a second arm: count preheader
instructions that are **also** rematerializable **and** whose def is
used inside the loop; refuse the candidate hoist when that count
reaches CheapPairBudget = 3.

This is the narrower replacement for the ravn/llvm-z80#220 attempt's
broad filter (which counted ANY preheader def with in-loop use, led
to over-counting + presence-cost).  The narrower
"rematable-AND-used-in-loop" filter targets the exact LICM-bypass
scenario.

Catches the autoload define_sextants nested LEAF loop case (where
ch1's CALL veto doesn't fire).  Three-cell verification: feature-OFF
byte-identical to pre-ch2 state (no presence-cost; the #220 trap
genuinely avoided).  Feature-ON adds AES −13 B + autoload −7 B raw.

### Phase 4 chapter 1 — flip master flag default ON

**Commits:** `1af0f1b857ad` + workspace `e10921b`

`UseTieredCostModel` cl::opt default flipped FALSE → TRUE.  Ships
the cumulative Phase 2 + ch1 + ch2 wins in default builds.  The
flag stays available for diagnosis (set false to restore pre-
tiered behavior).

### Phase 4 chapter 2 — documentation

**Commits:** `7e9217ce92cd` + workspace `368f002`

`tasks/known-suboptimal-codegen.md`: B11 marked SUPERSEDED (the
#220 count-based heuristic is replaced by the multi-phase tiered
cost-model refinement); B12 added for the CSE-induced over-hoist
residual that Phase 3 ch1+ch2 don't fully catch (~+51 B raw on
autoload remains; would need a CSE-cost-model integration which is
Phase 3 chapter 3+ territory).

## What worked

  - **The plan's exploratory framing** (per user 2026-06-08 "no hard
    restrictions on success") let phases ship incrementally — each
    one a small, measured, verified delta.  Compared to the earlier
    ambitious #220 attempt that landed-then-reverted, the
    incremental cadence here held up cleanly.
  - **The no-op-control discipline** (codified in this session as
    `feedback_no_op_control_measurement`) caught two distinct
    near-misses: my initial wrong comparison in Phase 3 ch1 (which
    I had to redo to compare the right cells), and the genuine
    presence-cost trap that #220 fell into (which ch2's narrower
    filter avoided).  Without the discipline I'd have re-shipped
    the same kind of bug.
  - **The measurement TSV harness** (Phase 0) eliminated the
    stale-file failure mode that bit twice earlier in the day.
    The header records clang SHA so old TSVs are dated by build,
    not by measurement timestamp; the long format (one row per
    metric) makes diffs surgical.

## What didn't (yet)

  - **CSE-induced over-hoist** is the residual.  Phase 3 ch1+ch2's
    shouldHoist veto fires for SOME of the cases but not all.
    Tracked as B12 in known-suboptimal-codegen.md.
  - **`getSpillCost` and `getRematCost`** are wired into shouldHoist
    only indirectly — Phase 1's API surface anticipated more
    sophisticated cost arithmetic that Phase 3 didn't end up
    implementing in this session.  Future work could revisit.
  - **Phase 5 upstream filing** — not started.  The two genuinely
    upstream-shaped pieces are:
    1. A `MachineLICM::shouldHoist`-equivalent cost-aware extension
       that consults rematerialization cost vs spill cost (Z80 is
       a witness, AVR/MSP430 likely beneficiaries too)
    2. The `getRegPressureSetLimit` "report-the-practical-budget"
       pattern — applicable to any target where TableGen counts
       registers that the regalloc cost model avoids

## Cumulative session deltas

Vs the pre-#23 disablePass-active baseline (~2 months before this
session):

| target | pre-#23 | end of session | delta |
|---|---:|---:|---:|
| AES -Oz text | 2226 | **2156** | **−70 B (−3.1 %)** |
| AES -O2 text | 4499 | **4367** | **−132 B (−2.9 %)** |
| AES -Oz tstates | 18.21 M | **16.59 M** | **−8.9 %** |
| AES -O2 tstates | 18.21 M | **16.54 M** | **−9.2 %** |
| autoload PROM (compressed) | 1658 | 1671 | +13 B |
| autoload raw .text | 1918 | 1969 | +51 B |
| autoload PROM free | 390 | 377 | −13 B free |
| cpnos PROM1 | 2029 | 1996 | −33 B (build noise; flag-independent) |
| rcbios BIOS | 5908 | 5915 | +7 B (unchanged this session) |
| lit | 149+4 | 149+4 | unchanged |
| runtime | 854/0/0 | 854/0/0 | unchanged |

## Next session candidate work

  - **Phase 3 chapter 3+** — CSE-cost-model integration (the
    autoload +51 B residual).  Open design per the plan.
  - **Phase 5** — upstream filing of the generic pieces.  Drafts
    + per-filing user go-ahead per `feedback_explain_before_filing`.
  - **Production rebuild commit** — `rc700-gensmedet/` PROM artefact
    files still reflect the pre-Phase-2 compiler output; a
    deliberate rebuild commit would update them to reflect the
    new state.  Defer to the moment when the production PROM
    delta matters for hardware testing.
