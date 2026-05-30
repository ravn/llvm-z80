# Z80 backend — architectural assessment (2026-05-30)

Triggered by: "the architectural design behind the z80 backend may be not fully
correct."  Verdict below is evidence-grounded against HEAD `5c174bc6410a`.

## CORRECTION (post-research, 2026-05-30)

The first draft of this doc (below) proposed fixing the residency gap with a
**dedicated global pre-RA residency pass** and cited **llvm-mos** as the
precedent.  Four parallel falsification investigations overturned that:

* **llvm-mos does the OPPOSITE.**  Read from its actual source
  (`MOSRegisterInfo.td`, `MOSTargetMachine.cpp`): it has **no** residency pass.
  It models A/X/Y + zero-page "imaginary registers" as ordinary register
  classes, constrains them at **instruction selection** (instruction-specific
  classes like `AImag8`), and hands residency to **stock greedy** (running the
  coalescer twice after TwoAddressInstructionPass).  The author's framing: greedy
  is "most pleased" with this.  So a global pass is *not* the canonical fix.
* **The idiomatic LLVM mechanism is tied two-address operands + a constrained
  register class** (`Constraints="$dst=$src"`, `TwoAddressInstructionPass` +
  RegisterCoalescer) — exactly how x86 keeps values in EAX.  The in-tree
  `INC16`/`DEC16`/`LSHR16`/`ASHR16` already ship this pattern (tied + constrained
  class, chained) default-on and produce good code.  A **single-register class
  (`AReg`) is a HARD constraint greedy cannot escape** — no pass, no hint needed.
* **"Vacuous RegBankSelect is the gap" was a category error.**  Banks partition
  register *files*; residency is a register *class* constraint, assigned at
  instruction selection via `constrainRegClass`.  RegBankSelect was the wrong
  phase to point at.
* **The urgency was overstated.**  Clang already beats SDCC on every production
  target (cpnos −6 %, BIOS −3.2 %, autoload) and on AES *speed* (all 13 configs).
  Production sources have **zero** `^=`/shift-assign chains — the gf-style pattern
  is benchmark-only.  SDCC's edge is multi-factor (its global tree-decomposition
  allocator + 600 peepholes + tuned calling conventions), and SDCC itself loses
  40–70 % to IAR on real code.

**Corrected conclusion:** the *diagnosis* (8-bit ALU residency is decided too
late, by greedy, instead of structurally) stands as a code-quality observation.
The *fix* is **not** a global pass — it is **tied-operand two-address modeling in
the single-register `AReg` class, expressed at instruction selection** (the
llvm-mos / INC16 pattern).  This is *smaller and more correct* than the proposed
pass, has direct in-tree precedent, and retires the parked
`Z80PinAluAccumulator`.  Its value is **architectural maturity / upstreamability**
(removing reactive machinery, founding the same discipline for HL), explicitly
*not* production bytes — which is the right reason to do it given the project's
maturity goal, and acceptable per the user's "fix it well, weeks don't matter."

See `accumulator-tied-areg-design-2026-05-30.md` for the implementation spec.
The original analysis below is retained for the diagnosis; its *prescription*
(global pass) is superseded.

---

## Verdict (original draft — diagnosis valid, prescription superseded above)

**The architecture is not *wrong*, but it is *incomplete at a structural phase
level* in one specific, decisive way: the Z80 is an accumulator machine, and the
backend defers the accumulator-residency decision (what lives in A / in HL) to
LLVM's late, global, hint-deaf greedy register allocator — then tries to recover
the accumulator structure reactively, after the fact.**  *(Corrected: the fix is
tied-operand modeling at ISel, not a global pre-RA pass — see top.)*

## The evidence

1. **RegBankSelect is vacuous.**  `Z80RegisterBanks.td` defines a *single* bank:
   `def AnyRegBank : RegisterBank<"Any", [GR8, GR16, Fakei32, Fakei64]>;`
   `Z80RegisterBankInfo::getRegBankFromRegClass` always returns `AnyRegBank`.
   GISel's RegBankSelect phase — whose entire purpose is to make the early,
   structural "which register family does this value belong to" decision before
   instruction selection — does *nothing* on Z80.  The comment rationalises it as
   "Z80 has limited registers, so place everything in one bank," which conflates
   *few* registers with *uniform* registers.  Z80 has few **and** highly
   non-uniform registers (A = sole ALU accumulator, HL = sole 16-bit address
   accumulator).  Non-uniformity is exactly what early structural assignment is
   for, and it was discarded.

2. **Residency is therefore decided by greedy — which can't see Z80 payoffs.**
   `getRegAllocationHints` (`Z80RegisterInfo.cpp:2098`) records that a soft A/HL
   hint produced **zero** byte change on AES (21 hints fire in `aes_mc_inv`, all
   overridden by greedy's copy-elimination).  Greedy's local model prefers
   turning a COPY into `LD X,X` (0 B, eliminated) over honouring a hint (`LD B,X`,
   1 B); the downstream Z80 payoff (eliding the `ld a,r…ld r,a` shuttle, enabling
   DJNZ, freeing HL) is invisible to it.

3. **The structure is clawed back reactively, in two fragile registers.**
   * Pre-RA, via *opt-in per-register* constraint passes that each pin ONE
     special register with a single-register class: `Z80SplitDjnzCounters` (B,
     on), `Z80PinAluAccumulator` (A, **off**), plus the `BReg/AReg/BCReg/GR16NoIR`
     classes (`Z80RegisterInfo.td:179-221`).
   * Post-RA, via ~2300 LOC of the 6349-line `Z80LateOptimization.cpp` (16 of 38
     peepholes are stand-ins for missing infrastructure, #180).
   Both are reactive patches on output greedy already shaped, not direction given
   to greedy beforehand.

4. **The backend already knows the reactive approach is wrong.**
   `Z80PinAluAccumulator.cpp:54-78` documents that local per-MBB pinning
   net-regresses AES (+61/+81/+24 B) because "pinning a proxy forces a
   materializing `LD A,r` that greedy was previously eliding… single-candidate
   doesn't imply pin is free," and names the real fix: "MachineLoopInfo +
   LiveIntervals to pin the loop CARRIER only when its full chain is
   interference-free."  That is precisely a *global, interference-aware residency
   decision* — the missing phase, scoped to A.

## What the correct architecture is

Reference precedent: **llvm-mos** (6502 — the canonical LLVM accumulator-machine
backend).  The principle it embodies: express accumulator/register-file structure
through **register classes fixed at/around instruction selection**, and where a
global residency choice is required, make it in a **dedicated pre-allocation pass
that solves the accumulator-assignment problem interference-aware** — reducing the
general allocator's job to spilling the remainder under pressure.  Never defer the
accumulator structure to the general allocator.

For Z80 concretely, the missing phase is a **single accumulator-residency
assignment pass** (a new pre-RA pass; the vacuous RegBankSelect is the *marker* of
the gap, not the thing to change — banks express physical register files, not
accumulator preference, so adding banks is the wrong tool):

* Decide, globally and interference-aware (LiveIntervals + MachineLoopInfo),
  which i8 SSA values live in A (ALU-chain carriers) and which i16 values live in
  HL (address accumulators).
* Constrain those vregs to `AReg` / an `HLReg`-family class.
* Insert the minimal `LD A,r` / `LD r,A` (and HL equivalents) only at residency
  *boundaries*, and only when the boundary copy isn't already free.
* Hand greedy a problem where the accumulator skeleton is fixed, so greedy
  allocates only the non-accumulator remainder + spills.

This is the principled *generalisation* of the three reactive pieces
(SplitDjnzCounters, PinAluAccumulator, the single-register classes) into ONE
coherent phase.  It is structurally what SDCC's tree-decomposition allocator does
for the accumulator dimension — and SDCC's edge here is exactly that it makes this
decision globally, which is why it wins.

### What it subsumes
* **Cluster 1 (#172/#173/#20)** — A-residency decided structurally; the shuttle
  and the A-routed spill disappear by construction.
* **Cluster 2 (#166/#178)** — once HL-residency is a structural decision, "recompute
  vs spill" is answerable, and the remat/peephole treadmill (#203) loses its job.
* **Cluster 3 (#27/#110/#111/#115)** — the keystone *is* this phase; the single-
  register-class hacks become its output, not hand-written exceptions.
* **Cluster 4 (#180)** — the migrate-class peepholes that exist because greedy
  emitted non-Z80 MIR are no longer needed; late-opt shrinks toward its ~3000-LOC
  ISA-specific core.

## Honest cost

This is significant.  The hard part is the global accumulator-assignment solver
(an interference-aware constrained assignment — SDCC-grade).  Scoped to A only and
to loop carriers, it is the ~200-400 LOC the pass author already flagged; extended
to HL-residency and straight-line code it is larger.  It is the *right* fix and it
subsumes the whole codegen-issue backlog, but it is weeks, not a weekend.  The
Tier-0 tactical fixes (#146, #203→#173) remain worthwhile interim bytes and are
independent of this.

## Recommended path (incremental, de-risks the thesis)

1. **Prove the architecture on the dominant lever first:** rebuild
   `Z80PinAluAccumulator` as a *global, interference-aware* A-residency pass
   (LiveIntervals + MachineLoopInfo; pin the carrier only when the chain is
   interference-free and the boundary COPY is free), and **measure on the AES
   corpus**.  Success criterion: net byte/tstate *improvement*, default-on-able.
   This validates the whole architectural thesis at the point of maximum payoff.
2. **Generalise to HL-residency** (address accumulators) once A is proven —
   folding #166/#178's recompute-vs-spill into the same phase.
3. **Retire the reactive pieces** (single-register-class passes, migrate-class
   peepholes) as the unified phase subsumes them.

The alternative — a from-scratch unified residency solver before any measurement —
is higher risk for the same destination.  Increment 1 is the test of whether the
architecture diagnosis is right, at the lowest cost to find out.
