# Plan: close ravn/llvm-z80 #250 (byte-array pointer strength reduction) — for a new session

**Author:** Copilot (AI), 2026-07-12, at @ravn's request ("investigate #250
thoroughly and prepare a plan for a new session").

**Scope note / model fit:** the *investigation* below is open-ended and was
done on Opus. The *execution* splits: **Phase 1a (coverage spike) is an
open-ended SCEV/LFTR debug of an unsolved unknown — keep it on Opus**; only once
it hits GO does the mechanical patch work (Phase 1b) become a Sonnet-friendly
targeted change against an established pass. Escalate to Opus for the
register-pressure model design (Phase 2). Do NOT treat Phase 1 as a bounded
"targeted patch" up front — its effort is unknown until the 1a spike resolves.

---

## 1. One-paragraph summary

#250 is the missing pointer-strength-reduction for scale-1 byte-array loops
(`for (i…) base[i]` re-derives `base+i` every iteration instead of walking a
running pointer). **The passes to fix it already exist and are wired, all
default-OFF**: `Z80LoopInstrFormPrep` (forms the pointer IV, IR-level,
post-LSR), `Z80PinLoopPointer` (pins it to HL via the `HLReg` class, MIR-level),
plus `Z80SinkColdLoopIV` (the separate #256/M3 scan-loop fix). Prior work (session
2026-07-09) proved they reach the optimal dcc form **in isolation** but
**net-regress the nested `sieve` kernel** because the rewrite is
register-pressure-blind and steals the enclosing scan loop's pairs. The blocker
to default-on is therefore a **profitability/pressure model**, not new
machinery. This session's job: (Phase 1a) spike the coverage unknown — root-cause
why prep doesn't fire on the *non-nested* candidates (tm/chkmem, ttt) and return a
GO/NO-GO; (Phase 1b, only on GO) ship those wins default-on without regressing
sieve/e; (Phase 2, harder) a real pressure model for the nested case.

---

## 2. Verified current state (checked this session — code + issues + writeup)

Files (all present, `llvm/lib/Target/Z80/`):
- `Z80LoopInstrFormPrep.{cpp,h}` — IR `FunctionPass`, flag
  `-z80-loop-instr-form-prep` (default OFF, `EnableZ80LoopInstrFormPrep`).
  Runs post-LSR in `Z80PassConfig::addIRPasses()`. Structure:
  - `collectAddrGroups` (cpp:130-176): finds scale-1 `i8` GEPs whose SCEV is an
    affine AddRec `{Start,+,Step}` with loop-invariant Start **and** Step. Keys
    groups on the SCEV `(Start,Step)` pair — **Start can be any loop-invariant
    value, NOT only a global** (so a *parameter* base like chkmem's `%p` is
    eligible in principle — verified by reading the code).
  - `registerPressureOK` (cpp:177-184): **coarse** — counts header PHIs, declines
    if `ExistingPHIs + NewGroups > Z80MaxLoopCarriedPtrs` (flag
    `-z80-loop-instr-form-prep-max-carried`, default **2**).
  - `tryEliminateOldIV` (cpp:210-320-ish): LFTR-style rewrite of the exit test to
    a pointer compare so the old integer IV dies (mandatory on Z80 — leaving it
    live keeps 3 live pairs and spills; documented as turning the #250 repro
    508→680 B).
  - `runOnFunctionImpl` (cpp:390): **innermost-only** (`if (L->isInnermost())`).
- `Z80PinLoopPointer.{cpp,h}` — MIR `MachineFunctionPass`, flag
  `-z80-pin-loop-pointer` (default OFF). `constrainRegClass(Ptr/Next, HLReg)` to
  keep the pointer in HL (sibling of the `BCReg` DJNZ-counter pin from #94/#99).
- `Z80SinkColdLoopIV.{cpp,h}` — IR pass, flag `-z80-sink-cold-loop-iv` (default
  OFF). The #256/M3 scan-loop fix; **clean −2.3% on sieve, ±0 elsewhere,
  outputs correct** (measured last session). Its own default-on gate is broader
  corpus + production-density validation.
- `HLReg` register class already in `Z80RegisterInfo.td` (`def HLReg :
  Z80Reg16Class<(add HL)>;`). Also the raw material #251 wants.
- Lit tests: `llvm/test/CodeGen/Z80/pointer-iv-strength-reduce.ll` (proposed in
  the issue body), `sink-cold-loop-iv.ll` (exists, red-green).

Prior-session verdict (`tasks/session-2026-07-09-sink-cold-loop-iv.md`,
per-region exec table, z88dk-ticks):

| region (sieve) | sink only | sink+prep+pin | Δ |
|---|--:|--:|--:|
| KILL loop | 2,699,820 | 1,649,890 | **−1,049,930** (the win) |
| SCAN loop | 1,064,830 | 2,375,390 | **+1,310,560** (the regression) |
| total T-states | 32.2M | 33.6M | **+1.4M WORSE** |

Root-caused: `prep` only touches the innermost KILL loop, but pinning HL there +
SCEV-expanded inits (`flags+k_start`, stride, end-ptr) + an IY spill cascade into
the enclosing SCAN loop, doubling its body (13→29 instr). Classic 3-pair
over-subscription: **the inner loop's pointer steals the outer loop's registers.**

## 3. New data this session (2026-07-12, verified)

Cycle-accurate z88dk-ticks, four dcc benchmarks, llvmz80 vs sdcc `-O2`:

| bench | llvmz80 | sdcc | note |
|---|--:|--:|---|
| sieve | 34.48M | 53.23M | llvmz80 **wins** (no fix needed for parity) |
| e | 41.83M | 59.55M | llvmz80 **wins** |
| ttt | 10.38M | 8.44M | llvmz80 **1.23× slower** — #250 global-base loop |
| tm | 272.2M | 164.6M | llvmz80 **1.65× slower** — #250 chkmem param-base loop |

(Posted as a comment on #250 and tracking issue #258.) So vs **sdcc**, #250 is
what costs us ttt and tm; sieve/e already win. (Note: the older
`plan-2026-07-09-beat-dcc-benchmarks.md` compares vs **dcc** where sieve/e also
lose — different, faster baseline. This plan targets the sdcc gap the user's
current benchmark run surfaced; dcc parity is the harder stretch goal.)

**"Just re-enable LSR now the backend is better?" — measured & rejected
(2026-07-12).** LSR is the stock LLVM pass (on at `-O2`/`-Oz`); it is NOT a
patched backend default. Only the **firmware** Makefiles disable it
(`autoload-in-c/Makefile:269`, `cpnos-in-c/Makefile:118` pass `-mllvm
-disable-lsr`); the dcc-benchmark zcc path (`build_zcc.sh`) never passes it, so
the ttt/tm/sieve/e numbers were already LSR-**on**. Toggling is per-invocation,
one flag, no rebuild. Re-measured on/off:

| bench (T-states) | LSR on | LSR off | winner |
|---|--:|--:|---|
| sieve | 34.48M | 27.55M | off −20% (nested loop) |
| e | 41.83M | 45.22M | on −8% |
| ttt | 10.38M | 11.97M | on −15% |
| tm | 272.2M | 317.1M | on −16% |

| firmware (autoload -Oz, 2048 B cap) | size |
|---|--:|
| LSR off (default) | 2035 B (13 B free) ✓ |
| LSR on | **link fails: exceeds 2048 B** ✗ |

Verdict: **do NOT flip the default.** LSR is a per-loop mixed bag (wins
e/ttt/tm, loses the nested sieve kernel — the source of the old "harmful"
verdict) AND blows the firmware hard size cap. But this is strong evidence FOR
#250: LSR-on proves ttt/tm can gain ~15% from better IV/pointer codegen (the
exact sdcc gap), so a *targeted* per-loop pointer-SR that fires on the non-nested
hot loops and declines on nested/size-critical code captures the win without the
global cost. LSR-on is a useful upper-bound oracle for Phase-1 target numbers.

**Opt-level / LSR nuance (verified, important):** the llvmz80 pipeline
**disables generic LSR** (`-mllvm -disable-lsr`; CLAUDE.md "LSR is Harmful").
Reproduced on the standalone `chk` (chkmem-shaped) function,
`--target=z80 +static-stack`:
- `-O2 -mllvm -disable-lsr` → **`add hl,de` recompute in the loop** (the #250 bug;
  matches the zcc `-O2` benchmark build).
- `-Oz` → already a clean running-pointer walk (`inc bc; dec de`).
- `-O2` (LSR left on) → yet another shape.

So #250 is specifically the "LSR-off pipeline lacks a *targeted*
non-counter-widening pointer strength reduction" gap — exactly what
`Z80LoopInstrFormPrep` is for.

**Blocker sharpened (verified this session):** under `-O2 -disable-lsr`,
turning on `-z80-loop-instr-form-prep -z80-pin-loop-pointer` **does NOT** fix the
standalone `chk` loop (still 1 `add hl,de`, no walk), and bumping
`-z80-loop-instr-form-prep-max-carried` 2→4 makes no difference. So for the
non-nested chkmem shape the blocker is **not** the pressure gate — `prep` is not
rewriting this shape at all under these flags. Why is unknown (GUESSED
candidates: `tryEliminateOldIV` bails on this exit-test shape and the pass then
declines; or the store's GEP SCEV isn't the expected affine AddRec once LSR is
off; or an early `runOnFunctionImpl` bail). **This is Phase-1 task #1.**

**New instances + first clean red/green (2026-07-12 sweep, verified).** Built a
repeatable detector (`tasks/tools/m5-loop-reload-scan.py`) — flags every in-loop
`add hl/ix/iy`, tags `GLOBAL-BASE(sym)` per-iteration base reloads — and ran it
over the whole compiler-comparison corpus + production firmware. Two findings
that change this plan's scope:

1. **fannkuch — a measured per-loop cost, but a NESTED (Phase-2) target, not
   Phase-1.** Its hot reversal-swap loop (`perm[i] <-> perm[k-i]`) reloads
   `_perm` every iteration. Red/green by SOURCE rewrite to two running pointers
   (`*lo`/`*hi`), all else identical, both PASS (0x10E4):

   | fannkuch (`-Oz -disable-lsr`, shipping config) | T-states | size |
   |---|--:|--:|
   | baseline (index `perm[i]`) | 36,203,397 | 584 B |
   | running pointers | 28,949,757 | 536 B |
   | delta | **−20.0%** | **−48 B** |

   (fannkuch's `-O1`/`-O2` cell is a separate verified backend miscompile → 0;
   `-Oz` is correct and is what the corpus measures.) IMPORTANT CAVEAT
   (verified 2026-07-12 from the asm loop-depth annotations): the `_perm` swap
   loop is at **Depth=3** (`BB0_14` inside `BB0_10` Depth=2 inside `BB0_4`
   Depth=1). The −20% was achieved by a *source-level* rewrite, which bypasses
   the register allocator's pressure constraints. An AUTOMATIC prep+pin on this
   loop faces the SAME (worse — 3-deep) register-pressure cascade that makes
   sieve net-negative. So fannkuch is a **Phase-2 opportunity marker** (proof
   the win is real, −20%), NOT a Phase-1 validation target. Do not expect the
   Phase-1 "decline nested loops" gate to capture it. This also means the only
   confirmed non-nested Phase-1 targets remain tm/chkmem and ttt — verify their
   loop depth is 1 before counting on them (chkmem's `for(i<c)` is a single
   loop; ttt's win-scan needs checking).

2. **`_perm`/`_count` are `int` arrays → scale-2 (word) indexing, NOT scale-1.**
   `collectAddrGroups` (`Z80LoopInstrFormPrep.cpp:130`) currently keys on
   scale-1 `i8` GEPs only, so a Phase-1 fix scoped to scale-1 **will not touch
   fannkuch.** SCOPE DECISION for Phase 1: either (a) explicitly declare scale-1
   only and leave fannkuch/word arrays to a follow-up, or (b) widen
   `collectAddrGroups` to any element size where the base reload dominates
   (`ld hl,GV; add hl,rr` is size-independent — the reload is the same 21 T
   whether the stride is `inc de` or `add hl,hl`). (b) captures the −20%
   fannkuch win; (a) is smaller/safer. Bring the choice to @ravn.

3. **Production is not entirely M5-free.** `autoload-in-c/rom.c` has genuine
   per-iteration `_fdc_result`/`_fdc_cmd` reloads — but both are cold,
   ≤7-iteration, call-bounded FDC loops, so perf impact is nil. This weakens
   open-question #2's "zero production impact" slightly (the kernels exist) but
   does not change the conclusion (they don't cost); the nesting/cold gate
   should decline them anyway (call-bounded, not worth the extra live pair).

## 4. The crux, stated plainly

Two independent obstacles, do not conflate:
1. **Coverage** — `prep` doesn't currently fire on the real non-nested benchmark
   loops (chkmem param-base under the production `-O2 -disable-lsr` flags), for a
   reason not yet root-caused. Fixing this is high-confidence value (tm is
   1.65× slower entirely because of this loop) and does **not** need a pressure
   model, because these loops are not nested inside another hot loop.
2. **Profitability under nesting** — where `prep` *does* fire (sieve KILL loop),
   it's net-negative because it's pressure-blind and the loop is nested. Fixing
   this needs a real model and is the multi-week part.

Phase 1 attacks (1); Phase 2 attacks (2). Ship Phase 1 alone if Phase 2 stalls.

---

## 5. Phased plan

### Phase 0 — baseline & harness (do first, ~½ day)

- Capture the **before** on the unmodified compiler (AGENTS.md "baseline before
  you change"): the 4-benchmark bytes+T-states table (script:
  `scratch/dcc-clang-bench/`, oracle: `ticks_cpm.py`), the full Z80 lit result
  (expect 183 PASS + 5 XFAIL per last session — reconfirm), and the production
  triplet sizes (rcbios/cpnos/autoload) for the byte-identical gate.
- Standalone repros already reduced this session: `chk`/`chkmem` (tm),
  `dcc/tests/ttt.c` win-scan loop (ttt), the issue's `kill()` (sieve). Add each
  as an `.ll` under `llvm/test/CodeGen/Z80/` as you pin behavior.

### Phase 1a — coverage spike — **DONE 2026-07-12, verdict GO**

> **RESOLVED.** Root cause found and verified — see
> `session-2026-07-12-issue250-phase1a-spike.md`. It is a clean, bounded,
> standard-LLVM fix, so the gate returns **GO**. Summary below; the guessed
> candidates in the original spike text (kept for the record) were all wrong.

**ROOT CAUSE (verified):** `runOnFunctionImpl` bails at its FIRST guard —
`if (!L->getLoopPreheader() || !L->getLoopLatch()) continue;`. A zero-trip
guard (`if (c==0) skip`) makes loop entry a *conditional* branch, so there is
**no dedicated preheader** and the pass declines before ever inspecting the
(perfectly matching `{%0,+,1}`) GEP. It only reproduces under `-disable-lsr`
because LSR is what pulls **LoopSimplify** (preheader insertion) into the
pre-codegen IR pipeline; drop LSR and guarded loops lose their preheader.
Proof: `opt -passes=loop-simplify` before prep → base reload eliminated,
running pointer walks in `bc`.

**FIX (Phase 1b):** make the pass require loop-simplified form the way LSR does
(`AU.addRequiredID(LoopSimplifyID)` + `INITIALIZE_PASS_DEPENDENCY`, and ensure
the codegen path has LoopSimplify ahead of it). Not a band-aid.

**CAVEAT that revises the beneficiary story (verified):** the real
`dcc/tests/tm.c` `chkmem` walks `*pc; pc++` in C — NOT `base[i]` — so it has NO
base reload (its waste is `val`-spill + a redundant counter; tm's runtime is
malloc-dominated anyway). tm is therefore NOT a base-reload beneficiary.
Genuine `base[i]` beneficiaries are `sieve` and index-shaped loops. Phase 1b
must re-confirm which real loops carry the `base[i]` shape before claiming
per-program wins.

<details><summary>Original spike plan (superseded — kept for record)</summary>

1. **Root-cause why `prep` doesn't rewrite `chk` under `-O2 -disable-lsr`.**
   Instrument with `-debug-only=z80-loop-instr-form-prep` (add `LLVM_DEBUG`
   traces to `collectAddrGroups`, `registerPressureOK`, `tryEliminateOldIV`,
   `runOnFunctionImpl` if not present). Determine which guard bails. Likely
   `tryEliminateOldIV` (the chkmem exit test is `i < c` with `c` a *parameter*
   bound, `i` the counter — should match, but the GEP index phase vs the
   counter phase may differ) OR the store GEP isn't recognized once LSR is off.
2. **Also confirm the load-bearing premises** while instrumented:
   (a) chkmem(tm) and ttt hot loops are actually Depth-1 (`-print-after` the loop
   info, or read the `Depth=N` asm annotations via the M5 scanner) — Phase 1's
   entire reach rests on these being non-nested; and (b) an automatic prep+pin on
   a *proven* non-nested loop does not itself regress (sanity that the nesting
   gate, not something else, is what protects sieve).

</details>

**GO/NO-GO GATE (end of Phase 1a — decide before writing any ship code):**
- **GO** if the coverage gap is a clean, bounded relaxation (e.g. widen the
  exit-test phase match in `tryEliminateOldIV`, or add the param-base start SCEV
  expansion) AND premises (2a) hold → proceed to Phase 1b.
- **NO-GO / re-scope** if the root cause is structural (SCEV non-affine without
  LSR, or the "non-nested" targets are actually nested) → Phase 1 does NOT ship
  this session; fold the finding into Phase 2 and report. Do not force a band-aid.

### Phase 1b — ship the non-nested wins default-on (CONDITIONAL on Phase 1a GO)

1. **Fix the coverage gap** minimally and cleanly (no band-aids), per the bounded
   shape Phase 1a identified. Add a `LLVM_DEBUG` worked example (chkmem concrete
   values) per the comment-style rules.
2. **Re-gate for nesting instead of raw PHI count.** Replace/augment
   `registerPressureOK` so it declines when the loop is **nested inside another
   loop that is not provably cold** (the sieve case), and allows non-nested /
   cold-enclosing loops (tm/chkmem, ttt). Cheapest correct proxy for "will this
   starve someone": `L->getParentLoop()`. First cut: only rewrite when
   `L->getParentLoop() == nullptr` OR the parent loop's header has low
   BlockFrequency relative to L. This is a heuristic, not a real pressure
   estimate — label it as such in the code comment, and lean on the corpus +
   production gate to catch mistakes.
3. **Decide prep-only vs prep+pin default.** Test both: pin (`Z80PinLoopPointer`)
   may be unnecessary if the allocator already keeps the pointer in a pair once
   the old IV is gone; if pin is needed, turn it on together. Measure.
4. **Validate** on all four benchmarks (T-states must not regress sieve/e,
   should improve ttt/tm — the confirmed non-nested targets), full corpus
   (`compiler-comparison-corpus/sweep.sh`), full lit, runtime suite
   (`cargo run -- clang`), and **production triplet byte-identical**
   (rcbios/cpnos/autoload — the FDC loops in §3.3 must stay identical; any diff =
   the nesting/cold gate is too loose). NOTE: fannkuch is Depth-3 nested (§3.1),
   so a correct Phase-1 nesting gate will DECLINE it — fannkuch staying at its
   baseline 36.2M is the *expected* Phase-1 outcome, not a failure.
5. **Ship default-on** only if net-positive and production byte-identical.
   Every codegen change ships a **lit test** (FileCheck pins the loop: base
   materialized in preheader, `CHECK-NOT: add hl` / `ld hl,GV` inside the loop,
   `add hl,de` walk present) AND a **runtime fixture** for the chkmem over-read
   correctness (an `/* expect */` sum over an array with a sentinel).

**Phase 1b success:** ttt and tm move to ≤ sdcc T-states; sieve/e unchanged;
production byte-identical; lit+runtime green. (Reachable only if Phase 1a
returned GO; a NO-GO is a legitimate Phase-1 outcome, not a failure.)

### DEFAULT-ON DECISION = **NO-GO** (2026-07-12, triplet measured)

The coverage fix + nesting gate + on-demand preheader are LANDED and correct as
an **opt-in** pass (`-z80-loop-instr-form-prep [-z80-pin-loop-pointer]`);
production is byte-identical by construction while the pass stays off. Flipping
it default-on was measured on the production triplet (baseline = pass OFF, as
today, vs pass ON = prep+pin):

| component | baseline | pass ON | delta |
|-----------|----------|---------|-------|
| autoload (PROM, ZX0) | 2035 B | 2047 B | **+12 B** |
| cpnos prom1-lineprog  | 2010 B | 2019 B | **+9 B** (payload byte-identical; `init.bin` 604→610) |
| rcbios BIOS (.cim)    | 5918 B | 5918 B | **byte-identical** |

Two of three regress; none improve. Root cause matches the Phase-1a beneficiary
caveat: production has **no genuine flat `base[i]` beneficiaries** — its loops
are `pc++`-style already-walking or nested (declined by the gate), so enabling
the pass only adds pin/preheader overhead without eliminating any base reload.
The wins are confined to the synthetic flat corpus loops (sieve-init shape),
which production doesn't contain.

**Verdict:** keep the pass **opt-in**; do NOT flip default-on. No MAME boot was
needed (production output unchanged with pass off). Revisit only if/when a
production component grows a genuine flat scale-1 `base[i]` scan hot enough to
pay for the pin.

### Phase 2 — pressure-aware model for the nested case (hard, optional, stretch)

Goal: also win the nested sieve KILL loop (the −1.05M that Phase 1 leaves on the
table by declining nested loops), *without* the +1.31M scan regression.

Options (evaluate empirically, decision at implementation time — genuine fork,
bring findings to @ravn before committing to one):
- **(2A) Real MIR-level profitability.** Move the go/no-go decision into
  `Z80PinLoopPointer` (or a new MIR loop pass) where `LiveIntervals` /
  `RegisterPressure` are available; only pin when a pair is free through the
  enclosing loop's live range. `prep` stays conservative (Phase-1 gate); pin
  becomes the real gate. Awkward split (prep already mutated IR) but uses true
  pressure info.
- **(2B) BFI-weighted pressure heuristic in prep.** Keep it IR-level but weight
  the "extra live pair" cost by the enclosing-loop trip count / block frequency,
  so an inner loop nested in a hot outer loop must clear a higher bar. Cheaper,
  still a heuristic.
- **(2C) Co-optimize with sink.** The scan regression is partly the M3 cold-IV
  hoist (#256). Enabling `Z80SinkColdLoopIV` *frees* scan-loop pairs; measure
  prep+pin+sink **with the nesting gate removed** — sink may create enough
  headroom that the KILL-loop pointer no longer starves the scan loop. (Last
  session measured prep+pin+sink together = still +217K exec, so 2C alone is
  insufficient, but combined with a tighter end-ptr materialization it may flip.)
- **(2D) Generic upstream angle (do NOT file without go-ahead).** Both #250 and
  #256 are, at root, LLVM cost models that are register-count- and
  block-frequency-blind on a 3-pair target (verified same IR on z80/avr/msp430 in
  the issue body). A block-frequency-aware LSR `AddRecCost` is the general fix.
  Per `feedback_upstream_routing_two_targets` + user rule "never file fixes
  upstream, only issues with testcases", this stays a **held observation** unless
  @ravn explicitly asks; #256 already holds it.

### Phase 3 — fold in the siblings (#249, #251) if Phase 1/2 machinery covers them

The `HLReg` class already exists. #251/#249 are the *counter/pointer regalloc
conflict* (pointer shuttled through IY) — a different trigger (function-arg
pointer coalesced into BC at entry) but the same "keep the walking pointer in a
main pair" goal. If Phase-1's pin path generalizes, wire it to the
`bench_word_fill.c` case and close #251 with a red-green test. Lower priority
(corpus-only, no production impact).

---

## 6. Gates & discipline (from AGENTS.md / CLAUDE.md — non-negotiable)

- **Baseline first**, then change, then re-measure with the *same* oracle.
- **Building is not behaving** — run the z88dk-ticks value oracle + runtime suite
  before any "done"; do not commit on size/lit alone.
- **Every compiler change ships a lit test** (CI-gated `build-and-lit`); add a
  runtime fixture when correctness is only observable at runtime (CI-gated
  `runtime-tests`).
- **Production triplet byte-identical** is the hard regression gate — these
  firmware targets have no byte-array kernels, so #250 work MUST NOT change them.
- **Default-OFF until proven** — keep new gating behind the existing flags; flip
  default-on only with the full corpus + production evidence in the commit body.
- **No PR unless asked**; commit locally, push only at a merge or on request.
- **File issues, not fixes, upstream**; generic-LLVM observations stay held.

## 7. Open questions for @ravn (ask before Phase 2)

1. Target baseline: **beat sdcc** (Phase 1 achieves this for ttt/tm; sieve/e
   already win) or the harder **beat dcc** (needs Phase 2 + the #244 divide work
   for `e`)? The 2026-07-09 plan targeted dcc; this session's data is vs sdcc.
2. Is benchmark/runtime speed worth the complexity given **near-zero production
   impact**? Firmware DOES contain M5-shaped loops (autoload `_fdc_result`/
   `_fdc_cmd`, §3.3) but they are cold/call-bounded so they cost nothing — the
   "off the critical path" conclusion stands. If firmware-finishing is the
   priority, Phase 1 alone (cheap, safe) may be the right stopping point.
   Counterweight: the §3.1 red/green shows a real ~20% on a hot byte/word-array
   kernel, so any future CP/M app with such a loop benefits.
3. Phase-2 approach (2A MIR-pressure vs 2B BFI-heuristic vs 2C sink-combo) is a
   genuine fork — bring measurements, let @ravn pick.

## 8. Key file/line references

- `llvm/lib/Target/Z80/Z80LoopInstrFormPrep.cpp`: `collectAddrGroups` :130,
  `registerPressureOK` :177, `tryEliminateOldIV` :210, innermost gate :395,
  flags :89 / :105.
- `llvm/lib/Target/Z80/Z80PinLoopPointer.cpp`: `constrainRegClass` :243, flag :81.
- `llvm/lib/Target/Z80/Z80SinkColdLoopIV.cpp` — #256/M3 sink.
- `Z80RegisterInfo.td` — `HLReg`.
- `Z80TargetMachine.cpp` `addIRPasses()` — pass wiring (post-LSR slot).
- Prior writeup: `tasks/session-2026-07-09-sink-cold-loop-iv.md`.
- Prior plan (dcc-targeted): `tasks/plan-2026-07-09-beat-dcc-benchmarks.md`.
- Living index: `tasks/known-suboptimal-codegen.md` → M5 (this), M3/M2 (#256).
- Detector/oracle: `tasks/tools/m5-loop-reload-scan.py` (2026-07-12) — enumerates
  M5 base-reload instances from clang/llc `-S`; used for the §3 sweep.
- Issues: #250 (this), #256 (M3 sink), #251/#249 (IY pointer shuttle), #258
  (benchmark tracking), #244 (`e` i16 divide).
- Bench harness: `scratch/dcc-clang-bench/{build_compare.sh,ticks_cpm.py}`;
  corpus: `rc700-gensmedet/tasks/compiler-comparison-corpus/sweep.sh`.
