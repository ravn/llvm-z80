# Session 2026-07-09 — Z80SinkColdLoopIV + Z80PinLoopPointer (sieve gap, #250)

## Goal

Close the sieve cycle gap vs dcc (clang 32.98M vs dcc 27.98M T-states, +18 %
at -Os, z88dk-ticks oracle). Two nested loops in `dcc/tests/sieve.c`:

```c
char flags[8191];
for (i = 0; i < SIZE; i++)          // SCAN loop  (~82K iters)
  if (flags[i]) {
    primes++;
    prime   = 2*i + 3;              // used ONLY here (~2% taken)
    k_start = 3*i + 3;
    for (k = k_start; k <= SIZE; k += prime)   // KILL loop (~150K iters)
      flags[k] = 0;
  }
```

## What was verified (known, not guessed)

### Profiling: the KILL loop dominates, not the SCAN loop

Per-PC profile (`ntvcm -g:file.csv`, instruction-execution counts):

| region | sink-only exec | share |
|--------|---------------:|------:|
| KILL loop (150K iters) | 2,699,820 | ~65 % |
| SCAN loop ( 82K iters) | 1,064,830 | ~26 % |
| other | 408,037 | ~10 % |

So the SCAN-loop M3 cost is real but secondary; the KILL loop (M5 pointer-SR
miss, tracked in #250) is the dominant term.

### Root cause of the SCAN-loop waste = M3 (LSR hoists cold-only IVs)

`prime = 2*i+3` and `k_start = 3*i+3` are used ONLY inside the ~2%-taken
`if(flags[i])` branch, but LSR strength-reduces them into **scan-loop-carried
IVs advanced every iteration** (`inc iy`×3 + `inc de`×2). This over-subscribes
Z80's 3 register pairs → the scan counter `i` spills to BSS
(`ld bc,(__sfrend_main-2); inc bc; ...` each iter). dcc instead keeps `i` in an
IX frame and recomputes `prime`/`k_start` on demand only when `flags[i]` is true.

Confirmed NOT a tunable LSR choice: `-mllvm -lsr-complexity-limit={8,32}` does
not remove the derived IVs; TTI already reports 3 regs and NumRegs-first in
`isLSRCostLess`. LSR's AddRecCost is **block-frequency-blind** — it advances the
IV unconditionally even though every use is under a low-probability branch.

## Deliverable 1 — Z80SinkColdLoopIV (opt-in, default OFF)

`-z80-sink-cold-loop-iv`. A post-LSR IR FunctionPass that undoes exactly the M3
hoist above (the "sink the recompute into the taken branch" fix the M2 note had
theorised).

Algorithm, per natural loop L with a canonical `{0,+,1}` IV:
1. Find header phis `PN` that are affine IVs (`add PN, const` in the latch, with
   loop-invariant init) whose **all** uses are cold — no use block dominates L's
   latch.
2. Materialise `Init + Step*i` at the cold nearest-common-dominator of the uses
   (hoisted to the sub-loop preheader if the uses are nested), at
   `getFirstInsertionPt()` of that block.
3. RAUW the phi with the recompute; delete the now-dead phi + latch step-add.

Dominance-crash lesson: the recompute must be inserted at
`Ins->getFirstInsertionPt()` (after phis, before other instructions), NOT at
`getTerminator()` — a use may live IN the NCD block (e.g. the guard's
`icmp %ivA`). The canonical IV is a header phi (live at block entry), so the
first insertion point dominates all in-block and successor/phi-edge uses.

### Measured (z88dk-ticks, -Os)

| build | sieve T-states | vs dcc | correctness |
|-------|---------------:|-------:|-------------|
| dcc | 27,979,082 | 1.00× | 1899 primes |
| clang default | 32,981,811 | 1.18× | 1899 primes |
| **clang + sink** | **32,212,301** | **1.15×** | 1899 primes |
| E / TTT / TM + sink | — | ±0 % | all correct |

**sieve −2.3 %, E/TTT/TM ±0 %, all outputs correct.** Only −2.3 % because it
fixes the secondary SCAN loop; the dominant KILL loop is untouched (that is M5).

### Verification

- Red-green lit test `llvm/test/CodeGen/Z80/sink-cold-loop-iv.ll`: OFF prefix has
  the two seed IVs (`inc de` + `inc hl` in the scan latch); ON prefix has only
  `inc de`, seed IVs gone, `__sframe_scan` shrinks 4→2 bytes.
- Full Z80 lit suite: **183 PASS + 5 XFAIL**, zero regressions.
- Default pipeline **byte-identical**: pass count 0 in `-debug-pass=Structure`
  without the flag; default sieve asm diff-identical to the pre-pass build.
  → production triplet (rcbios / cpnos / autoload) unaffected.

## Deliverable 2 — Z80PinLoopPointer (opt-in, default OFF) — net-negative, documented

`-z80-pin-loop-pointer` + an `HLReg` register class. Pins an innermost
pointer-walk IV to HL so the KILL loop becomes the optimal dcc form
(`ld (hl),a; add hl,rr; <ptr cmp>; jr c`) with no `bc↔hl` shuttle. This is the
M5 / #250 fix, paired with the existing `-z80-loop-instr-form-prep`.

### Why it net-regresses (root-caused this session)

The KILL loop becomes optimal in isolation, but sieve as a whole gets WORSE:

| region | sink | sink+prep+pin | Δ |
|--------|-----:|--------------:|---:|
| KILL | 2,699,820 | 1,649,890 | **−1,049,930** (pin win) |
| SCAN | 1,064,830 | 2,375,390 | **+1,310,560** (regress) |
| other | 408,037 | 364,832 | −43,205 |
| **total exec** | 4,172,687 | 4,390,112 | **+217,425** |
| **T-states** | 32.2M | 33.6M | **+1.4M worse** |

The KILL-loop pointer-walk saves ~1.05M exec, but the SCAN loop MORE than doubles
(13→29 instr in the scan body). `prep` only touches innermost loops
(`Z80LoopInstrFormPrep.cpp:394` `if (L->isInnermost())`), so it does not add a
pointer to the scan loop directly — the scan regression is a **whole-function
regalloc cascade**: pinning HL in the kill loop + `prep`'s SCEV-expanded
pointer-inits (`flags + k_start`, stride, end-ptr) + an IY spill all land in the
hot scan path, forcing extra spill-slot juggling (`push iy/pop hl`) every scan
iteration.

VERDICT: the M5 kill-loop pointer-SR cannot be made net-positive on sieve with
the current prep+pin+sink machinery — the scan loop pays for the kill loop's
registers (the classic 3-pair over-subscription). Stays opt-in.

## Conclusion / recommendation

- `sink` is a clean, non-regressing −2.3 % partial win; landed **default OFF**
  (default-on decision reserved for the user).
- Beating dcc on sieve needs a generic middle-end fix: block-frequency-aware
  LSR AddRecCost (don't strength-reduce an IV whose only uses are under a
  low-probability branch), plus a register-pressure-aware M5 pointer-IV rewrite
  that does not steal the enclosing loop's pairs. Multi-week, generic-LLVM,
  benchmark-only payoff (production firmware already beats SDCC). Parked with data.

## Files

- `llvm/lib/Target/Z80/Z80SinkColdLoopIV.{cpp,h}` — the sink pass.
- `llvm/lib/Target/Z80/Z80PinLoopPointer.{cpp,h}` + `Z80RegisterInfo.td` (HLReg).
- `llvm/test/CodeGen/Z80/sink-cold-loop-iv.ll` — red-green lit test.
- Wiring: `CMakeLists.txt`, `Z80.h`, `Z80TargetMachine.cpp` (both gated off).
- `tasks/known-suboptimal-codegen.md` — M2 verdict updated to reference this.
- Bench tools: `scratch/dcc-clang-bench/{ticks_cpm.py,build_compare.sh}`.

Commits (local, not pushed per merge-only rule):
`[Z80] add opt-in Z80SinkColdLoopIV + Z80PinLoopPointer passes (#250)`.

## Follow-ups filed (2026-07-09)

- **ravn/llvm-z80#256** (NEW) — tracks the M3 scan-loop half: LSR strength-reduces
  cold-only IVs into the hot scan loop → BSS spill on a register-starved target.
  `Z80SinkColdLoopIV` is the Z80-side mitigation; the generic angle
  (block-frequency-blind `AddRecCost`, verified same 8 `%lsr.iv` on
  z80/x86_64/avr via `opt -passes=loop-reduce`) is held for eventual upstream
  with go-ahead. Lists the default-on gate (broader corpus + production check).
- **ravn/llvm-z80#250** comment — the M5 kill-loop rewrite (`Z80PinLoopPointer`)
  is optimal in isolation but net-regresses sieve; per-region exec table + the
  nested-loop register-pressure design constraint for a default-on M5 variant.
- **ravn/llvm-z80#251** comment — the `HLReg` single-register class this session
  added (for the pin pass) is exactly the sister class #251 predicted; noted as
  landed raw material, NOT wired to #251's `bench_word_fill.c` case yet.
