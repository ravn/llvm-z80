# #250 lever 3 shipped — pointer-walk stack default-on at -O2 (beats dcc)

**Date:** 2026-07-13  **Branch:** `z80-issue250-lever3-shipping` (merged to main)

## Outcome

`sieve` (zcc +cpm -compiler=llvmz80, z88dk-ticks):

| config | cycles | bytes |
|--------|-------:|------:|
| clang baseline (no stack) | 33,029,994 | 7298 |
| **-O2 default (stack auto-on)** | **27,222,368** | 7308 |
| dcc reference | 27,979,152 | 1920 |

**clang now beats dcc on sieve by ~2.7% at -O2, with no flags.**

## The key finding

Lever 3 (#261, the scan-loop IV spilled to BSS) was already solved by an
existing default-OFF pass, `Z80SinkColdLoopIV`.  It sinks the scan loop's
cold-only derived IVs into on-demand recompute, freeing the register pairs that
let regalloc keep the scan IV resident.  The full five-pass stack
(form-prep + allow-nested + pin + hbf + sink-cold-iv) beats dcc.  No new
optimisation pass was needed — the work was shipping it safely.

## What shipped (this branch)

1. **Auto-enable the five #250 passes at -O2 only.**  Each `-z80-enable-…` flag
   is now three-state: explicit `-mllvm …[=false]` overrides; otherwise the pass
   fires iff `OptLevel == Default && !hasOptSize()` (== -O2).  IR passes
   (form-prep, sink-cold-iv) gate the addPass on `OptLevel == Default` and skip
   opt-size functions inside; MI passes (pin, hbf) gate inside runOn.

   **Why -O2 only** (measured):
   - **-O3 unrolls loops** into Z80's 3-pair register file — sieve balloons
     77 -> 591 instructions, 31.4M cycles, *before* our passes.  The stack can't
     help unrolled loops (it slightly worsens them), so it stays off at -O3.
   - **-Os/-Oz** (incl. the 2 KB production PROMs) must stay lean; the passes
     skip opt-size functions, so production (-Oz) codegen is **byte-identical**
     (verified: autoload PROM identical; cpnos payload byte-identical, the ≤1 B
     PROM-total wobble is embedded buildinfo, not code).

2. **hbf branch width by opt level** (user directive): -Os/-Oz all `jr`
   (compact), -O2 hybrid (`jp` hot backedge / `jr` cold exits), -O3 all `jp`.

3. **New `Z80RemoveJumpToNext`** general peephole: removes an unconditional
   branch to the layout-next block (a jump-to-fall-through).  Runs last in
   addPreEmitPass, after ExpandPseudo, catching redundant jumps from any source
   (hbf split blocks etc.) that BranchFolding ran too early to see.

## Known collateral (watch items, NOT blockers)

- At **-O2**, `Z80PinLoopPointer` also fires on LSR-created pointer walks (not
  just form-prep's), and on some simple loops (e.g. issue-177's `zero3` memset)
  it adds a `push` (register pressure).  Bounded, small, only at -O2 (production
  is -Oz).  The dcc benchmarks e/ttt/tm are unaffected (no matching loop shape).
## Net-positivity check (done 2026-07-13)

| corpus | opt | result |
|--------|-----|--------|
| compiler-comparison (fannkuch, licm_pessimize, pi, sieve, word_fill) | -O2 | **byte + cycle IDENTICAL** on/off — stack does not fire |
| AES (aes256) | -Oz | **byte-identical** (stack off at opt-size) — production unchanged |
| dcc sieve | -O2 | **34.3M → 27.2M (−21%)** — form-prep-driven |

**Verdict: the shipped -O2 default-on stack is net-neutral-to-positive.** Big win
on the dcc-sieve shape; completely neutral (byte + cycle identical) on the whole
compiler-comparison corpus AND AES. No regression across the corpora.

Why the corpus is neutral (not a bug):
- form-prep needs a SINGLE-USE byte-array address (`GEP->hasOneUse()`). The dcc
  sieve kill loop only STORES (`flags[k]=FALSE`) -> fires. The corpus sieve does
  read-modify-write (`count -= !flags[k]; flags[k]=1`) -> the GEP has 2 uses ->
  declined. Extending to read-modify loops is possible future work.
- Other corpus loops are already pointer-walked by LSR (on at -O2), so form-prep
  has nothing to add.
- AES builds -Oz -> the stack skips opt-size functions. (Forced on at -Oz it
  DOES change AES, but that is not the shipped config.)

Only regression seen anywhere: issue-177's `zero3` (a pure-store memset where
pin adds a push on the LSR walk) — an isolated lit test, not in the corpora.

## Verification done

- Full `CodeGen/Z80` lit suite green on release + asserts: **193 PASS + 5 XFAIL**.
- Asserts-build `llc -verify-machineinstrs` clean on the sieve pipeline.
- Production: autoload PROM byte-identical, cpnos payload byte-identical at -Oz.
- New lit: `remove-jump-to-next.mir`; updated pointer-iv-* / hbf / issue-177 for
  the default-on behaviour.
