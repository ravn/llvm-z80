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

## Refinement (after MIR dump): the bug is in RegisterCoalescer, not MachineLICM

A second pass with `-print-before=register-coalescer
-print-after=register-coalescer` clarifies the picture:

  - **Before RegisterCoalescer** in `licm_volatile`:
    `%7:gr16 = LD_r16_nn @target_fn` is at slot index **16B** (entry
    block).  MachineLICM has already done the hoist.
  - **After RegisterCoalescer**:
    `$de = LD_r16_nn @target_fn` is at slot index **320B** (loop
    body).  The coalescer rematerialized the def *back into* the
    loop.

Mechanism: `RegisterCoalescer::reMaterializeDef`
(`llvm/lib/CodeGen/RegisterCoalescer.cpp:1316`) gates on
`TII->isAsCheapAsAMove(*DefMI)` to decide whether to fold a def
into its copy's user.  When that fold pushes the def into a hotter
block (loop body), the result is the in-loop reload that #89
describes.

**Critically:** `RegisterCoalescer` *already has*
`MachineLoopInfo` (`llvm/lib/CodeGen/RegisterCoalescer.cpp:135`)
but does not consult it in the remat decision.  The infrastructure
exists; it just isn't being used at the cheap-as-move gate.

This narrows the fix surface considerably:

  - MachineLICM is fine.  Don't touch.
  - Greedy regalloc remat (line 1284 of MachineLICM, plus the
    spill-vs-remat path in `LiveRangeEdit`) is fine.  Don't touch.
  - Only `RegisterCoalescer::reMaterializeDef` line 1316 needs the
    loop-awareness fix.

## Refined options for #89

Replace the four options above with:

### (a-refined) Loop-aware override of `isAsCheapAsAMove(MI)`

`Z80InstrInfo::isAsCheapAsAMove(const MachineInstr &MI)` virtual
override returns FALSE when:

  - `MI` is `LD_r16_nn` (or `LD_r8_n`), AND
  - `MI`'s parent MBB has loop depth lower than... what?  TII
    doesn't have `MachineLoopInfo`.

Blocker: `TargetInstrInfo` is largely stateless.  Cannot directly
read `MachineLoopInfo` from inside the hook.  Requires either:

  1. Adding a new TII API the coalescer can call with MLI passed
     in: `bool shouldRematAcrossLoopDepth(const MachineInstr &Def,
     const MachineInstr &Use, const MachineLoopInfo &MLI) const`.
     Upstream LLVM change; small but cross-cutting.
  2. Pre-coalescer Z80 analysis pass that marks specific defs as
     "do not remat" via a custom `MachineInstr` flag, and TII reads
     the flag.  Self-contained but adds a Z80 pass.
  3. Modify `RegisterCoalescer.cpp` locally to pass MLI to the
     `TII->isAsCheapAsAMove` decision via a new optional API.
     Smallest cross-cut; cleanest.

Estimated: 2-3 sessions for option (a-refined), depending on which
sub-path.  Sub-path 3 is the most surgical.

### (b) Z80-specific MIR pass (still viable, parallel option)

A pass between MachineLICM and RegisterCoalescer that re-hoists
`LD rr,nn` defs into preheaders if loop pressure allows.

  - Pro: doesn't require modifying upstream LLVM.
  - Con: undoing what coalescer is *about to do* is fragile; the
    coalescer would just re-pull it down.  Probably needs to also
    block coalescer-time remat — which lands us back in (a).

### (c) Merge into the broader regalloc cost-model design

Now LESS attractive than it was before this refinement.  The
diagnosis isolates the fix to a single line in RegisterCoalescer;
folding it into a multi-issue cluster (#89 + #27 + #94 + #98)
loses that focus.

### (d) Loop rotation / DJNZ chain — depends on #100, separate thread.

## Updated recommendation

**Option (a-refined sub-path 3):** modify
`RegisterCoalescer::reMaterializeDef` to bail out of the remat
fold when the use's MBB has a loop depth strictly greater than the
def's MBB.  Universal change (not Z80-specific) and almost
certainly correct: rematerializing INTO a hotter block is a
performance regression on any architecture, the LLVM coalescer
just doesn't currently check.

**Why this is upstream-shaped:** the change is a one-liner in
RegisterCoalescer that adds a loop-depth comparison.  It's general
(not Z80-specific), it's a clear bug fix in a heuristic that's
clearly missing context the pass already has, and it would land
in `llvm-z80/llvm-z80` as a backend-touching but
generic-CodeGen-area patch — exactly the shape of contribution
that fits early engagement-mode work.

**Risk:** other targets may rely on the current "always remat
if cheap" behavior to flatten control flow.  Mitigation: gate the
new check behind a default-on `cl::opt` for one release cycle so
it can be disabled if a regression surfaces upstream.

Estimated: 1 session of implementation + lit tests + size
measurement on rcbios/cpnos-rom.  If the size delta on real
workloads is positive, this is an immediate close on #89.  If it
regresses something (it shouldn't, but might surface a different
heuristic interaction), revert and pivot to option (b).

## Path 2 result (option a-refined sub-path 3): also rules out

Implemented and tested in session 42.  Two variants of the
loop-depth check, both compared against the post-revert baseline
(BIOS 5929 B, cpnos-rom 1777 B):

### Variant 1: `if (UseDepth > DefDepth) return false;`

Reproducer: `licm_volatile`/`licm_local` correctly emit
`ld de,_target_fn` in entry block, NOT in the loop body.  Primary
#89 symptom is fixed.

Real workloads:

| Artifact          | Before | After  | Delta |
| ----------------- | -----: | -----: | ----: |
| BIOS (`bios.cim`) |  5929  |  5932  | +3 B  |
| cpnos-rom payload |  1777  |  1781  | +4 B  |
| Net               |        |        | +7 B  |

7 regressions, 2 improvements.  Improvements (`_chktrk -4`,
`_bios_reader_body -1`) are #89-style hoist wins.  Regressions
(`_erase_to_eol +8`, `_erase_to_eos +8`, `_delete_line +4`,
`_init_hardware +4`, `_bg_clear_from +5`, `_bios_write_c +1`,
`_fdc_read +3`) are register-pressure side effects: blocking
the in-loop remat forces the def to live across the loop, which
increases pressure inside the loop, which forces a *worse* spill
(typically 16-bit pointer onto BSS).

### Variant 2: `if (DefDepth == 0 && UseDepth > 0) return false;`

Tighter heuristic — only block when def is truly outside any
loop and use is inside a loop.  Result: **byte-identical to
Variant 1**.  All regression sites already match the depth-0-to-N
shape.  Tightening the heuristic does not narrow the blast radius
because the regression sites ARE the intended target — but
blocking remat at those sites is a net loss because of the
pressure increase.

### Conclusion

Loop-depth alone is not sufficient context.  The correct gate
needs **register-pressure awareness** at the coalescer-time remat
decision: "is keeping this def alive across the loop cheaper than
rematting it inside?"  That requires either:

  - Lifting the decision to the regalloc layer, where pressure is
    naturally available — i.e., option (c).
  - Building a pre-coalescer pressure-estimate analysis and
    annotating MIs accordingly — option (b).

Path 2 (option a-refined sub-path 3) reverted in session 42.
No commit landed for the change itself, only for the investigation
doc updates.

## Final session 42 conclusion on #89

Two empirical results in this session, both ruling out the simple
fixes:

  - **Path 1** (drop `isAsCheapAsAMove` from `LD_r16_nn`):
    BIOS +15 B, cpnos-rom +20 B.  Couples LICM hoist and
    coalescer remat; both regress.
  - **Path 2** (loop-depth check in
    `RegisterCoalescer::reMaterializeDef`): BIOS +3 B, cpnos-rom
    +4 B.  Better blast radius (1/5 of Path 1) but still net
    negative because the loop-depth comparison ignores register
    pressure, which is the actual decisive factor on Z80.

The diagnosis is now solid:

  - MachineLICM is fine; it hoists `LD_r16_nn` to entry blocks.
  - RegisterCoalescer pulls some of those defs back into loops via
    `reMaterializeDef`.
  - On a register-rich target, this would still be cheap (the
    cheap-as-move flag is roughly correct).
  - On Z80's 3-pair register file, the in-loop remat decision
    must consider whether the keep-alive alternative is cheaper.
    That's a register-pressure question, not a loop-depth one.

**Next-session entry on #89:** option (b) [Z80-specific pre-RA
pass with pressure estimate] or option (c) [merge into broader
regalloc cost-model surface, expected to subsume #89].  Option
(c) is simpler and remains the recommendation.

The investigation has now decisively shown that the coalescer-
side fix is not viable as a standalone change.  Future sessions
on #89 should not retry Paths 1 or 2 — both are documented dead
ends with measured byte costs.

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
