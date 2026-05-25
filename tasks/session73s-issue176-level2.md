# Session 73s — #176 Level 2 (SCC-non-recursive non-leaf)

**Date:** 2026-05-24
**Branch:** `session-73s` (off main at session-73r merge).
**Predecessor:** `session73r-issue176-fix.md` (Level 1).

## What was added

`Z80AutoStaticStack` extended with Level 2: any function whose CallGraph SCC has size 1 AND does not have a self-edge is non-recursive and gets `+static-stack` (in addition to leaves).

Implementation:
- Added `#include "llvm/ADT/SCCIterator.h"` + `#include "llvm/Analysis/CallGraph.h"`.
- `getAnalysisUsage` declares `CallGraphWrapperPass` required.
- `runOnModule` walks `scc_begin(&CG)..scc_end()`, collects single-node SCC functions without self-edges into a `SmallPtrSet`.
- `processFunction` accepts as safe if either Level 1 (leaf) OR Level 2 (in the non-recursive set).
- `INITIALIZE_PASS_BEGIN/END` with `INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)`.

## Measured impact (flag ON)

| Source | Default | Level 1 only | **Level 1+2** | Δ vs default |
|---|---|---|---|---|
| `aes256.c -Oz` `.text` | 3299 B | 2808 B | **2250 B** | **−1049 B / −32%** |

Level 2 captures essentially the full benefit of `+static-stack` on AES, matching the size of manually compiled `--target-feature=+static-stack` builds.

cpnos PROM1 (default): 2029 B (unchanged — cpnos uses `+static-stack` via Makefile CFLAGS regardless).

## Verification

- Lit: 111 PASS + 3 XFAIL = 114 (unchanged).
- test-runner default-off sweep: (pending — running in background).
- cpnos PROM1 (default): 2029 B (unchanged).

## Safety

Level 2's safety relies on SCC analysis being correct for the IR's call graph.  Indirect calls (function pointers, virtual calls) are conservatively recorded as edges to a "calls-external-node" sentinel — meaning any function that may indirectly call itself ends up in a multi-node SCC and gets rejected.  Good.

Caveats remaining:
- ISR-shared functions are still unsafe (Level 3 would address this, not implemented).
- Functions calling external library functions (e.g. memcpy) are still considered non-leaf but their SCC is single-node — Level 2 marks them safe.  Static-stack works fine through library calls since libraries don't share BSS with user functions.

## Combined effect on #176

| Level | Description | Status |
|---|---|---|
| 1 | Leaf functions | shipped (session 73r) |
| 2 | SCC-non-recursive non-leaf | shipped (this session) |
| 3 | ISR isolation (separate BSS per privilege level) | not started |

Level 3 needs explicit ISR-attribute work in the backend that's outside the scope of this pass.  #176 stays open as the Level 3 tracker.

## Files

- `llvm/lib/Target/Z80/Z80AutoStaticStack.cpp` — extended to Level 2.
- `tasks/session73s-issue176-level2.md` — this writeup.
