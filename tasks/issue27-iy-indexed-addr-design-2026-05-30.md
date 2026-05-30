# #27 → IX/IY-displacement addressing for register-held pointers (design)

Branch: `z80-27-iy-indexed-addr` (off main).  Session 2026-05-30.

## Problem (drill result)

Under the production model (`-Oz`/`-Os` + `+static-stack`, so
`z80IsIYAllocatable(MF)` is true), greedy regalloc parks a loop-invariant
pointer argument in IY to keep it live.  But the dereference *form* is chosen
at instruction selection (pre-RA), before the IY assignment exists, so ISel
emits the generic `G_PTR_ADD(ptr, const) + G_LOAD/STORE` →
`ld bc,d; add hl,bc; ld r,(hl)` and the post-RA copy `push iy; pop hl` feeds it.

`aes_shiftRows` pays, per access:
```
ld bc, 9        ; 3B
push iy         ; 2B   IY -> HL
pop  hl         ; 1B
add  hl, bc     ; 1B
ld   a, (hl)    ; 1B   = 8B
```
vs the ideal `ld a,(iy+9)` = 3B.  ~46 such sites in the AES corpus alone
(~150–180 B); BIOS/cpnos use the same model.

The existing ISel indexed-addr matcher (`Z80InstructionSelector.cpp:2416`)
only fires for `COPY $ix` — i.e. the **frame pointer**, materialised early —
never for an IY/IX value that is a *regalloc outcome*.  Phase-ordering gap.

## Approach B (chosen): deferred-addressing pseudo

Emit a pseudo at ISel that keeps the base as a GR16 vreg + an immediate
displacement, and expand it post-RA (in `Z80ExpandPseudo`, model =
`COPY16_PUSHPOP` at line 155) based on the **allocated** base register:

- base ∈ {IX, IY}, disp ∈ [-128,127] (and disp+1 for 16-bit): emit
  `LD r,(IX/IY+disp)` / `LD (IX/IY+disp),r` — opcodes already exist
  (`LD_{A,B,C,D,E,H,L}_I{X,Y}d`, `LD_I{X,Y}d_{...}`).  Touches only `$dst`.
- else (base ∈ {HL,DE,BC}): fallback that computes base+disp in HL and
  does `(hl)`, **preserving the base** (it is reused across the loop).

## The clobber tension (the hard part)

A fixed Defs set can't be accurate for both paths: the index path touches
only `$dst`; the fallback needs HL (+ scratch for the add).  Resolution:

1. **Regalloc hint** (`getRegAllocationHints`): hint the base pointer toward
   IY/IX when it is dereferenced ≥N times with constant offsets, so the cheap
   path is the common case.  (Precedent: DE/BC hints for ADD HL,rr operands.)
2. Declare the pseudo to match the **index** path (def `$dst` only); make the
   fallback self-contained — borrow scratch via `push/pop` so it clobbers
   nothing beyond `$dst`.  Fallback is larger than today's 7 B but is the rare
   path once the hint biases allocation.

If the hint proves ineffective (cf. #172/73n greedy-ignores-hints, though that
was the A-accumulator, not GR16→IX/IY), fall back to declaring Defs=[HL] and
accepting index-path pessimism; measure both.

## Staging (test + oracle-gate each stage)

- **S1**: 8-bit LOAD pseudo (`LOAD_IDX8`) — ISel emit for
  `G_LOAD(G_PTR_ADD(ptr,const))`, |const|≤127; expansion (index + fallback);
  lit test; build; AES measure; lit suite; differential oracle.
- **S2**: 8-bit STORE (`STORE_IDX8`), incl. store-of-constant.
- **S3**: 16-bit LOAD/STORE (disp and disp+1 in range).
- **S4**: regalloc hint; re-measure; decide default-on.

## Acceptance / gates

- lit Z80 suite green (add new test(s)).
- AES corpus 13/13 PASS; production config net size/ts improvement.
- Differential oracle (test-runner default + `-static-stack`) 0 divergences.
- cpnos PROM1 byte-delta measured + polypascal PASS; BIOS MAME boot.
- Default the feature only after S4 measurement; behind a flag until then.

## Risks

- Fallback bloat if the hint fails → net regression on fallback-heavy code.
  Mitigation: measure after S1 before widening; keep flag-gated.
- 16-bit indexed needs disp AND disp+1 ≤127 (same as the IX matcher guard).
- Don't emit for frame-index bases (the existing matcher already handles those
  via RELOAD pseudos) — only plain pointer vregs.
