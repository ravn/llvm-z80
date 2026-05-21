# Session 73o — #172 A-shuttle: hint negative + structural infrastructure

Date: 2026-05-21.  Follows session 73n (Z80NarrowIV default-on landed).
Triggered by "drill the regalloc A-shuttle" — the biggest remaining
SDCC-gap lever after Z80NarrowIV.

## TL;DR

Two-stage drill on the 8-bit ALU accumulator shuttle:

1. **Hint via `getRegAllocationHints`** — implemented, fires on every
   candidate vreg (verified with `errs()` trace), but greedy regalloc
   ignores it and still allocates D.  Reverted.  This matches the
   pre-existing `Z80RegisterInfo.cpp:1891` comment that hints don't
   move the allocator on this path.
2. **Structural fix via single-register-class pinning** — new `AReg`
   class + new `Z80PinAluAccumulator` pre-RA pass, mirroring
   `Z80SplitDjnzCounters` which is the documented workaround for #94 /
   #98 / #99.  Default-on regressed AES baselines (+261/+285 B on 01/02,
   +3.8 %/+3.9 % tstates) and **segfaulted** on
   `05_Oz_static_stack` because pinning multiple simultaneously-live
   accumulator vregs to A produces unsatisfiable constraints.
   Defaulted off; infrastructure committed.

Net session output:

- **Filed ravn/llvm-z80#172** (A-shuttle issue with the MIR pattern,
  the negative hint result, and four fix paths).
- **Commit `2c4627c80ef1`** on main: `AReg` class + `Z80PinAluAccumulator`
  pass + pipeline wiring + opt-in flag.  Default OFF.

No AES corpus / lit / test-runner regressions (default-off is
behaviourally identical to pre-session state).

## Why hints don't move greedy

The `getRegAllocationHints` hook lets the target add preferred
physical registers for a vreg.  Greedy regalloc uses these as soft
preferences — they're consulted alongside copy-elimination, eviction,
and interference heuristics.  When the hint conflicts with greedy's
copy-elim "route the destination to the source's physreg" preference,
greedy wins.

For the A-shuttle case, the recurrence vreg comes from a COPY whose
source is in some non-A physreg.  Greedy's copy-elim sees that and
keeps the vreg in the source's physreg, ignoring my A hint.  Adding A
to the hint list (even at the front) doesn't override copy-elim.

This is the same mechanism that defeated the i16-counter hint in #99 /
#94 / #98.  The fix there was a single-register-class constraint
(`BReg`, `BCReg`).  That bypasses greedy's heuristics entirely — the
vreg has only one legal allocation, so greedy must pick it.

For A, the equivalent is the new `AReg` single-register class.

## The structural fix attempt

`Z80PinAluAccumulator` walks all GR8 vregs and rewrites their register
class to `AReg` if either:

- The vreg's only use is `$a = COPY %vreg` immediately followed by a
  flag-setting A-RMW ALU opcode (XOR_r / XOR_n / AND_r / AND_n /
  OR_r / OR_n).  This is the "into A" half of a chain.
- The vreg's only def is `%vreg = COPY $a` immediately preceded by an
  A-RMW.  This is the "out of A" half.

Pinning works in isolation.  On `gf_alog_mini` (the standalone repro):

```asm
; before (default-off, GR8 unconstrained):
.LBB0_3: ld   a, h; xor 27; ld h, a       ; conditional XOR via H
.LBB0_4: ld   a, h; xor d; ld d, a        ; final XOR via D

; after (pin enabled):
.LBB0_3: ld   a, h; jr .LBB0_5            ; else: A=H
.LBB0_4: ld   a, h; xor 27                ; if: A=H^27 (no save back)
.LBB0_5: xor  d; ld e, a                  ; final XOR direct on A
```

The `xor 27` → `xor d` chain is now A-resident — exactly what #172
wants.  But the loop *carrier* (atb across iterations) is still in E,
and the entry of each iter still does `ld a, e` to load it.

Why?  The recurrence value flows through a PHI in the loop header.
My predicate only matches vregs at the COPY-to-/-from-$a boundaries,
not the PHI value itself.  Pinning the PHI vreg requires walking
through the PHI cycle.

## Why default-on broke the AES corpus

The pass currently pins ANY matching vreg without checking for
interference with other already-pinned vregs.  `aes_mixColumns` has
nested XOR chains where multiple accumulators are simultaneously live
(typical pattern for finite-field arithmetic with multiple parallel
products).  Forcing them all to `AReg` is unsatisfiable — at most one
can occupy A at any given program point.

Symptoms with default-on:

| Config | Effect |
|---|---|
| `01_baseline_Oz` | +261 B / +3.8 % ts |
| `02_Os` | +285 B / +3.9 % ts |
| `04_O2` | +433 B / +1.2 % ts |
| `05_Oz_static_stack` | **clang segfault during regalloc** |

The crash on 05 is the most informative: it confirms regalloc reaches
an unsatisfiable state on at least one function.  Spill explosions on
01/02/04 are the "merciful" version of the same problem.

## What a working version needs

A liveness-aware selector:

1. **MachineLoopInfo** to find natural loops + their headers/latches.
2. **LiveIntervals** to check whether a candidate vreg's range overlaps
   any already-pinned vreg.
3. **PHI walk** to identify the full accumulator cycle (header PHI +
   latch COPYs); pin the entire chain at once or not at all.
4. **One primary per loop** heuristic — if multiple chains compete,
   pick the one with highest use count / most ALU ops.

This is ~200-400 lines of new code plus careful testing.  Not for this
session.

## Files committed

  - `llvm/lib/Target/Z80/Z80RegisterInfo.td`: new `AReg : Z80Reg8Class<(add A)>`.
  - `llvm/lib/Target/Z80/Z80PinAluAccumulator.{h,cpp}`: the pass.
  - `llvm/lib/Target/Z80/Z80TargetMachine.cpp`: wired into
    `Z80PassConfig::addOptimizedRegAlloc` after MachineScheduler,
    before LiveIntervals re-run (same lifecycle as
    `Z80SplitDjnzCounters`).
  - `llvm/lib/Target/Z80/Z80.h`: init declaration.
  - `llvm/lib/Target/Z80/CMakeLists.txt`: build.

Single commit: `2c4627c80ef1`.

## Quantifying the residual SDCC gap (status snapshot)

Post-Z80NarrowIV, AES production target:

| | clang `09_Oz_prod_like` | SDCC `01_baseline_prod` | gap |
|---|---|---|---|
| size | 2667 B | 3323 B | clang -20 % (smaller) |
| tstates | 14,887,472 | 12,080,289 | SDCC -18.9 % (faster) |

Breakdown of the 2.8M tstate clang lag, by suspected cause (per session
73m / 73o analysis):

| Cause | Est. ts | Issue |
|---|---:|---|
| A-shuttle (XOR/AND/OR chains via A round-trip) | ~1.5M (5 pp) | #172 |
| Other regalloc churn (16-bit copy cost, IY prefix overhead) | ~0.8M (3 pp) | #27, #115 |
| Per-function size/codegen gaps (aes_mc_inv +549 B) | ~0.5M (2 pp) | various |

Plus opportunity from K&R int-promotion fix (#158 / #159) which
isn't gated by clang work -- it's SDCC-side fix that already landed
this branch's K&R REGPARM-preserve patch and gives `rj_sb_inv` -126 B
on the SDCC side.

## What was Easy / Hard

**Easy**: the hint experiment.  ~20 lines added to
`getRegAllocationHints`, instrumented with `errs()`, fired correctly,
zero asm change.  Clean negative result in 30 min.

**Easy**: filing #172.  The asm comparison and the four-fix-path
ranking practically wrote themselves from the hint failure.

**Medium**: the structural pass infrastructure.  Mirroring
`Z80SplitDjnzCounters` was straightforward (same file structure, same
helpers, same pipeline hook).  About 200 lines including comments.

**Hard**: predicates that match the real MIR pattern.  First try
(require both COPY-to-A and COPY-from-A on same vreg) fired zero
times because the chain has separate vregs at each boundary.  Relaxed
to either-side; fires but doesn't pin the PHI value itself.

**Painful**: the segfault on 05_Oz_static_stack.  No diagnostic from
the regalloc — just SIGSEGV.  Took the +3.8 % regression on 01/02 to
realize the issue is overlap, not a bug in my predicate.  Backing out
to default-off was the right call.

## Difficulty: Medium

Pass infrastructure is real progress (the AReg class + the pre-RA
hook + the documented "hints don't move greedy" finding are durable
artifacts), even though the default-on flip is gated on the liveness
work.  Honest stopping point.
