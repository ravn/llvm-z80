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

### B11. MachineLICM call-hoist heuristic — RESOLVED 2026-06-08 via count-based refinement

- **Status:** RESOLVED via ravn/llvm-z80#220 (count-based threshold).
- **Final state at HEAD:** `Z80InstrInfo::shouldHoist` now counts
  already-hoisted preheader defs whose vreg is used inside the loop,
  refuses when count >= `-z80-licm-call-hoist-threshold` (default 2,
  reflecting IX + IY callee-saved pair budget under sdcccall(1)).
  Default ON (`-z80-licm-block-on-call=true`).
- **Final measurements (vs pre-#23 disablePass baseline):**
  - AES -Oz: -51 B text, -9.0 % tstates
  - AES -O2: -132 B text, -9.2 % tstates
  - autoload PROM: +18 B compressed / +58 B raw (down from +25/+64 with
    the binary heuristic OFF; cap still has 372 B free)
  - cpnos PROM1: -7 B (vs -15 B with heuristic OFF; gives up 8 B for
    autoload relief — acceptable trade)
  - rcbios BIOS: +7 B (unchanged; heuristic doesn't fire on rcbios's
    non-call hoist)
- **Lit + runtime:** 149 PASS + 4 XFAIL; 854 PASS / 0 FAIL across O0..Oz.
- **Pointers:** ravn/llvm-z80#220, session writeup,
  `Z80InstrInfo.cpp::shouldHoist`.

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

**Last updated:** 2026-06-08 (session adding the icmp-narrow sound
gate v1 + v2 and surfacing M1).
