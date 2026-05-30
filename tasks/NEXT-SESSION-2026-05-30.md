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

## Pick one to start (ranked) — updated after the 2026-05-30 drills

1. **Upstream-submission packaging** — both Tier A gates resolved (#180 re-audit,
   #181 closed).  Package the AES codegen wins (#179 P1/P2, #128, #148, #185) +
   #27 + the #168/#182 already-written submission notes into a coherent patch
   series / writeup for `ravn/llvm-z80` review.  This is now the **highest-value
   remaining lever** (see why below).  Pattern: existing `tasks/upstream-*-submission.md`.

2. **Production density (regalloc) — TAPPED OUT, do NOT re-chase.**
   Drilled 2026-05-30 (`tasks/production-density-regalloc-drill-2026-05-30.md`):
   BIOS waste is **ISA-fundamental** — 324 BSS-via-A (8-bit memory is A-only),
   ~245 A-shuttle moves (irreducible #172-class), ~65 pair-copies.  Zero IX-stash;
   cpnos near-optimal; ~0 recoverable redundancy.  clang BIOS already **beats**
   SDCC (−194 B).  #178/#110/#115/#100 yield only single-digit-to-low-tens of
   bytes and are hard.  Revisit ONLY if a specific function regresses or a new
   (non-BIOS/cpnos/AES) workload surfaces a different pattern.

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
