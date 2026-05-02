# Session 33 — regalloc cluster + BSS spill family (2026-05-02)

## TL;DR

5 issues closed.  rcbios 5967→5920 B (-47 B, -0.79%); cpnos-rom payload
1730→1708 B (-22 B, -1.27%); Z80 lit 73/73 → 75/75.

Branch `z80-regalloc-cluster` (4 commits beyond the merge milestone
`c3dd17710b94`).

## Open-issue count

- Start of session: 28 open / 30 closed.
- End of session:   23 open / 35 closed.

## Closed this session (with verification)

- **#92** Nested-loop DJNZ direction reversed.  Fix: getRegAllocationHints
  now requires a self-back-edge on the dec/jr_nz block to hint B.
  Outer-loop latches branch back to a separate header, so they fall
  to the anti-hint path.  Lit test:
  `issue-92-nested-djnz.ll`.
- **#74** Register-alloc spills should use PUSH/POP for short-lived
  16-bit values.  Two refinements:
  1. Removed the `if (!HasCall) continue` bail (StackDepth balance is
     the real safety guard — pure register-pressure spills win on
     PUSH/POP too).
  2. Cross-register-pair spills (`ld (slot),hl` then `ld de,(slot)`)
     now convert to PUSH (storeReg); POP (loadReg).  Refactored to
     two-pass collect-then-apply so prior conversions' inserted
     PUSH/POP don't interfere with later candidates' scan windows.
  Lit tests: `issue-74-bss-spill-no-call.ll` (no-CALL spill) and
  the rewritten `static-stack-loop-counter-desync.ll` (cross-pair
  conversion of what used to be the #82 orphan).
- **#53** `+static-stack` allocates trivially-constant locals to BSS.
  Verified resolved on current backend — link-time-constant pointers
  are now rematerialised at use sites, no BSS spill.  Likely fixed
  via the broader rematerialisation tuning in earlier sessions.
- **#37** Undocumented `LD A,IYH` for sign-extension.  Verified
  resolved on current backend — the original repro and a stress
  test (5-call IY pressure test) emit zero IXH/IXL/IYH/IYL.  Likely
  fixed via the IY-reservation work in earlier sessions.
- **#39** IX constant-prop removes setup when +undocumented sub-reg
  reads present.  The bulk of the original report was already
  addressed (lines 529, 533, 537 already check IXH/IXL); this commit
  closes a remaining IY symmetry gap on line 538.

## Filed this session

None — all bugs encountered were either fixable in this session or
already covered by existing issues (#94 reproduces the issue's stated
shape; #89 reproduces; #77 reproduces).

## Still open in the regalloc cluster

These share the regalloc cost-model + MachineLoopInfo root cause;
all five would close together with a dedicated session.

- **#94** Sequential 8-bit countdown loops: only one of two gets
  DJNZ.  Live-range splitting needed -- the second loop's counter
  is held alive in B across the first loop, blocking the first from
  also using B.  Currently the fix would need MachineLoopInfo +
  pre-RA live-range splitting.
- **#89** Loop-invariant 16-bit constant reloaded into DE every iter.
  MachineLICM declines to hoist link-time-relocatable LD DE,sym
  because it's marked rematerialisable and -Oz prefers remat.  Fix
  needs MachineLICM tuning specifically for in-loop constant
  materialisations.
- **#77** 8-bit countdown loop with `dec a; ld r, a; or a; jr nz`.
  Root cause is the IR-level countdown→countup IV rewrite at -Oz
  (#95) which produces a head-test loop with unconditional back-jump.
  Fix path: either run LoopRotation at -Oz for this shape, or a Z80
  MIR-level pass that recognises the head-test pattern and reshapes.
- **#95** Long-term path (a) for #93: prevent the IV rewrite from
  countdown to count-up at -Oz.  Tracked as a separate issue.

## Tractable next session (S/M)

- **#18** Cross-MBB known-value tracking.  Per-MBB tracker exists;
  needs dataflow extension.  Modest impact (mentioned 1 instance in
  PROM).
- **#50** Unrolled LDI / Duff's device.  Pure perf optimisation;
  needs opt-level gating.
- **#20** Multi-value BSS spill across CALL.  The recent #74 work
  may have already taken the easy cases — a re-survey of rcbios
  fdc functions would confirm.

## Calling-conv cluster (L) — unchanged

#16, #12, #27, #40, #15.  Need a dedicated design session before
piecemeal changes.

## Correctness bugs (L) — each tractable, time-budget dependent

#28 (-O0 large functions), #36 (va_arg), #38 (large function codegen),
#63 (bench_string -O0).  Each needs reproducer minimisation.

## User feedback / housekeeping

- User flagged a TODO during this session: "Investigate peephole
  optimizations to see if they hide underlying issues to fix."
  Captured in `tasks/peephole-vs-root-cause.md` for the next pass.
- "standard c library is for later" -- #35 stays open as deferred,
  not wontfix.

## Sizes (final)

- rcbios BIOS:    5967 → **5920 B** (-47 B, -0.79%).  Smallest yet.
- cpnos-rom payload: 1730 → **1708 B** (-22 B, -1.27%).
- Z80 lit suite:  73/73 → **75/75**, no XFAILs.
- Open issues:    28 → **23**.
