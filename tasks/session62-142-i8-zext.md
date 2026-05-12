# Session 62 (2026-05-13) — #142 i8 zext residual elimination (safe variant)

## Context

Second closure from the corpus-mining batch (sessions 60c–60f).  #142:
i8 truncated from i16 then masked with a constant whose high byte is
zero (e.g. `r & 0x7F`), when subsequently equality-compared, was
producing a 5-byte `SUB_n + OR_H` "both bytes" sequence where 3 bytes
(`CP_n` only) suffice.

## Implementation

Two changes in `Z80InstructionSelector.cpp`:

1. **`isHighByteProvablyZero` recursion extended** to recognise:
   - `G_AND` — high byte zero if EITHER operand has high byte zero
     (bit-AND of a zero is zero).
   - `G_OR` / `G_XOR` — high byte zero if BOTH operands have high
     byte zero.
   - `G_MERGE_VALUES` — high byte zero iff the high s8 operand is
     `isProvablyZero`.

2. **New `isProvablyZero(Reg)` helper** that works on any register
   width.  Mutually recursive with `isHighByteProvablyZero` via the
   `G_MERGE_VALUES` / `G_UNMERGE_VALUES` cross-link (the legalizer's
   shape for i16 ops split into 2× i8 ops + MERGE).

The icmp i16 EQ/NE small-const path now correctly identifies
`(r & K)` results as high-byte-zero when `K` is.  Emits `CP_n`
instead of `SUB_n + OR_H` — saves 1 B per fire.

## What I tried that broke

Initial refinement aimed for a bigger saving by replacing the
existing `COPY $hl, VarReg; COPY $a, $l` two-step extraction with
a direct `COPY $a, VarReg:sub_lo`.  Lit tests passed; z80-utils
test-runner passed; cpnos+sio polypascal passed; **cpnos+pio-irq
polypascal failed** at stage 2 (PPAS load over CP/NET timed out).

Bisect:
  - recursion-only (no COPY refinement): 1866 B, both transports PASS
  - +sub_lo: 1858 B, sio PASS, pio-irq FAIL
  - +sub_lo + MERGE-bypass: 1836 B, sio PASS, pio-irq FAIL

The sub_lo COPY is the regression introducer.  Hypothesis: pure
sub_lo extraction leaves VarReg's high half dead in a way that
regalloc handles differently from the HL-materialising pair COPY;
some interaction with pio-irq-specific timing (ISR-fed ring buffer
in `transport_pio_recv_byte`) breaks runtime behaviour.  Not
reproducible at the lit level.

Filed as **ravn/llvm-z80#150** for investigation.  Committed the
safer recursion-only variant.

## Verification

  - **lit suite**: 97/97 (95 PASS + 2 XFAIL).  New fixture
    `issue-142-i8-zext-after-mask.ll` covers 4 cases:
      1. `(r & 0x7F) != 1` — the SNIOS protocol-byte check shape
      2. Same with EQ predicate
      3. `(r & 0xFF) == 0` — folds the AND away upstream
      4. Negative: `(r & 0x7FFF) == 1` — high byte non-zero, must
         fall back to `SUB_n + OR_H`
  - **z80-utils test-runner** clang Oz: 113 PASS / 0 FAIL.
    Baseline match.
  - **cpnos-rom clang/pio-irq payload**: 1878 → **1866 B (−12 B)**.
  - **cpnos-polypascal-test clang/pio-irq**: PASS.
  - **cpnos-polypascal-test clang/sio**: PASS.

Per-issue estimate was ~30 B; actual is 12 B with the safer variant.
The remaining ~18 B is gated on #150's resolution.

## Files touched

  - `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` — recursion
    extensions in `isHighByteProvablyZero`, new `isProvablyZero`
    helper.
  - `llvm/test/CodeGen/Z80/issue-142-i8-zext-after-mask.ll` — new
    lit fixture (4 cases).
  - `tasks/session62-142-i8-zext.md` — this doc.

## Cumulative state

  - Session 61 closed #141 — cpnos 1904 → 1878 B (−26 B)
  - Session 62 closed #142 — cpnos 1878 → 1866 B (−12 B)
  - Cumulative since corpus start: **−38 B on cpnos resident**

10 open issues remain from the corpus.  Added #150 to the
follow-up list.

## Rules-checked

  - `feedback_compiler_bug_test`: lit fixture covers all four cases.
  - `feedback_test_before_fix`: test failed before fix, passes after.
  - `feedback_no_commit_first_version`: full value-oracle satisfied;
    rolled back the version that failed pio-irq polypascal even
    though lit/test-runner passed.
  - `feedback_value_oracle_all_transport_cells`: both clang × pio-irq
    AND clang × sio polypascal cells run; both PASS.
  - `feedback_diff_binaries_before_blaming_codegen`: bisected by
    re-compiling the failing variant vs the passing recursion-only
    variant; cmp showed real codegen differences.
  - `feedback_ab_before_blaming_test_runner`: test-runner was clean
    in both bisect states; pio-irq polypascal was the diagnostic.
  - `feedback_ninja_clang_llc_together`: rebuilt clang AND llc.
