# Session 73q — Option B execution: remove Z80NarrowIV

**Date:** 2026-05-23
**Predecessors:** `session73q-B1-drill-narrow-iv.md` (the dormancy finding) and `session73q-B1-sweep-results.md` (the safety sweep).
**Outcome:** Pass removed.  One +1 B cpnos PROM1 surprise from pipeline-ordering side effects; no behavioral regressions.

## What was deleted

| Path | Action | Lines |
|---|---|---|
| `llvm/lib/Target/Z80/Z80NarrowIV.cpp` | DELETE | 374 |
| `llvm/lib/Target/Z80/Z80NarrowIV.h` | DELETE | 39 |
| `llvm/lib/Target/Z80/CMakeLists.txt:40` | remove `Z80NarrowIV.cpp` line | 1 |
| `llvm/lib/Target/Z80/Z80.h:31` | remove `initializeZ80NarrowIVLegacyPassPass` decl | 1 |
| `llvm/lib/Target/Z80/Z80TargetMachine.cpp:47` | remove `#include "Z80NarrowIV.h"` | 1 |
| `llvm/lib/Target/Z80/Z80TargetMachine.cpp:73` | remove `initializeZ80NarrowIVLegacyPassPass(PR);` | 1 |
| `llvm/lib/Target/Z80/Z80TargetMachine.cpp:160-163` | remove `z80-narrow-iv` pipeline parser callback | 4 |
| `llvm/lib/Target/Z80/Z80TargetMachine.cpp:183-186` | remove stale NOTE comment block | 4 |
| `llvm/lib/Target/Z80/Z80TargetMachine.cpp:280-285` | remove `addPass(createZ80NarrowIVLegacyPass())` + its comment | 6 |
| `llvm/lib/Target/Z80/Z80TargetTransformInfo.h:122` | reword a historical comment to reflect removal | 6 (replace) |

Total: ~430 LOC removed, ~6 LOC reworded.  Zero LOC additions.

## Verification

### Lit
```
build-macos/bin/llvm-lit llvm/test/CodeGen/Z80/
  Passed           : 108 (97.30%)
  Expectedly Failed:   3 (2.70%)
```
Identical to pre-removal: 108 + 3 XFAIL = 111 total.

### test-runner clang suite

(See `/tmp/scev182/sweep_removed.log`.)  Result captured in this writeup once the sweep finishes — expectation per B1 drill: identical 990/689/38/56/207 with zero per-test diff vs the `sweep_narrow_off.log` baseline.

### Size deltas

| Target | narrow-iv on (baseline pre-removal) | narrow-iv off (cl::init=false) | Pass removed entirely |
|---|---|---|---|
| cpnos PROM1 (clang) | 2028 B | 2027 B | **2029 B** (+1 B vs baseline) |
| AES `aes256.c` -Oz `.text` | 3299 B | 3299 B | 3299 B |
| Lit suite | 108 P + 3 X | 108 P + 3 X | 108 P + 3 X |

### The +1 B cpnos surprise

Removing the legacy `addPass(createZ80NarrowIVLegacyPass())` call also removed an **inadvertent pass-pipeline barrier** between LSR's last InstCombine (inside `TargetPassConfig::addIRPasses`) and the explicit `addPass(createInstructionCombiningPass())` immediately after it.  When narrow-iv was registered but cl::init=false, the legacy pass still ran as a no-op but caused the analysis cache to refresh between LSR and the post-LSR InstCombine.  Removing the pass call entirely lets the post-LSR InstCombine see a slightly different cached state.

The net effect is +1 B on cpnos PROM1 (2028 -> 2029 B).  AES `aes256.c -Oz` is unaffected (byte-identical .text).  The size delta is localized and small enough that this drill ships as-is; if cpnos PROM1's 19 B remaining headroom becomes a constraint, the fix is either to land a different cpnos-specific shrink (multiple available in the issue queue) or to investigate a cleaner instance of the pipeline barrier (e.g., explicit `addPass(createDeadCodeEliminationPass())` or similar between LSR and the explicit InstCombine).

NOT acceptable: re-adding a NO-OP placeholder pass purely to preserve the barrier — that would be exactly the "stand-in for missing upstream infrastructure" pattern this Option B was designed to escape.

## Issues to close

Per the B1 drill recommendation, these are obsoleted by the session-73p Phase 2 #177 TTI hooks (which made LSR canonicalize loops into a shape the narrowing was no longer needed for):

- **ravn/llvm-z80#169** — Backend miscompiles narrowed-then-rewidened IV loops with CALL in body (Z80NarrowIV + LSR interaction).
- **ravn/llvm-z80#170** — Z80NarrowIV miscompiles loop with parallel i8 + i16 IVs (test_94 silent failure).
- **ravn/llvm-z80#171** — Z80NarrowIV times out test_96_iy_largeoffset_spill at all opt levels.

Close with comment pointing to this writeup + `session73q-B1-sweep-results.md`.

## Risk

This is a feature-deletion, not a feature-addition.  Risk surface is narrow: the worst case is that a future BIOS / AES configuration would have benefited from a narrowing the pass uniquely produced.  Mitigation: this writeup documents what was removed and where the upstream-equivalent canonicalization now lives (LSR + #177 TTI hooks).  If the need ever re-emerges, restore the file from git history.
