# Session 60e (2026-05-12) — three more suboptimal-codegen issues from witness search

## Context

Continuation of the corpus-mining workflow from session 60d.  User
asked: "have you found any other things that might indicate the
compiler could be improved" and "please investigate further and
file issues".

This session ran witness searches in `clang/cpnos.lis` for several
suspected suboptimal patterns and filed the ones with concrete
reproducers.

## Issues filed this session

| # | Title | Witness | Per-site saving | cpnos impact |
|---:|---|---|---:|---:|
| 145 | post-RA "test old reg + commit dec via scratch" (8 B) could be SUB+JR NC (5 B) | `_snios_sndmsg_force` data-byte loop test+dec | 3 B | ~6 B |
| 146 | callee-cleanup epilog `pop bc; inc sp; inc sp; push bc; ret` (5 B) → `pop hl; ex (sp),hl; ret` (3 B) | `multi_exit(int,int,int)` synthetic | 2 B | ~10-20 B across rcbios + autoload |
| 147 | `or N; ld (mem),a` for single-bit memory updates not using `set n,(HL)` / `res n,(HL)` | 3 witnesses in `cpnos.lis` (cfgtbl manipulation) | 3 B per single-bit | ~7 B confirmed, 20-30 B estimated across cpnos suite |

## Investigations that did NOT lead to issues

- **C (memory-operand arithmetic forms underused)** — search for
  `ld a, (hl); op a, r` patterns in cpnos.lis found only test-only
  loads (`ld a,(hl); or a` for zero-test, `ld a,(hl); ld d,a` for
  save).  No `ADD A,(HL)` / `OR A,(HL)` etc. opportunities exist
  in this specific code; CP (HL) is already firing where applicable
  (5 uses).  No file.

- **E (`ex de, hl; ld a, l` redundancy)** — search found 4 occurrences
  of `ex de, hl; ld de, (slot)` in `snios_sndmsg_force` /
  `snios_rcvmsg_c`.  Initial impression: the EX is dead.  Detailed
  analysis showed the EX is **required** by the calling-convention
  rules — `xport_recv_byte` is not declared `z80_preserves_regs`,
  so HL is assumed clobbered across the call, forcing a BSS spill
  of `hcs` and the slot reload via DE.  The EX is then the
  optimal way to move r from DE to HL before the slot-load
  clobbers DE.  Not a compiler issue — would be addressable by
  adding `PRESERVES_REGS_CLANG("h","l")` to `xport_recv_byte`'s
  declaration in `transport.h` (a source-level change in
  rc700-gensmedet, deliberately not pursued per the corpus-mining
  workflow).  No file.

- **F (RST-instruction optimization for hot call targets)** —
  `xport_send_byte` is called 13 times, `recv_byte_t` 12 times.
  Replacing each call with `RST n` (1 byte vs `CALL nn` 3 bytes)
  would save ~50 B across cpnos.  But this is a **linker/source-
  level placement** decision (route hot routines to RST vector
  addresses 0x08, 0x10, etc.), not a compiler-attributable issue.
  No file.  Worth keeping as a project-level optimization
  opportunity for later.

- **G (Conditional RET tail-merge for more `jr cc, X; X: ret`
  patterns)** — `clang/cpnos.lis` has 6 conditional rets already.
  Several `jr z, X` / `jr nz, X` patterns where X points to a ret
  could potentially merge.  Did not bottom out individual cases;
  marginal savings expected.  Skipping for now; may revisit if a
  clear repro emerges.

## Pattern observations

### Reproduction difficulty

Issue #145's minimal IR reduction failed: every simplified version
of the `while (k--)` loop got optimized to either DJNZ (2 B) or
`dec a; jr nz` (3 B).  The 8-byte test+dec+commit only emerges in
production functions with enough register pressure + multiple
co-resident state variables that clang commits the counter to a
specific non-A register pre-loop.  Filed the issue anyway with the
production witness + proposed `.mir` fixture.  Suggests a general
rule: **for some optimizations, IR-level repros under-represent
the production codegen surface**; backend research needs `.mir`
fixtures to exercise patterns regalloc creates only under specific
register-pressure shapes.

### Investigation pruning

Two of the four candidate areas (E, F) bottomed out as
non-compiler issues during investigation:

  - E was a missing source-level preserves_regs annotation.
  - F was a linker/placement project decision.

This is healthy — the corpus-mining workflow is supposed to
filter compiler-attributable bloat from architecturally-locked
costs.  Documenting the non-fileable findings here so future
investigators don't re-tread the same ground.

## Cumulative status

After sessions 60c, 60d, 60e the open ravn/llvm-z80 issues
attributable to cpnos-rom analysis:

| # | Subject |
|---:|---|
| 138 | #132 follow-up: liveness-driven 1B compensation |
| 139 | #132 follow-up: investigate `_snios_rcvmsg_c` BB#3 (root-caused, fix proposed) |
| 140 | #132 follow-up: `.mir` lit coverage for edge-split path |
| 141 | i16 vs 0x0100 fold |
| 142 | i8→i16 zext residual after `(uint8_t)` mask |
| 143 | #132 multi-fire interaction in edge-split |
| 144 | i16 select-on-equality materialization chain |
| 145 | test+dec+commit peephole (this session) |
| 146 | callee-cleanup epilog peephole (this session) |
| 147 | SET/RES on memory peephole (this session) |

10 actionable issues with concrete test cases — first solid batch
of the compiler-improvement corpus.

## Files touched

  - `tasks/session60e-three-more-issues.md` — this doc.
  - ravn/llvm-z80 issues #145, #146, #147 — filed.
