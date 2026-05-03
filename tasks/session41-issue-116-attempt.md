# Session 41 — issue #116 attempt + revert (2026-05-03)

User asked for option B from end-of-session-40 menu:
> #116 prototype — post-RA peephole or new pseudo for i16 EQ/NE -> SBC HL,DE
> at hasMinSize().  ~40 LOC.  Smaller per-site win (-1 B / +3 T) but
> fires more often (~20 sites in BIOS).  Risk: low — local pattern,
> HL-dead-after check is straightforward.

The "low-risk warm-up" framing turned out to be wrong.  The change
ran the wrong way.  Reverted in the same session.

## What was tried

`Z80InstructionSelector.cpp`, `emitFusedCompareAndBranch`, the i16 EQ/NE
variable-RHS path (around line 1147).  At `hasMinSize()`:

  - constrain LHS to `GR16NoIR` (DE/HL/BC, no IX/IY)
  - constrain RHS to `GR16_BCDE` (DE/BC, required by `SUB_HL_rr`)
  - emit `COPY HL <- LHS`
  - emit `SUB_HL_rr RHS` (expands to `AND A; SBC HL,rr`, 3 B)

Replacing the existing 6-byte byte-XOR sequence
(`LD A,lhs_hi; XOR rhs_hi; LD tmp,A; LD A,lhs_lo; XOR rhs_lo; OR tmp`).

## Result

  - lit suite: 86 PASS + 1 XFAIL (was 85 + 1).  New guard test passed.
  - rcbios bios.cim: **5933 -> 5960 B (+27 B regression)**.

## Why it regressed

`SUB_HL_rr` has `Defs = [HL, A, FLAGS]` — it consumes HL.  The byte-XOR
form does not touch HL/DE/BC pairs; it only uses A + a GR8 scratch.

This means the byte-XOR form is *safe to insert in the middle of a loop
where HL holds a long-lived value*.  Substituting `SBC HL,rr` evicts
that long-lived value out of HL.  Real example from the strand-B
synthetic `_render`:

  - **Pre-change**: `end_idx` lives in HL across the outer loop;
    each outer iter does the 6-byte XOR test against BC-then-HL.
  - **Post-change**: `end_idx` is spilled to a BSS slot
    (`__sfrend_render-11`); each outer iter reloads `LD DE,(slot)`
    (4 B), copies BC->HL (2 B), then `AND A; SBC HL,DE` (3 B).
    Plus an extra `LD BC,(slot); INC BC` cycle around the loop tail.

Net per outer iter: ~+5 B vs old shape, plus ambient regalloc churn
elsewhere.  The estimated -1 B/fire savings was based on an implicit
assumption that LHS would already be in HL — which the constraint
itself disproves.

## Diagnosis

The savings model (XOR 6 B vs SBC 3 B = -3 B; minus 0-2 B for the
LHS->HL move = -1 to -3 B per fire) is correct *in isolation*.  It
fails to model the second-order cost: forcing HL into the compare
removes HL from the long-live regalloc options, and the value that
*was* there gets pushed to BSS (4 B per access on Z80) or shuffled
through DE/BC.

The right tool for #116 is a **post-RA peephole** that inspects actual
register placement and HL liveness:

  1. Match the byte-XOR sequence as emitted by ISel.
  2. Verify HL is dead-immediately-before AND dead-immediately-after
     (via `MachineBasicBlock::computeRegisterLiveness`).
  3. Verify LHS is in HL or cheaply movable to HL.
  4. Verify RHS is in BC or DE.
  5. Replace with `AND A; SBC HL,rr`.

Only steps 1+5 are mechanical; 2-4 are the regalloc-aware filter that
the ISel-time approach lacks.

Estimated effort: ~80-120 LOC in `Z80LateOptimization.cpp` (similar
shape to the existing BSS-spill -> PUSH/POP peephole), plus a richer
lit test that exercises the dead-HL precondition.

## What was kept

  - `llvm/test/CodeGen/Z80/issue-116-i16-eqne-sbc-hl.ll` —
    rewritten as a regression-guard for the *current* byte-XOR shape.
    Two functions (default and `minsize`) both expected to emit the
    XOR sequence today.  When the post-RA peephole lands, flip the
    `minsize` function's CHECK lines to expect `and a; sbc hl,...`.
  - 14-line note in `Z80InstructionSelector.cpp` at the variable-RHS
    branch documenting the attempt and steering future contributors
    to the post-RA path.

## Sizes & lit

  - rcbios bios.cim: 5933 B (baseline restored)
  - cpnos.bin payload: 1777 non-padding bytes (no compiler change
    by the time of revert; the 1708 -> 1777 drift relative to the
    CLAUDE.md figure predates this session)
  - Z80 lit suite: 86 PASS + 1 XFAIL

## Carry-forward

  - **#116 — post-RA peephole** is the right next attempt for this
    issue.  Concrete plan above; not started.
  - **#114 — Z80ShadowBankBracket** prototype remains the larger
    next step; the synthetic + lit fixture from session 40 follow-up
    are still in place as the regression guard.
