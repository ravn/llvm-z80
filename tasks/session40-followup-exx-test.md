# Session 40 follow-up — EXX-bracket synthetic test (2026-05-03)

Small post-merge follow-up to session 40.  User asked: "construct
a good test example that would benefit from EXX".

## Deliverables

  - `tasks/exx-candidate-synthetic.c` — C reproducer modelling
    `_specc` 0xde19-0xde3c, with header rationale on why all
    inputs are globals (so `+static-stack` BSS-mode kicks in)
    and why `inner_n` is `volatile` (so the inner loop can't
    be unrolled away).
  - `llvm/test/CodeGen/Z80/issue-114-exx-bracket-candidate.ll`
    — IR fixture, locks in today's `LD (sframe-6),BC` /
    `LD BC,(sframe-6)` spill pair around the inner loop.  Lit
    suite now 85 PASS + 1 XFAIL (was 84+1).
  - `tasks/exx-candidate-analysis.md` — byte-precise disassembly
    of `_render`, predicted post-#114 form, savings table.

## Verified savings (per fired loop)

| Metric                    | Today | Post-#114 | Δ        |
|---------------------------|------:|----------:|---------:|
| Spill code (bytes)        |     8 |         2 |    **-6** |
| BSS slot (bytes)          |     2 |         0 |    **-2** |
| **Total static**          | **10**|     **2** |    **-8** |
| Spill code T-states/iter  |    40 |         8 |   **-32** |

Across the 4 BIOS candidate functions (strand-B survey:
`_specc`, `_scroll`, `_cursor_left`, `_bios_conin`), projects to
roughly **24 B from rcbios bios.cim** when #114 lands.

## Firing predicate (refined)

The pass walks `MachineLoopInfo` after RA and accepts a region
only if **all six** hold:

  1. Single-MBB self-back-edge inner loop (DJNZ or JR).
  2. No CALL inside the bracketed region.  Hard exclusion.
  3. Spill/reload fingerprint present: matched
     `LD (nn),pair` / `LD pair,(nn)` pair on a single GR16
     bracketing the loop body.
  4. All three of BC, DE, HL are dead-or-redefined at both
     bracket boundaries (verified via
     `MachineBasicBlock::computeRegisterLiveness`).
  5. `+shadow-regs` enabled (so ISR prologue does its own EXX,
     keeping the swap safe under interrupt).
  6. Win threshold: ≥ 4 B of spill/reload code in the original
     (EXX bracket costs 2 B fixed).

In the synthetic, predicates 1, 2, 3, 4, 6 all pass.  Predicate
5 not set in the test today — would be required at prototype
time.

## Sibling issue filed

**ravn/llvm-z80#116** — i16 EQ/NE compare: prefer SBC HL,DE when
HL dead-after-compare (-Oz).

Discovered while reading `_render` outer.latch.  Today's emit:
`ld a,b; xor d; ld d,a; ld a,c; xor e; or d` (6 B / 24 T).
With BC->HL move + SBC HL,DE: 5 B / 27 T.  Net **-1 B / +3 T per
fired compare** — favourable at `-Oz`, gateable on
`hasMinSize()`.  Aligns with the "comparison sequences ~50 B"
gap entry in `Z80/CLAUDE.md`.  Orthogonal to #114.

## State at end

  - Lit: 85 PASS + 1 XFAIL.
  - rcbios bios.cim: byte-exact (no compiler change).
  - cpnos.bin: byte-exact (no compiler change).
  - Open issues: 25 → 26 (+#116).
  - No source changes to llvm/lib/Target/Z80/.

## Carry-forward

  - #114 prototype work itself remains the next big step;
    the synthetic test is now the regression-guard fixture
    for that work.
  - #116 is a small independent peephole, plays into the
    "Comparison sequences ~50 B" cluster.
