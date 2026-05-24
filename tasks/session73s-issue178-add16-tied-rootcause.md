# Session 73s — #178/#166 ADD16_tied: root cause isolated

**Date:** 2026-05-24
**Issues:** ravn/llvm-z80#178 (physreg-output pseudos break remat), #166 (ADD_HL_rr remat).
**Outcome:** The miscompile that blocked the ADD16_tied wire-up in session 73p ("13/13 AES FAIL, root cause not isolated, corrupts values unrelated to the add output") is now **fully root-caused** to a single mechanism, reproduced in a 5-line function. No fix shipped — the fix is a RegisterCoalescer-level change (or a non-tied remat model); the in-tree explicit `COPY-HL + ADD_HL_rr + COPY-from-HL` pattern stays. All code changes from this drill were reverted; tree is byte-identical to HEAD (comment + this writeup only).

## Why we want ADD16_tied at all

`ADD_HL_rr` defines HL as an **implicit physreg** (`Defs=[HL,FLAGS]`, empty `OutOperandList`). Greedy's rematerializer only clones instructions that define a **vreg** it can rewire, so `isReMaterializable` is silently ignored on it (confirmed 73p: byte-identical AES with the flag set). `ADD16_tied` (`(outs GR16:$dst), (ins GR16:$src, GR16_BCDE:$rhs)`, `Constraints="$dst=$src"`) is the SSA-shaped sibling — its `$dst` is a vreg, so it *could* be made rematerializable and shrink the dominant BC/DE↔HL base-reuse copy traffic that #27 identified.

## The 5-line reproducer

```c
/* base-reuse: p is used for two indexed loads; p survives the first add */
unsigned char two_idx(unsigned char *p, unsigned i, unsigned j) {
    return p[i] + p[j];
}
```
`clang --target=z80 -Oz -S -emit-llvm` → two `getelementptr` sharing base `%0` → two `G_PTR_ADD(%0, idx)`.

Diagnostic wire-up (reverted): behind a `-z80-add16-tied` `cl::opt`, lower `G_PTR_ADD` to
`%hl:HLI = COPY %base; %dst:HLI = ADD16_tied %hl(tied), %off`. `llc -mtriple=z80 -mattr=+static-stack -z80-add16-tied`.

## What the MIR shows

After ISel (correct, SSA):
```
%0:gr16 = COPY $hl            ; base p
%12:hli = COPY %0             ; HLI copy for 1st add
%4:hli  = ADD16_tied %12, %1  ; p+i
%11:hli = COPY %0             ; HLI copy for 2nd add
%7:hli  = ADD16_tied %11, %2  ; p+j
```
After virtregrewriter (the bug):
```
renamable $bc = COPY $hl                              ; base p -> BC
renamable $hl = COPY renamable $bc                    ; 1st HLI copy -> HL
renamable $hl = ADD16_tied killed $hl, killed $de     ; 1st add: dst=HL  ✓
LOAD8_IND killed $hl, implicit-def $a                 ; A = p[i]
renamable $h  = COPY $a                               ; stash p[i] in H
renamable $de = RELOAD_GR16 %fixed-stack.0, 0         ; DE = j
renamable $bc = ADD16_tied killed $bc, killed $de     ; 2nd add: dst=$BC  ✗✗✗
LOAD8_IND killed $bc, implicit-def $a                 ; A = p[j]
ADD_A_r killed $h                                     ; A = p[j] + H
```
`%7` is `hli`-classed, but regalloc gave it **`$bc`**. `expandPostRAPseudo` then takes the BC-accumulator fallback (`PUSH BC; POP HL; ADD HL,rr; PUSH HL; POP BC`). That `POP HL` **overwrites H**, destroying the `p[i]` stashed there → `ADD_A_r $h` reads `high(p+j)` instead of `p[i]`. The corrupted value (`p[i]`) is **unrelated to the ADD16_tied output** — exactly the 73p signature.

## Root cause (single mechanism)

In the base-reuse shape the pointer base dies at its **last** indexed use. There the **RegisterCoalescer** merges three intervals — the GR16 base, the inserted HLI accumulator copy, and the HLI-classed `ADD16_tied` tied-def — into one, and keeps the **base's physreg (BC)**. BC is **outside** the tied def's HLI class.

`HLI` (`{HL,IX,IY}`) is a *proper subclass* of `GR16` (verified: `HLISuperclasses` lists `GR16RegClassID`), so `getCommonSubClass(GR16, HLI)` should clamp the merged interval to `HLI` (allocatable = `{HL}` since IX/IY are reserved). The coalescer instead **widens the join to GR16**, letting an out-of-class physreg (BC) reach an HLI-constrained operand. dst=BC then triggers the fallback, and the fallback's undeclared HL borrow does the damage.

So there are two layered defects:
1. **Coalescer:** assigns a physreg outside the tied def's constrained class (the disease).
2. **Expansion:** the BC/DE fallback borrows HL without declaring it in `Defs` (a latent second bug, only reachable because of #1). Also it's 5 B vs the 1 B `ADD HL,rr` — even when *correct* it is a size regression.

## Why the obvious patches fail

- **Add `HL` to `ADD16_tied` Defs** (to make the fallback's HL borrow sound): `llc` reports *"ran out of registers"* on the repro. When dst is allocated to HL (the normal case), the implicit `HL`-def collides with the tied `HL`-def — an unsatisfiable two-address constraint. Reverted.
- **Narrow the `$dst` class** (HL-only): does not help. The coalescer escapes to GR16 regardless of how narrow the declared class is, because it uses the *source* (base) interval's class for the merged range.
- **Insert an explicit HLI copy of the base and tie to that throwaway** (this drill's ISel attempt): byte-identical bad output — the coalescer still folds the throwaway copy into the dying base and keeps BC.

## Conclusion / next step

#178 and its dependent #166 are blocked on **one** sub-problem with a now-crisp definition: *the RegisterCoalescer must honor the narrower tied-def register class when joining a tied def with a wider-classed, dying source operand* (or `ADD16_tied` must be replaced by a **non-tied** remat model so no tied-source join occurs). The first is a generic-LLVM RegisterCoalescer change (out of scope while upstream work is paused, and high-risk); the second is a backend redesign (define a remat-friendly HL-producing pseudo whose source is not tied — open question whether remat then has its inputs available at the clone point).

This is a complete drill result: the 73p "not isolated" blocker is isolated, with a one-command repro (`llc -z80-add16-tied`, re-addable in ~20 lines from this writeup) and the exact failing instruction identified. No further ADD16_tied wire-up attempts should be made until the coalescer class-clamp is addressed.

## Verification

- Functional source diff after revert: **empty** (only the in-place `Z80InstructionSelector.cpp` comment + this writeup). Codegen byte-identical to HEAD by construction.
- Z80 lit suite: **109 PASS + 5 XFAIL** (114), healthy after clang+llc rebuild.

## Files touched
- `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` — updated the in-place #166/#178 dead-end comment with the isolated root cause (no codegen change).
- This writeup.
