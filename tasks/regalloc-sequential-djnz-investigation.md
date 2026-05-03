# Sequential DJNZ-loop investigation (#98)

Date: 2026-05-03 (session 39)

## TL;DR

The two-sequential-loops case in `djnz-comprehensive.ll`
(`_two_sequential_loops`) hits a single-DJNZ result because **the
register coalescer extends the second loop's counter live range
backwards across the first loop**, after which both counters genuinely
interfere on B.  The "B is dead between the loops" mental model is
wrong post-coalescing.

The fix is a small pre-RA target pass that re-introduces a `COPY`
at each DJNZ-eligible loop's preheader.  The new vreg's live range
covers only the loop body; the original (long) vreg can land in any
8-bit register without conflict; both per-loop copies independently
take their B hint.

## Method

Reproduced with `_two_sequential_loops` from
`llvm/test/CodeGen/Z80/djnz-comprehensive.ll`:

```
$ llc -mtriple=z80 -O2 -print-after-all \
    -filter-print-funcs=two_sequential_loops djnz-comprehensive.ll
```

96 stages dumped.  Three matter:

  1. After **Eliminate PHI nodes for register allocation** (2115)
  2. After **Register Coalescer** (2298)
  3. After **Greedy Register Allocator** (2510)
  4. After **Virtual Register Rewriter** (2563)

## What the coalescer does

After PHI elimination but **before** coalescing:

```
bb.0.entry:
  %0 = COPY $a           ; receives arg n
  %1 = COPY $l           ; receives arg m
  %29 = COPY %0          ; PHI entry def for loop1 counter
bb.1.loop1:
  %2 = COPY %29
  ... ; loop1 body
  %29 = COPY %7          ; PHI back-edge def
  JP_NZ %bb.1
bb.2.between:
  %30 = COPY %1          ; PHI entry def for loop2 counter
bb.3.loop2:
  %9 = COPY %30
  ... ; loop2 body
  %30 = COPY %12         ; PHI back-edge def
  JP_NZ %bb.3
```

Live ranges at this point:

  - `%0`: bb.0 only           (arg-receiving COPY into vreg)
  - `%1`: bb.0 only           (same, for arg m)
  - `%29`: bb.0 → bb.1 latch  (loop1 counter)
  - `%30`: bb.2 → bb.3 latch  (loop2 counter)

`%29` and `%30` **do not overlap**.  Both could land in B.

After coalescing:

```
bb.0.entry:
  %29 = COPY $a           ; merged %0 → %29
  %30 = COPY $l           ; merged %1 → %30  ← now in bb.0 instead of bb.2
bb.1.loop1:
  ... %29 ...
bb.2.between:
  ; empty
bb.3.loop2:
  ... %30 ...
```

The coalescer merged the `%1 = COPY $l` chain end-to-end, eliminating
one COPY but **extending `%30`'s live range backward to bb.0**.  Now
`%29` and `%30` both span bb.0 → loop1 → bb.2 territory; they
genuinely interfere on B.

This is normal coalescer behaviour — fewer COPYs is its goal — but
on Z80's tiny register file the trade hurts: it loses the DJNZ on
the first loop.

## What greedy then does

At greedy entry, both `%29` and `%30` carry the prefer-B hint
(installed by `Z80RegisterInfo::getRegAllocationHints`, lines
~1796-1801, the IsSelfBackEdge branch).  They interfere on B.  Greedy
picks one for B (apparently by priority — `%30` is the longer-lived
of the two and wins) and the other is forced to a different
allocation order entry.

Post-rewriter:

```
$d = COPY $a              ; %29 → D
$b = COPY $l              ; %30 → B
loop1: ... dec d; jr nz   ; D-DEC pattern, no DJNZ peephole
loop2: ... dec b; jr nz   ; B-DEC, then DJNZ peephole fires
```

## What this rules out

  - **(a) Live-range bug** — There is no spurious phantom liveness;
    the live ranges genuinely overlap *after coalescing*.  Pre-
    coalescing they don't.  So the "live-range analysis is wrong"
    hypothesis is false.

  - **(b) Greedy heuristic over-conservatism** — Greedy is correct
    given its inputs.  Both vregs interfere on B; one wins, one
    doesn't.  Greedy doesn't know about the post-RA DJNZ peephole's
    pay-off, so it can't justify spilling/splitting `%30` to free B
    across loop1.

  - **(d) Sub-register / register-class mismatch** — `gr8` is the
    same class for both; no class-based interference.

The genuine cause is **(b'): the register coalescer's optimization
extends a live range and hides a DJNZ opportunity from greedy**.

## Why "just disable the coalescer" is wrong

The coalescer is a load-bearing pass; without it most register
copies stay as moves and the code blows up in size everywhere.  We
want the coalescer's output **except** for this specific pattern.

## Proposed fix

A small **pre-RA target MachineFunctionPass**, run after the
coalescer's second invocation but before greedy, that walks each
DJNZ-eligible self-loop MBB and:

  1. Identifies the loop counter vreg (via the
     `$a = COPY %v; DEC_A; %v = COPY $a` triplet that the
     `getRegAllocationHints` self-back-edge logic already keys on).

  2. If the counter vreg's live range extends outside the loop MBB
     (i.e. the entry def is in a different block — exactly the
     state the coalescer just produced), creates a fresh `gr8`
     vreg and:
        a. inserts `%new = COPY %old` at the end of the loop's
           non-self predecessor (the preheader);
        b. rewrites every `%old`-use inside the loop MBB to `%new`,
           including the back-edge `%old = COPY $a` to
           `%new = COPY $a`.

After the rewrite:

  - `%old` keeps a small live range (entry-block COPY → preheader);
    no longer a B-hint candidate; can land anywhere.
  - `%new` lives only inside the loop; gets the prefer-B hint via
    the existing logic; can independently take B.

For sequential loops, two `%new`s are produced — `%new29` and
`%new30` — with non-overlapping live ranges.  Both safely land in B.

## Cost model

Per-loop overhead = one `COPY %new = COPY %old` at the loop
preheader.  Three cases at regalloc time:

  1. **Best case (sequential loops)**: greedy puts `%old` and `%new`
     in the same physical register (B for both, since their live
     ranges don't overlap).  COPY becomes `LD B, B`, removed by
     `MachineCopyPropagation`.  Zero static cost; saves 1 byte per
     decrement-and-branch site in the loop body.

  2. **Mixed case**: `%old` lands in some other register (e.g. D),
     `%new` in B.  COPY emits as `LD B, D` (1 byte).  Net win iff
     the loop body has ≥ 1 dec-and-branch (always true by
     construction).

  3. **Worst case**: B is genuinely unavailable for both vregs (some
     other long-running B-live value forced both alternatives).  My
     pass still inserts the COPY but greedy may put `%new`
     elsewhere too.  Net cost: 1 byte for the unmotivated COPY.
     This case is rare and the cost is bounded.

## What the fix does NOT solve

  - **Cross-call DJNZ**: B is caller-saved.  When a loop body has
    a CALL, the counter cannot live in B across the call; this is
    a separate issue (#94's "no DJNZ across CALL" sub-case) and
    needs a different intervention.

  - **i16 counters**: 16-bit DJNZ doesn't exist; #99 handles the
    BC-vs-HL ping-pong for 16-bit countdowns.

  - **#38**: a different regalloc problem (IY allocation under
    high pressure produces wrong code).  Not addressed here.

## Implementation marker

Filed as **#94** for the implementation issue.  The investigation
itself (this doc) closes #98.

## Implementation prototype + second-order finding

A prototype of the proposed pass was written this session
(`Z80SplitDjnzCounters` MachineFunctionPass, ~150 LOC) and tested on
`_two_sequential_loops`.  Result:

  - With the pass + **`-regalloc=basic`**: **both** loops produce
    DJNZ.  Asm shows `LD B, D` (1 byte) at the loop2 preheader, then
    `djnz .LBB5_3` for loop2's body.  The fix works as designed.

  - With the pass + **`-regalloc=greedy`** (default at all opt
    levels): only one loop produces DJNZ.  The pass swaps which
    loop wins (loop1 instead of loop2) but doesn't make both win.

Greedy is iterating B as the first hint, the live ranges of `%new29`
and `%new30` (post-pass) genuinely don't interfere on B, and yet
greedy assigns one of them to D.  Adding `return true` to the
target's `getRegAllocationHints` (HardHints — restrict greedy to the
hint list only) does not change the outcome.  Also adding an MRI
hint via `setRegAllocationHint(NewCounter, /*Type=*/0, Z80::B)` does
not change the outcome.

The blockage is that greedy weighs **copy-elimination** (assigning
%32 the same physreg as %30 so the preheader COPY becomes a no-op,
1-byte savings) against **target hint** (B for DJNZ, ~1-byte
savings per loop body emission).  Greedy doesn't know about the
post-RA DJNZ peephole's payoff and picks the locally cheaper
option.

Pure-prototype net effect on greedy build: zero.  loop1 trades with
loop2 for which gets DJNZ but the asm bytecount is unchanged.
Therefore the prototype was reverted from this branch — landing it
without the greedy heuristic fix would be dead complexity.

The pass itself is correct and ready to land once the greedy
heuristic is also addressed.  Two paths forward for the heuristic:

  1. **Single-register class for B-only counter vregs.**  A new
     TableGen `BReg` class (just `B`) with the new counter vreg
     constrained to it.  Greedy MUST assign B (or split / spill;
     splitting would copy `%30` or `%32` to/from B as needed).
     The basic regalloc behaviour is exactly this: it has no
     copy-elimination heuristic, so it just picks B.

  2. **Custom CostPerUse for non-B 8-bit allocations on counter
     vregs.**  Override greedy's local-cost ranking for vregs
     identified as DJNZ-eligible counters.  Heavier; needs a
     target-side eviction-advisor extension.

Recommendation: take path (1) when implementing #94.  TableGen
constraint is more declarative and survives greedy's heuristics by
construction.

## Bookkeeping

  - This doc closes **#98** (investigation done).
  - The pass implementation + greedy fix tracked under **#94**
    (with the path-1 design above as the proposed approach).
  - The prototype code is in this branch's git reflog only; not
    landed.  Keys: file `Z80SplitDjnzCounters.cpp`, ~150 LOC,
    inserts `%new = COPY %old` at each DJNZ-eligible loop's
    unique non-self predecessor and renames in-loop uses.

## Test plan

  - `_two_sequential_loops` in `djnz-comprehensive.ll` flips its
    `; CHECK-NOT: djnz` to a second `; CHECK: djnz` (the test
    already prepares this transition, see lines 169-174 of the
    file).
  - Per-function size baseline: any regression must be one of
    the three cost-model cases above; investigate any net growth.
  - rcbios / cpnos-rom: byte-exact or smaller (sequential DJNZ
    loops are rare in BIOS, expected to be no-op).
  - Z80 lit suite: no regressions (the new pass is additive).
  - Clang test runner -O2 / -O0: no new FAILs (semantically
    invariant — only adds COPYs that the rewriter eliminates).
