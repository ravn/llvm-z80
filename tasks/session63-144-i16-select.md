# Session 63 (2026-05-13) — #144 i16 sext-of-icmp lowering (-7 B per occurrence)

## Context

Third closure from the corpus-mining batch.  #144: `(a == K) ? -1 : 0`
for i16 was lowering to a 22-byte chain because the legalizer was
expanding `G_SEXT_INREG width=1` (synthesised from `sext i1` after IR
canonicalization) to `G_SHL i16, 15; G_ASHR i16, 15`.

The SHL by 15 lowering does `RRCA; AND $80; LD H, A; LD L, 0` (the
bit-positioning chain).  The ASHR by 15 lowering does the SEXT16
pseudo (high-byte-based sign-extend).  Together: 12 B of
bit-positioning + sign-extension on top of the 8 B icmp prologue.

## Implementation

Two changes:

1. **`Z80LegalizerInfo.cpp`**: kept `G_SEXT_INREG width=1` legal
   (previously only width=8 was kept; other widths were lowered
   via SHL+ASHR).

2. **`Z80InstructionSelector.cpp`**: added a case in the existing
   `G_SEXT_INREG` handler for `DstTy=i16, Width=1`.  Emits:
   - `COPY $a, src:sub_lo`
   - `RRCA` (bit 0 → CF)
   - `SBC A, A` (A = -CF = 0xFF or 0)
   - `REG_SEQUENCE Dst, A:sub_lo, A:sub_hi`

   ~5 bytes post-RA.

## Result

Repro (`(int a) → return (a == 1) ? -1 : 0;`):

| | Pre-#144 | Post-#144 | Theoretical optimal |
|---|---:|---:|---:|
| Bytes | 22 | 15 | 10 |
| Savings | — | −7 | −12 |

The post-fix output has a residual `and 1; rrca; sbc a, a`
sequence that's logically a no-op (the icmp's own `SBC A, A`
already produced 0xFF/0 in A; the residual chain round-trips
through {0,1} and back).  Filed as **ravn/llvm-z80#151** for a
post-RA peephole follow-up.

## Verification

  - **lit suite**: 98/98 (96 PASS + 2 XFAIL).  New fixture
    `issue-144-i16-select-on-eq.ll` covers the canonical
    `sext i1 (icmp eq i16 X, K) to i16` pattern.
  - **z80-utils test-runner** clang Oz: 113 PASS / 0 FAIL.
  - **cpnos-rom clang/pio-irq payload**: **1866 B unchanged**.
    The pattern doesn't appear in cpnos production code — the
    fix is general-utility (any C source with `(x == K) ? -1 : 0`
    or `-(int)(x == K)` benefits).
  - **cpnos-polypascal-test**: PASS on both pio-irq and sio cells
    (regression check; same binary).

## Files touched

  - `llvm/lib/Target/Z80/Z80LegalizerInfo.cpp` — widen the
    `G_SEXT_INREG` legal-widths set to include 1.
  - `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` — emit
    optimal i1→i16 sign extension.
  - `llvm/test/CodeGen/Z80/issue-144-i16-select-on-eq.ll` —
    new lit fixture.
  - `tasks/session63-144-i16-select.md` — this doc.

## Cumulative state

  - Session 61 closed #141 — cpnos 1904 → 1878 B (−26 B)
  - Session 62 closed #142 — cpnos 1878 → 1866 B (−12 B)
  - Session 63 closed #144 — cpnos 1866 B unchanged (general C
    benefit; no cpnos witness)

**Cumulative cpnos savings**: −38 B.

10 open issues remain from the corpus (was 11 — #144 closed; +#151
filed as follow-up).

## Rules-checked

  - `feedback_compiler_bug_test`: lit fixture demonstrates the fix.
  - `feedback_test_before_fix`: test failed pre-fix, passes post-fix.
  - `feedback_no_commit_first_version`: full value-oracle satisfied.
  - `feedback_value_oracle_all_transport_cells`: both pio-irq and
    sio polypascal cells PASS (regression check despite no cpnos
    code-size change — the binary IS different in unrelated
    upstream code, so testing was warranted).
  - `feedback_ninja_clang_llc_together`: rebuilt clang AND llc.
