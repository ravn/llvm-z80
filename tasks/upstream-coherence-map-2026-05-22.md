# Upstream coherence map — clang/llvm-z80 (2026-05-22)

Companion to `unpark-2026-05-22.md`.  Goal: place every known clang
bug or shortcoming into a coherent picture for upstream submission.
Inputs: 65 open issues in ravn/llvm-z80 (`gh issue list --state open`),
`backend-research-not-filed.md` (24 not-filed findings), `CLAUDE.md`
known-bugs and known-deferred sections, and the session-73p Phase 3
closeout.

Each item below is tagged with **state** (Active / Blocked / Parked
/ ZeroYield) and **upstream-target**:

- **U-LLVM** — generic upstream patch (llvm/llvm-project), independent
  of Z80 target acceptance
- **U-Z80** — Z80-backend-specific patch, lands in llvm-z80/llvm-z80
  first, then llvm/llvm-project once the Z80 target is upstreamed
- **Fork-only** — workaround / experiment / Z80-only feature that
  would not go upstream in current form
- **Never** — source-/ABI-/project-side, not a compiler change

## Tier I — Generic LLVM improvements (U-LLVM, fastest to upstream)

**What kind of problem:** improvements / missing infrastructure —
code that already compiles correctly but suboptimally because some
target-agnostic pass made a wrong call (cost model, scheduling,
narrowing heuristic).

**Where the fix lives:** `llvm/lib/...` outside `Target/Z80/`
(SimplifyCFG, ScalarEvolution, TruncInstCombine, MachineLICM, etc.).
**Submittable to llvm/llvm-project today**, doesn't depend on the
Z80 target being accepted upstream.  **Highest upstream-submission
ROI per item.**

**Edge case:** #182 (ScalarEvolution crash) is in Tier I even
though it's a correctness bug, because the fix lives in generic
code.  The "where the fix lives" axis decides the tier, not the
"kind of problem" axis.

| # | Title | State | Note |
|---|---|---|---|
| **#128** | MachineLICM/MachineCSE pessimize at -Oz on Z80 | CLOSED via `disablePass()` | The fix is a workaround in `Z80PassConfig`.  **True upstream fix is a target-aware cost gate in MachineLICM/MachineCSE.**  Currently shipped as fork-only `disablePass()`; documenting the underlying bug as a U-LLVM candidate would let other tiny-register-file targets benefit. |
| **#164** | TruncInstCombine no zext re-insertion cost model | Active | AggressiveInstCombine narrowing sinks regress when re-insertion is needed.  Generic — affects every target with narrow register classes. |
| **#168** | foldTwoEntryPHINode bailout for non-predictable branches | CLOSED via 12-line SimplifyCFG cost gate (commit `cd2a2ace8754`) | Already submission-shaped; main remaining question is whether the cost gate should be more aggressive for predictable=false targets generally. |
| **#163** | AggressiveInstCombine: treat (and X, MASK) as trunc-equivalent root | CLOSED (session 73-base) | Extension landed in TruncInstCombine; **U-LLVM submission candidate as the canonical extension of #165**. |
| **#165** | AggressiveInstCombine: extend icmp outside-user to narrowable non-const | CLOSED | Same family as #163, submission candidate. |
| **#179** | GISel ISel + MachineScheduler don't reorder register-independent ops | CLOSED via new `Z80ReorderTestDec` MIR pass | Fork-only pass currently.  **True upstream fix is a MachineScheduler heuristic that recognizes the reload-after-test pattern.** |
| **#182** | ScalarEvolution SmallVector capacity overflow (crash) at -O1+ | Active | Crash in generic ScalarEvolution code.  Repro is small (sequential loops on same array).  **U-LLVM bug, submission-ready as a repro + minimal-reduction.** |
| **#123** | Investigate which optimizer decisions are influenced by `-g` | Active | Tracking-only; resolution may be a U-LLVM doc improvement or finding of a real `-g`-dependent codegen drift. |
| **#177** | No Z80 TargetTransformInfo — LSR/IndVarSimplify/MachineLICM/MachineCSE wrong | PARTIAL ship (Phase 2) | i16=2 case split out as #184.  Pieces of the TTI infra itself are generic; specific cost choices are Z80. |

**Filed as meta-tracker:**

- **#186** — `[meta] Upstream-submission queue: U-LLVM patches PRable
  to llvm/llvm-project`.  Tracks the 6-row queue including the
  previously-unfiled #128-underlying and #179-underlying bugs (the
  `disablePass(MachineLICM/MachineCSE)` and `Z80ReorderTestDec`
  workarounds are shipped fork-only; #186 captures the upstream bugs
  they work around).

## Tier II — Z80 correctness bugs (must-fix before related codegen ships)

**What kind of problem:** correctness bugs in the Z80 backend —
silent miscompiles, infinite loops, or crashes.  The code that comes
out is wrong, not just big or slow.

**Where the fix lives:** `llvm/lib/Target/Z80/` (or a Z80-specific
pass like `Z80NarrowIV`).  **Only ships through llvm-z80/llvm-z80**
until/unless the Z80 target lands upstream.

**Why this tier blocks Tier III submission:** a reviewer who sees
"here's a 27% AES tstate improvement" alongside "here are 4 silent
miscompiles in the same backend" will reject the series.  Tier II
must be at zero before the matching Tier III codegen wins can be
presented.

| # | Title | State | Note |
|---|---|---|---|
| **#125** | Z80LateOptimization crashes at -O0 on multi-CALL +static-stack +shadow-regs | Active | Crash, not miscompile.  Repro exists. |
| **#136** | Pre-existing O1 miscompile on test_90/91 edge_* (38 failures) | Active | Known noise in test-runner; **must be root-caused before claiming O1 quality is regression-free**. |
| **#150** | i16 EQ/NE HighByteZero path breaks cpnos pio-irq polypascal-test | Active | Direct sub_lo extraction.  Confirmed miscompile. |
| **#159** | Silent miscompile: rj_sb_inv chained u8 rotates use uninit register | Active | Real miscompile; repro in `tasks/compiler-comparison-corpus/`. |
| **#169** | Z80NarrowIV + LSR interaction miscompiles narrowed-then-rewidened IV loops | Active (worked around) | Workaround: place NarrowIV after LSR.  **Underlying bug still needs fixing — would block enabling NarrowIV earlier.** |
| **#170** | Z80NarrowIV miscompiles loop with parallel i8 + i16 IVs (test_94) | Active (worked around) | Workaround: single-phi-header guard.  **Same — underlying bug must be fixed before guard can be lifted.** |
| **#171** | Z80NarrowIV times out test_96 IY-spill | Active (worked around) | Same guard; underlying timeout still unexplained. |
| **#184** | getArithmeticInstrCost i16=2 miscompiles AES (infinite loop) | RESOLVED as won't-ship (session 73s confirm) | Correctness root-caused + fixed (#148 peephole fall-through + #185 DJNZ B-clobber); AES 13/13 PASS with i16=2 applied.  Held back purely on production-size tradeoff (cpnos +9 B, autoload +16 B, AES09 +44 B, BIOS -6 B) — net regression.  Decision + numbers documented in Z80TargetTransformInfo.h header.  No further code action; GitHub issue may be closed as won't-fix-tuning. |
| **#185** | i16=2 AES halts after ~28 tstates (independent of #184 r/c 1) | CLOSED via DJNZ B-clobber safety check | Was a peephole miscompile, not a cost-model bug — surface estimate "regalloc-level multi-week" collapsed to a 5-line safety check. |
| **#150** | (above) | Active | — |
| **#2**  | IRTranslator crash: inline asm with "hl" register constraint | Active | Crash.  Workaround: avoid `"hl"` in inline asm (project source already does). |
| **#12** | hasFP=false correct but larger; runtime bug present | Parked | Runtime bug (PROM hangs after banner) still unexplained — interacts with ISR/timing.  Issue says "low priority given minimal savings." |

## Tier III — Z80 backend completion (U-Z80)

The Z80 backend is preliminary by design (`project_z80_backend_unfinished`).
These items are "the Z80 backend exists and is good enough to upstream"
patches.  They are not bugs in clang — they are missing functionality.

### III.a — Target-info infrastructure

| # | Title | State | Note |
|---|---|---|---|
| **#177** | No Z80-specific TTI (cross-listed with Tier I) | PARTIAL | Phase 2 landed: Mul=Expensive, getCastInstrCost (trunc/zext free, sext=2), prefersVectorizedAddressing=false.  Open: i16=2 (#184), per-callsite refinement. |
| **#178** | Pseudos with implicit physreg outputs break rematerialization | Active (blocker isolated, session 73s) | Scope drill confirmed: SSA template `ADD16_tied` exists (Path A viable) but is NOT emitted — gated on a tied-operand two-address regalloc miscompile that corrupts UNRELATED values (attempt 2 on uncommitted `session-73p-issue166-add16-tied`, root cause unisolated). #178 + #166 both gate on isolating that one bug.  Next-session plan in `session73s-issue178-remat-drill.md`. |
| **#172** | 8-bit ALU accumulator should live in A | Active (blocker — ISel pattern) | Default-off pass shipped; true fix is ISel-level snapshot-rotate XOR chain pattern. |

### III.b — Missing GISel patterns / instruction-selection coverage

| # | Title | State | Note |
|---|---|---|---|
| **#7** | Implement Z80 instruction-driven codegen: DJNZ, LDIR, CPIR, CP (HL) | Active (umbrella) | Long-running umbrella; most concrete pieces already shipped (DJNZ, LDIR, CP (HL)). |
| **#175** | Missing 8-bit ALU with memory operand (XOR/AND/OR/ADD/SUB/ADC/SBC (HL)/(IX+d)/(IY+d)) | Active (ZeroYield until #40 flips) | HL-indirect ALU ops ARE defined and used; IX/IY-indexed not (production uses `+static-stack`, no IX-frame).  Verified 0 production bytes until #40 flips. |
| **#166** | ADD_HL_rr / LD_HL_a16 remat | Blocked on #178 | — |
| **#149** | i16 != -1 lowers via 8-byte cpl test, 'inc de + or' gives 5 B | Active | Single-pattern peephole/ISel candidate. |
| **#141** | i16 comparison against 0x0100 should fold to high-byte test | Active | Pattern fold. |
| **#117** | i16 EQ/NE peephole: handle 'neither operand in HL' case | Active | ~1 B per fire. |
| **#122** | i16 ULT/UGE with small-const RHS + high-zero variable: missing 8-bit CP | Active | Pattern fold. |
| **#146** | Callee-cleanup epilog 'pop bc; inc sp; inc sp; push bc; ret' could be 'pop hl; ex (sp),hl; ret' | Active | −2 B per epilog. |
| **#152** | SET/RES on memory through intervening LD A-readers | Active | #147 follow-up. |
| **#151** | Redundant 'and 1; rrca; sbc a, a' after icmp setting A=0xFF/0 | Active | #144 follow-up. |
| **#109** | ADD HL,rr commutativity: docs say check BC not read, code does not | Active | Minor logic bug in commutativity. |

### III.c — Memcpy / memmove / memset lowering

| # | Title | State | Note |
|---|---|---|---|
| **#50** | Unroll memcpy/memmove into LDI chains for speed-critical paths | Active | LDIR is current; LDI chain wins on small N for speed. |
| **#126** | __builtin_memmove emits much larger code than inline LDDR/LDIR | Active | Breaks budget-constrained PROMs. |
| **#127** | Peephole / GISel pattern for downward memmove → LDDR | Active | Pair with #126. |
| **#130** | Recognize memset_pattern for arbitrary fill widths via LDIR-overlap | Active | New lowering pattern. |

## Tier IV — Regalloc cost model (U-Z80, Cluster A residual)

The dominant remaining BIOS gap.  Closing one of these would likely
close several since they share a root cause (greedy cost model
ignoring per-class prefix overhead and short-lived-value spill cost).

| # | Title | State | Note |
|---|---|---|---|
| **#27** | Per-pair 16-bit register copy cost | Active (RECLASSIFIED, session 73s) | Drill found the per-pair *copy-cost* lever exhausted: `copyPhysReg` already uses `EX DE,HL` optimally; IX/IY copies don't occur (reserved); dominant BC/DE<->HL traffic is *necessary* base re-materialization (loading base once + 2 B copy-to-HL beats reloading). Prototype reload-retarget peephole fired 0× on AES (all sites have the pair live). Reclassified to "reduce base re-materialization under 3-pair pressure" (regalloc-level, pairs with #110/#115). CopyCost knob offers no win while IX/IY reserved. See `session73s-issue27-percopy-cost-drill.md`. |
| **#74** | Regalloc spills default to BSS; should use push/pop for short-lived 16-bit | Active | Companion to #96. |
| **#96** | Investigation: regalloc-level PUSH/POP for short-lived 16-bit values (layer 3) | Active | "Layer 3" of the spill cluster.  Drill: instrument LiveInterval lengths at spill sites. |
| **#16** | PUSH/POP instead of IX-indexed spills across CALLs | Active | Has a `BSS spill→PUSH/POP` peephole landed already; #16 is the regalloc-native version. |
| **#20** | BSS spill across CALL: multi-value pattern not handled | Active | Same family as #16. |
| **#100** | Loop rotation forces BSS-spill of loop carrier across CALL | Active | **Gates #77a default-on.** Cluster-A-adjacent. |
| **#110** | Greedy regalloc copy-elimination heuristic overrides target hints | Active | Workaround: single-register classes.  **Likely the same root cause as #27.** |
| **#111** | HLReg single-register class for pointer-arg in i16 self-loop | Active | #99 follow-up; another single-reg-class workaround. |
| **#115** | Greedy picks IY for LDIR/LDDR/HL-tied operands when un-reserved | Active | Same family — cost model doesn't know IY prefix overhead. |
| **#114** | Shadow-bank EXX-bracket pass for hot no-CALL inner loops | Active | New regalloc surface; uses shadow registers as extra pairs in limited contexts. |
| **#18** | Known-value register copy optimization | Active (enhancement) | Coalescer surface. |

## Tier V — Z80-specific codegen peepholes (U-Z80)

Mostly individual small wins.  Most need root-cause auditing (per
`peephole-vs-root-cause.md`) before upstreaming.

| # | Title | State | Note |
|---|---|---|---|
| **#108** | Peephole audit: skipped FLAGS-after-branch checks (DJNZ rewrites, others) | Active | Audit task. |
| **#117/#122/#141/#146/#149/#151/#152** | (in Tier III.b) | — | — |
| **#155** | #132 cross-MBB BSS peephole's UsedElsewhere gate over-conservative | Active | Follow-up. |
| **#138/#139/#140/#143** | #132 follow-ups (lit coverage, liveness, second candidate, layout-predecessor) | Active (small) | All #132 derivatives. |
| **#153** | Stray unconditional errs() print in #132 peephole | Active (trivial) | Polish before upstream. |
| **#154** | Reg-to-reg copy opcodes carry spurious mayLoad=1 / mayStore=1 | Active | TableGen polish; affects pass authors using `MI.mayLoad()`. |
| **#173** | 8-bit BSS spill via A is 6 B per cycle; PUSH/POP rr would be 2 B | ZeroYield (closed by survey 73p Phase 3) | Re-survey trigger: any new cross-MBB candidate. |

## Tier VI — Upstream-submission gating (U-Z80 cleanup)

Without these, the Z80 backend cannot be packaged as a coherent
patch series for llvm/llvm-project.

| # | Title | State | Note |
|---|---|---|---|
| **#180** | Peephole audit — 16/38 stand-ins for missing upstream infra (~2300 LOC) | Active | **Top-priority upstream gate per unpark doc.** |
| **#181** | DAGISel vs GISel coexistence audit — Z80ISelLowering.cpp possibly dead code | RESOLVED (session 73s) | No DAGISel path exists; `Z80ISelLowering` is the live shared GISel `TargetLowering` (keep). Dead `-gen-dag-isel` tablegen line removed. See `session73s-issue181-dagisel-gisel-audit.md`. |
| **#119** | Delete disabled EXX shadow-register conversion block (~150 LOC under #if 0) | CLOSED | Already cleaned up. |
| **#121** | Dead code: in-pseudo IR16 PUSH/POP fallback in XOR_CMP_*16 expansion | CLOSED | Already cleaned up. |

## Tier VII — Attribute / convention / intrinsic surface (U-Z80, new C)

These extend the C-language surface for Z80 (new attributes, new
intrinsics).  Higher review bar upstream because they touch the
language frontend.

| # | Title | State | Note |
|---|---|---|---|
| **#4**   | __critical equivalent (DI/EI function wrapper) | Active | Attribute surface. |
| **#42**  | Built-in intrinsics for DI, EI, HALT, IM 2, LD I,A | Active | Intrinsic surface. |
| **#43**  | Custom calling convention for CP/M BIOS entry points (BC/DE/HL params, C return) | Active | New CC. |
| **#131** | Register-preserving CC attribute (clang analog of SDCC __preserves_regs) | Active | Caller-side support landed; callee-side is #133. |
| **#133** | #131 follow-up: honor z80_preserves_regs on function definitions | Active | Callee-side completion of #131. |
| **#176** | Auto-infer +static-stack safety per-function | Active | "~30% of AES bin bloat vs SDCC" driver.  High-yield but big design surface. |

## Tier VIII — Tooling / infra (mixed targets)

| # | Title | State | Note |
|---|---|---|---|
| **#70**  | -fverbose-asm does not annotate assembly with source comments | Active | Quality-of-life; likely U-Z80 backend output formatter. |
| **#108** | (in Tier V) | — | — |
| **#118** | Audit Z80InstructionSelector::emitFusedCompareAndBranch for constant-RHS fold | CLOSED | — |
| **#124** | Workspace: cmake 4.2 + macOS treats benchmark HAVE_PTHREAD_AFFINITY failure as fatal | Active | Build workspace issue; not a compiler bug. |
| **#137** | test-runner: capture port-1 stdout text alongside DE register | Active | z80-utils improvement; orthogonal to compiler. |
| **#183** | Compiler-comparison-corpus: enable libc-dependent benchmarks | Blocked on #35 | — |
| **#35**  | No standard C library (libc) — only compiler-rt builtins | Active (umbrella) | Project-level decision; not a clang bug per se but limits corpus. |
| **#158** | K&R-style param int-promotion disables u8 rotate-pattern recognition | Active | Source-style limitation; partial U-Z80 lowering fix is possible. |

## Tier IX — Source / ABI / project (Never)

These are documented in `backend-research-not-filed.md` Section A.
Listed here so the upstream picture is complete (i.e. reviewers can
see what is explicitly NOT compiler-attributable).

- **A1** — recv_byte_t wide-return-with-sentinel ABI: source-level rewrite
- **A2** — Missing z80_preserves_regs on xport_recv_byte: source-level annotation
- **A3** — RST-instruction placement for hot call targets: linker/source-level
- **A4** — pop h; jmp common_exit inter-procedural unwind trick: no portable C semantic

## Tier X — Verified zero-yield (re-survey triggers)

From `backend-research-not-filed.md` Section B and 73p Phase 3 closeout.
Closed by survey, not by failed implementation.  Re-open if witness
surfaces in a new corpus addition.

- **B1** — Memory-operand arithmetic on (HL)/(IX+d) — re-survey on rcbios/byte-twiddling code
- **B2** — Small-N `inc hl × N` vs `ld de, N; add hl, de`
- **B3** — Direct addressing for byte loads (already working per #45)
- **B4** — `inc (HL)` / `dec (HL)` in-memory inc/dec
- **B5** — Dead-load detection
- **B6** — Switch / jump-table lowering bloat — re-survey when rcbios command-byte dispatch lands
- **B7** — `dec a` for A==1 (folded into #148 generalisation)
- **#173** — 8-bit BSS spill via A — same status

## Tier XI — Strategic decisions (not bug fixes)

These are design choices, not bugs.  They shape what other items can
be worked on.

- **#40** — IX frame pointer vs static-stack BSS per-function: gates #175
- **#35** — No standard C libc: gates #183 (corpus expansion)
- **EXX shadow-register spill conversion** (CLAUDE.md known-deferred):
  not separately filed; documented limitation.  #114 is the closest
  filed surface.
- **BSS overlay / call-graph BSS sharing** (CLAUDE.md known-deferred):
  not separately filed; parked alongside #12.
- **Mixed-mode BSS** (direct for locals + IX-indexed for stack args):
  not separately filed; "will matter when source switches back to
  register parameters."

## Coherence summary for upstream

If we had to put the Z80-backend story to llvm/llvm-project reviewers
today, the picture is:

**A — Generic LLVM improvements we already have evidence for:**
Tier I = 9 items.  Of these, **#128 underlying pessimization**,
**#164 trunc/zext cost model**, **#168 SimplifyCFG cost gate**, and
**#182 ScalarEvolution crash** are the most submission-shaped.
**These do not require Z80 target acceptance.**

**B — Z80 codegen wins that demonstrate the backend works:**
The landed-in-73p set (`#179 P1`, `#179 P2`, `#128 workaround`,
`#148 safety`, `#185 DJNZ safety`, `#177 partial TTI`, `#173
peephole`).  These are presentable as patch series **once #180 +
#181 cleanup is done**.

**C — Z80 backend completeness work still needed:**
Tier III + IV + VI = the gating list.  Most concrete single-item
gate is **#180** (peephole audit) since reviewers will reject
2300 LOC of stand-ins.

**D — Z80 correctness bugs that must close before B is presented:**
Tier II = 9 items.  Of these, **#159, #169-171, #136, #150, #182, #2,
#125** are real miscompiles or crashes.  Each needs an XFAIL lit
test today (per `feedback_compiler_bug_test`) and a fix before
ship.

**E — Items not for upstream:**
Tiers IX, X, XI plus the not-filed Section A — listed so the
inventory is complete.

## What to do with this map

- **Add to it** when a new issue is filed; do not let the map drift
  more than one session behind the issue tracker.
- **Promote from Tier X to a filed issue** if a new corpus witness
  surfaces; cross-link the survey-trigger note.
- **Demote a Tier II item to CLOSED** as fixes land; track the
  miscompile-fixes-shipped count as a coherence metric.
- **The map is index, not analysis.**  Detail belongs in the
  per-issue investigation docs (`tasks/issue<n>-*.md`).
