# Plan: regalloc cost-model cluster for `aes_mc_inv` (#115 + #27)

Date: 2026-05-15.  Multi-session investigation kicked off in session 73
after the `aes_mc_inv` reconnaissance (see
`aes-mc-inv-recon-session73.md`).

## Scope & ROI ceiling

- Gap on `aes_mc_inv` at production config: **+221 B** (clang 535 vs
  SDCC 314).  Decomposition: ~142 B from 16-bit pointer spills clang
  has and SDCC doesn't, ~76 B from extra reg-to-reg copies (mostly
  A-shuttling), ~16 B from pair-split 16-bit copies.
- AES corpus bin total is already **clang +909 B** ahead of SDCC at
  production config — this is polish, not a critical path.
- Realistic ceiling: ~60-90 B reachable via #115's class lever on
  `aes_mc_inv`; rest needs #15 (rematerialization) or different
  allocator (out of scope).
- Every stage must have a "stop if N B not reached" exit.

## LLVM surfaces and what each can move

| Surface | Hook / file | Can it close the 16-bit-spill gap? | Failure mode if guessed |
|---|---|---|---|
| `getRegAllocationHints` | `Z80RegisterInfo.cpp:1818` | **Soft.** Greedy's copy-elimination heuristic overrides target hints when a cheaper local copy exists (per #110/#115).  Won't reliably keep P in HL across a CALL/loop. | Silent: hint ignored, no `.s` diff. |
| Single-register class (`HLReg`, `DEReg`) | `Z80RegisterInfo.td` + new pre-RA pass mirroring `Z80SplitDjnzCounters.cpp` | **Hard constraint** — bypasses greedy heuristics.  This is what #94/#99 used.  Bounded to ~3 active "single-class pointers" before classes collide. | If applied too broadly: forces spills *worse* than greedy.  Need live-range scope check before applying. |
| `CostPerUse` per-physreg | `Z80RegisterInfo.td:118-122` | Only affects per-use byte cost (DD/FD prefix).  Cannot model "BC↔HL copy cheaper than IX↔HL". | Moves nothing 16-bit-spill-wise. |
| Class-level `CopyCost` | `Z80RegisterInfo.td:232` (IR16 has CopyCost=3) | One number per class — cannot model EX-DE-HL vs PUSH/POP vs LD-pair.  Exactly the #27 limitation. | If raised: allocator avoids the class, spills more.  If lowered: more IX/IY use. |
| `AllocationPriority` on class | `Z80RegisterInfo.td` | Affects which class greedy considers first; won't move pointer-stays-in-pair decisions. | Mostly invisible. |
| Tied operands / SubReg hints | `Z80InstrInfo.cpp` + `.td` | **Strong** — produces COPY-eliminable two-address forms.  Could collapse some pair-split copies. | New ties wrong → ExpandPseudo SP-balance crash or wrong-class assert. |
| `getRegClassWeight` / spill weight | not overridden | Could discourage spilling 16-bit values *as a class*.  Greedy already weights live-range length × use count. | Risk regressing non-AES functions. |
| Post-RA fix-up | `Z80LateOptimization.cpp` | **Explicitly out of scope.**  MEMORY.md forbids papering over allocator decisions for #115/#27. | — |

**Headline:** the only mechanism with a demonstrated track record on
this backend for moving allocator decisions of this kind is
**single-register-class via a pre-RA pass** (`Z80SplitDjnzCounters`
is the working precedent).  Hints alone won't close the spill gap.

## Staged plan

| Stage | Goal (falsifiable) | Measurement | Negative result → |
|---|---|---|---|
| **S1** instrumentation | Hook is reached, can identify pointer vregs in aes_mc_inv | `-debug-only=z80-regalloc-hint` log line; rebuild; run `llc aes.ll`; grep for entries per pointer vreg.  No `.s` diff expected. | If not reached: pass ordering wrong, fix before S2. |
| **S2** per-pair copy cost via hints | Reduce IX/IY landing on 16-bit pointer vregs | Count `PUSH IX/IY; POP rr` + `ADD HL,IX` peephole hits in aes_mc_inv `.s` + corpus | If greedy ignores hints: document, proceed to S3. |
| **S3** pointer-keep-in-pair pre-RA pass | Mirror `Z80SplitDjnzCounters` for LDIR/`(HL)`/`CP (HL)`/indirect-load shape; new `HLReg`/`DEReg` single-class scaffold | Count 16-bit BSS spills in aes_mc_inv; target ≥40 B reduction.  lit green; test-runner 685/42/56/207 unchanged. | If spills move HL→DE-vreg (whack-a-mole): try paired application. |
| **S4** pair-split → EX DE,HL | Pre-RA — never post-RA peephole | Pair-split copies in aes_mc_inv (recon: 8, 16 B).  Target ≥6 B. | If liveness analysis fragile: park. |
| **S5** A-shuttling | Only if S3+S4 leave >40 B.  Hint A for binop results. | A-copy count (recon: 70% of 90 copies) | High risk of regressing non-AES; gate strictly. |

## First-session concrete step (S1)

A landable no-op that proves wiring:

1. In `Z80RegisterInfo.cpp:1818-1856` at the top of
   `getRegAllocationHints`, add a single `LLVM_DEBUG(dbgs() << ...)`
   line logging `VirtReg`, its RegClass, and the set of use opcodes
   (especially `Z80::LDIR`, `Z80::LD8rp`, `Z80::CP_HL_`, `G_LOAD` /
   `G_STORE`).  No behavioral change.
2. Add one lit test `llvm/test/CodeGen/Z80/regalloc-hint-aes-shape.ll`
   with a stripped-down kernel resembling `aes_mc_inv`'s inner block
   (two 16-bit pointer vregs both used as `(HL)`-source).  Run with
   `-debug-only=z80-regalloc-hint`.  FileCheck on the log lines, NOT
   the `.s`.
3. Verify the log fires for both pointer vregs.  **No size
   measurement.**  Pure wiring verification.

This shakes out, in one session, the otherwise invisible failure
modes: (a) vregs coalesced earlier than expected; (b) use site
thought to be `LDIR` is actually a `COPY` to a tied operand; (c) RC
at hint time broader than expected.  Until those are nailed down,
S2/S3 design rests on guesses.

## The SDCC-vs-greedy boundary

SDCC's iCode allocator: **whole-iCode-tree liveness within a BB**,
then a heuristic for crossing BBs.  Assigns physregs by walking
iCode in order, grabbing the cheapest free pair, immediately.  No
"global live range" — values can change physreg between BBs cheaply
because iCode emits the COPY without a coalescer that fights it.
That's why SDCC keeps the AES pointer in HL across one BB, in DE
across the next, never spills.

LLVM's greedy: operates on global LiveIntervals.  If a value's live
range spans a CALL, all general-class regs are call-clobbered → spill.
SDCC sees "alive within a single iCode segment after the call
returns and before the next; re-load the constant pointer".

Two structural differences:

- **SDCC re-materializes pointer constants** instead of preserving
  live ranges.  LLVM has rematerialization, but constant pointers
  from `G_GLOBAL_VALUE + offset` are not always marked
  `isRematerializable` for the allocator.  This is reachable without
  rewriting regalloc — see issue #15 (parked) for the same pattern in
  cpnos-rom.
- **SDCC's allocator naturally does live-range splitting on every BB
  boundary.**  LLVM greedy does split, but the split cost is
  `CopyCost`-class-driven; with three GR16 pairs and call-clobbered
  DE/HL/BC, the split decision often falls back to a memory slot.

**Where the boundary is:** S3 (single-class pointer pass) can recover
most of the *within-BB* SDCC behavior — vregs whose live range
happens to be local to one BB but greedy spills anyway because of
pressure.  Cross-call live ranges are **not reachable** by hint/class
work and would require either (a) marking `G_PTRTOINT(GV+const)`
chains as truly rematerializable (issue #15's territory), or (b)
replacing greedy with a BB-local allocator on a feature flag (a
rewrite, not a fix).

**Estimate:** of the 142 B 16-bit-spill ceiling, perhaps 60-90 B is
reachable via #115's class lever; the rest is structurally outside
greedy without remat changes (#15) or a different allocator.

## Decision points where to pause and ask

| After | Pause if | Why |
|---|---|---|
| **S1** | Hook fires but RC is `Anyi16`/`Ptr16` not `GR16` at hint time | Hints issued too early relative to RB selection; shift to post-RB pass.  Re-plan. |
| **S2** | 0 B change but lit/test-runner regress | A hint that looks soft is actually changing tie-breaking; quantify regression scope before continuing. |
| **S3 design** | New pass touches >~20 lines beyond `Z80SplitDjnzCounters` template | Shape wrong; pause — likely missed existing constraint. |
| **S3 result** | <30 B saved but corpus regresses >50 B | Negative ROI; park strand, revisit after #15 lands. |
| **S4** | Any `EX DE,HL` regresses 4-cell AES verifier | EX modifies both — pause and re-check liveness, do not commit.  CLAUDE.md "Lessons Learned" lists this exact failure mode. |
| **Any stage** | test-runner 685/42/56/207 baseline shifts | Stop; canary, regardless of size win. |

## Risks to track

- **#136 (corpus noise) interaction.**  Any hint change can move
  register pairings across the full corpus, not just AES.  Run a
  corpus diff at every stage.
- **4-cell AES verifier.**  Single oracle; any reordering of `add hl,rr`
  operands risks subtle wrong-result-but-still-runs bugs.  Always run.
- **IY un-reservation interaction.**  #115 lists itself as a gate for
  #38.  Don't accidentally satisfy #115 in a way that also
  un-reserves IY — should land separately to bisect regressions.
- **GR16 ordering is settled.**  Do not touch `Z80RegisterInfo.td:192`.
- **`+static-stack` only.**  Spill costs differ wildly with hasFP=true.

## Critical files

- `llvm/lib/Target/Z80/Z80RegisterInfo.cpp` (lines 1818-1985 —
  `getRegAllocationHints`)
- `llvm/lib/Target/Z80/Z80RegisterInfo.td` (lines 172-245 — class
  definitions; add `HLReg`/`DEReg` mirrors of `BReg`/`BCReg`)
- `llvm/lib/Target/Z80/Z80SplitDjnzCounters.cpp` (template for S3
  single-class pre-RA pass)
- `llvm/lib/Target/Z80/Z80TargetMachine.cpp` (pass pipeline
  registration for the new pre-RA pointer-split pass)
- `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` (lines 1370-5450
  — sites currently using `GR16_BCDERegClass` constraints; new
  `HLReg`/`DEReg` constraints attach here for LDIR/`(HL)` operands)

## Status

- **Planned**: session 73 (2026-05-15).
- **S1**: landed in `0dd9e4e24daf` — `-z80-log-regalloc-hints` cl::opt
  + lit test.  Smoke test confirmed: pointer vregs land in class
  `GR16` at hint time (NOT `Anyi16`/`Ptr16`); first decision point in
  the plan NOT triggered.
- **S2 (session 73)**: attempted, **negative result**.  Soft
  `Hints.insert(begin, HL)` for vregs whose uses are
  `LOAD8_IND`/`STORE8_IND` fires 21 times in `aes_mc_inv` alone,
  produces ZERO byte change on the AES corpus, test-runner unchanged.
  Per plan's S2 decision point ("greedy ignores hints as #110/#115
  predict: proceed to S3, document"), confirms hint-flavored work
  cannot move bytes for #27 / `aes_mc_inv` on this backend.  Code
  reverted; comment retained in `Z80RegisterInfo.cpp` near
  `done_16bit_hints:` so future contributors don't retry the same
  path.
- **S3 (session 73, deferred)**: shape-mismatch reconnaissance found
  S3-as-written (single-register class `HLReg` pre-RA pass mirroring
  `Z80SplitDjnzCounters`) **will not move bytes on aes_mc_inv**.  See
  `tasks/aes-mc-inv-s3-shape-mismatch.md`.  Plan decision point #3
  ("Shape wrong; pause — likely missed existing constraint") triggered.

  Empirical: post-greedy MIR shows 4 i16 pointer vregs (`%205`, `%194`,
  `%188`, `%179`) materialized via clean `INC16` chain, all in `$hl`
  across their LOAD8_IND uses.  Greedy *already* keeps the load-side
  pointer in HL via the existing hint.  Spills happen because all 4
  must SURVIVE to the matching STORE8_IND later in the iteration
  body; the intervening XOR chain uses the remaining pairs.  Pinning
  one of the 4 to HLReg cannot prevent the other 3 from spilling —
  greedy's hint already wins HL for whichever one HLReg would pick.
  Reclassified to S3'.

- **S3' (next session)**: pointer rematerialization for
  `G_PTR_ADD(base, const)` + `INC16` chains.  Same target byte range
  (~60-90 B on aes_mc_inv) via a different mechanism (`isReally
  TriviallyReMaterializable` audit for INC16 / LD16-from-BSS).  Maps
  onto open issue #15 broadened beyond IX constants.  Plan steps in
  `aes-mc-inv-s3-shape-mismatch.md` §Recommendation.

- **S4-S5**: future sessions; decision points above.

### S2 empirical breakdown

| Step | Result |
|---|---|
| Build clang+llc with HL hint | OK |
| AES corpus sweep (13 configs) | byte-identical to post-#165 |
| `aes_mc_inv` (09_Oz_prod_like) | 535 → 535 B (no change) |
| `aes_mc_inv` (05_Oz_static_stack) | 548 → 548 B (no change) |
| `.s` diff pre/post S2 on aes256.c | 0 lines (byte-identical) |
| Hint fired in aes_mc_inv | 21 times for LOAD8_IND/STORE8_IND uses |
| z80-utils test-runner | 685/42/56/207 (unchanged) |

**Interpretation**: greedy's copy-elim heuristic dominates target
hints when the live range crosses any cheaper local copy.  This is
the structural finding that motivated #115's design.  S3 is the
required escalation.
