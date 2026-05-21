# Session 73p — #172 A-pin: conservative MBB-local scope

Date: 2026-05-21.  Direct follow-on to session 73o.

## TL;DR

Tightened `Z80PinAluAccumulator` from "pin every matching GR8 vreg" to
"pin only vregs whose single-MBB liveness puts them in a bucket of
size 1 within that MBB."  Multi-candidate MBBs (the parallel-XOR
shape in aes_mixColumns / aes_subBytes that crashed 73o) now get no
pinning at all.

**Segfault on `05_Oz_static_stack` is gone.**  Empirically the
remaining pin-on AES numbers are still net-negative — small but real
regressions even on single-candidate MBBs because pinning a proxy
forces a materializing `LD A, r` that greedy was previously eliding
via a non-A allocation.  Default stays OFF.

## Pin=on vs pin=off (AES corpus, 73p HEAD)

| Config              | bin off | bin on | Δbin | ts off    | ts on     | Δts    | verify |
|---------------------|--------:|-------:|-----:|----------:|----------:|-------:|:------:|
| 01_baseline_Oz      |   4109  |  4170  |  +61 | 15049927  | 15078609  | +0.19% | PASS   |
| 05_Oz_static_stack  |   2830  |  2911  |  +81 | 14604468  | 14656828  | +0.36% | PASS   |
| 09_Oz_prod_like     |   2667  |  2691  |  +24 | 14887472  | 14890032  | +0.02% | PASS   |

Compare to 73o numbers (over-aggressive scope): 01_baseline +261 B /
+3.8% ts, 05_static_stack **segfault**.  73p is strictly safer: no
crash, ~4× smaller worst-case regression.  But still wrong direction.

## Why the regression persists

Single-candidate MBBs still regress because pinning a proxy vreg
constrains its allocated register class to AReg, which forces a
materializing `LD A, source_reg` upstream when the source vreg
wasn't itself in A.  Before pinning, greedy could route the proxy's
allocation to share the source's physreg, eliding the COPY entirely.
Pinning trades the savings of "no LD A" against the savings of "no
shuttle round-trip" — and for short proxies the round-trip wasn't
that bad to begin with, while the materializing LD is now mandatory.

The real win requires pinning the **loop carrier** (the PHI cycle) so
that the carrier value lives in A across iterations and the round-trip
disappears.  Pinning short-lived proxies in isolation doesn't deliver
that.

## Path forward (still parked, but better-scoped)

A working version needs all three:

1. **MachineLoopInfo** to find natural loops + their headers/latches.
2. **LiveIntervals** to verify the carrier candidate's full PHI cycle
   has no interfering A-using ranges.
3. **PHI walk** to identify the entire cycle (header PHI + latch
   COPYs); pin the whole chain at once or not at all.  Pinning only
   the proxies is what 73p does and isn't enough.

Estimated ~200-400 lines of new code; not for a follow-on session
without a clearer cost/benefit estimate.  The AES residual gap from
the A-shuttle is ~5pp of the 19% clang-vs-SDCC ts gap; other levers
(#166 HL remat, #169 LSR miscompile root-cause, #170/#171) may be
higher-yield per session-hour.

## Files touched (single commit)

  - `llvm/lib/Target/Z80/Z80PinAluAccumulator.cpp`: scope tightening
    (single-candidate-per-MBB invariant + uniqueRefMBB helper +
    DenseMap/SmallVector bucketing).

No test changes — there isn't a clean lit shape that exercises
single-candidate-per-MBB without involving the multi-candidate path
that the rule deliberately ignores.  AES corpus 13/13 PASS at HEAD
with default-off; pin-on subset (01/05/09) PASS at HEAD with the new
scope (segfault gone).

## What was Easy / Hard

**Easy**: switching from per-vreg to per-MBB bucketing.  ~30 lines of
code: collect candidates into `DenseMap<MBB*, SmallVector<Register>>`,
pin only when bucket size == 1.  Builds clean, fixes segfault.

**Easy**: confirming the segfault is gone.  Re-ran the same three
configs from 73o; 05_Oz_static_stack now PASSes with pin on.

**Medium**: realizing the conservative scope still regresses on the
single-candidate MBBs.  Took the targeted A/B sweep on 01/05/09 to
see: even the safe cases give +24 to +81 B, not the +0 expected for
"only pin when free."  The free-pin assumption was wrong.

**Honest**: parking pin-A here.  Both attempts (73o aggressive, 73p
conservative) showed the AReg constraint costs upstream materializing
LDs that the proxy approach doesn't recover.  Loop-carrier pinning
(the real fix) needs LiveIntervals + PHI-walk; not a follow-on-session
investment without a higher-confidence yield estimate.

## Difficulty: Easy

The change itself is small.  The honest result — "this approach won't
deliver, here's why, here's what would" — is the artifact worth
preserving.
