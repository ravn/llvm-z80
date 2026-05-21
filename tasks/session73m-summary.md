# Session 73m — #168 SimplifyCFG cost-gate lands; #77a guards refined; SDCC gap dissected

Date: 2026-05-21.  Follows session 73l (SimplifyCFG patch revert).
Triggered by the user picking up the AES corpus parity track and asking
"are clang as fast as SDCC now?"

## TL;DR

- **#168 (cost-gated `foldTwoEntryPHINode`) landed** (commit
  `cd2a2ace8754`).  Closes the `gf_alog` / `gf_log` branchless-XOR
  pattern at the mid-end correctly.  AES `09_Oz_prod_like` clang
  2695 → **2679 B**, tstates 15.05M → 14.88M (−1.1% global), every
  `xor 27` in AES hot paths is now reached only via a taken
  conditional branch.  cpnos +5 B (2025 → 2030, 18 B free under the
  2 KB hard cap).  z80-utils test-runner 681/46/56/207 unchanged;
  cpnos-polypascal-test PASS.
- **`experiment-cpnos-prom-4k` branch merged back to main**, cpnos
  `CPNOS_PROM1_CAP` reverted to 2048 B.  Workspace bumped to llvm-z80
  `cd2a2ace8754` + rc700-gensmedet `71c131f`.
- **AES "as fast as SDCC?" → No, still −19% on speed**.  clang
  `09_Oz_prod_like` 2679 B / 14.88M ts vs SDCC `01_baseline_prod`
  3323 B / 12.08M ts — clang **19% smaller** but **19% slower**.
- **Why** dissected via per-instr counting on `gf_alog` and
  `aes_mc_inv`: residual gap is **A-register churn from regalloc**,
  not pattern recognition.  Tracked by **#77** (8-bit `dec r` flag use),
  **#89** + **#27** (regalloc cluster).  #168 closes only the
  SimplifyCFG portion of #167's "+38% per-iter" headline (the
  branchless-XOR slice, ~10-14 ts of 24 ts).  Post-fix comment
  posted on #167 with the decomposition.
- **#77a loop rotation guards landed** (commit `5118ca7b97b7`),
  default still off.  Attempted the default-on flip with two new
  guards (CALL-skip + min-trip-count) but AES `01_baseline_Oz`
  regressed +11% ts (size recovered, speed didn't).  Per-iter
  math on `gf_alog` shows the rotated form *is* 4 ts cheaper,
  so the +11% is in another function (`aes_mixColumns` /
  `aes_mc_inv` suspected) interacting with LICM/CSE.  Diagnosed
  but not isolated.  Comment posted on #100 with measurements and
  the cleaner alternative path (post-RA `ld a,r; or a; jr nz` →
  `dec r; jr nz` peephole — surgical, no LICM interaction risk).

## Why SDCC is still 19% faster

Side-by-side `_gf_alog` inner loop, post-#168 fix:

**SDCC (~65 ts/iter):**
```
ld   b, c; dec c; inc b; dec b; jr Z, end     ; 23 ts loop test
ld   b, a; add a, a; bit 7, b; jr Z, skip     ; 23 ts shift + bit-7
  xor  a, 0x1b                                ; 7 ts (when bit 7 set)
xor  a, b; jr loop                            ; 16 ts close
```

**clang post-#168 (~89 ts/iter, +24 ts / +37% per iter still):**
```
ld   a, c; dec a; ld e, a; ld a, c; or a; jr z, end  ; 27 ts (extra reload)
ld   a, d; add a, a; ld h, a; ld a, d; rlca; jr nc   ; 27 ts (CF re-derive)
  ld   a, h; xor 27; ld h, a                         ; 15 ts vs SDCC 7
ld   a, h; xor d; ld d, a; ld c, e; jr loop          ; 28 ts (16 ts A-churn)
```

Three stacked causes, attributable per row:

| Cause | Per-iter ts | Open issue |
|---|---:|---|
| A-shuttle for 8-bit ALU (close: `ld a,h; xor d; ld d,a`) | ~12 | #27 / #89 |
| No flag-using `dec r` on non-A counter | ~8 | #77 |
| Re-derive CF via `rlca` instead of fall-through from `add a,a` | ~4 | #89 scheduling |

The empirical AES speedup from #168 alone was ~1.1% (167 K of 15 M ts).
The remaining ~18% is the bottom three rows — **regalloc, not
SimplifyCFG**.

## #168 implementation

12 lines added to `llvm/lib/Transforms/Utils/SimplifyCFG.cpp`, inside
`foldTwoEntryPHINode` after the `Cost` accumulation loop:

```cpp
if (TTI.getPredictableBranchThreshold().isZero() &&
    Cost > TargetTransformInfo::TCC_Free)
  return Changed;
```

Threshold notes:
- `Cost > TCC_Basic` (the obvious first try) doesn't fire for
  `gf_alog`'s single XOR because `Cost = 1 = TCC_Basic`.
- `Cost > TCC_Free` (the landed form) fires for any non-free
  speculation — exactly the right behavior for Z80 where every
  branch is cheap (no misprediction) and the speculated work is
  never amortized.

Earlier `672f24188ca8` (blanket bailout `if (TTI....isZero())
return false`) regressed cpnos +23 B; the cost gate above is +5 B
on cpnos.  Both refactor at the same callsite; the difference is
the Cost threshold.

## Loop rotation #77a investigation

Attempt: flip `Z80LoopRotate` default from off → on with two new
guards.

Guards added (both stay in tree even with default off):

1. **CALL-skip** — skip loops whose body contains a non-debug call.
   Mirrors #100 Option 4.  Rotated loops with a CALL inside force
   regalloc to BSS-spill the loop carrier across the call.
2. **Min-trip-count** — skip when `SE.getSmallConstantMaxTripCount(L)`
   is known and below `Z80LoopRotateMinTripCount = 8`.  Empirical
   break-even on AES corpus: 3-iter loop in `aes_expandEncKey`
   regressed +214 B (-Oz baseline).

With both guards + default on:

  - cpnos: −4 B
  - AES `09_Oz_prod_like`: 0 B / **−2.2% ts**
  - AES `07_Oz_no_lsr`: −11 B / **−5.9% ts**
  - AES `01_baseline_Oz`: −8 B but **+11.0% ts** ← blocker
  - AES `05_Oz_static_stack`: 0 B / **+11.8% ts** ← blocker

Size guard worked (recovered the +295 B baseline regression).
Speed didn't.  `gf_alog` per-iter math shows rotated form is
~4 ts cheaper, so the +11% slowdown lives in another function.
Suspect `aes_mixColumns` or `aes_mc_inv` where LICM/CSE-hoisted
header invariants don't recoalesce with the duplicated entry
guard.  Not isolated this session.

Default stays off; guards land regardless because they make
opt-in (`-mllvm -enable-z80-loop-rotate=true`) safer.

## Cleaner alternative for #77

A post-RA peephole in `Z80LateOptimization.cpp`:

  Match: `ld a, r ; or a ; jr nz, label`  (3 instructions)
  Predecessor with `dec r` whose flags reach this point unclobbered.

  Replace with: `dec r ; jr nz, label`  (2 instructions, ~5 ts faster).

This is **surgical**: only fires where the redundant flag rederive
exists, no loop-structure change, no LICM interaction.  Likely
closes a healthy chunk of the residual AES gap on top of #168.

Filed as a comment on #77 (this session's follow-up note).

## Issues touched

  - **#167** closed (logically; not by literal "Closes" magic in
    #168's body — the body says `Refs #167`).  Comment posted with
    per-iter decomposition.
  - **#168** "Closes #168" in commit `cd2a2ace8754`; GitHub didn't
    auto-close (issue is in another fork's ref?).  Need a manual
    close.
  - **#100** comment posted: Option 4 measurement + the speed
    regression's not-isolated status + peephole alternative recommendation.
  - **#77** to receive a comment recommending the peephole route
    (cleaner than the rotation route which is gated by #100).

## Verification

  - llvm/test/CodeGen/Z80/: 104 PASS + 3 XFAIL (unchanged baseline).
  - z80-utils test-runner clang suite: 681 / 46 / 56 / 207 (unchanged).
  - cpnos-polypascal-test: PASS 51.69 s (within historical band).
  - AES corpus 13/13 PASS at every config.

## Files changed

  - `llvm/lib/Transforms/Utils/SimplifyCFG.cpp` — cost gate (+12 lines)
  - `llvm/lib/Target/Z80/Z80LoopRotate.cpp` — guards + documentation
    (+77 lines)
  - `llvm/test/CodeGen/Z80/issue-167-branchless-conditional-xor.ll` — XFAIL test (added earlier session)
  - `llvm/test/CodeGen/Z80/issue-77a-loop-rotate.ll` — commentary refresh

## Commits

  - `cd2a2ace8754` (llvm-z80 main) — SimplifyCFG cost-gate
  - `5118ca7b97b7` (llvm-z80 main) — Z80LoopRotate guards
  - `dec28de` (rc700-gensmedet experiment branch) — cpnos cap revert
  - `efb1ee3` / `423c88c` (workspace) — experiment-branch merge + submodule bump
  - rc700-gensmedet `71c131f` — experiment merge to main

## Difficulty: Medium

Medium because the diagnosis was clean (per-instr counting on
`gf_alog` made the regalloc-vs-SimplifyCFG split obvious) and the
landed patches were tiny (12 + 77 lines).  The rotation default-flip
attempt produced data not insight; the +11% baseline regression
remains undiagnosed and would have been Hard to chase further this
session, so it was correctly handed off as a comment on #100.
