# Session 41 — issue #116 post-RA peephole (2026-05-03)

After the morning's ISel-time attempt regressed bios.cim by +27 B and
was reverted (see `session41-issue-116-attempt.md`), this is the
proper implementation: a post-RA peephole that inspects actual
register placement and HL liveness before firing.

## What landed

`Z80LateOptimization.cpp`, ~120 LOC inserted after the existing
constant-fold-into-XOR-compare peephole (around line 3354).

Pattern matched (6 consecutive instructions emitted by ISel for i16
EQ/NE compare-and-branch with variable RHS):

```
LD  A, X        ; X = sub_{hi,lo} of QPair (the "loaded" pair)
XOR R1          ; R1 = matching half of PPair (the "XOR'd" pair)
LD  T, A        ; T = some GR8 scratch
LD  A, Y        ; Y = the other half of QPair
XOR R2          ; R2 = the other half of PPair
OR  T           ; combine: Z=1 iff QPair == PPair
JR Z|NZ / JP Z|NZ
```

Replaced with:

```
AND A
SBC HL, otherpair       ; otherpair = whichever of {QPair,PPair} isn't HL
JR Z|NZ / JP Z|NZ
```

Saves **3 B and 5 T-states per fire** (6 B / 24 T -> 3 B / 19 T).

## Firing predicate

All six must hold:

  1. The 6 consecutive instructions match the byte-XOR shape exactly.
  2. The (hi,lo) polarity of (X, R1) matches; (Y, R2) is the opposite.
  3. T does not alias either pair (so `LD T,A` doesn't clobber an
     operand still needed at I5/I6).
  4. **Exactly one** of QPair / PPair is HL; the other is BC or DE.
     (The "neither HL" case would need an extra LHS->HL move and is
     skipped — it risks the same regalloc-eviction problem that bit
     the ISel-time attempt.)
  5. The instruction immediately after OR T is a Z-only branch
     (`JR Z|NZ` / `JP Z|NZ`).  Any other FLAGS consumer would see
     different C/N/P/V/S/H values from the SBC vs the original XOR.
  6. A, T, and HL are all dead-after the branch.  The replacement
     preserves A and T but writes HL — A/T must be dead so observation
     is unchanged, HL must be dead so the SBC's HL clobber is safe.

The "exactly one is HL" predicate is what sidesteps the eviction
problem.  We only fire when regalloc has *already* parked one of the
operands in HL of its own accord — we never force the choice.

## Result

  - Z80 lit suite: **86 PASS + 1 XFAIL** (no regressions; new
    `issue-116-i16-eqne-sbc-hl.ll` exercises both the firing case
    and the loop-carried-HL non-firing case).
  - rcbios bios.cim: **5933 -> 5929 B (-4 B)**.
  - cpnos.bin payload: **1777 B (no change)** — peephole doesn't
    fire there.

## Why so few fires

Initial framing in #116 estimated ~20 sites in BIOS.  Reality: ~1-2
fires (-4 B total).  The estimate was wrong because most i16 EQ/NE
in BIOS uses *constant* RHS (folded by the existing
constant-fold peephole at line 3205) and never enters the
variable-RHS byte-XOR path the peephole targets.

The remaining variable-RHS sites mostly have HL spoken for by
something else (loop-carried address, `end_idx`-style invariant) —
exactly the cases where the peephole correctly *doesn't* fire.

So #116 is a small targeted win.  The bigger comparison-sequence
gap from `Z80/CLAUDE.md` ("Comparison sequences ~50 B") is mostly
the constant-RHS path, which is already handled.

## Files

  - `llvm/lib/Target/Z80/Z80LateOptimization.cpp` (+170 lines)
  - `llvm/test/CodeGen/Z80/issue-116-i16-eqne-sbc-hl.ll`
    (rewritten: two-function test, firing + non-firing case)
  - `tasks/session41-issue-116-peephole.md` (this doc)

## Carry-forward

  - **#114 prototype (Z80ShadowBankBracket)** is the bigger
    next step.  Today's session validated that post-RA liveness
    checks via `isRegDeadAfter` + the firing-predicate discipline
    works without regressing — same machinery applies to the
    three-pair (BC/DE/HL) bracket #114 needs.
  - The "neither HL" case in #116 (requires an LHS->HL move
    before SBC) is left out.  Could be added if a real motivating
    site appears, but rcbios doesn't have one today.
