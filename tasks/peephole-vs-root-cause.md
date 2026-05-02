# TODO: Audit peephole optimizations for hidden root causes

User flag (2026-05-02, mid session 33): peepholes can paper over bugs in
earlier passes -- the surface symptom disappears but the root issue
(why was that pattern emitted in the first place?) remains and may
hurt other shapes that the peephole doesn't recognize.

## What to audit

Walk every peephole in `Z80LateOptimization.cpp` (and the new
`Z80LoopIdiomFill`) and for each ask:

1. **Why is the pre-peephole shape emitted at all?**  Is it a
   GISel/legalization choice we could change upstream?  An InstrSelect
   pattern that should be a single instruction?  A regalloc decision
   that's just suboptimal?
2. **Does the peephole fire often enough to mask a frequent mis-emission?**
   If 90% of cases get rewritten, the upstream fix would close 100%
   including the cases the peephole misses.
3. **Are there nearby shapes the peephole doesn't catch but should?**
   E.g. issue #74's cross-register-pair spill (HL→DE via BSS) is the
   same family as the same-register case (HL→HL) just with one operand
   different.  A root-cause fix in regalloc would handle both.

## Likely candidates (first pass)

- **BSS spill→PUSH/POP** (#74, #82): the regalloc keeps choosing BSS
  spill when push/pop is strictly cheaper.  Root cause: spill weight
  for short-lived 16-bit live ranges is too low / push/pop isn't
  modelled as a spill option.  Cross-register pairs aren't handled by
  the peephole today.
- **Carry-roundtrip elimination** (#93): fixed late, but the
  `add a,1; ld r,a` shape comes from the IR-level countdown→countup
  IV rewrite at -Oz (#95).  Path-a fix is already filed.
- **LDIR aftermath DE-state** (#78): the legalizer emits DE post-state
  reuse paths because of how G_MEMCPY lowering handles the trailing
  pointer.  Could the legalizer skip the redundant adjustment?
- **u8 switch range-check 16→8** (#86): the comparison gets widened
  by the type legalizer; the peephole undoes it.  Root cause: the
  legalizer should narrow when both sides are u8.
- **LD A,(HL); LD r,A → LD r,(HL)** (#76): GISel doesn't have a
  single-instruction pattern for `load → r != A`; it always goes via
  A.  Root: add the direct pattern in `Z80InstructionSelector.cpp`.
- **IX constant propagation**: the regalloc puts a constant in IX
  even though it's a callee-saved with high cost.  Root: regalloc
  cost model.
- **PUSH IX; POP rr; ADD HL,rr; PUSH HL; POP IX → ADD IX,rr**: GISel
  doesn't pick `ADD IX,rr` directly.  Root: `Z80InstructionSelector`
  pattern.

## Bar for "fix the root cause"

The user is OK with peepholes when:
- The root cause is fundamentally hard (e.g. requires changes in
  upstream LLVM passes we don't control).
- The peephole has near-100% coverage of the shape it claims to fix.
- A test exists that fails without the peephole AND fails (in the
  same way) without the root-cause fix -- proving they are the
  same defect.

The user is NOT OK with peepholes that:
- Cover only a narrow shape, leaving siblings unfixed.
- Hide a regression that would re-surface elsewhere.
- Make the IR-level / earlier-pass analysis harder to reason about.

## Action

When closing this TODO: produce a list "peephole X covers shape Y;
root cause is Z; fix-at-root effort is N hours" so the user can
pick which to elevate.
