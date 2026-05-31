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

## Follow-ups (same session)

### CI: runtime-tests job (test tiers now both CI-gated)
CI previously ran lit only; the whole `z80-utils/test-runner` suite was local-only
(no `z88dk-ticks` on the runner).  Added a `runtime-tests` job (`.github/workflows
/z80-ci.yml`) that builds `z88dk-ticks` (z88dk checkout incl. the `uthash` submodule
+ flex/bison) and `clang`/`lld`/`llvm-objcopy`/`llvm-nm`/`llvm-size`, then runs
`cargo run --release -- clang`.  CI restricted to the **main branch** only.  The
two-tier model (lit = primary CI gate; runtime fixtures = additional, now also
CI-gated) is documented in CLAUDE.md + AGENTS.md.

### ZX0 reclaim: cpnos PROM1 2031 -> 2029 B (and the +1 #205 had cost)
**Finding (user-flagged "cpnos prom has grown"):** #205 was byte-identical
UNCOMPRESSED (init.bin 605 B, `_init_hardware` 86 B, resident payload 2016 B) but
cost **+1 byte after ZX0** (cpnos PROM1 2030 -> 2031; init.zx0 528 -> 529).  Cause:
the intrinsic lowering emitted the seed store before the `LD DE,dst` LDIR-dest load,
and that byte order compresses 1 B worse than the pre-intrinsic memcpy lowering's
DE-first order.  (My original "#205 cpnos byte-identical" claim was wrong -- I'd
checked only the resident payload, not the ZX0-compressed `init` region.)

**Fix:** emit `LD DE,dst` before the seed (`Z80LegalizerInfo`), restoring DE-first
order.  Functionally identical; ZX0 now compresses it better than even pre-#205 --
**cpnos PROM1 2031 -> 2029 B** (the corrected `LD BC,$22` also compresses 1 B better
than the old buggy `$24`).  Net session effect on cpnos: **-1 B vs pre-#205**.

### Reversed (HL) fill-seed peephole (experimental, default OFF)
`-z80-reverse-fill-seed`: rewrite a K=2 fill seed into a reversed (HL) byte store
that lands HL on the base, folding away the separate source load (-1 B UNCOMPRESSED
per site).  VAL may be constant or symbol; symbol support wired the long-unused
MO_LO/MO_HI operand flags through `Z80MCInstLower` -> Addr16_Low/High relocations
(first user; `ld.lld` resolves them, cpnos IVT links + boots).  **Default OFF,
adopted by no target:** the reversed seed compresses WORSE under ZX0 (cpnos PROM1
2032 -> 2033 when forced), so the uncompressed -1 B is a net +1 on compressed PROMs;
the win only exists on uncompressed targets, none of which currently have a K=2
fill (AES has zero LDIR fills).  Kept as correct, tested infra + the reusable
byte-half symbol-store lowering.  Tests: `issue-205-reverse-fill-seed.ll` +
`test_205_reverse_fill_seed.c`.

**Lesson:** on ZX0-compressed PROMs, fewer UNCOMPRESSED bytes can mean MORE
compressed bytes -- always measure the post-compression artifact, and byte adjacency
/ repetition matters as much as instruction count.

## #197 verifier red-surface (later in the same session)

Swept `-verify-machineinstrs` over the clang testcases at {O2,O0}×{+static-stack,none}.
Two classes of "Using an undefined physical register":

**Class 1 — `AND_A` undef `$a` (production, FIXED).** The byte-XOR i16 EQ/NE compare
-> `AND A; SBC HL,rr` peephole (`Z80LateOptimization.cpp`) already proves A dead after
the branch (`isRegDeadAfter`) but built the carry-clear `AND A` without marking its
`$a` read undef -> verifier abort (test_98 @ -O2 +static-stack; and cpnos has the same
pattern in a loop compare).  Fix: mark the `$a` read undef.  **Production
`-verify-machineinstrs` is now clean at every shipping opt level (-Oz/-Os/-O2
+static-stack).**  Oracle: lit 150+4 (new `issue-197-and-a-undef-loop-compare.ll`,
distilled from test_98 walk_three_buffers), diff-opt-full 824/0/0, AES 13/13, cpnos
polypascal boot + **byte-identical** (disasm proven identical with/without the fix),
autoload 1658 B + BIOS 5911 B byte-identical.  Metadata-only (`setIsUndef`).

**Class 2 — `PUSH_HL` undef `$hl` (O0 only, NOT fixed; documented).** Initially
hypothesised as the IX/SP-relative spill-borrow saves; built + applied a reaching-def
`saveBorrowHL`/`emitBorrowHLSave` to those paths -> it cleared **zero** O0 cases (0
`IMPLICIT_DEF` fired).  Root cause: the failing `PUSH_HL` are NOT spill borrows -- they
are pre-PEI O0 **struct-copy / address-computation borrows** (`PUSH HL; PUSH IX; POP HL;
...; EX DE,HL`), spread across many sites, where O0's coarse liveness leaves HL undef.
A broad, multi-source **O0-liveness-precision** problem (the liveness lists a backend
check would trust are themselves unreliable at O0), not a single backend bug, and
upstream-adjacent.  O0 is **not a shipping config** (production is -Oz/-Os).  Reverted
the ineffective + production-path-touching spill-borrow change entirely.
**Recommendation:** gate CI `-verify-machineinstrs` enforcement on production opt
levels; treat the O0 residual as a known non-production limitation / future work.

**Lesson (process):** verify the fix actually fires before declaring scope (the 0
`IMPLICIT_DEF` count exposed that Class 2 targeted the wrong code); and bound memory on
mass sweeps -- `xargs -P 8` x ~412 MB/clang OOM-crashed the machine, fixed with `-P 2`
+ per-process `ulimit -t`.
