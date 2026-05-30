# Fresh-session entry point — after 2026-05-30 (session 74)

Read `tasks/MEMORY.md` first (session-start rule), then this.

## State
- main `94d83af`, CI green, clean.  #27 IX/IY-indexed addressing SHIPPED
  (flag-gated `-mllvm -z80-idx-addr`, default OFF, AES −136 B, production
  byte-identical).  See `tasks/session74-iy-indexed-addr-180-audit-2026-05-30.md`.
- #172 A-pin PARKED (5 negative approaches); branch `z80-loop-carrier-areg-pin`.
- #180 re-audited: tracker ~half stale; ~3–5 genuine migrations remain.

## Issue-status note (verified 2026-05-30, end of session 74)
Both Tier A upstream gates are now effectively closed: **#180 re-audited**
(stand-in alarm mostly false) and **#181 CLOSED** (no DAGISel path — confirmed
session 73s/73ab; `Z80ISelLowering` is live GISel `TargetLowering`).  Of the old
"infra gates", **#177 (TTI) and #179 CLOSED**; **#178 (remat) still OPEN**.
Production-density regalloc issues **#110 / #115 / #100 OPEN**.  **#184 CLOSED**.

## Pick one to start (ranked) — updated after the 2026-05-30 #211 investigation

1. **Production density (the actual project goal)** — #27 did NOT move cpnos/BIOS
   (measured zero; they lack the pointer pattern).  The real gap is BSS-spill
   traffic + regalloc churn: **#110 / #115 / #100** (regalloc cost model) and
   **#178** (remat, mechanism-blocked — pseudos with implicit physreg defs break
   `isReMaterializable`).  This is where cpnos's 26 B headroom and BIOS density
   live.  **Hard, multi-session, but the only high-value lever left.**
   Caveat (verified 2026-05-30): the cheap levers are exhausted — #27 indexed
   addressing = 0 on cpnos; #173 BSS-via-A is an AES driver pattern (cpnos has
   only 7 `push af`).  cpnos is already near-optimal per CLAUDE.md; the win is in
   regalloc, not new addressing modes.

2. **#180 genuine migrations** (#211/#8, #10, #17, #19, #25) — **DEPRIORITISED.**
   Investigation of #211 (see its issue comment) found these are pure
   upstream-cleanliness with **zero codegen win**, plus ISA-split / clobber-
   tension complexity (e.g. `LD r,(BC/DE)` doesn't exist; A-live case can't be
   improved).  The peepholes are correct post-RA "Keep"s.  Only worth doing as
   part of a deliberate upstream-submission cleanup pass, not for codegen.

3. **Upstream submission packaging** — both Tier A gates (#180, #181) are
   resolved; the AES codegen wins (#179 P1/P2, #128, #148, #185) + #27 could be
   packaged into a submission writeup (cf. existing `tasks/upstream-*-submission.md`).

## Don't re-do
- #27 Stage 3 (cross-call): ruled out — cpnos/BIOS lack the pattern (measured).
- #172: 5 approaches exhausted; only an ISel snapshot-rotate reshape could move
  it, payoff bounded by a target that already beats SDCC. Not worth it.

## Outstanding GitHub housekeeping (this session filed/commented)
- **#211** filed — #180 sub-task: migrate #8 to a load-into-GR8 ISel pseudo.
- **#27** + **#180** have status comments (resolution / re-audit).
- Maintainer call pending on whether to close **#27** (the IX/IY-indexed lever
  is implemented flag-gated; the per-pair copy-cost-model ask has no concrete
  pessimization).
