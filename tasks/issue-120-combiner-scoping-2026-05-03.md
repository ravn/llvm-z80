# Issue #120 — combiner scoping (session 42)

**Status:** scoping doc only.  No code landed.  Next session can
start coding from the entry points listed below.

## Summary

#120 is a multi-session migration of three post-RA peepholes
(#26, #27, #28 in `tasks/late-opt-audit-2026-05-02.md`, totalling
~230 LOC in `Z80LateOptimization.cpp`) into GISel combiners that
fix the same shapes structurally.  This doc captures the IR/MIR
patterns and the surface area for the combiner rules so the next
session can begin implementation without re-deriving the analysis.

## Existing combiner infrastructure

The Z80 backend already has a working GISel combiner:

  - `llvm/lib/Target/Z80/Z80Combine.td` — defines
    `Z80PreLegalizerCombiner` (uses `[all_combines]`) and
    `Z80PostLegalizerCombiner` (uses standard combines plus one
    custom rule `z80_cross_size_copy`).
  - `llvm/lib/Target/Z80/Z80PostLegalizerCombiner.cpp` — host
    file for custom match/apply functions.  See `matchCrossSizeCopy`
    / `applyCrossSizeCopy` (lines 46-74) as the template for new
    rules.
  - `llvm/lib/Target/Z80/Z80PreLegalizerCombiner.cpp` — currently
    has no custom rules but the file exists and is wired up.

Adding a new rule requires:

  1. Define `def my_rule_matchdata : GIDefMatchData<"...">;` if
     the rule needs to pass state from match to apply.
  2. Define `def my_rule : GICombineRule<(defs ...), (match ...),
     (apply ...)>;` in `Z80Combine.td`.
  3. Add `my_rule` to the rule list of either combiner def.
  4. Implement `bool matchMyRule(MachineInstr &MI, ..., &state)`
     and `void applyMyRule(MachineInstr &MI, ..., &state)` in
     the appropriate combiner .cpp.

That's all.  No CMake or build-system changes needed.

## Pattern 1: #79 mask-roundtrip → combine `G_SEXT (G_ICMP)`

### IR shape

```llvm
%cmp = icmp ne i8 %x, %y
%m   = sext i1 %cmp to i8
ret i8 %m
```

### Current lowering (without peephole)

```asm
sub  a, l        ; A = x - y
add  a, $ff      ; carry := (x != y)
sbc  a, a        ; A = -carry = 0xFF / 0x00   <-- correct mask already
and  $1          ; \
rrca             ;  \  the redundant 5-instr identity tail
and  $80         ;   } applied because GISel sext-i1-to-i8
add  a, a        ;  /  uses the canonical (shl 7; ashr 7) idiom
sbc  a, a        ; /
ret
```

### Diagnosis

GISel lowers `icmp ne` to a compare sequence that **already produces
an i8 mask** in A (the `add a,$ff; sbc a,a` triple).  Then
`sext i1 → i8` is lowered via the canonical `(shl 7; ashr 7)`
idiom, which on Z80 expands to the redundant 5-instruction tail.

The structural fix: the i1 → i8 sext is unnecessary because the
i1 result is *already* held as an i8 mask in A by the time
selection sees it.

### Combiner rule

`G_SEXT_INREG` or `G_SEXT` of an `i1` result of `G_ICMP` →
fold the sext away.  The `G_ICMP` already produces an i1, but
the Z80 lowering of `G_ICMP` produces a full i8 mask in A.  The
sext is then formally widening i1 → i8 of a register that *is
already* the i8 mask.

Two viable approaches:

  - **(1a)** PostLegalizerCombiner rule: match
    `G_SEXT %dst:i8 = G_SEXT (G_ICMP %cmp:i1, ...)` and replace
    with `G_ANYEXT %dst:i8 = G_ANYEXT (G_ICMP %cmp:i1, ...)`.
    The anyext is a no-op when the source is already a full-width
    mask (which the Z80 ICMP lowering guarantees).  Then standard
    `cast_of_cast_combines` cleans it up.
  - **(1b)** Legalizer change: tell `Z80LegalizerInfo` to lower
    `G_SEXT_INREG` from i1 differently when the source is an
    i1 from `G_ICMP`.  More invasive; requires legalizer plumbing.

Recommend **(1a)** — single-rule, contained, mirrors the shape
of `z80_cross_size_copy`.

### Acceptance for #79 sub-task

  - Combiner rule lands in `Z80PostLegalizerCombiner`.
  - `mask-from-flag.ll` passes WITHOUT peephole #26 firing.
    Verify by removing #26 in a side branch and re-running lit.
  - Delete peephole #26 (lines 2670-2732 of
    `Z80LateOptimization.cpp`, ~63 LOC including comments).
  - rcbios bios.cim and cpnos.bin sizes unchanged.

## Pattern 2: #93 carry-roundtrip → combine count-up-to-zero

### IR shape

```llvm
loop:
  %i      = phi i8 [50, %entry], [%i.next, %loop]
  ; ... body
  %i.next = add i8 %i, -1
  %cond   = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
```

### Current lowering (without peephole)

```asm
ld   a, r        ; reload counter
add  a, 1        ; counter++ (sets carry on wrap to 0)  -- LSR-rewritten
ld   r, a        ; save back
sbc  a, a        ; \
and  1           ;  \  4-inst carry-roundtrip:
xor  1           ;  /  recovers the no-carry-then-loop bit
rrca             ; /
jr   c, loop
```

### Diagnosis

This is the symptom of a chain of issues:

  - LSR rewrites `for (i=N; i; i--)` (count-down) into count-up
    with a wrap-to-zero exit test.
  - GISel lowers `add i8 %i, -1` (which IS a decrement!) as
    `add a, 1` (count up by 1, then test for wrap to zero) instead
    of as `dec a` (which sets Z directly).
  - The `icmp ne %i.next, 0` then re-derives carry from the i1
    cmp result via the SBC/AND/XOR/RRCA chain.

### Combiner rule(s)

Two layers to address:

  - **(2a)** PreLegalizerCombiner rule: match
    `G_ICMP NE (G_ADD %x:i8, -1), 0` → `G_ICMP NE %x, 1`.  This
    converts the wrap-to-zero test into a "is the value 1?" test,
    avoiding the carry round-trip entirely.  Then the existing
    GISel selection of `dec; jr nz` handles the rest.
  - **(2b)** Alternative — Legalizer canonicalization: lower
    `add %x, -1` on i8 to `G_SUB %x, 1` so that the carry from
    SUB (not ADD's wrap-carry) drives the icmp.

Recommend **(2a)** as the first attempt — single combiner rule,
preserves current selection patterns.  If selection doesn't pick
up the simplified IR cleanly, fall back to **(2b)**.

Note: this also unblocks the LSR-related issues elsewhere — the
underlying problem is that count-up-from-(-N) is the wrong
canonical form for Z80, and a combiner that reverses LSR's
preference here is structurally aligned with what we want.

### Acceptance for #93 sub-task

  - Combiner rule(s) land.
  - `issue-93-carry-roundtrip.ll` passes WITHOUT peepholes #27 +
    #28 firing.
  - Delete peepholes #27 + #28 together (lines 2734-2898,
    ~165 LOC).  They are tightly coupled — #28 only fires when
    #27 fires.
  - rcbios bios.cim and cpnos.bin sizes unchanged.

## Verification protocol (per pattern)

Apply per sub-task — do NOT delete the peephole on the first
combiner-landing commit.

  1. Land the combiner rule.  Verify lit suite stays at 90/90.
  2. Build rcbios + cpnos-rom.  Verify sizes unchanged or
     improved.  If size delta is positive (i.e. new combiner +
     existing peephole compose to a better lowering), record but
     don't block.  If negative, root-cause before continuing.
  3. In a separate branch, remove the peephole #N.  Re-run lit.
     If lit fails, the combiner rule isn't covering all cases the
     peephole was — extend the combiner, don't restore the peephole.
  4. Re-run rcbios + cpnos-rom in the deletion branch.  If sizes
     match the combiner-only baseline (step 2), the peephole is
     subsumed.  If sizes regress, the peephole was catching cases
     the combiner missed — extend the combiner.
  5. Land the deletion as a separate commit.

Two commits per pattern: "land combiner" + "delete subsumed
peephole".  Never bundled — they need to be revertable independently.

## Estimated effort

Per the issue's L tag and refined here:

  - #79 sub-task: 1 session (rule + verification + deletion).
    Smaller because the IR pattern is local (a single
    G_SEXT ← G_ICMP edge).
  - #93 sub-task: 1-2 sessions.  Larger because the rewrite
    interacts with LSR's choice + branch-condition selection.
  - Combined deletion + verification + lit cleanup: 1 session.

Total estimate: 3-4 sessions to close #120 fully.

## Why this is upstream-shaped

Both rules are pure GISel combiner work with no Z80-specific
TableGen surface beyond the combiner rule definition itself.
They follow the precedent of `z80_cross_size_copy` (already in
the codebase).  Suitable for engagement-mode PRs to
`llvm-z80/llvm-z80` once landed locally.

## Next-session entry point

Start with **#79 sub-task**.  Concrete plan:

  1. Add `def z80_sext_from_icmp` to `Z80Combine.td`
     (matchdata + rule), included in `Z80PostLegalizerCombiner`'s
     rule list.
  2. Implement `matchSextFromIcmp` and `applySextFromIcmp` in
     `Z80PostLegalizerCombiner.cpp`.  Match shape: any `G_SEXT`
     whose source register is defined by `G_ICMP`.  Apply: rewrite
     the `G_SEXT` to `G_ANYEXT`.
  3. Build + lit (90/90 expected).
  4. Run rcbios + cpnos-rom, record any improvement.
  5. Side-branch peephole #26 deletion + re-run lit + sizes.
  6. Two commits: land combiner; delete peephole.

The same template applies to #93, with the more complex match
condition.

## Session 42 attempted implementation: BOTH rules ruled out as unsound

### What was tried

Two GISel PostLegalizerCombiner rules:

  1. `z80_sext_from_icmp`: G_SEXT %dst, %src where %src is from
     G_ICMP → G_ANYEXT.  Idea: let downstream cast_of_cast /
     identity_combines absorb the no-op widen.
  2. `z80_ashr_shl_from_icmp`: G_ASHR (G_SHL %x, 7), 7 where %x
     is from G_ICMP → COPY %x.  Idea: catch the post-legalization
     form of `sext i1 → i8` after the legalizer has expanded
     G_SEXT into the canonical shift idiom.

### Why both are unsound

Z80's `BooleanContents` is `ZeroOrOneBooleanContent`
(Z80ISelLowering.cpp:49), NOT `ZeroOrNegativeOneBooleanContent`.
The G_ICMP s8 result is therefore `0x01` / `0x00` (low bit only,
high bits zero) by IR contract — even though the Z80 instruction
selector lowers G_ICMP via the SUB/ADD/SBC sequence that
*physically* leaves a full mask in A.

The (shl 7; ashr 7) shift idiom is therefore NOT an identity at
the IR layer:

  - Input (i1 in low bit): 0x01 or 0x00.
  - SHL by 7: 0x80 or 0x00.
  - ASHR by 7: 0xFF (sign extend from bit 7) or 0x00.

It is a meaningful *widen* from 1-bit-encoded i1 to full-mask
i8.  Eliding it produces 0x01 / 0x00 instead of 0xFF / 0x00 —
which silently breaks any consumer that expects the full mask
(e.g. `kbstat = (kbtail != kbhead) ? 0xFF : 0x00;` in
rcbios/bios.c:936, where `kbstat` would be stored as 0x01 instead
of 0xFF).

### Why peephole #26 IS sound

Peephole #26 runs POST-Instruction-Selection, so it sees the
physical asm sequence chosen by the Z80 backend for G_ICMP:
`SUB; ADD a,$ff; SBC a,a` — which leaves A = 0xFF / 0x00 (full
mask) regardless of the IR-level `BooleanContents` convention.
At THIS layer, the trailing `and $1; rrca; and $80; add a,a;
sbc a,a` is genuinely an identity on the value already in A —
it conceptually truncates the i8 mask back to an i1 and re-
extends it to a full mask, both ending where they started.

The peephole exploits a *target-specific lowering invariant* that
isn't expressible at the GISel layer.

### Lessons logged

  - **A combiner cannot rely on lowering invariants.**  GISel
    runs before instruction selection.  Any "the asm sequence will
    leave a full mask" assumption is unsound at this layer.
  - **`mask-from-flag.ll` is a weak test.**  It uses `CHECK-NOT`
    on specific instructions; it does NOT verify the ABI value
    matches the C source semantics.  The unsound combiner passed
    the test but produced wrong code at non-test sites.  When the
    true rcbios/cpnos-rom builds are available, prefer them as
    the verification harness for any combiner change.
  - **The `and $1` after `sbc a,a` in pre-peephole asm is the
    `ZeroOrOneBoolean` truncation — meaningful, not redundant.**
    Future readers should not assume it's part of the redundant
    tail; it's the IR-mandated narrowing of the mask back to a
    1-bit-encoded i1 before the (shl;ashr) widen-to-full-mask.

### Reverted in session 42

Both combiner rules removed from `Z80Combine.td` and the
matching match/apply functions removed from
`Z80PostLegalizerCombiner.cpp`.  Peephole #26 source comment
updated to record the negative result.  Sizes back to baseline
(BIOS 5929, cpnos-rom 1777); lit 90/90.

### Revised paths to retire peephole #26

The peephole-migration framing in this doc was wrong: the
peephole exploits a target-specific invariant that the GISel
combiner can't access.  Three options remain:

  - **(A) Target-specific post-ISel combiner.**  After ISel has
    chosen the SUB/ADD/SBC mask sequence for G_ICMP, a Z80-
    specific MIR-level pass (running BEFORE Z80LateOptimization)
    can recognize the post-ISel pattern and rewrite it.  This is
    structurally just "Z80LateOptimization #26 moved earlier in
    the pipeline" — same logic, different location.  Not a
    structural improvement.
  - **(B) Split G_ICMP lowering into a "produce full mask" form.**
    Extend the Z80 instruction selector to lower G_ICMP into a
    pseudo whose i8 result is contractually full-mask.  Then a
    GISel combiner can soundly elide the (shl;ashr) widen because
    the producer explicitly guarantees the high bits.  This is
    larger surgery.
  - **(C) Change Z80's BooleanContents.**  If we set
    `setBooleanContents(ZeroOrNegativeOneBooleanContent)`, then
    the G_ICMP result IS the full mask by IR contract, and the
    legalizer would not emit the (shl;ashr) widen at all.  But
    this affects ALL boolean-result lowering paths target-wide;
    it is not a localized change.  Risk of broad regressions.

None of (A)/(B)/(C) is a single-session task.  Recommendation:
park #120 entirely until the broader regalloc cluster work
(#89/#27/#94/#98) is complete, then revisit options (B) and (C)
with a fuller picture of how they interact with cluster-level
changes.

Peephole #26 stays.  The session-42 attempt to retire it produced
empirical evidence of WHY it is structurally needed — that is the
deliverable.

## See also

  - `tasks/late-opt-audit-2026-05-02.md` patterns #26-#28.
  - `llvm/lib/Target/Z80/Z80LateOptimization.cpp` lines 2670-2898.
  - `llvm/lib/Target/Z80/Z80Combine.td` (existing rule template).
  - `llvm/lib/Target/Z80/Z80PostLegalizerCombiner.cpp`
    (`matchCrossSizeCopy`/`applyCrossSizeCopy` as starter template).
  - `llvm/test/CodeGen/Z80/mask-from-flag.ll`
  - `llvm/test/CodeGen/Z80/issue-93-carry-roundtrip.ll`
