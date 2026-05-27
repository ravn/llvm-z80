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

- **#146** epilog `pop bc; inc sp; inc sp; push bc; ret` -> `pop hl; ex (sp),hl; ret`
  (-2 B/epilog).  Clear repro+fix in the issue. (verified the pattern exists in
  stack-arg functions this session.)
- **#18** known-value reg copy: `LD r,A` after `XOR A`/`OR A` instead of `LD r,#0`.
- **#117** i16 EQ/NE peephole: handle "neither operand in HL" (~1 B/fire).
- **#122** i16 ULT/UGE small-const RHS + provably-high-zero -> 8-bit `CP n` fast path
  (sibling of #117; same emitFusedCompareAndBranch area — do together).
- **#151** remove redundant `and 1; rrca; sbc a,a` after icmp already set A=0xFF/0.
- **#152** SET/RES on memory through intervening `LD A,(HL)` readers (#147 follow-up).
- **#173** 8-bit BSS spill via A (`push af; ld a,r; ld (nn),a; pop af` = 6 B) ->
  PUSH/POP rr (2 B) — bigger win (AES driver), medium not small, but same peephole area.

Group as: (a) compare-peepholes {#117, #122}, (b) A/flag-peepholes {#18, #151, #152},
(c) epilog {#146}, (d) spill {#173}.  Expected: **4–6 closed**.  Risk: low (each
guarded by oracles + lit; #150 is the cautionary precedent — a similar refinement
broke cpnos pio, so run the cpnos polypascal oracle too).

## Cluster 2 — #132 SPILL-FAMILY CLOSEOUT (#188) — ~½ day, coordinated

Already coordinated under #188.  The two drifted guards (loop-carried, address-taken)
are unified; remaining:
- **#203** unify the rest (orphan / UsedElsewhere / SP-write / stack-depth) into
  shared helpers.  Behavior-sensitive -> gate on byte-identical + oracles.
- **#155** UsedElsewhere over-conservative — **fold into #203** (same guard).
- **#143** multi-fire edge-split conflict (CFG reasoning, ~2 h).
- **#139** diagnostic loose end (~30 min investigate-and-close).
- **#140** add .mir lit coverage (mechanical).
- (#20 multi-value = defer; real design extension.)

Expected: **4 closed** (#203, #155, #139, #140; #143 if time).

## Cluster 3 — VERIFIER / CORRECTNESS CLOSEOUT (#197) — ~1 session

Clearing these flips the `-verify` test-runner flag to a blocking CI lane (closes #197).
- **#200** SPILL_GR16 2-vs-3 operand count (cosmetic, frame-lowering) — bounded.
- **#194** undefined-$a stale liveins after late-opt — delicate (blanket recompute
  rejected +2 B; needs path-limited recompute).  Medium-hard.
- **#125** Z80LateOpt crash at -O0 +static-stack +shadow-regs — a crash; isolate +
  guard (may share root with the liveness class).
- **#190** IY-unreserve alloca FATAL — quick part: fix `test_48`'s missing `alloca.h`
  so it builds (test-setup); deeper FP-interaction part can defer.
Expected: **2–4 closed** + #197 unblocked.

## Cluster 4 — PRIVILEGED INTRINSICS + ATOMIC — ~1 session, self-contained, low risk

No codegen-pipeline risk; pure feature addition.
- **#42** `__builtin_*` for DI, EI, HALT, IM 2, LD I,A.
- **#4** `__critical` DI/EI function wrapper (builds on #42).
- **#133** honor `z80_preserves_regs` on definitions (callee-side save/restore) — adjacent
  ABI/attribute work; group if doing attribute plumbing.
Expected: **2–3 closed**.

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
