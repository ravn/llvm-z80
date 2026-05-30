# Step 2 empirical findings — tied-AReg ALU modeling (#172)

Branch `z80-accumulator-tied-aReg-step2`.  This records the **measured outcome**
of wiring G_OR/G_XOR reg-reg to the tied-AReg pseudos (Step 2 of the design).
Bottom line: **the mechanism is correct but NET-REGRESSES real AES (+21 B). The
clean ISel-only fix does not pan out.**

## What was built

`Z80InstructionSelector.cpp` G_OR/G_XOR reg-reg case now emits the tied pseudo
(`COPY AccReg(areg)=Src1; ResReg(areg)=XOR_acc AccReg,Src2; COPY Dst=ResReg`)
instead of the physical-A triple.  Confirmed firing: post-isel MIR shows
`%N:areg = XOR_acc %a, %b, implicit-def $flags` at every reg-reg XOR/OR site;
zero `XOR_r`/`OR_r` remain.  Lit 138+4 green (correctness preserved).

## Measurements

| Shape | base | tied | Δ |
|---|---|---|---|
| straight-line reg-reg XOR chain (`mix3`) | 0x18 | 0x18 | **0** |
| loop-carried reg-reg XOR (`loop_rr`) | byte-identical | byte-identical | **0** |
| **aes256.c `aes_mixColumns`** | 248 | 261 | **+13** |
| **aes256.c `aes_mc_inv`** | 340 | 348 | **+8** |
| aes256.c total .text | 3231 | 3252 | **+21** |

## Why it fails — two distinct facts

1. **Straight-line chains were ALREADY optimal.**  Baseline `mix3` emits
   `ld a,d; xor e; xor h; xor l` — the physical-A path + coalescer already keeps
   a reg-reg XOR chain A-resident.  There was no shuttle to remove; tied-AReg is
   byte-identical.  The mechanism's headline benefit doesn't exist here.

2. **The loop-carried #172 shuttle is NOT fixed.**  `loop_rr` is byte-identical:
   the carrier still `ld a,d … ld d,a` each iteration.  Constraining the XOR
   *result* to A does **not** constrain the loop-carried *PHI carrier* to A — and
   the carrier cannot stay in A anyway, because A is contended within the loop
   body by the counter (`ld a,c; dec a`) and the shift (`add a,a`).  The shuttle
   is a property of the PHI's residency + intra-loop A-contention, which a local
   result-constraint cannot touch.

3. **Parallel kernels REGRESS.**  `aes_mixColumns`/`aes_mc_inv` have multiple
   simultaneously-live accumulators.  Forcing each reg-reg XOR result into the
   single-member `AReg` class removes the allocator's freedom to spread them
   across D/E/H/L, so it inserts worse copies / spills → +21 B.  This is the
   exact parallel-accumulator wall that regressed the global pass (#172, +24–81 B,
   sessions 73o/73p).  **Both naive mechanisms hit the same wall.**

## The real conclusion

There is **no cheap correct fix** for #172 via local constraint:

* Tied-operand ISel modeling (this attempt) and the global pre-RA pin pass
  (73o/73p) both fail for the **same** reason — they apply the single-A
  constraint *without discriminating* a single A-resident chain (where it helps,
  but baseline is already optimal) from parallel accumulators (where it hurts).
* The fix that would work must **discriminate the loop carrier** and pin only it,
  interference-aware — which ISel cannot do (no loop/interference info per-MI),
  and which the naive pre-RA pass also got wrong (crude MBB heuristic).
* Correct discrimination = a global, interference-aware loop-carrier+counter
  co-allocation — i.e. what SDCC's tree-decomposition allocator does, and *why*
  it wins on this shape.  That is a major undertaking (LiveIntervals +
  MachineLoopInfo + PHI-cycle analysis + a real spill-vs-pin cost model), with
  the magnitude analysis showing the prize is **benchmark-only** (production
  sources have zero gf-style chains; clang already beats SDCC on all production
  targets and on AES speed).

## Recommendation

**Do not ship this change** (it regresses).  **Do not pursue either naive
mechanism further.**  The #172 prize is real but small, confined to AES (a
non-shipped benchmark), and only recoverable by SDCC-grade global loop-carrier
allocation — a large, high-risk effort for benchmark-only bytes.  Park #172 with
this finding; the honest engineering call is that the cost/benefit does not
justify the global allocator work, and the tactical Tier-0 fixes (#146, #173 —
which target the spill traffic that production code DOES have) are the better use
of effort.

The valuable durable output: tied-operand ALU modeling is **mechanically sound
and correct** (it fires, lit-green), and the architectural diagnosis is now
empirically *bounded* — local residency constraints cannot fix loop-carried
accumulator residency on a sole-accumulator machine; only global allocation can,
and the gap doesn't justify it.
