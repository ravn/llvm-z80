# Session 73q — Track B drill B1 (NarrowIV trio: #169, #170, #171)

**Date:** 2026-05-23
**Budget:** 30 min (per `execution-plan-2026-05-22.md`)
**Wall:** ~50 min (over)
**Outcome:** GO with strong reclassification.  The three NarrowIV-related issues are **obsoleted at HEAD by the session-73p Phase 2 TTI hooks**.  `Z80NarrowIV` is currently a no-op pass.  The "leverage" the brief promised (lift guards -> more BIOS wins) is illusory: lifting the guard does nothing because the pass's own preconditions reject every candidate it sees.

## What the brief expected

> B1. NarrowIV trio (#169, #170, #171) — all three are silent miscompiles or
> timeouts in Z80NarrowIV worked around by conservative guards.  First drill:
> re-enable the guards, capture the failing MIR for one repro, bisect.

i.e., the assumption was: guards currently bypass a real bug; lifting them re-exposes the bug; we then drill into the underlying transform.

## What actually happens

Procedure: lifted the single-phi guard in `Z80NarrowIV.cpp` (lines 287-293, comment-out the `if (Phis.size() != 1) continue;`), rebuilt clang+llc, and ran all three documented repros.

| Repro | At HEAD with guard ON | At HEAD with guard LIFTED |
|---|---|---|
| `test_94_bss_self_clear` (#170) | PASS 6/6 | **PASS 6/6** (DE=0x0007 expected) |
| `test_96_iy_largeoffset_spill` (#171) | PASS 6/6 | **PASS 6/6** (DE=0x0001 expected) |
| AES `aes256.c` -Oz (#169 corpus) | (not run; would be in corpus sweep) | asm byte-identical |

Then compared asm with `-mllvm -enable-z80-narrow-iv=false` vs default-on:

| Source | `diff` lines |
|---|---|
| `test_94_bss_self_clear.c` -O1 | **0** |
| `test_96_iy_largeoffset_spill.c` -O1 | **0** |
| `aes256.c` -Oz | **0** |

So narrow-iv produces no observable change on any of the documented inputs at HEAD.  Lifting the multi-phi guard cannot re-expose a miscompile when the pass isn't firing in the first place.

## Why the pass is dormant

Two pieces fit together:

**1. NarrowIV runs only in the legacy CodeGen-IR pipeline** (`Z80PassConfig::addIRPasses`, after `addStandardIRPasses` which includes LSR).  This is a deliberate choice — see the in-source NOTE at `Z80TargetMachine.cpp:182-186`:

```cpp
PB.registerLateLoopOptimizationsEPCallback(
    [](LoopPassManager &PM, OptimizationLevel Level) {
      if (Level != OptimizationLevel::O0) {
        // NOTE: Z80NarrowIV is NOT registered here.  LSR (which runs
        // later in the CodeGen-IR pipeline) corrupts narrowed phis.
        // Instead the legacy-PM wrapper is added in
        // `Z80PassConfig::addIRPasses` AFTER LSR.
        PM.addPass(Z80IndexIV());
      }
    });
```

So narrow-iv sees IR **after** LSR.

**2. Post-LSR loop shape doesn't match `tryNarrowPhi`'s pattern.**  Inspecting the asm for test_94's verifier loops shows LSR has rewritten them into **countdown** form:

```asm
ld    hl,16         ; counter init at trip-count
...
.loop:
  ... body ...
  inc  bc           ; pointer step
  dec  hl           ; counter step
  ld   a,l ; or h
  jr   z, .exit     ; exit on zero
```

`tryNarrowPhi` only accepts the **count-up** shape `phi i16 [ init_const_le_255, preheader ], [ add %phi, const, latch ]` with the icmp comparing the **add** against a constant ≤ 255.  The post-LSR shape is `phi i16 [ trip_count, preheader ], [ sub %phi, 1, latch ]` (or equivalent dec) with the icmp comparing the **phi** against 0 — different pattern entirely.

So every candidate is rejected before the guard at line 292 has a chance to matter.  The guard is moot.

## What changed between session 73n and HEAD

Session 73n landed Z80NarrowIV with these three issues open.  Since then:

- **#128 disablePass** (LICM/CSE/EarlyMachineLICM, 73p Phase 1) — machine-level, doesn't reach LSR's IV-form decision.
- **#179 P1/P2** (Z80ReorderTestDec, 73p Phase 1) — pre-RA MIR, post-LSR.
- **#177 TTI hooks** (73p Phase 2 partial ship, commit `541b687bbecc`): `Mul -> TCC_Expensive`, `getCastInstrCost(trunc/zext) = TCC_Free`, `prefersVectorizedAddressing = false`.  **This is the suspected cause.**  LSR consults TTI for its IV-form cost model.  With trunc/zext free, LSR no longer pays for the `zext narrow_phi to i16` materialization on the way out, so it prefers the i16 countdown form over the count-up form (which would have required the narrow-iv pass to fix).

The Phase 2 hooks effectively did Z80NarrowIV's job, but via LSR's own canonicalization rather than via an extra pass.  Z80NarrowIV is now redundant on the documented inputs.

## Options going forward (not for this drill — for the next session's plan)

**Option A — declare the three issues obsolete.**  Close #169, #170, #171 as "no longer reproducible at HEAD; root cause was LSR IV-form choice, now resolved by session-73p #177 TTI hooks."  Keep the pass; it's a no-op safety net for inputs LSR doesn't canonicalize.  Lowest risk, no further code change.

**Option B — remove `Z80NarrowIV` and `Z80NarrowIVLegacyPass` entirely.**  At HEAD the pass produces no observable change.  Removing it shrinks the maintenance surface (~330 lines).  Risk: any test we don't currently cover might rely on narrow-iv firing on a shape LSR doesn't canonicalize.  Validate against the full lit + AES corpus + test-runner before removing.

**Option C — re-fit `tryNarrowPhi` to accept the post-LSR countdown shape.**  Add patterns for `phi i16 [trip, preheader], [sub %phi, step, latch]` + `icmp eq %phi, 0`.  Potential BIOS wins beyond what LSR's countdown gives (replace `dec hl; ld a,l; or h; jr z` 4-byte sequence with `dec a; jr nz` 3-byte when only an 8-bit counter is needed).  But: most or all of these are likely already caught by the existing post-RA `DEC; LD; OR; JR` peepholes (#179 P1 in particular).  Diminishing returns.

**Option D — move `Z80NarrowIV` to run BEFORE LSR (NewPM late-loop-optimizer callback), in addition to or instead of after.**  The in-source NOTE explicitly warns against this because "LSR corrupts narrowed phis."  That note was added in session 73n based on #169's behavior — but #169 may itself have been a consequence of the LSR-IV-form choice that #177 has since changed.  Re-validate against #169's repro (AES `01_baseline_Oz`, `05_Oz_static_stack`, `08_Oz_gc_sections`) — if those configs PASS at HEAD with Z80NarrowIV moved to pre-LSR, the NOTE is also obsolete.

## Recommendation

Bias toward **Option A** (declare obsolete) for the immediate term, with **Option B** (remove) gated on a broader sweep:

1. Run the full test-runner suite (`cargo run -- clang`) with `-mllvm -enable-z80-narrow-iv=false` and confirm zero regressions vs default.  If clean, Option B is safe.
2. Run the AES corpus sweep with the flag flipped to confirm no size regression on any of 13 configs.
3. If both clean, remove `Z80NarrowIV.{h,cpp}` and the pipeline-registration calls in `Z80TargetMachine.cpp` + the legacy-PM registration block.  Close #169, #170, #171 as "obsoleted by #177."

Optionally pursue Option C as a separate, scoped follow-up if the BSS-spill / DJNZ wins from session 73p are already well-saturated and a 3-4 B / loop incremental shrink is worth a pass dedicated to it.  Not a priority.

## Drill cost vs estimate

Plan estimate was "30 min" for "highest leverage."  Actual was ~50 min, but the brief's premise was wrong: there's no MIR to capture because the pass doesn't fire.  The drill's real cost was the **rebuild + dual-config asm diff + grep through `tryNarrowPhi`**, which took 30 min once I committed to the experiment.  The remaining 20 min was the initial guard-lifting and the wrong path of expecting test_94 to fail (it didn't).

## A1 cross-check (negative)

Briefly tested whether `Z80NarrowIV` was the trigger for the #182 LoopRotate-SSA-corruption finding from drill A1.  Result: `-mllvm -enable-z80-narrow-iv=false` does NOT prevent the #182 crash.  The dual-IV shape in #182's IR was created by some other pass (most likely upstream `IndVarSimplify`).  A1 writeup needs that correction.

## Files

- Edit to `llvm/lib/Target/Z80/Z80NarrowIV.cpp` lifting the guard: **reverted** at end of drill.  `git diff` clean.
- Asm captures: `/tmp/scev182/t94.narrow_{on,off}.s`, `/tmp/scev182/t96.narrow_{on,off}.s`, `/tmp/scev182/aes.narrow_{on,off}.s` — all diff-zero between on/off.
