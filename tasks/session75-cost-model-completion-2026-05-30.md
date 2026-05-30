# Session 75 — Z80 TargetTransformInfo cost-model completion (2026-05-30)

Goal (user): "complete the cost model" for the z80 upstream (llvm-z80/llvm-z80),
correct across **all** opt levels ("-Oz is only for size" — speed configs matter
too).  Branch `complete-cost-model-177` off `c8744ee`.  Approved plan:
`~/.claude/plans/tidy-churning-squid.md`.

## What shipped (5 commits, branch, NOT pushed/merged)

- **`a028c37` Step 0** — moved the header-only `Z80TTIImpl` bodies into a new
  `Z80TargetTransformInfo.cpp` (declarations stay in the header), matching the
  in-tree convention.  Proven no-op: AES 13-config byte-identical, lit unchanged.

- **`0296273` opt-crash fix** — `opt -mtriple=z80` aborted at startup
  ("Option 'z80-auto-static-stack' registered more than once").  Root cause: the
  `Z80AutoStaticStack` pass was registered (INITIALIZE_PASS) under `DEBUG_TYPE`,
  the SAME string as its user-facing `cl::opt` flag; `opt`'s PassNameParser
  registers every pass name as a CLI literal, colliding.  Only `opt` builds that
  parser, so llc/clang never crashed and no lit test caught it.  Fixed by
  registering the pass as `DEBUG_TYPE "-pass"`.  This unblocked ALL `opt`-based
  cost-model lit testing.  Pre-existing bug, found while building the tests.

- **`f059cc5` Step 1 — `isLegalAddImmediate`** — the only **codegen win**.  Z80
  inherits `TargetLoweringBase::isLegalAddImmediate` = true for every immediate
  (false: no `ADD rr,nn`; only `INC/DEC` for |Imm|<=3).  Override to |Imm|<=3.
  LSR stops keeping a single base IV + folded large member-offsets for
  multi-array loops (which forced an in-loop pointer-pair spill on the 3-pair
  file) and keeps pointers in registers instead.  **AES: -8 B (-Oz), -124 B
  (-Os), -42 B (-O2), -9 B (static-stack), faster t-states on every LSR-active
  config**; `-disable-lsr` and production (`09`) configs BYTE-IDENTICAL
  (production-safe by construction).  Mechanism verified by reading the
  old-vs-new asm (NOT assumed — the comment was corrected after the evidence
  showed the win is reduced register pressure, not "fewer large adds").

- **`793cee5` Step 2 — shift costs** — Z80 has no barrel shifter; charge
  `Shl/LShr/AShr` by constant amount (min(amt,8)), variable amount = expensive.
  **Codegen-neutral on every measured workload** (model-accuracy).

- **`fe8c1bc` Step 3 — cast costs** — all trunc = 0 (fixes stray i64->i32=1);
  sext scales monotonically with width (was non-monotonic: i8->i32 cheaper than
  i8->i16).  **Codegen-neutral on every measured workload** (model-accuracy).

## Step 4 (`getIntImmCost`) — SKIPPED
Base already ~TCC_Basic (accurate for cheap `LD rr,nn`); ConstantHoisting's
default correctly does not hoist on Z80; the oracle shows imm cost drives no
decision.  Implementing it = #184-style risk for zero measured benefit.

## Oracle (expanded after the user asked "do we need other oracles?")
AES is i8/i16-centric and does NOT exercise Steps 2/3 — so I added the non-AES
**compiler-comparison-corpus** (pi/fannkuch/sieve) + **BIOS** + **autoload** and
ran a true cumulative A/B (baseline `c8744ee` vs branch HEAD):

| Oracle | Result (whole series vs baseline) |
|---|---|
| AES 13-config (size+ts) | Step 1 wins on LSR configs; Steps 2/3 neutral; 13/13 PASS |
| cpnos PROM1 | 2022 B byte-identical (2 KB cap safe) |
| BIOS | 5897 B byte-identical |
| compiler-comparison-corpus | sieve 189 / fannkuch 564 / pi 875 — byte-identical |
| test-runner `-diff-opt -full` | Pass 800 / Fail 0 / Fatal 50 / no _DIFFOPT (= baseline) |
| lit | CodeGen/Z80 140 + Analysis/CostModel/Z80 3 + 4 XFAIL |

**Conclusion:** Steps 2 & 3 are codegen-neutral on every available workload — no
project code exercises variable shifts / wide-int casts in a decision-changing
way.  They ship as model-accuracy/upstream-completeness, provably non-regressing.
Step 1 is the lone density win and is confined to LSR-active array/loop code.

## New lit tests
- `llvm/test/Analysis/CostModel/Z80/{arith,shift,cast}.ll` — `print<cost-model>
  -cost-kind=all`; document the TTI costs AND guard the opt-crash fix.
- `llvm/test/CodeGen/Z80/issue-177-islegaladdimmediate-lsr.ll` — `llc -O2`
  3-array-zero loop must not spill a register pair; **verified to FAIL on the old
  always-true hook and PASS on the fix** (a real regression guard, not theater).

## Win-chase (user asked: "chase a Step-2/3 win") — conclusive NEGATIVE + 1 accuracy fix
`447d29a`.  Built CRC32 + fixed-point-MAC + variable-shift-bitfield microbenches
(designed to exercise shifts/wide-int casts) and A/B'd baseline vs HEAD:
- **-Os**: byte-identical even with EXTREME (50x) costs -> cost-driven passes
  (LoopUnroll/SimplifyCFG/CGP) do not fire at size-opt.
- **-O2/-O3**: the CRC inner loop fully unrolls (885 insns).  Only an *inflated*
  cost suppressed it (->260); the *accurate* width-scaled cost (i32 shift = 4)
  leaves it unrolled -- and fully unrolling a trip-8 loop is CORRECT for -O2
  speed.  So suppressing it would be a regression, not a win.
- **Verdict:** accurate shift/cast costs are architecturally **inert** on Z80
  (the consuming passes have no cheaper alternative to pick).  No project or
  synthetic workload turns Steps 2/3 into a codegen win.
- **Salvage:** the chase exposed that the original shift cost ignored WIDTH
  (32-bit shift charged 1, not ~4).  Fixed (width-scaling) -- strictly more
  accurate, oracle-neutral.  Net: Steps 2/3 remain correctness/accuracy only.

## Disposition (RESOLVED) — keep-but-experimental (`6dc79ac`)
User chose: keep Steps 2/3 as documented/tested model-accuracy but gate behind a
hidden, **default-off** flag `-z80-experimental-tti-costs`, so default codegen is
untouched until a workload proves them out.  Step 1 (the win) + Mul=Expensive are
NOT gated.  Verified default (flag off) AES = the Step-1 win unchanged, 13/13
PASS; cost-model lit tests pass the flag to validate the experimental costs.

7 commits total: `a028c37` `0296273` `f059cc5` `793cee5` `fe8c1bc` `447d29a`
`6dc79ac`.

## State / next
- Branch `complete-cost-model-177`, 5 commits, CI not yet run (not pushed; merge
  is a user decision).  Working tree clean.  Native build = Step 3.
- #177 was already CLOSED (partial); these commits are the follow-on completion.
- Open question deliberately left: the #184 i16/i32-Add-width charge stays HELD
  (the CostKind split cannot isolate it — it flows through IndVarSimplify's Add
  query at TCK_RecipThroughput; tied to the open +static-stack miscompile).
