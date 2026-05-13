# Session 66 (2026-05-13) — #148 dec/inc-a for A==1 / A==0xFF tests (-2 B cpnos)

## Context

Sixth closure from the corpus-mining batch.  #148: equality tests
`A == 1` and `A == 0xFF` (compiled as `CP_n 1` or `CP_n 0xFF`,
2 bytes) followed by a conditional branch can use Z80's 1-byte
`DEC_A` / `INC_A` when A's modified value is dead afterwards.
`OR A` already covers `A == 0`; this closes K ∈ {1, 0xFF}.

## Implementation

Post-RA peephole in `Z80LateOptimization.cpp`, inserted between
the SET/RES peephole (#147) and the redundant PUSH-AF/POP-AF
cleanup.

Pattern: `{CP_n, XOR_n} K` followed by `{JR,JP}_{Z,NZ}` or
`RET_{Z,NZ}`, where K ∈ {1, 0xFF}.  Replaces the CP/XOR with
`DEC_A` (for K=1) or `INC_A` (for K=0xFF).

## Safety guards (this is where the work was)

Three orthogonal checks:

1. **A dead-after on fall-through** — `isRegDeadAfter(AfterBr,
   MBB, TRI, Z80::A)`.  Required because DEC_A / INC_A mutate A
   while CP_n leaves it unchanged.  XOR_n mutates differently
   (A ^ K vs A − 1), so even for the original XOR case, the new
   A value diverges on the Z=0 path.

2. **A dead-after on the taken branch** (JP cc / JR cc only).
   The branch target's first non-debug instruction must either
   (a) define A before reading it, or (b) be a `RET` with A not
   in successor liveins (covers void-returning functions).
   Implemented via a `targetDeadA` helper.

3. **FLAGS dead-after on both paths.**  Critical — CP_n sets
   the C flag (`C = A < n`); XOR_n clears C; but `DEC_A` and
   `INC_A` **leave C unchanged**.  Downstream consumers of C
   (e.g. `JR C, X; SBC A, A; RR L`) would see different flag
   state with the replacement.

   Checked via `isRegDeadAfter(AfterBr, MBB, TRI, Z80::FLAGS)`
   on the fall-through, and a target-MBB scan on the taken
   branch (must redefine FLAGS before any read).

The FLAGS check is what distinguishes this from the obvious
"just replace CP with DEC" rewrite.  Earlier iterations of the
peephole without the FLAGS check could break code shapes like:

```asm
cp   1
jr   z, equal       ; Z consumed; safe
jr   c, less        ; C consumed; my DEC A doesn't set C
                    ; → C still holds whatever it was before — wrong!
```

The 3 lit-suite regressions during initial verification (all
stale CHECK lines, not real codegen bugs) didn't exercise this
case, but real code might.

## Step-back analysis caught this

Initial implementation skipped the FLAGS check.  Lit suite
showed 3 "FAIL" results.  Per the user's explicit "step back and
analyze thoroughly first" directive, I confirmed:
  - All 3 were stale CHECK lines (test infrastructure issues),
    not real codegen regressions.
  - BUT the FLAGS-correctness concern was real for the broader
    set of patterns my peephole could fire on.
Added the FLAGS check before declaring done.

## Result

| | Pre-#148 | Post-#148 |
|---|---:|---:|
| cpnos payload | 1860 B | **1858 B (−2 B)** |
| `dec a` instances in cpnos | (some) | 19 |
| `inc a` instances in cpnos | (some) | 7 |
| Lit suite | 101/101 (after CHECK updates) | 101/101 |

Per-issue estimate was ~3 B (3 cpnos witnesses × 1 B).  Actual:
2 B.  The discrepancy may be due to some witnesses' FLAGS check
gating them out — unverified.

## Verification

  - **lit suite**: 101/101 (99 PASS + 2 XFAIL).
    - New fixture `issue-148-dec-inc-equality.ll` (3 cases: K=1
      fires, K=0xFF fires, K=2 negative).
    - Stale CHECK updates: `issue-142-i8-zext-after-mask.ll`,
      `redundant-ld-a-reg.ll` — both now accept the new
      `dec a` output via alternation `{{(cp 1|dec a)}}`.
  - **z80-utils test-runner** clang Oz: 113 PASS / 0 FAIL.
  - **cpnos-rom clang/pio-irq payload**: 1860 → **1858 B (−2 B)**.
  - **cpnos-polypascal-test**: PASS on both pio-irq and sio cells.

## Files touched

  - `llvm/lib/Target/Z80/Z80LateOptimization.cpp` — new
    peephole (~120 LOC).
  - `llvm/test/CodeGen/Z80/issue-148-dec-inc-equality.ll` — new.
  - `llvm/test/CodeGen/Z80/issue-142-i8-zext-after-mask.ll` —
    CHECK update for compounded fire.
  - `llvm/test/CodeGen/Z80/redundant-ld-a-reg.ll` — CHECK update
    for compounded fire.
  - `tasks/session66-148-dec-inc-equality.md` — this doc.

## Ninja rebuild tracking

  | # | Trigger | Files |
  |---:|---|---:|
  | 1 | First peephole version | 5 |
  | 2 | Add use-count diagnostic | 5 |
  | 3 | Replace targetDefinesA with targetDeadA | 5 |
  | 4 | Add FLAGS-dead checks | 5 |
  | 5 | Final cleanup (remove debug trace) | 5 |

5× 5-file incrementals.  No full rebuilds.

## Cumulative state

  - Session 61 closed #141 — cpnos 1904 → 1878 B (−26 B)
  - Session 62 closed #142 — cpnos 1878 → 1866 B (−12 B)
  - Session 63 closed #144 — cpnos 1866 B unchanged
  - Session 64 closed #147 — cpnos 1866 → 1860 B (−6 B)
  - Session 65 closed #149 — cpnos 1860 B unchanged
  - Session 66 closed #148 — cpnos 1860 → 1858 B (−2 B)

**Cumulative cpnos: −46 B (1904 → 1858).**

6 closed, 8 corpus issues + 3 follow-ups remain.

## Rules-checked

  - `feedback_compiler_bug_test`: lit fixture demonstrates fix.
  - `feedback_test_before_fix`: test failed pre-fix, passes post-fix.
  - `feedback_no_commit_first_version`: full value-oracle satisfied
    including the FLAGS-correctness step-back analysis.
  - `feedback_value_oracle_all_transport_cells`: both pio-irq and
    sio polypascal cells PASS.
  - `feedback_polypascal_stage1_flake`: applied (`_kill-mpm` before
    sio retry to avoid daemon-state flake from prior runs).
  - `feedback_ninja_clang_llc_together`: rebuilt clang AND llc.
