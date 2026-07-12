# #250 Phase 1a coverage spike — root cause found, decision GO

**Date:** 2026-07-12  **Author:** Copilot (AI), executing the Phase-1a spike in
`plan-2026-07-12-issue250-pointer-strength-reduction.md`.

## Deliverable: GO/NO-GO = **GO** (clean, bounded root cause)

## The question

Why does `-z80-loop-instr-form-prep -z80-pin-loop-pointer` **not** rewrite the
`#250` byte-array base-reload under the production `-O2 -disable-lsr` pipeline,
even though the loop is Depth-1 and the address SCEV is a perfect affine AddRec?

## Repro (standalone, verified this session)

`base[i]` indexing is the M5 shape (NOT `pc++`, see caveat below):

```c
int chk_idx(unsigned char *p, unsigned char val, size_t c) {
    for (size_t i = 0; i < c; i++)
        if (p[i] != val) { boom(); return 1; }
    return 0;
}
```

Compiled `--target=z80 +static-stack -O2 -mllvm -disable-lsr`, the loop reloads
the base every iteration (the `#250` bug):

```
.LBB0_2:                 ; Depth=1
    ld  l,(ix+-2)        ; reload base p from frame
    ld  h,(ix+-1)
    add hl,de            ; hl = p + i      <-- #250 base reload
    ld  a,(hl)
    ...
```

Turning on prep+pin leaves this **identical** — no rewrite.

## Root cause (verified, not guessed)

`runOnFunctionImpl` bails at its **first** guard:

```cpp
if (!L->getLoopPreheader() || !L->getLoopLatch())
    continue;
```

The zero-trip guard (`if (c==0) skip`) makes the loop's entry a **conditional**
branch (`br i1 %c, %exit, %header`), so there is **no dedicated preheader** and
`getLoopPreheader()` returns null. The pass never even reaches `collectAddrGroups`
(whose every guard the GEP satisfies — SCEV printer confirms `%10 -> {%0,+,1}`).

### Why it only reproduces under `-disable-lsr`

- `-O2` (LSR **on**): LSR requires loop-simplified form, so the pipeline runs
  **LoopSimplify**, which inserts the preheader → prep fires. (And LSR alone
  already eliminates the reload: 0 `add hl,de` in the loop.)
- `-O2 -disable-lsr` (production): LSR is gone, and with it the only thing that
  was pulling LoopSimplify into the pre-codegen IR pipeline. Guarded loops lose
  their preheader → prep silently declines.

### Proof

`opt -passes=loop-simplify` before prep → preheader `.preheader` inserted →
prep fires (`z80.ivptr` PHI + init + next appear) → asm loses the base reload,
`bc` becomes a running pointer walked with `inc bc`:

```
.LBB0_2:                 ; Depth=1
    ld  a,(bc)           ; a = *pc   (bc walks)
    ld  l,(ix+-2)        ; hl = &val (invariant)
    ld  h,(ix+-1)
    cp  (hl)
    jr  nz,.LBB0_5
    inc de               ; i++
    inc bc               ; pc++      (no add hl,de)
    ...
```

## Fix shape (bounded, standard LLVM — for Phase 1b)

Make `Z80LoopInstrFormPrep` require loop-simplified form, the same way LSR does:
legacy pass `AU.addRequiredID(LoopSimplifyID)` + `INITIALIZE_PASS_DEPENDENCY`,
and ensure the NewPM/codegen path also has LoopSimplify ahead of it. No
band-aid; this is how every loop pass that needs a preheader states it.

## Caveat that changes the Phase-1 beneficiary story (verified)

The plan's headline non-nested targets were `tm/chkmem` and `ttt`. But the real
`dcc/tests/tm.c` `chkmem` walks a pointer in C (`*pc; pc++`), **not** `base[i]`.
Compiled standalone it produces **no base reload** — its waste is different
(`val` spilled to frame + a redundant `i` counter for the exit test). So `tm`
is unlikely to be a base-reload beneficiary; its slowness is dominated by
`malloc`'s O(n) best-fit scan (per session-2026-07-09 Finding 3). Genuine
`base[i]` beneficiaries are `sieve` (`flags[k]`) and index-shaped loops.
Phase 1b should re-confirm which real corpus/production loops carry the `base[i]`
shape before claiming per-program wins; the *mechanism* fix is sound regardless.

## Premise checks (Phase 1a asked for these)

- `chk_idx` (tm/chkmem-shaped) hot loop: **Depth=1** (verified in asm). ✓
- After a preheader exists, prep+pin on this non-nested loop rewrites cleanly
  with no spill regression in this repro (`bc` walk, old counter survives only
  for the exit test — `tryEliminateOldIV` did NOT fire here; a secondary
  opportunity, separate from the base-reload fix). ✓
- fannkuch remains Depth-3 nested (see plan §3.1) → correctly out of Phase 1.

## FIX LANDED (2026-07-12, commits e4895cd5 → 16324269)

**Coverage fix (final form, commit 16324269):** insert a dedicated preheader
**on demand** via `InsertPreheaderForLoop(L, DT, LI, nullptr, false)`, called
per loop **only after** it passes `collectAddrGroups` + `registerPressureOK`.
The first attempt (e4895cd5) instead required whole-function LoopSimplify
(`AU.addRequiredID(LoopSimplifyID)`, mirroring LoopStrengthReduce); that pulled
LoopSimplify into every function whenever the opt-in pass was enabled and
perturbed regalloc even where prep rewrote nothing — sieve regressed +8 B /
+408K t-states (extra spill slot) though its only (nested, innermost) loop was
declined. The on-demand form confines CFG mutation to loops actually rewritten,
so declined functions stay byte-identical; **sieve returned to baseline 198 B /
3204513 ts** (corpus sweep, verified). Dropped `setPreservesCFG` /
`preserveSet<CFGAnalyses>` (we now mutate the CFG) and threaded DominatorTree
into `runOnFunctionImpl` for both legacy and NewPM.

New lit test `llvm/test/CodeGen/Z80/pointer-iv-strength-reduce-guarded.ll` — a
zero-trip guarded loop under `-disable-lsr`, CHECK (pass ON) = no `add hl,de` in
the loop / OFF control = reload present. Red/green archived: pre-fix `llc` FAILs
(reload survives), post-fix PASSes. Pass stays opt-in → production byte-identical
by construction. Full Z80 lit **189 PASS + 5 XFAIL**, 0 unexpected.

### Nesting re-gate LANDED (commit 2616e242)
`registerPressureOK` declines `L.getParentLoop() != nullptr`; hidden hatch
`-z80-loop-instr-form-prep-allow-nested` re-enables. Test
`pointer-iv-strength-reduce-nesting-gate.ll` (declined nested keeps reload;
NESTED prefix w/ hatch = rewritten).

### Still open for Phase 1b (NOT done here)
- **Default-on decision.** With the collateral regression removed, the opt-in
  pass is net-neutral on the corpus (sieve non-regressed). Flipping default-on
  still needs the production triplet byte-identical validation
  (autoload/cpnos/rcbios) + MAME boot — a larger cycle; present with data, do
  not flip blind.
- Beneficiary re-grounding (the tm caveat above) — user chose to SKIP; do not
  claim per-program wins for tm.
- `tryEliminateOldIV` did not fire on the guarded repro (old counter survives
  for the exit test) — secondary LFTR opportunity, separate from the base reload.
