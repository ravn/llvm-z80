# Session 64 (2026-05-13) — #147 SET/RES on memory peephole (-6 B cpnos)

## Context

Fourth closure from the corpus-mining batch.  #147: `mem |= 1<<N`
and `mem &= ~(1<<N)` for single-byte constant-address globals
were lowering to 8-byte sequences (`LD A,(sym); OR n; LD (sym),A`)
when Z80's CB-prefix `SET b,(HL)` / `RES b,(HL)` could do it in
5 bytes.

Concrete cpnos witnesses (all in SNIOS error paths):

  - f275: `cfgtbl.netst |= CFG_NETST_SNDERR` (bit 0)
  - f3c6: `cfgtbl.netst |= CFG_NETST_RCVERR` (bit 1)
  - f1ad: `cfgtbl.netst &= ~(CFG_NETST_RCVERR | CFG_NETST_SNDERR)`
    (two-bit clear)

## Implementation

Post-RA peephole in `Z80LateOptimization.cpp`, inserted just before
the existing "Redundant PUSH AF/POP AF" cleanup.

Pattern (after stepping over A-unrelated intervening insns):
```
LD_A_nnind <Sym>            ; 3 B
{OR_n, AND_n} K              ; 2 B
LD_nnind_A <Sym>            ; 3 B   (same Sym)
                            ; = 8 B
```

Replaced with:
```
LD_HL_nn <Sym>              ; 3 B
{SET, RES}_b_(HL)            ; 2 B each, N=popcount
                            ; = 5 B (single-bit) or 7 B (two-bit)
```

Single-bit OR/AND: saves 3 B.  Two-bit: saves 1 B.  Three or more
bits: skipped (would cost more than 8 B).

### Safety guards

  - **A must be dead** after the store position.  The new sequence
    doesn't write A, so any downstream read would see a stale value.
    In practice the witnesses are immediately followed by a CALL
    (which clobbers A per the sdcccall(1) convention), so A is dead.
  - **HL must be dead** at the load position.  The new sequence
    writes HL via `LD_HL_nn`, clobbering whatever it held.
  - **Intervening insns must not touch A**.  This is conservative
    — strictly we only need them not to *write* A, but reading the
    pre-load A value is fine since we're keeping the value computation
    locally consistent.  See the bail in the forward-walk loop.
    The f1ad witness (`LD A; LD D,A; AND $fc; LD (sym),A`) has an
    intervening A-reader (`LD D, A` to save the return value) and
    is correctly skipped.  ravn/llvm-z80#152 tracks the "preserve
    A via LD A,(HL)" variant that would catch it.

## Result

| | Pre-#147 | Post-#147 |
|---|---:|---:|
| cpnos payload | 1866 B | **1860 B (−6 B)** |
| SET/RES fires in cpnos | 0 | 2 (single-bit OR sites) |
| AND-clear witness (f1ad) | unchanged | unchanged (intervening LD D,A; tracked as #152) |

Per-issue estimate: ~7 B.  Actual: 6 B.  Close; the missed
two-bit witness (f1ad) accounts for the 1 B gap.

## Verification

  - **lit suite**: 99/99 (97 PASS + 2 XFAIL).  New fixture
    `issue-147-set-res-mem.ll` covers 5 cases: bit 0 set, bit 1
    set, bit 0 clear, two-bit set, three-bit negative (must
    fall back).
  - **z80-utils test-runner** clang Oz: 113 PASS / 0 FAIL.
  - **cpnos-rom clang/pio-irq payload**: 1866 → **1860 B (−6 B)**.
  - **cpnos-polypascal-test**: PASS on both pio-irq and sio cells.

## Files touched

  - `llvm/lib/Target/Z80/Z80LateOptimization.cpp` — new peephole
    (~100 LOC).
  - `llvm/test/CodeGen/Z80/issue-147-set-res-mem.ll` — new lit
    fixture (5 cases).
  - `tasks/session64-147-set-res-mem.md` — this doc.

## Ninja rebuild tracking (per user request, started session 63)

This session ninja rebuilds:

  | # | Trigger | Files rebuilt | Notes |
  |---:|---|---:|---|
  | 1 | session-64 start, after editing Z80LateOptimization.cpp | 5 (.cpp.o → libZ80CodeGen.a → llc + clang + symlink) | Normal incremental — correct minimum set |
  | 2 | second edit (safety check refinement) | 5 | Same shape, ~30 s |

No "confused full rebuilds" this session.  Pattern is consistent:
ninja only rebuilds the .cpp.o that changed, the libLLVMZ80CodeGen.a
that contains it, and the two executables (llc + clang) that link
that library.  This is the correct minimum.

The only outlier across the recent sessions was the **session-63
PCH mismatch** (~25 min full rebuild) caused by a host clang
upgrade invalidating the precompiled headers.  That's a one-off,
not a ninja confusion issue.

## Cumulative state

  - Session 61 closed #141 — cpnos 1904 → 1878 B (−26 B)
  - Session 62 closed #142 — cpnos 1878 → 1866 B (−12 B)
  - Session 63 closed #144 — cpnos 1866 B unchanged (general C)
  - Session 64 closed #147 — cpnos 1866 → 1860 B (−6 B)

**Cumulative cpnos savings: −44 B.**

Filed ravn/llvm-z80#152 (preserve A through `LD A,(HL)` for the
intervening-A-reader case).  10 open issues remain from the
corpus (was 11; +#151, +#152, −#147).

## Rules-checked

  - `feedback_compiler_bug_test`: lit fixture demonstrates fix.
  - `feedback_test_before_fix`: test failed pre-fix, passes post-fix.
  - `feedback_no_commit_first_version`: full value-oracle satisfied.
  - `feedback_value_oracle_all_transport_cells`: both pio-irq and
    sio polypascal cells PASS.
  - `feedback_ninja_clang_llc_together`: rebuilt clang AND llc.
