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

## The clobber tension (the hard part) — resolved with RegScavenger

A *fixed* Defs set can't be accurate for both paths: the index path touches
only `$dst`; the fallback needs HL + a scratch pair for the add.  Declaring
`Defs=[HL,BC]` would pessimise the **cheap** path — and AES is exactly HL/BC-
pressured, so that pessimism could erode/reverse the win.

**Resolution:** declare NO extra Defs (pseudo defs `$dst` only).  Expand post-RA
with the **RegScavenger** (same mechanism `eliminateFrameIndex` uses) to obtain
a scratch pair in the fallback only.  Cheap path (base ∈ IX/IY) needs no
scratch.  So the cost model stays honest on the win path and the fallback is
still correct.  The expansion pass must set `RequiresRegisterScavenging`.

Optional later: a `getRegAllocationHints` nudge toward IY/IX for pointers
dereferenced ≥N times with constant offsets (precedent: DE/BC hints for
ADD HL,rr), to make the cheap path the common case.

## Correctness details nailed down (2026-05-30)

- **Model the index-reg Use.**  `LD_{r}_I{X,Y}d` defs declare only `Defs=[r]`,
  NOT `Uses=[IX/IY]` (the IX matcher relies on IX being the always-live frame
  pointer).  Expansion MUST add `.addReg(Base, RegState::Implicit)` so post-RA
  liveness knows IY/IX is read; `fullyRecomputeLiveIns` then propagates it.
- **dst∈{H,L} aliasing in the HL-base fallback.**  If base lands in HL and the
  load dst is H or L, naive `ld Dst,(hl)` + restore-HL clobbers the result.
  Fallback must load via the scavenged pair or sequence the restore carefully.
- **Opcode selection.**  Pick `LD_<dstPhys>_I{X,Y}d` by the allocated GR8
  dst regunit (full A/B/C/D/E/H/L set exists for both IX and IY).
- **Do NOT capture** frame-index bases (RELOAD pseudos own those) nor the
  existing `COPY $ix` frame-pointer base (matcher at 2416 owns it).  Fire only
  for a plain pointer vreg base with a G_CONSTANT offset in range.

## Flag gating

Behind `-mllvm -enable-z80-idx-addr` (default OFF) until S4 measurement clears
the differential oracle.  Lets an incomplete fallback land safely and be
measured without touching production codegen.

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

## Stage-1 result (2026-05-30) — mechanism works, naive forcing does not

Implemented LOAD_IDX8 + ISel emission (base→IR16) + expansion.  Flag
`-mllvm -z80-idx-addr`, default OFF.  Build green; lit 138 PASS + 4 XFAIL.

- **Correct in isolation**: `one()` → `push hl; pop iy; ld a,(iy+9)`;
  `sum3()` shares one IY setup across three `ld r,(iy+d)`.  ✓
- **AES prod config**: indexed loads 19→76, insns 1338→1293,
  **t-states 10.72M→10.56M (−1.5%)** — the idea has real merit.
- **BUT**: size **+158 B** (2190→2348) and a **semantic miscompile**
  (test FAIL; machine verifier is CLEAN, so it's a well-formed-but-wrong
  transform, not a liveness bug).

Two root problems with the naive Stage-1 (fire on *every* 8-bit const-offset
load + hard IR16 forcing):
1. **Size**: forcing single-deref pointers into IX/IY adds a 3 B `push/pop`
   copy that isn't amortised.  Needs **selectivity** — only loop-invariant
   pointers dereferenced ≥2–3× at constant offsets; add a size guard.
2. **Miscompile**: subtle interaction on full AES.  Prime suspects — the
   existing Z80LateOptimization IX/IY-transfer / `PUSH IY;POP IY`-removal
   peepholes (run BEFORE ExpandPseudo; #14 liveness-guard history) meeting the
   new `push iy; pop hl` shapes, or a pointer that is BOTH indexed-loaded and
   stored-through (base also lives in BC/DE for `ld (bc),a`).  Needs MIR diff
   between OFF and ON on the first diverging function.

Production unaffected (flag default OFF; OFF build byte-identical: 2190 B,
PASS).  Committed as WIP checkpoint.

## Risks

- Fallback bloat if the hint fails → net regression on fallback-heavy code.
  Mitigation: measure after S1 before widening; keep flag-gated.
- 16-bit indexed needs disp AND disp+1 ≤127 (same as the IX matcher guard).
- Don't emit for frame-index bases (the existing matcher already handles those
  via RELOAD pseudos) — only plain pointer vregs.
