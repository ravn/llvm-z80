# Session 73p Phase 2 summary (2026-05-22)

Cross-cutting investigation arc on ravn/llvm-z80 #177 (TTI) and #173
(BSS spill peephole).  Two production-yield commits land; one TTI
bug filed (#184) with reproducer; one peephole follow-up cataloged
(#173 cross-MBB).

## Final production deltas

| Target                            | Pre-session | Post-session | Δ      |
|-----------------------------------|------------:|-------------:|-------:|
| AES `09_Oz_prod_like` (bin)       |        2574 |         2562 |   −12 |
| AES `09_Oz_prod_like` (tstates)   |     10.75 M |      10.74 M | −0.11% |
| cpnos PROM1 (clang)               |        2030 |         2028 |    −2 |
| BIOS (clang)                      |        5925 |         5922 |    −3 |
| Autoload PROM (clang)             |        1652 |         1652 |     0 |
| Z80 lit suite                     |       104+3 |        106+3 |    +2 |

End-to-end verifiers all green:
- AES corpus 13/13 PASS.
- cpnos polypascal-test PASS @ 51.11 s.
- Wider oracle (sieve/fannkuch/pi) byte-identical to baseline.
- Z80 lit suite: 106 PASS + 3 XFAIL.

## Two main work streams

### Stream A: #177 Z80 TargetTransformInfo (partial ship)

Branch: `session-73p-phase2-issue177` (merged via `541b687bbecc`).

Investigation arc (5 docs):
- **Phase A** (`issue177-phase-a-investigation.md`): pass→hook map.
  Empirically found MachineLICM/MachineCSE do NOT use TTI hooks.
  Retired Phase E (would have TTI-gated those passes).  Saved ~1 wk.
- **Phase B0** (`issue177-phase-b0-investigation.md`): predicted
  near-zero impact from Tier 1 hooks based on per-pass reasoning.
  Prediction was WRONG.
- **Phase B1** (`issue177-phase-b1-finding.md`): ran the full Phase-B
  bundle (3 hook overrides) on AES corpus.  `05_Oz_static_stack`
  FAILed at 100M ts (infinite loop); `02_Os` and `04_O2` silently
  miscompiled.  I parked #177.  **Wrong reflex.**
- User redirect: "i am fine with you accidentially introducing bugs,
  as long as you fix them correctly."
- **Phase B2** (`issue177-phase-b2-bisect.md`): bisected the bundle.
  ~15 min per cycle × 4 cycles = ~1 hour.  Isolated cause to ONE
  line: `getArithmeticInstrCost` returning 2 for i16 type.  Filed
  reproducer + asm diff as **ravn/llvm-z80#184**.

Shipped (commit `e585f3301c5f`):
- `prefersVectorizedAddressing=false` (Z80 has no SIMD).
- `getArithmeticInstrCost`: only `Mul -> TCC_Expensive`.
- `getCastInstrCost`: trunc i16->i8 free, zext i8->i16 free,
  sext i8->i16 = 2.

Production delta from Stream A: cpnos PROM1 −1 B.

### Stream B: #173 BSS spill peephole (same-MBB shipped)

Branch: `session-73p-issue173` (merged via `bc85622f9bef`).

Investigation arc:
1. First MVP: scoped for symmetric 4+4 same-MBB pattern.  AES
   sweep byte-identical.  Initial conclusion: "Path A doesn't fire."
2. User redirect: "reinvestigate thoroughly."
3. Programmatic catalog of bare-store + 4-instr-reload pairs in AES
   asm: 11 pairs total, 8 clean candidates.  Real pattern is
   asymmetric (bare store + wrapped reload), not symmetric 4+4.
4. Second implementation: bailed everywhere because CALL implicit-
   defs counted as partner modifications.  Fixed by excluding
   implicit defs (CALL clobbers ≠ value-producing definitions).
5. Two-phase stack tracking added to handle matched reloads nested
   inside other PUSH/POP brackets (e.g. the across-CALL push hl /
   pop de that wraps the call in aes_subBytes).

Shipped (commit `530f385a64ba`):
- Z80LateOpt peephole: bare-store + 4-instr A-preserving reload →
  PUSH/POP rr at stack-balanced point.

Production delta from Stream B:
- AES `09_Oz_prod_like`: −12 B (saves 6 B × 2 fired instances in
  `aes_subBytes` and `aes_sb_inv`).
- cpnos PROM1: −1 B.
- BIOS: −3 B.

## Methodology lessons (cross-cutting)

These are the lessons that paid off — by saving misdirected work
and by enabling the correct work to land.

1. **Catalog before implementing**.  Pattern frequency in real code
   is the prerequisite, not the abstract issue text's yield estimate.
   The original #173 estimated 100-200 B; my early catalog showed
   the actual yield was much lower because most "candidate" stores
   were either part of mismatch patterns or had partner-half
   modifications that block conversion.

2. **The "first abandoned conclusion" is often wrong**.  Twice in
   this session I declared "doesn't fire" / "miscompiles, park it"
   prematurely.  Both times the user pushed back with a redirect
   that turned out to be load-bearing.  The right reflex on
   first failure is **bisect** (or in #173's case, **reinvestigate
   thoroughly**), not park.

3. **CALL clobbers are not value-producing definitions**.  MIR
   represents caller-saved register clobbers as implicit-def
   operands on CALL.  Iterating `MI.operands()` checking
   `MO.isReg() && MO.isDef()` will treat these as "register has
   been modified".  For peepholes that ask "is this register
   safe to restore from stack?", that's a false positive — the
   ABI says the caller has no right to observe a specific value
   across the CALL.  Exclude CALL clobbers / implicit defs from
   the partner-defined check.

4. **Two-phase stack tracking**.  Real-world spill+reload patterns
   are nested inside other stack manipulations (across-CALL
   peephole's push hl/pop de bracketing the call).  Single-pass
   "must balance at reload" check misses these; two-phase scan
   (find matched reload, then continue to next balanced point)
   catches them.

5. **When IR-cost-hook predictions disagree with the oracle, trust
   the oracle**.  Phase B0's "near-zero impact" prediction was a
   plausible-sounding read of pass call sites that turned out wrong.
   The oracle ran in one minute and falsified it immediately.

## Estimate accuracy retrospective

| Phase           | Estimated | Actual    | Accuracy |
|-----------------|-----------|-----------|----------|
| Phase A         | ~30 min   | ~30 min   | ✓        |
| Phase B0        | "Phase B = 2-3 d, near-zero impact" | WRONG    | ✗   |
| Phase B1 fail   | "1-2 wk per hook to fix"      | (NOT NEEDED — B2 bisect fixed in 1 h) | ✗ |
| Phase B2 bisect | ~1 h      | ~1 h      | ✓        |
| #173 first MVP  | "would catch real patterns"  | DIDN'T FIRE | ✗   |
| #173 second pass| "8 firable pairs × 6 B"      | 2 firable, ~6 B each | partial ✓ |

Pattern: estimates that survive contact with the oracle were
catalog-based; estimates that didn't survive were reasoning-from-
abstract-issue-text-based.

## Issues and follow-ups

Filed during Phase 2:
- **ravn/llvm-z80#184** — `getArithmeticInstrCost(i16)=2` miscompile.
  Reproducer + asm diff in the issue.  Needs IR-pass-trace via
  assertion-enabled build to find which IR pass narrows wrong.

Status updated:
- **ravn/llvm-z80#173** — same-MBB path shipped; cross-MBB extension
  yield re-estimated at 0-20 B (down from naive 100-200 B) due to
  mixed-target-register reloads and partner-half modifications in
  the heavily-pressured AES functions.

Local follow-up tasks (TaskList):
- Task #29 — #173 cross-MBB multi-reload extension (deferred).
- Task #30 — #184 root-cause investigation.

## Open levers (ranked by realistic yield)

1. **#184 root-cause** (uncertain; 0-50 B potential): if the
   underlying bug is fixable, i16=2 cost would unlock substantial
   AES `01_baseline_Oz` shrinkage (current gap to SDCC: 380 B).
   Needs IR-pass-trace via asserts build.

2. **#173 cross-MBB multi-reload** (0-20 B realistic): substantial
   ~200 LOC implementation work; only 2 of 4 cataloged slots have
   non-mixed-target reloads; partner-half safety needs path-aware
   liveness analysis.

3. **#175 8-bit ALU memory operand** (~50 B AES): XOR/AND/OR/ADD
   etc. with (HL)/(IX+d) operand.  Substantial backend work.

4. **#172 A-register pinning** (~5% ts on AES): the structural pass
   landed default-off in session 73o; needs liveness-aware selector
   to flip default-on safely.

## Conclusion

Phase 2 ships two distinct optimization mechanisms (TTI hooks +
peephole) with measured production yield across 3 different targets.
The investigation arc is more valuable than the byte yield:
methodology lessons recorded in `tasks/issue173-investigation.md`,
`tasks/issue177-phase-b1-finding.md`, and (this file) will save
future-me from repeating the same false stops.
