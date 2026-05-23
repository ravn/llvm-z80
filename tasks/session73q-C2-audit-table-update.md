# Session 73q — C2 audit-table update (#180 Migrate column re-audit)

**Date:** 2026-05-23
**Predecessors:** `session73q-C1-drill-180.md`, `session73q-C1-migration-attempt.md`.
**Method:** Quick C1-lens reclassification of the 16 Migrate
candidates from `late-opt-audit-2026-05-02.md`.  Based on:

- C1 finding: FLAGS-dead post-RA peepholes look like easy migrations but
  the late-opt pass's MBB iteration acts as an inadvertent pipeline
  barrier; removing it produces a ~1 B pipeline-ordering regression
  even after full ISel-side migration of the transform.
- C1 finding: even when ISel can emit the optimal form directly, the
  peephole catches additional emit sites (e.g. the i16 EQ/NE byte-XOR
  path).  Full peephole removal requires migrating ALL emit sites,
  and the LOC saved still doesn't cover the regression.

## Reclassification

| #  | Name                                              | Old class | New class            | Note                                                                                                  |
| -- | ------------------------------------------------- | --------- | -------------------- | ----------------------------------------------------------------------------------------------------- |
|  6 | XOR #0xFF → CPL                                   | Migrate   | **Keep** (CONFIRMED) | Drill-tested in C1 — full removal regresses cpnos PROM1 by 1 B even after migrating both emit sites. |
|  7 | LD A,#0 → XOR A                                   | Migrate   | **Keep**             | Same FLAGS-dead post-RA shape as #6.  Zero `ld a,0` survive in BIOS/AES/cpnos, so the peephole catches everything; removal would regress similarly. |
|  9 | OR A; LD r,0; JR Z → OR A; LD r,A; JR Z           | Migrate   | **Likely Keep**      | Branch-form FLAGS-sensitive optimization.  Same shape family — needs FLAGS-dead reasoning at ISel time. |
| 11 | ALU #imm; ALU #imm idempotent (AND/OR)            | Migrate   | **Likely Keep**      | Not a simple CSE — requires backward dataflow ("A is already masked by M, second AND M is dead").  Generic MIR-DCE doesn't model this. |
| 15 | 16-bit increment overflow test idiom              | Migrate   | **Re-test**          | Was filed as "IR-level idiom (LSR) / TTI cost."  After session-73p #128 (LICM/CSE disable) + #177 (TTI hooks), LSR canonicalization may now handle this case.  Drill: disable peephole, measure on AES sweep. |
|  2 | POP rr; PUSH rr elimination                       | Migrate   | Migrate (unchanged)  | Stack-effect; legitimate MIR-DCE candidate.  No FLAGS interaction. |
|  8 | A-via-(HL) via-r → direct LD r,(HL)               | Migrate   | Migrate (unchanged)  | GISel ISel pattern — real codegen improvement, pre-RA. |
| 10 | LD rr,nn; INC/DEC rr → LD rr,nn±1                 | Migrate   | Migrate-and-Keep     | Like #6: ISel migration adds a direct-emit path but peephole catches other sites.  Expect Keep after migration. |
| 12 | LD r,A; LD A,r2; ALU r → ALU r2 (commutative)     | Migrate   | Migrate (unchanged)  | Pre-RA combiner candidate. |
| 13 | LD L,H; LD H,0; LD A,L → LD A,H                   | Migrate   | Migrate (unchanged)  | Truncate fusion via GISel combiner. |
| 14 | dead HL copy in pre-compare narrowed loop         | Migrate   | Migrate (unchanged)  | GISel DCE. |
| 16 | ADD HL,rr commutativity                           | Migrate   | Migrate (unchanged)  | Regalloc cost. |
| 17 | in-memory INC/DEC                                 | Migrate   | Migrate (unchanged)  | Pseudo-expansion or ISel pattern. |
| 18 | comparison reversal (CP r vs CP #imm)             | Migrate   | Migrate (unchanged)  | GISel ISel + regalloc hints. |
| 19 | LD (sym),A + LD HL,sym reordering                 | Migrate   | Migrate (unchanged)  | GISel combiner. |
| 20 | redundant LD A,reg removal                        | Migrate   | Migrate (unchanged)  | Real MIR-CSE candidate. |
| 21 | known-immediate A tracking                        | Migrate   | Migrate (unchanged)  | GISel constant tracking (pre-RA). |
| 23 | HL save-via-BC roundtrip                          | Migrate   | Migrate (unchanged)  | Regalloc cost model. |
| 24 | BC ping-pong in single-BB self-loops              | Migrate   | Migrate (unchanged)  | Regalloc loop-live cost. |
| 25 | u8 switch range-check 16-bit → 8-bit              | Migrate   | Migrate (unchanged)  | IR switch lowering. |

## Summary of changes vs the original audit

- **Migrate → Keep**: 2 (#6 confirmed; #7).
- **Migrate → Likely Keep**: 2 (#9, #11).
- **Migrate → Re-test**: 1 (#15 — possibly obsoleted by #128 + #177).
- **Stay Migrate**: 11 (#2, #8, #10, #12, #13, #14, #16, #17, #18, #19, #20, #21, #23, #24, #25 — note #10 keeps "Migrate" classification but with expected "Migrate-and-keep-peephole" outcome).

## Implication for #180

The original audit's "16 Migrate, ~2300 LOC saving" framing was too
optimistic.  Realistic expectation:
- 4-5 of the 16 are NOT cleanly removable (reclassified above).  LOC saving is zero
  for those; ISel duplication is the only outcome of attempted migration.
- The remaining 11 are likely real Migrate candidates, but each needs the
  C1 "try removing and measure" step before LOC removal can be claimed.
- Expected actual LOC saving on full migration: ~1100-1500 LOC, not 2300.

## Next steps for the audit

1. C3: process #181 DAGISel/GISel coexistence audit (independent of this work).
2. Per-Migrate-candidate drills: pick the highest-LOC candidate (#24 BC
   ping-pong, ~340 LOC; or #21 known-immediate A tracking, ~200 LOC) and
   drill end-to-end.  Each drill ~2-4 h based on C1 evidence.
3. #15 re-test: ~30 min to verify whether session-73p changes obsoleted the
   peephole.  Quick win if confirmed.

This writeup updates `tasks/late-opt-audit-2026-05-02.md`'s classification
column without rewriting the file — the original audit doc is the
"first-pass" record; this is the C1-informed revision.  When the full
C2/C4 work completes, the audit doc can be re-emitted with the final
table.
