# Plan: make clang faster than dcc on the 4 dcc benchmarks

**Goal (user):** clang should be *faster* than dcc on `sieve`, `e`, `ttt`, `tm`
when compiled via `zcc +cpm -compiler=llvmz80` (real CP/M .COM with z88dk clib).

**Updated 2026-07-12.** Previous version (2026-07-09) used freestanding llvm-z80
builds + ntvcm (inaccurate T-states for DD/FD/ED opcodes, ~5% low). This version
uses `zcc +cpm -compiler=llvmz80` + z88dk-ticks (cycle-accurate), which is the
correct comparison: real CP/M programs with z88dk runtime, matching how end-users
build.

Runtime note: all arithmetic helpers (`___divhi3`, `___modhi3`, `___mulhi3`) resolve
to z88dk's `l_divs_16_16x16` / `l_mulu_16_16x16` via wrappers in
`z88dk/libsrc/l/llvmz80/__divhi3.asm`. z88dk's `cpm_clib` supplies the full
runtime — none of llvm-z80's compiler-rt (`z80_rt`) is linked.

---

## Current baseline (2026-07-12, z88dk-ticks, zcc+llvmz80 vs dcc)

| program | dcc cycles | zcc -Oz | zcc -Os | zcc -O3 | dominant cost |
|---------|-----------|---------|---------|---------|---------------|
| sieve   | 27,979,152 | 34,277,588 (+23%) | 33,198,620 (+19%) | 32,952,691 (+18%) | pointer strength reduction |
| e       | 25,381,975 | 37,947,563 (+50%) | 40,321,883 (+59%) | 41,829,740 (+65%) | BSS spill traffic (M2 class) |
| ttt     |  6,346,956 | 10,423,676 (+64%) | 10,445,561 (+65%) |  7,604,026 (+20%) | call overhead at -Oz/-Os; BSS at -O3 |
| tm      | 79,435,464 |300,141,986 (+278%) |292,231,444 (+268%) |272,222,718 (+243%) | allocator pattern (not codegen) |

Note: -Oz is smaller but slower than -Os on sieve/ttt/tm; -Oz beats -Os on `e`
(50% vs 59%) — likely because a more aggressively size-reduced body has fewer
BSS-resident temporaries.

---

## Root-cause analysis (verified on zcc+llvmz80 assembly, 2026-07-12)

### sieve — pointer strength reduction (unchanged from 2026-07-09)

The `for (k=i+prime; k<=SIZE; k+=prime) flags[k]=FALSE;` inner loop is the
hot path. zcc+llvmz80 -Os emits **97 T-states/iteration**:

```asm
.LBB0_5:
    ld   hl,_flags   ; 10T  RELOAD global base address every iteration
    add  hl,bc       ; 11T  bc = integer index k
    xor  a           ;  4T
    ld   (hl),a      ;  7T  flags[k] = 0
    ld   l,c         ;  4T  shuttle bc -> hl
    ld   h,b         ;  4T
    add  hl,de       ; 11T  k += prime
    ld   c,l         ;  4T  shuttle hl -> bc
    ld   b,h         ;  4T
    ld   hl,8191     ; 10T  RELOAD limit constant every iteration
    ld   a,c         ;  4T
    sub  l           ;  4T
    ld   a,b         ;  4T
    sbc  a,h         ;  4T  16-bit index compare
    jr   c,.LBB0_5   ; 12T
```

dcc emits **39 T-states/iteration** by pointer-walking:

```asm
L52:
    ld   (hl),0      ; 10T  hl IS the walking pointer
    add  hl,de       ; 11T  de = prime (byte stride)
    ld   a,h         ;  4T
    cp   b           ;  4T  compare pointer high byte vs end.hi
    jp   c,L52       ; 10T  fast path: high bytes differ
    jp   nz,L54      ; 10T  h > b: done
    ld   a,l         ;  4T
    cp   c           ;  4T  high bytes equal: compare low bytes
    jp   c,L52       ; 10T
```

dcc pre-computes `p = &flags[i+prime]` and `end = &flags[8191]` outside the
loop; HL walks the array, DE holds the stride, BC holds the end pointer — all
loaded ONCE. Clang reloads the base address and the limit every iteration and
uses an integer index shuffled bc<->hl for address computation.

`Z80LoopInstrFormPrep` (#250) is the incomplete fix: it strength-reduces the
store pointer but still reloads stride and end-pointer constants per iteration
and increases register pressure; the net effect is SLOWER and LARGER. It is not
the solution as-is.

**Achievable:** 149,990 iterations × 39T + (33.2M − hot_cycles) ≈ **~20M
cycles → beats dcc's 28M.** A correct pointer-walk fix is the single highest
ROI lever.

---

### e — BSS spill traffic (CORRECTED from 2026-07-09)

**Previous analysis was wrong on two counts:**

1. The old claim "two divmod calls per iteration" is FALSE. clang emits ONE call
   to `___divhi3` per inner iteration. The `___divhi3` wrapper calls
   `l_divs_16_16x16` which returns quotient in HL AND remainder in DE. The
   wrapper does `ex de,hl` so on return: DE = quotient, HL = remainder. clang
   then uses DE for `x/n` and HL for `x%n` — both from one division. This is
   correct combined divmod codegen.

2. The dominant cost is **not** divmod speed — it is **BSS spill traffic**.
   Every local variable (`x`, `n`, loop counter, `a[n]`, `a[n-1]`) is resident
   in BSS via `+static-stack`. Each 16-bit load from BSS costs:

   ```asm
   ld   hl,__sfrend_main   ; 10T  load BSS base pointer
   ld   de,<offset>        ; 10T  load offset constant
   add  hl,de              ; 11T  compute address
   ld   e,(hl)             ;  7T  load low byte
   inc  hl                 ;  6T
   ld   d,(hl)             ;  7T  load high byte
   ```
   = **51 T-states per 16-bit BSS load** (stores are similar).

   The inner loop body contains 6–8 such load/store sequences for `x`, `n`,
   `a[n]`, `a[n-1]` — easily **300–400T of spill traffic per iteration**
   on top of the 50–80T for actual computation.

   dcc keeps `x` and the running multiply accumulator in registers and uses
   a pointer walk for `a[n]` accesses.

**Root cause:** M2 class (BSS load/store) — same family as the dominant cost in
the BIOS large functions. The TTI cost model assigns zero cost to BSS-resident
locals, so LLVM never pays to keep them register-resident.

---

### ttt — call overhead at -Oz/-Os; BSS at -O3

9 `posNfunc()` win-check helpers and a recursive `minmax`. The -O3 gap (1.20x)
is 3× smaller than the -Os gap (1.65x), confirming that **at -Oz/-Os the
dominant cost is call overhead** from 9 un-inlined leaf functions.

At -O3, clang inlines the helpers. The residual 1.20x gap is then recursion
frame overhead: each `minmax` call spills state to BSS (same M2 class as `e`).
dcc uses a simpler frame that fits more naturally in registers.

**Two levers:** (1) lower the Z80 inlining threshold for single-block leaf
functions at -Os/-Oz (TTI `getInliningThresholdMultiplier`); (2) address M2
class BSS spills for the recursive case.

---

### tm — allocator pattern (not codegen)

`tm.c` stress-tests `malloc`/`calloc`/`free` with growing blocks up to ~22 KB
live simultaneously. The linked allocator is z88dk's `HeapFree_callee` /
`HeapSbrk_callee` (a real but simple first-fit allocator), with 48 KB heap
configured in `build_zcc.sh`.

dcc links its own allocator tuned for this workload. The gap (3.4–3.8×) is
allocator-quality, not clang codegen. **tm is not a meaningful compiler metric.**

---

## Common structural theme

Three of four gaps trace to the **same two Z80 backend weaknesses**:

1. **Missing pointer strength reduction** (sieve, secondary in e/ttt):
   LLVM's LSR does not produce Z80-friendly pointer walks. Array indexing via
   integer IV + `ld hl,GV; add hl,rr` per iteration instead of a walking
   pointer pre-computed once outside the loop.

2. **BSS spill traffic — M2 class** (dominant in e, residual in ttt at -O3):
   `+static-stack` stores locals at BSS addresses. Each access costs 51T for a
   16-bit load vs 4T for a register reference. The TTI cost model does not
   charge for this so LLVM's register allocator does not fight to keep values
   live in registers.

dcc wins because it was designed specifically for CP/M Z80: it pointer-walks,
keeps loop-carried scalars in HL/DE/BC, and uses a compact frame model.

---

## Plan (highest-ROI first)

### Phase A — sieve: pointer strength reduction (target: beat dcc)

Fix or replace `Z80LoopInstrFormPrep` (#250) to produce a correct 3-value
pointer walk for byte-array loops: walking pointer in HL, stride in DE, end
pointer in BC, all hoisted outside the loop, exit test = pointer compare.

Options (A1 fix/extend #250; A2 LSR cost model; A3 late-machine peephole) —
see original 2026-07-09 analysis for detail. Prefer A2 if LSR can be steered.

Gate: production byte-identical or better, lit green, value oracle green.
**Success = sieve clang < 27.98M cycles.**

### Phase B — e: reduce BSS spill traffic (M2 class)

The BSS spill problem is systemic. Concrete levers for `e`'s pattern:

1. TTI `getMemoryOpCost` / `getCastInstrCost`: charge the 51T per BSS
   16-bit access so LLVM's register pressure model prefers register-resident
   values over BSS-resident ones.
2. Register coalescing for short-lived temporaries that are currently spilled
   to BSS immediately after computation and reloaded before use.
3. If (1)+(2) are insufficient, a late pass that sinks BSS stores into uses
   and hoists BSS loads out of loops where the value is loop-invariant.

The divmod situation is ALREADY CORRECT — one call, both values extracted.
Do not "fix" divmod for `e`.

**Success = e clang < 25.38M cycles.**

### Phase C — ttt: Z80-tuned inlining threshold at -Os/-Oz

Raise `getInliningThresholdMultiplier` (or equivalent TTI hook) specifically
for single-basic-block leaf functions on Z80 so the 9 `posNfunc` helpers
inline at -Os/-Oz. Watch code size — confirm production triplet unaffected.

**Success = ttt -Os clang < 6.35M cycles.**

### Phase D — tm: document as allocator-bound, do not chase

Not a codegen gap. Document as "allocator-bound; z88dk heap vs dcc's allocator"
and exclude from the compiler-quality signal.

---

## Sequencing & gates

- Phase A first: cleanest win, archetype for the wider pointer-walk class.
- Each phase: baseline → change → re-measure with ticks (not ntvcm).
- Every codegen change needs a lit test (FileCheck pins the new loop) and,
  where the win is runtime, a test-runner fixture.
- Production triplet (rcbios/cpnos/autoload) must be byte-identical or better.
- Upstream-routing per `feedback_upstream_routing_two_targets`.

## Risks

- Phase A register pressure: 3 pairs (HL=pointer, DE=stride, BC=end) leave
  only 1 free for the store value — tight but feasible for sieve. e/ttt have
  more live values; measure per-benchmark.
- Phase B TTI changes affect all loops globally; full corpus + production guard
  needed before committing.
- Phase C inlining may grow .text at -Os; confirm with user before enabling.
