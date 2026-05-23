# Session 73q — Open-issue triage

**Date:** 2026-05-23
**Scope:** All 65 open issues on `ravn/llvm-z80` as of session-73q close.
**Method:** Read each issue body + comments, classify against session-73q state and the C1/C2 audit, decide action.

## Classification key

- **REFRESH** — issue is still valid but body/comments are stale; need a status comment with current findings.
- **ACTIVE** — recently touched (this session or one before); no action needed.
- **STAY** — open, accurate as filed, no session-73q-relevant change.
- **CLOSE** — obsoleted by recent work.
- **META** — tracker / meta issue, not a single bug.

## Table

| # | Title (short) | Class | Note / session-73q delta |
|---|---|---|---|
| 187 | peephole MBB pipeline-barrier meta | META | Filed this session. |
| 186 | Upstream-submission queue | META | 73p closeout. |
| 185 | i16=2 -Os/-O2 AES halts after ~28 ts | ACTIVE | 73p Phase 2 ship deferred i16=2; still applies. |
| 184 | i16=2 +static-stack infinite loop | REFRESH | Status updated: i16=2 path SHIPPED with safety guards in 73p Phase 2 (#185 fix), but i16=2 default stays off pending #185 root cause 2. |
| 183 | corpus libc-dependent benchmarks | STAY | Blocked on #35 (no libc). |
| 182 | SCEV SmallVector overflow | ACTIVE | Long comment today: real cause is upstream LoopRotate, not SCEV. |
| 181 | DAGISel/GISel coexistence audit | STAY | Queued as C3 next session. |
| 180 | late-opt peephole audit tracker | ACTIVE | Long comment today with C2 reclassification. |
| 178 | pseudos w/ implicit physreg outputs break remat | STAY | Real upstream issue, queued. |
| 177 | No Z80 TTI | REFRESH | PARTIALLY SHIPPED 73p Phase 2 commit `541b687bbecc` (Mul=Expensive, trunc/zext free, prefersVectorizedAddressing=false). i16=2 remains the open piece (filed as #184). Note: Z80NarrowIV removed this session (#169-#171 closed) precisely because the shipped 73p Phase 2 hooks make LSR canonicalize the loops directly. |
| 176 | auto-infer +static-stack safety per-function | STAY | Active design issue. |
| 175 | 8-bit ALU mem-operand (XOR/AND/OR (HL)/(IX+d)) | STAY | Active codegen gap. |
| 173 | 8-bit BSS spill via A — 6 B → 2 B | STAY | Active investigation queued. |
| 172 | 8-bit ALU accumulator should live in A | STAY | Active investigation queued. |
| 166 | ADD_HL_rr / LD_HL_a16 remat | STAY | Blocked on #178 (implicit physreg outputs). |
| 164 | TruncInstCombine zext re-insertion cost | STAY | Real gap; #163/#165 land local patches; #164 is the open cost-model design. |
| 159 | rj_sb_inv silent miscompile (uninit E) | STAY | Real correctness bug, queued as B2. |
| 158 | K&R int promotion disables u8 rotate | STAY | C-level shape; needs zext probe redesign. |
| 155 | #132 UsedElsewhere over-conservative for coalesced slots | STAY | #132 follow-up. |
| 154 | reg-to-reg copy opcodes carry spurious mayLoad/mayStore | STAY | Real flag-noise bug. Memory rule `feedback_z80_copy_spurious_mem_flags` documents the workaround. |
| 153 | stray errs() print in #132 peephole | STAY | Trivial cleanup. ~5 min fix. |
| 152 | #147 follow-up: SET/RES through A-readers | STAY | Real peephole opportunity. |
| 151 | #144 follow-up: redundant `and 1; rrca; sbc a, a` | STAY | Real peephole opportunity. |
| 150 | #142 follow-up: sub_lo extraction breaks polypascal-test | STAY | Open correctness risk. |
| 146 | callee-cleanup epilog could use EX (SP),HL | STAY | -2 B per fire. |
| 143 | #132 second-fire breaks first fire's edge-split | STAY | #132 follow-up. |
| 140 | #132 add .mir lit coverage | STAY | Test debt. |
| 139 | #132 succ-gate-passes-but-no-delta candidate | STAY | Diagnostic loose end. |
| 138 | #132 liveness-driven 1B compensation | STAY | #132 follow-up. |
| 137 | test-runner port-1 stdout capture | STAY | Tooling UX. |
| 136 | pre-existing O1 miscompile 38 failures | STAY | Sweep confirmed 38 still at HEAD. Stable baseline noise. |
| 133 | #131 follow-up: z80_preserves_regs callee-side | STAY | #131 caller-side landed. |
| 132 | cross-MBB BSS-spill→PUSH/POP | STAY | Multi-fire open. |
| 131 | z80_preserves_regs attribute | STAY | Caller-side complete; callee-side at #133. |
| 130 | memset_pattern via LDIR-overlap | STAY | Real codegen opportunity. |
| 127 | downward memmove → LDDR peephole/GISel | STAY | Investigation. |
| 126 | __builtin_memmove too large | STAY | Real PROM-budget concern. |
| 125 | Z80LateOptimization crash at -O0 +static-stack +shadow-regs | STAY | Niche but real crash. |
| 124 | cmake 4.2 + macOS pthread affinity fatal | STAY | Environment issue. |
| 123 | -g influences optimizer | STAY | Investigation. |
| 122 | i16 ULT/UGE small-const + provably-high-zero | STAY | Sibling of #118 (closed). |
| 120 | GISel combiners for #79/#93 to delete late-opt #26/#27 | REFRESH | C1/C2 reclassified #26/#27/#28 as Keep/Likely-Keep.  This issue's premise (migrate then delete) faces the #187 pipeline-barrier finding — full delete is likely a +1 B regression on cpnos PROM1. Re-scoping needed. |
| 117 | i16 EQ/NE neither-in-HL case | STAY | Real opportunity ~1 B/fire. |
| 115 | regalloc IY picks for LDIR/LDDR/HL-tied ops | STAY | Cluster A residual. |
| 114 | shadow-bank EXX bracket pass | STAY | Phase 5 work. |
| 111 | HLReg single-register class | STAY | Cluster A residual. |
| 110 | greedy regalloc copy-elim overrides hints | STAY | Cluster A residual. |
| 109 | ADD HL,rr commutativity doc/code mismatch | STAY | Trivial code-or-doc-fix. |
| 108 | peephole audit: FLAGS-after-branch gaps | STAY | Audit-driven cleanup. |
| 100 | loop rotation BSS-spill blocks #77a | REFRESH | After Option B (Z80NarrowIV removed this session), the loop-counter narrowing path is gone.  This issue's framing ("gates #77a default-on") is now historical; the underlying loop-rotation BSS-spill concern stays valid but no longer gates a removed pass. |
| 96 | regalloc-level PUSH/POP spilling (layer 3) | STAY | Long-running investigation. |
| 74 | push/pop short-lived 16-bit spills | STAY | Major design issue. |
| 70 | -fverbose-asm not annotated | STAY | Tooling gap. |
| 50 | unroll memcpy/memmove into LDI chains | STAY | Speed-critical paths. |
| 43 | custom CC for CP/M BIOS entry points | STAY | Long-term ergonomics. |
| 42 | builtin intrinsics for DI/EI/HALT/IM2/LD I,A | STAY | Related to #4. |
| 40 | IX vs static-stack per function | STAY | Related to #176. |
| 35 | no libc | STAY | Blocking #183, others. |
| 27 | per-pair 16-bit copy cost | STAY | Last Cluster A item; queued as B9. |
| 20 | BSS spill multi-value not handled | REFRESH | #132 family addressed cross-MBB single-value. Multi-value still open. Add status comment. |
| 18 | known-value register copy optimization | STAY | Old; related to constant tracking. |
| 16 | PUSH/POP instead of IX-indexed spills | STAY | Cluster A residual; relates to #74. |
| 12 | hasFP=false correct but larger | STAY | Parked design issue. |
| 7 | Z80 instruction-driven codegen (master) | META | Master tracker. |
| 4 | __critical (DI/EI wrapper) equivalent | STAY | C-language ergonomics. |

## Action breakdown

- **CLOSE this session**: **0**.  No issues were obsoleted by session-73q work (the three Z80NarrowIV-trio issues #169/#170/#171 closed during the session, not in this triage).
- **REFRESH comment this session**: **5** — #184, #177, #120, #100, #20.
- **ACTIVE / META (no action)**: **6** — #187, #186, #185, #183, #182, #181, #180, #178, #136.
- **STAY (no session-73q delta)**: **54**.

## Macro observations

1. **Most open issues are real, current, and queued.**  No "shadow backlog" of obsolete items.  The execution-plan-2026-05-22 roadmap covers the major themes (Track A upstreaming, Track B correctness, Track C audit, Track D codegen win packaging).

2. **The #132 family has accumulated 6 follow-ups** (#138, #139, #140, #143, #155, plus #20 multi-value).  Worth a "#132 closeout" pass: either land the remaining cleanups or formally park the family.  Likely a half-day session that closes 3-5 issues.

3. **The Cluster A residual count is now 4-5** (#27, #110, #111, #115; #27 is queued as B9).  Closing all four would satisfy the execution plan's Track B Tier IV trigger.

4. **The Track C (audit) outcomes** — after the C2 reclassification — suggest realistic LOC saving on the #180 peephole audit is ~1100-1500, not the original ~2300 estimate.  That's still substantial.  #181 (DAGISel/GISel coexistence) audit is independent and could close as "GISel-only" if the audit shows DAGISel paths are dead.

5. **C-language ergonomics gap**: #4 (__critical), #42 (DI/EI intrinsics), #43 (custom CC for CP/M BIOS), #131/#133 (z80_preserves_regs), #35 (no libc), #126 (memmove builtin too large).  All separate issues, all valid, no single fix-all.  Worth one focused session to close 2-3 of the smallest (e.g., #4 + #42 + parts of #126 via builtin intrinsics).

## Recommended priorities for next session

(Same as the closeout queue, but informed by the triage.)

1. **#15 re-test** (in #180): ~30 min, quick win if obsoleted by 73p.
2. **#182 LoopRotate root cause**: half-day.  Real upstream-LLVM bug.
3. **#153 + #154 cleanups**: ~1 h each.  Closes two #132-family items and a flag-noise rule.
4. **#176 + #40 per-function +static-stack decision**: 2-4 h.  Largest size opportunity remaining.
5. **#178 implicit-physreg-output remat**: half-day.  Blocks #166 + relates to multiple Cluster A items.
