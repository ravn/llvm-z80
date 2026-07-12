# Known suboptimal codegen — living document

**Purpose.**  A single place to track Z80 codegen patterns we KNOW are
suboptimal but cannot (or will not) fix yet.  Per-session writeups go in
`tasks/sessionNN-*.md`; this file is the cross-session **index** of
unresolved issues so a new session can see the open landscape without
spelunking commit history.

**Discipline.**  One entry per known pattern.  Each entry has:

- **Status** — `parked` (won't fix this generation), `accepted`
  (won't fix without a trigger), `awaiting-X` (blocked on something
  specific), `wontfix-mechanism` (documented as inherent).
- **Impact** — best estimate of bytes / tstates and where it surfaces.
- **Why we can't fix it** — the structural reason, in one paragraph.
- **Revisit when** — a concrete trigger that would justify reopening.
- **Repro / pointers** — where to look in the codebase / writeups.

When a pattern is fixed, MOVE the entry out of this doc to a session
writeup that closes it (don't strikethrough — keep this file's signal
density high).

When a NEW suboptimal pattern is discovered: add an entry.  Don't
postpone — empty entries are fine if you don't have impact numbers yet.

---

## Middle-end (target-independent) — patterns blocked by LLVM pipeline shape

### M1. AES K&R `gf_log`-shape: cyclic-phi narrowing through stripped marker

- **Status:** accepted (2026-06-08).
- **Impact:** AES `09_Oz_prod_like` clang +51 % slower than SDCC
  (18.21 M vs 12.08 M tstates).  Size win (−22 %) intact.
- **Pattern:**

  ```c
  uint8_t f(x) uint8_t x; {            /* K&R int-promotion → i16 phi */
      uint8_t atb = 1, i = 0;
      do {
          if (atb == x) break;
          ... bit-7 test, xor with i8 zext ...
      } while (++i > 0);
      return i;
  }
  ```

  The i16-typed `atb` phi is provably narrow by source semantics
  (always `<= 255`) but LLVM's KnownBits can't prove it (cyclic).
  `LazyValueInfo` can prove it WHEN an `(and X, MASK)` marker is
  present — but `CorrelatedValuePropagationPass` then folds the
  marker away as redundant.  `AggressiveInstCombine`'s Phase 2
  (synthetic-trunc-root) needs the marker to fire; by the time it
  runs, the marker is gone.

- **Why Z80 differs from AVR:** Z80's
  `Z80TTIImpl::getPredictableBranchThreshold() = BranchProbability(0,1)`
  makes SimplifyCFG keep branches.  Branch form gives LVI per-edge
  ranges, which lets CVP prove the mask redundant.  AVR keeps the
  mask (select form, LVI has no per-edge info, CVP doesn't fire).

- **Why we can't fix it:** five options analysed
  (`session-2026-06-08-clang-vs-sdcc-speed-investigation.md`):
  - (2) LVI fallback in TruncInstCombine — *tried*; LVI returns
    full-set on post-CVP IR; the constraint LVI would propagate from
    is the mask CVP itself stripped.
  - (3) Change `getPredictableBranchThreshold` — high risk
    (affects every branch decision in the backend).
  - (4) Frontend `!range` metadata on uint8_t-sourced phis — needs
    clang AST/CodeGen work; cleanest architectural fix.
  - (5) Stronger middle-end cyclic-phi range analysis — research-grade.

- **Revisit when:** a finishing-firmware component genuinely depends
  on AES K&R-shape narrowing; user opts into option (4) frontend
  work; upstream lands stronger range analysis.

- **Pointers:** `tasks/session-2026-06-08-clang-vs-sdcc-speed-investigation.md`,
  `tasks/upstream-5bug/draft-cvp-strips-narrowness-marker.md`,
  memory note [[project_aes_kr_speed_gap_accepted]].

- **Revalidation + `gf_log` decomposition (2026-06-28).**  Re-ran the
  clang flag sweep on current HEAD (post CRC-sign-test, B17, #242/#243).
  The gap is **unchanged** on equal footing: `09_Oz_prod_like` clang
  **18.21 M** vs zsdcc **12.08 M** (+50.8 %), byte-for-byte the same
  18.21 M as 2026-06-08.  (A separate plain-`-Oz` build is 17.02 M /
  +41 %; the ~7 % delta is the production size knobs
  `-disable-lsr/-machine-licm/-machine-cse`, NOT a narrowing change —
  don't mistake the two configs for "the gap narrowed".)

  Drilled the AES gap into **`gf_log`** (not just the `gf_alog`/`sb_inv`
  Phase-2 root above) and decomposed its inner loop into TWO concrete
  waste sources, **both** caused by the SAME i16 `atb` phi.  The phi
  stays i16 because of the loop-top `if (atb == x)` test, lowered as
  `icmp eq i16 atb, x`:

  1. **20 T/iter — loop-invariant `x` spilled and reloaded.**  Greedy
     emits `SPILL_GR16 $hl` in the preheader and places the
     `RELOAD_GR16 %stack.0` (= `ld de,(__sfrend_gf_log-2)`, ED-prefix
     extended addressing, 20 T) **inside** the loop.  Verified from the
     post-greedy MIR.  Root mechanism: `x` is a `uint8_t` but is held as
     a 16-bit `gr16noir` PAIR (it arrives in `$hl`), and `atb`=`$hl`
     already pins one pair, so keeping `x` in a second pair across the
     register-hungry body loses to spilling.  If `x` were an 8-bit
     `gr8` (one of 7 single regs) the spill wouldn't happen.

  2. **8 T/iter — redundant bit-7 carry recompute.**  `add a,a` (the
     `atb<<1` shift) already leaves CY = bit 7, and the only intervening
     op (`ld c,a`, saving the shift result) preserves flags — yet the
     bit-7 test re-derives the same carry with `ld a,l; rlca` before
     `jr nc`.  In ISOLATION the backend is optimal here (`add a,a;
     jr c` — verified with a standalone repro); the recompute appears
     ONLY because `atb` lives in `HL` (so the shift result must be
     shuttled out to `C` and `A` reloaded), which is again the i16-phi
     consequence.

  Control: **`gf_alog`** has the identical bit-7 GF step but keeps
  `atb` in a single 8-bit reg (`D`) because its loop counter is a
  separate `while (x--)`, not an `atb == x` compare — and its codegen
  is already OPTIMAL (`add a,a; jr nc; xor $1b; xor d`).  So the cause
  is specifically "the i16 compare keeps `atb` wide", not the GF step.

  Confirmed neither `instcombine` nor `aggressive-instcombine` narrows
  the `%5 = phi i16` to i8 (it is provably `<= 255`: seeds at 1, only
  ever XORed with `zext i8` values, so the high byte is always 0 — but
  proving it needs the cyclic-phi width analysis of option (5)).  No
  fresh cheaply-fixable cause; this stays accepted.  Repro:
  `/tmp`-style minimal `gf_log` + `llc -mtriple=z80 -O2 -print-after-all`,
  inspect MIR after `greedy`.

### M2. BSS load/store traffic dominant in 8-bit memory writes

- **Status:** wontfix-mechanism (#74 production-density drill).
- **Impact:** 30–48 % of large BIOS functions; the dominant residual
  vs SDCC.  ISA-fundamental.
- **Pattern:** 8-bit memory access is A-register-only (`LD (nn),A` /
  `LD A,(nn)`).  Any non-A value used at a BSS site requires
  A-shuttle moves (`LD A,r; LD (nn),A` etc.).
- **Why we can't fix it:** every approach explored (#172 A-pin,
  multiple regalloc-cost-model variants) was net-negative in
  production.  Mechanism documented in
  `tasks/issue184-wontfix-mechanism-2026-05-30.md`.
- **Revisit when:** new Z80-specific A-shuttle elimination idea with
  positive A/B evidence on real production code.
- **#172 A-pin re-validated on current compiler (2026-06-28).**  Flipped
  `-mllvm -enable-z80-pin-alu-accumulator=true` (the connected-component
  pin, the 5th and most sophisticated #172 attempt) and re-measured:
  AES `09_Oz_prod_like` `.text` **4502 -> 4503 B (+1 B)**; cpnos
  prom1-lineprog **byte-identical** (no candidates fire — cpnos has no
  tight ALU-accumulator loops); on `gf_log` the pin even PESSIMIZES
  (adds a `push de`/`pop de`).  Confirms the parked verdict on fresh
  binaries: **A-shuttle count is conserved** — A is the sole 8-bit ALU
  destination AND the sole `IN`/`OUT`/`LD (nn),A` operand, so pinning the
  carrier relocates the shuttle (plus a boundary `LD`) rather than
  removing it.  CRUCIAL distinction: `gf_log`'s cost is **M1 (the i16
  `atb` phi width)**, NOT the A-shuttle — Path C (#172) and M1 are
  distinct root causes, and the A-pin addresses neither `gf_log` nor the
  #173 8-bit BSS spills (spills come from register PRESSURE; pinning does
  not create registers).  Only theoretical avenue left: an ISel
  snapshot-rotate reshape (carrier as XOR *destination* + separate
  snapshot vreg) — multi-week, payoff bounded because production AES
  already beats SDCC.
- **Benchmark data point (2026-06-27, dcc comparison sweep):** the
  Byte `sieve` benchmark is a clean non-BIOS witness of this effect.
  clang **33.0 M** tstates vs dcc **28.5 M** (clang **+14 %**); both
  AGREE (identical output), and the hot loops contain NO runtime calls,
  so the gap is pure codegen.  Suspected dominant cause (NOT yet
  isolated by cycle-profiling — symptom verified, cause a hypothesis):
  the static-stack scan loop spills/reloads its i16 counter to BSS
  every iteration — `ld (__sfrend_main-2),bc` ... `ld bc,(__sfrend_main-2)`
  over 81,910 scan iterations x 10 passes — the M2 mechanism on a
  16-bit IV.  Secondary: the inner kill loop recomputes the element
  address each iteration (`ld hl,_flags; add hl,bc`, no pointer
  strength-reduction -> see M3) and shuffles `k` between BC and HL.
  CAVEAT against over-claiming: dcc's code is *more* verbose here
  (reloads `k` from its IX frame 3x/iter, uses 3-byte `jp` not `jr`)
  yet still wins, so the 14 % is not cleanly attributable to a single
  clang deficiency.  Repro: `cd dcc && scripts/compare3.sh sieve`.
  Runtime-library speed gaps from the same sweep (e/tstring/tqsort,
  NOT codegen) are tracked as ravn/llvm-z80 #244/#245/#246.
- **Root cause ISOLATED by cycle-profiling (2026-07-09).**  The sieve gap
  is NOT primarily the 8-bit A-only mechanism nor a bare static-stack
  increment -- it is **M3 (LSR hoisting conditionally-used IVs) coupled
  to register pressure**, and the two passes #250 add (loop-instr-form-prep
  + pin-loop-pointer) only fix the INNER kill loop, not this scan loop.
  Verified from asm + ntvcm/ticks profile:
  * The middle SCAN loop (`for i: if(flags[i]) {...}`) runs 81,910 x 10
    iterations.  LSR strength-reduces `prime = 2*i+3` and
    `k_start = 3*i+3` -- values used ONLY inside the rarely-taken
    `if(flags[i])` branch (~2 % of scans, the prime density) -- into
    loop-carried IVs advanced EVERY scan iteration (`inc iy` x3 +
    `inc de` x2 = ~42T) plus the scan counter `i` reloaded/stored to BSS
    each iteration (`ld bc,(__sfrend_main-2); inc bc; ...`).
  * dcc keeps `i` in an IX frame (`inc (ix-d)`, in place) and recomputes
    `prime`/`k_start` on demand only when `flags[i]` is true.
  * `-mllvm -lsr-complexity-limit={8,32}` does NOT remove the derived
    IVs -- they are not a tunable LSR-formula choice, so TTI's NumRegs
    penalty (already first in isLSRCostLess, getNumberOfRegisters()=3)
    does not prevent the hoist.  The IVs are only USED under a
    low-probability branch, but LSR advances them unconditionally.
  * The inner kill loop IS fully fixable: with both #250 passes on it
    becomes the optimal dcc form (`ld (hl),a; add hl,bc; <ptr cmp>;
    jr c`, ptr=HL/stride=BC/end=DE, no spill).  But net sieve still
    REGRESSES (+9.3 % ticks) because prep makes the scan loop carry an
    extra flags-pointer, deepening the scan-loop spill cascade above.
  * VERDICT: beating dcc on sieve needs a generic middle-end fix --
    teach LSR not to strength-reduce an IV whose only uses are guarded
    by a low-probability branch on a register-starved target (block-freq
    aware AddRecCost), OR sink the `prime`/`k_start` recompute into the
    taken branch.  This is multi-week, generic-LLVM-scoped, and the
    payoff is benchmark-only (production firmware already beats SDCC).
    Parked with data; #250 passes stay opt-in.  Repro:
    `scratch/dcc-clang-bench/build_compare.sh` +
    `-mllvm -z80-loop-instr-form-prep -mllvm -z80-pin-loop-pointer`.
- **UPDATE 2026-07-09 -- both fix directions implemented + measured**
  (opt-in, default OFF; full writeup
  `tasks/session-2026-07-09-sink-cold-loop-iv.md`):
  * `Z80SinkColdLoopIV` (`-z80-sink-cold-loop-iv`) IMPLEMENTS the "sink
    the recompute into the taken branch" fix: a post-LSR IR pass that
    RAUWs cold-only seed-IV phis to a recompute at the cold NCD, deleting
    the every-iteration `inc`.  Measured **sieve -2.3 %, E/TTT/TM +/-0 %,
    all correct** (z88dk-ticks, -Os).  Only -2.3 % because the SCAN loop
    it fixes is secondary -- profiling shows the KILL loop is ~65 % of
    exec (M5, #250), the SCAN loop only ~26 %.
  * `Z80PinLoopPointer` (`-z80-pin-loop-pointer`, + HLReg class) pins the
    KILL-loop pointer to HL.  Kill loop becomes optimal (2.70M->1.65M
    exec) but SCAN loop regresses 1.06M->2.38M (whole-function regalloc
    cascade: HL pin + prep pointer-inits + IY spill in the hot scan
    path), net **+1.4M T-states WORSE** than sink alone.  Stays opt-in.
  * Net: sink is the shippable non-regressing partial win; the dominant
    KILL-loop gap still needs the generic block-freq LSR +
    pressure-aware M5 rewrite.  Both passes default OFF -> production
    byte-identical (verified: pass absent from default -debug-pass).
  * TRACKING: the M3 scan-loop half now has its own issue
    **ravn/llvm-z80#256** (LSR hoists cold-only IVs -> BSS spill on a
    register-starved target; Z80SinkColdLoopIV opt-in mitigation; generic
    block-freq-blind AddRecCost angle held for upstream w/ go-ahead).

### M3. Loop strength reduction creates wider IVs harmful on Z80

- **Status:** mitigated (Z80NarrowIV pass + LSR cost-less), not eliminated.
- **Impact:** historically catastrophic (16-bit IVs spilled across
  Z80's 3-pair register file); now mitigated to occasional residual.
- **Pattern:** LSR introduces an i16 induction variable for an
  originally i8 loop counter.  Z80NarrowIV pass narrows back when
  SCEV proves narrowness.
- **Why mitigated, not fixed:** Z80NarrowIV handles the
  scev-provable cases.  Other LSR widening (more general) still
  fires.  `isLSRCostLess` (register-count-first) penalises wide IVs
  but doesn't ban them.
- **Revisit when:** new failing-case repro from production.

### M4. Frontend uint8_t int-promotion loses narrowness signal

- **Status:** structural; needs frontend cooperation.
- **Impact:** every K&R-style or auto-promoted uint8_t arithmetic
  expression at the IR boundary (M1 is the worst-hit case but not
  the only one).
- **Pattern:** clang frontend lowers `uint8_t x` parameters to
  default-promoted `int` (i16 on Z80) without emitting any
  narrowness annotation.  Downstream passes have to re-discover the
  narrowness from the body — which often fails (M1).
- **Why we can't fix it:** clang doesn't emit `!range` metadata for
  the boundary today.  Adding it is a frontend AST/CodeGen change.
  Cross-target benefit (AVR + MSP430 + WebAssembly).
- **Revisit when:** a user opts into option (4) frontend work, or
  clang upstream adds it for other reasons.

---

## Backend (Z80-specific) — patterns blocked on backend infrastructure

### B1. IY allocatable as general 16-bit register — gated on regalloc cost-model

- **Status:** parked (#38).
- **Impact:** modest (residual `push/pop iy` density in wide-int /
  float).  See `tasks/issue112-189-iy-leak-taxonomy-2026-05-25.md`.
- **Pattern:** with `-z80-unreserve-iy`, the regalloc may allocate IY
  for normal-priority values, but the FD-prefix overhead exceeds the
  spill savings on production code.  Currently default-off.
- **Why we can't fix it:** needs a proper cost model that accounts
  for the FD-prefix penalty per use site vs the spill savings —
  RA-call-graph work.
- **Revisit when:** a real production function shows measurable IY
  cost vs spill win.

### B2. hasFP=false runtime bug for static-stack

- **Status:** parked (#12).
- **Impact:** 8 B per function with no stack args (PROM total ~24 B
  potential).  Some functions get BIGGER (fdc_read_data +26 B).
- **Pattern:** with `+static-stack` and no stack arguments, IX could
  be freed for allocation, but the PROM hangs after banner (ISR/
  timing-related, not stack imbalance).
- **Why we can't fix it:** runtime hang root cause not identified
  after multiple investigation rounds.
- **Revisit when:** a static-stack profile shows it on the critical
  path, or someone identifies the ISR/timing root cause.

### B3. DJNZ nested loop depth

- **Status:** parked (no issue number).
- **Impact:** modest (DJNZ saves 2 bytes per loop; only outer fires).
- **Pattern:** DJNZ fires but on outer loop of nested pairs (RA hint
  can't distinguish inner from outer).  Inner-loop DJNZ would need
  pre-RA loop-depth analysis.
- **Why we can't fix it:** would require pre-RA pass.
- **Revisit when:** a production function with hot nested loops
  surfaces.

### B4. Rematerializable constants held in IX across calls/LDIR

- **Status:** parked (#15).
- **Impact:** ~10 B in boot_main; ~5–15 B elsewhere.
- **Pattern:** the allocator places cheap-to-rematerialize constants
  (`LD rr,imm` / `LD rr,sym`) in IX to keep them alive across
  calls/LDIR, costing 13 B (PUSH IX + LD IX + copy-out + POP IX)
  vs 3 B for rematerialization at the use site.
- **Why we can't fix it:** needs allocator to prefer remat over
  callee-saved IX for cheap constants — non-trivial RA work.
- **Revisit when:** density audit shows it as the dominant residual.

### B5. Duplicate `LD rr,imm` peephole

- **Status:** parked (#16).
- **Impact:** ~1–2 B per duplicate site.
- **Pattern:** `LD HL,$68e4; LD DE,$68e4` should become
  `LD HL,$68e4; LD D,H; LD E,L`.  Post-RA peephole in
  `Z80LateOptimization`.
- **Why we can't fix it:** straightforward peephole, just not
  written.  Low priority.
- **Revisit when:** a density audit shows it.

### B6. Conditional RET with epilogue duplication crashes with -ffunction-sections

- **Status:** parked (no issue).
- **Impact:** N/A (the optimization is disabled to avoid the crash).
- **Pattern:** epilogue-duplication-based conditional RET emission
  crashes when each function is in its own section.
- **Why we can't fix it:** not investigated.
- **Revisit when:** -ffunction-sections becomes a build requirement
  (e.g. for linker gc-sections wins).

### B7. Machine outliner disabled (CALL overhead > most instructions)

- **Status:** wontfix-mechanism.
- **Pattern:** the cost of a CALL (3 B + 17 T) exceeds most outlined
  candidate sequences on Z80.
- **Why we can't fix it:** ISA-fundamental; outlining only pays for
  very large patterns.
- **Revisit when:** a workload surfaces an outline-able pattern
  > 6 B and frequent enough to amortize.

### B8. EXX spill conversion not a free register pool

- **Status:** wontfix-mechanism (#7).
- **Pattern:** users sometimes expect `+shadow-regs` (EXX) to act
  like extra registers for spill reduction.  It's a CONTEXT SWITCH
  that swaps all three pairs (BC, DE, HL) atomically.  Cannot be
  inserted at arbitrary points.
- **Why we can't fix it:** ISA-fundamental.  Safe only at function
  entry/exit or after CALL.
- **Revisit when:** never; documented limitation.

### B9. BSS overlay (call-graph-based BSS sharing)

- **Status:** parked alongside hasFP=false (#12 era).
- **Impact:** unknown; potentially significant on call-graph-shaped
  PROM/BIOS code.
- **Pattern:** sequential BSS layout per function.  Call-graph
  analysis could share BSS slots between non-overlapping callers.
- **Why we can't fix it:** the overlay algorithm worked but
  interacts with hasFP=false, both parked together.
- **Revisit when:** B2 reopens, or someone shows a clean separation
  from hasFP=false.

### B10. Mixed-mode BSS (direct for locals, IX-indexed for stack args)

- **Status:** parked, not currently needed.
- **Impact:** zero on current PROM (all functions globals-only, no
  stack args).
- **Pattern:** currently IX-indexed (6 B per access) for any function
  with stack args; direct BSS would be 3–4 B for locals if mixed.
- **Why we can't fix it:** straightforward but unnecessary today.
- **Revisit when:** PROM source switches to per-function locals with
  stack args.

### B11. MachineLICM call-hoist heuristic — SUPERSEDED 2026-06-08 by tiered cost model

- **Status:** SUPERSEDED.  The #220 count-based heuristic was reverted
  (had a presence-cost bug); the replacement is the multi-phase
  tiered cost-model refinement landed across Phases 2-4 of the
  cost-model project (see
  `tasks/plan-z80-cost-model-refinement-2026-06-08.md`).
- **Replacement infrastructure (default ON 2026-06-08):**
  - `getRegPressureSetLimit` override: GR16 reported as 6 register
    units instead of TableGen's 12, reflecting the 3 cheap pairs
    HL/DE/BC.  Aligns MachineLICM's `CanCauseHighRegPressure` check
    with what regalloc actually does.
  - `Z80InstrInfo::shouldHoist` two-arm veto: refuses to hoist a
    rematerializable instruction when the loop body contains a CALL
    (chapter 1) OR when the preheader already has 3+ rematable defs
    used inside the loop (chapter 2, leaf-loop high-pressure case).
  - `getRematCost` / `getSpillCost` cost-query hooks (Phase 1; not
    yet consumed by any pass besides `shouldHoist`).
  - Master gate `-mllvm -z80-use-tiered-cost-model` (default TRUE).
- **Final measurements (vs pre-#23 disablePass baseline):**
  - AES -Oz: −70 B text / −8.9 % tstates
  - AES -O2: −132 B text / −9.2 % tstates
  - autoload: +51 B raw (recovered 13 B of the post-#23 regression)
  - cpnos / rcbios: unchanged from session-start
  - Lit + runtime: 149 PASS + 4 XFAIL; 854 PASS / 0 FAIL across O0..Oz.
- **Pointers:** ravn/llvm-z80 commits 087953bb4 (#23), b081796b8 (P2),
  81165bdfc (P3 ch1), 4035d3cbd (P3 ch2), 1af0f1b85 (P4 ch1);
  plan in `tasks/plan-z80-cost-model-refinement-2026-06-08.md`.

### B13. IX-as-allocatable confirmed net-negative — RE-VALIDATED 2026-06-08

- **Status:** confirmed wontfix-mechanism (re-validated experiment).
- **Hypothesis tested:** user asked whether the parked #12 "hasFP=false
  PROM hangs after banner" runtime bug might be a stale artifact of an
  earlier broader attempt at making IX allocatable.  We added a
  scoped `-mllvm -z80-unreserve-ix-no-fp` flag that makes IX
  allocatable only when hasFP=false (no allocas, no stack args) and
  optsize + static-stack are set (same gating as IY).
- **Result:** the documented size regression IS REAL on current HEAD.
  Measured deltas vs default-OFF baseline (clean rebuild, three-cell
  no-op-control discipline applied):
  - AES Oz text: 2156 -> 2210 (**+54 B WORSE**)
  - AES Oz tstates: 16.58 M -> 16.60 M (+0.12% WORSE)
  - autoload raw .text: 1969 -> 1974 (+5 B WORSE)
  - autoload rom.o text: 1846 -> 1865 (+19 B WORSE)
  - rcbios BIOS: 5915 -> 5986 (**+71 B WORSE**)
  - cpnos: unchanged (build-env noise only)
- **Mechanism:** each IX-as-allocatable use pays a PUSH IX/POP IX
  (2 B) or LD IXH/IXL (1-2 B undocumented) to shuttle values between
  IX and HL/DE/BC for ops that don't support IX directly.  Shuttle
  frequency exceeds spill-to-BSS savings.  Matches the original
  CLAUDE.md note "fdc_read_data +26B from IX PUSH/POP copies" --
  the cost is structural, not implementation-specific.
- **Why we can't fix it:** for the byte cost to work out, IX-resident
  values would need to be USED directly by Z80 instructions, but the
  Z80 ISA only supports HL natively for most ops.  The PUSH/POP
  shuttle is mandatory; only frequency reduction (i.e. fewer hoists
  in the first place) helps.
- **Revisit when:** never absent a fundamental change to Z80 ISA
  support (which isn't happening).  The runtime-bug parking remains
  in effect; this entry adds the byte-cost angle for completeness.
- **Pointers:** session 2026-06-08 cost-model writeup (the experiment
  is documented but not committed); CLAUDE.md "Known Non-Working"
  section.

### B14. `+undocumented` codegen net-negative on production targets — measured 2026-06-08

- **Status:** measured wontfix-for-production (correctness gate now
  passes after the IY peephole fix in commit 2fa38d87245c; before
  the fix, `+undocumented` miscompiled AES).
- **Hypothesis tested:** user asked whether enabling
  `+undocumented` (which adds IXH/IXL/IYH/IYL 8-bit half-register
  ops + SLL + DDCB register-result variants on NMOS Z80) would
  unlock the "IX/IY as spill tier" idea by making IX/IY-resident
  values cheaper to access.
- **Result:** every production target gets LARGER with
  `+undocumented`:
  - AES Oz text: 2156 -> 2171 (+15 B)
  - AES Oz tstates: +0.07% slower
  - AES O2: unchanged
  - autoload PROM compressed: 1671 -> 1678 (+7 B)
  - autoload raw .text: 1969 -> 1971 (+2 B)
  - autoload `_main_relocated`: unchanged
  - cpnos PROM1 / payload: unchanged
  - rcbios BIOS: 5915 -> 5920 (+5 B)
  - lit + runtime suites: clean
- **Mechanism:** the half-register ops save 1-2 B at single sites
  where IX/IY-resident 8-bit values are accessed directly, but the
  compiler's response to having those ops available is to keep more
  values in IX/IY for longer periods, which adds shuttle traffic
  (PUSH IX/POP rr or undocumented LD r,IXH chains) elsewhere.  Net:
  every byte column wider.  Matches the structural byte-arithmetic
  analysis from the same session (`LD A,(IX+0)` = 3 B = same as
  direct BSS `LD A,(nn)`).
- **Side-finding (now resolved):** witnessed AES `_aes_mixColumns`
  miscompile when `+undocumented` was enabled.  Root cause: the
  unused-IY-save/restore peephole in `Z80LateOptimization.cpp`
  matched a `POP_IY` in the entry block with a `PUSH_IY` in the
  loop body, treating them as a save/restore pair when they were
  actually parts of two SEPARATE `PUSH HL ; POP IY` ... `PUSH IY ;
  POP HL` value-transfer patterns spanning the loop's back edge.
  Erasing them left `PUSH HL` (entry) and `POP HL` (loop body)
  unbalanced across the back edge.  Fixed in commit
  ravn/llvm-z80@2fa38d87245c (require both ops to live in the
  function's entry MBB).
- **Revisit when:** a specific workload demonstrates a measurable
  win, OR the Z80 backend's `+undocumented` codegen quality
  improves (e.g. a regalloc hint that says "prefer IX/IY for
  long-live-range values when half-register ops are available").
  Production targets remain `+undocumented`-disabled until then.
- **Pointers:** ravn/llvm-z80@2fa38d87245c (the IY peephole fix);
  session writeup `session-2026-06-08-cost-model-phases-0-through-4.md`
  (the cost-model arc this branched off of).

### B12. CSE-induced over-hoist in tight leaf loops — partial mitigation; ~50 B residual

- **Status:** SUPERSEDED 2026-06-08 same-day by B15.  The "+51 B raw"
  residual was specific to the LICM+CSE default; with CSE now disabled
  by default (B15) the residual goes away.  Kept for history.
- **Impact:** autoload-in-c PROM is +13 B compressed / +51 B raw vs
  pre-#23.  The leaf-loop high-pressure veto (Phase 3 ch2) recovers
  about 13 B of the original +64 B regression; the remaining +51 B
  is from CSE de-duplications that the veto's "count of preheader
  rematable defs" doesn't fully capture.
- **Pattern:** CSE de-duplicates small constants (port addresses,
  comparison constants) used multiple times in `define_sextants`'s
  nested loops.  The de-duplicated vreg has multiple in-loop uses;
  LICM hoists.  Regalloc spills to BSS because cumulative pressure
  exceeds the cheap-pair budget.  Phase 3 ch2's veto fires for some
  of these but not all.
- **Why we can't fix it (this session):** would need either (a) a
  Z80-specific pre-CSE filter that prevents CSE on small-constant
  patterns in pressure-heavy leaf loops, or (b) a `getCSECost` hook
  on `TargetInstrInfo` upstream that CSE consults.  Either is
  Phase 3 chapter 3+ territory.
- **Revisit when:** the +51 B raw begins to threaten the 2 KB PROM
  cap (currently 377 B free); OR a Z80 backend session focuses on
  CSE cost-model integration.
- **Pointers:** `tasks/plan-z80-cost-model-refinement-2026-06-08.md`
  Phase 3 chapter 3 (CSE wiring is mentioned as "open design
  question"); session writeup for Phase 4 ch 1.

### B15. Branch Folder unsound hoist exposed by MachineCSE — ROOT-CAUSED + FIXED 2026-07-01

- **Status:** ROOT-CAUSED + FIXED 2026-07-01 (same mechanism as
  ravn/llvm-z80#247, the clang -O2 fannkuch miscompile).  The fix is a
  generic two-operand change in `llvm/lib/CodeGen/MachineOperand.cpp`:
  `MO_MCSymbol` `isIdenticalTo`/`getHashValue` now also compare/hash
  `getOffset()` (previously ignored, unlike MO_GlobalAddress etc.).  The
  Z80 static-frame lowering attaches a nonzero offset to an MO_MCSymbol
  via `setOffset()` (`Z80InstrInfo.cpp:1147,...`); branch-folder's
  `isIdenticalTo` treated `__sfrend-2` and `__sfrend-4` stores as equal
  and tail-merged them, dropping one -> wrong result.  Upstream filing to
  `llvm/llvm-project` prepared, held for the user's per-filing go-ahead
  per HARD rule `feedback_explain_before_filing`.  **Attribution VERIFIED
  by A/B (2026-07-01):** reproducing the pi trigger via `llc -O2
  -z80-enable-cse pi_o2.ll`, the fix-reverted baseline llc FAILS
  (exit=1) and the fixed llc PASSES (exit=0) — so this change, not #248's
  orthogonal shape-mitigation, is what root-fixes B15.  See
  `rc700-gensmedet/tasks/clang-fannkuch-O1-backend-miscompile-2026-06-28.md`
  and lit test `llvm/test/CodeGen/Z80/branch-folder-mcsymbol-offset-247.ll`.
- **Original status (kept for history):** KNOWN BUG, PARKED 2026-06-09
  (user-directed).  Root cause in generic LLVM (Branch Folder), not the
  Z80 backend; production builds are NOT affected because the trigger MIR
  shape requires MachineCSE, which is OFF by default in our fork
  (Z80TargetMachine.cpp `EnableMachineCSE` cl::opt default FALSE).
- **History:** #23 retirement (2026-06-08, earlier same day) defaulted
  both LICM and CSE to ON, citing "AES -Oz -8.9% tstates / -13 B"
  and "#198 -O2 miscompile no longer reproduces."  Same-day
  re-evaluation of clang vs SDCC across the full
  compiler-comparison-corpus surfaced `pi llvm-z80 FAIL(exit=1)` --
  not from the cost-model project (Phases 0-4 toggle leaves it
  unchanged), but from the CSE enable.  Bisecting LICM vs CSE
  independently (corpus + `aes256-corpus/probe_cse.sh`):
  - LICM+CSE on : pi FAIL, AES aes_text=2156 ts=16,577,307
  - LICM only   : pi PASS, AES aes_text=2238 ts=16,571,818  (faster!)
  - both off    : pi PASS, AES aes_text=2226 ts=18,214,790
  The −8.9% AES tstates win comes from **LICM, not CSE**; CSE only
  contributed size (+79 B aes_text when off).  Disabling CSE keeps
  the speed win and fixes pi.
- **Reproducer:** `rc700-gensmedet/tasks/compiler-comparison-corpus/`
  → `./sweep.sh` will surface `pi llvm-z80 FAIL(exit=1)` when CSE is
  enabled (`-mllvm -z80-enable-cse`).  pi computes a checksum of pi
  digits to 800 places via spigot algorithm; expected 28116, with
  CSE-on returns some other value.  Not yet minimised to a small
  IR-level test.
- **Why not yet filed upstream:** filing-READY 2026-06-09 (still
  awaiting user go-ahead per HARD rule explain-before-filing).
  Root cause confirmed: **Branch Folder (`llvm/lib/CodeGen/BranchFolding.cpp`)
  unsoundly moves a store from `bb.0`'s tail to `bb.1`'s head when
  `bb.1` has multiple predecessors and the hoisted store depends on a
  register whose live-out value differs between predecessors.**
  Toggle `-mllvm -disable-branch-fold` alone (with CSE on) restores
  correctness.  MachineCSE merely exposes the bug by collapsing a
  forward-only prelude block.  Generic LLVM pass → upstream route is
  `llvm/llvm-project`, NOT the fork (per HARD rule
  `feedback_upstream_routing_two_targets`).  Full writeup in
  `tasks/session-2026-06-09-pi-cse-miscompile-investigation.md`;
  69-line `.ll` reducer at `/tmp/pi_reduce_out.ll`.
- **Cost of mitigation:** vs LICM+CSE config, defaulting CSE off
  costs:
  - autoload: 1652 → 1673 B (+21 B; 375 B free in 2 KB cap)
  - cpnos PROM1: 2023 → 2030 B (+7 B; 18 B free in 2 KB cap)
  - rcbios BIOS: 5897 → 5905 B (+8 B)
  - AES Oz aes_text: 2156 → 2238 B (+82 B; not size-bounded)
  - AES Oz tstates: 16.577M → 16.572M (-5k ts, marginally faster)
- **Revisit when:** a reduced reproducer exists; or a Z80-specific
  CSE filter can identify the bad transformation; or the autoload
  +21 B becomes binding.
- **Pointers:** Z80TargetMachine.cpp lines 86-112 (the EnableMachineCSE
  cl::opt + rationale comment); `aes256-corpus/probe_cse.sh` for
  the three-state A/B; commit flipping default to false.

### B16. CPIR/CPDR not used for memchr/memcmp/strchr lowering — accepted ZeroYield 2026-06-09

- **Status:** accepted (ZeroYield on the four production firmware
  components).  Re-survey trigger only.
- **Impact:** zero today.  A C-source `memchr` / `memcmp` / `strchr` /
  `strlen` / `strncmp` call would lower to a libcall (or an
  open-coded byte-by-byte loop) instead of `CPIR` / `CPDR`.  A
  CPIR-fused `memchr` on 256 B would be 2 B + ~21 T/byte vs a typical
  open-coded loop at ~15+ B + ~40 T/byte (rough estimate; not
  measured against a concrete witness because there is none).
- **Current state of the backend:** CPI/CPIR/CPD/CPDR are defined
  (`Z80InstrInfo.td:106-109`) but no GISel pattern, libcall expansion,
  or middle-end combiner lowers a C-level `memchr` / `memcmp` /
  `strchr` to them.  `cpnos-in-c/src/runtime.s:48-71` has a
  hand-written `_memchr` using CPIR — but it is **unreferenced** by
  any C source and is linker-stripped.  Verified 2026-06-09: `grep`
  across autoload-in-c / cpnos-in-c / rcbios-in-c returns zero
  C-source references to those symbols.
- **Why we can't fix it (yet):** not a mechanism block — the surface
  is implementable (a GISel combiner that turns
  `G_INTRINSIC @llvm.memcmp.eq` / library-call `memchr` into a
  CPIR-based pseudo, plus a runtime `_memcmp`/`_memchr` switch in
  compiler-rt).  But with zero in-tree witnesses on the four
  finishing-firmware components, the work has no measurable payoff.
  Per the session #74 production-density verdict ("cheap codegen +
  regalloc levers are exhausted; the remaining high-value compiler
  work is upstream-submission packaging") we don't invest in
  motivator-less ISel coverage.
- **Revisit when:** any of the following surfaces:
  - A new firmware component or corpus benchmark uses `memchr` /
    `memcmp` / `strchr` / `strlen` / `strncmp` from C source.
  - A libc (#35) lands that wires `string.h` to compiler-rt — then
    the compiler-rt `_memcmp` / `_memchr` themselves can be CPIR-based
    (single hand-written file, no compiler change needed).
  - A motivating benchmark in `compiler-comparison-corpus` materially
    regresses vs SDCC due to byte-loop memchr / memcmp / strchr.
- **Pointers:** `ravn/llvm-z80#7` issue + 2026-06-09 correction
  comment; `rc700-gensmedet/cpnos-in-c/src/runtime.s:48-71` for the
  hand-written CPIR pattern (would be the model for any future
  compiler-rt implementation).

### B17. Multi-byte (i32/i64) arithmetic materializes carries via `sbc a,a` instead of threading the native ADC/SBC chain — FIXED 2026-06-24 (`Z80FuseCarryChain`, see `tasks/b17-fuse-carry-chain-2026-06-24.md`)

**FIXED.** New post-RA `Z80FuseCarryChain` pass threads the inter-limb carry in
the carry FLAG for add/sub chains with a dead terminal carry (rewrites the
`_CO`/`_CIO`/`_BO`/`_BIO` pseudos to real `ADD_HL_rr`/`ADC_HL_rr`/`SBC_HL_rr`).
add32 `sbc a,a` 2->0 / −5 instr; i64 add 4->0 / −11 instr.  Production
byte-identical (BIOS 5462, autoload 1945/1481); lit 173+6; runtime 872 PASS;
new `fuse-carry-chain.ll` + `test_224_carry_chain.c`.  Original analysis below.



- **Status:** open, needs-root-cause (discovered in the dcc-corpus
  three-compiler comparison, `z80-utils/compiler-zoo/cpm_zoo.py`).
- **Impact:** the dominant driver of clang's **1.5–2.4× raw-code gap
  vs zsdcc on integer-arithmetic tests**.  Measured 2026-06-24 (raw
  code+rodata of the test TU, no runtime):
  - `triangle` clang 323 B vs zsdcc 137 B (2.4×)
  - `e`        clang 503 B vs zsdcc 242 B (2.1×)
  - `fact`     clang 238 B vs zsdcc 151 B (1.6×)
- **Pattern:** for multi-byte add/sub/compare, clang materializes the
  carry/comparison result into a register as 0x00/0xFF via `sbc a,a`,
  then `and 1` / masks / spills it — instead of consuming the flag
  directly.  Instruction-count witness (`grep -c 'sbc a,a'`):
  triangle **14**, fact **10**, e **2**; zsdcc emits **0** on all
  three.  zsdcc instead threads the native carry chain
  (`add a,lo; adc a,..; adc a,..; adc a,hi`) across the bytes and
  branches on flags (`or`-chain + `jr NZ` for the `==0` test).
- **Why not fixed yet:** NOT root-caused.  Unknown whether the
  `sbc a,a` originates in GISel legalization of i16/i32 G_ADD/G_SUB
  carry handling, in icmp lowering, or as residue of wider (i33)
  overflow arithmetic.  Per `feedback_file_bugs_not_fixes` /
  `feedback_verdict_after_real_pass_output`, no issue is filed and no
  fix proposed until a minimal repro pins the emitting pass.  Note it
  is ADJACENT to but distinct from #93 (constant-trip carry-test
  loop), #120 (`SBC A,A` mask/carry-roundtrip peephole deletion), and
  #216 (`sbc a,a` as a *wanted* select-lowering) — those treat
  `sbc a,a` as either a loop idiom or a desired output; B17 is its
  OVERUSE as the default multi-byte carry-propagation shape.
- **Revisit when:** TASK queued — minimal i32 `a+b` / `a==0` repro,
  `-print-after-all` to find the first pass emitting the `sbc a,a`,
  AVR cross-check (`feedback_avr_density_oracle`).  Then decide
  GISel-legalization fix vs late-opt peephole vs upstream.
- **Pointers:** `compiler-zoo/cpm_zoo.py` (the comparison that
  surfaced it); dcc `tests/{triangle,fact,e}.c`.

### M5. Scale-1 char-array loops miss pointer strength reduction

- **Status:** open, root-caused to MIR level (2026-07-04).  IR-level fixes
  (LSR cost tweak, pre-isel pointer-IV pass) are undone by canonicalization;
  a pre-RA MachineFunction pointer-IV pass is the only viable lever.
- **Impact:** 16–40% slower inner loops on byte-array scan/modify benchmarks
  (sieve −16%, e −22%, nqueens −21%, tqsort −43% T-states vs dcc).  Does not
  affect production targets directly (rcbios/cpnos/autoload lack tight
  char-array loop kernels); relevant for any future CP/M app with inner loops
  over byte arrays.
- **Pattern:** a loop `for (k = start; k <= size; k += prime) flags[k] = 0`
  emits `ld hl, _flags; add hl, bc` on EVERY iteration — 21 T wasted
  reloading the base address.  DCC keeps a running pointer in HL with
  `add hl, de` (11 T stride advance only), reaching ~39 T vs ~90 T per
  iteration.  The "pointer form" IR (manually verified) compiles to ~57 T
  and requires no IY.
- **Why LSR won't fix it:** `isLSRCostLess` weights `NumRegs` first.  The
  pointer form needs 3 live pairs (ptr, stride, bound) vs the integer form's
  2 (index, stride), so it loses at the first tiebreak.
- **CORRECTED 2026-07-04 — the fix must be MIR-level, NOT IR-level.** Two
  earlier hypotheses in this entry were wrong on the current backend:
  (1) "`opt -passes=loop-reduce` on the sieve IR is a no-op" is now STALE —
  LSR *does* transform, emitting an integer-IV `%lsr.iv` + `getelementptr
  @flags, %lsr.iv` (verified `opt -mtriple=z80 -passes=loop-reduce`).
  (2) The proposed IR-level fixes (a Z80GEPStrengthReduce *pre-isel* pass, or
  an `isLSRCostLess`/`NumBaseAdds` tweak) would be UNDONE by canonicalization.
  Proof: hand-writing the inner loop as a pure pointer IV in IR (`%p = phi
  ptr [@flags+start], [%p+prime]`, compare `%p < @flags+8191`) still compiles
  to `ld hl,_flags; add hl,bc` every iteration — and is even *larger*.  The IR
  is canonicalized back to `offset-phi + gep(@flags, offset)` *before*
  IRTranslator, and this persists with BOTH `-disable-lsr` AND `-disable-cgp`
  (the rewrite rides in around the SCEV/LoopInfo-requiring passes; exact pass
  not pinned but irrelevant).  `offset-phi + gep(base, offset)` is LLVM's
  universal canonical IV form for `array[i]` loops — free on `[base+reg]`
  targets, a per-iteration base reload on Z80 where `BaseGV + reg` is an
  illegal addressing mode (`isLegalAddressingMode` correctly returns false;
  the cost is real, LSR just can't spend the extra register to avoid it).
- **CORRECTION 2026-07-04 (same day, follow-up):** "must be MIR-level" above
  is TOO STRONG.  It was based on testing with `-disable-lsr`/`-disable-cgp`
  and hand-writing the pointer IV in *clang-frontend* IR (before LSR/CGP run)
  — that test is valid for that timing but doesn't generalize.  An IR-level
  pass DOES survive if placed AFTER LSR in the pipeline.  Prior art in-tree:
  `PPCLoopInstrFormPrep.cpp` ("update form" case) does exactly this rewrite
  for PowerPC using `ScalarEvolution`+`SCEVExpander`, registered via
  `addPreISel()` — which runs after `TargetPassConfig::addIRPasses()`
  (LSR included) and right before ISel.  Z80 already has this exact slot
  in use: `Z80PassConfig::addIRPasses()` calls the base `addIRPasses()`
  (which runs LSR) and THEN adds `Z80PatternFillRecognize`/`Z80LoopRotate`
  (`Z80TargetMachine.cpp:366-374`) — proven to survive to final asm.  Also
  ruled out empirically: `-lsr-preferred-addressing-mode=postindexed` (the
  TTI `getPreferredAddressingMode` hook) has ZERO effect on the repro — Z80
  has no hardware post-increment addressing mode at all, so that hook (built
  for ARM MVE/Hexagon/PowerPC update-form instructions) doesn't apply.  See
  ravn/llvm-z80#250 comment (2026-07-04) for the full writeup and concrete
  implementation plan (a `Z80LoopInstrFormPrep` FunctionPass modeled on
  `Z80PatternFillRecognize`, registered in the same post-LSR slot).
- **NOT Z80-specific — confirmed on AVR + MSP430 (2026-07-04).**  Same
  `killidx` kernel via `llc -O2 -mtriple={avr,msp430,z80}`:
    * **MSP430**: inner loop is `clr.b flags(r12); add r13,r12; cmp #8191,r12`
      — the `symbol(reg)` indexed mode folds `flags+r12` for free, so the
      integer-IV form is OPTIMAL.  LLVM's canonical form is correct here.
    * **AVR**: inner loop `mov r26,r24; mov r27,r25; subi r26,lo8(-(flags));
      sbci r27,hi8(-(flags)); st X,r1; add r24,r22; adc r25,r23; ...` — keeps
      the integer index in r24:r25 and RE-ADDS the base every iteration into X.
      IDENTICAL shape to Z80's `ld hl,_flags; add hl,bc`.
    * **Z80**: `ld hl,_flags; add hl,bc; ...`.
  Conclusion: this is a target-independent LLVM canonical-IV choice that
  assumes `base+reg` addressing is cheap (true on MSP430/x86/ARM); it penalises
  every target LACKING `symbol+reg` addressing (AVR, Z80).  AVR — a mature
  in-tree target whose `isLegalAddressingMode` also rejects `symbol+reg` — does
  NOT form a pointer IV either, strong evidence that LSR simply won't spend the
  register regardless of addressing-mode info.  => a generic-LLVM improvement
  (benefits AVR too) is conceivable; per upstream discipline that would route
  to llvm/llvm-project with explain+go-ahead.  Pragmatic path stays a Z80 MIR
  pass.
- **Revisit when:** a **pre-RA MachineFunction pass** (or a very-late IR pass
  immune to re-canonicalization) recognises the selected `LD HL,GV; ADD HL,rr`
  (from `G_PTR_ADD(G_GLOBAL_VALUE, offset-IV)`) inside a loop and rewrites it
  to a genuine pointer IV: materialise `GV` once in the preheader into a pair,
  carry `HL = base+offset` across iterations advancing by the stride
  (`add hl,de`), and rewrite the exit test to a pointer compare.  This is the
  only lever that survives IR canonicalization.
- **Pointers:** dcc-corpus investigation 2026-06-25; repro:
  `dcc/tests/sieve.c`, `dcc/tests/e.c`.  Tracking issue: ravn/llvm-z80#250
  (filed 2026-07-04, full problem statement + repro + lit testcase).
- **Re-verified 2026-07-04** (post upstream-dcc merge + backend gains): fresh
  asm read of `sieve` confirms the mechanism unchanged.  clang inner loop
  (`BB0_5`, the multiple-killing loop) keeps the *integer index* `k` in `bc`,
  re-emits `ld hl,_flags; add hl,bc` every iteration, and shuffles `bc`↔`hl`
  for the `k += prime` update (~13 insns); dcc walks a `char*` in `hl` with a
  single `add hl,de` + pointer-vs-endpointer compare (~7 insns).  CAVEAT: of
  the compare3 tests where dcc beats clang, only `sieve` is a *pure* M5 codegen
  case.  `tqsort`/`tbsearch` are RUNTIME-LIBRARY confounds — dcc's qsort/bsearch
  are hand-written asm in `DCCRTL.MAC` (Shell sort), clang links an O(n^2)
  insertion-sort `qsort` in C (`z80-utils/cpm/cpm_stdlib.c`).  `tstring` clang
  hits the 2e9-tstate ceiling (capped, not measured).  `e` is a mix of divide-
  helper impl (clang 1× combined `___divhi3`; dcc 2× `__divs`+`__mods`) and
  scale-2 array indexing — NOT isolated to M5, do not cite it as pure M5.
- **New data point 2026-07-09 — `tm` (dcc malloc test) is the worst M5 case
  seen: 4.27x slower (clang 339.4M vs dcc 79.4M T-states, ticks oracle).**
  The gap is NOT the heap: per-PC profiling (`ntvcm -g`) attributes 22.3M of
  the 32M instruction-executions to `_chkmem`, the byte-verify loop
  `for (i=0;i<c;i++){ if (*pc!=val) err; pc++; }`.  Its inner loop `.LBB0_4`
  (1,112,100 iters) is the canonical M5 shape AMPLIFIED by register pressure:
  the base `p` is parked in IY and the frame is in IX, so the i16 counter `i`
  spills to the IX frame and is reloaded every iteration, the loop-invariant
  limit `c` is reloaded from the stack param `(ix+4/5)` every iteration, and
  the address is recomputed `push iy / pop hl / add hl,bc` instead of `inc hl`.
  Measured 234 T/iter (20 insns) vs ~50 T/iter (8 insns) for an optimal
  pointer-walk = 4.68x; the ~205M wasted T-states account for essentially the
  whole 260M gap.  Same root cause and same MIR-pass fix as the sieve case;
  the IX-frame + IY-base pressure is why the multiplier is 4.7x here vs 1.2x
  on sieve (which has no frame pointer competing for regs).  Repro:
  `dcc/tests/tm.c`; measure via `scratch/dcc-clang-bench/ticks_cpm.py`.
- **Fix ATTEMPTED 2026-07-09 — `Z80PinLoopPointer` implemented, net-regresses
  sieve, kept opt-in (default OFF).**  The post-LSR IR machinery the #250
  comment proposed exists: `-z80-loop-instr-form-prep` (forms the kill-loop
  pointer IV) + `-z80-pin-loop-pointer` (pins it to HL via a new HLReg class).
  In ISOLATION the kill loop reaches the optimal dcc form
  (`ld (hl),a; add hl,rr; <ptr cmp>; jr c`, 18->11 insns, kill exec
  2.70M->1.65M).  But NET sieve gets +1.4M T-states WORSE: the SCAN loop
  regresses 1.06M->2.38M exec (whole-function regalloc cascade — pinning HL +
  prep's SCEV-expanded pointer-inits + an IY spill all land in the hot scan
  path).  On Z80's 3 pairs the enclosing loop pays for the inner loop's
  registers.  So the M5 rewrite as built is correct but not net-positive on a
  nested-loop kernel; a register-pressure-aware variant that does not steal the
  parent loop's pairs is required before it can go default-on.  Full data +
  per-region exec table: `tasks/session-2026-07-09-sink-cold-loop-iv.md`.  Note
  the SCAN-loop half of the sieve gap is separately mitigated by
  `Z80SinkColdLoopIV` (M2/M3, sieve -2.3 % clean) — see the M2 UPDATE entry.
- **Systematic sweep 2026-07-12 — the pattern is broader than the dcc
  benchmarks, and it costs where the loop is hot (fannkuch −20 % verified).**
  Built a repeatable detector (`tasks/tools/m5-loop-reload-scan.py`) that
  flags every in-loop `add hl/ix/iy` and tags `GLOBAL-BASE(sym)` per-iteration
  base reloads.  Ran it over the compiler-comparison corpus + production
  firmware (each with its real flags).  New / confirmed instances:
    * **fannkuch** `_perm` (hot reversal-swap loop) + `_count` (colder
      bookkeeping) — NOT previously cited in #250.  Red/green: rewriting the
      hot `perm[i] <-> perm[k-i]` loop as two running pointers (`*lo`/`*hi`)
      drops fannkuch from **36,203,397 → 28,949,757 T-states (−20.0 %) and
      584 → 536 B (−48 B)**, both PASS (result 0x10E4).  So the reload cost is
      real and large even though clang still WINS the fannkuch lane
      (competitors pay it too) — corrects the earlier hunch that fannkuch's
      loop "isn't hot enough to matter."  Measured with the SIZE cell flags
      (`-Oz -disable-lsr`, the shipping/correct config; note fannkuch's -O1/-O2
      SPEED cell is a separate verified backend miscompile, returns 0).
    * **production autoload `rom.c`**: `_fdc_result` and `_fdc_cmd` are genuine
      per-iteration `ld hl,<global>; add hl,de` reloads — so #250's literal
      "rcbios/cpnos/autoload lack tight char-array kernels" is imprecise.  BUT
      both are cold, ≤7-iteration, call-bounded FDC loops (each iter does a
      `call _fdc_read/write_when_ready`), so the 21 T reload is noise: the
      practical "no production perf impact" claim HOLDS.
    * cpnos/rcbios: a `transport_pio.c` apparent hit was a false positive (the
      `add` was in the fall-through EXIT block, not the loop) — fixed the
      detector's block attribution (read `in_loop` off the block-start line,
      not accumulated over the body); re-scan clean.
  Upshot: M5 is not a benchmark curiosity — any CP/M app with a hot byte/word
  array reversal or scan loop pays ~20 %.  Reinforces the "register-pressure-
  aware pointer-IV rewrite" as the real fix (the isolated-loop rewrite already
  reaches optimal; the blocker is the nested-loop regalloc cascade above).

### M6. Z80LowerSelect pre-compute forces IY in pointer-scan loops

- **Status:** open, root-caused.
- **Impact:** `strrchr` (cpm_stdlib.c) inner loop ~80 T vs optimal ~37 T
  (2.2× slowdown).  Affects any "find last occurrence" / "conditional pointer
  update" loop: `while (*s) { if (*s == c) last = s; s++; }`.  On tstring,
  strrchr contributes ~316M of 3342M total T-states; fixing it saves ~170M
  (~5%).
- **Pattern:** `G_SELECT cond, new_ptr, old_ptr` in a back-edge loop is
  lowered by Z80LowerSelect to a "pre-compute + restore" pattern:
  `DE = s (true-value, always); if match: keep DE; else: DE = IY (save reg)`.
  This forces `old_ptr` (last) into IY as a save register, causing
  `push iy; pop de` + `push de; pop iy` (42 T wasted) per iteration.
  Physical registers confirmed by `-print-after=virtregrewriter`: `$hl`=s,
  `$b`=c_char, `$c`=cur_char, `$iy`=last, `$de`=temp.
- **Optimal form:** hand-crafted IR with a "conditional update" CFG (branch
  on match, only update DE=HL when match, fall-through unchanged) produces
  `$bc`=s, `$l`=c, `$de`=last — NO IY.  Inner loop: 37 T (no match),
  57 T (match).
- **Why not fixed yet:** Z80LowerSelect lowers `G_SELECT` to a triangle CFG
  without detecting the "conditional update" special case where
  `false_val == incoming phi value`.  For that case the optimal lowering is:
  test condition; if-true: `ld d,h; ld e,l`; fall-through unchanged.  No
  pre-compute, no IY needed.
- **Revisit when:** Z80LowerSelect is extended to detect
  `select cond, new_val, phi_self` and emit the branch-to-update form.
- **Pointers:** dcc-corpus strrchr investigation 2026-06-25; repro:
  `/tmp/strrchr_mini.c`; optimal IR: `/tmp/strrchr_opt.ll`.

### Note — production `-disable-lsr` confirmed stale (relates to M3, #232/#234)

The 2026-06-24 comparison independently reproduced #232's conclusion:
carrying `-mllvm -disable-lsr` (the historical "LSR is harmful"
sledgehammer) is **net-negative on general loops** — it cost the
`clangp` flavor **−38 % speed and +57 B on tqsort** (LSR's
strength-reduction of `base+i*size` index math is a real win;
disabling it recomputes the index every iteration).  Production already
removed the flag (#234, verified no-op 2026-06-21); `cpm_zoo.py`'s
`clangp` was updated to drop it too, after which clangp no longer
regresses vs plain clang.  The residual M3 concern is only the
*counter-widening* half of LSR, which `Z80NarrowIV` + `isLSRCostLess`
already mostly contain — the right long-term lever is finishing that
cost model, never a global LSR off-switch.

### B18. OTIR/OTDR/INIR/INDR block-I/O idiom not recognized — accepted ZeroYield 2026-06-28

- **Status:** accepted (ZeroYield on the four production firmware
  components).  Re-survey trigger only.  Filed at the user's request
  after the autoload SIO-B debug skeleton was forced to hand-write the
  `otir` in inline asm.
- **Impact:** zero on shipping size today (the one production OTIR site,
  rcbios `bios_hw_init.c` SIO programming, is already hand-written inline
  asm; autoload's new debug-only SIO init likewise).  A C-source
  fixed-count block-output loop `for (i=0;i<N;i++) port_out(C, tab[i])`
  emits a 5-instruction loop body (`ld a,(de); out (c),a; inc de; dec hl;
  jr`) ≈ 8 B + ~? T/byte, vs a single 2 B `otir` at 21 T/byte.  Saving is
  ~6 B + the loop overhead per such site.
- **Pattern (verified 2026-06-28):** with the live `build-macos` clang,
  `--target=z80 -Oz` lowers
  ```c
  for (byte i = 0; i < sizeof seq; i++)
      *(volatile __attribute__((address_space(2))) byte *)0x0B = seq[i];
  ```
  to a manual `ld a,(de); out (11),a; inc de; dec hl; jr` loop — NOT
  `otir`.  Same applies to the input direction (`INIR`/`INDR`) and the
  decrement variants (`OTDR`/`INDR`).
- **Why we can't fix it (yet):** not a mechanism block — a GISel combiner
  or a post-RA peephole could recognize the `{load (HL/DE++); OUT (C),A;
  dec count; branch}` shape and emit the block-I/O pseudo, exactly as the
  CPIR/CPDR case (B16).  But every real OTIR site in the four firmware
  components is already hand-written inline asm (rcbios SIO init), so
  there is no in-tree C-source witness whose size/speed would improve.
  Per the session #74 production-density verdict, motivator-less ISel
  coverage is not invested in.
- **Revisit when:** a firmware component or corpus benchmark drives a
  fixed-count port block-copy from C source (not inline asm) where the
  open-coded loop measurably regresses vs an `otir`/`inir`; or a libc/HAL
  layer wants portable `port_block_out()` helpers lowered natively rather
  than per-target inline asm.
- **Pointers:** sibling of B16 (CPIR/CPDR block-compare idiom);
  hand-written model at `rc700-gensmedet/rcbios-in-c/bios_hw_init.c:174-186`
  (`otir` ×2) and `rc700-gensmedet/autoload-in-c/rom.c` `sio_b_debug_init`.

---

### B20. Walking i16 pointer in a `*p++` loop allocated to IY (push/pop shuttle) even at the IY-reserved default

- **Status:** open, tracked at **ravn/llvm-z80#249** (filed 2026-06-30).
  (ravn/llvm-z80#99 is CLOSED and covered the *sibling* i16 BC-counter
  ping-pong, not this IY pointer-walk; the bench's source comment still cites
  #99 for the "sister reg-class" idea.)  Gap measured + reproduced; re-verified
  2026-06-30.
- **Impact:** compiler-comparison-corpus **`word_fill`** is the only corpus
  *speed* loss after #248 — clang **+63 % slower than zsdcc** (210,147 vs
  128,796 t-states; size still smallest, 178 vs 526 B).  Pure codegen (no
  runtime calls in the loop).  Each iteration spends **three `push/pop` pairs
  (~75 t-states)** shuttling the pointer IY↔BC↔HL.
- **Pattern (re-verified 2026-06-30 on a minimal loop at the production
  default, `Z80UnreserveIY=false`):**
  ```c
  for (unsigned int i = n; i; --i) *p++ = i;   /* i16 counter + walking i16 ptr */
  ```
  lowers to (the pointer is in IY):
  ```
      push hl / pop iy           ; iy = p
  .L: ...counter test...
      push iy / pop bc / inc bc / inc bc   ; bc = p+2
      push iy / pop hl           ; hl = p ; ld (hl),e ; inc hl ; ld (hl),d
      push bc / pop iy           ; iy = p+2  (advance)
      jr .L
  ```
- **Why it happens:** with HL needed for the store address, DE for the
  counter/value, and BC for `p+2`, the walking-pointer vreg is placed in IY and
  copied in/out via the documented `COPY16_PUSHPOP` shuttle every iteration —
  **despite IY being the reserved production default** (the pointer vreg's
  reg-class still lets the allocator reach IY).  The bench was authored as the
  witness for this: the proposed fix (sketched against the now-closed #99) is a
  **sister single-register class for the pointer vreg** (sibling of the existing
  `BCReg` counter class) so the coalescer/allocator can't drag the pointer off
  HL into IY/BC.
- **Revisit when:** the pointer reg-class lands (baseline doc estimates loop-1 →
  parity with SDCC, `bench_run` −47 %); or a production hot loop shows the same
  `*p++` i16-pointer-walk shape (none today — corpus-only).
- **Repro / pointers:** `BENCH=word_fill ./sweep.sh`;
  `rc700-gensmedet/tasks/compiler-comparison-corpus/bench_word_fill.c` (authored
  as the #99 witness) + `word_fill_baseline_2026-06-08.md` (per-iteration
  T-state breakdown + the −47 % fix sketch); ravn/llvm-z80#99.

---

### B21. Inner-loop stride-N index not strength-reduced → recomputed shift-multiply, cascading to an IY invariant-shuttle + counter spill

- **Status:** open, unfiled (documented here 2026-07-01).  **ZeroYield on
  production runtime** — the witness is boot-only one-shot code — but the
  *pattern class* (a stride-constant address inside an inner loop) is generic
  and can appear in hotter code.
- **Witness:** `autoload-in-c/rom.c` `define_sextants()` (inlined into
  `_main_relocated`), the SEM702 font transpose-copy:
  ```c
  for (ch = 0; ch < 128; ch++) {
      port_out(chargen_char, ch); port_out(chargen_dot, 0);
      for (line = 0; line < 16; line++) {
          port_out(chargen_data, sem702_font[((word) line << 7) | ch]);  /* stride +128 */
          port_out(chargen_dot, (byte)((line + 1) & 0x0F));
      }
  }
  ```
  The inner index `&sem702_font[(line<<7)|ch]` advances by exactly **+128 per
  iteration**, but clang recomputes it from scratch each time:
  ```
  ld l,c ; ld h,b
  add hl,hl ×7          ; hl = line<<7   ← recomputed every iteration
  ex de,hl
  push iy ; pop hl      ; hl = &font[ch]  (the loop-invariant base, parked in IY)
  add hl,de
  ld a,(hl)
  ```
- **Cascade (this is the interesting part):** because `hl` is destroyed by the
  `line<<7` shift-chain, the loop-invariant base `&font[ch]` has to be *saved*
  across iterations.  The allocator parks it in **IY** and shuttles it back with
  `push iy ; pop hl` (3 B) every iteration; and because IY + DE are both consumed
  by that dance, the **outer `ch` counter is spilled to a BSS slot**
  (`ld (__sfrend-2),de` / reload).  One missed strength-reduction thus produces
  ~13 B/iter of address recompute **plus** an IY shuttle **plus** a memory spill.
- **Strength-reduced target:** keep a running pointer in `hl`, `add hl,de`
  (de=128 hoisted) each iteration.  Then `hl` simply *is* the pointer across
  iterations — no `line<<7`, no IY invariant, no counter spill.  All three
  wastes collapse together.
- **SP-is-inviolable observation (user, 2026-07-01 — follow-up filed as
  ravn/llvm-z80#252 on 2026-07-04):** given
  the current shape, even the IY shuttle is suboptimal — `pop hl ; push hl`
  (peek top-of-stack, 2 B) would beat `push iy ; pop hl` (3 B) and free IY.  But
  LLVM's codegen **treats `SP` as inviolable**: stack-resident SSA values are
  reached only through fixed frame indices (`ld hl,(slot)`), never by peeking the
  stack top with a pop/re-push idiom.  So the compiler will never emit that
  hand-asm trick; it picks a callee reg (IY here) or a memory slot instead.
  Inconsistently, it spilled the *counter* to a BSS slot but the *base* to IY.
  **The user flagged the SP-inviolable modelling as a thing to revisit** — worth
  a separate investigation into whether a Z80-aware stack-peek/`ld hl,(slot)`
  choice could ever be cheaper than a callee-reg park.  Investigated
  2026-07-04: the existing spill->PUSH/POP peephole family
  (`Z80LateOptimization.cpp:391-520`, #195/#198/#202/#203/#204) already
  converts a store+matching-reload pair to a PUSH/POP bracket when safe, but
  only as a late cleanup of a store/reload regalloc already emitted -- never
  as a proactive "peek instead of shuttle" strategy.  Scoping issue filed:
  ravn/llvm-z80#252.
- **Related:** shares the IY-invariant-in-inner-loop shape with **B20**
  (ravn/llvm-z80#249); the underlying miss is classic **IV strength reduction**,
  which LSR would normally do but is disabled on Z80 (LSR widens 8-bit counters
  → spills; see CLAUDE.md "Loop Strength Reduction is Harmful").  A *targeted*
  stride-SR that does not widen counters is the shape that would help here.
- **Revisit when:** a production *hot* loop shows a recomputed stride-N inner
  index (none today — witness is boot-only); or the SP-inviolable follow-up is
  taken up.
- **Repro:** `cd rc700-gensmedet/autoload-in-c && make clang_asm` → search
  `define_sextants` / `out (211)`.

---

### B22. Variable shift by a loop induction variable not strength-reduced to an incremental shift

- **Status:** open, unfiled (documented 2026-07-01).  **ZeroYield on production
  runtime** (witness is a boot-time status render), but measurable PROM size.
- **Witness:** `autoload-in-c/rom.c` `display_sw1_status()` rendering the 8 SW1
  bits:
  ```c
  for (i = 0; i < 8; i++) *p++ = '0' + ((sw >> i) & 1);   /* sw >> IV */
  ```
  Z80 has no variable shift, so `sw >> i` lowers to an `srl a; djnz` loop that
  re-shifts **`i` times each iteration** — O(n²) over the 8 bits.
- **Why it happens:** `sw >> i` with `i` a simple IV is a recurrence
  (`sw>>i == (sw>>(i-1))>>1`) reducible to one `>> 1` per iteration by
  maintaining a running shifted value.  LLVM's LSR/OSR only targets
  multiply/GEP/address recurrences, **never shift-by-IV**, so nothing performs
  this even with LSR enabled — and on Z80 LSR is disabled anyway (see CLAUDE.md
  "LSR is Harmful").  There is **no cost on normal targets** (variable shift is
  one cheap instruction), which is exactly why upstream LLVM has no incentive —
  it is Z80-specific.
- **Fix / workaround:** hand-rewrite to `*p++ = '0' + (sw & 1); sw >>= 1;`
  (constant shift-by-1, no inner loop).  Measured **−5 B PROM** in autoload.
- **Related:** same family as **B21** (stride-N index recurrence) and the
  CLAUDE.md "LSR is Harmful — but there IS a beneficial use case" note.  Both are
  IV-driven recurrences (`<<`/`>>`/`*k`) recomputed from scratch; the shared fix
  is a **Z80-aware operator strength reduction that does not widen the counter**
  (the thing that made generic LSR harmful).
- **Revisit when:** a production hot loop shows a variable-shift-by-IV; or the
  Z80-aware non-widening SR (B21/B22 shared fix) is taken up.
- **Repro:** the pre-fix form is in git history of `display_sw1_status`;
  `make clang_asm` on it shows the `srl a; djnz` inner shift loop.

---

### B23. Multi-site inline LDDR can't beat a hand-written shared memmove helper

- **Status:** wontfix-mechanism (2026-07-08).  Inherent; documented.
- **Impact:** rcbios `insert_line` 2-site screen scroll: `__builtin_memmove`
  with all folds = 5947 B vs hand-written `lddr_copy` = 5908 B (**+39 B**).
  Even hypothetical guard-elision (−8 B) leaves +31.
- **Pattern:** `base = p + i; memmove(base + K, base, C - i)` at N≥2 sites.
  After the three 2026-07-08 folds (direction / runtime-term cancellation /
  constant-address-base immediate; see
  `tasks/session-2026-07-08-memmove-lddr-lowering.md`) the inline form is
  tight — `ld hl,$ff7f; ld de,$ffcf; <guard>; lddr` — but still loses.
- **Why we can't fix it:** four structural mismatches between the flexible
  `memmove` intrinsic (start pointers, size may be 0, arbitrary context) and
  the rigid `LDDR` instruction (end pointers, `BC=0`→65536-byte copy, pins
  HL+DE+BC).  Bridging costs "glue" paid **per site** when inlined; a shared
  helper amortizes guard + count-pop + LDDR + return over all callers.  For
  N≥2 sites of one shape the shared helper wins by construction.  This is the
  general-contract-vs-hand-tuned tradeoff, not a missing optimization.
- **Revisit when:** a single-site hot memmove where inline (no call/amortize
  pressure) is the right call — there the folds already win.  Do NOT chase
  multi-site parity.
- **Repro / pointers:** `tasks/session-2026-07-08-memmove-lddr-lowering.md`
  (full four-mismatch analysis); rc700 `rcbios-in-c/clang/runtime.s`
  (`lddr_copy`) and `bios.c::insert_line`.

### B24. Runtime-count LDDR keeps a `BC==0` guard even when count is provably non-zero (ravn/llvm-z80#255)

- **Status:** accepted (2026-07-08) — needs a two-part fix, low value.
- **Impact:** +4 B/site (`ld a,b; or c; jr z`) on every runtime-count inline
  LDDR/LDIR, e.g. both `insert_line` sites.
- **Pattern:** `if (count) memmove(dst, src, count);` — the `LDDR_GUARDED`
  BC==0 guard is redundant with the C `if`, but emitted anyway.
- **Why we can't fix it:** the G_MEMMOVE→LDDR lowering uses unguarded `LDDR`
  only when `Size` is a compile-time **constant** (`SizeC ? LDDR :
  LDDR_GUARDED`); it never checks *known-non-zero* for a runtime `Size`.
  Verified: neither the dominating `if(count)` nor `__builtin_assume(count !=
  0)` changes the choice.  A fix would need (a) the legalizer to query
  `GISelKnownBits::isKnownNonZero(Size)`, AND (b) the non-zero fact to survive
  to GISel — `llvm.assume` is typically dropped before GISel, so (b) is
  uncertain.
- **Revisit when:** a workload where runtime-count block copies dominate size
  AND the callers reliably guard non-zero.  Otherwise leave it — correctness
  (avoiding the 65536-byte BC=0 disaster) is worth 4 B.
- **Repro / pointers:** `Z80LegalizerInfo.cpp` G_MEMMOVE case (`SizeC ? LDDR :
  LDDR_GUARDED`); `tasks/session-2026-07-08-memmove-lddr-lowering.md` §"four
  mismatches" #2.

### B25. `-O1`/`-O2` slower than `-Os` on integer-loop benchmarks — loop rotation adds BSS spills (2026-07-09)

- **Status:** accepted (2026-07-09) — root cause identified, no fix attempted.
- **Impact:** `sieve.c` -O1 is 6.7% slower than -Os (28.03M vs 26.25M cycles).
  `e.c` -O1 is 5.5% slower than -Os (29.78M vs 28.15M cycles).
- **Pattern:** `-O1`'s loop canonicalization passes (likely LoopRotate + LICM)
  restructure loop nests in a way that creates more simultaneously-live variables
  across loop back-edges.  The Z80 regalloc runs out of the 4 general-purpose 16-bit
  pairs (BC, DE, HL, and sometimes IX) and spills the excess to BSS.  Each BSS
  spill/reload is ~32 T-states (16T store through A + 16T load through A) vs 0 extra
  T-states for a register-held value.
- **Concrete evidence (sieve.c):** diff of `-Os` vs `-O1` assembly shows 13 BSS
  accesses (`ld r16,(sfrend-N)` / `ld (sfrend-N),r16`) at `-O1` vs 9 at `-Os` —
  4 extra accesses.  At `-Os`, the hot inner loop (depth 3) has zero BSS accesses;
  `de` holds the stride throughout.  At `-O1`, the restructured loop writes and
  re-reads 2–3 extra BSS slots per outer iteration.  At 8191 inner iterations, 4
  extra ×32 T-states ≈ 1M extra T-states ≈ 5–7% of the total, matching the
  observed regression.
- **Root cause classification:** M2 (middle-end, BSS-spill cost) — the cost model
  doesn't know that increasing live-range width on Z80 is expensive because each
  spill goes through the A-shuttle (16T load + 16T store).  `getRegisterSpillCost`
  and the register pressure heuristics in LoopRotate/LICM are not Z80-calibrated.
- **Why not fixed:** fixing requires either (a) TTI `getRegisterSpillCost` returning
  a much higher value to suppress loop transformations that increase spills, or (b)
  teaching the scheduler/regalloc to account for BSS spill cost being `~32 T-states`
  vs `~0` for a register.  Both touch core LLVM infrastructure; (a) might help but
  risks regressing other patterns.  The -Os flag is the correct default for Z80.
- **Recommendation:** always use `-Os` for Z80 production builds.  Document the
  `-O1`/`-O2` counter-productivity in user-facing docs when available.
- **Repro:** `dcc/tests/sieve.c` and `dcc/tests/e.c`, compiled at `-Os` vs `-O1`
  with the clang freestanding pipeline.  Assembly diff: `/tmp/sieve_Os.s` vs
  `/tmp/sieve_O1.s` (scratch; regenerate with
  `clang --target=z80 -{Os,O1} -ffreestanding -nostdlibinc ... -S sieve.c`).
  See `tasks/session-2026-07-09-dcc-clang-comparison.md`.

---

## Frontend — patterns blocked on clang AST/CodeGen work

(See M4 above.  Frontend gaps tend to cascade to multiple middle-end
shapes, so one upstream change can unblock many entries here.)

---

## Per-shape findings filed as issues (older sessions)

These are documented in detail elsewhere; listed here for cross-reference:

- **#141** — i16 comparison against 0x0100 should fold to high-byte
  test (~5 B per site).  Session 60d.
- **#142** — residual i8→i16 zext after `(uint8_t)` cast feeding
  equality compare (~4 B per site).  Session 60d.
- **#143** — #132 multi-fire interaction blocking peer fires.
  Session 60d.
- **#144** — `(a == K) ? -1 : 0` materialises via i1→shift chain
  (~12 B per site).  Session 60d.
- **Code Density Gap Analysis** (BIOS, 2026-05-02): rich per-cause
  breakdown still in CLAUDE.md "Code Density Gap Analysis" — the
  taxonomy is largely M1/M2/B-class entries above.

---

## How to add a new entry

```
### <Mn or Bn>. <Short title>

- **Status:** <parked / accepted / awaiting-X / wontfix-mechanism>.
- **Impact:** <bytes / tstates and where>.
- **Pattern:** <one-paragraph description, IR or asm if helpful>.
- **Why we can't fix it:** <structural reason in one paragraph>.
- **Revisit when:** <concrete trigger>.
- **Pointers:** <writeup / memory note / issue links>.
```

Then bump this file's last-updated note below and commit.

---

**Last updated:** 2026-07-09 (M2/M3 + M5 updated — Z80SinkColdLoopIV opt-in
sieve -2.3 % clean partial win; Z80PinLoopPointer opt-in kill-loop fix
net-regresses via scan-loop regalloc cascade; both default OFF. See
`tasks/session-2026-07-09-sink-cold-loop-iv.md`. B25 added — `-O1`/`-O2`
slower than `-Os` on
integer-loop benchmarks; loop rotation adds BSS spills; root-caused via
sieve.c/e.c assembly diff showing 4 extra BSS accesses per outer iteration
at `-O1` vs zero inside the hot inner loop at `-Os`).
