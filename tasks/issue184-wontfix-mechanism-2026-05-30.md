# ravn/llvm-z80#184 — WONTFIX, with the full mechanism (2026-05-30, session 75)

**Decision: WON'T FIX.**  The `getArithmeticInstrCost(i16)=2` cost is not a bug
to fix and not a knob to ship — it is an *inaccurate* cost whose only benefit
(on AES) is a side effect of the inaccuracy, and the same inaccuracy actively
regresses production code.  No *accurate* cost reproduces the benefit, and the
proper fix is architecturally out of reach of LLVM's cost model (see §4).

This supersedes the original issue framing ("miscompiles AES under +static-stack
/ infinite loop") — that miscompile was root-caused to two peephole bugs
(**#148** fall-through MBB check, **#185** DJNZ B-clobber), both since fixed.
With those fixed, `i16=2` is **correctness-safe**; it is held purely on the
size/quality grounds documented here.

## 1. What `i16=2` is
A one-line option in `Z80TTIImpl::getArithmeticInstrCost`: charge 16-bit integer
arithmetic = 2 (i32 = 4, i64+ = expensive) instead of the `BasicTTIImpl`
default of ~1.  Intent: tell IndVarSimplify that 16-bit arithmetic is dearer
than 8-bit, so it stops widening 8-bit loop induction variables to 16-bit.

## 2. Why it helps AES but hurts cpnos (measured, instruction-level)
The decision it drives is IndVarSimplify's IV-widening check
(`llvm/lib/Transforms/Scalar/IndVarSimplify.cpp`, ~line 645):
```
if (cost(add, wideTy) > cost(add, narrowTy)) return; // do NOT widen
```
queried at the **default CostKind = TCK_RecipThroughput**.

- **AES** IVs are genuinely 8-bit (`uint8_t i = 16; while (i--) buf[i]=…`).
  Widening them to i16 buys nothing, so suppressing the widening (i16=2 makes
  the check fire) shrinks AES.  This is the entire #184 "benefit."
- **cpnos** `_netboot_mpm` has an **i8** loop counter that IndVarSimplify, by
  default, *widens* to i16 — and on Z80 that is the RIGHT call: the widened
  value lands in a register pair and uses the 1-byte ops `inc bc` /
  `add hl,bc`.  `i16=2` suppresses that widening, leaving an 8-bit counter that
  the backend must shuttle through the single accumulator A and a BSS spill
  slot.

Confirmed by building cpnos with `i16=2` and diffing `_netboot_mpm`
(`clang/init.o`):  **78 → 84 instructions, 168 → 178 B (+10 B)**.  The two
narrowing-induced regressions:
```
; a 16-bit value split into 8-bit and reconstructed:
base:  ld bc,N           ... add hl,bc          (value in a pair; 1-byte add)
i16:   ld c,N  ...  ld e,c; ld d,N; add hl,de    (+2 insns to rebuild the pair)

; a 16-bit counter increment narrowed to an 8-bit A-shuttle + BSS spill:
base:  push bc ... pop bc; inc bc                (inc bc = 1 byte)
i16:   push af; ld a,c; ld (nn),a; pop af ...
       ld a,(nn); inc a; ld c,a                  (+4 insns, A-shuttle + BSS)
```
Production effect (final ZX0-compressed PROM1; payload byte-identical, the
growth is in the init/cold section): **cpnos PROM1 2022 → 2033 B (+11 B)**,
eating into the 2 KB hard cap.  (The old issue note said +9 B from a 2028 B
baseline.)  AES, autoload, BIOS deltas from the original note: AES09 +44 B,
autoload +16 B, BIOS −6 B.

## 3. Why no *accurate* cost fixes it
On Z80 a 16-bit add is genuinely ~1 byte: `inc bc` and `add hl,rr` are 1 byte —
the *same* as an 8-bit `inc r` / `add a,r`.  So the accurate cost is
`cost(add i16) ≈ cost(add i8) ≈ 1`, which equals the `BasicTTIImpl` baseline:
no #184, no AES win, no cpnos regression.  The AES win exists ONLY because
`i16=2` is a deliberate overcharge.  An accurate model cannot produce it.

## 4. Why clang can't model the real tradeoff (the architectural reason)
The decision and the cost that should drive it live in different compiler
phases, and the cost-model vocabulary is too coarse to bridge them:

1. **Decision is pre-allocation.**  IndVarSimplify (mid-end IR) picks IV width
   before register allocation runs.
2. **The real cost is post-allocation and pressure-dominated.**  Whether the
   add costs 1 byte (`inc bc` in a pair) or many (A-shuttle + BSS spill) is
   decided by register allocation — which hasn't happened yet and from which
   there is no feedback path back to the mid-end.
3. **The cost API is a per-(opcode,type) scalar.**  `getArithmeticInstrCost`
   returns one number keyed on operation + type.  That encodes the assumption
   *cost ≈ f(opcode, type)*, which holds on register-rich targets (x86/ARM:
   i16 and i8 adds both ~1 cycle, pressure rarely flips it).  On Z80 the
   relation is **inverted** — *cost ≈ f(allocation, register pressure)* on a
   3-pair file — and a `(opcode,type)` scalar has no slot for "in a pair vs
   spilled."  The same `add i16` is cheap or expensive depending on a downstream
   decision the scalar cannot reference.

So "model it properly" is not a missing Z80 hook — it would require either
allocation-aware costs (a change to LLVM's cost-model contract affecting every
target), a mid-end↔regalloc feedback loop (which LLVM deliberately avoids), or
moving the IV-width decision into a Z80-specific allocation-aware (post-RA or
post-RA-informed) pass.  The last is the only tractable path and is real,
fragile work: it is essentially a smarter `Z80NarrowIV` (built #73n, removed
#73q; its earlier incarnations hit the miscompiles #169/#170/#171), and it must
narrow *only* IVs widened from i8 purely for addressing (the AES pattern) while
leaving genuine 16-bit counters in pairs (the cpnos pattern).

## 5. Outstanding caveat
There is also an open **+static-stack miscompile** associated with the
narrowed-IV shape (a MIR/BSS-lowering issue, not a cost-model one) noted in the
phase-B2 bisect.  It would have to be root-caused before any narrowing pass
could ship even if §4's pass were built.

## 6. Bottom line
`i16=2` cannot be made both accurate and beneficial; accuracy = baseline.  The
benefit is bounded (a small AES gain) and the cost is a real production
regression on the size-constrained targets that are the project priority.
Closed **WONT-FIX**.  Revisit only if someone builds the allocation-aware
narrowing pass of §4 *and* fixes the §5 miscompile — at which point it stops
being a cost-model question entirely.

Code reference: the held-decision comment in
`llvm/lib/Target/Z80/Z80TargetTransformInfo.cpp` (getArithmeticInstrCost) points
here.
