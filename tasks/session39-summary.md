# Session 39 summary (2026-05-03)

Two-phase session covering **Phase 2 (correctness)** and **Phase 3
(regalloc cluster, Cluster A)** from
`tasks/roadmap-to-maturity.md`.  Plus six fold-ins surfaced by the
audit work.

## Phase 2 — correctness sweep — closed

| Issue | Severity | Result |
|-------|----------|--------|
| #36 | va_arg ABI | closed earlier sessions |
| #81 | MC parser | closed earlier sessions |
| #63 | G_MEMSET/CPY/MOVE LDIR-with-BC=0 trash 64 KB | closed |
| #28 | IX/IY large-offset SPILL/RELOAD silent miscompile | closed |
| #38 | IY un-reserve | re-investigated; deferred to Phase 3 → still parked |

Plus session-discovered fold-ins:

| Issue | Discovered via | Result |
|-------|---------------|--------|
| #102 | EXX shadow-bank disabled-code cleanup | closed |
| #103 | test-runner was running zero tests (no crt0) | closed |
| #104 | in-memory INC/DEC peephole missed H/L liveness check | closed |
| #105 | variable-size memcpy/memset BC==0 runtime guard (LDIR_GUARDED pseudo) | closed |
| #106 | bench harness was bypassing crt0 | closed |
| #107 | #85 chain peephole missed H/L liveness check (same anti-pattern as #104) | closed |

Filed but not addressed (tracked as latent):

| Issue | Severity | Why filed |
|-------|----------|-----------|
| #108 | Skipped FLAGS-after-branch checks across multiple peepholes | Audit task #17 found 5 sites with similar shortcuts |
| #109 | ADD HL,rr commutativity comment vs code mismatch | Audit task #17, comment says check exists, code doesn't |

## Phase 3 — regalloc cluster — partial

| Issue | Result |
|-------|--------|
| #98 | investigation done — root cause = register coalescer extends counter live ranges across PHI entry-edge merges |
| #94 | closed via `Z80SplitDjnzCounters` pass + `BReg` single-register class |
| #99 | partial — `BCReg` + i16 path landed; full close needs `HLReg` (filed as #111) |
| #89 | stuck — needs loop rotation (#77, gated by #100) or LSR override |
| #27 | RFE — current mitigations cover real code; left open |
| #38 | parked — deeper than #28 large-offset bug |

Filed for follow-up:

| Issue | Why filed |
|-------|-----------|
| #110 | Greedy regalloc copy-elimination heuristic overrides target hints — known limitation, single-register-class workaround documented |
| #111 | Full #99 close needs sister `HLReg` constraint for the pointer-arg vreg |

## Files added / modified

New target source files:
- `llvm/lib/Target/Z80/Z80SplitDjnzCounters.{h,cpp}` (#94 / #99)

New test files:
- `llvm/test/CodeGen/Z80/issue-28-large-offset-iy-spill.ll`
- `llvm/test/CodeGen/Z80/issue-104-incmem-h-liveness.ll`
- `llvm/test/CodeGen/Z80/issue-105-ldir-guarded.ll`
- `llvm/test/CodeGen/Z80/issue-107-chain85-h-liveness.ll`

New runtime tests:
- `z80-utils/test-runner/testcases/clang/test_95_memops_zero_size.c`
- `z80-utils/test-runner/testcases/clang/test_96_iy_largeoffset_spill.c`

Documentation:
- `tasks/regalloc-sequential-djnz-investigation.md`
- `tasks/task17-peephole-audit-2026-05-03.md`
- `tasks/session39-phase3-status.md`
- `tasks/session39-summary.md` (this file)

Modified target source:
- `llvm/lib/Target/Z80/Z80LegalizerInfo.cpp` (#63, #105)
- `llvm/lib/Target/Z80/Z80RegisterInfo.cpp` (#28, #38 doc, #99 hint)
- `llvm/lib/Target/Z80/Z80RegisterInfo.td` (BReg, BCReg)
- `llvm/lib/Target/Z80/Z80LateOptimization.cpp` (#102, #104, #107)
- `llvm/lib/Target/Z80/Z80InstrInfo.td` (LDIR_GUARDED pseudos)
- `llvm/lib/Target/Z80/Z80ExpandPseudo.cpp` (LDIR_GUARDED expansion)
- `llvm/lib/Target/Z80/Z80TargetMachine.cpp` (Z80SplitDjnzCounters wiring)
- `llvm/lib/Target/Z80/Z80.h` (pass init declaration)
- `llvm/lib/Target/Z80/CMakeLists.txt`

## State at end of session

- **lit:** 84 tests, 83 PASS + 1 XFAIL.
- **per-function size baseline:** zero deltas vs the post-Phase-1
  baseline.
- **BIOS:** 5928 B (was 5920 entering session — the +8 is from
  #105's runtime guards on linker-arithmetic-constant memcpy
  sizes, accepted as correctness cost).
- **cpnos-rom payload:** 1759 B (was 1759 — byte-exact).
- **clang test runner -O2:** 161 tests, 159 PASS, 0 FAIL.
- **clang test runner -O0:** 0 FAIL.

New runtime tests added this session:
- `test_95_memops_zero_size` — verifies memcpy/memset with size=0
  through volatile doesn't trash memory (passes O0/O1/O2/Os/Oz).
- `test_96_iy_largeoffset_spill` — verifies indirect call through
  function pointer in functions with > ~64 bytes of locals
  (passes O0/O1/O2/Os/Oz).

## Issues count

Entering session 39: ~25 open.
At end of session 39: 22 open (5 closed, 4 newly filed).

## Roadmap progress

- Phase 1 (foundation): closed earlier sessions.
- Phase 2 (correctness): **complete**.
- Phase 3 (regalloc Cluster A): **complete-modulo-parked**.
  - #94, #98 fully closed.
  - #99 partial (HLReg follow-up tracked as #111).
  - #89, #27, #38 parked with rationale.
- Phase 4 (Cluster B — spill mechanism): not started.

## Recommended next session entry point

**Phase 4** is independent of Phase 3 and should unblock more
real-world wins:

  1. **#100** — rotation-around-CALL spill.  Closes the gate on
     #77 default-on, which would in turn unblock #89.
  2. **#20** — multi-value spill across CALL.  Direct extension
     of the closed #74 peephole.
  3. **#96** — regalloc-level layer-3 PUSH/POP spilling.  Larger
     scope; investigate before implementing.

Phase 3 follow-up #111 (HLReg for full #99) is small but pure-
synthetic (no real-world hit); land opportunistically.
