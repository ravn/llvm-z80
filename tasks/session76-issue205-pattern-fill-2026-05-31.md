# Session 76 — #205: defined `llvm.z80.pattern.fill` intrinsic + non-rotated trip-count fix

**Date:** 2026-05-31
**Merge:** `c6a7af69614c` (`--no-ff`, main), feature `5db13dbb9250`
**Issue:** ravn/llvm-z80 #205 (UB-in-IR overlapping-memcpy fill idiom)

## Problem

`Z80LoopIdiomFill` recognised a pattern-fill loop (`for(i;i<N;i++) base[i]=v`) and
lowered it to a **seed store + overlapping `llvm.memcpy(base+K, base, K*(N-1))`**.
An overlapping memcpy is **UB in IR** — it only survived because of a `volatile`
marker (#136). The optimizer was within its rights to break it.

`llvm.experimental.memset.pattern` is *defined* but unusable here: `PreISelIntrinsicLowering`
unconditionally expands it (libcall or loop) with no target hook, reaching GISel
un-legalized → crash.

## Fix — a defined Z80 target intrinsic

`llvm.z80.pattern.fill(ptr dst, iN pattern, i16 K, i16 count)`
(`IntrinsicsZ80.td`). Target-specific ⇒ opaque to generic passes (no InstCombine
exploitation, no PreISel loop-expansion). Lowered in `Z80LegalizerInfo` to the
classic **seed store + forward LDIR** (`BC = K*(count-1)`); SM83 (no LDIR) unrolls
to `count` defined stores.

## Three bugs folded in

### 1. Non-rotated trip-count over-run (the headline #205 bug + the cpnos regression)

`getSmallConstantTripCount` counts executions of the **exiting** block. That equals
the store-block count only for a **rotated** loop (latch exits). For a **non-rotated**
while/for (header exits, store in the latch) it returns store-count **+ 1**, so the
fill wrote one pattern too many (`K=2`: `BC = K*N` not `K*(N-1)`).

**Where the cpnos bytes went** (the user's "find out where the bytes went"): my first
attempt bailed the idiom entirely on non-rotated loops. cpnos's IVT-init fill
`for(n=18;n;--n) *ivt++=isr_noop` (a non-rotated 2-block loop) then degraded to a
**scalar store loop** — `_init_hardware` 86 → 94 B (the entire +11 B cpnos PROM1
regression). The proper fix maps the trip count to N **topologically**:

| topology | rule |
|---|---|
| rotated (latch is the unique exit; incl. single-block) | `N = TripCount` (any block runs TripCount times) |
| non-rotated 2-block (header exits, store in latch) | `N = TripCount - 1` |
| store in header of a non-rotated loop / other | **bail** to generic opt |

Result: `_init_hardware` back to 86 B **and** the over-run is gone — baseline wrote
19 IVT entries via `ld bc,$24`; now the correct 18 via `ld bc,$22`. Pure correctness
win at zero size cost.

### 2. K=3 `i24` store crash

Assembling the pattern into an exact `iK*8` width gave an `i24` for K=3, which the
backend can't store as one MI ("unable to legalize G_STORE s24"). Fix: assemble into
a **power-of-two container** (i8/i16/i32, never i24) and pass **K explicitly**; the
legalizer emits the seed as **s16 chunks + an s8 tail** (so K=2 stays a single
`LD (nn),HL`, K=3 = `LD (nn),HL` + `LD (nn),A`). A same-width `G_TRUNC` is skipped
(it's degenerate and spins the artifact combiner).

### 3. ImmArg-induced OOM (the machine-crash)

K was first marked `ImmArg<ArgIndex<2>>`. An ImmArg lowers to a **literal MI operand**
(no `.getReg()`), so `getIConstantVRegSExtVal` returned `nullopt` → `K` became a
garbage value → the seed loop `for(Off=0; Off<K; ...)` built **billions** of stores
→ OOM (the machine ran out of memory and restarted). Fix: pass K as an ordinary i16
**vreg** arg (a `G_CONSTANT`, read like `count`), plus a hard `report_fatal_error`
guard for a non-constant / out-of-range K (asserts are off in Release).

## Oracle (all green; production size-byte-identical)

- **lit** 148 PASS + 4 XFAIL — new `issue-205-pattern-fill.ll` (lowering, Z80 + SM83,
  `-verify-machineinstrs`); `loop-idiom-fill.ll` CHECKs updated for the K=3 s16+s8 seed.
- **test-runner** `clang -diff-opt -full`: **818 PASS / 0 FAIL / 0 FATAL** + 18 new
  runtime fixtures `test_205_fill_{byte,word,dword}` (K=1/2/4 × 6 opt levels) with
  in-array over-run sentinels.
- **AES** 13/13 verify PASS (09_Oz_prod_like 2555 B).
- **cpnos** PROM1 payload **2016 B** (byte-identical), polypascal MAME boot PASS
  (PRIMES 29989 → Q → E>).
- **autoload** 1658 B and **BIOS** 5911 B byte-identical to baseline (the CLAUDE.md
  "1652"/"5897" figures were stale pre-#150 numbers).

## Files

- `llvm/include/llvm/IR/IntrinsicsZ80.td` — `int_z80_pattern_fill` (4-arg, no ImmArg).
- `llvm/lib/Target/Z80/Z80LegalizerInfo.cpp` — `z80_pattern_fill` lowering: `emitSeedAt`
  legal-width seed decomposition, K from vreg, hard guard, SM83 unroll.
- `llvm/lib/Target/Z80/Z80LoopIdiomFill.cpp` — pow2 container + explicit K; topology-aware
  N mapping; header comment rewritten for the intrinsic.
- `llvm/test/CodeGen/Z80/{issue-205-pattern-fill.ll (new), loop-idiom-fill.ll}`
- `z80-utils/test-runner/testcases/clang/test_205_fill_{byte,word,dword}.c` (new)
