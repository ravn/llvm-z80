# Session 73q — FINAL closeout

**Date:** 2026-05-23
**Predecessors:** `session73q-closeout.md` (mid-session summary, pre-autonomous-loop).
**Scope:** Full session including the post-closeout autonomous-loop work.
**Why a second closeout:** The previous closeout was written before a substantial autonomous run that closed 8 more issues; recording the final state.

## Headline outcomes (full session)

1. **Z80NarrowIV pass removed** (commit `59bc5533f9c9`) — closed #169/#170/#171.  ~430 LOC down.
2. **#182 root-caused and fixed** (commit `6dc359f0c63c`) — UPSTREAM LLVM bug in `deleteDeadLoop`.  ~20-LOC fix, target-agnostic, queued for upstream-LLVM PR via #186.
3. **Track C audit harvest** — closed 5 audit items via cleanup + reclassification: #15 (peephole removed), #154 (TableGen flag-clean), #109 (BC safety), #108 (FLAGS-dead × 4), #138 (POP rr compensation).  C2 audit-table updated and #180 audit re-scoped.
4. **Other closures**: #153 (already-fixed obsolete), comment-refresh on 5 issues.
5. **New tracking issues filed**: #187 (peephole pipeline-barrier meta), **#188** (#132 family closeout coordination).

## Issues touched (full session)

**Closed: 11**
- #169, #170, #171 (Z80NarrowIV obsolescence)
- #153 (already-fixed obsolete)
- #15 (peephole removed; audit re-classified Migrate -> Delete-by-obsolescence)
- #154 (reg-reg LD flag-clean)
- #109 (BC dead-after for ADD HL,rr)
- #182 (upstream deleteDeadLoop fix)
- #108 (FLAGS-dead checks × 4 sites)
- #138 (liveness-driven 1B compensation)

**Refresh comments / triage**: #184, #177, #120, #100, #20, #180 (×2), #182 (×2), #186 (#182 added to queue), #139 (status check), #125 (deferred), #143 (skipped).

**Filed new**: #187 (pipeline-barrier meta), **#188** (#132 family closeout coordination).

## Commits

### llvm-z80 (12 since session start, all on main)
1. `a8aa4bb24693` opening triple drill writeups
2. `59bc5533f9c9` **remove Z80NarrowIV** (~430 LOC)
3. `dbd98576a7f0` A1 patch attempt — NEGATIVE writeup
4. `46446c7e1953` ISel: G_XOR i8 imm 0xFF → CPL (C1)
5. `cc8baecdbbb9` C2 audit-table update
6. `76a9b99074da` open-issue triage (all 65)
7. `b3564567be54` **remove peephole #15** (~96 LOC)
8. `421639105089` **#154 fix** (TableGen reg-reg LD flag-clean)
9. `d0d889c6a93e` **#109 fix** (BC dead-after)
10. `6dc359f0c63c` **upstream LoopUtils #182 fix** (~20 LOC)
11. `f4ea8e48cd83` **#108 fix** (FLAGS-dead × 4 sites)
12. `1cf873c80d0f` **#138 fix** (POP rr compensation)

### Workspace (`/Users/ravn/z80`)
Corresponding submodule-pin bumps after each llvm-z80 commit (~12 bumps + the memory and z88dk-pin commits from session start).

## Codegen state at close

| Surface | Pre-session | Session close | Delta |
|---|---|---|---|
| Lit suite | 108 PASS + 3 XFAIL | **110 PASS + 3 XFAIL = 113** | +2 PASS (new lit tests for xor-ff-to-cpl + issue-182) |
| test-runner clang | 681/46/56/207 (stale baseline) | **990/689/38/56/207** | new tests added since baseline; zero per-test diff between session-start state and session-close state (after Option B) |
| cpnos PROM1 (clang) | 2028 B | **2029 B** | +1 B (sum of pipeline-ordering effects across the 8 fixes; #154 + #138 individually net positive but cumulative was −1+1+0+0... see per-commit table below) |
| AES `aes256.c -Oz` `.text` | 3299 B | **3299 B** | byte-identical |
| Backend LOC | ref | **−~530** | ~430 Z80NarrowIV + ~96 peephole #15 + 20 misc |

### cpnos PROM1 history this session
| Commit | cpnos | Delta from previous |
|---|---|---|
| Session start | 2028 | — |
| #169/170/171 closed (Z80NarrowIV removed) | 2029 | +1 (pipeline barrier loss) |
| C1 i8 G_XOR → CPL | 2028 | −1 (recovered) |
| #15 peephole removed | 2027 | −1 |
| #154 reg-reg LD flag-clean | 2029 | +2 (pipeline ordering) |
| #109 BC dead-after | 2029 | 0 |
| #182 deleteDeadLoop fix | 2029 | 0 |
| #108 FLAGS-dead × 4 | 2028 | −1 |
| #138 POP rr compensation | 2029 | +1 |
| **Final** | **2029** | **+1 net from session start** |

cpnos PROM1 is at 19 B free under the 2 KB hard cap.  Within budget.  The +1 B net is from accumulated pipeline-ordering side effects (cf. **#187**); per-fix code is correct.

## Key methodology lessons

1. **"Delete and measure" before designing migrations.**  Multiple #180 audit "Migrate" candidates turned out to be obsolete or to need only safety hardening, not full migration.  Confirmed three times: Z80NarrowIV (#169/170/171), peephole #15, peephole #6 (XOR FF -> CPL partial migration).  Audit LOC-saving estimate revised from ~2300 to ~1500-1800.

2. **Pass-pipeline ordering side effects are real and small.**  Filed as #187.  Each TableGen flag change or peephole-loop addition/removal shifts downstream decisions by ~1 B on cpnos.  Generally not worth chasing individually; AES is unaffected.

3. **Dump IR at every pass boundary BEFORE hypothesizing.**  The #182 root-cause emerged in 50 min once I dumped IR pass-by-pass.  My initial A1 hypothesis (LoopRotate) was wrong; actual bug was `deleteDeadLoop` two passes earlier.

4. **Defensive patches don't compose cleanly with brittle pass interactions.**  The A1 SCEV cycle-detection patch was correct in isolation but caused downstream `LoopDeletionPass` to assert when given `SCEVUnknown` for an expected-IV.  Sometimes the cheap-defensive-fix isn't actually cheap.

## Next-session priority queue (refined)

In rough priority order:

### Track A (U-LLVM upstreaming)
- **A1 follow-up**: **#186 upstream submission for #182 deleteDeadLoop fix**.  Target-agnostic; needs ~2 h reduce + ~1 h PR mechanics.  Highest leverage upstream-LLVM contribution.
- A2/A3/A4/A5: existing locally-landed patches (#168, #163, #165, #164, #128) ready for upstream packaging.  Each ~30-60 min.

### Track B (correctness)
- **B7 (#2 hl inline-asm IRTranslator crash)**: ~30 min drill.
- **B8 (#184 i16=2 root cause 2)**: still open as tuning decision.
- **B9 (#27 per-pair 16-bit copy cost)**: last Cluster A item; ~half-day.

### Track C (audit completion)
- **#188 #132 family closeout** (newly filed): coordinate #143, #155, #139, #140.  Half-day batched.
- **C3 (#181 DAGISel/GISel coexistence)**: drill — confirm whether `Z80ISelLowering.cpp` is dead code.  Half-day.
- C4 first real migration from the remaining 11 Migrate candidates: pick #24 (BC ping-pong, ~340 LOC) or #21 (known-immediate A tracking, ~200 LOC).

### Track D (largest size opportunity)
- **#176 + #40 per-function +static-stack decision**: 2-4 h.  ~30% of AES bin gap vs SDCC.

### Track D' (lower priority)
- **#178 implicit-physreg-output remat**: half-day.  Blocks #166.
- **#125** Z80LateOptimization -O0 crash with +static-stack +shadow-regs: investigation deferred.

## Open issue count

- Started session: 65.
- Closed: 11 (#169/170/171 + 8 in the autonomous loop).
- Filed: 2 (#187, #188).
- Net open: **56**.

## State of the tree

```
main (workspace, /Users/ravn/z80):
  ... 14 commits ahead of session start ...

main (llvm-z80):
  1cf873c80d0f  Z80: #138 liveness-driven 1B compensation
  f4ea8e48cd83  Z80: add FLAGS-dead checks to 4 peepholes (#108)
  6dc359f0c63c  Upstream LoopUtils: fix deleteDeadLoop #182
  d0d889c6a93e  Z80: enforce #109 safety
  421639105089  Z80: fix #154
  b3564567be54  Z80: remove dead peephole #15
  76a9b99074da  tasks: session 73q triage
  ... 5 earlier this-session commits ...

Working tree: clean.  Submodules show "modified content" for the
persistent local state (rc700-gensmedet runtime artifacts, z88dk
NO_GMP macOS patches).
```

## Recommended next move

**Start with the #186 / #182 upstream-LLVM submission.**  The fix is in hand, the lit test is in hand, the writeup explains the root cause.  The remaining work — reducing the lit test to be target-agnostic and bisecting upstream LLVM main to confirm the bug — is mechanical.  Once the PR is open, the contribution clock starts on upstream review, which is the slowest variable in the Track A pipeline.

Everything else can wait.
