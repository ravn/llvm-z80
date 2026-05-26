# Session 73s cont. — #198 fix + -verify-machineinstrs surface triage (2026-05-26)

## Headline

- **#198 (real -O2 miscompile) root-caused, fixed, verified, merged, closed.**
- **Full `-verify-machineinstrs` -O2 surface triaged** into three determinate
  classes, each mapped to an issue; CI `-verify` flag landed in the test-runner.
- Stood up a reusable **assertions build** + recorded the LLVM debug-tooling kit.

## #198 — MachineCSE -O2 miscompile -> BSS-spill peephole bug (FIXED)

**Root cause (verified at MIR/pass level, not inferred):**
MachineCSE on `rj_sb_inv` was the *trigger*; the actual defect was in the
same-class **BSS-spill->PUSH/POP peephole** (`Z80LateOptimization.cpp` ~4470). Its
cross-block guard matched only the spilled pair's own opcodes
(`SI->LoadOpc`/`SI->StoreOpc`), so a **cross-class** reader of the same slot in
another block (`LD_DE_nnind` of a BC-spilled slot) slipped through. Converting the
store to `PUSH_BC` then orphaned that reader: PUSH writes the stack, not the BSS
slot, so the other block read a never-written slot -> garbage.

**Method:** `-opt-bisect-limit` binary search pinned the trigger to `machine-cse`
on `rj_sb_inv` (2032 PASS / 2033 FAIL); per-pass MIR showed `PUSH_BC` first appears
in Z80LateOptimization; before/after asm showed the orphaned `ld de,(slot)`.

**Fix:** widen the cross-block guard to any register class
(`isAnyBssLoad || isAnyBssStore`) — identical to the sibling cross-class peephole's
existing guard. The in-block orphan guard (#82) was already class-agnostic; this
makes the cross-block guard match.

**Verification (full oracle):**
- New MIR test `issue-198-cross-class-cross-block-spill.mir`: PASS with fix, **FAIL
  without** (baseline emits PUSH_BC/POP_BC + orphaned LD_DE_nnind).
- AES with MachineCSE force-enabled now PASSES (was the #198 FAIL).
- lit 118 PASS + 5 XFAIL; test-runner `clang` and `clang -static-stack` FAIL lists
  **byte-identical** to baseline; cpnos polypascal MAME boot PASS (51.6 s).
- Size (more-conservative peephole = correctness cost): cpnos PROM1 2028 -> 2036 B
  (12 B free), autoload 1473 -> 1483 B compressed, BIOS unchanged.

## `-verify-machineinstrs` -O2 surface — 171/174, three determinate classes

`clang -opt O2 -verify` FATALs 171/174 (same with/without `+static-stack`). Triaged:

1. **"Illegal virtual register" (most common) = #112/#189.** GR16NoIR-constrained
   pseudos (XOR_CMP_EQ16, `Z80InstrInfo.td:1577`) fed `gr16` by ISel; Z80NarrowNoIndex
   narrows pre-RA. Benign in default config. Clean fix = create-time GR16NoIR
   chokepoint (carries a Class-C density tradeoff).
2. **"Using an undefined physical register" = #194.** The cross-block #60 `LD A,r`
   removal (`Z80LateOptimization.cpp` ~3874, monotone A-dataflow) removes gf_log
   bb.2's redundant `LD_A_E` (A==E from bb.1) but doesn't add `$a` to bb.2 live-ins.
   Benign at runtime. Blanket `fullyRecomputeLiveIns` rejected (+2 B cpnos via
   `aes_ar_cpy` block-placement); open path = byte-neutral surgical live-in update
   (path-limited recompute def->reload block). (I filed #199 as a dup of #194 without
   searching first — closed it; hardened the grep-before-filing rule.)
3. **"Too few operands" = #200 (new).** SPILL_GR16 array/offset 2-operand
   intermediate vs 3-operand declaration. Benign. Cosmetic TableGen cleanup.

None confirmed a *new* miscompile, but #198 proved this "invalid-but-runs" family
*can* miscompile for some inputs -> clearing the classes closes latent risk.
Tracked under #197 (CI/verification hardening). Flip `-verify` to a blocking gate
once #112/#189 + #194 + #200 clear.

## Tooling stood up

- `build-macos-asserts/` (RelWithDebInfo + LLVM_ENABLE_ASSERTIONS + DUMP) for
  bug-hunting; recipe + build-size facts (~2897 ninja edges) in the tool-paths memory.
- Test-runner `clang -verify` flag (opt-in `-mllvm -verify-machineinstrs`).
- Debug-tooling kit confirmed: `-opt-bisect-limit` (pass localization),
  `llc -run-pass` (MIR tests), `llvm-reduce` (built), `-Rpass-missed`, `-print-changed`.

## Open / deferred (each a focused effort or a decision)

- **#194** surgical liveins fix (cosmetic; path-limited recompute; byte-neutrality
  plausible since the +2 B was aes_ar_cpy, not gf_log).
- **#112/#189** create-time GR16NoIR chokepoint — **density tradeoff = user decision**.
- **#200** SPILL_GR16 operand-count cleanup (cosmetic).
- **#197** flip the `-verify` CI gate once the three classes clear.
- **#38** IX/IY MIR cost model — the big density lever (independent, fresh session).
