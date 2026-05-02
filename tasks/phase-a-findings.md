# Phase A findings — code-density plan revision

**Date:** 2026-05-02 (same day as plan).
**Branch:** `session-36-code-density-plan` in llvm-z80.
**TL;DR:** Phase B as originally written has **no target**.  The
"load-after-store-same-addr" pattern I cited as diagnostic is mandated
by C `volatile` semantics in `_isr_crt`; the same scan over all BIOS
asm finds **zero** such patterns at non-volatile addresses.  Plan
pivots directly to Phase C (regalloc cluster) + D (remat framework).

## What I measured

Built clang BIOS fresh (5920 B, matches CLAUDE.md headline).  Took
the four-line shape `ld hl,(nn); op; ld (nn),hl; ld hl,(nn)` from
`_isr_crt` as a representative diagnostic.  Wrote a perl scanner that:

1. Strict 4-line match over the whole BIOS disasm finds 3 occurrences
   (`$fffc`, `$ffdf`, `$ffe1`).
2. Looser scan with a 6-line window across `hl`/`bc`/`de`/`a` and
   excluding cross-CALL pairs finds the same 3.

All three addresses are members of `WorkArea` declared
`volatile word` (bios.h:268-278).  The reload after store is required
by C semantics — an ISR may modify the location between the store and
the next read.

## Why this kills Phase B

The reload-after-store at volatile addresses is **correct codegen, not
missed optimization**.  SDCC must produce the same number of accesses
for the same C source.  No peephole or MIR-level pass should remove
these loads, because doing so would be a miscompile.

The non-volatile case scan: `0` matches.  This means either (a) the
existing pipeline already eliminates non-volatile dead loads
correctly, or (b) the BIOS source happens not to write the pattern.
Either way: there is no aggregate saving available here for clang.

## What this implies for the plan

| Phase    | Original status                    | Revised status |
| -------- | ---------------------------------- | --- |
| **A**    | Instrumentation                    | partial — see below |
| **B**    | MachineDCE / load-store forwarding | **REMOVED** (no target) |
| **C**    | Regalloc cluster                   | unchanged — still the right next step |
| **D**    | Rematerialization framework        | unchanged |
| **E**    | GISel combiner audit               | unchanged |

Phase A's MIR-dump harness (A.2) is no longer needed for the dead-load
diagnosis (there's nothing to diagnose).  The other Phase A items are
still useful:

- A.1 (per-function size baseline tracker): still high-leverage.  We
  need a CI-level guard so the next Phase C/D regression doesn't slip
  through unmeasured.
- A.3 (categorize existing peephole layer in `Z80LateOptimization.cpp`):
  still useful for understanding which existing late-opt patterns are
  legitimately Z80-ISA-specific vs candidates for migration upstream.

## Where the bytes actually go (revised diagnosis)

Re-examining `_rwoper` (263 B, 105 B BSS-access, 35 reg-reg moves)
with the new lens:

- BSS-access bytes split between **volatile** (timers, dirty flags)
  and **non-volatile** statics (`cpm_sector`, `cpm_disk`,
  `cpm_track`, `hostbuf_*`, `disk_error`, `write_type`, etc.).
- Volatile portion is mandatory.  Cannot be reduced by codegen.
- Non-volatile portion is mostly **independent accesses**, not
  store-reload pairs.  Reducing it requires holding more values in
  registers across the function, which is the regalloc cost-model
  question (Phase C), not a peephole.
- One specific shape worth investigating: at the join block following
  `wrthst()` (the CALL site), `cpm_disk` and `cpm_sector_as_host` are
  reloaded from BSS — but only the post-CALL path needs the reload;
  the two non-CALL paths reaching the same join could in principle
  keep the values in `e`/`d`.  That's tail-duplication or
  jump-threading territory, not a peephole.  Real opportunity, ~8 B
  in `_rwoper` alone, but generalizes only modestly.

In `_specc` (676 B, 208 B BSS-access, 58 reg-reg) the BSS targets are
mostly non-volatile state (`xflg`, `usession`, `locbuf`, `locad`,
`adr0`, function-local statics for the escape-sequence state machine).
Same diagnosis: independent accesses, not redundant reloads.

In `_isr_crt` (166 B, 80 B BSS-access) the BSS access is dominated by
volatile timers — almost all mandatory.  This explains the 48% BSS-
access ratio: the function is structurally a chain of volatile word
counters.  Cannot meaningfully shrink without source restructuring
(e.g., grouping timers into a shared 16-bit decrement helper).

## Pivot

Phase B is removed.  Plan moves directly to Phase C (regalloc cluster)
as the next executable step.  Phase A.1 (size baseline tracker) is
still useful and can run in parallel with the Phase C investigation.

The methodological lesson: I picked `_isr_crt` for Phase A.2 because
it had the highest BSS-access ratio (48%), but high BSS ratio in an
ISR is a *structural* signal of mandatory volatile access, not a
*defect* signal.  Better starting point would have been a non-ISR
function with high BSS access AND non-volatile state (e.g.,
`_specc`).  Recording this in the plan revision so the next session
doesn't repeat the same diagnostic mistake.

## Action items

1. Write a per-function size baseline tracker (Phase A.1).  Output:
   sorted `(function, bytes)` CSV plus diff-against-baseline mode.
2. Read #94, #95, #98, #89, #99, #100 in one sitting; produce a
   Phase C investigation doc tying their root cause together.
3. Skip MIR-dump harness (Phase A.2) — no longer load-bearing.
4. Update the main plan file to reflect this pivot.
