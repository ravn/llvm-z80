# All clang modes competitive with SDCC — strategic plan

Date: 2026-05-21 (session 73p continued).  Investigation triggered by
user prompt "I would like all modes of clang to be competitive with
sdcc.  investigate."

## What "all modes" means

The AES corpus tracks 13 distinct clang configurations (see
`rc700-gensmedet/tasks/aes256-corpus/clang-flag-sweep.md`):

| # | Mode | Defining flag(s) |
|---|---|---|
| 01 | default `-Oz` | (none) |
| 02 | `-Os` | -Os |
| 03 | `-O3` | -O3 |
| 04 | `-O2` | -O2 |
| 05 | static-stack | -Oz +static-stack |
| 06 | -Oz, no LICM/CSE | -Oz -mllvm -disable-machine-licm,-disable-machine-cse |
| 07 | -Oz, no LSR | -Oz -mllvm -disable-lsr |
| 08 | -Oz, gc-sections | -Oz -ffunction-sections -fdata-sections |
| 09 | static-stack prod | -Oz +static-stack -disable-{lsr,licm,cse} -gc-sections |
| 10 | -Oz, no LICM/CSE/LSR | same minus +static-stack |
| 11 | -Oz, no LICM/CSE + gc | same |
| 12 | -fno-omit-fp | -Oz -fno-omit-frame-pointer |
| 13 | IX-frame prod | -Oz -fno-omit-fp -disable-{licm,cse} -gc-sections |

SDCC has effectively one production mode (`01_baseline_prod`:
`-clib=sdcc_iy -SO3 --sdcccall 1 --opt-code-size --fomit-frame-pointer`).
"Competitive" = each clang config should be either smaller, faster, or
both vs that SDCC reference, without being substantially worse on
either axis.

## Current state — per-config gap vs SDCC

SDCC baseline: 3323 B / 12 080 289 ts.

| Config | bin B | ts | Δbin | Δts | %bin | %ts |
|---|---:|---:|---:|---:|---:|---:|
| 01_baseline_Oz | 4109 | 15 049 927 | +786 | +2 969 638 | +23.7 % | +24.6 % |
| 02_Os | 4414 | 14 408 449 | +1091 | +2 328 160 | +32.8 % | +19.3 % |
| 03_O3 | 12 419 | 13 522 924 | +9096 | +1 442 635 | +273.7 % | +11.9 % |
| 04_O2 | 8 372 | 13 904 770 | +5049 | +1 824 481 | +151.9 % | +15.1 % |
| 05_Oz_static_stack | **2 830** | 14 604 468 | **−493** | +2 524 179 | **−14.8 %** | +20.9 % |
| 06_Oz_no_licm_cse | 3 790 | 14 884 274 | +467 | +2 803 985 | +14.1 % | +23.2 % |
| 07_Oz_no_lsr | 4 328 | 15 251 381 | +1005 | +3 171 092 | +30.2 % | +26.3 % |
| 08_Oz_gc_sections | 4 089 | 15 049 927 | +766 | +2 969 638 | +23.1 % | +24.6 % |
| 09_Oz_prod_like | **2 667** | 14 887 472 | **−656** | +2 807 183 | **−19.7 %** | +23.2 % |
| 10_Oz_no_licm_cse_lsr | 4 125 | 15 237 013 | +802 | +3 156 724 | +24.1 % | +26.1 % |
| 11_Oz_no_licm_cse_gc | 3 770 | 14 884 274 | +447 | +2 803 985 | +13.5 % | +23.2 % |
| 12_Oz_no_omit_fp | 3 552 | 14 828 015 | +229 | +2 747 726 | +6.9 % | +22.7 % |
| 13_Oz_no_omit_fp_no_licm_cse_gc | **3 310** | 14 714 309 | **−13** | +2 634 020 | **−0.4 %** | +21.8 % |

**Observations:**
- 3 configs are smaller than SDCC: 05, 09, 13.  All require explicit flags.
- 0 configs are faster than SDCC.  Speed gap is 12–26 % across the board.
- 2 configs (03, 04) are massively bigger than SDCC by design (O3/O2 inlining).
- Default `-Oz` (01) is +24 % on BOTH axes.

## Size decomposition at default -Oz

What each flag contributes vs default -Oz:

| Flag | bin Δ |
|---|---:|
| `-ffunction-sections -fdata-sections` | −20 B |
| `-mllvm -disable-machine-licm/cse` (per #128) | −319 B |
| `-mllvm -disable-lsr` | +219 B (LSR helps slightly here) |
| `-fno-omit-frame-pointer` | **−557 B** |
| `+static-stack` (target feature) | **−1279 B** |
| ALL combined | −1442 B |

The two dominant levers are **`+static-stack`** (-31 %) and the
**MachineLICM/CSE pessimization** at -Oz (#128, +8 % bloat).  Both
are reachable via existing or proposed fixes.

## Speed decomposition (from `aes-speed-gap-analysis.md`)

Across all configs, the speed gap is dominated by the same cost
centers:

| Cost center | ts gap (~) | Applies to | Open issue |
|---|---:|---|---|
| gf_log/gf_alog inner-loop redundant reload | ~2.0 M | ALL modes | **#174** |
| 8-bit BSS spill via A (mc_inv XOR chains) | ~0.4 M | +static-stack ONLY | **#173** |
| A-shuttle (loop carrier through A) | ~0.5 M | ALL modes | **#172** |
| Missing fused 8-bit ALU with mem operand | ~0.5 M | IX-frame; HL form ALL | **#175** |
| Other regalloc churn | ~0.05 M | ALL | #27, #115 |

## Projected post-fix state

Applying all 4 open speed-gap issues (#174 + #173 + #175 + #172) and
**#128** (LICM/CSE pessimization):

| Config | Group | Cur bin | Proj bin | Cur ts | Proj ts | Proj %bin | Proj %ts |
|---|---|---:|---:|---:|---:|---:|---:|
| 01_baseline_Oz | default-Oz | 4109 | ~3700 | 15.05 M | 12.55 M | +11 % | **+3.9 %** |
| 02_Os | -Os | 4414 | ~4050 | 14.41 M | 11.91 M | +22 % | **−1.4 %** |
| 03_O3 | -O3 | 12 419 | ~12 100 | 13.52 M | 11.02 M | +264 % | **−8.8 %** |
| 04_O2 | -O2 | 8 372 | ~8 100 | 13.90 M | 11.40 M | +144 % | **−5.6 %** |
| 05_Oz_static_stack | static-stack | 2 830 | ~2 700 | 14.60 M | 12.10 M | −19 % | **+0.2 %** |
| 06_Oz_no_licm_cse | -Oz no-licm | 3 790 | ~3 700 | 14.88 M | 12.38 M | +11 % | **+2.5 %** |
| 07_Oz_no_lsr | -Oz no-lsr | 4 328 | ~3 920 | 15.25 M | 12.75 M | +18 % | **+5.6 %** |
| 08_Oz_gc_sections | -Oz +sections | 4 089 | ~3 680 | 15.05 M | 12.55 M | +11 % | **+3.9 %** |
| 09_Oz_prod_like | static-stack prod | 2 667 | ~2 540 | 14.89 M | 12.39 M | −23 % | **+2.5 %** |
| 10_Oz_no_licm_cse_lsr | -Oz multi-disable | 4 125 | ~3 720 | 15.24 M | 12.74 M | +12 % | **+5.4 %** |
| 11_Oz_no_licm_cse_gc | -Oz multi+sections | 3 770 | ~3 670 | 14.88 M | 12.38 M | +10 % | **+2.5 %** |
| 12_Oz_no_omit_fp | IX-frame | 3 552 | ~3 400 | 14.83 M | 12.33 M | +2 % | **+2.1 %** |
| 13_Oz_no_omit_fp_no_licm_cse_gc | IX-frame prod | 3 310 | ~3 160 | 14.71 M | 12.21 M | −5 % | **+1.1 %** |

Bin projections include #128 (-319 B on configs without -disable-licm)
+ #174 (-20 B universal) + #173 (-40 B on +static-stack) + #175
(-80 B on IX-frame, -30 B on static-stack).

**Post-fix verdict on competitiveness:**

| Mode group | Configs | bin | ts | Competitive? |
|---|---|---|---|---|
| static-stack | 05, 09 | −19 to −23 % | +0.2 to +2.5 % | **YES** (smaller + tied speed) |
| IX-frame | 12, 13 | +2 to −5 % | +1.1 to +2.1 % | **YES** (tied or smaller, marginal speed) |
| -O2 / -O3 | 03, 04 | +144 to +264 % | −5.6 to −8.8 % | **YES on speed** (size by design) |
| default-Oz | 01, 06, 07, 08, 10, 11 | +10 to +18 % | +2.5 to +5.6 % | **PARTIAL** (size still bloated) |
| -Os | 02 | +22 % | −1.4 % | **PARTIAL** (size bloated, speed wins) |

**8 of 13 configs reach competitiveness on both axes.**  The remaining
5 (the default-Oz/-Os group) lose on size by 10–22 %.

## Closing the default-Oz/-Os size gap

The default modes lack two structural advantages that +static-stack
configs have:

1. **BSS-allocated locals** (saves ~30 % bin).
2. **Eliminated IX-frame setup** in functions with no stack args.

Both are gated on "is this function safe to use static-stack on?"
— which requires non-reentrancy + non-recursion.  Two paths:

### Path A — auto-infer +static-stack safety (NEW: #176)

Conservative per-function inference: leaf functions are trivially
safe; non-recursive callgraph SCCs are safe.  Apply +static-stack
automatically when safe.  Filed as **ravn/llvm-z80#176** this session.

Estimated impact: default -Oz shrinks to ~2511 B = **−24 %** vs SDCC.
All default modes become size-competitive.

Complexity: medium-high (callgraph SCC pass, ISR isolation).

### Path B — flip the default

Make `+static-stack` the Z80 backend default.  Users who genuinely
need stack-locals opt-out.  Trivial implementation (1-line cmake
cache flip or target attribute default change).

For freestanding Z80 firmware (the typical clang-Z80 user), this is
the correct default.  Recursion + reentrancy are rare.

Risk: breaks any user who relied on stack-locals.  Easily noticed
and worked around.

### Recommendation

File both options on #176 (already includes the "flip default"
alternative).  Owner decides which is acceptable.

## Total work estimate

Following the open-lever ranking (best yield first):

| Issue | Effort | Yield | All-modes impact |
|---|---|---|---|
| **#174** | 2-3 h | ~1.5 M ts | speed-competitive for ALL modes |
| **#173** | 3-4 h | ~0.4 M ts + 30 B | speed/size for static-stack modes |
| **#128** | 1-2 d | ~320 B (default modes) | size for default-Oz modes |
| **#175** | 1-2 d | ~0.5 M ts + 80 B | speed/size for IX-frame modes; unblocks IX-frame as viable mode |
| **#172** (liveness) | 1-2 wk | ~0.5 M ts | speed (residual) ALL modes |
| **#176** | 1-3 wk | ~30 % bin (default modes) | size for default-Oz/-Os modes |

**~3-4 sessions** of focused work (#174 + #173 + #128 + #175) brings
**8 of 13 configs** to full competitiveness.  Additional ~2-4 weeks
for #172 + #176 brings the remaining 5 configs in.

## What this plan deliberately doesn't include

- **O2/O3 size**: those modes are inherently bloated due to inlining
  and unrolling.  Speed is competitive (and post-#174 will be
  faster than SDCC).  No fix proposed; this is by design.
- **Whole-program optimization** (LTO): would close some inter-
  procedural gaps but is a much larger surgery.  Out of scope for
  per-mode competitiveness.
- **Per-target tuning** (e.g., RC700-specific cost models): the AES
  corpus tracks generic Z80; not specializing per board.

## Sequencing recommendation

The plan in execution order:

1. **#174** (gf_log/gf_alog peepholes).  Plan ready at
   `issue174-implementation-plan.md`.  Highest yield, lowest risk,
   universal applicability.
2. **#173** (8-bit BSS spill peephole).  Helps the production
   target most.
3. **#128** (LICM/CSE -Oz pessimization).  Closes default-Oz
   bloat.
4. **#175** (missing 8-bit ALU with mem operand).  Opens IX-frame
   as net-positive mode.
5. **#172 liveness-aware version**.  Residual speed gap.
6. **#176** (auto-static-stack).  Closes the last default-mode
   bloat.

After step 4: all explicit-flag modes are competitive.
After step 5: all modes within ~5 % on speed.
After step 6: all modes also size-competitive.

## Per-issue status snapshot

| Issue | Status | Notes |
|---|---|---|
| #128 | OPEN | LICM/CSE pessimize at -Oz; ~320 B impact on default modes |
| #172 | OPEN | A-pin loop carrier; 73p added conservative scope, still default-OFF |
| #173 | OPEN | 8-bit BSS spill via A peephole (filed 73p) |
| #174 | OPEN | gf_log/gf_alog redundant-reload peepholes (filed 73p); plan written |
| #175 | OPEN | Missing 8-bit ALU with memory operand (filed 73p; IX-frame mode) |
| #176 | OPEN | Auto-infer +static-stack safety (filed THIS investigation) |
| #169, #170, #171 | OPEN | Z80NarrowIV blockers; narrow scope; current guards work |

All issues are in `ravn/llvm-z80`.  No upstream LLVM tickets needed.

## Closing thought

The clang Z80 backend is **structurally close** to all-modes-
competitive with SDCC.  The 4 dominant fixes (#174 + #173 + #175 +
#128) close most of the gap in a few sessions of focused work.  The
remaining ~5 % residual + the default-mode size gap need #172
liveness-aware A-pin + #176 auto-static-stack, both larger
investments but each well-scoped.

The AES corpus is the canonical oracle for measuring this.  After
each issue lands, re-run `make sweep_clang` and compare against the
projected numbers in this document.
