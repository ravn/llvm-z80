# Issue #89 — investigation 2026-05-03 (session 42)

**Status of #89:** still open; this session ruled out one path.

## What I tried

Path 1 from the prior session-32 / session-39 investigations: tune
the rematerialization cost model so 3-byte `LD rr,nn` isn't pushed
into loop bodies as if it were free.

Concrete change attempted:

```td
// llvm/lib/Target/Z80/Z80InstrInfo.td, def LD_r16_nn
let isAsCheapAsAMove = true;     // <-- removed
let isMoveImm = true;            // kept
let isReMaterializable = true;   // kept
```

Rationale: `LD rr,nn` is 3 B / 10 T-states; a register move
(`LD r,r'` over an 8-bit half) is 1 B / 4 T-states.  Marking the
pseudo `isAsCheapAsAMove` claims rematerialization-cost == move-cost,
which is the lie that biases regalloc and blocks MachineLICM hoisting.

## What happened

### On the synthetic reproducer (issue body)

`licm_volatile` and `licm_local` (the two cases that previously
emitted `ld de,_target_fn` *inside* the loop body):

  - **Before:** 24 B per function; constant reloaded every iteration.
  - **After:** 23 B per function; **constant correctly hoisted into
    the entry block** (good!).  But the regalloc reaction was to
    BSS-spill the i8 counter via `LD (sfr-1),A` / `LD A,(sfr-1)`
    every iteration instead of allocating it to free B/C.
  - Net byte count on the synthetic: ~unchanged.  Bottleneck moved
    from "16-bit constant in DE" to "8-bit counter via BSS slot".

`licm_nonvol` (the case already collapsing to seed-LDIR via #88 /
`Z80LoopIdiomFill`) was unaffected: still 16 B clean.

### On real workloads (post-revert baseline: BIOS 5929 B, cpnos-rom 1777 B)

`size-baseline.py check` after the change applied:

| Artifact          | Before  | After   | Delta |
| ----------------- | ------: | ------: | ----: |
| BIOS (`bios.elf`) |  5929 B |  5944 B | +15 B |
| cpnos-rom payload |  1777 B |  1797 B | +20 B |

Per-function detail (showing 9 regressions, 2 improvements):

| Function (artifact)               | Before | After | Delta | Class           |
| --------------------------------- | -----: | ----: | ----: | --------------- |
| `_chktrk` (bios)                  |   136  |  132  |   −4  | improvement (LICM) |
| `_bios_reader_body` (bios)        |   102  |  101  |   −1  | improvement (LICM) |
| `_delete_line` (cpnos)            |    67  |   71  |   +4  | regression (remat) |
| `_erase_to_eol` (cpnos)           |    47  |   55  |   +8  | regression (remat) |
| `_erase_to_eos` (cpnos)           |    52  |   60  |   +8  | regression (remat) |
| `_bg_clear_from` (bios)           |   262  |  267  |   +5  | regression (remat) |
| `_bios_conin` (bios)              |   128  |  135  |   +7  | regression (remat) |
| `_bios_seldsk_c` (bios)           |   199  |  201  |   +2  | regression (remat) |
| `_bios_write_c` (bios)            |   170  |  171  |   +1  | regression (remat) |
| `_specc` (bios)                   |   676  |  680  |   +4  | regression (remat) |
| `_relocate_bios` (bios)           |    55  |   57  |   +2  | regression (remat) |

The improvements are LICM-hoist wins: a loop-invariant constant
that previously paid in-loop reload cost now pays it once at
function entry.  The regressions are regalloc-resists-remat: a
constant used at a few sites with a CALL or other live-range
break in between is now spilled-and-reloaded instead of being
re-emitted as `LD rr,nn` at the use site.

The two regression families have *opposite* sensitivity to the
flag.  `isAsCheapAsAMove` controls both:

  - **MachineLICM.cpp** (`MachineLICM::IsLICMCandidate` →
    `MachineInstr::isCheapInstruction`): cheap-as-move is treated
    as "not worth hoisting" — hoisting saves nothing if a remat
    at the use site would be free.
  - **RegisterCoalescer.cpp / LiveRangeEdit.cpp**
    (`MachineInstr::isAsCheapAsAMove` → remat decision): cheap-
    as-move is treated as "remat at use site beats keeping live
    across the gap."

Z80 wants:

  - LICM: hoist (because the in-loop `LD rr,nn` is genuinely
    expensive — 3 B × N iterations).
  - Regalloc remat: prefer remat over BSS spill (because a 3 B
    `LD rr,nn` is cheaper than 6+ B of BSS spill+reload).

The TableGen flag can't satisfy both.  Path 1 is therefore the
wrong shape of intervention.

## Conclusion

**Path 1 (remove `isAsCheapAsAMove` from `LD_r16_nn`) is wrong.**
Reverted in this session (no commit landed; doc-only deliverable).

The cost-model gap is real, but fixing it requires intervention at
the *pass* level, not the TableGen level.  The flag is a single
boolean and cannot encode the context-sensitive answer Z80 needs.

## Next-step options for #89

Listed in increasing scope:

### (a) Fix downstream of the flag — narrow, target-specific override

Override `Z80InstrInfo::isAsCheapAsAMove(const MachineInstr &MI)`
on the C++ side to return a context-sensitive answer.  The default
queries the `MCInstrDesc` bit; backends can override.  However:

  - LICM queries the bit indirectly through `isLoopInvariantInst`
    helpers, not through the virtual hook.  Likely insufficient
    leverage on its own.
  - Verify by reading `MachineLICM.cpp` — does it call
    `MI.isAsCheapAsAMove()` (virtual) or `MI.getDesc().isAsCheapAsAMove()`
    (descriptor-direct)?  If the latter, the hook is moot.

Estimated: 1-2 sessions of source reading + small experiment.

### (b) Pre-RA Z80-specific LICM-supplement pass

Run AFTER MachineLICM but BEFORE register allocation.  Walk inner
loops, find `LD rr,nn` defs in the loop body whose imm is loop-
invariant (trivially true for an immediate), check that the loop
preheader has a free 16-bit pair across the loop's live-out, and
hoist if so.

Mirrors `Z80LoopIdiomFill` in spirit but at a different layer.

  - Pro: solves #89 surgically without touching regalloc remat.
  - Con: requires building a small live-range / register-pressure
    model at this layer.  Z80's 3-pair file makes the analysis
    tractable but non-trivial.

Estimated: 2-3 sessions (design doc, implementation, tests).

### (c) Fix at the regalloc layer — context-sensitive remat cost

Override the greedy-regalloc remat decision via
`TargetInstrInfo::isReallyTriviallyReMaterializable` or via the
remat-cost component of the spill-vs-remat heuristic.  Goal: keep
remat as a viable choice (so `_specc` etc. don't regress) but
penalize it enough that LICM hoisting dominates when both are
available.

  - Pro: addresses the underlying mismodel.
  - Con: this is exactly the surface-area #94/#98/#89/#27 are
    already working on.  The "right fix" is probably to wait for
    that cluster's design to land and merge #89's needs into it.

Estimated: blocked on #89/#27/#94/#98 cost-model work landing.
Multi-session.

### (d) Address #89 via loop rotation instead

If the loop is rotated do-while, the existing DJNZ-prefer chain
+ B-counter hint puts the counter in B, which leaves DE free for
the constant (no regalloc conflict, no need to hoist).  Path
already documented as #100-gated (rotation regresses BSS-spill
shape on rcbios) — needs #100 fixed first.

  - Pro: closes #89 as a side effect of #100, no new pass needed.
  - Con: depends on #100, which depends on regalloc cost-model
    work (option c).  Same critical path.

Estimated: blocked on #100.

## Recommendation

**Stop work on #89 in this session.**  Fold its requirements into
the upcoming #89/#27 regalloc-cost-model design (option c); it is
not a standalone fix.  The session-32 / session-39 prior comments
on #89 already arrived at the same conclusion via different
investigations; this session's contribution is empirical evidence
that the simplest TableGen-level intervention (Path 1) regresses
real workloads even though it solves the synthetic.

**For the structural plan:** #89 is still on the list (next active
entry per session 42's plan update), but the first session of work
on it has now produced an investigation deliverable rather than a
fix.  Update the plan to reflect that "#89 design phase" is the
next deliverable, not "#89 fix attempt".

## Lessons logged

  1. **TableGen flags are coarse.**  `isAsCheapAsAMove` couples
     LICM hoisting and regalloc remat.  When a target needs the
     two behaviors to diverge, the flag is the wrong knob.
  2. **Synthetic-only data can mislead.**  The synthetic showed
     the structural fix "working" (LICM hoist), with the regalloc
     bottleneck merely shifting.  Real-workload measurement was
     what surfaced the regression.  Always measure rcbios +
     cpnos-rom before believing a regalloc-area change.
  3. **Prior-session conclusions still hold.**  Sessions 32 and 39
     each reached the same "deeper than the hint" conclusion via
     different routes.  This session adds the
     `isAsCheapAsAMove`-removal data point to the same conclusion.
     The pattern is: #89's residual genuinely requires regalloc
     surgery, not a TableGen tweak.

## References

  - Issue text: ravn/llvm-z80#89
  - Comment 1 (session 32, 2026-05-02): partial relief via #88;
    hint extension reverted.
  - Comment 2 (session 39): `Z80SplitDjnzCounters` doesn't apply;
    three fix paths identified, all multi-session.
  - This document: 2026-05-03 (session 42); rules out
    `LD_r16_nn` `isAsCheapAsAMove` removal.
  - Related: #100, #94, #98, #27, #38 (re-test step waits on the
    same cost-model work).
