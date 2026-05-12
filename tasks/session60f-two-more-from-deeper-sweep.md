# Session 60f (2026-05-12) — two more issues from a deeper sweep

## Context

User asked: "have you seen anything else indicating clang could
generate better code.  You may search your memory and look back in
time".

Re-swept cpnos.lis and earlier compiler outputs for patterns I had
noticed but not formally analysed.  Filed two more concrete issues
with reproducers.

## Issues filed this session

| # | Title | Witness | Saving | cpnos impact |
|---:|---|---|---:|---:|
| 148 | `xor $1` / `cp $ff` for A==1 / A==0xFF equality tests should use `dec a` / `inc a` when A is dead | 3 witnesses (xport_send_byte at f0e8, f104; cfgtbl check at f403) | 1 B per site | 3 B confirmed |
| 149 | i16 != -1 (0xFFFF) lowers via 8-byte cpl-based test; `inc de + or` gives 5 B | 1 witness (snios_rcvmsg_c TRANSPORT_TIMEOUT check at f1df) | 3 B per site | 3 B confirmed |

## Other patterns investigated

  - **`ld de, small_const; add hl, de`** with N ∈ {1, 2, 3}
    (potential `inc hl × N`): no witnesses with small N found in
    cpnos.lis.  Existing witnesses all have N ≥ 4 (where the
    `ld de, N; add hl, de` form is already optimal).  Not filed.

  - **`ld hl, n; ld a, (hl)`** vs direct `ld a, (n)`: no witnesses
    in cpnos.  clang's #45 "Direct addressing for constant address
    loads/stores" appears to be working consistently.  Not filed.

  - **`ld a, (mem); inc a; ld (mem), a`** vs `inc (HL) if HL is set`:
    no witnesses found (clang already uses `inc (hl)` where
    appropriate, per the existing `inc (hl)` at edc7).  Not filed.

  - **Dead-load detection** (`ld r, X; ld r, Y` consecutive): no
    real witnesses; clang's DCE catches these.  Not filed.

  - **Switch / jump-table lowering**: cpnos has 0 JP (HL) / JP (IY)
    dispatch instructions and only 2 cp-then-jz chains.  Switches
    aren't a significant code-density surface in cpnos.  Not filed.

  - **`scroll_lines` multiplication by 80 via `add hl, hl × N`**:
    9 bytes for the multiply.  Optimal for constant multiplier
    when no hardware multiply exists.  Not filed.

  - **`xor 1; jr z`** vs `cp 1; jr z` (both 4 B): equivalent cost.
    The peephole opportunity is only the `dec a` form (1 B less)
    when A is dead — captured by #148.

  - **RST-instruction placement for hot call targets**: noted in
    session 60e; project-level optimization, not compiler-attributable.

## Cumulative corpus

After sessions 60c, 60d, 60e, 60f:

| # | Subject | Saving estimate |
|---:|---|---:|
| 138 | liveness-driven 1B compensation (#132 follow-up) | ~5 B |
| 139 | slot coalescing root-causes #132 narrow surface | ~30 B |
| 140 | `.mir` lit fixture for edge-split path | (test) |
| 141 | i16 vs 0x0100 fold | ~45 B |
| 142 | i8→i16 zext residual after `(uint8_t)` mask | ~30 B |
| 143 | #132 multi-fire interaction | ~10 B |
| 144 | i16 select-on-equality bloat | ~30 B |
| 145 | test+dec+commit peephole | ~6 B |
| 146 | callee-cleanup epilog peephole | ~10 B |
| 147 | SET/RES on memory peephole | ~20 B |
| 148 | dec a / inc a for == 1 / == 0xFF | ~5 B |
| 149 | i16 != -1 cpl-based test | ~5 B |

**Total potential savings on cpnos resident if all close: ~200 B**
(currently 1904 → hypothetically ~1700).  Realistically not all
will close (some are speculative; some have multi-session
implementations).  But individual closures of even half would
significantly tighten cpnos and broader projects.

## Cross-cutting themes

After 12 issues filed, three themes dominate:

1. **i8/i16 boundary lowering** (#141, #142, #144, #148, #149 —
   5 issues, ~115 B): clang's handling of patterns that mix
   narrow values, sentinels in wider types, and equality
   comparisons consistently produces 2-3× the optimal byte count
   on Z80.  A focused i8-narrowing combiner overhaul could close
   all five.

2. **BSS spill recovery and FrameIndex liveness** (#132, #138,
   #139, #140, #143 — 5 issues, ~50 B): the cross-MBB peephole
   landed this session has narrow surface area because of slot
   coalescing.  Closing the family needs FrameIndex-lifetime
   awareness in the late-pass.

3. **Peephole pattern recognition** (#145, #146, #147 — 3 issues,
   ~35 B): specific multi-instruction patterns where simple
   sub-of-1, EX (SP), HL, SET/RES bit ops could replace longer
   forms.  Independent local fixes.

## Recommendation for next session

  - **(a) Start fixing #141** — single-peephole, clean lit fixture
    already in the issue, ~45 B expected on cpnos, demonstrates the
    end-to-end pipeline.
  - **(b) Or fix #142** — same family (i8/i16 boundary), similar
    surface area, also clean repro.
  - **(c) Or batch-implement the three "obvious peephole" issues
    (#145, #146, #147)** — all are local pattern-replace in
    Z80LateOptimization, no MIR-level changes; could potentially
    land all three in one session.

Stopping point for the corpus-mining work itself.  Further mining
on other functions (sndmsg_force, netboot_mpm, rcbios, autoload)
is available if needed but the patterns are likely to repeat the
themes above with diminishing new findings.

## Rules-checked

  - `feedback_compiler_bug_test`: each issue has a minimal
    reproducer (or production witness when minimal repro is
    elusive due to regalloc shape).
  - `feedback_no_upstream_issues`: all filed in ravn fork.
  - `feedback_extract_rules_from_time_sinks`: no new memory rules
    extracted this session.

## Files

  - `tasks/session60f-two-more-from-deeper-sweep.md` — this doc.
  - ravn/llvm-z80 issues #148, #149 — filed.
