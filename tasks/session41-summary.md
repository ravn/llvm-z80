# Session 41 summary (2026-05-03)

Closes ravn/llvm-z80#116 with a -4 B rcbios win.  No movement on #114.
Two attempts, one revert, one merge.

## Timeline

  1. **Morning**: #116 ISel-time gate (`hasMinSize()` -> `SUB_HL_rr`).
     Synthetic + lit test passed; rcbios regressed +27 B.  Forcing
     LHS into HL evicts long-lived values, adds BSS reload traffic.
     Reverted in same session (commits `33ceae174673` + merge
     `2bd035317e55`).  See `session41-issue-116-attempt.md`.

  2. **Afternoon**: #116 post-RA peephole in `Z80LateOptimization.cpp`.
     Only fires when regalloc has *already* parked one operand in HL
     and HL is dead-after-branch.  Sidesteps eviction by construction.
     -4 B on rcbios bios.cim.  Commits `f1eece6e0c55` + merge
     `9afb40502956`.  See `session41-issue-116-peephole.md`.

  3. **#114 survey** declined commit.  Six BSS-pair stores in
     rcbios; one (`_bg_set_at` bgstar drawing) matches the static
     #114 shape but is unsafe (DE loop-carried across the inner
     loop).  Other five are misclassified by static pattern
     (memcpy arg setup, BC<->HL transfer through memory).
     ROI: 0 fire sites on current rcbios.  Session 35's BC
     ping-pong peephole already absorbed the simple cases #114
     was designed for; the original `_specc / _scroll /
     _cursor_left / _bios_conin` candidate list (session 36
     strand-B notes) is stale.

## State at end

  - **Z80 lit suite**: 86 PASS + 1 XFAIL (was 85 + 1 at session start).
    New `issue-116-i16-eqne-sbc-hl.ll` covers firing + non-firing.
  - **rcbios bios.cim**: 5933 -> **5929 B (-4 B)**.
  - **cpnos.bin** payload: 1777 B (no change; #116 doesn't fire there).
  - **Open issues**: 26 -> 25 (closed #116).
  - **Branches**: `session-41-issue-116-attempt` (revert),
    `session-41-issue-116-peephole` (closing fix); both merged
    --no-ff into `main` and pushed.

## Lessons

  - "Per-fire savings" math is incomplete without a regalloc-state
    model.  The morning's -1 B/fire projection ignored that consuming
    HL evicts whatever else lived there.  When the savings model
    doesn't account for second-order register pressure, post-RA is
    the safer surface (-> see what regalloc actually did).

  - Static fingerprints of "pattern X" can match *non*-X
    instructions that happen to share opcodes (memcpy arg setup
    looks like a spill bracket).  Counting the shape isn't a
    substitute for understanding the data flow at each match.

  - #114's original ROI projection (~24 B across 4 functions)
    presumed pre-session-35 codegen.  The landscape moved.  Worth
    re-running ROI math whenever a sibling pass lands.

## Carry-forward

**Update 2026-05-03 (post-triage):** the original "Phase 4 Cluster B
(BSS-spill family) — #100, #20, #96, #16" recommendation below is
retired.  See `tasks/triage-2026-05-03-cluster-b.md` for the full
analysis.  Summary:

  - **#20** owner-downgraded April 2026 ("deprioritize in favor of
    #43 or wait for regalloc upstream").
  - **#16** owner-downgraded March 2026 from ~40 B to ~6-8 B; "no
    peephole fix is practical".  Belongs to Cluster A per
    `roadmap-to-maturity.md` section 5.2, not Cluster B.
  - **#96** is filed as investigation only ("no deadline; lower
    priority than #77 and the active regalloc cluster").
  - **#100** is the only live, implementation-ready Cluster B item.

Better next entries, ranked by risk-adjusted ROI:

  1. **#109** — ~50 LOC `LivePhysRegs` walk, finishes session 41
     audit thread (ADD HL,rr commutativity check).
  2. **#89** — Cluster A regalloc; loop-invariant 16-bit constant
     reloaded into DE every iteration despite IR-level hoist.
     Concrete cpnos-rom repro: `setup_ivt` 25 B -> ~17 B.
  3. **#100** — only live Cluster B; option 1 (cross-back-edge
     peephole) is the targeted fix; option 4 (skip rotation when
     CALL in body) defers cheaply.
  4. **#113** — class restriction for undocumented IXH/IXL gating;
     gates IY un-reserve (#38) along with #115.

Roadmap-aligned alternative: start Phase 1 Foundation (CI + size
baseline tracker) per `roadmap-to-maturity.md` Section 12.1.

---

**Re-rank 2026-05-03 (structural lens applied):** the user's
principle "underlying datastructures should reflect z80 properties,
not fix bad modelling with peephole optimizations" reorders the
list above.  See `tasks/triage-2026-05-03-cluster-b.md` "Lens:
structural fixes over peephole accumulation".  Updated ranking:

  1. **#113** — TableGen class restriction (most structural;
     declarative; gates IY un-reserve).
  2. **#98 + #94** — regalloc hint cluster per roadmap section
     12.3.  One cost-model change closes 4-5 issues (#89, #99,
     #27, plus already-closed #92).  Most leveraged.
  3. **#89 alone** — if standalone session preferred over the
     #98 investigation.  Regalloc cost-model fix; structural.
  4. **#100 option 2 or 3** — regalloc layer (rematerialize
     across CALL, or pre-rotation hint).  **Avoid option 1**
     (peephole extension) — it accumulates post-RA fixups for a
     modelling gap.
  5. **Phase 1 Foundation** — infra; makes structural work
     measurable.

Demoted: #109, #108 (peephole audits — safety hardening, not
modelling progress); #100 option 1 (peephole extension).

---

**Original (retired) recommendation:**

Recommended next-session entry point: **Phase 4 Cluster B (BSS-spill
family)** — issues #100, #20, #96, #16.  Per the CLAUDE.md table,
BSS load/store traffic accounts for 30-48% of bytes in the largest
clang BIOS functions; this is the dominant remaining clang-vs-SDCC
gap.  Larger surface area than today's #116, but the gap is real
and measured.

Parked items:

  - **#114 (Z80ShadowBankBracket)**: zero current fire sites; revisit
    when a real motivating loop reappears.  Synthetic + lit fixture
    from session 40 followup remain in place as regression guard.
  - **#116 "neither HL" case**: the post-RA peephole skips when
    neither pair is in HL.  Adding the 2 B move + 3 B SBC = 5 B
    branch (vs 6 B XOR) would save 1 B/fire at the cost of a
    safety check that LHS->HL doesn't evict.  No real motivating
    site in rcbios today; deferred.
  - **`Z80InstructionSelector::emitFusedCompareAndBranch` constant-
    fold**: the existing peephole at line 3205 handles constant-RHS
    XOR-compare folding.  Worth a closer audit — it's adjacent to
    today's work and may have its own tightening opportunities.

## Files

  - `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` — 10-line
    steering note where the failed ISel-time gate was reverted.
  - `llvm/lib/Target/Z80/Z80LateOptimization.cpp` — +170 lines
    (the post-RA peephole proper).
  - `llvm/test/CodeGen/Z80/issue-116-i16-eqne-sbc-hl.ll` —
    firing + non-firing test.
  - `tasks/session41-issue-116-attempt.md` — diagnosis of the revert.
  - `tasks/session41-issue-116-peephole.md` — design of the fix.
  - `tasks/session41-summary.md` — this doc.
