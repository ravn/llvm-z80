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

## Session 42 partial implementation: #79 combiner LANDED, peephole #26 NOT YET RETIRED

Steps 1-3 of the next-session plan above were completed in the
same session.  Combiner rule `z80_sext_from_icmp` added to
`Z80Combine.td`; `matchSextFromIcmp` / `applySextFromIcmp`
implemented in `Z80PostLegalizerCombiner.cpp`.  Build clean; lit
90/90; sizes unchanged with both combiner + peephole active.

Step 5 (verification by side-branch deletion) revealed that the
combiner does NOT yet subsume peephole #26:

  - With peephole #26 disabled: BIOS 5929 → 5943 (+14 B);
    cpnos-rom 1777 → 1781 (+4 B) [actually cpnos.bin "non-padding"
    metric was unchanged at 1777, but per-function `payload.elf`
    showed +27 B].  8 regression sites, 2 improvement sites.
  - Regression sites match those that #89 Path 2 also flagged:
    `_erase_to_eol`, `_erase_to_eos`, `_delete_line`,
    `_bg_clear_from`, `_bios_conin`, `_bios_reads_body`,
    `_impl_const`, `_bios_write_c`.  Magnitudes (+4/+5/+7/+8) match
    the 8-byte tail this peephole catches, suggesting these sites
    produce the SBC-roundtrip asm via IR shapes other than
    `G_SEXT (G_ICMP)`.

Conclusion: the combiner correctly handles the canonical lit case
(`mask-from-flag.ll`) but the production codebase has additional
IR shapes (likely `G_ASHR (G_SHL 7)` standalone, or i1 results
flowing through copies/phis) that produce the same asm pattern.
Peephole #26 catches the asm pattern regardless of IR shape;
combiner is strictly weaker.

**Action taken (session 42):** combiner LANDS as foundation work;
peephole #26 STAYS as catch-all backstop.  Source comment on
peephole #26 updated to reference this finding.  Code is
structurally cleaner (one IR shape now closes structurally), but
the peephole-deletion goal of #120 is not yet met.

**Future-session work for full #79 retirement:**

  - Compile a regressed function (e.g. `_erase_to_eol`) with
    `-print-after=z80-postlegalizer-combiner` and identify the
    actual IR shape feeding the SBC-roundtrip.
  - Extend the combiner match to cover that shape too.  Likely
    candidates: `G_ASHR (G_SHL %x, 7), 7` standalone (canonical
    sext-i1-to-i8 idiom not coming through G_SEXT); `G_SEXT
    (G_PHI)` where the phi merges G_ICMP results; copy-through
    cases.
  - Re-run the side-branch verification.  Repeat until disabling
    peephole #26 leaves rcbios + cpnos-rom unchanged.
  - Then land peephole #26 deletion as the second commit.

Estimated additional work: 1-2 sessions to fully retire peephole
#26.

## See also

  - `tasks/late-opt-audit-2026-05-02.md` patterns #26-#28.
  - `llvm/lib/Target/Z80/Z80LateOptimization.cpp` lines 2670-2898.
  - `llvm/lib/Target/Z80/Z80Combine.td` (existing rule template).
  - `llvm/lib/Target/Z80/Z80PostLegalizerCombiner.cpp`
    (`matchCrossSizeCopy`/`applyCrossSizeCopy` as starter template).
  - `llvm/test/CodeGen/Z80/mask-from-flag.ll`
  - `llvm/test/CodeGen/Z80/issue-93-carry-roundtrip.ll`
