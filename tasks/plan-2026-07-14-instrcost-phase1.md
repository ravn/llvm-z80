# #240 Phase 1 — RESOLVED: delete dead `Z80InstrCost`

**Date:** 2026-07-14. **Outcome:** the dead struct was DELETED, not extended.

## What #240's comment proposed vs. what the tree actually holds

The #240 comment sketched "unify `Z80InstrCost` into one queryable
`costOf(MI) -> {bytes, cycles}` model". Investigating to plan that revealed
two material facts that changed the decision:

1. **`Z80InstrCost` (`Z80InstrCost.{h,cpp}`) was dead** — zero consumers; only
   its own `.cpp` compiled it in (CMakeLists). No `#include` anywhere else.
2. **A live, wired cost model already exists** from ravn/llvm-z80 #23
   (2026-06-08): `Z80InstrInfo::getRematCost()` / `getSpillCost()` +
   `-z80-use-tiered-cost-model` (default ON), producing real size wins. It uses
   plain `unsigned` byte costs, NOT the `{bytes,cycles}` struct. It has its own
   6-phase plan (`plan-z80-cost-model-refinement-2026-06-08.md`).

So `Z80InstrCost` was a superseded parallel path. Building its cycles half
(hand-transcribing all of `Z80SchedData.td`'s T-state table into a
`getInstCyclesEstimate` switch over hundreds of opcodes) would have been a
large, latent, unproven addition to dead code — and per #177/#184 an accurate
per-opcode cycle scalar rarely changes Z80 codegen anyway (the dominant real
cost is post-regalloc register pressure).

## Decision (user, 2026-07-14)

Delete `Z80InstrCost` entirely; consolidate future cost work on #23's live
`unsigned` model.

## What was done

- Corrected the #240 comment's claim in passing: the alleged "public ctor *256
  vs operators *1 magnitude-mixing defect" is FALSE — the public ctor is the
  only scaling entry (*256), every operator takes Multiplier=1 on already-scaled
  members, so scale is uniform; `*256` is deliberate 8.8 fixed-point that lets
  `operator/` average without truncating. (Moot now the file is gone, but worth
  recording so the claim isn't re-raised.)
- `git rm` `Z80InstrCost.cpp` + `.h`; removed the CMakeLists entry.
- Rebuilt `llc` clean; Z80 lit **200 PASS + 5 XFAIL** (baseline, no regression).

## Not done (deliberately)
- No `getInstCyclesEstimate`, no `costOf`. If a cycles-aware cost is ever needed,
  add it to the live #23 model, driven by a real consumer + red-green per site
  (that model's Phases 2-5), not by resurrecting a struct.

## `Z80SchedData.td` note
Still present, still inert (22 `Z80WriteN` classes, 0 `InstRW`), with the rich
mnemonic->T-state reference table. Left as documentation. Its fate (delete vs
keep as reference) is unchanged by this work.
