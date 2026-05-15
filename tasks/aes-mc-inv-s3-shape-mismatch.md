# aes_mc_inv: why S3 single-register class is the wrong lever

Date: 2026-05-15.  S3 design-point reconnaissance from
`plan-115-27-regalloc-cluster.md`.

## Plan decision point firing

> **S3 design** — Pause if new pass touches >~20 lines beyond
> `Z80SplitDjnzCounters` template.  Shape wrong; pause — likely missed
> existing constraint.

Triggered.  The shape of `aes_mc_inv` post-greedy does not match the
DJNZ-counter shape that S3 was designed to mirror.

## Empirical MIR shape (post-greedy, -Oz aes256.c)

- 22 MBBs, ~10 instrs/block (loop body + branches from inlined
  `rj_xtime` carry tests; no CALL sites)
- 7 i16 spill slots, 6 i8 spill slots
- Pointer chain in `bb.2`: `%205 -> %194 -> %188 -> %179` via INC16,
  each is a fresh SSA vreg derived from the previous.  Greedy already
  uses HL across these — they live in `$hl` from the moment of
  computation through the matching `LOAD8_IND`.
- **All four are then immediately SPILL_GR16'd** because they must
  survive to the matching STORE8_IND later in the same iteration, and
  the intervening XOR/RLCA chain uses the remaining register pairs.

## Why pinning to HLReg won't help

The DJNZ S3 template assumes:
- One counter vreg per loop
- Counter dominates a self-back-edge MBB
- Constraining it to BReg bypasses copy-elim because the *only*
  candidate placement is B

`aes_mc_inv` has:
- 4 i16 pointer vregs simultaneously live across the XOR chain of
  each iteration body
- HL can hold one — the other three would still go to DE/BC/IX/IY or
  spill
- Forcing HLReg on one vreg just relocates the spill, doesn't remove it
- The plan already predicted this: "Bounded to ~3 active single-class
  pointers before classes collide"

Empirical evidence from MIR: greedy is already keeping `%205` in HL
across all 4 LOAD8_INDs.  The HL hint **already works** for the load
side.  HL pinning would change nothing on the load side and add no
holding power for the store-side reuse.

## What's actually missing: pointer rematerialization

Each spill of `%205` / `%194` / `%188` / `%179` is a SPILL_GR16 (3-4B
store) + a later RELOAD_GR16 (3-4B load) = 6-8B per slot × 4 slots =
24-32B per iteration × 4 iterations of the outer loop.  But all four
pointers are `buf+i+0`, `buf+i+1`, `buf+i+2`, `buf+i+3` — fully
rematerializable from a live `(buf, i)` pair via `ADD HL,DE` + 0-3
INC16 ops.

The rematerialization cost:
- `LD HL,(buf_slot)` + `ADD HL,DE` (where DE holds i) = 5B
- Or just INC16/INC16/INC16 from a live %205 in HL = 1B each

Either is competitive with or cheaper than spill+reload.  The
allocator currently doesn't see these as remat candidates because:
1. `G_PTR_ADD(global, vreg)` chains aren't marked
   `isRematerializable` in `Z80InstrInfo::isReallyTriviallyReMaterializable`
2. INC16 has implicit-def-flags side effect (clobbers flags), which
   `isAsCheapAsAMove` defaults to false on
3. The remat path in `RegAllocBase` only fires for instructions whose
   def the allocator already considered worth rematerializing

This maps onto open issue **#15** ("Rematerializable constants held in
IX") — same root cause, broader scope.  #15 was scoped to constants
loaded into IX; the aes_mc_inv finding extends it to `base + small
const_offset` pointer chains.

## Recommendation: replan S3 around remat, not single-class

Stop S3 single-class scaffold work.  Replace with:

**S3' — `G_PTR_ADD` rematerialization**

| Step | Action | Test |
|---|---|---|
| 1 | Audit `Z80InstrInfo::isReallyTriviallyReMaterializable` for `INC16`, `DEC16`, `LD_HL_a16` (load BSS), `LD_rr_nn` (load immediate) | None yet |
| 2 | If INC16 not marked, mark it; verify lit suite stays green | lit |
| 3 | Add a small remat-friendliness lit test that exercises the `%205 -> %194 -> %188 -> %179` chain shape | new lit |
| 4 | Measure aes_mc_inv | AES corpus sweep |
| 5 | Stop if ≤30 B saved | — |

If step 4 shows progress, expand to BSS-load remat (the `LD HL,(buf_slot)`
preceding the chain).

## Open question: GISel-level fix?

An alternative path: in `Z80InstructionSelector` or
`Z80LegalizerInfo`, fold short INC16 chains into a single
`G_PTR_ADD(base, const)` so the allocator sees fewer derived vregs.
Less elegant — the chain is already SSA — and InstCombine should have
canonicalised this earlier, so a missed-canonicalization fix in
TruncInstCombine peers might be cleaner.  Punt this until after
isRematerializable audit.

## What the AES corpus loses by parking S3

Per the plan's ceiling estimate, single-class S3 had ~60-90 B
reachable on `aes_mc_inv`.  Remat (S3') has the same target range
with a different mechanism, plus generalizes to all other AES
functions (`aes_mixColumns` +91 B, `rj_sb_inv` +91 B — same shape).

No bytes lost by parking S3-as-written.

## Files

- This recon: `tasks/aes-mc-inv-s3-shape-mismatch.md`
- Plan updated: `tasks/plan-115-27-regalloc-cluster.md` (S3 status: PARKED, S3' added)
- MIR snapshot consulted: `/tmp/aes_mc_inv.full.mir` (local, not committed)
