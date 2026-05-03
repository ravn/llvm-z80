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
| **Phase 2 — Correctness sweep** | **DONE** (5 of 5).  #28, #36, #63, #81 closed 2026-05-02/03.  **#38 reclassified to Phase 3** in session 42 (2026-05-03 admin pass): session 39's re-test (with #28 + #105 already fixed) produced 11 runtime FAILs + 52 compile FATALs after un-reserving IY, confirming the residual is greedy-regalloc cost-model, not a Phase 2 codegen-correctness item.  #112 + #113 (IY un-reserve plumbing) had already removed the policy-violation half. |
| **Phase 3 — Cluster A regalloc** | **3 of 5 closed already** (excluding the newly-inherited #38).  #94 closed via `Z80SplitDjnzCounters` + `BReg` class (session 39); #98 investigation closed same session (write-up in `tasks/regalloc-sequential-djnz-investigation.md`); **#99 also closed** (BC ping-pong i16-counter sub-case via session 35 peephole + #111 follow-up).  Open: **#89** (loop-invariant DE reload), **#27** (per-pair 16-bit copy cost), and **#38** (carried over from Phase 2; expected to close as a side effect of the cost-model work behind #89/#27, see roadmap §12.3 step 6). |
| **Phase 4 — Cluster B spill mechanism** | Retired per triage (only #100 live; #20/#16/#96 owner-downgraded).  See `tasks/triage-2026-05-03-cluster-b.md`. |
| **Phase 5+** | Not yet active. |

The morning version of this plan claimed Phase 3 was "NOT STARTED"
because the issue list showed #94/#98/#99 as open.  In fact they
were already closed — the triage was reading stale local state.
This evening's re-verification corrects that.

## Engagement-mode gating

Per roadmap section 10.2, engagement-mode (PRs to llvm-z80/llvm-z80)
opens when:

  1. All correctness bugs closed locally.  **Status:** done — Phase
     2 closed in session 42; #38 reclassified to Phase 3 since its
     residual is regalloc cost-model, not codegen-correctness.
  2. One coherent cluster of pessimization fixes closed locally.
     **Status:** Cluster A is **3 of 5 closed** on its original
     membership (#94, #98, #99).  Now also owns #89, #27, #38.  If
     50% closure counts as "cluster fundamentally addressed", we
     are effectively done; otherwise the cost-model work on #89/#27
     (which is expected to subsume #38) is the gate.
  3. Test infrastructure ready.  **Status:** done (Phase 1).
  4. Coordinated narrative for upstream.  **Status:** drafted in
     roadmap section 14.

Engagement-mode is now **one cluster away** (close one Phase 3
cluster).  Loose reading: already met if 3-of-5 Cluster A counts
as "fundamentally addressed".  Strict reading: close #89 and #27
(which carry #38 along).

## Near-term sessions (1-3 ahead)

### Done in session 42 (2026-05-03)

  - **Phase 2 admin pass** — Phase 2 declared DONE; #38
    reclassified Phase 2 → Phase 3.  See roadmap §12.2 update
    and CLAUDE.md session 42 entry.  Commits `de311bfbda4a` (llvm-z80)
    + `df9ed69` (root).
  - **#89 investigation** — TWO paths ruled out empirically.
    Path 1 (drop `isAsCheapAsAMove` from `LD_r16_nn` pseudo):
    BIOS +15 B, cpnos-rom +20 B.  Path 2 (loop-depth check in
    `RegisterCoalescer::reMaterializeDef`): BIOS +3 B, cpnos-rom
    +4 B (5x smaller blast radius but still net negative).  Both
    paths fix the synthetic but regress real workloads because
    the decisive factor at the coalescer-time remat gate is
    register pressure on Z80's 3-pair file, not the structural
    properties either path tried to address.  Diagnosis: MachineLICM
    is fine; RegisterCoalescer pulls hoisted defs back into loops
    via reMaterializeDef; the missing context is pressure, not
    loop depth.  Both reverted; no compiler-source commits landed.
    Findings in `tasks/issue-89-investigation-2026-05-03.md`.
    Conclusion: future #89 work should pursue option (b) [Z80
    pre-RA pressure-aware pass] or option (c) [merge into broader
    regalloc cost-model surface].  Recommendation remains (c).

### Done in evening session 2026-05-03

  - **#113** — GR16NoIR on `XOR_CMP_*16` + `SM83_CMP_Z16`
    operands.  Commit `e4b3496a81b1`.  Lit 90/90 (89 PASS + 1
    XFAIL #99).  Sizes byte-exact.
  - **#121** (filed and closed same session) — drop unreachable
    IR16 PUSH/POP fallback in the four `XOR_CMP_*16` expansion
    cases (~38 LOC).  Commit `c8d2dbedff90`.
  - **#119** — closed as duplicate of #102; the disabled EXX
    block was already deleted in commit `2c9395f645a2` (session
    37).  #119 was filed in error during yesterday's plan re-
    verification.  Late-opt audit doc updated to mark #36 done.
  - **#118** — audit complete (`tasks/audit-
    emitFusedCompareAndBranch.md`, commit `1aed6169`).  Six
    potential gaps for constant-RHS folds in the ISel-time
    compare-and-branch helper examined; **all skip** under the
    structural-first lens.  Function is at a good local optimum
    at ~550 LOC.  Filed sibling **#122** as low-ROI tracking
    issue for the only real-but-zero-fire-site gap (i16 ULT/UGE
    small-const + HighByteZero variable).

### Session N (next): pick one structural entry

**Recommended order under structural lens** (post-session-42
reclassification — #38 is now Phase 3, gated on #89/#27 cost-model
work, NOT a standalone next entry):

1. **#89 — loop-invariant DE reload** (multi-session;
   investigation-first)
   - Layer: regalloc cost model (counter-vs-pointer-vs-pattern
     allocation interaction).
   - Closes: one Cluster A issue + concrete cpnos-rom
     `setup_ivt` 25→17 B win.
   - Why first: largest leverage on remaining Cluster A work; the
     cost-model insight is expected to subsume #38's residual
     greedy-regalloc bug.
   - **Status (session 42):** TableGen-level Path 1 ruled out
     (regression on real workloads).  Next session's deliverable
     is a *design doc* picking among the four options listed in
     `tasks/issue-89-investigation-2026-05-03.md` (a/b/c/d), not
     a fix attempt.  Most likely path is option (c): merge #89
     into the #94/#98/#27 cost-model surface.
2. **#27 — per-pair 16-bit register copy cost** (multi-session)
   - Layer: `getRegAllocationHints` / `getRegClassWeight`.
   - Sibling of #89; touches the same allocator surface.
3. **#120 — combiner work for closed #79 / #93** (parallel thread)
   - Layer: GISel combiner.
   - Enables deletion of audit-Delete peepholes #26, #27, #28
     (~230 LOC).
   - Why available: independent of other threads; can run
     alongside any of 1-2.
   - **Status (session 42):** scoping doc landed
     (`tasks/issue-120-combiner-scoping-2026-05-03.md`).  Next-
     session entry point is the #79 mask-roundtrip sub-task — a
     single combiner rule (`G_SEXT (G_ICMP)` → `G_ANYEXT`) added
     to `Z80PostLegalizerCombiner`.  Doc walks through the IR/MIR
     analysis, the surface area, the verification protocol, and
     the same shape for #93 as the second sub-task.  Estimated
     1 session for #79, 1-2 for #93, 1 for the combined deletion +
     verification.  Total 3-4 sessions for #120.

**Do NOT attempt #38 directly before #89/#27 land.**  Session 39
already proved that path is a dead end (un-reserving IY produces
11 runtime FAILs + 52 compile FATALs even with #28/#105 fixed).
The cost-model work on #89/#27 is the upstream fix; #38's re-test
is a downstream verification step.

**Lesson logged**: when filing follow-up issues, **read the
current source state first**.  #119 was filed from the audit's
text references (lines 4036-4192) without checking that the block
had already been deleted in the same session as the audit was
written.  Cost: trivial (closed as dup), but small audit-the-
state-before-filing waste.

### Sessions N+1 / N+2: Cluster A finishing

  - Whichever of **#89** / **#27** wasn't picked in session N.
  - **#115 — regalloc heuristics for IY-allocatable** — likely
    folded into #38's re-test step rather than handled standalone.

### Sessions N+3 onward: #38 re-test + combiner deletions

- **#38 (carried from Phase 2; re-classified Phase 3 in session 42).**
  Re-test step only — un-reserve IY and re-run the edge_prom suite
  after #89/#27 cost-model fixes have landed.  Expected outcome:
  closes as a side effect of the cost-model work, or narrows to a
  residual that's now bisectable against a known-good cost model.
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
- ~~**#118** (audit emitFusedCompareAndBranch).  Read-only audit;
  fits in any session as drive-by.~~  **DONE** evening 2026-05-03,
  commit `1aed6169`.  Surfaced #122 (low-ROI tracking).

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

## Issue-state snapshot 2026-05-03 end-of-evening (fourth pass)

  - 27 ravn/llvm-z80 issues open at start-of-day.
  - Filed evening: #117, #118, #119, #120, #121, **#122**.
  - Closed evening: #116 (end of session 41 morning), **#113** +
    **#121** (real structural work), **#119** (dup of #102),
    **#118** (audit complete, no code change).
  - **27 currently open.**
  - Z80 lit suite: **90/90** (89 PASS + 1 XFAIL #99) — added
    `issue-113-gr16noir-cmp.ll`.
  - rcbios bios.cim: **5929 B** (byte-exact across all evening
    commits).
  - cpnos.bin: **1777 B** (byte-exact across all evening commits).
  - Working tree clean.

## Commit chain (evening 2026-05-03)

  - `e4b3496a` — #113 GR16NoIR on XOR_CMP_*16 + SM83_CMP_Z16
  - `c8d2dbed` — #121 drop dead IR16 fallback in XOR_CMP_*16 expansion
  - `b1202704` — plan re-verify after #113/#121 close-out
  - `1583e311` — peephole #13 + #17 lit tests (prior-session leftover)
  - `0e26a8a0` — Cluster B retirement triage docs (prior-session leftover)
  - `37d48a9f` — late-opt audit #36 marked done; #119 dup-close note;
                  next-session re-rank
  - `1aed6169` — #118 audit complete

## Lessons logged this session

  1. **Phantom-issue avoidance.**  #119 was filed yesterday from
     the audit document's text references without checking that
     the block had been deleted in the same session as the audit
     was written.  Closed as duplicate; trivial cost but
     preventable: read current source state before filing follow-
     up issues.
  2. **Audit docs decay.**  Line refs in audit documents become
     stale within the same session that wrote them.  Update or
     mark items DONE in the audit doc when fixes land.  The late-
     opt audit's #36 had been pending in the doc for 1.5 days
     after its fix shipped.
  3. **The structural-first lens prevents busywork.**  Six gaps
     in the #118 audit looked individually tempting but all
     deferred under the principle.  Function-level audit confirmed
     the helper is mature; further gains require structural moves
     (combiners / regalloc / IR transforms), not more ISel
     branches.

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

Phase 1 Foundation is done.  **Phase 2 Correctness is done** as
of session 42 (2026-05-03 admin pass): four issues fixed (#28,
#36, #63, #81); **#38 reclassified to Phase 3** because session 39
proved its residual is greedy-regalloc cost-model under -Os
pressure, not a Phase 2 codegen-correctness item.  Phase 3
Cluster A is **3 of 5 closed** on its original membership (#94,
#98, #99 already landed in earlier sessions); now also owns #38.
**#89** and **#27** remain as multi-session investigations and
are expected to subsume #38 as a side effect of the cost-model
fix.  #113 + #121 (evening 2026-05-03) had already closed the IY
un-reservation policy-violation half of #38; what remains is the
regalloc heuristics piece (#115), which is the same surface area
as #89/#27.  Stop adding post-RA peepholes; migrate or delete the
existing ones as their structural fix lands.  Engagement-mode
gate is one cluster away — finish #89/#27 (which carry #38), then
begin upstream interaction.
