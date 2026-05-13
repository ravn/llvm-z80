# Session 68 (2026-05-13) — #152 SET/RES via LD A,(HL) preserves intervening A-readers

## Context

Eighth closure from the corpus-mining batch.  Extends #147's
SET/RES n,(HL) peephole to fire when intervening insns READ A
(without writing it) by inserting a 1 B `LD A,(HL)` after the
address load to keep A's value reachable for the readers.

## Implementation

In `Z80LateOptimization.cpp`, the SET/RES peephole previously
bailed on ANY intervening A-touch (read or write).  Now:

1. Track `HadAReader` during the intervening-scan; tolerate reads
   of A, bail on writes.
2. Replace the conservative `mayLoad/mayStore` check (which
   misfires on Z80 register copies that spuriously carry both
   flags) with an MMO-based test: `!OpIt->memoperands_empty()`.
   This correctly accepts plain `LD r,r` copies while still
   bailing on real memory access.
3. With `HadAReader` set, only fire when `Pop == 1`.  Cost
   becomes 4 B + 2*Pop B vs 8 B:
     - Pop 1: 6 B, −2 B, −1 T  →  fire.
     - Pop 2: 8 B, ±0 B, +14 T →  skip (T-state regression).
4. Emit `LD A,(HL)` right after `LD HL,nn`; emit SET/RES at the
   OpIt position (after the intervening insns).

H/L deadness across the stretch is already guaranteed by the
existing `isRegDeadAfter(LdIt, …, H/L)` check.

## What went wrong (and was caught)

Initial implementation used `OpIt->mayLoad()||mayStore()` to bail
on memory access.  Z80 GISel register copies (`LD_D_A` etc.)
have BOTH `mayLoad=1` AND `mayStore=1` set by TableGen
conservative defaults despite being pure register-to-register
moves.  That bailed before reaching the A-reader check, so the
peephole never fired.  Caught by a tracing print; fixed by
switching to `memoperands_empty()` which respects the actual
machine memory operands.

Initial polypascal pio-irq run flaked twice in a row at two
different stages (stages 1 and 2).  `feedback_diff_binaries_…`
rule applied: stashed #152, rebuilt baseline llc, rebuilt cpnos,
`cmp -l` showed **0 byte differences** between baseline and
post-#152 payloads — confirming the flakes were environmental
(MP/M daemon state), not codegen.  Restored #152, re-ran with
`make _kill-mpm; sleep 8` cleanup → both cells PASS.

## Result

| | Pre-#152 | Post-#152 |
|---|---:|---:|
| cpnos payload | 1858 B | 1858 B (no fire) |
| Lit suite | 100/100 + 2 XFAIL | 101/101 + 2 XFAIL |

cpnos has no Pop=1 witness for this pattern; the f1ad witness
called out in the issue is 2-bit (`AND $fc`) which my gate
correctly skips.  The issue itself predicted "Not net-positive
on cpnos but completes the pattern correctly."

## Verification

  - **lit**: 101/101 (99 PASS + 2 XFAIL).  New fixture
    `issue-152-set-res-with-a-reader.ll` covers single-bit clear,
    single-bit set, and 2-bit negative.
  - **z80-utils test-runner** clang: byte-identical failure set
    vs baseline A/B (681 PASS / 46 FAIL / 56 FATAL — all
    pre-existing #136 noise).
  - **cpnos-rom clang/pio-irq payload**: `cmp -l` → 0 differences
    vs baseline (peephole has no Pop=1 witness in cpnos).
  - **cpnos-polypascal-test**: PASS on both pio-irq and sio
    cells.

## Files touched

  - `llvm/lib/Target/Z80/Z80LateOptimization.cpp` — extended
    SET/RES peephole (+~30 LOC of new conditional emission).
  - `llvm/test/CodeGen/Z80/issue-152-set-res-with-a-reader.ll`
    — new fixture.
  - `tasks/session68-152-set-res-with-a-reader.md` — this doc.

## Ninja rebuild tracking

  | # | Trigger | Files |
  |---:|---|---:|
  | 1 | First peephole version | 5 |
  | 2 | Trace print added | 2 |
  | 3 | Trace detail | 2 |
  | 4 | mayLoad/mayStore → memoperands_empty + trace remove | 5 |
  | 5 | Stash baseline | 5 |
  | 6 | Restore #152 | 5 |

No full rebuilds.

## Cumulative state

  - Session 61 closed #141 — cpnos 1904 → 1878 B (−26 B)
  - Session 62 closed #142 — cpnos 1878 → 1866 B (−12 B)
  - Session 63 closed #144 — cpnos 1866 B unchanged
  - Session 64 closed #147 — cpnos 1866 → 1860 B (−6 B)
  - Session 65 closed #149 — cpnos 1860 B unchanged
  - Session 66 closed #148 — cpnos 1860 → 1858 B (−2 B)
  - Session 67 closed #151 — cpnos 1858 B unchanged
  - Session 68 closed #152 — cpnos 1858 B unchanged

**Cumulative cpnos: −46 B (1904 → 1858).**

8 closed, 6 corpus issues + follow-ups remain (#138, #139,
#143, #145, #146, #150).

## Side-quest: #143 attempted, reverted

#143 (cross-MBB BSS-spill second-fire layout-pred bail) was
attempted earlier this session.  Implemented Option B (reuse
existing compensation MBB) with a tight `isReuseCandidate`
check.  Code is correct by inspection, but slot-coalescing in
the current LLVM tree makes every natural two-private-fires
repro hit the `UsedElsewhere` gate FIRST, so the layout-pred
fix is inert.  Reverted to a clean state — the real fix
requires either also relaxing `UsedElsewhere` (likely with a
dominator-based check) or a hand-written `.mir` fixture that
bypasses slot coalescing entirely (overlaps with #140's
parked .mir-coverage task).  The errs() → LLVM_DEBUG cleanup
was also dropped with the revert; that stray print is still
present in the source.

## Rules-checked

  - `feedback_compiler_bug_test`: lit fixture demonstrates fix.
  - `feedback_test_before_fix`: fixture failed pre-fix, passes
    post-fix.
  - `feedback_no_commit_first_version`: full value-oracle
    satisfied including A/B test-runner baseline.
  - `feedback_value_oracle_all_transport_cells`: both pio-irq
    and sio polypascal cells PASS.
  - `feedback_diff_binaries_before_blaming_codegen`: byte-
    identical cmp -l with baseline confirmed initial polypascal
    flakes were MP/M daemon state, not codegen.
  - `feedback_polypascal_stage1_flake`: applied `_kill-mpm` +
    longer sleep between retries; resolved both flakes.
  - `feedback_ninja_clang_llc_together`: rebuilt clang AND llc.
  - `feedback_ab_before_blaming_test_runner`: A/B confirmed
    test-runner failures are pre-existing #136 noise.
