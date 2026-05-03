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

- **Keep** (Z80-ISA-specific, no IR/MIR home): **19 patterns** (~1270 LOC).
  DJNZ fusion, EX DE,HL / EXX, ISA-specific immediates (RRCA, SBC A,A),
  branch form selection (JR vs JP), tail-call CALL+RET → JP, plus #26
  reclassified Keep 2026-05-04 (post-ISel invariant; see §"Reclassification").
- **Migrate** (repairing higher-pass deficiencies): **16 patterns**
  (~2300 LOC) **— treat as "candidate for migration pending the
  discriminator check"; not yet re-audited**.  Targets: GISel combiner
  (pre-RA), MIR-CSE / MIR-DCE, regalloc cost model and hints, TableGen
  ISel patterns.  See "Migrate/Delete entries NOT yet re-audited"
  paragraph in §"Reclassification".
- **Likely Keep** (under reclassification, defer until profiled):
  **2 patterns** (#27 + #28, ~165 LOC).  Same family as #26 by analysis;
  empirical confirmation pending.
- **Delete (DONE)**: **1 pattern** (#36, ~150 LOC, the disabled EXX
  conversion — already deleted in `2c9395f645a2`).

**Ceiling estimate revised** (all *true* migrations + #36 deletion
land): unknown until the Migrate column is re-audited under the
discriminator.  The original ~2450 LOC ceiling assumed all 16
Migrate entries are category (a); session 42 evidence suggests
several may be category (b) (post-ISel invariant) and not
migratable without large surgery.  Phase 8 will revise after the
discriminator pass.

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
| 26 | identity mask-roundtrip after SBC A,A (#79)                    | 2654–2716  | **Keep** (was Delete; reclassified 2026-05-04 — see §"Reclassification" below) | post-ISel invariant only |
| 27 | carry-roundtrip + JR C → JR NC (#93)                           | 2718–2812  | **Likely Keep** (audit reclassification candidate; defer until profiled) | likely post-ISel invariant |
| 28 | ADD A,1; LD r,A → INC r when carry dead (#93 sequel)           | 2814–2898  | **Likely Keep** (audit reclassification candidate; defer until profiled) | downstream of #27          |
| 29 | 16-bit copy loop via HL+ (SM83)                                | 2900–3007  | Keep     | —                                              |
| 30 | LD rr,#imm + LDHL SP folding (SM83)                            | 3009–3088  | Keep     | —                                              |
| 31 | consecutive LDHL SP,#N → INC/DEC HL (SM83)                     | 3090–3167  | Keep     | —                                              |
| 32 | fold constant into XOR compare (CMP_Z16 + imm)                 | 3169–3317  | Keep     | —                                              |
| 33 | LD A,(HL); INC/DEC HL → LD A,(HL+/-) (SM83)                    | 3319–3513  | Keep     | —                                              |
| 34 | SM83 SP-relative store-to-load forwarding                      | 3515–3920  | Keep     | —                                              |
| 35 | IX-indexed static-stack store-to-load forwarding               | 3922–4034  | Keep     | —                                              |
| 36 | EXX shadow-register conversion (DISABLED)                      | (deleted)  | ~~Delete~~ DONE | deleted in `2c9395f645a2` (closes #102; #119 dup-closed) |
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

1. ~~**GISel combiner fixes** (highest leverage; unlock deletions)~~
   ~~Combiner patch for #79 mask-roundtrip → unblocks delete of #26.~~
   ~~Combiner patch for #93 carry-roundtrip → unblocks delete of #27, #28.~~
   ~~Net: 3 deletions, ~120 LOC.~~

   **Withdrawn 2026-05-04**: session 42 empirically ruled out the
   #79 combiner approach (Z80 BooleanContents = ZeroOrOne; the
   shift idiom is a meaningful widen at IR layer, not an
   identity).  #26 is now classified Keep.  #27/#28 likely Keep
   pending the same reclassification check.  Three migration
   paths remain open (post-ISel combiner, split G_ICMP, change
   BooleanContents) but each is multi-session.  See
   `tasks/lessons-2026-05-04-structural-fix-failures.md` and
   `tasks/issue-120-combiner-scoping-2026-05-03.md`.

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
   Zero risk.  Could be done in any session.  **Done same session
   in commit `2c9395f645a2`** (closed #102; #119 filed in error
   2026-05-03 and dup-closed).

### Deletion blockers
- ~~#26–28 blocked on GISel combiner work (in-flight per agent's read).~~
  Withdrawn 2026-05-04: #26 reclassified Keep; #27/#28 likely Keep.
  See §"Reclassification".
- #2, #12, #20 require trusted MIR-DCE / MIR-CSE (verify before delete).
  Also: re-audit each under the (a)/(b) discriminator before delete.

## Reclassification (2026-05-04, post-session-42 #120 attempt)

The original Keep/Migrate/Delete classification was made without
distinguishing two structurally different reasons a peephole exists:

  - **(a)** **Migratable**: re-derives information available
    earlier in the pipeline, or catches cases that should never
    have been emitted by a cleaner upstream pass.  Migration is
    a structural improvement.
  - **(b)** **Post-ISel-invariant**: exploits a target-specific
    physical-register invariant created by the chosen lowering
    (e.g., "after SBC A,A on Z80, A holds a full mask").  Cannot
    be migrated to GISel/IR without one of: a target-specific
    intermediate analysis pass, a contractual change in the IR-
    level type system (e.g., BooleanContents), or a split of
    instruction-selection into "produce X" pseudo forms.  These
    are large surgeries.  In the meantime the peephole IS the
    right home.

Session 42's attempt to migrate **#26** to a GISel combiner
(`tasks/issue-120-combiner-scoping-2026-05-03.md` "Session 42
attempted implementation") empirically demonstrated #26 is in
category (b): Z80's `BooleanContents = ZeroOrOne` means the
`(shl 7; ashr 7)` shift idiom that the combiner would elide is a
meaningful widen at the IR layer, not an identity.  The peephole
works only because it operates post-ISel where the SBC-A-A full-
mask invariant is observable.

**Reclassified:**

  - **#26** Delete → **Keep**.  Empirically shown in session 42
    that GISel combiner can't soundly elide the asm pattern; the
    invariant the peephole exploits doesn't exist at the IR layer.
  - **#27 / #28** Delete → **Likely Keep**.  Same family as #26
    (the `(shl 7; ashr 7)` idiom appears in #93's count-up-to-zero
    lowering too).  Not yet empirically tested but the soundness
    argument is identical.  Treat as Likely Keep until a profile-
    fire-sites pass confirms.  See lessons doc process rule 4.

**Migration paths for category (b) peepholes** remain open but
multi-session each:

  1. Target-specific post-ISel combiner (Z80-MIR pass between
     ISel and Z80LateOptimization that can see physical-register
     state).  Same effect as keeping the peephole, just at a
     different file location.  Not a structural improvement.
  2. Split G_ICMP lowering into a "produce full mask" pseudo whose
     i8 result is contractually full-mask.  Then a GISel combiner
     can soundly elide the widen.  Larger surgery.
  3. Change Z80's `BooleanContents` to `ZeroOrNegativeOneBoolean`
     target-wide.  Affects every boolean-result lowering path.
     High risk of broad regressions.

**Process rule for future audit-Migrate / audit-Delete entries:**
before declaring an entry migratable, profile its fire sites and
categorise by IR shape; only category (a) is migratable.  See
`tasks/lessons-2026-05-04-structural-fix-failures.md` for the
full process recipe (pre-write MIR dump, layer-context check,
size+value oracles, profile fire sites first).

**Migrate/Delete entries NOT yet re-audited under the discriminator:**
all 16 Migrate entries plus #27/#28 (already flagged Likely Keep
above).  Treat the Migrate column as "candidate for migration
pending the discriminator check", not "ready for migration".  The
discriminator check is cheap (10 minutes per entry: read the
peephole comment, identify the upstream IR shape that triggers it,
ask whether the peephole's rewrite depends on a post-ISel physical-
register invariant).  Worth doing as a batch pass before any
further migration attempt.

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
   diffs and code reading.  Truly free win.  **DONE** in
   `2c9395f645a2` (same session as the audit was written; closed
   #102).  #119 filed 2026-05-03 evening as if pending — closed
   as duplicate.

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
