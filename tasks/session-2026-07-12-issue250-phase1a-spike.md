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
