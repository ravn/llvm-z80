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
- **Default-on decision = NO-GO** (measured 2026-07-12). Triplet: autoload
  +12 B, cpnos +9 B, rcbios byte-identical. Production has no flat `base[i]`
  beneficiaries (loops are `pc++`-walking or nested→declined), so enabling the
  pass only adds pin/preheader overhead. Pass STAYS opt-in; production
  byte-identical with it off. Details in the plan's "DEFAULT-ON DECISION"
  section.
- Beneficiary re-grounding (the tm caveat above) — user chose to SKIP; do not
  claim per-program wins for tm.
- `tryEliminateOldIV` did not fire on the guarded repro (old counter survives
  for the exit test) — secondary LFTR opportunity, separate from the base reload.

## COST GATE LANDED (2026-07-12, commit ffc4867c)

Made the pass **cost-aware** so a size-minimum build never picks the larger
form. The gate: only rewrite a group when `canEliminateOldIV` holds — i.e. the
loop's exit test can be rephrased as a pointer compare so the old integer IV
dies, dropping the loop back to {pointer[,stride]} on Z80's 3 GP pairs. A
rewrite that leaves the old IV alive was the common thread in every regressor.

**Refactor:** `matchOldIV` (pure predicate; now matches BOTH the LSR post-inc
`icmp %kn,N` and the `-disable-lsr` phi-direct `icmp %k,N` exit shapes) +
`canEliminateOldIV` (the gate, run before any mutation via `erase_if` on the
group list) + slimmed `tryEliminateOldIV` (mutator). Hatch
`-z80-loop-instr-form-prep-no-cost-gate` re-exposes the ungated behaviour.

**Production triplet byte-identical under the cost gate** (prep-only AND
prep+pin), measured with the CURRENT clang:

| target   | OFF baseline | cost-gated ON | delta |
|----------|--------------|---------------|-------|
| autoload | 2035 B       | 2035 B        | 0     |
| cpnos    | 2013 B       | 2013 B        | 0     |
| rcbios   | 5918 B       | 5918 B        | 0     |

(The earlier NO-GO's +12/+9 were the UN-gated pass. The transient +3/-1 were a
stale 2010/2035 baseline from an older clang — re-measuring OFF with the
current clang gives 2013/2035, i.e. the gated pass changes nothing.)

**-O2 sieve regression root-caused to `-z80-pin-loop-pointer`, NOT the prep
pass.** Isolated by running each flag alone through the corpus sweep:

| flags               | sieve size  | sieve speed (-O2) |
|---------------------|-------------|-------------------|
| OFF (baseline)      | 198/3204513 | 261/3498167       |
| prep only (gated)   | 198/3204513 | 261/3498167       | ← byte-identical
| pin only            | 198/3204513 | 269/4087139       | ← +17% ts
| prep + pin          | 198/3204513 | 269/4087139       |

`Z80PinLoopPointer` is a *separate* pre-RA machine pass that pins the walking
pointer to HL. Its doc-comment reasoning was for the `-disable-lsr` shape; at
-O2 with LSR on, LSR has already built its own pointer IV and pin perturbs a
sound allocation. So **prep (cost-gated) is the production-safe half and a
default-on candidate; pin STAYS opt-in** as the sole -O2 regressor.

**Tests reworked:** `guarded` → simple eliminable guarded fill (prep-only;
exercises on-demand preheader + cost gate; walking pointer in DE, no `ld
hl,_arr` reload in body). `nesting-gate` → declined nested inner loop (base
reload kept) + a flat positive control (rewritten), prep-only, dropping the
brittle pin-flagged NESTED hatch. Full Z80 lit **189 PASS + 5 XFAIL**, 0
unexpected.

**Honest scope:** the cost-gated pass produces ZERO measured wins on real
corpus/production code (their loops are `pc++`-walking, nested→declined, or
non-eliminable) — it only strength-reduces synthetic eliminable shapes. So
"cost-aware" here means "safe: byte-identical in production, wins only where it
provably helps". That makes prep default-on-able without risk, but not
value-adding on the current firmware.
