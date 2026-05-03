# Session 40 plan (2026-05-03) — IX/IY allocation + shadow bank

Branch: `session-40-ix-iy-shadow`.  Started from main at
`e801d8974573` (post Phase 3 milestone).

## Direction (user)

> i want you to keep an eye on using the ix and iy registers too,
> and investigate if the separate bank can be useful

Two distinct strands.  Each is investigation-first.

## Strand A — IX/IY un-reservation (revisit #38)

Current state (entering session 40):

  - `Z80RegisterInfo::getReservedRegs` reserves both `Z80::IX` and
    `Z80::IY` unconditionally on Z80 (lines 150-151).
  - Comment block at lines 141-149 records that session 39
    re-investigated #38 after the #28 silent-miscompile fix; un-
    reserving IY still produced 11 runtime FAILs in the clang -Os
    suite.
  - `CLAUDE.md` (this dir) lists "IX/IY allocatable as general 16-bit
    registers" under *Known Working Optimizations* — this is stale,
    documents an earlier era.
  - Phase 3 (#94 / #99) demonstrated the single-register-class
    technique (`BReg`, `BCReg`) to wedge specific values past
    greedy's heuristics.  The same construction may be the lever
    that lets IX/IY be *opportunistically* allocated for a small
    set of patterns (e.g. functions with N>3 long-lived 16-bit
    values, or specific instructions where IX/IY is required like
    `ADD IX,rr`).

Sub-tasks:

  1. **Re-establish failure surface.**  Un-reserve IY only, build
     full clang -Os runtime suite, capture *which* tests fail and
     classify by symptom (wrong return value, hang, asm crash,
     code-size regression).  Compare to "11 FAILs" recorded in
     session 39.
  2. **Single-fault isolation.**  Pick the smallest failing test;
     compare un-reserved-IY asm to reserved-IY asm; identify the
     minimal codegen difference; trace to the regalloc decision
     that introduced it.
  3. **Design lever.**  If the failures cluster into a small number
     of root causes (likely: greedy chooses IY when a cheaper option
     exists, or DJNZ-style hint isn't communicated to IY), apply the
     single-register-class technique inverted: a *exclusion* class
     (`GR16NoIY` similar to existing) or a per-vreg negative hint.
  4. **Cost-model audit.**  Already known: `CostPerUse=1` for IX,
     `CostPerUse=2` for IY.  Verify these are still active and that
     greedy honours them; compare against what greedy actually
     selects in failing functions.
  5. **Land minimal subset that's a strict win.**  Even if full IY
     un-reservation isn't viable yet, a narrow path (e.g.
     `+ix-allocatable` feature flag, or only un-reserve in functions
     meeting specific criteria) may carry real-world wins.

## Strand B — Shadow bank as a second register set

Current state:

  - `+shadow-regs` feature flag exists; gates whether shadow
    registers are reserved (`getReservedRegs` lines 168-181).
  - Wired up only for ISR save/restore via EXX + EX AF,AF'
    (`getCalleeSavedRegs` lines 109-110, EXX_CSR_SaveList).
  - Not used as an alternate register set within general functions.
  - Issue #102 (closed session 39) removed disabled-pre-#102 EXX
    code that was attempting to use shadow as spill-replacement.
    Closure rationale (CLAUDE.md, "EXX spill conversion"): the
    shadow bank is a CONTEXT SWITCH not extra registers — cannot
    be inserted at arbitrary points.

Open question for this strand: *is there a useful niche for shadow
bank within a function*?  Three candidate niches:

  1. **Whole-function bank flip.**  Frame entry: EXX; do all work
     in shadow set; EXX before return.  Only viable if no callee
     observes the swap (no nested CALL while flipped, OR all
     callees are also flip-aware).
  2. **Hot loop with no CALL.**  Pre-loop: EXX (save BC/DE/HL by
     swap); loop body uses shadow set freely; post-loop: EXX.  Cost:
     2 bytes total.  Wins if loop body has > 2 bytes worth of
     reload pressure that disappears under swap.
  3. **ISR augmentation.**  ISRs already save via EXX.  If the ISR
     body needs MORE than the 6 shadow regs (rare), EX AF,AF' is
     also free.  Already handled by current ISR machinery.

Niche 2 looks tractable: a custom MIR pass detecting MBBs that
(a) form a loop, (b) contain no CALL, (c) have ≥ N register-class
pressure, and inserting EXX brackets.  Risk: hidden state changes
(e.g. interrupt handler that observes shadow reg) make this unsafe
on bare-metal Z80 without `interrupt-disable` discipline.

Sub-tasks:

  1. **Enumerate target loops in cpnos-rom + rcbios.**  Find loops
     that meet niche-2 conditions; measure how much pressure they
     have.
  2. **Compatibility check.**  Determine if any existing ISR or
     bare-metal idiom in our own sources reads the shadow set
     externally (would break under whole-function bank flip).
  3. **Prototype a single-loop bracketing transform**, gated by an
     `+shadow-loops` feature, on one targeted function.
  4. **Measure win vs cost.**  EXX is 1 byte each; bracket pair = 2
     bytes.  Need ≥ 3 bytes of avoided spill traffic per fired loop
     to beat parity.

## Order of operations

1. Strand A first (Z80 specifically; clearer immediate wins).
2. Strand B if Strand A finishes early.
3. Cross-check: any IY-reserved decision that becomes obviously
   wrong after looking at shadow-bank state.

## Acceptance criteria for strand A

  - All clang -O0/-Os runtime tests stay green.
  - lit suite stays at 83 PASS + 1 XFAIL (or grows with new
    targeted lit tests for the new behaviour).
  - rcbios + cpnos-rom byte-exact OR strictly smaller.
  - At least one identified clang test or function gets
    measurably smaller code via IX/IY allocation (otherwise:
    investigation-only result, not landed).

## Acceptance criteria for strand B

  - Investigation deliverable only this session.  Implementation
    gated on showing ≥ 3-byte win on a real loop without breaking
    correctness.
