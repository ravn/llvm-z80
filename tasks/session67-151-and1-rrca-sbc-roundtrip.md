# Session 67 (2026-05-13) — #151 sbc/and-1/rrca/sbc round-trip elimination

## Context

Seventh closure from the corpus-mining batch.  After #144 (G_SEXT_INREG
width=1 i16 direct lowering), an `(a == K) ? -1 : 0` over an icmp
whose own `SBC A,A` already produced 0xFF/0 leaves a redundant
4-instruction tail:

    sbc a,a       ; from the icmp result, A = 0xFF iff equal
    and 1         ; A = 0x01 / 0x00          \
    rrca          ; CF = bit 0, A rotated     | round-trip
    sbc a,a       ; A = 0xFF / 0x00          /

The middle three are a no-op relative to the first SBC.  The original
#144 fix left this for a follow-up post-RA peephole — this session.

## Implementation

Post-RA peephole in `Z80LateOptimization.cpp`, inserted before the
existing redundant-PUSH-AF/POP-AF section.  Matches the exact
4-instruction sequence `SBC_A_A; AND_n 1; RRCA; SBC_A_A` and erases
the trailing three.

No safety guards required: the deleted instructions only touch A
and FLAGS, and the surviving `SBC A,A` produces an A value
identical to the deleted final `SBC A,A` (both write {0x00, 0xFF}
from CF).  FLAGS state after deletion matches the original (SBC
sets all of S/Z/H/P/V/N/C identically to the new final SBC).

## Verification

  - **lit suite**: 100/100 (98 PASS + 2 XFAIL).  Stale CHECK update
    in `issue-144-i16-select-on-eq.ll` (the test comment itself
    flagged the round-trip as "tracked in a follow-up issue").
  - **z80-utils test-runner** clang: byte-identical failure set
    vs baseline A/B (681 PASS / 46 FAIL / 56 FATAL — all
    pre-existing #136 noise; rule `feedback_ab_before_blaming_test_runner`
    applied).
  - **cpnos-rom clang/pio-irq payload**: 1858 B (unchanged).  The
    peephole doesn't fire in cpnos because i8 booleans narrow
    earlier in legalization, so the AND/RRCA/SBC triple never
    materialises post-RA.  The lit fixture demonstrates the fix on
    the i16 form.
  - **cpnos-polypascal-test**: PASS on both pio-irq and sio cells.

## Files touched

  - `llvm/lib/Target/Z80/Z80LateOptimization.cpp` — new peephole
    (~30 LOC).
  - `llvm/test/CodeGen/Z80/issue-151-and1-rrca-sbc-roundtrip.ll`
    — new fixture.
  - `llvm/test/CodeGen/Z80/issue-144-i16-select-on-eq.ll` — CHECK
    update for the compounded fire (rrca + and 1 now expected
    absent; new tail `sub 1; sbc a,a; ld e,a; ld d,a`).
  - `tasks/session67-151-and1-rrca-sbc-roundtrip.md` — this doc.

## Ninja rebuild tracking

  | # | Trigger | Files |
  |---:|---|---:|
  | 1 | First peephole version | 5 |
  | 2 | Stash baseline (no peephole) | 2 |
  | 3 | Restore peephole | 2 |

No full rebuilds.

## Cumulative state

  - Session 61 closed #141 — cpnos 1904 -> 1878 B (-26 B)
  - Session 62 closed #142 — cpnos 1878 -> 1866 B (-12 B)
  - Session 63 closed #144 — cpnos 1866 B unchanged
  - Session 64 closed #147 — cpnos 1866 -> 1860 B (-6 B)
  - Session 65 closed #149 — cpnos 1860 B unchanged
  - Session 66 closed #148 — cpnos 1860 -> 1858 B (-2 B)
  - Session 67 closed #151 — cpnos 1858 B unchanged

**Cumulative cpnos: -46 B (1904 -> 1858).**

7 closed, 7 corpus issues + follow-ups remain (#138, #139, #143,
#145, #146, #150, #152).

## Rules-checked

  - `feedback_compiler_bug_test`: lit fixture demonstrates fix.
  - `feedback_test_before_fix`: test failed pre-fix, passes post-fix.
  - `feedback_no_commit_first_version`: full value oracle satisfied
    including A/B test-runner baseline.
  - `feedback_value_oracle_all_transport_cells`: both pio-irq and
    sio polypascal cells PASS.
  - `feedback_polypascal_stage1_flake`: applied (`_kill-mpm` between
    cells).
  - `feedback_ninja_clang_llc_together`: rebuilt clang AND llc.
  - `feedback_ab_before_blaming_test_runner`: stash baseline +
    rebuild + rerun confirmed the 46+56 test-runner failures are
    pre-existing #136 noise, not introduced by #151.
