# Structural plan, 2026-05-03

**Supersedes** the "Phase 4 Cluster B (#100, #20, #96, #16)"
recommendation from session 41 carry-forward and the session-36
roadmap framing where Phase 1 was "next active phase".  Folds in:

  - 2026-05-03 triage (`tasks/triage-2026-05-03-cluster-b.md`)
  - User's restated structural-first principle
  - Cross-reference with the session-37 late-opt audit
    (`tasks/late-opt-audit-2026-05-02.md`)
  - Verified phase status against current open-issue state

## Phase status (verified 2026-05-03 evening — second pass)

Roadmap-to-maturity.md was written session 36 (2026-05-02).  Phase
state has advanced further than the morning re-verification claimed:

| Phase | Status |
|---|---|
| **Phase 1 — Foundation** | **DONE.**  CI workflow `.github/workflows/z80-ci.yml` in place; size baseline tracker `tasks/size-baseline.py` (183 LOC) + JSON in place; late-opt audit (session 37) + source-cleanup audit (session 34) both landed. |
| **Phase 2 — Correctness sweep** | **4 of 5 closed.**  #28, #36, #63, #81 closed 2026-05-02/03.  Only **#38** (large-fn IY codegen / IY un-reserve) remains.  #112 + **#113** (IY un-reserve plumbing) closed; together they remove the policy-violation half of #38. |
| **Phase 3 — Cluster A regalloc** | **3 of 5 closed already.**  #94 closed via `Z80SplitDjnzCounters` + `BReg` class (session 39); #98 investigation closed same session (write-up in `tasks/regalloc-sequential-djnz-investigation.md`); **#99 also closed** (BC ping-pong i16-counter sub-case via session 35 peephole + #111 follow-up).  Open: **#89** (loop-invariant DE reload) and **#27** (per-pair 16-bit copy cost).  Both are multi-session investigations. |
| **Phase 4 — Cluster B spill mechanism** | Retired per triage (only #100 live; #20/#16/#96 owner-downgraded).  See `tasks/triage-2026-05-03-cluster-b.md`. |
| **Phase 5+** | Not yet active. |

The morning version of this plan claimed Phase 3 was "NOT STARTED"
because the issue list showed #94/#98/#99 as open.  In fact they
were already closed — the triage was reading stale local state.
This evening's re-verification corrects that.

## Engagement-mode gating

Per roadmap section 10.2, engagement-mode (PRs to llvm-z80/llvm-z80)
opens when:

  1. All correctness bugs closed locally.  **Status:** 4/5 done; #38
     remains.
  2. One coherent cluster of pessimization fixes closed locally.
     **Status:** Cluster A is **3 of 5 closed**.  If 60% closure
     counts as "cluster fundamentally addressed", we are effectively
     done; otherwise #89 + #27 remain.
  3. Test infrastructure ready.  **Status:** done (Phase 1).
  4. Coordinated narrative for upstream.  **Status:** drafted in
     roadmap section 14.

Engagement-mode is now **one or two threads away**, depending on
how strictly "cluster closed" is interpreted.  The pragmatic read:
**close #38, then engage** — Cluster A's remaining two items can
be raised on the upstream tracker if they're genuinely upstream-
relevant rather than serving as a prerequisite gate.

## Near-term sessions (1-3 ahead)

### Done in evening session 2026-05-03

  - **#113** — GR16NoIR on `XOR_CMP_*16` + `SM83_CMP_Z16`
    operands.  Commit `e4b3496a81b1`.  Lit 90/90 (89 PASS + 1
    XFAIL #99).  Sizes byte-exact.
  - **#121** (filed and closed same session) — drop unreachable
    IR16 PUSH/POP fallback in the four `XOR_CMP_*16` expansion
    cases (~38 LOC).  Commit `c8d2dbedff90`.

### Session N (next): pick one structural entry

**Recommended order under structural lens:**

1. **#119 — disabled EXX block deletion** (~150 LOC, zero risk)
   - Layer: pure source cleanup
   - Closes: audit-Delete item; `#if 0` block in
     `Z80LateOptimization.cpp`.
   - Why first: same shape as #121 just done — declarative
     cleanup, byte-neutral, single session.  Useful warm-up
     before tackling #38 / #89.
2. **#38 (Phase 2 closing item).**  IY un-reservation; gated by
   #112 (closed) + #113 (closed) + #115 (regalloc heuristics).
   With both policy-violation gates now closed, the remaining
   work is purely regalloc heuristics — picking IY for
   LDIR/LDDR/HL-tied operands needs to back off.
   - Why second: closes Phase 2 entirely.  After #38 lands,
     engagement-mode gate is met.
3. **#89 — loop-invariant DE reload** (multi-session)
   - Layer: regalloc cost model (counter-vs-pointer-vs-pattern
     allocation interaction).
   - Closes: one Cluster A issue + concrete cpnos-rom
     `setup_ivt` 25→17 B win.
   - Why third: largest leverage on remaining Cluster A work.
4. **#120 — combiner work for closed #79 / #93** (parallel thread)
   - Layer: GISel combiner.
   - Enables deletion of audit-Delete peepholes #26, #27, #28
     (~230 LOC).
   - Why available: independent of other threads; can run
     alongside any of 1-3.

### Sessions N+1 / N+2: Cluster A finishing + Phase 2 close

  - **#27 — per-pair 16-bit register copy cost.**  Sibling of
    #89 in Cluster A.  Multi-session — touches
    `getRegAllocationHints` and possibly `getRegClassWeight`.
    May subsume parts of #89.
  - **#115 — regalloc heuristics for IY-allocatable** if not
    already covered by #38's investigation.

### Sessions N+3 onward: Phase 2 closing (#38) + combiner deletions

- **#38 (Phase 2 last item).**  IY un-reservation; gated by #112
  (closed) + #113 (TableGen restriction, see Session N option 1) +
  #115 (regalloc heuristics).  Largest single-issue effort in the
  immature-backend cluster.
- **#120 — combiner work for closed #79 / #93.**  Write GISel
  combiners that match the IR shapes the post-RA peepholes
  currently rewrite.  When landed, delete audit peepholes **#26,
  #27, #28** (~230 LOC).

Both threads are independent of Cluster A and can run in parallel
with sessions N+1 / N+2.

## Longer-running threads (parked / batched)

- **#100 option 2 or 3** (regalloc layer for rotation-around-CALL
  spill).  Avoid option 1 (peephole).  Depends on #98 design
  landing first.
- **Audit-Migrate batch cleanup.**  Once Cluster A and combiner
  work lands, walk the remaining 8-10 Migrate peepholes (#2, #6-13,
  #15-21 minus the ones already covered above) and migrate or
  close in batches.  Each is small individually; the value is in
  fewer accumulated post-RA layers.
- **Cluster B (only #100 live).**  Park; revisit after Cluster A
  cost-model changes might subsume.
- **#117** (closed-#116 neither-HL extension).  Parked — no
  motivating site today.
- **#118** (audit emitFusedCompareAndBranch).  Read-only audit;
  fits in any session as drive-by.

## Explicit non-goals

- **New post-RA peepholes.**  Including #100 option 1 (cross-back-
  edge peephole extension) and #117's positive-firing extension.
- **#109 / #108 hardening as a primary session goal.**  These are
  safety hardening on peepholes that should be migrated.  Acceptable
  as drive-by while the peephole still exists; not the focus.
- **#20, #16.**  Owner-downgraded; do not reopen.
- **llvm-z80/llvm-z80 engagement (issues, PRs, comments).**
  Workspace mode until both Phase 2 closed and one cluster done
  (per roadmap section 10.2).

## Issue-state snapshot 2026-05-03 evening (second pass)

  - 27 ravn/llvm-z80 issues open at start-of-day.
  - Filed today: #117, #118, #119, #120, #121.
  - Closed today: #116 (end of session 41), **#113** (evening),
    **#121** (evening).
  - **28 currently open.**
  - Z80 lit suite: **90/90** (89 PASS + 1 XFAIL #99) — added
    `issue-113-gr16noir-cmp.ll`.
  - rcbios bios.cim: **5929 B** (byte-exact).
  - cpnos.bin: **1777 B** (byte-exact).

## Cross-references

  - `tasks/roadmap-to-maturity.md` — master plan, per-phase
    detail.  Section 12.4 (Phase 4) updated 2026-05-03 with
    triage results.
  - `tasks/triage-2026-05-03-cluster-b.md` — Cluster B retirement
    rationale + structural-lens reclassification.
  - `tasks/late-opt-audit-2026-05-02.md` — 46 peephole patterns
    classified Keep/Migrate/Delete.
  - `tasks/session41-summary.md` — most recent session log;
    Carry-forward updated 2026-05-03 with structural re-rank.
  - `/Users/ravn/z80/CLAUDE.md` — top-level project pointer;
    updated with this plan.

## Summary, one paragraph

Phase 1 Foundation is done.  Phase 2 has one issue left (#38).
Phase 3 Cluster A is **3 of 5 closed** (#94, #98, #99 already
landed in earlier sessions); **#89** and **#27** remain as multi-
session investigations.  #113 + #121 (evening 2026-05-03) closed
the IY un-reservation policy-violation half of #38; the remaining
gate is regalloc heuristics (#115).  Stop adding post-RA peepholes;
migrate or delete the existing ones as their structural fix lands.
Engagement-mode gate is one or two threads away — close #38,
optionally finish #89/#27, then begin upstream interaction.
