# Plan — static known-bits-driven division-routine selection (#244)

**Status:** DESIGN / PLANNED (not implemented).  Prereq shipped separately:
the speculative `_fast` repeated-subtraction routine (option **A**, see
"Shipped baseline" below).  This document is the thorough design for option
**C** (and its superset **D**) so a later session can implement it without
re-deriving the analysis.

**Owner concept:** at `-O3`, pick the i16 div/mod runtime routine *per call
site* from statically provable operand ranges, so `-O3` is **never slower than
`-Os`** while still using the fast repeated-subtraction routine wherever it is
provably (or speculatively) beneficial.

---

## 1. Background & evidence (this session, 2026-07-14)

The `-O3` speed lever for #244 is a `_fast` division routine.  Its algorithm is
**repeated subtraction** (dcc's DRSU/D16U shape): subtract the divisor up to a
cap, counting the quotient; on cap, fall back to a bounded bit divider.

Measured on dcc's `e.c` (digits of *e*), z88dk-ticks, ret-only BDOS, identical
harness for every row:

| workload | `-Os` | `-O3` speculative (cap16 + tail-call) |
|---|---|---|
| `e` (quotients ~≤11) | 34.73 M | **17.54 M**  (−49.5 %, digits byte-identical) |
| large quotient (`num/3` ≈ 20000) | 4.96 M | 7.88 M  (**≈1.6× slower than `-Os`**) |
| small quotient (`num/7000` ≈ 8) | 7.13 M | 2.40 M  (3× faster) |

dcc `E.COM` on the same harness = 25.37 M; clang `-O3` on `e` is ~31 % faster.

**The problem this plan solves.**  Repeated subtraction is *speculative*: for a
large quotient it wastes up to `cap` subtractions (~`cap`×~43 T) before it can
tell the quotient is large, then runs the same divider `-Os` would have used.
So `-O3` regresses vs `-Os` on large-quotient division.

**Two facts that make the regression matter more than expected (verified):**

1. The Z80 backend does **not** strength-reduce division by a constant.  Even
   `x / 10` emits `jp ___udivhi3_fast` (checked `-O3 -S`).  So decimal
   formatting of large values hits the runtime routine and pays the tax.
2. `__builtin_assume(x < 16u*n)` does **not** currently help.  The assume
   survives into IR (`llvm.assume`), but nothing consumes "quotient < 16": the
   division stays a full i16 libcall, and GISelKnownBits does not read
   `llvm.assume` at instruction-selection time anyway.  (Checked: codegen
   byte-identical with/without the assume.)

**What is / isn't statically provable (verified against `e`'s IR):**

- The **divisor** `n` in `e` is a loop counter phi (`%14 = phi [%14-1, ...]`,
  200 → 1).  Its range `[1,199]` **is** provable (SCEV/LVI territory).
- The **quotient** is *not* provable small: `quotient = x/n` needs an *upper*
  bound on the dividend `x`, and `x` is a recursive phi fed by an unbounded
  array load (`a[n-1]`, no `!range`).  Also `n`'s *lower* bound is 1, so even a
  known `n ∈ [1,199]` only gives `x/n ≤ x` = unbounded.  `e` stays fast solely
  because the *runtime* data invariant `x < ~11·n` holds — a property no
  automatic range analysis derives.

**Consequence (the honest limitation):** pure static selection can *remove the
regression* for the provable cases and give *guaranteed wins* for provably-small
cases, but it **cannot reproduce `e`'s win**, because `e`'s small quotients are
not statically provable.  Static analysis and the `e` win are partly orthogonal.

---

## 2. Existing precedent in the backend

`Z80InstructionSelector::tryNarrowSDivMod16` (Z80InstructionSelector.cpp:655)
already does *compile-time magnitude-based routine selection*: when **both**
operands are `G_SEXT` from i8, it proves they fit in 8 bits and emits the cheap
inline 8-bit `SDIV8`/`SMOD8` instead of the i16 libcall.  This plan generalises
the same idea from the `G_SEXT`-pattern match to a **known-bits** query, and
applies it to routine *choice* (`_fast` vs bounded) rather than width.

The routine choice today lives in `selectDivModRuntimeName(MF, STI, Base)`
(Z80InstructionSelector.cpp ~line 373), called from both `selectRuntimeLibCall16`
(non-fused G_SDIV/UDIV/SREM/UREM) and the fused G_SDIVREM/G_UDIVREM path
(~line 5851).  Today it returns `_fast` for *every* div/mod site at `-O3`.  This
plan makes that decision operand-range-aware.

---

## 3. The mechanism

Query **GISelKnownBits** for the div/mod operands at selection time and derive:

- `dividendMax` = `(1 << (16 - KB.getKnownBits(dividend).countMinLeadingZeros())) - 1`
  — the largest value the dividend can take.
- `divisorConst` = `getIConstantVRegValWithLookThrough(divisor)` if the divisor
  is a G_CONSTANT (exact value), else `divisorMin`/`divisorMax` from known bits
  (note: known-zero *high* bits give an *upper* bound; a useful *lower* bound is
  usually only available from a constant).

From these, bound the quotient:

- `quotientMax = dividendMax / max(divisorMin, 1)`.
- If `divisor` is a constant `c`: `quotientMax = dividendMax / c` (tight).
- If only `dividendMax` is known and divisor unknown: `quotientMax = dividendMax`
  (since divisor ≥ 1) — useful only when the dividend itself is small.

### Wiring GISelKnownBits into the selector

The selector does not currently hold a `GISelKnownBits*`.  Add it the standard
way: request it in `getAnalysisUsage`/the ISel pass setup and store a pointer on
`Z80InstructionSelector` (mirror how other targets do, e.g. AArch64's
`KnownBits` member initialised in `setupMF`).  Guard every use with a null check
so selection still works if the analysis is unavailable.

---

## 4. Policy — two variants on the same mechanism

The known-bits query is shared; only the *policy* differs.

### C (strict "never worse than `-Os`")

```
routine = _fast   iff   quotientMax <= CAP        // provably small → pure win
        = bounded otherwise                        // never worse than -Os
```

- Guarantees `-O3` ≥ `-Os` everywhere (bounded routine == the `-Os` routine).
- Wins where provable (narrow dividend, or constant-divisor with small
  quotient, or both-operands-8-bit).
- **`e` gets no win** (its quotient is unprovable) — accepted under this policy.

### D (superset: keep the `e` win, drop only the provable tax)

```
routine = bounded iff   quotientMin > CAP         // provably LARGE → skip speculation
        = _fast   otherwise                        // small OR unknown → speculate
```

- `provably-large` needs a *lower* bound on the quotient: `dividendMin /
  divisorMax > CAP`.  Achievable mainly when the dividend has known **low**
  structure giving a min, or (more usefully) via a future middle-end that turns
  `__builtin_assume`/`!range` into a quotient lower bound.
- Unknown sites (incl. `e`) keep the speculative `_fast` → **`e` win preserved**.
- Provably-large sites avoid the tax.
- This is the coherent end-state that composes with shipped option **A**.

**Recommendation:** implement the shared known-bits query once, expose both a
`quotientMax` and `quotientMin` estimate, and choose policy **D** as the default
(keeps `e`; removes tax where provable), with **C**'s provably-small branch also
firing (pure `_fast`, and it could even use a cap-free variant there since the
quotient is bounded).  Pure **C** remains selectable if a "never-speculate"
policy is ever wanted (e.g. a `-z80-div-no-speculate` flag).

---

## 5. Provable conditions to detect (priority order)

1. **Both operands 8-bit** — already handled by `tryNarrowSDivMod16` for signed;
   add the unsigned analogue and/or fold into the known-bits path
   (`dividendMax ≤ 255 && divisorMax ≤ 255`).  Cheapest, highest-frequency.
2. **Narrow dividend** — `dividendMax ≤ CAP` ⟹ `quotientMax ≤ CAP` for any
   divisor ⟹ pure `_fast`.  Sources: `G_ZEXT` from i8, `G_AND` with a small
   mask, a load with `!range`.
3. **Constant divisor** — exact `c`; combine with `dividendMax` for a tight
   `quotientMax = dividendMax / c`.  Also the natural hook for a *real* future
   win: udiv-by-constant strength reduction (magic multiply) — but Z80 lacks a
   fast 16×16→hi multiply, so that is a separate investigation, not this plan.
4. **Provably-large (policy D)** — `dividendMin / divisorMax > CAP` ⟹ bounded
   routine, no speculation.  Weakest source today (needs a dividend *min*);
   strongest once option **E**'s middle-end feeds `assume`/`!range`.

---

## 6. Implementation steps

1. Add `GISelKnownBits *KB` to `Z80InstructionSelector`; initialise in `setupMF`
   / request the analysis; null-guard all uses.
2. Add a helper `estimateDivQuotientRange(dividendReg, divisorReg) ->
   {min, max}` using `KB->getKnownBits` + `getIConstantVRegValWithLookThrough`.
   Return conservative `{0, UINT16_MAX}` when unknown.
3. Change `selectDivModRuntimeName` (or its callers) to take the two operand
   regs + `MRI`/`KB` and apply the policy-D decision.  Keep the current
   `-O3 && !SM83 && !optsize` gate as the outer condition.
4. Update both call sites: `selectRuntimeLibCall16` (pass Src1/Src2 regs) and
   the fused G_SDIVREM/G_UDIVREM path (~line 5851).
5. Signed wrappers: `__divhi3`/`__modhi3` route to `___udivhi3(_fast)` after sign
   handling; make the fast/slow choice on the *magnitude* of the operands
   (abs), i.e. use `dividendMax`/`divisorMax` of the absolute values.  Simplest:
   apply the same known-bits bound to the signed operands (sign bit unknown just
   widens the range → conservatively picks bounded, which is safe).

---

## 7. Test plan (red-green, per repo discipline)

- **Lit** (`llvm/test/CodeGen/Z80/issue-244-div-static-select.ll`): FileCheck
  that each provable shape selects the expected routine:
  - narrow dividend (`x & 0x0F` / `zext i8`) at `-O3` → `___udivhi3_fast`;
  - full-width unknown i16 `/` i16 at `-O3` → `___udivhi3_fast` (policy D keeps
    speculation) **or** `___udivhi3` (policy C) — pin whichever is chosen;
  - provably-large (`(x|0x8000) / smallconst`, dividend min high) → bounded;
  - `-Os` → always bounded (regression guard);
  - both-8-bit → inline `UDIV8` (existing behaviour, keep green).
- **Runtime oracle**: extend `test_249_i16_divmod_fast.c` (or a sibling) with
  cases that hit each selected routine; keep `-full` green (currently 0x0FFF).
- **`e`**: digits byte-identical to `-Os` and to *e*; timing unchanged under
  policy D (still speculative), or documented regression to `-Os` speed under
  policy C.
- **Large-quotient bench** (`num/3` shape): under policy D, provably-large must
  drop back to `-Os` speed (tax gone); under policy C, `-O3 == -Os`.

---

## 8. Limitations & links to E

- **GISelKnownBits does not read `llvm.assume`.**  To make `__builtin_assume(x <
  16u*n)` (or `!range`) actually drive selection, a **middle-end** step (option
  **E**) must convert the assumed/derived range into something visible at ISel —
  e.g. attach `!range` to the division result/operands, or narrow the div type,
  before GlobalISel.  That is a separate, larger piece; this plan (C/D) only
  uses ranges that GISelKnownBits can see from the def-use graph.
- Known-bits at ISel is *local*; cross-block ranges (like the loop counter's
  `[1,199]`) may not be visible without LVI.  Do not rely on them; treat
  unavailable as unknown (→ speculative `_fast` under D, safe).
- No change to `-Os`/`-Oz`/production firmware: they never use `_fast`.

---

## 9. Relationship to shipped option A

Option **A** (this session) ships the speculative `_fast` = repeated subtraction
(cap 16) + tail-call to the bounded `___udivhi3` on cap, selected for *all*
div/mod sites at `-O3`.  That is exactly policy **D** with the static
"provably-large" branch **not yet implemented** (every site treated as unknown →
speculate).  Implementing this plan is therefore *additive*: it only *removes*
`_fast` from sites where the quotient is provably large (D) or provably small
enough to use a cap-free variant (C's win branch).  No re-architecture of the
runtime routines is required.
