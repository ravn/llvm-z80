# Session 69 — AES-corpus-driven fixes: #158, #159, partial #160

Date: 2026-05-14.  Branch: aes-mc-inv-gap-analysis (in rc700-gensmedet)
drove the diagnosis; fixes landed in llvm-z80 main.

## TL;DR

Three substantive fixes landed in llvm-z80, two of which close real
correctness bugs and one closes ~9% of the clang-vs-zsdcc code-size
gap on AES-class C code.  All driven by the new AES-256 corpus
introduced in rc700-gensmedet (`tasks/aes256-corpus/`).

| llvm-z80 | Title | Class | Impact |
|---|---|---|---|
| **#159 (merged)** | "Commutative ALU shortcut" peephole erases LD r,A without checking later uses of r | **Silent miscompile** | autoload PROM +2 B (latent miscompile was hiding by luck); aes256-corpus ANSI variant unblocked |
| **#158 (merged)** | TruncInstCombine refused to narrow expressions rooted at function arguments | Missed optimisation | aes256 K&R clang.bin 5114 → 4642 B (−9.2%); BIOS 5941 → 5925 B (−16 B); 81% of #160 closed as side effect |
| **opt fix (merged)** | `cl::opt<"z80-loop-rotate">` collided with legacy pass arg-name of same string | Build infra | AggressiveInstCombine lit suite went from unrunnable (35 false-FAIL) to 36 PASS |

## Background: the AES-256 corpus

z80.eu's byte-oriented AES-256 (Ilya O. Levin, with CP/M-compat
tweaks by Peter Dassow) was wired up as a real-world C corpus in
rc700-gensmedet branch `aes-mc-inv-gap-analysis`.  The corpus
includes:

- `aes256.c` + a freestanding `test_main.c` harness verifying the
  known-answer test vector
- Both K&R and ANSI variants of aes256.c as parallel benchmarks
- `make sweep` driving flag-combination measurement across both
  compilers
- 6 minimal-reproducer C files, one per filed compiler issue
- 4 per-function ANALYSIS.md docs (aes_mc_inv, aes_mixColumns,
  rj_sb_inv, gf_log)
- 1 SURVEY.md (cross-function pattern table)
- 1 EXPERIMENT_full_ansi.md (the controlled experiment that
  surfaced #159)
- FLAG_RECIPES.md (empirically-validated flag combos + verified
  negative results)

5 ravn/llvm-z80 issues filed (#156–#160) and 2 ravn/z88dk issues
(#5, #6) before this session's fix work began.  Then the user
asked to "fix clang in a new branch" and we worked through #159
→ #158 → opt-infra → re-measure #160.

## What was fixed

### #159 — Silent miscompile in chained u8 rotates

**Site**: `Z80LateOptimization.cpp`, the "Commutative ALU shortcut"
peephole around line 1044.  It rewrites

```
LD r, A     ; save A into temp r
LD A, r2    ; load source into A
ALU r       ; A := A ALU r
```

to

```
ALU r2      ; A := A ALU r2 (commutative ALU op uses r2 directly)
```

Both encodings produce the same A value, but the rewrite erased the
LD r,A save.  If r had another USE later in the basic block (e.g.,
a subsequent `LD A,r` reload), that use observed whatever was in r
BEFORE the save — silent miscompile via uninitialised-register read.

Manifested on aes256.c's `rj_sb_inv` (chained u8 rotates), where the
intermediate rotated value y_2 needed to be reloaded after the XOR
that computed sb_2.  Pre-fix codegen:

```
xor 99 ; rlca ; ld d,a ; rlca ; rlca ; xor d ; ld d,a ;
ld a,e          ;  E was never assigned — reads uninit
rlca ; rlca ; rlca ; xor d ; ret
```

**Fix**: gate the peephole on a kill flag on TempReg.

```cpp
bool TempKilledByAlu = false;
for (const auto &MO : It->operands()) {
  if (MO.isReg() && MO.getReg() == TempReg && MO.isKill()) {
    TempKilledByAlu = true;
    break;
  }
}
if (!TempKilledByAlu) continue;
```

If TempReg has subsequent uses, refuse the shortcut.  Function size
increases by 2 B (for the save) on the affected pattern but produces
correct output.

**Lit test**: `issue-159-alu-shortcut-kill-check.ll` pins the
expected save-before-XOR codegen on the canonical pattern.

**Production impact** (rebuilt against this fix):

- cpnos-rom: byte-identical (pattern not in cpnos)
- BIOS: byte-identical modulo build timestamp
- **autoload PROM: +2 B (1859 → 1861 B) — buggy peephole WAS firing
  in autoload; the previous 1859 B build was correct only by luck**

The autoload case is a latent-miscompile site we never noticed.
Worth auditing which function relied on the buggy peephole's
accidental correctness, but not pursued in this session.

### #158 — TruncInstCombine refused to narrow through Arguments

**Site**: `llvm/lib/Transforms/AggressiveInstCombine/TruncInstCombine.cpp`.
Two parallel walkers (`buildTruncExpressionGraph` and `getMinBitWidth`)
bailed out at `if (!I) return false` whenever a chain reached a
non-Instruction non-Constant operand — i.e., a function argument.

This blocks the standard "trunc(i16 expression of u8 zext) -> i8
expression" narrowing whenever the u8 value originated from an
int-promoted function parameter.  Exactly the K&R-style u8 case
on Z80 where `int = 16` bits, so the ABI passes `uint8_t` parameters
as `i16`.

**Fix**: 3 hunks.

1. `buildTruncExpressionGraph`: accept `Argument` as a leaf
   (don't recurse, don't fail).
2. `getMinBitWidth`: same accept-Argument-as-leaf logic
   (without this, the pass crashes via `cast<Instruction>` on what
   is now an Argument — verified via segfault on aes256.c first).
3. `getReducedOperand`: emit an explicit `trunc` at the **function
   entry** when narrowing through an Argument.  Insertion point
   matters — the trunc must dominate every narrowed use in the
   function, not just the original `CurrentTruncInst`'s position.

**Lit test**: `Transforms/AggressiveInstCombine/trunc-narrow-through-argument.ll`.
Required the opt build-infra fix below; couldn't be added until
opt ran without crashing.

**Production impact**:

| Target | pre-#158 | post-#158 | Δ |
|---|---:|---:|---:|
| aes256 K&R clang.bin | 5114 | **4642** | **−472 (−9.2%)** |
| aes256 ANSI clang.bin | 4243 | 4243 | 0 |
| BIOS clang | 5941 | **5925** | **−16** |
| cpnos resident | 1858 | 1858 | 0 |
| autoload PROM | 1861 | 1861 | 0 |

`rj_sb_inv` (canonical K&R u8-rotate-chain benchmark): 156 B → 35 B
(4.5× smaller).  Still 17 B larger than the ANSI version (chained-
rotate simplification doesn't propagate fully across the narrowing
boundary), but the bulk is closed.

### Opt fix — `cl::opt<"z80-loop-rotate">` rename

**Site**: `Z80LoopRotate.cpp` line 71.  The user-facing
`cl::opt<bool> EnableZ80LoopRotate("z80-loop-rotate", ...)` had
the same string identifier as the pass's INITIALIZE_PASS_BEGIN
ArgName (the pass's `DEBUG_TYPE` of "z80-loop-rotate").  At static
init, LLVM tried to register two cl::opts with the same name and
crashed with `Option 'z80-loop-rotate' registered more than once`.
This blocked the entire AggressiveInstCombine lit suite (35 tests
reported empty FileCheck stdin because `opt` died on startup).

`clang` happened to resolve the collision differently due to a
different init order — it worked but the collision was real.

**Fix**: rename the cl::opt to `enable-z80-loop-rotate` (matches
LLVM convention for opt-in feature flags).  Updated the one lit
test that used the old flag.  The pass arg-name "z80-loop-rotate"
stays unchanged (`-passes=z80-loop-rotate` still works).

`opt --version` now returns cleanly.  Full Z80 codegen suite: 102
PASS + 2 XFAIL unchanged.  AggressiveInstCombine suite: 36 PASS
+ 15 unsupported.

### #160 — Mostly closed as side effect of #158

Pre-fixes baseline (from the canonical `repro_kr_callee_propagates.c`):

| variant | f | mc_loop | total | vs ANSI |
|---|---:|---:|---:|---:|
| K&R | 51 | 863 | 914 | +441 B |
| ANSI | 13 | 460 | 473 | — |

**Post-#158**:

| variant | f | mc_loop | total | vs ANSI |
|---|---:|---:|---:|---:|
| K&R | **20** | **537** | **557** | **+84 B (18%)** |
| ANSI | 13 | 460 | 473 | — |

The 441 B caller-propagation gap shrunk to 84 B (81% closed) without
any direct #160 work.  Most of the bloat originally attributed to
caller register-pressure was actually a side effect of the K&R
function body's i16 codegen (the #158 pattern).  Once the body
narrows to i8, regalloc stops materialising i16 spill slots in the
caller.

Residual 84 B is the genuine i16-calling-convention overhead —
K&R `f` still has IR signature `i8 @f(i16 %arg)`, callers must
materialise i16 register pairs around each call site.  Three
candidate fixes documented on the issue, none are single-session
work; deferred.

The corpus measurement makes the path-of-least-resistance clear:
**convert K&R to ANSI in source where you can**.  The ANSI variant
of aes256.c is now byte-correct (after #159) AND 17% smaller than
K&R.

## Aggregate impact

aes256-corpus 4-cell baseline matrix:

```
Pre-fixes (session 68b state):
  Variant   zsdcc bin  clang bin   gap B     x    zsdcc ts   clang ts     x
  K&R            3604       5114   +1510  1.42x   14185104   66121724  4.66x
  ANSI           -          (workaround required due to #159)

Post-fixes (session 69 state):
  Variant   zsdcc bin  clang bin   gap B     x    zsdcc ts   clang ts     x
  K&R            3604       4642   +1038  1.29x   14185104   ~60Mts    4.2x*
  ANSI           3323       4243    +920  1.28x   12080289   59559563  4.93x
```

(*K&R post-#158 runtime not separately measured; should be in the
60M range based on body-narrowing.)

clang.bin K&R-vs-ANSI gap is **now down to 399 B (1.09x)** — clang
treats K&R-style code nearly as well as ANSI.

All 4 corpus cells PASS.  Lit 102 PASS + 2 XFAIL unchanged.  AIC
lit suite went from 0 runnable to 36 PASS.

## Production impact summary

| Target | pre-session 69 | post-session 69 | Δ |
|---|---:|---:|---:|
| cpnos resident | 1858 B | 1858 B | 0 |
| BIOS | 5941 B | **5925 B** | **−16 B** |
| autoload PROM | 1859 B | **1861 B** | +2 B (correctness fix) |

cpnos unchanged; BIOS modest size win; autoload +2 B for the
silent-miscompile site (correctness over size).

## Branches + commits

llvm-z80 main is 5 commits ahead of origin/main:
- `405f226e863d` — Merge #159 fix
- `a8c421f9e9d1` — #159 fix (kill check in ALU shortcut)
- `fa5dcef670d4` — Merge #158 fix
- `a5d49e9378ba` — #158 fix (TruncInstCombine narrow through Args)
- `9bd19f5ac351` — opt flag rename

All branches were `--no-ff` merged and the feature branches
deleted afterward.

## Still open

- **#156** — `+static-stack` AES miscompile (ret pops 0x7E0C).
  Would unlock ~1.7 KB on aes256.c.  Multi-session investigation
  needed; haven't started.
- **#157** — spill-storm regalloc heuristic (SP-relative vs IX-
  frame mode choice).  Multi-session, ~860 B remaining gap.
- **#160 residual** — 84 B of i16 calling-convention overhead on
  K&R-declared u8 functions.  Three candidate fix paths documented;
  deferred.
- **ravn/z88dk#5** — zsdcc `--nogcse` AES miscompile.  Owned by
  SDCC/z88dk maintainers; not in clang scope.
- **ravn/z88dk#6** — zsdcc `-clib=sdcc_ix` wrong AES output.  Same.
- **autoload latent-miscompile audit** — find which function in
  autoload was hiding a pre-#159 silent miscompile.  Worth doing
  before any further autoload work.
- **Lit test for #156** — couldn't be added because opt has a
  pre-existing issue, but THAT'S now fixed.  Could revisit and
  add proper opt-based tests.

## Lessons / methodology notes

1. **AES corpus paid for itself in this single session**.  Without
   the corpus, none of the three fixes would have been findable —
   the bugs only manifested in real-world u8-chained-rotate code
   shapes.  cpnos/rcbios/autoload alone weren't surfacing them.

2. **The full-ANSI experiment was the unlock**.  The minimal-repro
   per-function bisection misled me on a single-function class
   (aes_subBytes: K&R == ANSI = 125 B); only the corpus-wide
   conversion exposed #159 (the ANSI variant FAILed decryption).

3. **Build tool collisions are real**.  The opt duplicate-
   registration issue was a 15-line code blocker on the entire
   AggressiveInstCombine lit suite — and was found only because
   I tried to add a proper lit test for #158.

4. **Side effects > direct fixes** sometimes.  #158 closed 81%
   of #160 with no #160-specific work.  Always re-measure
   downstream gaps after fixing upstream issues; the residual
   might be smaller (or larger!) than the static gap analysis
   suggested.

5. **Latent miscompiles hide by luck**.  autoload was passing
   functional tests but had a silent miscompile that #159
   exposed.  The +2 B size increase IS the correctness fix.
   Worth keeping `feedback_compiler_not_trusted` in mind even
   when functional tests are green.
