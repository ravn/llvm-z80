# Session 38 — Phase 1 fold-ins + Phase 2 correctness landed (LDIR-BC=0)

Date: 2026-05-02 evening → 2026-05-03 (continuation of session 37 across
a `/compact` boundary; treated as a separate session because the date
rolled over and the work scope expanded substantially).
Branch: `session-37-phase-1-foundation` (still off `main`).

## TL;DR

Three issues closed by codegen / infra fixes, two new issues filed, one
new lit test, and a major correctness root cause identified.  The
roadmap's "Phase 2 #28 + #63: shared FastRegAlloc / spill-slot" guess
turned out to be wrong: #63's actual root cause was a legalizer-level
LDIR-with-BC=0 that trashes 64 KB of memory.  About one third of #28's
-O0 failures vanished with the same fix.

End-of-session sizes: BIOS **5920 B** byte-exact; cpnos-rom payload
1759 B / relocator 128 B byte-exact.  Lit suite: **79 PASS + 1 XFAIL**
(was 78 + 1 — net +1 from the new `issue-63-memset-size1.ll`).
Per-function size baseline: **zero deltas**.

## Issue ledger

### Closed this session

| #   | Mechanism                                          | Commit     |
| --- | -------------------------------------------------- | ---------- |
| 102 | Removed disabled EXX shadow-bank block (~158 LOC)  | `2c9395f6` |
| 103 | Test runner: link clang suite through crt0 + ELF compiler-rt | `31e6593d` |
| 63  | Legalizer fix: LDIR/LDDR with BC=0 trashing 64 KB  | `bbd882f2` |

(Plus a belated manual close of #81 / #101 from the prior session — auto-close
only fires on the default branch, and the work has all been on
`session-37-phase-1-foundation`.)

### Filed this session

| #   | Topic                                                       |
| --- | ----------------------------------------------------------- |
| 104 | Short-circuit `&&` / `\|\|` semantics broken at -O1+         |
| 105 | Variable-size memcpy/memset runtime guard (follow-up to #63) |
| 106 | Bench harness has same crt0 gap as clang suite (follow-up to #103) |

### Partial / still open

- **#28** — gen_z80_full at -O0 still produces a residual after the #63
  fix (200 tests → 2 failures, was 3).  About one third of failures
  came from #63; remainder is a separate -O0 root cause.  Tracked as
  task #13.

## Commits (this branch, today only — oldest first)

1. `2c9395f6` — `[Z80] remove disabled EXX shadow-bank spill block (#102)`
2. `31e6593d` — `[test-runner] link clang tests through crt0 + ELF
   compiler-rt (#103)`
3. `b0e41688` — `tasks: triage notes for runner-exposed regressions
   (post-#103)`
4. `bbd882f2` — `[Z80] fix LDIR/LDDR with BC=0 for tiny constant-size
   mem ops (#63)`

All four pushed to `origin/session-37-phase-1-foundation`.

## #103 — test runner functional, plus ~40 latent regressions surfaced

The clang suite was effectively a no-op: every test fatal'd with
`_halt symbol not found in ELF` because `run_single` invoked the driver
as `clang test.c -o test.elf` with no crt0, no linker script, and no
compiler-rt archive.  The issue's original "fix sketch" was incomplete
— even simple tests pull in `___mulhi3` / `___udivhi3` / `___mulsi3`
that have no compiled archive in the build tree.

The fix:

- New `runtime::ensure_elf` lazily assembles `compiler-rt/lib/builtins/
  {z80,sm83}/*.asm` (excluding crt0, `*_sdcc.asm`, `cpm_*`) into per-
  target object files under `<build>/lib/{triple}/elf-runtime/`.
- crt0.asm pre-assembled into the same staging dir.
- `clang.rs run_single` now compiles to .o with `-c -nostdlib
  -ffreestanding`, then explicitly invokes `ld.lld` with
  `--gc-sections`, `-T <triple>.ld`, the staged crt0.o, the test
  object, and every staged builtin object.

End-to-end result on z80, six-opt-level matrix (960 tests):

```
Total: 960  Pass: 655  Fail: 42  Fatal: 57  Skip: 206
```

Pass rate among non-skipped: **86.9%**.  See
`tasks/runner-exposed-regressions-2026-05-03.md` for the regression
buckets.  Highlights:

- 50 FATALs are test_90/91 sub-tests at -O0 only — the binaries blow
  past 64 KB at -O0 (test_90_edge_0000.c ELF = 117 KB at -O0, fits in
  16 KB at -Os).  Test infrastructure issue, not codegen.
- 6 FATALs are test_48 missing `<alloca.h>` for `--target=z80`.  Pre-
  existing test bug.
- 5 FAILs are test_18 short-circuit semantics at -O1+ — filed as #104.
- ~30 FAILs are real higher-opt regressions in test_90/91 family —
  triage parked until Phase 2 #28 residual lands.

SM83 path validated end-to-end on the same vararg test (BC=0x00FF).

## #63 — LDIR/LDDR with BC=0 trashes 64 KB

The roadmap framed #63 (and #28) as "FastRegAlloc / -O0 spill-slot".
The actual root cause is upstream of regalloc: in `Z80LegalizerInfo.cpp`,
`G_MEMSET` lowers to:

```
LD (HL), val          ; store first byte
LD DE, HL+1
LD BC, size-1
LDIR
```

When `size == 1`, `BC = 0`.  Z80's LDIR decrements BC and tests for
zero **after** the move, so BC=0 means 65536 iterations — propagating
the byte at HL forward through every byte of the 64 KB address space.
Code, stack, BSS, everything overwritten with the source byte.  At
higher opt levels, DCE folds the degenerate init away; at -O0 every
`uint8_t s[] = "";` (or any 1-byte zero-init) emitted the runaway LDIR.

`G_MEMCPY` and `G_MEMMOVE` had the parallel bug at `size == 0`.

Fix (constant-size only): special-case `Size == 0` (drop the MI
entirely) and `G_MEMSET Size == 1` (emit only the leading single-byte
store, skip the LDIR setup).  Variable-size cases are filed as #105.

New lit test `issue-63-memset-size1.ll` covers all five degenerate
shapes.  bench_string at -O0 now returns DE=0x00FF (was 0x045B / 045C).

About one third of the gen_z80_full -O0 failures vanish with this fix.
The residual (#28) is a separate -O0 issue — not the same root cause —
and is tracked as task #13.

## #102 — EXX shadow-bank cleanup

158 LOC `#if 0` block in `Z80LateOptimization.cpp` removed.  See
session-37 audit (`tasks/late-opt-audit-2026-05-02.md`) for the
analysis.  Pure janitorial; the disabled implementation had been
declared unsalvageable (EXX swaps all three pairs atomically, can't
be inserted at arbitrary points).

## What this session did NOT do

- Did not touch the #28 -O0 residual — bisecting within
  `gen_z80_full.py`'s 27 categories deferred as task #13.
- Did not touch #104 (short-circuit -O1+) — task #12.
- Did not touch #38 (IY un-reserve) — last remaining Phase 2 item.
- Did not patch the bench harness (`bench.rs`) for the crt0 gap —
  filed as #106, task #15.
- Did not implement variable-size memcpy/memset guard — filed as
  #105, task #14.
- Did not merge `session-37-phase-1-foundation` into `main`.

## Phase 2 score after this session

| Item             | Status                                     |
| ---------------- | ------------------------------------------ |
| #36 va_arg       | Closed (verification, session 37)          |
| #81 ex af, af'   | Closed (asm parser, session 37)            |
| **#63 -O0 mem**  | **Closed (legalizer, this session)**       |
| #28 -O0 large fn | Partial — residual tracked as task #13     |
| #38 IY unreserve | Untouched                                  |

3 of 5 items closed, plus one partial.

## Pickup for session 39

Highest-leverage candidates, in roadmap order:

1. **#104** (short-circuit semantics at -O1+) — task #12.  Real
   correctness regression now visible thanks to #103.
2. **#28 residual** — task #13.  Bisect within gen_z80_full to find
   the surviving -O0 root cause.  Probably a single root cause behind
   the linear scaling we still see.
3. **#106** (bench harness crt0) — task #15.  Small, mirrors #103
   pattern; would unlock the clang-vs-SDCC bench numbers.
4. **#105** (variable-size memcpy/memset guard) — task #14.  Latent;
   no production caller hits it, but worth landing for hygiene.
5. **#38** (IY un-reserve) — last Phase 2 correctness item;
   coordinate with `+undocumented` IXH/IXL handling.

After the residual Phase 2 items: optional merge of
`session-37-phase-1-foundation` into `main` with `--no-ff` per the
project's session-merge convention.
