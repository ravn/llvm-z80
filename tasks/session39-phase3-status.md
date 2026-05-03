# Phase 3 status (session 39, 2026-05-03)

Phase 3 of `tasks/roadmap-to-maturity.md` is the **regalloc cluster
(Cluster A)**: `{#98, #94, #89, #99, #27, #38 (deferred)}`.

## Closed / landed

- **#98** — investigation done.  Root cause = register coalescer
  extends loop-counter live ranges across PHI entry-edge merges.
  Doc: `tasks/regalloc-sequential-djnz-investigation.md`.
- **#94** — fix landed (90687fc74d6d).  New pre-RA pass
  `Z80SplitDjnzCounters` + `BReg` single-register class.  Two
  sequential 8-bit DJNZ-eligible loops now both produce DJNZ.

## Partially landed

- **#99** — BCReg + i16 path landed (e9564bf0a9ef).  The i16
  counter is now constrained to BC by class (not just by hint),
  so greedy puts the counter in BC.  Test `issue-97a-bc-pingpong-
  i16-counter.ll` stays XFAIL because the pointer's sdcccall-driven
  coalescing into BC at function entry conflicts with the counter's
  BCReg constraint.  Resolving needs sister `HLReg` constraint for
  the pointer (Phase 3 follow-up).

## Stuck

- **#89** — loop-invariant 16-bit constant reloaded each iteration.
  The repro is a while-shape countdown rewritten by middle-end LSR
  to count UP (`inc a` not `dec a`), in two-MBB header/body form.
  My `Z80SplitDjnzCounters` doesn't apply (no DEC_A in the body),
  the existing DJNZ peephole doesn't apply (no DEC B; JR NZ on
  back-edge), and the rematerialization heuristic re-emits
  `LD DE, @target_fn` inside the loop because the counter's
  allocation chose D.  Real fix needs either:
    - Loop rotation (Phase 5 #77, parked behind #100), OR
    - LSR override on Z80 to keep countdown form, OR
    - Rematerialization cost-model tuning so cheap LD rr,nn isn't
      remat'd into the loop body.
  None of these are in scope for Phase 3.
- **#27** — RFE for per-pair 16-bit copy cost.  No concrete
  regression in cpnos-rom or rcbios; the existing mitigations
  (`CopyCost=3` on IR16, `CostPerUse` on IX/IY, post-RA
  PUSH/POP-fold peephole) hold.  The `BReg`/`BCReg` infrastructure
  added in #94/#99 demonstrates the technique for forcing
  individual-physreg placement when regalloc heuristics
  misallocate; can be reapplied if a concrete IX/IY copy
  pessimization shows up in real code.  Leaving open as RFE.
- **#38** — IY un-reserve.  Re-investigated session 39 (8b268e18eedc)
  after #28's silent IY large-offset spill bug was fixed.  Even
  with that fix, un-reserving IY still produces 11 new runtime
  FAILs across the clang suite — the deeper regalloc / register-
  class issue from the original investigation remains.  Same
  family as #99 (single-register class would help), but spans
  more sites and needs more careful design.  Stays parked.

## State at end of Phase 3 work

- lit: 83 PASS + 1 XFAIL.
- per-function size baseline: zero deltas vs the post-Phase-2
  baseline.
- BIOS: 5928 B byte-exact.
- cpnos-rom payload: 1759 B byte-exact.
- clang test runner -O2: 160 PASS.
- Issues closed by Phase 3: #94, #98.  Partial: #99.  Stuck: #89,
  #27, #38.

## Recommended next steps

The Cluster A items that landed (#94 + #98) are the fully resolved
parts of Phase 3.  Cluster B (Phase 4) is independent and should
unblock more wins:

  1. **#100** — rotation-around-CALL spill.  Closes the gate on
     #77 default-on, which would in turn unblock #89.
  2. **#20** — multi-value spill across CALL.  Direct extension
     of the closed #74 peephole.
  3. **#96** — regalloc-level layer-3 PUSH/POP spilling.  Larger
     scope; investigate before implementing.

Phase 3 follow-up (HLReg + pointer-arg detection for full #99)
can land as a small follow-up commit but isn't required for any
real-world target — pinned for opportunistic work.
