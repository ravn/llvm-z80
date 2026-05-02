# Phase C investigation: regalloc cluster root cause

**Date:** 2026-05-02.
**Branch:** `session-36-code-density-plan`.
**Issues read in one sitting:** #94, #95, #98, #89, #99, #100.

## Common root cause

All six issues are surface symptoms of the same underlying property:

> **The Z80 backend's regalloc and pre-RA pipeline inherit LLVM's
> generic cost model and live-range model without enough target-
> specific overrides for Z80's cost asymmetries.**

Z80's relevant cost asymmetries:

1. **Cheap rematerialization.**  `LD r,nn` (3 B), `LD HL,nn` (3 B),
   `LD r,(nn)` (3 B), `LD HL,(nn)` (4 B) for non-volatile globals are
   cheaper or equal cost to a BSS spill+reload pair (4+4 = 8 B for
   `LD (nn),HL; LD HL,(nn)`).  Generic LLVM regalloc treats spill as
   the default; for Z80, remat is often strictly cheaper.

2. **DJNZ semantics.**  After `DEC B; JR NZ` (= the DJNZ peephole's
   input), B is dead on the fall-through path (= 0).  Generic regalloc
   doesn't see this because at regalloc time the instruction sequence
   is `DEC B; JR NZ`, not `DJNZ`, and the dead-flag isn't carried.

3. **Countdown vs count-up asymmetry.**  Generic LSR / IndVarSimplify
   prefers count-up-with-carry-test (cheap on x86/ARM; expensive on
   Z80).  Z80 wants countdown (`DEC r; JR NZ` is 3 B, `DJNZ` is 2 B;
   count-up wrap-test is 5+ B).

4. **Register class preferences.**  B for counters (DJNZ-eligible),
   HL for pointers (the only 16-bit indirect-addressable form), DE
   for loop-invariant 16-bit values (no good alternative).  Generic
   regalloc treats GR8 as a flat class; Z80 wants strong intra-class
   biases.

## Decomposition into actionable sub-phases

| Sub-phase | What | Closes | Layer |
| --- | --- | --- | --- |
| **C.1** | Model DJNZ as a primary opcode/pseudo with proper dead-B-on-fall-through | #94, #98 (by construction); simplifies #92 workaround | Instruction selection + TableGen |
| **C.2** | Extend rematerialization recognition for cheap Z80 forms (`LD r,nn`, `LD HL,nn`, `LD r,(nn)` for non-volatile globals) | #15, #89, #100; helps #99 | `Z80InstrInfo::isReallyTriviallyReMaterializable` + remat cost in regalloc |
| **C.3** | Target-aware IV form preference (Z80 prefers countdown) | #95 | `TargetTransformInfo` hooks or new `Z80LoopForm` IR-level pass |
| **C.4** | Regalloc swap-hint for competing-class registers (counter vs pointer when both want HL) | #99 | `getRegAllocationHints` extension |

## Ordering and dependencies

```
              C.1 (DJNZ primary)
              /   \
             /     \
            v       v
          #94      #98
        closes   closes
        (by      (by
        construction)
                       \
                        \
                         v
              C.2 (remat framework)
              /   |   \
             /    |    \
            v     v     v
          #15   #89   #100
                      (rotate
                       default-on)

C.3 (IV form) — independent of C.1/C.2
   |
   v
  #95

C.4 (swap-hint) — depends on C.2 (cleaner with remat available)
   |
   v
  #99 closes
```

C.1 is the **highest-leverage starting point**: closes 2 issues by
construction, removes the post-RA DJNZ peephole entirely (a strict
peephole→primary migration matching the project's
"root-cause-over-peephole" feedback), and simplifies #92's regalloc
hint workaround.

C.2 is the **largest-scope item**: closes 3 issues, gates the
`Z80LoopRotate` default-on flip (which is the original goal of #77a),
and is conceptually the cleanest fit for Z80's cost asymmetry.

C.3 is parallelizable with C.1/C.2.

C.4 is small and narrow; can wait until C.2 lands (the swap is
cleaner when remat is an alternative).

## Risks per sub-phase

- **C.1**: changing how DJNZ is selected affects every Z80 loop.
  Risk: regressions on patterns that today benefit from the peephole's
  permissive matching but break under stricter primary-opcode rules.
  Mitigation: keep the peephole alive during C.1 development as a
  safety net; remove only after measurement shows no regressions.

- **C.2**: extending remat recognition means more values get
  rematerialized instead of spilled.  Risk: T-state regression
  (memory `feedback_tstates.md` applies — code size AND execution
  time matter).  A `LD HL,nn` is 3 B / 10 T, vs spill `LD (nn),HL` +
  reload `LD HL,(nn)` = 8 B / 32 T.  Remat usually wins on both
  axes, but verify.  Mitigation: T-state benchmarks on the
  representative cpnos-rom and rcbios fixtures.

- **C.3**: changing IV form on Z80 may regress LSR's other
  optimizations.  Mitigation: scope the change narrowly via a Z80-
  target opt-in TTI hook, not a generic disable of LSR.

- **C.4**: swap-hint can create infinite-loop hint cycles in regalloc
  if not carefully guarded.  Mitigation: only emit the swap hint when
  the live-range structure unambiguously calls for it (counter live
  across the entire loop body AND pointer live across the entire
  loop body AND both initially want HL).

## Open architectural questions (answer before starting C.1)

1. What is the Z80 backend's current treatment of `DEC B; JR NZ`?
   Is it a fused MIR pseudo, or two separate MIR instructions joined
   only by the peephole?  Investigation needed in
   `Z80InstrInfo.td:160` and `Z80LateOptimization.cpp:760-803`.

2. Does `MachineLICM` already hoist `LD r,nn` for loop-invariant
   constants pre-regalloc?  If yes, C.2's remat extension only needs
   to handle the post-spill case.  If no, C.2 needs to coordinate
   with `MachineLICM`.

3. Is there an existing `isReMaterializable` flag on `LD_HL_nn` /
   `LD_BC_nn` / `LD_DE_nn` in `Z80InstrInfo.td`?  Quick check
   determines whether C.2 is a flag-flip or a code change.

4. Is there a Z80-specific `TargetTransformInfo` already?
   Determines whether C.3 has a hook to extend or needs a new
   `Z80TargetTransformInfo.cpp`.

These four checks should take 30 minutes and form the start of the
C.1 implementation session.

## Recommended starting move

**C.1 first**, with the four architectural questions answered up
front.  C.1 has:

- Smallest scope (one MIR-pseudo migration)
- Highest hit rate (closes 2 issues by construction)
- Aligns with the "root-cause-over-peephole" feedback
- Sets up the live-range model that C.2 needs to be effective

If C.1 reveals that DJNZ-as-primary is harder than expected (e.g.,
the peephole's matching is doing real semantic work that's hard to
encode in TableGen), pivot to C.2 (remat framework) instead and
revisit C.1 later.

## Out of scope for this investigation

- C.5+ (other regalloc tweaks not yet identified): wait until C.1-C.4
  measurements complete.  Don't speculate.
- Performance benchmarking infrastructure: covered separately as
  Phase A.1 (size baseline tracker, already pending).
- SDCC parity comparison: done at Phase D exit, not here.
