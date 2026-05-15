# Session 73b — S3' (INC16/DEC16 remat) production-target impact

Date: 2026-05-15.  Continuation of session 73b after the AES corpus
win was committed in `006ba9607dd1`.  This document records the
production-target A/B for the same compiler change.

## TL;DR

**S3' is workload-limited.**  It wins on AES-corpus shapes (8 of 13
configs improved, −25 to −118 B) but has **zero effect** on production
targets (autoload PROM, cpnos-rom resident, rcbios BIOS).  The
optimization only fires where MachineLICM / MachineCSE have unified
pointer-chain expressions across a loop; production code at `-Oz` runs
with LICM/CSE active on AES but doesn't actually produce the same
chain shape in autoload/cpnos/rcbios.

| Target | Pre-S3' | Post-S3' | Δ | Note |
|---|---:|---:|---:|---|
| autoload PROM (clang) | 1859 B | **1859 B** | 0 | unchanged |
| cpnos-rom resident (PIO transport, .payload non-padding) | 2003 B | **2003 B** | 0 | unchanged |
| rcbios-in-c BIOS | 5925 B | **5925 B** | 0 | unchanged |
| AES corpus 01_baseline_Oz | 4205 B | **4111 B** | −94 | win |
| AES corpus 04_O2 | 8529 B | **8411 B** | −118 | win |
| AES corpus 09_Oz_prod_like | 2695 B | **2695 B** | 0 | LICM/CSE disabled — no chain to remat |
| Z80 lit suite (104+2) | green | green | — | no regression |
| z80-utils test-runner | 685/42/56/207 | 685/42/56/207 | — | identical |

## Why production targets don't move

`aes_mc_inv` is unusual: an inner block computes `buf[i+0..3]` four
times, with rj_xtime inlined eight times between reads and writes.
That creates 4 long-lived i16 pointer vregs in a single MBB, all
derived from `buf+i` via INC16.  Greedy spills all four; post-S3'
the three derived pointers rematerialize from the still-live base.

Autoload, cpnos, BIOS code shapes:
- Single pointer per loop (e.g., LDIR/LDDR memcpy) → already optimal
- Short-lived i16 vregs in straight-line code → don't survive long
  enough for spill pressure
- Indirect-jump table dispatch (BIOS) → mostly i8 work
- No "4 simultaneously-live derived pointers" pattern observed

The empirical signature was visible in the AES sweep itself: configs
06, 09, 10, 11, 13 (all with `-mllvm -disable-machine-licm -mllvm
-disable-machine-cse`) were inert.  Those flags prevent the cross-
iteration unification that creates the chain.  Production targets are
written in a style that doesn't lean on that unification.

## Stale CLAUDE.md size numbers (not S3'-related)

A/B confirmed S3' is byte-neutral, but production sizes drifted vs
the documented values for unrelated reasons:

| Target | Documented | Measured | Δ vs doc |
|---|---:|---:|---:|
| autoload PROM | 1756 B | 1859 B | +103 |
| cpnos-rom resident | 1928 B | 2003 B | +75 |
| rcbios BIOS | 5961 B | 5925 B | −36 |

CLAUDE.md updated to reflect post-S3'-equivalent reality.

## Lit suite + value oracles

- llvm/test/CodeGen/Z80/: 104 PASS + 2 XFAIL (unchanged)
- AES verifier (4 cells): all PASS (unchanged from pre-S3')
- z80-utils test-runner: 685/42/56/207 (identical to pre-S3')

## Issues filed

- (none required — S3' is closed.  Follow-up filed as `ravn/llvm-z80#`
  for ADD_HL_rr/LEA_FI remat investigation; see below.)

## Follow-up scope for extending remat

`isReMaterializable + isAsCheapAsAMove` worked cleanly on INC16/DEC16
because Z80 INC/DEC rr (16-bit) does not modify flags and is 1 byte.
Other 16-bit pseudos that *might* be similarly safe:

| Pseudo | Real op | Cost | Side effect | Notes |
|---|---|---|---|---|
| `ADD_HL_rr` | `ADD HL,rr` | 1B | Flags (C, H, N) | Used widely; flag clobber may matter |
| `LEA_IX_FI` | `LD IX,nn + ADD IX,SP` | 4-7B | Flags from ADD | Too expensive for routine remat |
| `LD_HL_a16` | `LD HL,(nn)` | 3B | None | Already remat for fixed args via #15 |
| `EX_DE_HL` | `EX DE,HL` | 1B | None — but destroys both | Not a remat candidate (clobbers source) |

ADD_HL_rr is the most interesting candidate.  The flag clobber rules
it out at sites with live flags, but the regalloc/remat machinery
already handles "remat unavailable if destroyed regs are live".

This is **not** a confirmed win — `aes_mc_inv` doesn't have a
post-S3' chain rooted at ADD_HL_rr in the spill-dominant blocks (the
chain root is COPY-from-physreg, not ADD_HL_rr).  Filed as a
speculative follow-up issue.

## Production target re-baseline

S3' deliverable was **AES-corpus only**.  Production targets benefit
indirectly via the lit/test-runner regression check, but no net
size change.  Treat S3' as "polishes a specific shape that
production code doesn't exercise", not as "saves bytes everywhere".

If we want to move production sizes, the levers are still:
- #15 (rematerialization of GV-derived constants across CALLs)
- #110/#115 (regalloc copy-elim overriding hints)
- #27 (per-pair copy cost model)
- #89 (cross-MBB regalloc spills)

Of these, **#15** is the most aligned with the S3' lever direction
and would likely show up on production code.

## Cumulative AES corpus position (post-S3')

| Config | post-#165 | post-S3' | Δ |
|---|---:|---:|---:|
| 01_baseline_Oz | 4205 | 4111 | −94 |
| 02_Os | 4480 | 4417 | −63 |
| 03_O3 | 12559 | 12472 | −87 |
| 04_O2 | 8529 | 8411 | −118 |
| 05_Oz_static_stack | 2855 | 2830 | −25 |
| 06_Oz_no_licm_cse | 3815 | 3815 | 0 |
| 07_Oz_no_lsr | 4571 | 4477 | −94 |
| 08_Oz_gc_sections | 4185 | 4091 | −94 |
| **09_Oz_prod_like** | **2695** | **2695** | 0 |
| 10_Oz_no_licm_cse_lsr | 4171 | 4171 | 0 |
| 11_Oz_no_licm_cse_gc | 3795 | 3795 | 0 |
| 12_Oz_no_omit_fp | 3606 | 3568 | −38 |
| 13_Oz_no_omit_fp_no_licm_cse_gc | 3328 | 3328 | 0 |

`09_Oz_prod_like` remains the canonical production knob at 2695 B
(zsdcc baseline 3604 B, clang ahead by 909 B).
