# Session 2026-06-22 — Issue #217 close: Z80PatternFillRecognize dedicated-exits

## TL;DR

Closed ravn/llvm-z80#217 by:

1. **Caller-side fix in `Z80PatternFillRecognize.cpp`** — call
   `formDedicatedExitBlocks(L, &DT, &LI, /*MSSAU=*/nullptr, /*PreserveLCSSA=*/true)`
   right before `deleteDeadLoop(&L, &DT, &SE, &LI)`. The upstream
   `deleteDeadLoop` asserts `L->hasDedicatedExits()`; our pass was violating
   the contract when two sequential loops over the same array share a CFG
   edge (the deleted loop's unique exit *is* the next loop's header, which
   has a self-backedge predecessor outside the deleted loop).

2. **Reverted the generic `LoopUtils.cpp` divergence** from the original
   #182 fix (commit `6dc359f0c63c`). The fork-local loop-aware phi rewrite
   was dead code behind the upstream assert that the 4940-commit merge
   `2b971123e3bd` re-imported; restoring upstream's simpler exit-phi
   cleanup eliminates an avoidable generic-LLVM diff.

3. **New lit test** `llvm/test/CodeGen/Z80/issue-217-pattern-fill-dedicated-exits.ll`
   drives `Z80PatternFillRecognize` directly via `opt -passes=z80-pattern-fill-recognize`
   and asserts the post-rewrite IR (memset.pattern + loop2 intact).

## Why the existing #182 test missed the regression

`llvm/test/CodeGen/Z80/issue-182-deletedeadloop-phi.ll` runs `llc -O1`,
but `Z80PatternFillRecognize` is a **middle-end IR pass** registered via
`registerVectorizerStartEPCallback` and `registerPipelineParsingCallback`
— `llc` never invokes it. The test was a smoke check for a SmallVector
explosion symptom downstream; it never actually drove the pass that
contains the bug.

The new #217 test invokes the pass explicitly:

```
RUN: opt -mtriple=z80 -passes=z80-pattern-fill-recognize -S < %s | FileCheck %s
```

This is the right granularity for any future regression in this pass.

## Reproducer (from the issue body)

```c
unsigned char a[100];
void g(void) {
   unsigned short i;
   for (i = 0; i < 100; ++i) a[i] = 0;
   for (i = 0; i < 100; ++i) ++a[i];
}
```

`build-macos-asserts/bin/clang --target=z80 -nostdlib -ffreestanding -O1 -c repro.c`

Before fix: `Assertion failed: (L->hasDedicatedExits() && "Loop should have dedicated exits!"), function deleteDeadLoop, file LoopUtils.cpp, line 548.`
After fix: exit 0, object emitted.

## Oracle

- Lit: **153 PASS + 6 XFAIL** (`llvm/test/CodeGen/Z80/`), including the
  new `issue-217-pattern-fill-dedicated-exits.ll`.
- Test-runner clang suite (build-macos): **866 PASS / 0 FAIL / 0 FATAL / 256 SKIP**.
- Production byte-compare:
  - **autoload PROM**: byte-identical except banner timestamp (offset
    0x30-0x4F is `2026-06-22 HH.MM`); same hash `ea7d075`, same size
    1660 B compressed / 1948 B raw → 1481 B compressed.
  - **rcbios BIOS**: **5462 B** — matches workspace `CLAUDE.md` baseline
    exactly.
  - **cpnos-in-c prom1-lineprog**: pre-existing build failure at HEAD on
    this machine (undefined symbols `_cfgtbl`, `_cury`, `_kbd_head`,
    `_kbd_tail`, `_kbd_ring`), reproduces with and without the fix.
    Not a regression. Documented separately if needed.

## Files changed

```
llvm/lib/Target/Z80/Z80PatternFillRecognize.cpp        | +7
llvm/lib/Transforms/Utils/LoopUtils.cpp                | +14 -31  (revert)
llvm/test/CodeGen/Z80/issue-217-pattern-fill-dedicated-exits.ll | new
```

## Cross-references

- Issue: ravn/llvm-z80#217
- Original #182 fix being reverted: `6dc359f0c63c`
- Upstream merge that re-imported the assert: `2b971123e3bd`
- Existing (kept) #182 test: `llvm/test/CodeGen/Z80/issue-182-deletedeadloop-phi.ll`
- Workspace `CLAUDE.md`: rcbios baseline 5462 B (2026-06-15)
