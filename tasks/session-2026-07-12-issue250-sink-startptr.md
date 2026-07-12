# #250 — kill-loop start-pointer sink (lever 1 of 3 for the sieve win)

**Date:** 2026-07-12  **Branch:** `z80-issue250-sink-startptr`

## Context

Prior sessions (Copilot) built the two-pass pointer-strength-reduction machinery,
both default-OFF:

- `Z80LoopInstrFormPrep` (IR) — turns `base[k]=v; k+=stride` into a pointer walk
  and eliminates the old integer IV (exit test becomes a pointer compare).
- `Z80PinLoopPointer` (pre-RA MIR) — pins the walk pointer to HL via a
  single-class constraint (same mechanism as the DJNZ counter split), so the
  inner loop becomes the dcc shape `ld (hl),v; add hl,de; <cmp>`.

Both fire cleanly on FLAT / 2-level loops but were declined on the sieve KILL
loop because it is nested and `-z80-loop-instr-form-prep-allow-nested` regressed
it (44.9M). This session root-caused and fixed the dominant part of that
regression.

## Measurement (sieve, zcc +cpm -compiler=llvmz80, z88dk-ticks, -Os)

dcc reference = **27,979,152**. clang baseline (no flags) = **33,029,994**.

| config | cycles | vs baseline |
|--------|--------|-------------|
| fp + nest (no pin)         | 44,908,626 | +36% |
| fp + nest + pin (pre-sink) | 36,110,292 | +9%  |
| **fp + nest + pin + SINK** | **31,053,898** | **−6%** (beats baseline) |
| dcc                        | 27,979,152 | −15% |

All variants print the correct `1899 primes.`

## Root cause of the pre-sink +9%

With prep+pin the inner kill loop is already dcc-quality (50 T/iter, HL-pinned
pointer walk, old IV eliminated — verified in IR + asm). The residual regression
was entirely in the SCAN loop: `SCEVExpander.expandCodeFor(G.Start)` recognises
the kill start pointer `&flags[k_start]` as a *scan-loop* AddRec and, per its
invariant of placing AddRec expansions at the AddRec loop's header, drops
`ld hl,_flags; add hl,de; ld (nn),hl` into the scan header — computed
unconditionally every scan iteration (before the `if (flags[i])` guard) and
BSS-spilled because it is consumed only later, in the kill preheader. ~35 T ×
~82k scan iterations ≈ +2.9M, matching the 36.1M − 33.0M gap exactly.

## The fix

`sinkStartPtrToPreheader` (in `Z80LoopInstrFormPrep.cpp`, called from
`rewriteAddrGroup` after the pointer PHI is created): move the single terminal
start-pointer instruction from wherever SCEVExpander hoisted it down into the
loop's real preheader, where it is conditional (past the guard) and
register-adjacent to the kill loop (no BSS round-trip). Block-level dominance
checks (operands properly-dominate the preheader; uses are dominated by it)
keep it SSA-safe; only the terminal GEP is moved (not the index-arithmetic
chain) so there is no operand-reordering hazard. No-op in the flat case (the
expander already lands the start pointer in the preheader).

Notes:
- The instruction-level `DT.dominates(SinkPt, PHIuser)` query is WRONG here (the
  PHI's use is on the preheader→header edge, which the preheader terminator does
  not "dominate" under the PHI-edge rule) — hence the block-level checks.
- Cascading the operand chain was tried and dropped: moving an operand after its
  already-moved user violates SSA order; the residual index arithmetic left in
  the enclosing header is cheap and register-resident anyway.

Lit: `pointer-iv-strength-reduce-sink-startptr.ll` (checks the sunk GEP lands in
`%ipre`, not the scan header). Full Z80 CodeGen suite green (191 PASS + 5 XFAIL).
Pass is default-OFF → production byte-identical.

## Remaining levers to actually BEAT dcc (not done)

The sieve win needs THREE levers; this session landed lever 1.

2. **high-byte-first inner exit test** (hand-proven): rewrite the inner
   `ld a,l; sub e; ld a,h; sbc a,d; jr c` (16 T) into dcc's
   `ld a,h; cp d; jp c; jr nz; ld a,l; cp e; jp c` (8 T fast path). Hand-edited
   asm measured **29,555,288** (jp form). SIZE-NEGATIVE (adds branches) → conflicts
   with the size-first mandate; would need to be speed-gated. Reaches parity-ish,
   does not beat dcc alone.

3. **scan-index spill** (M2 class, ISA-fundamental family): the scan loop spills
   its IV `ld (nn),de` (20 T) every iteration because the kill loop clobbers all
   three GP pairs (HL=ptr, BC=stride, DE=end). 20 T × ~82k ≈ 1.6M — the rest of
   the gap to dcc. Hard greedy-regalloc problem; size-NEUTRAL. Combined with
   lever 2 this would land ~27.9M and beat dcc.

See `plan-2026-07-09-beat-dcc-benchmarks.md` for the full decomposition.
