# Session 61 (2026-05-12) — #141 i16 vs byte-aligned-constant fold landed (−26 B cpnos)

## Context

Corpus-mining (sessions 60c–60f) produced 12 filed issues.  Session
61 closes the first one: #141 (i16 comparison against 0x0100
should fold to high-byte test).

## Implementation

`Z80InstructionSelector.cpp` — in the existing i16 unsigned-compare
branch (around the previously-unconditional CMP16_FLAGS emission),
added a pre-check that recognises one operand as a byte-aligned
constant and emits a single 8-bit high-byte test instead.

Patterns folded:

| Source (post-normalisation) | Constant shape | Effective test |
|---|---|---|
| `icmp uge var, N*256` (var-LHS, K-RHS) | K mod 256 == 0, K ∈ [0x100, 0xFF00] | `var_hi ≥ N` |
| `icmp ult var, N*256` (var-LHS, K-RHS) | K mod 256 == 0, K ∈ [0x100, 0xFF00] | `var_hi < N` |
| `icmp uge K, var` (K-LHS, var-RHS, from `ule`) | K mod 256 == 0xFF, K ∈ [0xFF, 0xFEFF] | `var_hi < N+1` |
| `icmp ult K, var` (K-LHS, var-RHS, from `ugt`) | K mod 256 == 0xFF, K ∈ [0xFF, 0xFEFF] | `var_hi ≥ N+1` |

Emit shape:
  - `Thresh == 1`: `COPY $a, var:sub_hi; OR_A; JP_{NZ,Z}` (4 B total)
  - `Thresh ≥ 2`:  `COPY $a, var:sub_hi; CP_n N; JP_{NC,C}` (5 B total)

vs the existing `CMP16_FLAGS` chain at 9 B (ld bc,nn; sub e; ld a,b;
sbc a,d; jr cc) — saving 4–5 B per occurrence.

The `Z80::sub_hi` sub-register index is used to extract the high
byte directly during ISel, avoiding an HL-forcing COPY that the
existing pattern at line 1074 uses for the EQ/NE case.  Pattern
matched against existing usages at lines 1136, 1143, 1194, 1209
(byte-XOR EQ/NE forms).  No new register-class plumbing required.

## Verification

  - **Z80 lit suite**: 96/96 (94 PASS + 2 XFAIL).  New fixture
    `issue-141-i16-vs-0x100-fold.ll` covers 5 cases:
      1. `r >= 256` (Case 1 UGE, Thresh=1)
      2. `r < 256` (Case 1 ULT, Thresh=1)
      3. `r <= 255` (Case 2 UGE, Thresh=1 via swap)
      4. `r >= 512` (Case 1 UGE, Thresh=2, exercises CP_n path)
      5. `r >= 257` (negative — NOT byte-aligned, must fall back)
  - **z80-utils test-runner** clang Oz: 113 PASS / 0 FAIL.  Baseline
    match (1 pre-existing FATAL on alloca.h).
  - **cpnos-rom clang/pio-irq payload**: **1904 B → 1878 B** (−26 B).
    Per-issue estimate was ~45 B; actual is below that, consistent
    with some sites not matching (multi-byte sentinels, slot-coalesced
    spills that don't reach this lowering, etc.).  Solid concrete win.
  - **cpnos-polypascal-test clang/pio-irq**: PASS (PPAS PRIMES 29989).
  - **cpnos-polypascal-test clang/sio**: PASS (PPAS PRIMES 29989).

## Production fire confirmation

Re-disassembling cpnos.lis post-fix shows the `(uint8_t)r & 0x7F`
recv-byte-checks now compile with `ld a,d; or a; jr {z,nz}` (3 B
test+branch) instead of the previous 9 B compare-with-0xFF chain.
The recv-byte-test sites in `_snios_rcvmsg_c` and
`_snios_sndmsg_force` are the primary beneficiaries.

## Rules-checked

  - `feedback_compiler_bug_test`: new lit fixture added
    (`issue-141-i16-vs-0x100-fold.ll`).
  - `feedback_test_before_fix`: test was failing before
    implementation, passes after.
  - `feedback_no_commit_first_version`: full value-oracle satisfied:
    lit + test-runner (no regressions) + cpnos size delta + 2-cell
    polypascal-test PASS.
  - `feedback_value_oracle_all_transport_cells`: both clang × pio-irq
    AND clang × sio polypascal tests run; both PASS.
  - `feedback_compare_total_section_sizes`: `.payload` section measured.
  - `feedback_ninja_clang_llc_together`: rebuilt clang AND llc.

## Files touched

  - `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` — added
    high-byte fold in the i16 unsigned-compare branch (~70 LOC).
  - `llvm/test/CodeGen/Z80/issue-141-i16-vs-0x100-fold.ll` — new
    lit fixture (5 cases).
  - `tasks/session61-141-i16-fold.md` — this doc.

## Cumulative state

The first issue from the corpus-mining batch is closed.  cpnos-rom
resident: 1906 B (pre-corpus) → 1904 B (#132) → 1878 B (#141).
**Net −28 B** delivered to date.

Remaining open from sessions 60c–60f corpus: #138, #139, #140,
#142, #143, #144, #145, #146, #147, #148, #149.  Plus #132 itself
left open for the multi-fire-interaction follow-up (#143).

## Follow-up note for #141

The issue can be closed once it's been verified upstream-of-this
work that the lit fixture continues passing.  Will mark with a
"closed by commit X" comment.
