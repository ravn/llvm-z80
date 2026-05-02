# Z80 Late Optimization Pass Audit

**Date:** 2026-05-02 (session 37)
**File:** `llvm/lib/Target/Z80/Z80LateOptimization.cpp`
**LOC:** 5272
**Method:** Explore agent (read-only) walked the file end-to-end and
produced this first-pass classification.  Content is the agent's
output; numbers are approximate and should be revalidated when
acting on individual patterns in Phase 8.

## Executive summary

37 distinct peephole patterns across 5272 LOC.  Per roadmap §3.3:

- **Keep** (Z80-ISA-specific, no IR/MIR home): **18 patterns** (~1200 LOC)
  DJNZ fusion, EX DE,HL / EXX, ISA-specific immediates (RRCA, SBC A,A),
  branch form selection (JR vs JP), tail-call CALL+RET → JP.
- **Migrate** (repairing higher-pass deficiencies): **16 patterns**
  (~2300 LOC).  Targets: GISel combiner (pre-RA), MIR-CSE / MIR-DCE,
  regalloc cost model and hints, TableGen ISel patterns.
- **Delete** (obsolete or subsumed): **3 patterns** (~150 LOC).
  Disabled EXX conversion (architecturally unsound) plus two
  patterns whose triggers should disappear once GISel combiner
  patches for #79 and #93 land.

**Ceiling estimate** (all migrations + deletions land): ~2450 LOC
removed (≈46%), leaving a hardened ~2800 LOC ISA-specific post-RA
layer.  Phase 8 will revise this number as items move.

## Peephole table

| #  | Name                                                           | Lines      | Class    | Upstream home                                  |
| -- | -------------------------------------------------------------- | ---------- | -------- | ---------------------------------------------- |
|  1 | IX constant propagation + unused IX/IY setup removal           | 371–619    | Keep     | —                                              |
|  2 | POP rr; PUSH rr elimination                                    | 622–660    | Migrate  | MIR-DCE; regalloc cost                         |
|  3 | LD A,r; DEC A; LD r,A; OR A; JR NZ → DEC r; JR NZ              | 662–725    | Keep     | —                                              |
|  4 | DEC A; LD B,A; [OR A;] JR NZ → DJNZ                            | 734–778    | Keep     | (depends on regalloc hint)                     |
|  5 | DEC B; JR NZ → DJNZ                                            | 780–803    | Keep     | —                                              |
|  6 | XOR #0xFF → CPL                                                | 805–823    | Migrate  | GISel combiner (flag-safe form)                |
|  7 | LD A,#0 → XOR A                                                | 825–842    | Migrate  | GISel combiner / ISel pattern                  |
|  8 | A-via-(HL) via-r → direct LD r,(HL) / LD (HL),r (#76)          | 844–915    | Migrate  | GISel ISel patterns                            |
|  9 | OR A; LD r,0; JR Z → OR A; LD r,A; JR Z                        | 917–972    | Migrate  | TableGen select patterns                       |
| 10 | LD rr,nn; INC/DEC rr → LD rr,nn±1                              | 974–1022   | Migrate  | TableGen LD patterns                           |
| 11 | ALU #imm; ALU #imm → ALU #imm (idempotent AND/OR)              | 1024–1041  | Migrate  | MIR-CSE / MIR-DCE                              |
| 12 | LD r,A; LD A,r2; ALU r → ALU r2 (commutative)                  | 1043–1136  | Migrate  | GISel ISel; regalloc tuning                    |
| 13 | LD L,H; LD H,0; LD A,L → LD A,H                                | 1138–1172  | Migrate  | GISel combiner (truncate fusion)               |
| 14 | dead HL copy in pre-compare narrowed loop (#62)                | 1174–1251  | Migrate  | GISel ISel / DCE                               |
| 15 | 16-bit increment overflow test idiom                           | 1253–1357  | Migrate  | IR-level idiom (LSR) / TTI cost                |
| 16 | ADD HL,rr commutativity                                        | 1359–1411  | Migrate  | GISel ISel; regalloc cost                      |
| 17 | in-memory INC/DEC (LD A,(addr); INC A; LD (addr),A)            | 1413–1536  | Migrate  | Pseudo-expansion / ISel pattern                |
| 18 | comparison reversal (CP r vs CP #imm)                          | 1538–1637  | Migrate  | GISel ISel; regalloc hints                     |
| 19 | LD (sym),A + LD HL,sym reordering                              | 1639–1718  | Migrate  | GISel combiner / ISel pattern                  |
| 20 | redundant LD A,reg removal (#60, intra-block)                  | 1720–1799  | Migrate  | MIR-CSE / MIR-DCE                              |
| 21 | known-immediate A tracking (#60/#83/#79)                       | 1801–1999  | Migrate  | GISel combiner (constant tracking)             |
| 22 | LDIR aftermath DE-reuse rewrite (#78)                          | 1950–2083  | Keep     | —                                              |
| 23 | HL save-via-BC roundtrip (#84)                                 | 2085–2200  | Migrate  | Regalloc cost model                            |
| 24 | BC ping-pong in single-BB self-loops (#97)                     | 2202–2541  | Migrate  | Regalloc loop-live cost                        |
| 25 | u8 switch range-check 16-bit → 8-bit (#86)                     | 2543–2652  | Migrate  | GISel ISel / IR switch lowering                |
| 26 | identity mask-roundtrip after SBC A,A (#79)                    | 2654–2716  | **Delete**   | GISel combiner (in-flight)                |
| 27 | carry-roundtrip + JR C → JR NC (#93)                           | 2718–2812  | **Delete**   | GISel combiner (count-up-to-zero idiom)   |
| 28 | ADD A,1; LD r,A → INC r when carry dead (#93 sequel)           | 2814–2898  | **Delete**   | (depends on #27 landing)                  |
| 29 | 16-bit copy loop via HL+ (SM83)                                | 2900–3007  | Keep     | —                                              |
| 30 | LD rr,#imm + LDHL SP folding (SM83)                            | 3009–3088  | Keep     | —                                              |
| 31 | consecutive LDHL SP,#N → INC/DEC HL (SM83)                     | 3090–3167  | Keep     | —                                              |
| 32 | fold constant into XOR compare (CMP_Z16 + imm)                 | 3169–3317  | Keep     | —                                              |
| 33 | LD A,(HL); INC/DEC HL → LD A,(HL+/-) (SM83)                    | 3319–3513  | Keep     | —                                              |
| 34 | SM83 SP-relative store-to-load forwarding                      | 3515–3920  | Keep     | —                                              |
| 35 | IX-indexed static-stack store-to-load forwarding               | 3922–4034  | Keep     | —                                              |
| 36 | EXX shadow-register conversion (DISABLED)                      | 4036–4192  | **Delete**   | (none — unsalvageable)                    |
| 37 | CALL nn; RET → JP nn (tail call)                               | 4194–4251  | Keep     | —                                              |
| 38 | cross-MBB CALL; ⟨fall⟩; RET → JP (#75)                         | 4253–4295  | Keep     | —                                              |
| 39 | Cross-block redundant LD A,r removal (#60, dataflow)           | 4297–4486  | Migrate  | MIR-CSE / regalloc liveness                    |
| 40 | AND $1 + branch/ret → RRCA + carry branch/ret                  | 4489–4563  | Keep     | —                                              |
| 41 | PUSH IX; POP HL; ADD HL,rr; PUSH HL; POP IX → ADD IX,rr        | 4565–4652  | Keep     | —                                              |
| 42 | IX/IY transfer elimination                                     | 4654–4786  | Migrate  | MIR-CSE / regalloc                             |
| 43 | BSS spill/reload → PUSH/POP across CALLs                       | 4787–5033  | Migrate  | Regalloc cost model                            |
| 44 | Redundant PUSH AF/POP AF around BSS spill                      | 5035–5094  | Migrate  | Regalloc / ISel                                |
| 45 | BSS load forwarding (static-stack mode)                        | 5095–5235  | Keep     | —                                              |
| 46 | JP → JR branch shortening (#58)                                | 5236–5258  | Keep     | —                                              |

(Table has 46 rows: 37 distinct patterns + secondary/sub-patterns
broken out for migration ordering.)

## Ordering and dependencies

### Recommended Phase 8 sequence

1. **GISel combiner fixes** (highest leverage; unlock deletions)
   - Combiner patch for #79 mask-roundtrip → unblocks delete of #26.
   - Combiner patch for #93 carry-roundtrip → unblocks delete of
     #27, and #28 (which is downstream of #27 firing).
   - Net: 3 deletions, ~120 LOC.

2. **Regalloc cost-model tuning** (medium leverage; 4 migrations)
   - Counter-allocation hint extension (#92, #94, #99 cluster) for
     better DJNZ formation.
   - Spill-mechanism cost overrides: LD (bss),rr vs PUSH/POP.
   - Loop-live register cost (BC ping-pong, #97).
   - Net: migrations of #23, #24, #43 (~800 LOC).

3. **GISel ISel pattern expansion** (medium leverage)
   - Direct LD r,(HL) / LD (HL),r (#8).
   - Select-lowering TableGen patterns (#9).
   - LD rr,nn ± INC/DEC fold in TableGen (#10).
   - Net: ~600 LOC.

4. **MIR-DCE / MIR-CSE alignment**
   - Verify MIR-DCE eliminates redundant PUSH/POP (#2).
   - Verify MIR-CSE catches redundant LD A,r and commutative ALU
     (#12, #20, #42).
   - Net: ~400 LOC.

5. **IR-level idiom recognition** (lower priority)
   - Count-up-to-zero loop lowering (#15).
   - Switch-lowering delays widening (#25).
   - Net: ~300 LOC.

6. **Janitorial**: delete disabled EXX block (#36, ~150 LOC).
   Zero risk.  Could be done in any session.

### Deletion blockers
- #26–28 blocked on GISel combiner work (in-flight per agent's read).
- #2, #12, #20 require trusted MIR-DCE / MIR-CSE (verify before delete).

## Notable findings

1. **SM83 peephole cluster** (lines 2900–3920, ~900 LOC, patterns
   #29–34): SM83-specific.  Z80 never matches.  Worth considering a
   separate SM83 late-opt pass guarded by `STI.hasSM83()` if SM83
   support keeps growing.  No urgency.

2. **Pattern #39 (cross-block A-tracking)** is a dataflow lattice
   (Top/Bottom/Reg) that fires frequently in real code.  Migrating
   it requires confidence in MIR-CSE + regalloc liveness on real
   workloads.  Defer until Phase 3 (regalloc) is solid.

3. **Tight coupling #26↔#27↔#28**: #28 only fires when #27 fires.
   When the GISel combiner fix for #93 lands, delete #27 and #28
   together, not separately.  #26 (mask-roundtrip) is independent.

4. **Disabled EXX code (#36)**: 150 LOC under `#if 0`.  Should be
   `git rm`'d; current behavior is a no-op but the code drags on
   diffs and code reading.  Truly free win.

5. **Counter-intuitive Keeps**: #41 (PUSH IX; POP HL; ADD HL,rr;
   PUSH HL; POP IX → ADD IX,rr) looks like a peephole-level workaround
   but is the **right home** because the Z80 ISA itself lacks ADD IX,rr
   in the relevant form.  No upstream fix is possible.

## Open questions for Phase 8 executor

1. **SM83 guarding**: gate SM83 patterns behind `STI.hasSM83()` to
   reduce Z80-only binary, or leave them inert for code reuse?
2. **MIR-DCE / MIR-CSE state**: confirm both are actually running
   in the Z80 pipeline before deleting peepholes that depend on
   them.  Check `MachineCSE.cpp` opt-in and `-mllvm` flag defaults.
3. **Regalloc-rewrite risk**: patterns #23, #24, #43 are heavy
   spill-mechanism rewrites.  Before deleting, run rcbios +
   cpnos-rom + bench_string at O0/O1/Os/Oz with and without to
   measure regression risk.

## Caveats on this audit

- Numbers (LOC, pattern counts, line ranges) are agent estimates
  from reading the file.  Validate before quoting in upstream PRs.
- "In-flight" GISel combiner status for #79 / #93 is the agent's
  characterisation.  Confirm with the issue tracker before relying
  on this.
- Pattern names were synthesized from comments + function names;
  some may not match git-blame-level granularity.

## Phase 1 deliverable status

**Complete.**  Roadmap §12.1 calls for "per-peephole classification
keep/migrate/delete".  Delivered above.  Phase 8 will use this as
the starting checklist.
