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

## Stage-2 result (2026-05-30) — WIN on AES, flag-gated

Added STORE_IDX8 (mirror of LOAD_IDX8) + ISel store emission + expansion, and a
`!FnHasCalls` correctness gate on BOTH (cached in
`setupGeneratedPerFunctionState`).  Root cause of the S1 decrypt FAIL was
confirmed: IY is caller-saved (`Z80_CSR = IX` only); `aes_expDecKey` (8 calls)
with IX as frame pointer forced the base into IY, which didn't survive the
calls.  The gate fixes it.

With BOTH load and store indexed, read-modify-write drops the address
computation entirely:
- **AES prod config: aes256.c .text 2190 → 2043 B (−147 B / −6.7%)**, t-states
  10.72M → 10.71M (−0.14%), encrypt+decrypt PASS (kr + ansi).
- lit suite 139 PASS + 4 XFAIL (added `issue-27-iy-indexed-addr.ll`).
- Default OFF; production codegen unchanged.

Note: single-deref RMW is *bigger* under the flag (the `push hl;pop iy` setup,
3 B, only amortises across ≥2 accesses).  AES wins because its functions do
many accesses per pointer.

## Profitability gate added (2026-05-30)

`countIndexedSites(base)` — emit only when the base has >=2 distinct in-range
constant-offset G_PTR_ADD sites (so the one-time `push hl; pop iy` setup
amortises; a single site is larger under the flag).  Counting SITES (not their
load/store users) is selection-order-stable: a G_PTR_ADD persists while its mem
users are selected and erased one by one.

AES prod config (flag on): **2190 -> 2054 B (-136 B / -6.2%)**, t-states −0.11%,
PASS; vs −147 B ungated (≈11 B traded for single-site safety).  lit 139 + 4.

Net state of #27 work this session: correct, tested, flag-gated (default OFF),
AES −136 B.  Gates: call-free (correctness) + >=2 sites (profitability).

## Production impact (2026-05-30) — measured ZERO, and Stage 3 won't change that

Built cpnos PROM1 with the flag on: **2022 B — byte-identical to baseline.**
The feature fires on zero cpnos functions (call-heavy → no-calls gate).

Crucially, relaxing the gate would NOT help: disassembling the cpnos payload
(`payload.elf`) shows only **6 `push ix/iy`** total (vs 64 in AES) and **0**
existing indexed loads, and those IY uses are constant/stack manipulation, not
the loop-invariant-pointer-dereferenced-at-many-constant-offsets pattern.
**cpnos lacks the target pattern entirely.**  BIOS shares the same model
(direct BSS addressing + pointer-walking, per CLAUDE.md density analysis), so
the same conclusion applies.

**Verdict:** the IX/IY-indexed transform is a correct, validated optimization
whose benefit is intrinsic to array/crypto code (AES −136 B) where one base is
dereferenced at many constant offsets.  The production targets (cpnos/BIOS)
don't have that shape, so **Stage 3 (cross-call) is NOT worth building** — there
is nothing for it to capture.  Keep the feature flag-gated for array-heavy
workloads; do not expand it.

## Remaining work (Stage 3+) — SUPERSEDED, see verdict above

1. **Cross-call handling** (the production limiter).  The `!FnHasCalls` gate
   excludes cpnos/BIOS systems code (call-heavy).  To benefit them, allow the
   base in IX (callee-saved, survives calls) when it would otherwise be live
   across a call — or spill IY around calls.  Needs liveness, not just a
   function-level flag.
2. **Profitability gate** for default-on: only emit when the base is
   dereferenced ≥2× at constant offsets (count G_PTR_ADD+load/store users of
   the base), else single-access functions regress.
3. **Measure cpnos PROM1 / BIOS** once (1) lands.
4. Decide default-on vs keep flag-gated after (2)+(3).

## Risks

- Fallback bloat if the hint fails → net regression on fallback-heavy code.
  Mitigation: measure after S1 before widening; keep flag-gated.
- 16-bit indexed needs disp AND disp+1 ≤127 (same as the IX matcher guard).
- Don't emit for frame-index bases (the existing matcher already handles those
  via RELOAD pseudos) — only plain pointer vregs.
