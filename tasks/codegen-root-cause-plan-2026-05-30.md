# Z80 codegen issues — root-cause analysis + plan (2026-05-30)

Step-back synthesis of all still-open **codegen** issues (#172 #173 #20 #175 #166
#178 #100 #203 #27 #110 #111 #115 #96 #180 #146 #117 #206 #50), grounding each in
a shared structural cause and sequencing the fixes by leverage × tractability ×
risk.  Evidence gathered by reading the backend at HEAD `5c174bc6410a`
(clang binary `a24ad81`).  This supersedes the framing in
`unpark-2026-05-22.md` for the codegen subset.

## The one-sentence root cause

**Z80 expresses its most common operations as reads/writes of *fixed physical
registers* (A is the only ALU accumulator; HL is the only 16-bit address-math
accumulator; FLAGS is clobbered by every arithmetic op; B is the only DJNZ
counter), and these ops have *no virtual-register-output form* — so LLVM's
SSA-based rematerializer, its spill cost model, and its hint-based greedy
allocator are all blind to what the Z80 actually needs, and the backend is
forced to re-impose those decisions *after* register allocation, fragilely.**

Two reinforcing structural facts drive everything below:

* **F1 — physreg-result instructions.**  `ADD_HL_rr` (`Z80InstrInfo.td:1351`)
  has `OutOperandList=(outs)`, `Uses=[HL]`, `Defs=[HL,FLAGS]`.  The 8-bit ALU
  pseudos (`ADD_A_r`/`XOR_r`/… `:1459-1499`) have `Uses=[A]`, `Defs=[A,FLAGS]`
  and deliver their result into A, which ISel then `COPY`s into an
  *unconstrained* GR8 vreg.  No value carries a "lives in A/HL" type.
* **F2 — hint-deaf greedy allocator.**  `getRegAllocationHints`
  (`Z80RegisterInfo.cpp:2037`) soft-hints DE/BC/B; the inline note at `:2098`
  records that an HL/A soft hint produced **zero** byte change on AES (21 hints
  fire in `aes_mc_inv`, all overridden by greedy's copy-elimination).  HardHints
  don't help (`AllocationOrder` treats them as a soft cap, #110).  The only
  thing that moves greedy is a **single-register class** (`AReg`/`BReg`/`BCReg`/
  `GR16NoIR`, `Z80RegisterInfo.td:179-221`) — allocation by *exclusion*, not by
  *cost*.

## Four issue clusters, one cause

### Cluster 1 — the A-accumulator bottleneck  (#172, #173, #20, 8-bit #175)
*Cause:* F1 (ALU result → unconstrained vreg) + F2 + the hardware fact that the
**only** 8-bit absolute store is `LD (nn),A` (`Z80InstrInfo.td:41`; there is no
`LD (nn),r`).
* #172 — every ALU op in a chain does `ld a,r; <alu> s; ld r,a` because the
  carrier vreg isn't pinned to A; greedy scatters it to D/E/H/L.
* #173 — an 8-bit BSS spill must route through A: `push af; ld a,r; ld (nn),a;
  pop af` = 6 B (`Z80RegisterInfo.cpp:1470-1505`).  SDCC: `push rr`/`pop rr` = 2 B.
* #20 — multi-value lifetimes across a CALL → multiple A-routed BSS round-trips.
* #175 (8-bit half) — `(HL)`-operand ALU fusion exists only for OR/XOR
  (`Z80InstructionSelector.cpp:3544`); AND/ADD/SUB/ADC/SBC/CP still load to a reg
  first, and the `(IX+d)` ALU defs (added since the issue; `Z80InstrInfo.td:378-463`)
  are essentially never *selected* from C.

### Cluster 2 — HL-pinned remat + spill traffic  (#166, #178, #100, #203, #96)
*Cause:* F1 (HL/FLAGS physreg defs) → the generic rematerializer
(`TargetInstrInfo.cpp:1656-1666`) returns false on any non-constant physreg def
**or** use, so `let isReMaterializable=1` on `ADD_HL_rr` is a **dead flag**
(byte-identical A/B test, td comment `:1343-1350`).
* #178 — the systemic statement of the above.  `INC16`/`DEC16` *do* remat
  (clean SSA `$dst`, no FLAGS — `:1315-1328`); `ADD16_acc` (the SSA-shaped
  rescue, `:1388`) still **doesn't**, because its mandatory `Defs=[FLAGS]` trips
  the same gate *and* pinning every result to HL costs +219 B / +9.8 % under the
  3-pair (IX/IY-reserved) allocator — so it's default-OFF (`z80-add16-acc`).
* #166 — concrete symptom: an `ADD HL,rr` pointer spilled across a CALL is
  reloaded from the slot, never recomputed.
* #100 — loop-carried value across a CALL is *structurally ineligible* for the
  cheap PUSH/POP form (`z80SlotReadBeforeStoreInBlock` must bail, else stale slot).
* #203 — because PUSH/POP-spilling is a **post-RA peephole** (two copies:
  single-block `Z80LateOptimization.cpp:~4380` and cross-block `:~4992`) rather
  than a first-class spill mode, every liveness safety condition is re-derived by
  hand *twice* and drifts (#14/#192/#193/#195/#198/#202 — #202 was a silent
  miscompile from a guard present in one copy, missing in the other).

### Cluster 3 — the keystone: allocation-by-exclusion  (#27, #110, #111, #115)
*Cause:* F2 directly.  This cluster is the **amplifier** for Clusters 1 & 2 —
both the A-pin (#172) and the HL-pin (#166/ADD16_acc) can only be done by
exclusion-class today, and both regress when done naively, *because* there is no
per-pair cost model to weigh the pin against its materialization cost.
* #27 — `CopyCost` is per-class; real Z80 copy cost is 1 B (`EX DE,HL`) … 4 B
  (IX/IY undoc `LD`).  Backend can only forbid or discourage, never weigh.
* #110 — greedy's local model prefers eliminating a COPY (0 B) over honoring a
  hint (1 B `LD B,X`); the downstream payoff (DJNZ, freeing HL) is invisible.
  HardHints + `setRegAllocationHint` both confirmed not to move it.
* #115 — the *legality* half (byte-decompose IY-leak) is **already fixed**
  (`getLargestLegalSuperClass` GR16NoIR gate `:347/370`, `Z80NarrowNoIndex` pass,
  session-73ab); residual is Class-C `push/pop iy` *density* — a cost-model
  tradeoff, not a bug.
* #111 — `HLReg`/`DEReg` pointer-pinning classes proposed-but-unbuilt; belongs
  with the cost-model work, not as another N-of-1 class.

### Cluster 4 — peephole accretion  (#180, #146, #117, #206, #50)
*Cause:* partly a *symptom* of Clusters 1-3 (generic pipeline emits non-Z80 MIR,
backend patches post-RA), partly genuinely-missing ISel/combiner patterns.
* #180 — 16 of 38 late-opt peepholes (~2300 LOC, 37 %) are stand-ins for missing
  GISel combiners / ISel patterns / MIR-CSE-DCE; migration gated on #177/#178/#179.
* #146 — EX (SP),HL epilog rewrite: **not implemented**.  Small, self-contained.
* #117 — i16 EQ/NE "neither operand in HL": **skipped case**
  (`Z80LateOptimization.cpp:3144`).  Medium; needs an eviction-safety guard; low
  real-world yield (no current firing site).
* #206 — known-const copy for non-A regs: **not implemented**.  Small, low-yield.
* #50 — memcpy/memmove LDI-unroll for speed: still always `ldir` (no unroll).

## Plan — sequenced by leverage × tractability × risk

### Tier 0 — tactical, in-backend, low-risk (ship in 1 session each)
0a. **#203 guard unification (finish).**  Single-source the two peepholes' safety
    predicates before adding *more* PUSH/POP surface in 0b.  Steps 1-3 already
    landed (helpers extracted); the remaining forward-scan/stack-depth
    restructure removes the drift treadmill.  *Prereq for 0b.*  Risk: low
    (behavior-preserving, oracle-gated).
0b. **#173 — pair 8-bit BSS spill into 16-bit PUSH/POP.**  Highest tactical yield
    (AES + cpnos; the across-CALL liveness analysis already exists in the
    peephole).  Turns 6 B → 2 B per spill.  Risk: medium (LIFO bracket safety —
    reuse the 0a-unified guards).
0c. **#146 — EX (SP),HL epilog peephole.**  Cleanest near-term win: fixed epilog
    shape match + dead-HL `LivePhysRegs` check, −2 B/fire, no new infra.  Risk: low.
0d. **#206 / #117 — measure-first.**  Both small but low-yield; instrument for a
    real firing site before investing.  Defer if none.

### Tier 1 — the A-accumulator structural fix (#172) — dominant single lever
Make `Z80PinAluAccumulator` production-viable: add LiveIntervals +
MachineLoopInfo carrier selection so it constrains the **one** interference-free
chain carrier to `AReg` (not every candidate — the current naive scope
net-regresses +24..+81 B by materializing an `LD A,r` greedy was eliding).
~200-400 LOC, in-backend, flag-gated until it beats baseline on the AES corpus.
This is THE dominant AES size/speed gap.  Risk: medium-high (regalloc
interaction); gate hard on the differential oracle + AES 13/13.

### Tier 2 — HL-pinned rematerialization (#166/#178)
Z80-specific `isReallyTriviallyReMaterializable` override allowing remat of an
HL-pinned/flag-setting op when FLAGS is provably dead **and** HL is free at the
clone site.  One virtual method, in-backend.  Narrow but real; pairs with Tier 3
un-reserve to relieve the HL-pinning pressure that sank `ADD16_acc`.  Risk: low
(only *enables* a transform; gate on oracle).

### Tier 3 — deep / upstream-shaped (multi-session, gated)
3a. **#110 + #27 — Z80-aware cost model + custom `RegAllocEvictionAdvisor`.**
    THE keystone.  Replaces the single-register-class hacks with directed
    allocation and makes `getRegAllocationHints` actually work — which retro-
    actively turns #172 (A-pin), #166 (HL remat pressure), #111/#115 (pointer
    pinning) from exclusion hacks into cost tunings.  Highest *strategic*
    leverage, highest risk/effort (LLVM 17+ advisor API, upstream-shaped).
3b. **#96 — first-class PUSH/POP spill mode.**  Blocked: LLVM `InlineSpiller`
    assumes addressable, order-independent slots; PUSH/POP are LIFO/SP-relative.
    Deep upstream-shaped change.  Retires the 0a/0b peephole entirely if landed.
3c. **#180 migration + #112/#38 un-reserve IX/IY.**  Delete ~2300 LOC of stand-in
    peepholes once #177/#178/#179 land (upstreamability gate); un-reserve relieves
    the 3-pair pressure underlying Clusters 1-2 (legality already fixed; Class-C
    density is the remaining cost-model tradeoff).

### Dependency / leverage summary
* **Cluster 3 (F2, hint-deaf allocator) is the keystone** — 3a unblocks the
  *proper* form of Tiers 1 & 2.  But it's the riskiest, so we ship the
  exclusion-class tactical versions (Tier 0/1/2) first for immediate bytes, then
  let 3a subsume them.
* Tier 0a **must precede** 0b (shared guards).
* Tier 2 and Tier 3c (un-reserve) **compose** — un-reserve relieves the pressure
  that makes HL-pinning a regression.
* Every tier gates on: differential oracle (799/0/50/207 + 793/0/50/213), AES
  13/13 PASS, cpnos PROM1 byte-watch (26 B free), lit green.

## Recommended opening move
**Tier 0c (#146)** as a clean confidence-builder, then **Tier 0a→0b (#203→#173)**
for the highest tactical byte yield, then commit a focused session to **Tier 1
(#172)** — the single biggest lever.  Hold Tier 3a (#110) for a dedicated
multi-session effort once the tactical wins are banked.
