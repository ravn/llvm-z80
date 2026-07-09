# Plan: make clang faster than dcc on the 4 dcc benchmarks (2026-07-09)

**Goal (user):** clang should be *faster* than dcc on `sieve`, `e`, `ttt`, `tm`.

**Current state (2026-07-09, ntvcm full-speed cycles):**

| program | dcc        | clang -Os        | gap    | dominant cost |
|---------|------------|------------------|--------|---------------|
| sieve   | 18,180,494 | 26,251,719 (1.44×) | +44%   | inner-loop pointer strength reduction |
| e       | 20,923,181 | 28,152,176 (1.35×) | +35%   | 16-bit divide/modulo runtime helpers |
| ttt     |  4,751,136 |  6,677,394 (1.41×) | +41%   | tiny helpers not inlined at -Os (call overhead) |
| tm      | 49,501,528 |180,149,702 (3.64×) | +264%  | ad-hoc malloc O(n) best-fit scan |

Each benchmark has a DIFFERENT dominant cost. This is four separate work items,
not one fix. Ordered below by ROI (cleanest/highest-confidence win first).

---

## Root-cause analysis (verified, not guessed)

### sieve — missing pointer strength reduction (VERIFIED via ntvcm -g profile)

The `for (k=i+prime; k<=SIZE; k+=prime) flags[k]=FALSE;` inner loop is **55% of
total runtime** (14.55M / 26.25M cycles; PC 354–372 executed 149,990×). Per
iteration clang emits **97 T-states**:

```asm
.LBB0_5:
    ld   hl,_flags   ; 10T  RELOAD base constant every iteration
    add  hl,bc       ; 11T  bc = integer index k
    xor  a           ;  4T
    ld   (hl),a      ;  7T
    ld   l,c         ;  4T  shuttle k: bc -> hl
    ld   h,b         ;  4T
    add  hl,de       ; 11T  k += prime
    ld   c,l         ;  4T  shuttle back: hl -> bc
    ld   b,h         ;  4T
    ld   hl,8191     ; 10T  RELOAD limit constant every iteration
    ld   a,c;sub l;ld a,b;sbc a,h  ; 16T  16-bit compare k<8191
    jr   c,.LBB0_5   ; 12T
```

dcc emits **~39 T-states** for the same loop by pointer-walking:

```asm
L52:
    ld   (hl),0      ; 10T  hl IS the live pointer &flags[k]
    add  hl,de       ; 11T  de = byte stride (prime)
    ld   a,h;cp b;jp c,L52  ; 18T  compare pointer hl vs END pointer bc
    jp   nz,L54
    ld   a,l;cp c;jp c,L52
```

dcc keeps three loop-carried values in registers for the whole loop:
`hl` = walking pointer, `de` = stride, `bc` = end pointer — all loaded ONCE
outside the loop. clang reloads the base (`ld hl,_flags`) and limit
(`ld hl,8191`) every iteration and keeps the index as an integer that it
shuttles bc<->hl to do the add.

**Why:** LLVM's LSR does not produce a Z80-friendly pointer-walk here. The
existing opt-in `Z80LoopInstrFormPrep` pass (`-mllvm -z80-loop-instr-form-prep`,
#250) is INCOMPLETE — enabling it makes sieve *slower* (29.68M vs 26.25M) and
larger (2015 vs 1964 B) because it strength-reduces the store pointer but still
reloads the stride and end-pointer constant every iteration and adds register
pressure. Measured this session.

**Achievable:** if the inner loop matched dcc (~39T), sieve = 149,990×39 +
(26.25M − 14.55M) = 5.85M + 11.70M ≈ **17.5M cycles → beats dcc's 18.18M.**
An even tighter Z80 loop (jr instead of jp) would win by more.

### e — 16-bit divide/modulo runtime helpers (issue #244)

Inner loop: `a[n] = x % n; x = 10*a[n-1] + x/n;`. Each iteration does a 16-bit
`x % n` AND `x / n` — two calls into `__modhi3`/`__divhi3`. Issue #244 already
tracks "__divhi3 ~21% behind dcc". The array accesses `a[n]`, `a[n-1]` add the
same strength-reduction gap as sieve but are secondary here. Root cause is
primarily runtime-helper speed, not codegen shape.

### ttt — call overhead (tiny helpers not inlined at -Os)

Minimax tic-tac-toe with 9 tiny `posNfunc()` win-check helpers plus a recursive
`minmax`. At -O3 (which inlines the helpers) ttt is only 1.18× behind dcc vs
1.41× at -Os — so ~⅓ of the gap is un-inlined call/return + caller-save flush.
The rest is the recursion frame (each `minmax` call sets up a static-stack
frame). dcc inlines more aggressively and has cheaper calls.

### tm — ad-hoc allocator (stub quality, not core codegen)

`heap.c`'s `malloc()` is an O(n) best-fit linear scan with no coalescing.
Session 78 profiled it at ~22% of instructions AFTER the calloc-memset fix.
This is a throwaway-stub-quality issue (issue #35 libc), not an llvm-z80 codegen
gap — dcc links a real allocator. tm is the outlier and the least
representative of compiler quality.

---

## Common structural theme

Three of four gaps trace to the same Z80 backend weakness: **loop-carried
values (pointers, strides, limits) are not kept in registers across the loop**,
and **array indexing is not strength-reduced to pointer-walking**. This is
worsened by the default `+static-stack` BSS frame (M2 in
`known-suboptimal-codegen.md`) that pushes spills to absolute BSS addresses at
16 T-states each. dcc's simpler codegen wins precisely because it pointer-walks
and register-allocates loop bodies tightly.

The single highest-leverage fix is a **correct pointer strength reduction for
Z80 array loops** — it directly wins sieve, helps e, and is the archetype for
the whole M2 class.

---

## Plan (phased, highest-ROI first)

### Phase A — sieve: complete pointer strength reduction (target: WIN)

The existing `Z80LoopInstrFormPrep` only reduces the store address. A complete
fix must, for a loop `for(k=lo; k<hi; k+=stride) base[k]=v;`:

1. Introduce a pointer IV `p = &base[lo]` incremented by `stride` each iteration
   (kept in HL or BC).
2. Introduce an end-pointer `pend = &base[hi]` computed ONCE before the loop
   (kept in a register), and change the exit test to `p != pend` / `p < pend`
   (a 16-bit pointer compare, not an index compare against a reloaded constant).
3. Ensure the stride stays in a register (DE) across the loop — no BSS reload.
4. Ensure the regalloc keeps p/stride/pend in registers (may need a
   single-register class hint, cf. #99/#111/#251, or fewer simultaneously-live
   values so the 4 pairs suffice).

Approach options (to evaluate, decision at implementation time):
- (A1) Fix/extend `Z80LoopInstrFormPrep` to also hoist the end-pointer and
  stride, and to rewrite the exit condition to a pointer compare. Then re-gate
  it on a register-pressure check and flip default-on where it wins.
- (A2) Tune the generic LSR cost model via TTI so LLVM's own LSR prefers the
  pointer-walk formula on Z80 (e.g. `isLegalAddImmediate`, `getScalingFactorCost`,
  making index-with-reloaded-base look expensive). Cleaner if it works;
  upstreamable.
- (A3) A late-machine-IR loop peephole that recognizes the `ld hl,GV; add hl,rr`
  reload-in-loop idiom and converts to a pointer PHI. Most Z80-specific.

Prefer A2 if the LSR cost model can be steered; fall back to A1. Validate with
the ntvcm profile (inner loop must drop to <=45T) and the full timing table.
Gate: production byte-identical or better (rcbios/cpnos/autoload), lit green,
runtime suite green.

**Success = sieve clang < 18.18M cycles.**

### Phase B — e: faster 16-bit divide/modulo + combined divmod

1. Profile e to confirm `__divhi3`/`__modhi3` share (expected dominant).
2. Issue #244 + #248 (i32 divrem fusion, already closed for i32) — extend the
   divmod fusion to i16 so `x%n` and `x/n` in the same loop share ONE division.
   `dcc` computes both from one divide; clang currently calls twice.
3. Speed-tune `__divhi3`/`__modhi3` asm (compare against dcc's divide routine).
4. Apply Phase A strength reduction to the `a[n]`/`a[n-1]` accesses.

**Success = e clang < 20.92M cycles.**

### Phase C — ttt: reduce call overhead at -Os

1. Confirm via profile that call/return + caller-save dominates.
2. Options: raise the inliner threshold for tiny leaf functions on Z80 (TTI
   `getInliningThresholdMultiplier` or size-based), OR make the sdcccall
   caller-save flush cheaper. Since -O3 already closes most of the gap by
   inlining, the cleanest lever is Z80-tuned inlining cost at -Os for
   single-block leaf functions.
3. Watch code size — inlining 9 helpers may grow .text; the goal is speed but
   production density must not regress (these are non-production benchmarks, so
   the tradeoff is acceptable if isolated to -Os benchmark behavior — decide
   with user).

**Success = ttt clang < 4.75M cycles.**

### Phase D — tm: allocator (lowest priority, stub-quality)

Not a core-codegen gap. Options: (i) accept it as a libc-stub limitation and
note tm is not a compiler-quality signal; (ii) if a win is wanted, give
`heap.c`'s `malloc` coalescing or a segregated free-list to cut the O(n) scan.
This is scaffolding work (issue #35), best deferred until a real CP/M libc
exists. Recommend documenting tm as "allocator-bound, not codegen-bound" and
not chasing it as a compiler metric.

**Success (if pursued) = tm clang < 49.5M cycles — requires a better allocator,
not compiler work.**

---

## Sequencing & gates

1. Start Phase A (sieve) — highest confidence, cleanest win, archetype for M2.
2. Each phase: baseline first, then change, then re-measure with the exact
   ntvcm profile + full timing table. No commit on size/lit alone — value oracle
   (test-runner runtime suite + production triplet byte-identical) required per
   `feedback_no_commit_first_version`.
3. Every codegen change ships a lit test (FileCheck pins the tightened loop)
   AND, where a runtime win is the point, a test-runner fixture.
4. Upstream-relevant pieces (LSR cost model, divmod fusion) route per
   `feedback_upstream_routing_two_targets`; explain-before-filing.

## Risks

- Phase A via LSR cost model (A2) may regress other loops — needs the full
  corpus + production triplet as regression guard, not just sieve.
- Pointer strength reduction increases register pressure; on Z80's 4 pairs this
  can backfire into MORE BSS spills (exactly why the current opt-in pass loses).
  The end-pointer + stride + walking-pointer = 3 pairs, leaving 1 for the store
  value — tight but feasible for sieve. e/ttt have more live values and may not
  fit; measure per-benchmark.
- ttt inlining may grow code; confirm the -Os size budget with the user.

## Open question for the user

tm is allocator-bound (stub `heap.c`), not codegen-bound. Beating dcc on tm
means writing a better allocator in the throwaway libc stub, which is issue-#35
scaffolding work rather than compiler improvement. Do you want tm pursued, or
is "clang beats dcc on the 3 codegen-bound benchmarks (sieve/e/ttt)" the real
goal with tm documented as allocator-bound?

