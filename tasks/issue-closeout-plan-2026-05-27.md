# Issue closeout plan — maximize fixes-closed (2026-05-27)

Goal: close as many open issues **with fixes** as possible, by grouping work that
shares context so one focused session retires several.  61 open issues in
ravn/llvm-z80 at time of writing.

## Method (applies to every cluster)

1. **Verify-first, per issue (~30 min drill):** before assuming an issue is open,
   reproduce it on HEAD.  Several may already be fixed or implemented (like #130,
   closed this session as implemented) — see Cluster 0.  Per
   `feedback_dig_deeper_before_parking` + `feedback_baseline_before_implementing`.
2. **The safety net is now strong:** `test-runner -diff-opt` + `-native-oracle`
   are at a green baseline (0 DIFFOPT/0 NATIVE, default + static-stack) and lit is
   121+5.  This makes the fragile peephole/spill class (which produced #136/#202/
   #204) **safe to batch-fix**: gate every change on byte-identical production +
   green oracles + lit.  This is the single biggest reason a closeout push is now
   feasible.
3. **Each codegen fix ships with a lit test** (and a runtime test where the bug is
   runtime-only).  Value oracle before commit.

Effort labels below are **estimates from the issue text**, not verified, unless
marked (verified).  Order is by yield-per-effort and risk.

---

## Cluster 0 — VERIFY-AND-CLOSE (likely already fixed/implemented) — do FIRST, ~½ day

Cheapest closures: confirm the fix exists, add a regression test if missing, close.

- **#16** "PUSH/POP instead of IX-indexed spills across CALLs" — the BSS-spill->
  PUSH/POP peephole family (#129/#132) now does exactly this.  Verify on the #16
  repro (`fdc_*`); likely closeable as implemented (residual multi-value = #20).
- **#74** "spills should use push/pop for short-lived 16-bit" — same mechanism as
  #16; verify + close or point at #96 (the regalloc-native version) for the deep part.
- **#159** "silent rotate miscompile" — my chained-rotate repro is oracle-clean this
  session; run the exact original rj_sb_inv source under `-native-oracle`; close if clean.
- **#131** "z80_preserves_regs caller-side" — caller-side landed (per #133 background);
  confirm what remains here vs #133 (callee-side); close #131 if caller-side is its scope.
- **#123** "-g influences optimizer" / **#187** "peephole removal side-channel" —
  investigation issues; resolve with a measurement comment-and-close if no action remains.

Expected: **3–5 closed** for the price of reproduction.

## Cluster 1 — SMALL PEEPHOLE WINS — highest fix-count-per-effort, ~1–2 sessions

Self-contained post-RA / ISel peepholes in `Z80LateOptimization.cpp` /
`Z80InstructionSelector.cpp`, each with a clear repro and a few-LOC fix; all share
the lit + differential-oracle test loop.

- **#18** ✅ CLOSED 2026-05-27 (main `bfce0fff25db`).  New per-MBB peephole:
  `LD r,n` -> `LD r,A` when A holds the constant.  Oracles clean, cpnos −2 B,
  polypascal PASS.  Lit `issue-18-ld-r-a-known-const.ll`.
- **#151** ✅ CLOSED 2026-05-27 (verify-only — already fixed; clean
  `sub 1; sbc a,a` lowering, no `and 1; rrca` residual).
- **#152** ✅ CLOSED 2026-05-27 (verify-only — already implemented + lit-tested;
  SET/RES through intervening A-readers via the `LD A,(HL)` form).
- **#146** ⚠ RECLASSIFIED (not a small win).  The `ex (sp),hl` rewrite clobbers
  HL, which is LIVE at the epilog of value-returning functions (that's why
  codegen uses `pop bc` there).  Safe only for void/non-HL-return + one stack
  word + HL-dead — needs a liveness guard, 1 B, and **zero production impact**
  (`+static-stack` has no stack-arg functions).  Drill comment on the issue.
- **#117** i16 EQ/NE peephole: handle "neither operand in HL" (~1 B/fire).  LIVE
  but marginal; risky `emitFusedCompareAndBranch` lowering area.
- **#122** i16 ULT/UGE small-const RHS -> 8-bit `CP n` fast path.  LIVE but
  marginal: the 8-bit LOAD already happens; only the 16-bit subtract tail is
  suboptimal.  (Ruled out a suspected `and 254` miscompile — it's value-preserving
  demanded-bits folding.)  Same risky compare-lowering area.
- **#173** 8-bit BSS spill via A — bigger win (AES driver), MEDIUM not small.
  NOTE: the obvious repro (`u8 t=x+1; use(); return t+s`) did NOT produce the
  `push af; ld a,r; ld (nn),a; pop af` shape — it produced the already-optimal
  `push af; call; ...; pop af`.  Needs fresh investigation to reproduce the
  actual pattern before fixing.

Cluster-1 result (2026-05-27): **3 closed** (#18 fixed + #151/#152 verify-close).
Remainder (#117, #122, #146, #173) are marginal/narrow/medium — not clean small
wins.  #173 is the only one with real (AES) value; promote it to a focused drill.

## Cluster 2 — #132 SPILL-FAMILY CLOSEOUT (#188) — SUBSTANTIALLY DONE 2026-05-27

- **#155, #143, #140**: ✅ already CLOSED (found via verify-first).
- **#139**: ✅ CLOSED (verify) — stale cpnos-rom diagnostic, source removed.
- **#203**: PARTIAL (3 behavior-preserving steps landed: predicates `f3282df`,
  UsedElsewhere `f009ab4`, SP-write `4bdeea1`; −80 net lines, oracles 0/0, cpnos
  byte-identical).  **Remaining: forward-scan orphan/stack-depth restructure** —
  the one delicate piece, interleaved with per-peephole load-collection; deferred
  to a focused session (gate on byte-identical + oracles + lit).
- **#20**: deferred (multi-value design extension).
- **#188** (meta): drift surface eliminated; stays open until #203 fully closes.

Result: **4 closed this session** (#139 + the 3 already-closed); #203 advanced.

## Cluster 3 — VERIFIER / CORRECTNESS CLOSEOUT (#197) — ~1 session

Clearing these flips the `-verify` test-runner flag to a blocking CI lane (closes #197).
- **#200** ✅ CLOSED 2026-05-27.  Root cause: `eliminateFrameIndex` folds the
  `$offset` displacement into the resolved frame offset and `removeOperand`s it,
  leaving SPILL/RELOAD FI pseudos (declared with 3 ops) in a 2-op form that
  `-verify-machineinstrs` rejects after PEI.  Fix: in the small-offset survivor
  path, re-append the consumed `$offset` as a `0` placeholder (value already
  folded into operand 1; `expandPostRAPseudo` reads only operand 1) for all five
  `$offset`-bearing pseudos (SPILL/RELOAD_GR16, SPILL/RELOAD_GR8, SPILL_IMM8).
  Codegen-neutral: lit 124+5 (new `issue-200-...mir`), diff-oracles 0/0 (default
  + static-stack, Fail 0), cpnos PROM1 payload byte-identical (2028 B).
- **#194** undefined-$a stale liveins after late-opt — delicate (blanket recompute
  rejected +2 B; needs path-limited recompute).  Medium-hard.
- **#125** Z80LateOpt crash at -O0 +static-stack +shadow-regs — a crash; isolate +
  guard (may share root with the liveness class).
- **#190** IY-unreserve alloca FATAL — quick part: fix `test_48`'s missing `alloca.h`
  so it builds (test-setup); deeper FP-interaction part can defer.
Expected: **2–4 closed** + #197 unblocked.

## Cluster 4 — PRIVILEGED INTRINSICS + ATOMIC — ✅ DONE 2026-05-27 (3 closed)

- **#42** ✅ CLOSED (main `81b46fe`).  Compiler ships `<intrinsic.h>` +
  `__builtin_z80_di/ei/halt/nop/im2/set_i`; rcbios adopts it (same source clang+SDCC,
  no ifdef).  BIOS 5922 -> 5897 B, MAME boot OK.
- **#4** ✅ CLOSED (main `736f83f`).  `__attribute__((z80_critical))` -> DI/EI;
  rcbios `__critical` now real (was a no-op).  MAME boot OK.
- **#133** ✅ CLOSED (verify): callee-side `z80_preserves_regs` save/restore already
  implemented + tested.  Closed on substance; Part B (advisory warning) -> **#207**.
- Follow-ups filed: **#207** (Part B advisory warning), **#208** (gate im2/set_i off SM83).
- Outcome: **3 closed** (target was 2-3).  See `session73s-cluster4-intrinsics-2026-05-27.md`.

## Cluster 5 — MEMCPY / MEMMOVE / FILL — ~1 session, shared mechanism

- **#126** `__builtin_memmove` emits much larger code than inline LDDR/LDIR.
- **#127** memmove -> LDDR peephole/GISel (same mechanism as #126; do together).
- **#205** LDIR-fill non-UB representation (the #136 follow-up: memset for K=1 — note
  the -Oz regression caveat — / target intrinsic for K>1).
- **#50** unroll memcpy/memmove into LDI chains for speed (enhancement; optional).
Expected: **2–3 closed**.

## Cluster 6 — TOOLING / INFRA — ~½ day, low risk, momentum

- **#124** cmake 4.2 + benchmark HAVE_PTHREAD_AFFINITY fatal (workspace build fix).
- **#137** test-runner capture port-1 stdout (or first-failing-line mode) for fixture
  diagnosis (I have a working ad-hoc version this session).
- **#70** `-fverbose-asm` source annotations (AsmPrinter comment plumbing).
Expected: **2–3 closed**.

---

## Deferred — DEEP / BLOCKED (not part of the "many fixes" push)

- **Regalloc cost-model cluster** (deep, fresh focused sessions each): #12 (IX CSR /
  hasFP — needs FP-elimination), #27 (per-pair copy cost — TableGen limit), #40
  (per-function IX-vs-static-stack), #96 (regalloc-native PUSH/POP spill), #110
  (greedy copy-elim overrides hints), #111 (HLReg single-reg class — medium), #114
  (shadow-bank EXX), #115 (greedy picks IY), #172 (A-accumulator pinning), #178
  (implicit-physreg-output pseudos break remat -> blocks #166), #166, #176.
- **TTI / cost-model:** #164 (TruncInstCombine zext cost), #184/#185 (i16=2 AES
  miscompile — experimental, keep gated).
- **Big features:** #35 (libc — large), #183 (blocked on #35).
- **Meta/trackers** (close by completion of their children, not direct fixes): #7,
  #180, #186, #188, #195.
- **#158** (K&R int-promotion disables u8 rotate) — frontend/ABI; medium, standalone.
- **#43** (CP/M BIOS calling convention) — investigation + new CC; medium, standalone.
- **#150** (sub_lo extraction broke cpnos pio) — investigation; risky (precedent).

---

## Recommended execution order (by yield/risk)

1. **Cluster 0** (verify-and-close) — cheapest closures, sets an accurate board.
2. **Cluster 6** (tooling) + **Cluster 4** (intrinsics) — low-risk, self-contained,
   build momentum while the oracle-guarded codegen work is planned.
3. **Cluster 1** (small peepholes) — highest fix-count; oracle-guarded.
4. **Cluster 2** (#132 family) — coordinated, oracle-guarded.
5. **Cluster 3** (verifier) — unblocks #197 / the CI gate.
6. **Cluster 5** (memmove/fill).

Realistic target: **~15–20 issues closed with fixes** across clusters 0–5 over a
handful of focused sessions, leaving the deep regalloc/TTI/libc work as deliberate
separate efforts.  Every codegen change gated on lit + the green differential
oracles + byte-identical production.
