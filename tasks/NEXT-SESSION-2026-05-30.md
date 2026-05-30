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

## Pick one to start (ranked)

1. **#211 — migrate #8 `LD A,(HL); LD r,A → LD r,(HL)`** (the sub-task I filed).
   Build a load-into-GR8 pseudo (`LOAD_HL8`) + ISel emission + post-RA expansion
   to `LD <phys>,(HL)` — **reuse the #27 `LOAD_IDX8` scaffolding** (`Z80InstrInfo.td`,
   `Z80InstructionSelector.cpp` G_LOAD/G_STORE, `Z80ExpandPseudo.cpp`).  Then
   delete peephole #8 (`Z80LateOptimization.cpp:982`); verify byte-identical
   (no codegen win — upstream cleanliness only).  ~1 session.  Caveat: `(HL)`
   needs the base in **HL specifically** (not IR16) — may need an HL-constrained
   class, more involved than #27's IR16 case.  #6/#27 show the pseudo route works
   without #178.

2. **Production density (the actual project goal)** — #27 did NOT move cpnos/BIOS
   (measured zero; they lack the pointer pattern).  The real gap is BSS-spill
   traffic + regalloc churn: **#110 / #115 / #100** (regalloc cost model) and
   **#178** (remat, mechanism-blocked).  This is where cpnos's 26 B headroom and
   BIOS density live — and the harder, higher-value work.

3. **Other genuine #180 migrations** (#10, #17, #19, #25) — same infra-build
   shape as #211, all cleanliness-only (no codegen win).  Lower priority than (2).

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
