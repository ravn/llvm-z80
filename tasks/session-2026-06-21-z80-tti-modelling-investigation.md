# Session 2026-06-21 — is the Z80 modelled correctly in TTI?

**Trigger.** The user asked whether bug 3 (SimplifyCFG `foldTwoEntryPHINode`
no-PGO path; demoted from upstream queue after AVR triage 2026-06-07, kept
as fork-local fix #168 `cd2a2ace8754`) might be a symptom of a deeper
modelling gap rather than a SimplifyCFG bug.  Speculation: if the Z80 cost
model told the truth about how expensive a select is on Z80, the fold
would cost-gate itself and we would not need the SimplifyCFG patch.

This session checks the speculation across the whole TTI surface, not just
the one hook.

## What we found

The Z80 TTI (`llvm/lib/Target/Z80/Z80TargetTransformInfo.{h,cpp}`) is
well-thought-out where it speaks (register pressure, `isLegalAddImmediate`,
`Mul=Expensive`, `areInlineCompatible`, `getPredictableBranchThreshold=0`,
type-narrow casts, `shouldExpandExperimentalMemSetPattern`), but four
hooks consumed by **scalar** passes are absent and fall back to
target-independent defaults that do not reflect Z80 reality.  The
mis-labelling at `Z80TargetTransformInfo.cpp:38-44`
("Vectorizer-only hooks (...getCmpSelInstrCost...) intentionally NOT
implemented (no SIMD on Z80)") is factually wrong: SimplifyCFG,
SelectOptimize, SpeculativeExecution, IROutliner, ConstantHoisting, CGP
all consume those hooks in scalar code paths.

### Hole 1 (direct cause of bug 3): `getCmpSelInstrCost`

Not implemented.  `BasicTTIImpl::getCmpSelInstrCost` returns `LT.first * 1`
for any legal scalar select, i.e. **1**.

A Z80 select has no hardware instruction.  `Z80LowerSelect` expands it to
a compare + branch + materialize sequence, easily 10+ bytes.

Direct scalar consumers verified by grep on the in-tree LLVM at HEAD:

- `llvm/lib/Transforms/Utils/SimplifyCFG.cpp:3116`
  `validateAndCostRequiredSelects` -> `foldTwoEntryPHINode`. **This is
  bug 3.**  An honest Z80 select cost makes the fold cost-gate itself; the
  −16 B Z80 win currently captured by #168's bespoke SimplifyCFG patch is
  recovered for free.
- `llvm/lib/CodeGen/SelectOptimize.cpp:1411`
- `llvm/lib/Transforms/Scalar/SpeculativeExecution.cpp:252`
- LICM, LoopUnroll, IROutliner via `getInstructionCost` dispatch.

### Hole 2: `getCallInstrCost`

Not implemented.  Default = 1.  Z80 reality: `CALL nn` = 3 B / 17 T plus
caller-saved register flush.  Affects IROutliner, IPO inlining heuristics
beyond our `areInlineCompatible` short-circuit, and any cost analysis
that estimates a basic block containing calls.

### Hole 3: `getIntImmCost` / `getIntImmCostInst` / `getIntImmCostIntrin`

Not implemented.  Default returns `TCC_Free` for the immediate.  Z80
reality: `LD A,n` = 2 B, `LD r,n` = 2 B for any 8-bit reg, `LD rr,nn` = 3
B (BC/DE/HL), 4 B for IX/IY (DD/FD prefix).  ConstantHoisting reads
these to decide hoist-once vs rematerialize-per-use.  With the default,
multi-use 16-bit constants are not hoisted on Z80 when they should be (or
vice versa).

**Empirical follow-up 2026-06-21 (same day)**: implemented the override
with RED-GREEN-REFACTOR.  Test: 3-block function with constant 4660 used
as call arg in three distinct blocks.  RED confirmed (3 × `ld hl, 4660`).
Implemented `getIntImmCost`, `getIntImmCostInst` (with Add/Sub |Imm|<=3
and i8 ALU folds returning TCC_Free), and `getIntImmCostIntrin`.  Rebuilt
llc, re-ran.

**Zero codegen change.**  Verified ConstantHoisting DOES fire at IR level
(`-print-after=consthoist` shows the hoisted `%const` marker and three
uses sharing it).  Verified the hoist survives IRTranslation to GMIR
(`%11:_(s16) = G_CONSTANT i16 4660` in entry, `$hl = COPY %11` at each
use).  Then the register allocator dematerializes it because
`Z80InstrInfo.td:1316`'s `LD_r16_nn` pseudo is marked
`isAsCheapAsAMove = true; isReMaterializable = true` -- a *correct*
marking (keeping a constant alive across multiple branches consumes one of
Z80's 3 register pairs for the dominating region, which is expensive;
rematerializing at each use is cheaper).

Reverted both .h and .cpp.  Lit test deleted.  Issue #229 held open at
fork (not WONT-FIX) for a future scenario where regalloc gains awareness
of hoisted constants.  Full attribution comment at
`ravn/llvm-z80#229#issuecomment-4762027638`.

This is the **second inert hole** of the 2026-06-21 inventory (after Hole
4 / #230).  Both follow the same pattern: cost fix correct as a model
statement, upstream consumer fires correctly, downstream Z80-specific
decision (LSR canonicalization for #230, regalloc's `isReMaterializable`
for #229) makes the IR-level transformation invisible in final codegen.

**Implication: the entire 2026-06-21 hole inventory is at risk of being
inert on GISel-Z80.**  Holes 1 (#227 `getCmpSelInstrCost`) and 2 (#228
`getCallInstrCost`) deserve the same empirical test before assuming
either is a production win.

### Hole 4: `isLegalICmpImmediate` on Z80TargetLowering

Not overridden.  `Z80TargetLowering` has `isLegalAddressingMode`,
`isTruncateFree`, `isZExtFree` but no `isLegalICmpImmediate`.  Default =
true for the full 64-bit range.  Z80 reality: `CP n` exists for 8-bit
A-vs-immediate (1 byte).  No `CP rr,nn` form for 16-bit — a 16-bit
cmp-vs-imm must `LD rr,nn` first.

**Empirical follow-up 2026-06-21 (later, same day)**: attempted to ship
the override with RED-GREEN-REFACTOR.  Test-first: 5-case lit, run RED
(passed: codegen matches today's output).  Implemented the override
exactly as proposed.  Re-ran: **zero codegen change on any of the five
cases.**

Traced consumers: LSR's only call site is `LoopStrengthReduce.cpp:1899`
in `LSRUse::ICmpZero` formula scoring -- saying "false" makes LSR
*decline* to propose the `cmp x, c => cmp (x-c), 0` normalization, NOT
force a countdown IV.  The four `TargetLowering.cpp` consumers
(`:5234`, `:5430`, `:5443`, `:5646`) are all SelectionDAG combine layer
-- dead for our GISel-only backend.  CGP `optimizeCmpInstr` does not
consult this hook (uses `shouldFoldICmpWithConstant` instead).

Net: the override is correct as a model statement but has **no
observable codegen impact** on Z80 today.  Reverted both .h and .cpp
changes; lit test deleted.  Issue #230 held open (not WONT-FIX -- if
Z80 ever gains a SelectionDAG path, the four `TargetLowering.cpp`
consumers become live) but moved to back of the queue.  Full
attribution comment at `ravn/llvm-z80#230#issuecomment-4761976501`.

This finding **strengthens** the #184 reconsideration in the next
section: Hole 4 was already argued not to disambiguate the AES-vs-cpnos
asymmetry; the empirical inertness confirms it can't influence anything
in the relevant code path.

### Hole 5 (not a hole, just a comment fix)

`Z80TargetTransformInfo.cpp:38-44` lists `getCmpSelInstrCost`,
`getCFInstrCost`, `getMemoryOpCost`, `getGEPCost` as "vectorizer-only
hooks ... intentionally NOT implemented (no SIMD on Z80)".  Three of
these are scalar consumers in real passes; the comment is misleading
future readers (including us).  Strike `getCmpSelInstrCost` and
`getCFInstrCost` from the list; rephrase the rationale.

### Architecturally inexpressible (accept as-is)

The same wall that closed #184 WONT-FIX (per-(opcode, type) scalar cost
cannot encode regalloc outcomes) also limits how much benefit we can
extract from a "more accurate" `getMemoryOpCost`: the dominant code-size
waste (BSS shuttle traffic, Code Density row 1) is a regalloc-outcome
property, not a per-load property.  Modelling load-address-mode in TTI
would gain little.  Leave `getMemoryOpCost` alone.  Same reasoning for
`getCFInstrCost`: default (1 for branches, 0 for PHI under size) is
close enough to Z80 reality (JR=2B, JP=3B) that an "accurate" model
risks more regression than gain.

### Reconsideration of #184 (i16 arithmetic cost) in light of Holes 1-4

The user asked to reconsider #184 (closed WONT-FIX 2026-05-30) given the
broader hole inventory.  Outcome: **#184 stays WONT-FIX.**  None of
Holes 1-4 give a route through the structural limit:

- Hole 1 `getCmpSelInstrCost` — IndVarSimplify's widening cost loop
  doesn't sum select costs (only the IV add); no leverage.
- Hole 2 `getCallInstrCost` — IndVarSimplify doesn't see calls in its
  cost summation; no leverage.
- Hole 3 `getIntImmCost` — the IV increment is constant (+/-1) at both
  i8 (`inc a`) and i16 (`inc bc`), both 1 byte; an *honest* cost is
  identical and can't break the tie.
- Hole 4 `isLegalICmpImmediate` — saying i16-cmp-imm is more expensive
  favors keeping i8 (cheaper exit cmp); that's the AES preference but
  the cpnos *anti*-preference.  Same asymmetry, no disambiguation.

The real asymmetry is **call-crossing liveness**, not width-cost:

- AES IV is i8 in source.  IndVarSimplify widens to i16.  We want the
  i8 form (smaller addressing patterns + Z80NarrowIV reverts to i8).
- cpnos `_netboot_mpm` IV is i8 in source.  IndVarSimplify *would*
  widen to i16 — which we want, because the i16 lives in a callee-save
  pair (BC) across the in-loop CALL.  An i8 alive across a CALL gets
  shuttled through A + a BSS spill slot (A is the only 8-bit ALU reg
  and caller-saved), 78->84 insns / +11 B.

Both IVs are i8 in source.  Both have constant +/-1 increments.
The cost API has no signal to distinguish them.  The right fix is a
regalloc-aware narrowing pass — captured as a new tracker (Hole 5).

This reconsideration is the *load-bearing* contribution of this
session: it doesn't reopen #184; it *strengthens* #184's WONT-FIX
verdict by canvassing every adjacent cost hook and finding none gives
an alternate path.  Future readers of #184 should be pointed at this
session writeup as the "we tried again, the wall is still there"
record.

### Hole 5: `Z80NarrowIV` doesn't distinguish call-crossing IVs

`Z80NarrowIV` (session 73n, `llvm/lib/Target/Z80/Z80NarrowIV.cpp`)
narrows i16 IVs back to i8 after IndVarSimplify widens them.  It
helps AES.  But it shouldn't fire on the cpnos shape (i8 IV that
*needs* widening because of call-crossing).

The pass currently is structural ("narrow this IV if SCEV says it's
safe") rather than regalloc-aware ("narrow this IV only if it stays in
a register pair through the loop body").  Per the closing comment of
\#184: "an allocation-aware Z80 IV-narrowing pass (a smarter
`Z80NarrowIV`, built #73n / removed #73q, earlier versions hit
#169/#170/#171)".

Concrete missing predicate: "does this IV cross a CALL?"  If yes,
prefer the wider form (lands in a callee-saved pair).  If no, prefer
the narrower form (smaller addressing).  Implementable with a forward
scan of the loop body for CALL instructions plus IV use-site
classification.  Won't capture every regalloc subtlety, but should
disambiguate the AES-vs-cpnos asymmetry cleanly enough to flip the
underlying mid-end widening decision in either direction.

This is the only remaining route to #184's stated goal (close the AES
density gap by stopping over-widening) that doesn't ride on a
correctness-fragile cost-model lie.

## Why the speculation holds

Bug 3 was demoted from upstream because there is no in-tree
constituency: no in-tree target overrides `getPredictableBranchThreshold`
to zero, and no in-tree target makes select expensive enough to suppress
`foldTwoEntryPHINode` via cost.  But **our** target does — we just don't
tell LLVM that.  Filing upstream against SimplifyCFG was always the wrong
layer; the fix is in our TTI.  Filing the SimplifyCFG cost gate (#168) as
a fork-local patch was the right *correctness* outcome but the wrong
*architectural layer* — it patches downstream of a sound passing-test
that's reading bad data we gave it.

This same mis-layering pattern likely applies elsewhere: any time we
notice a generic pass making a Z80-bad decision, the first question
should be "what cost is it reading, and are we lying to it?"  This
session formalises that as a discipline going forward.

## Recommendation / tracker layout

File five scoped trackers at ravn/llvm-z80 (one per hole), each gated
behind the existing `-z80-experimental-tti-costs` flag for the same
proven-codegen-neutral measure-before-default-on rhythm that #177 set
up.  Order by signal-to-risk:

1. **`getCmpSelInstrCost`** — highest impact, lowest risk.  Closes bug
   3 properly.  Retires #168 SimplifyCFG cost gate as redundant.
2. **`getCallInstrCost`** — moderate impact.  Affects IPO/outliner
   decisions; some risk of inlining regressions on the production
   triplet, must A/B.
3. **`getIntImmCost` family** — moderate impact.  Affects
   ConstantHoisting.  Low risk because the pass tends to be
   conservative.
4. **`isLegalICmpImmediate` on Z80TargetLowering** — VERIFIED INERT on
   GISel-Z80 (empirical test 2026-06-21, see Hole 4 details above).
   Held open at the back of the queue for a future SelectionDAG path
   or LSR evolution.  Do NOT re-attempt without a new in-tree witness.
5. **Z80NarrowIV call-crossing predicate** — independent track
   (not a cost-model change).  The remaining route to closing #184's
   stated goal once Holes 1-4 are ruled out as alternate paths.

Plus a tiny code-comment fix at `Z80TargetTransformInfo.cpp:38-44` —
ride along with #1.

The four trackers do NOT motivate any upstream filing.  This is
internal modelling work.  If it goes well, the result is bug 3 closing
cleanly and a body of accurate-cost trackers that may surface
**further** late-patch retirements as a side effect — but no upstream
posts until we have specific in-tree witnesses for each, per the rules
that retired the 5-bug queue.

## Measurement discipline (carry from #177)

For each tracker that lands a cost-model change:

1. Implement behind `-z80-experimental-tti-costs` (already exists).
2. A/B on the production triplet (autoload PROM, cpnos PROM1, BIOS),
   the AES corpus, and compiler-comparison-corpus.
3. Add a CodeGen lit test pinning the output sequence with FileCheck
   (CLAUDE.md rule: every compiler change ships with a lit test).
4. If proven codegen-neutral-or-better on all measured workloads,
   default the flag on; if mixed, keep the flag opt-in and document
   the trade-off in the source comment.

## Files touched in this writeup

- (this file) `llvm-z80/tasks/session-2026-06-21-z80-tti-modelling-investigation.md`
- ravn/llvm-z80 tracker issues filed:
  - **#227** Hole 1: `getCmpSelInstrCost` not implemented (root cause of bug 3 and #168)
  - **#228** Hole 2: `getCallInstrCost` not implemented
  - **#229** Hole 3: `getIntImmCost` family not implemented
  - **#230** Hole 4: `isLegalICmpImmediate` not overridden on Z80TargetLowering
  - **#231** Hole 5: `Z80NarrowIV` call-crossing predicate (the route #184 left open)
- `tasks/todo.md` updated with the new trackers.
