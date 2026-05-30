# Fresh-session entry point — after 2026-05-30 (session 74)

Read `tasks/MEMORY.md` first (session-start rule), then this.

## State
- main `94d83af`, CI green, clean.  #27 IX/IY-indexed addressing SHIPPED
  (flag-gated `-mllvm -z80-idx-addr`, default OFF, AES −136 B, production
  byte-identical).  See `tasks/session74-iy-indexed-addr-180-audit-2026-05-30.md`.
- #172 A-pin PARKED (5 negative approaches); branch `z80-loop-carrier-areg-pin`.
- #180 re-audited: tracker ~half stale; ~3–5 genuine migrations remain.

## Pick one to start (ranked)

1. **#181 — DAGISel/GISel coexistence audit** (Tier A upstream gate, fresh).
   First drill: grep `Z80PassConfig` + TableGen selectors for which combiners
   run pre-/post-ISel; list optimizations existing in both paths; find the first
   divergence.  Pairs with the #180 work as the second submission gate.

2. **#180 genuine migration: #8 `LD A,(HL); LD r,A → LD r,(HL)`.**
   Build a load-into-GR8 pseudo (e.g. `LOAD_HL8`) + ISel emission + post-RA
   expansion to `LD <phys>,(HL)` — **reuse the #27 `LOAD_IDX8` pattern verbatim**
   (`Z80InstrInfo.td`, `Z80InstructionSelector.cpp` G_LOAD/G_STORE,
   `Z80ExpandPseudo.cpp`).  Then delete peephole #8 (Z80LateOptimization.cpp:982)
   and verify byte-identical (no codegen win — cleanliness only).  ~1 session.
   NOTE: gated in spirit on #178 (SSA pseudos) but #6/#27 show the pseudo route
   works without it.

3. **Production density (the actual goal)** — #27 did NOT move cpnos/BIOS.
   Per CLAUDE.md the production gap is BSS-spill traffic + regalloc churn
   (#100/#115/#132 family), NOT pointer-indexing.  If chasing cpnos's 26 B
   headroom, look there, not at more addressing-mode work.

## Don't re-do
- #27 Stage 3 (cross-call): ruled out — cpnos/BIOS lack the pattern (measured).
- #172: 5 approaches exhausted; only an ISel snapshot-rotate reshape could move
  it, payoff bounded by a target that already beats SDCC. Not worth it.

## Outstanding GitHub housekeeping (this session filed/commented)
- #27, #180 have status comments; #8-migration sub-issue filed (see its number
  in the session summary / `gh issue list`).
