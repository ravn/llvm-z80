# Implementation spec — tied-AReg 8-bit ALU modeling (#172, architecturally correct)

Branch: `z80-accumulator-tied-aReg`.  Supersedes the global-pass approach (see
`architecture-assessment-2026-05-30.md` CORRECTION).  Grounded in: llvm-mos
(constrained classes at ISel + tied operands + stock greedy), the in-tree
`INC16`/`LSHR16` precedent, and the tied-operand-alternative investigation.

## Goal

Model the user-level 8-bit ALU op `A = A op r` as an explicit **two-address
pseudo whose dst and accumulator-source are tied and constrained to the
single-register `AReg` class**, so a chain of such ops stays resident in A *by
construction* (TwoAddressInstructionPass + RegisterCoalescer collapse the ties;
AReg has one member, so greedy cannot escape to D/E/H/L).  This eliminates the
`ld a,r; <alu>; ld r,a` shuttle (#172) structurally, and lets the allocator —
not a hand-rolled pre-RA heuristic — resolve the genuinely-hard
parallel-accumulator case by spilling via its real interference graph.  When it
works, the parked `Z80PinAluAccumulator` pass is deleted, not extended.

## Critical design constraint: ADDITIVE, not a redefine

The existing pseudos `ADD_A_r`/`SUB_r`/`AND_r`/`OR_r`/`XOR_r`/`CP_r`
(`Z80InstrInfo.td:1459-1499`, implicit `Uses=[A] Defs=[A,FLAGS]`, empty outs) are
a **shared primitive used in ~30+ ISel sites** — many of which hand-build
physical-A multi-instruction idioms (e.g. `Z80InstructionSelector.cpp:769-802`:
`COPY $a=LhsHi; XOR_r RhsHi; RLCA; SBC_A_A; ...`).  Those sites legitimately use A
as scratch across a fused sequence and must **not** be tied.

Therefore: **add new tied pseudos; leave the implicit-A pseudos in place.**
Only the user-level binary-ALU selection sites switch to the new form.

## New pseudos (Z80InstrInfo.td)

For each accumulator-writing op (NOT CP — it has no dst):

```
def XOR_acc : Z80Inst<"xor_acc"> {
  let OutOperandList = (outs AReg:$dst);
  let InOperandList  = (ins AReg:$acc, GR8:$rhs);
  let Constraints    = "$dst = $acc";
  let Defs           = [FLAGS];
  let isPseudo = 1; let isCodeGenOnly = 1;
}
```

Same shape for `OR_acc`, `AND_acc`, `ADD_acc` (ADD A,r), `SUB_acc`.  Defer
`ADC_acc`/`SBC_acc` to a later slice (they involve carry-in; 8-bit ADC/SBC go
through carry-variant paths — confirm before touching).  `$rhs` stays `GR8` (any
of A/B/C/D/E/H/L; rhs==A is the legal `xor a` etc.).

Rationale this is safe (vs the `ADD16_tied` failure, `session73s-issue178`): that
bug needed a **narrower-def-than-source** class asymmetry (HLI def, GR16 source)
the coalescer widened.  Here `$dst` and `$acc` are **both AReg** — symmetric, the
configuration that works (matches `INC16`/`LSHR16`), nothing for the coalescer to
widen into.

## ISel wiring (Z80InstructionSelector.cpp)

At the user-level binary i8 selection sites — `G_ADD` (:3035), `G_SUB` (:3198),
`G_AND` (:3335), `G_OR`/`G_XOR` (:3493) — for the **register–register 8-bit**
case (not the constant/`(HL)`/CPL special cases already handled there), emit the
tied pseudo following the `INC16` recipe (`:1175-1188`):

```
Register Dst = MRI.createVirtualRegister(&Z80::AReg RegClass);   // DISTINCT def
BuildMI(MBB, MI, DL, TII.get(Z80::XOR_acc), Dst)
    .addReg(LhsReg)            // $acc — the chain carrier (AReg-constrained)
    .addReg(RhsReg);           // $rhs — GR8
// TwoAddressInstructionPass ties Dst==acc; result vreg Dst replaces MI's def.
constrainSelectedInstRegOperands(*MIB, TII, TRI, RBI);
```

The selector must route the *result* of the G_* op to `Dst` (replace
`MI.getOperand(0)`), and keep `$acc` = the LHS operand.  Commutativity: for
AND/OR/XOR/ADD either operand may be the carrier; prefer the one already in/headed
for A (the one with a longer downstream ALU chain) as `$acc` — a small heuristic,
refine after measuring.

## Post-RA expansion (Z80InstrInfo.cpp expandPostRAPseudo)

Add cases for the `*_acc` pseudos mirroring the existing `*_r` expansion
(`:1615-1690`): dst==acc==A is guaranteed by tie+class, so just read `$rhs`'s
physreg and emit the real `ADD_A_<r>`/`XOR_<r>`/... via the existing
`getADD8Opcode`/`getXOR...` helpers.  Operand indices shift (dst at 0, acc at 1,
rhs at 2) vs the old form (rhs at 0).

## Lit test (mandatory per CLAUDE.md)

`llvm/test/CodeGen/Z80/alu8-accumulator-chain.ll`: a `uint8_t` XOR/AND/OR chain
(`y = a^b; y ^= c; y ^= d;`) must emit a single A-resident run
(`xor b; xor c; xor d`) with **no** `ld r,a`/`ld a,r` shuttle between ops.
Regression guard: the comparison-lowering idioms (i16 EQ/NE, signed compare) must
be byte-identical (the implicit-A pseudos are untouched).

## Build + measurement gates (every iteration)

1. `ninja -C build clang` (Docker `llvm-z80-build`) or native `build-macos`.
2. `llvm-lit llvm/test/CodeGen/Z80/` — green (+ the new test).
3. **AES corpus all 13 configs** — the target; expect the gf/rj chain functions
   to shed the shuttle.  Watch for parallel-XOR kernels
   (`aes_mixColumns`/`aes_subBytes`) **regressing** via forced AReg spills — if
   so, the fix is a per-chain decision (constrain only the loop carrier, or a
   MachineCombiner-style cost check), NOT a blanket tie.
4. test-runner default + `-static-stack` differential oracle: 799/0/50/207 +
   793/0/50/213 must hold (correctness).
5. Production byte-watch: cpnos PROM1 / BIOS / autoload should be **byte-identical**
   (they have no gf-chains) — confirms no collateral damage.

## Known risk & stop condition

The parallel-accumulator case (two ALU values live at once) is unsatisfiable for
one physical A under *any* mechanism — AReg interference forces a spill
(`LD (nn),A` 3 B) instead of a shuttle (`ld r,a` 1 B), which can be *worse*.  The
honest expectation: the single-chain case improves structurally; the parallel
case must be left to the allocator and may need the carrier-only refinement.  If
a blanket tie net-regresses AES after the carrier refinement, the conclusion is
that the residency win is real but small and the mechanism is sound — ship it for
the architectural-maturity value (retiring the parked pass + reactive machinery),
not for benchmark bytes.

## Sequence

1. Add the 5 tied pseudos + expansion (no ISel wiring yet) — builds, no behavior
   change. Verify lit still green.
2. Wire `G_XOR` reg-reg only → `XOR_acc`. Build, add lit test, measure AES.
   First real datapoint: does the single-chain shuttle vanish?
3. If clean, extend to OR/AND/ADD/SUB. Re-measure.
4. Handle parallel-XOR regression if it appears (carrier-only constraint).
5. Delete `Z80PinAluAccumulator` + its pipeline hook once superseded.
6. (Future) Apply the same tied-class discipline to the HL 16-bit address
   accumulator (folds in #166/#178 recompute-vs-spill).
