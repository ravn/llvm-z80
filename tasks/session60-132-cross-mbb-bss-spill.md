# Session 60 (2026-05-12) — #132 cross-MBB BSS-spill peephole (single-pred escape)

## Context

Session 58 reframed Phase 3 Cluster A: the in-MBB BSS-spill→PUSH/POP
peephole (#129) was found already exhaustively implemented in two
forms (same-class at `Z80LateOptimization.cpp:4859`, cross-class at
`:5104`), with zero in-MBB residuals in cpnos.lis.  Cross-MBB
residuals — concentrated in the SNIOS retry-loop pattern — were
filed as **#132**.

## Scope this session

The conservative single-pred-escape variant only: every non-LOAD
successor of MBB_A must have MBB_A as its sole predecessor and
must not reference the slot.  Compensation is `inc sp; inc sp`
(2 B, no register clobber) prepended at each escape MBB head.

Deferred (multi-session, per the issue body):
  - edge-splitting variant (handles multi-pred escape targets;
    needed for the SNIOS shape, which exits to a shared error
    path)
  - `MachineDominatorTree`/`MachinePostDominatorTree` plumbing
  - multi-load / multi-escape generalizations
  - liveness-driven compensation (1 B `pop af` / `pop hl` when
    register class is dead at escape entry)

## Implementation

`Z80LateOptimization.cpp` — third peephole inserted between the
existing cross-class peephole (`:5104`) and the redundant
`PUSH AF; POP AF` cleanup (now at `:5550`).

Algorithm:
  1. For each `STORE rr,(sfrend/sframe)` in MBB_A, scan forward to
     terminator tracking PUSH/POP balance.  Bail on any other
     access to the same slot in MBB_A (in-MBB peepholes own that
     pattern) or unbalanced SP at terminator.
  2. Walk MBB_A's successors:
     - Exactly one successor (MBB_B) must contain the matching
       LOAD as its first slot-touching instruction with balanced
       stack from entry.
     - All other successors (escape MBBs) must have MBB_A as sole
       predecessor and not reference the slot.
  3. Confirm no other MBB references the slot (covers transitive
     blocks and unreachable code).
  4. POP AF case: FLAGS dead after LOAD position in MBB_B.
  5. Cost gate: `Save = (StoreBytes-1)+(LoadBytes-1) - 2*Nesc`.
     Fire only when `Save > 0`.
  6. Rewrite: STORE → PUSH (MBB_A), LOAD → POP (MBB_B),
     `inc sp; inc sp` prepended to each escape MBB head.

## Verification

  - **lit suite**: 95/95 (93 PASS + 2 XFAIL, no regressions).
    New fixture `issue-132-bss-spill-cross-mbb.ll` PASS;
    demonstrates the rewrite on a minimal repro (counter spilled
    across `call _target` with bypass-LOAD escape returning 1).
  - **z80-utils test-runner clang Oz** (165 tests):
    113 PASS / 0 FAIL / 1 FATAL / 51 SKIP.  Identical to baseline.
    The single FATAL (`test_48_dynamic_alloca`) is a pre-existing
    `alloca.h` missing on macOS host, unrelated.
  - **cpnos-rom** (clang + pio-irq + #131 callee-side preserves):
    payload **byte-identical** with baseline (1906 B).  Only
    cpnos.bin differs at the 3 BUILD_INFO_STR timestamp bytes.
    The peephole's conservative single-pred-escape gate does not
    fire on any production cpnos function — every SNIOS retry
    loop's escape branch targets a shared error MBB (multi-pred),
    correctly bailed.
  - **MAME boot**: not run; superseded by byte-identical-payload
    finding (per `feedback_diff_binaries_before_blaming_codegen`).

## Why no production-size impact yet

The SNIOS witness in `_snios_sndmsg_force` is exactly the shape
described in #132, but its bypass branch (`jr nz` on
"frame received" or "checksum mismatch") targets a **shared**
error-recovery MBB also reached from later loops.  Sole-predecessor
gate correctly excludes this.  The conservative variant lands
the structural extension and serves as the correctness foundation
for the edge-splitting variant that will fire on these patterns.

## Files touched

  - `llvm/lib/Target/Z80/Z80LateOptimization.cpp` — third peephole
    (single-pred-escape) after the cross-class in-MBB peephole.
  - `llvm/test/CodeGen/Z80/issue-132-bss-spill-cross-mbb.ll` —
    new lit fixture.
  - `tasks/session60-132-cross-mbb-bss-spill.md` — this doc.

## Follow-ups

  - Update GitHub #132 with a comment: single-pred-escape variant
    landed; edge-splitting variant still open as the higher-impact
    follow-up.
  - Edge-splitting variant — needs critical-edge insertion + new
    MBB for compensation `inc sp; inc sp` + back-link to escape
    target.  Adds 2 B per escape edge.  Net save on SNIOS retry
    loops estimated 4 B per pair × ~6 sites ≈ 20-30 B per
    cpnos build (issue body's optimistic estimate).
  - Liveness-driven 1 B compensation (`pop af` when A+FLAGS dead).

## Rules-checked

  - `feedback_compiler_bug_test`: lit test added
    (`issue-132-bss-spill-cross-mbb.ll`).
  - `feedback_test_before_fix`: test was failing before
    implementation, passes after.
  - `feedback_no_commit_first_version`: peephole is value-oracle-
    satisfied via byte-identical cpnos payload + identical
    test-runner results.
  - `feedback_compare_total_section_sizes`: `.payload` total
    measured (1906 B identical).
  - `feedback_diff_binaries_before_blaming_codegen`: cpnos.bin
    diff confined to 3 timestamp bytes; rules out runtime impact
    without an explicit MAME run.
