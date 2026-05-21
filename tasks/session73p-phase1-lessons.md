# Session 73p Phase 1 — lessons learned

Date: 2026-05-21.  Companion to `session73p-phase1-summary.md`.
Captures what worked, what didn't, and what to do differently next
session.

## What worked

### 1. Decision A–I framework

The 9 pre-confirmed decisions (fix-layer choice, branch strategy,
validation cadence, failure handling, test-case philosophy, etc.)
were referenced repeatedly during the session.  Two payoffs:

- **No mid-session direction churn.**  When I hit a question
  ("should I commit this?"), the framework already had the answer
  (Decision E: full oracle gating; if green, commit).
- **Decision H (stop-and-ask criteria) triggered cleanly.**  When
  #173 turned out smaller than estimated, the rule "scope expansion
  > 2× estimate" surfaced honestly — I asked the user instead of
  silently grinding.

### 2. TDD discipline (Decision I)

Writing the failing lit test BEFORE the fix caught a real near-miss:
my initial Z80ReorderTestDec safety gate (must verify I2's
destination vreg differs from I0's source) wasn't in v1 of the
patch.  The lit test alone would have passed.  But running the full
Z80 lit suite caught `issue-132-bss-spill-cross-mbb.ll` regressing
because of the missing gate.  Without TDD + Decision E full-oracle
gating, that bug would have shipped.

### 3. Wider-oracle paid for itself immediately

Time invested in `compiler-comparison-corpus`: ~3-4 hours.
Result: surfaced **#182 (LLVM ScalarEvolution capacity overflow)
within minutes of compiling the first benchmark**.  That's a real
upstream-LLVM bug we'd have shipped #179 without noticing.

### 4. Pre-RA MIR pass was the right layer

The temptation with #179 was to add a post-RA peephole in
`Z80LateOptimization.cpp` — small, focused, easy to reason about.
But per `feedback_root_cause_over_peephole` and
`project_z80_backend_unfinished`, that's the wrong layer when the
underlying issue is missing-regalloc/scheduler work.  Instead I
mirrored `Z80SplitDjnzCounters`'s precedent (a documented pre-RA
MIR pass for issue families #94/#98/#99) and the result is
substantively better:

- The pass handles 2 patterns (P1 + P2) with shared safety logic.
- Adding a P3, P4 (e.g., INC_A, SHL chains) is a small extension
  per pattern, no scaling penalty.
- Upstream-PR story is cleaner: "this is a pre-RA MIR pass for
  a Z80-specific regalloc gap" reads better than "this is one
  more peephole in our 6234-line post-RA file."

### 5. Honest estimate tracking

Every fix's actual yield got compared to the planning estimate
in the commit message.  This builds calibration over time.  In
this session:

| Fix | Estimate | Actual | Direction |
|---|---|---|---|
| #179 P1 | 1.5 M ts | 0.76 M ts | LOW |
| #179 P2 | (bundled in P1) | 3.4 M ts (solo) | HIGH |
| #128 | −320 B | −281 B | close |

Net #179: ~3× HIGH (P2 fires far more broadly than the gf_alog
analysis credited).  Net #128: 12 % LOW.  Recalibrate down on the
next plan's estimates.

## What didn't work as well

### 1. The speed-gap analysis (`aes-speed-gap-analysis.md`) underestimated P2

The session 73o analysis estimated gf_log + gf_alog's two patterns
(P1 counter test + P2 bit-7 test) as ~70 % of the AES speed gap,
with P1 being the bigger one.  Reality: P2 alone closed ~80 % of
the gap.  The bit-7-test idiom (ADD_A_A + RLCA + JR_C) appears
throughout AES sboxes, GF multiplication, and other bit-banging
code — not just in gf_log/gf_alog.

**Lesson:** when analyzing patterns at the per-function or per-IR-shape
level, also count site occurrences in the rest of the workload.  A
pattern that's "10 % per function" but appears in 50 functions is
bigger than a "70 % per function" pattern in 1 function.

### 2. The (HL) ALU forms were already in TableGen

#175's initial analysis claimed clang emitted 0 fused 8-bit
ALU-with-memory-operand instructions.  Reality: clang emits 35
`xor (hl)` etc. in AES; what's missing is only the `(IX+d)` /
`(IY+d)` forms.

**Lesson:** before filing "missing feature" issues, grep the
`Z80InstrCommon.td` AND `Z80InstrInfo.td` (both files — they're
split) for the asm mnemonic.  Don't grep just one.

### 3. Estimates over multiple fixes compound

I had planned #173 / #175 / #128 to collectively close ~10-15 %
more of the AES gap on top of #179.  Reality after #179 P2: the
gap is already closed (and reversed); #173/#175 give incremental
wins of ~1 % each that aren't worth the implementation time at
this point.

**Lesson:** re-evaluate the plan after each landed fix.  Don't
auto-pilot through a multi-issue plan when the landscape has
shifted.

### 4. Multiple "go" commands risked auto-pilot

The user gave me "go" several times.  Each one I interpreted as
"continue with the next plan item".  But after #128 landed, the
production target was already won; #173 would have been a
~3-4 hour commit for marginal gain.  Decision H's "stop and ask"
rule fired correctly at that point, and the user agreed.

**Lesson:** "go" doesn't mean "do everything on the list."  It
means "make the next-best decision."  When the next-best decision
has poor ROI, surface and ask.

## Specific surprises (recorded for memory)

### The I2-destination-vreg safety gate

In the post-MachineScheduler MIR for `issue-132-bss-spill-cross-mbb.ll`,
the regalloc reused the SAME vreg `%21` for both the PRE-DEC value
(loaded into A) and the POST-DEC value (stored back to %21).  My
naive matcher saw:

```
$a = COPY %21       ; pre-DEC value into A
DEC_A
%21 = COPY $a       ; post-DEC value back to %21 (vreg reuse!)
$a = COPY %21       ; loads POST-DEC value, not PRE-DEC
OR_r $a             ; tests POST-DEC == 0
JR_Z exit
```

This is a POST-test, not a PRE-test.  My P1 rewrite assumed PRE-test
semantics — would have miscompiled.  The fix: require I2's
destination vreg ≠ I0's source vreg.

**Lesson:** when matching multi-instruction shapes that depend on
vreg identity across the chain, EXPLICITLY check that intermediate
vregs don't alias the source.  Don't assume regalloc preserves
source-destination distinction.

### The compiler-comparison-corpus SDCC tstate measurement is broken

zsdcc's `-create-app` includes z88dk's CRT startup with a halt
mechanism that the AES corpus's `ticks -end <done_addr>` flow
doesn't catch.  zsdcc runs to the 200 M ticks counter limit
even on benchmarks that complete the work in <2 M ts.

The PASS verifier (memory sentinel at 0xC000) still works — we
know correctness — but the tstate column is meaningless for zsdcc.

**Workaround for now:** use the AES corpus (which has the proper
flow) as the primary tstate oracle.  compiler-comparison-corpus is
PASS/FAIL + size only for zsdcc.

**Future fix:** intrinsic_label markers in C source + ticks
`-start LABEL -end LABEL` flow.  Tracked informally; not yet a
GitHub issue.

### The XX-style asm patterns aren't always Z80-specific

When I wrote the speed-gap analysis, I attributed the gf_log/gf_alog
patterns to "Z80 regalloc gaps."  Reality: GCC and other compilers
have similar inefficiencies on other small register-pressured ISAs
(8051, AVR, MSP430).  The PATTERN is general; the SPECIFIC fix is
Z80-isa-aware (use SUB_n 1's borrow flag rather than DEC_A's lack
of carry).

**Implication:** the Z80ReorderTestDec pass's CONCEPT is upstream-
LLVM-meaningful even if the SPECIFIC implementation is target-
gated.  Could file the concept as an MIR-pass pattern that other
small-register targets could implement.

## Process improvements for Phase 2

1. **Re-evaluate after each landed fix.**  Don't just go to the
   next planned item — check whether the planned item's estimated
   yield still holds given the new state.

2. **Pre-emptively check both .td files** (`Z80InstrCommon.td` +
   `Z80InstrInfo.td`) for existing instructions before filing
   "missing primitive" issues.

3. **Count pattern site occurrences across the workload,** not just
   per-function frequency.  P2's underestimation was a counting
   miss.

4. **Surface ROI shifts to the user.**  When a plan item's
   estimated yield drops sharply due to other fixes landing,
   re-rank against alternatives instead of executing reflexively.

5. **The wider-oracle investment is worth it.**  Both the
   compiler-comparison-corpus's new-bug-discovery (#182) and the
   TDD discipline's catch (issue-132 safety gate) prove it.
   Future structural fixes should always have a wider-oracle
   regression check before commit, not just AES.

## Phase 2 readiness

Open issues by yield (revised post-Phase-1):

| Issue | Yield (revised) | Effort | Notes |
|---|---|---|---|
| #173 | 50-100 B AES, 5-10 B cpnos | 3-4 h | Smaller than originally estimated; useful for cpnos budget |
| #175 (IX+d) | ~50 B IX-frame configs | 1 day | Mechanical .td additions |
| #176 auto-static-stack | -30 % default-Oz size | 1-3 wk | Closes the remaining all-modes gap |
| #177 Z80 TTI | unlocks many smaller | 1-2 wk | Meta-fix; subsumes #128's permanent solution |
| #172 A-pin liveness | smaller now | 1-2 wk | Diminishing returns since #179 closed gf_log/gf_alog |

**Recommended next session:** start with #173 (smallest concrete
win, helps cpnos PROM1 budget which has only 18 B free) or
go directly to #177 (TTI) if time permits the larger investment.

## Closing thought

Phase 1's headline — "clang beats SDCC on AES production target" —
is durable.  The codegen is in the repo, the lit tests guard
against regression, the AES corpus's verifier guards against silent
miscompiles.  Future sessions inherit this as the new baseline.

The structural insight that produced #179 — "GISel ISel + scheduler
emit IR-source-order MIR that doesn't recover under multi-PHI
register pressure" — is a real, named, narrowly-scoped backend
infrastructure gap that I now have a working solution-shape for.
Phase 2's first fix could extend the same pass to a third pattern
(e.g., SHL/SAR chains) for marginal additional yield.
