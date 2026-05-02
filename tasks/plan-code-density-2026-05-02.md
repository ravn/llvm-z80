# Plan: clang/Z80 code-density work after session 35

**Date:** 2026-05-02.  Author session: post-#35 remeasure.
**Working directory:** `/Users/ravn/z80/`.
**Governing principle (memory `feedback_root_cause_over_peephole.md`):**
*Favor upstream fixes over post-RA peepholes.*  When a missed
optimization can be addressed at the IR level, in the GISel combiner,
in the legalizer, in the register-allocator cost model, or via a
MachineDCE-class pass, that path is preferred over adding a new
matcher to `Z80LateOptimization.cpp`.

## 1.  Where we are

Measured 2026-05-02 with `make clang-bios` and per-function `llvm-nm`
sizing on `clang/bios.elf`:

- BIOS = 5920 B (clang).  SDCC BIOS = 6123 B.  Clang **-203 B** overall.
- cpnos-rom payload = 1708 B.  PROM = 1756 B (clang) vs 1910 B (SDCC).
- Lit suite: 76 PASS + 1 XFAIL (#99).
- Open issues: 27 on ravn/llvm-z80.  Active cluster (regalloc/spill):
  #15 / #20 / #89 / #94 / #95 / #98 / #99 / #100.

Per-function profile of the **largest clang BIOS functions** (excludes
data symbols `_conv_tables`, boot blocks):

| Function          | Bytes | BSS-access bytes | reg-reg moves |
| ----------------- | ----: | ---------------: | ------------: |
| `_specc`          |  676  | 208 (31%)        | 58            |
| `_rwoper`         |  263  | 105 (40%)        | 35            |
| `_bg_clear_from`  |  262  | (not profiled)   | (n/a)         |
| `_sec_rw`         |  247  | (not profiled)   | (n/a)         |
| `_bios_seldsk_c`  |  199  |  66 (33%)        | 28            |
| `_isr_crt`        |  166  |  80 (48%)        |  6            |
| `_xyadd`          |  149  |  64 (43%)        | 22            |

IX/IY refs in top BIOS functions: **0** (one stray FD prefix in
`_isr_crt`).  IX-frame and IY-prefix overhead are not active gaps.

## 2.  Diagnostic: the load-after-store-same-addr signal

Within `_isr_crt` the four-line shape

```
ld   hl,(nn)
inc/dec hl                 ; or other rr-preserving op
ld   (nn),hl
ld   hl,(nn)               ; <-- redundant: rr unchanged since the store
ld   a,l
or   h
```

appears at four BSS slots ($fffc, $ffdf, $ffe1, $ffe3).  Strict 4-line
match across all of BIOS finds three definite hits = 12 B; widening
the scan finds the same shape in `_specc`, `_rwoper`, and the disk
selectors.  Estimated aggregate savings from removing the redundant
reload across BIOS: ~50-80 B.

This shape is **diagnostic**, not just an opportunity.  It tells us:

- A dead load is surviving from MIR all the way into assembly, despite
  being a textbook MachineDCE / load-store-forwarding target.
- Either MachineCSE/DCE doesn't run on the Z80 backend, runs too early
  (before the loads are formed), or sees these as may-alias due to
  conservative TargetTransformInfo.
- The right fix is *not* "add a peephole that detects the four lines."
  The right fix is "find the pass that should have caught this, and
  fix it."

## 3.  Strategy

### 3.1  Pipeline-stage taxonomy

Order each candidate fix by where it lives in the codegen pipeline,
preferring upstream:

```
IR  →  GISel Combiner  →  Legalizer  →  InstSelector  →  MachineDCE
       (IR-level fold)    (illegal→        (pseudo →     /CSE/Sinking
                          legal types)    real opcodes)   /Folding
                                                                 ↓
                                              RegBankInfo →  Regalloc
                                              (bank class      (cost
                                               assignment)      model
                                                                + remat)
                                                                 ↓
                                                         MIR-DCE-2
                                                                 ↓
                                                    Z80ExpandPseudo
                                                                 ↓
                                                Z80LateOptimization
                                                    (PEEPHOLE)
                                                                 ↓
                                                       MC / asm out
```

Rule of thumb: if a fix could live at MachineDCE-2 or earlier, it
**must** live there.  Late-opt is reserved for Z80-ISA-specific
patterns with no IR/MIR representation (`EX DE,HL`, `BIT n,A`,
`SBC A,A`, `JR` vs `JP`, `DJNZ`, `RRCA`, etc.).

### 3.2  Cost model for prioritization

For each candidate fix, score on:

- **Leverage**: how many distinct functions does it shrink?
- **Generality**: does it eliminate one shape, or a whole family?
- **Pipeline depth**: earlier = better (catches downstream variants).
- **Risk**: how many tests + how invasive?

The session-32 BSS-spill peephole family (~250 LOC, 3 pred shapes
× 2 body orderings, parked subcase #99) is exactly the warning shape
of "we picked late-opt when we should have picked regalloc."

## 4.  Phased plan

### Phase A — instrumentation (no codegen changes)

Goal: be able to see where pessimism originates in the pipeline.

1. **Per-function size baseline tracker.**
   - Script in `llvm-z80/tools/` (or `rcbios-in-c/tools/`) that runs
     `llvm-nm --print-size --size-sort` on `clang/bios.elf` and
     `cpnos-rom/clang/cpnos.bin` and prints a stable, sorted CSV.
   - Commit the current CSV as baseline; CI step diffs against it on
     each compiler change.  Threshold: **±0 B per function**, fail
     loudly on regressions.
   - Rationale: today, sizes are remeasured by hand and only at
     session boundaries.  #100 was discovered the hard way (flipping a
     default and seeing rcbios +33 B) — that should have been a CI
     red.

2. **MIR-stage dump harness.**
   - Add `-print-after-all` capture for one designated function
     (`_isr_crt`, smallest at 166 B with 48% BSS density).
   - Diff the MIR after each pass and identify exactly which pass
     **should** have killed the redundant reload.
   - Commit the captured MIR snapshots as test fixtures.

3. **Categorize the existing peephole layer.**
   - Read `Z80LateOptimization.cpp` and tag each peephole by whether
     it (a) handles a Z80-ISA-only pattern (legitimate late-opt) or
     (b) is a stand-in for a missed upstream pass (candidate for
     migration).
   - This audit informs the order of Phase B/C work.

**Exit criterion:** the baseline tracker reports any byte movement
on commit; we have an MIR diff that pinpoints the load-after-store
miss to a specific pass.

### Phase B — MachineDCE / load-store forwarding (highest leverage)

Goal: kill the load-after-store-same-addr family at MIR level so
peepholes never see it.

Predicted leverage: 50-80 B aggregate across BIOS, plus an unknown
amount in cpnos-rom + autoload-PROM, plus future-proofing every
similar shape.

1. From Phase A.2 we know which pass is missing the elimination.
   Likely candidates:
   - `MachineCSE` (if BSS-slot loads aren't recognised as memory
     operands the alias-tracker can prove non-aliasing for).
   - `MachineSinking` (if the load is correctly identified but
     not re-evaluated after the store).
   - `RegisterCoalescer` (if the value is stuck in two different
     virtual registers because of a copy that should have been
     eliminated).
   - Custom `Z80MIRDeadStoreLoadForward` pass (last resort, but
     still lives at MIR level, not in late-opt).

2. Fix at the identified pass.  Add a lit test under
   `llvm/test/CodeGen/Z80/` that locks the optimization in.

3. Re-run baseline tracker (Phase A.1) and confirm BIOS shrinks.

**Exit criterion:** each of the four `_isr_crt` redundant reloads
disappears.  Baseline tracker reports negative bytes on at least
4 functions.

### Phase C — regalloc cluster fix

Goal: address #94 / #95 / #98 / #89 as one structural change instead
of four peepholes.

These are tagged in their issue text as "Investigation" / "Long-term"
— the right framing.  Common factor: the regalloc undermodels
Z80-specific liveness around DJNZ, sequential loops, and 16-bit
loop-invariant constants.

1. **Read all four issues** plus #99 / #100 in one sitting; produce a
   single root-cause writeup that ties them together.
2. **Decide on the fix layer**:
   - Cost-model tweak in `Z80RegisterInfo::getCalleeSavedRegs` /
     `getRegPressureSetLimit` /
     `Z80TargetMachine::adjustPassManager`?
   - Pre-RA hint pass like `getRegAllocationHints` (which #92 used)?
   - GlobalISel `RegBankInfo::getInstrMapping` change?
3. Implement once; verify all four regress fixtures close together.
4. Add a rotation-on measurement: with the regalloc fix landed, does
   `Z80LoopRotate` default-on still regress?  If yes, that informs
   Phase D.

**Exit criterion:** at least 3 of {#94, #95, #98, #89} close;
combined size win measurable in the baseline tracker.

### Phase D — rematerialization framework (gates #15 / #99 / #100)

Goal: when a value is needed across a CALL, back-edge, or other live
range that today forces a BSS spill, prefer rematerialization of the
cheap form.

Z80 has unusually-many cheap remat forms: `LD r,nn` (3 B), `LD HL,nn`
(3 B), `LD A,(nn)` (3 B), small constant arithmetic.  Spilling these
to BSS costs the same 3-4 B for the spill **and** 3-4 B for the
reload — strictly worse than rematerialization.

1. Read existing `Z80InstrInfo::isReallyTriviallyReMaterializable`
   (or equivalent) and identify gaps.
2. Audit the regalloc cost model: when is "rematerialize across X" a
   considered alternative to "spill across X"?
3. Extend the recognised remat set to cover BSS-resident loop
   carriers and constant globals.
4. With the framework extended, #15 / #99 / #100 should largely close
   themselves.  Verify by running the tests and remeasuring.
5. **Once Phase D lands and #100 closes, flip `Z80LoopRotate` to
   `cl::init(true)`** (the original goal of #77a).  Re-measure: BIOS
   should drop, not rise.

**Exit criterion:** #15, #99, #100 close.  `Z80LoopRotate` default-on
no longer regresses BIOS or cpnos-rom.

### Phase E — GISel combiner / Legalizer (lower priority)

Goal: catch IR-level pessimism that produces downstream bloat.

Examples to investigate (not committed yet):

- `if (--*p == 0)` could be lowered to a single fused DEC + Z-flag
  test pattern in the Legalizer, eliminating the dead-load that
  Phase B otherwise has to clean up.
- 8-bit loop counters that get widened to i16 in IR should be
  detectable + reverted to i8 at GISel combine time, before any
  i16 arithmetic instructions are selected.

This phase is where we audit the IR vs the desired MIR and ask
"could this have been a smaller IR instead of waiting for late
passes to clean up?"

**Exit criterion:** open as needed only after Phase B/C/D close.

## 5.  Test and verification strategy

- **Lit suite** (`build/bin/llvm-lit llvm/test/CodeGen/Z80/`): must
  stay at 76+ PASS, 1 XFAIL (#99).  Add a new test for every fix.
- **Per-function size baseline** (Phase A.1): no function regresses
  unless explicitly justified in the commit message.
- **MAME boot test**: rcbios + cpnos-rom must still boot to CCP +
  banner.  Run before each merge.  Memory `feedback_screenshot_to_verify`
  applies.
- **SDCC parity check** (lower frequency, gated on Docker rebuild):
  rebuild SDCC BIOS once at the start of Phase B and once at the end
  of Phase D to confirm we're still ahead.  30-min rebuild, do it
  twice across the project, not per-commit.

## 6.  Risks and rollback

- **MIR-level changes can affect non-Z80 backends** if implemented in
  generic LLVM passes.  Prefer Z80-target-specific fixes (a new pass
  in `lib/Target/Z80/`) over generic-pass changes.
- **Regalloc tweaks have wide blast radius**.  Roll back via revert,
  not via additive guard flags — the latter accumulates and obscures.
- **Z80LoopRotate default-on flip** (Phase D exit) is the highest-
  visibility user-facing change.  Hold it until Phase D's measurement
  shows green; do not flip speculatively.
- **Branch hygiene**: each phase gets its own branch off `main`.
  Phase B → Phase C → Phase D, not in parallel — they touch
  overlapping code (regalloc cost model, MIR passes).

## 7.  What this plan deliberately does *not* do

- **No new post-RA peephole** for the load-after-store shape.  The
  user feedback (memory `feedback_root_cause_over_peephole.md`)
  forbids it as a default reflex; the existence of this same shape in
  4+ different BIOS functions is exactly why a peephole is the wrong
  layer.
- **No new GEP-sink-style IR rewrite**.  Session 34 already reverted
  one such rewrite for being IR-level cure-for-MIR-disease.
- **No `cpnos-rom` source-level cleanup pass**.  The remaining
  cpnos-rom bloat is single-digit-percent; the leverage is in BIOS.
- **No SDCC-side benchmarking work** unless Phase B/C/D lands and we
  want to confirm the headline number; the goal is to shrink clang,
  not to track SDCC.

## 8.  First concrete action

Phase A.2 (MIR-stage dump harness) is the smallest, most diagnostic
step and unblocks Phase B.  Suggested commands:

```sh
cd /Users/ravn/z80/llvm-z80
build-macos/bin/clang --target=z80 -mllvm -print-after-all \
    -c /Users/ravn/z80/rc700-gensmedet/rcbios-in-c/bios.c \
    -o /tmp/bios.o 2> /tmp/mir-trace.txt
grep -n 'IR Dump After' /tmp/mir-trace.txt | head -40
```

Identify the first MIR pass after which `MOV $loc, %vreg ; MOV %vreg2,
$loc` survives unfolded for a static-stack BSS slot.  That pass is
Phase B's target.

---

This plan supersedes the loose "next direction options" discussion at
the start of the post-session-35 conversation.  It will be refined as
Phase A produces ground truth.
