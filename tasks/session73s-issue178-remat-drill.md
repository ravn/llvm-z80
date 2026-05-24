# Session 73s — #178 physreg-output rematerialization: scope drill

**Date:** 2026-05-24
**Issue:** ravn/llvm-z80#178 ("Pseudos with implicit physreg outputs break rematerialization"); Tier C (mechanism-blocked).
**Outcome:** Bounded read-only drill. Confirmed the issue's premise and isolated the precise gating sub-problem. The fix is gated on a deep regalloc bug (tied-operand two-address miscompile) that needs a dedicated session; not started here.

> **SUPERSEDED (later in session 73s):** the "dedicated session" was done — the tied-operand miscompile is now fully root-caused (RegisterCoalescer assigns an out-of-HLI-class physreg to the tied def in the base-reuse pattern; the BC fallback's undeclared HL clobber corrupts unrelated values). See `session73s-issue178-add16-tied-rootcause.md` for the 5-line repro, MIR evidence, and why the obvious patches fail.

## The mechanism (confirmed)

`ADD_HL_rr` and its siblings define their 16-bit result as an *implicit physreg* (`Defs = [HL, FLAGS]`) with an **empty `OutOperandList`**. Greedy's rematerializer only clones instructions that define a *vreg* it can rewire to a new use point, so `isAsCheapAsAMove`/`isReMaterializable` on these pseudos are silently ignored (confirmed in 73p: setting them on `ADD_HL_rr` produced byte-identical AES output).

## Affected pseudos (audit)

Physreg-def 16-bit value producers that would be remat candidates if SSA-shaped:
- `ADD_HL_rr`, `SUB_HL_rr`, `SADD_HL_rr`, `ADD_HL_rr_CO`, `SUB_HL_rr_BO`, `ADC_HL_rr_CIO`, `SBC_HL_rr_BIO` (Z80InstrInfo.td:1351+)
- `LD_HL_a16` / `LD_HL_nnind` (load HL from absolute address)

(The bulk of `Defs=[...HL...]` hits in the .td are *real* hardware instructions — `ADD_HL_BC`, `INC_HL`, `LDIR`, etc. — which legitimately define HL as a physreg and are not the remat target. The remat target is the small set of pre-RA pseudos above.)

## The SSA-shaped template already exists

`ADD16_tied` (Z80InstrInfo.td:1367) is exactly the Path-A shape the issue proposes:
```
def ADD16_tied : Z80Pseudo<(outs GR16:$dst), (ins GR16:$src, GR16_BCDE:$rhs)> {
  let Constraints = "$dst = $src";
  let Defs = [FLAGS];          // HL is the SSA $dst, not an implicit physreg def
}
```
So Path A is structurally viable — the template is in tree. It does **not** yet carry `isReMaterializable`, and crucially it is **not emitted by ISel**.

## The gating blocker (precise)

All 5 `ADD16_tied` references in `Z80InstructionSelector.cpp` are inside a comment block (lines 2926–2943) recording two failed wire-up attempts on the (uncommitted) `session-73p-issue166-add16-tied` branch:

1. `$dst` class `GR16` (BC/DE allowed): the expansion's BC/DE-through-HL fallback clobbered HL without declaring it in `Defs` -> 13/13 AES FAIL.
2. `$dst` class `HLI` (HL/IX/IY only, fallback removed): still miscompiles -> 13/13 AES FAIL + test-runner FAIL. Root cause **not isolated**; "appears to be a regalloc/two-address interaction with tied operands on a narrow physreg class when BaseReg lands outside HLI, **and the miscompile corrupts values UNRELATED to the ADD16_tied output**."

The "unrelated-value corruption" signature is the hallmark of a subtle two-address-expansion / liveness bug, not a simple lowering mistake. Until that is isolated and fixed, `ADD16_tied` cannot be emitted, so remat for the HL-producing pseudos stays blocked.

## Recommendation / next step

#178 (and its dependent #166) are gated on **one** sub-problem: isolate the tied-operand two-address regalloc miscompile that corrupts unrelated values when `ADD16_tied` is emitted with `BaseReg` outside HLI. Concrete next-session plan:
1. Re-create the minimal `ADD16_tied`-at-`G_PTR_ADD` emission (HLI `$dst` variant, attempt 2).
2. Reduce to the smallest AES function that FAILs; `-print-after-all` around TwoAddressInstructionPass + regalloc.
3. Diff the MIR at the point the unrelated value is clobbered — the corruption point identifies whether it's TwoAddr expansion, the tied-operand coalescing, or the GR16/HLI class interaction.

This is a focused deep-regalloc session, not a tail-of-session task. The remat flags + per-pseudo SSA conversion are mechanical *once* the tied-operand bug is fixed.

## Files touched
None (read-only drill). Evidence: this writeup.
