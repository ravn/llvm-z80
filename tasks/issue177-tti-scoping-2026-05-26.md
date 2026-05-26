# #177 (Z80 TargetTransformInfo) — scoping for the next session (2026-05-26)

Scoping pass before committing a session to #177.  Outcome: #177 is **largely
already built**; what remains is mostly *validation and cleanup*, plus one
correctness-held item.  The issue's original framing (session-73p) over-claims
what TTI unlocks — corrected below so the next session doesn't chase a false
dependency.

## State of play (what is ALREADY implemented + wired in)

`Z80TargetTransformInfo.h` exists (header-only) and is registered
(`Z80TargetMachine.cpp:149 getTargetTransformInfo -> Z80TTIImpl`).  Hooks present:

- `getNumberOfRegisters = 3`, `getRegisterBitWidth = 8` — register-pressure model.
- `isLSRCostLess` — register-count-first LSR cost (the issue's hook 6: **done**).
- `getPredictableBranchThreshold = 0` — bias to branch, not select.
- `areInlineCompatible` — inline policy (inlinehint / <=10 insn / single-call).
- `prefersVectorizedAddressing = false`.
- `getArithmeticInstrCost` — Mul + div/rem -> `TCC_Expensive`.
- `getCastInstrCost` — trunc i16->i8 free, zext i8->i16 free, sext i8->i16 = 2.
- `hasDivRemOp`, `isValidAddrSpaceCast`.

Held back (correctness-safe post #184/#185, but **production-negative**): the i16
arithmetic width charge (`i16=2 / i32=4`).  Measured: cpnos PROM1 +9 B (eats the
2 KB cap), autoload +16 B, AES `09_Oz_prod_like` +44 B, BIOS −6 B.  **Keep held.**

## Correction A — TTI is IR-level; it does NOT unlock the MIR/regalloc peepholes

`TargetTransformInfo` is consulted by **IR transform passes**: LSR (`-loop-reduce`),
IndVarSimplify, the inliner, LoopIdiomRecognize/MemcpyOpt, the vectorizer (n/a on
Z80).  It is **not** consulted by the MIR passes:

- **MachineLICM / MachineCSE** are MIR passes that use `TargetInstrInfo` /
  schedule model, **not** TTI.  They were neutralized for Z80 by an unconditional
  `disablePass()` (`Z80TargetMachine.cpp:228-230`, #128) — *not* by a TTI cost.
  This confirms TTI was never their lever.
- **MIR-CSE / MIR-DCE / the regalloc cost model** (the homes the #180 audit
  assigned to ~10 migrate candidates) are MIR/`TargetRegisterInfo`-level.  TTI
  does not reach them.

So the #180 claim *"#177 TTI unlocks ~10 of the 16 migrate candidates"* is
**incorrect**.  #177 can, at most, affect the LSR/IndVars-driven peephole shapes.
The regalloc-cost candidates (#23 HL-save roundtrip, #24 BC ping-pong, etc.) need
a **MIR/TRI cost model** — that is **#38** (the IX/IY cost-model work), not #177.
Update the #180 cross-reference accordingly.

## Correction B — the production workaround state today

The three `-mllvm -disable-*` flags the issue cites are no longer uniform:

| workaround | current state |
|---|---|
| `-disable-machine-licm` | **redundant** — `MachineLICMID` + `EarlyMachineLICMID` disabled globally in the backend (`disablePass`). The Makefile/AES-config flags are no-ops. |
| `-disable-machine-cse` | **redundant** — `MachineCSELegacyID` disabled globally. Makefile flags are no-ops. |
| `-disable-lsr` | **still active and contested.** LSR is IR-level (TTI-governed). `FLAG_RECIPES.md`: disabling LSR is *+333 B on AES* (LSR HELPS u8-heavy code) yet cpnos/autoload/rcbios disable it — the note says "the choice should be re-measured." |

Used in: `cpnos-in-c/Makefile:111-113`, `autoload-in-c/Makefile:178`,
`rcbios-in-c/clang/Makefile:40`, AES `09_Oz_prod_like`.

## RESOLUTION (2026-05-26 finish session) — Tasks 1+2 landed, Task 3 fork surfaced

**Task 1 LANDED** (rc700 branch `session-177-task1-cleanup`, commit `8bfec2b`).
`-disable-machine-licm` / `-disable-machine-cse` removed from `cpnos-in-c/Makefile`.
Re-verified byte-identical the rigorous way (the earlier "byte-identical" was vs a
stale committed bin): built with vs without the two flags **back-to-back**, then
compared the *uncompressed* layer — `payload.bin` bit-for-bit identical, `init.bin`
differs in exactly one byte (a build-timestamp minute digit `'7'`/`'6'`).  The
1422-byte diff on the *compressed* PROM image was a ZX0 cascade from that one
timestamp byte, NOT codegen.  PROM1 line program 2027/2048 B both ways.

**Task 2 COMPLETE** — `-disable-lsr` measured on all three production targets
(A/B, BIOS A/B/A-confirmed):

| target | with `-disable-lsr` | without | Δ removing flag | verdict |
|---|---|---|---|---|
| cpnos PROM1 (compressed) | 2027 B | +2 B | raw payload byte-identical; +2 B is ZX0 artifact | keep (2 KB cap) |
| autoload PROM | 1904 B raw / 1473 comp | 1918 / 1486 | +14 B raw / +13 comp | keep |
| rcbios BIOS | 5919 B | 5929 B | +10 B | keep |

LSR *pessimizes every production target* — opposite of AES (disabling LSR there is
+333 B because AES is tight u8 arithmetic; RC700 firmware is control-flow/IO-heavy).
So `-disable-lsr` is correct on all production builds and stays.  No flag removed
by Task 2.

**Task 3 FINDING — the gate's premise is unmeasured (degenerate AES configs).**
The global `disablePass(EarlyMachineLICM/MachineLICM/MachineCSE)` (#128) overrides
the `-disable-machine-licm/-cse` flags *everywhere*, including the 7 AES-corpus
files that use those flags to toggle LICM/CSE on vs off (`flag_sweep.sh`,
`measure_aes09.sh`, `clang-flag-sweep*.md`, `FLAG_RECIPES.md`, `findings.md`).
Those "LICM-on" configs have silently been measuring LICM-OFF.  Consequence: the
hypothesis behind a per-opt-level gate — *"keep LICM/CSE at -O2 where the speed
gain beats the spill cost"* — **has never actually been measured on Z80**, and is
questionable a priori (a BSS spill+reload across a loop is both bigger AND slower
than recomputing a cheap value on the 3-pair register file, so LICM may hurt
*speed* at -O2 too, not just size).

**Task 3 RESOLVED 2026-05-26 — gate REFUTED, unconditional disable VALIDATED.**
Ran the measurement (`aes256-corpus/task3_licm_ab.sh`): commented out the three
`disablePass` lines, rebuilt clang, A/B'd LICM/CSE on-vs-off at -Oz and -O2 with
all other flags fixed at production values.

| config | aes_text | bin | tstates | verify |
|---|---|---|---|---|
| -Oz LICM/CSE off | 2238 | 2598 | 10,743,671 | PASS |
| -Oz LICM/CSE on  | 2272 (+34) | 2742 (+144) | 10,767,362 (+0.22%) | PASS |
| -O2 LICM/CSE off | 4473 | 5662 | 10,713,500 | PASS |
| -O2 LICM/CSE on  | 4362 (-111) | 5548 | 10,730,766 (+0.16%) | **FAIL** |

Two independent reasons NOT to gate:
1. **-Oz**: LICM/CSE-on is bigger AND slower -> disable is correct (re-confirms #128).
2. **-O2**: LICM/CSE-on is smaller but *slower AND miscompiles*.  Isolated
   (repeated, deterministic): **MachineCSE** is the miscompiler (CSE-on -> FAIL
   regardless of LICM; CSE-off -> PASS regardless of LICM; LICM alone is correct).
   Filed **ravn/llvm-z80#198**.  So the disable is a CORRECTNESS guard at -O2, not
   only a size knob.

Conclusion: there is no opt level where enabling LICM/CSE helps Z80.  The
unconditional `disablePass` STAYS (no gate).  `Z80TargetMachine.cpp` comment
updated to record this with the #198 cross-ref.  Codegen-identical to baseline
(comment-only change; AES prod config re-verified aes_text=2238 PASS).

This was the last open decision in #177.  Net #177 status: TTI built (10 hooks);
Tasks 1 (LICM/CSE flag cleanup, committed) + 2 (LSR validated on all 3 production
targets) + 3 (this) DONE.  Task 4 (#184 i16 width cost) HELD.  Task 5 (speculative
hooks) not pursued.  Spun-off trackers: #198 (CSE -O2 miscompile), #184 (i16 cost),
#38 (MIR/regalloc cost model = the real IX/IY density lever, independent of TTI).

## The actual remaining #177 work (next session) — concrete, mostly non-gated

**Task 1 — remove the redundant LICM/CSE flags (cleanup, low-risk).**
`-disable-machine-licm` / `-disable-machine-cse` are no-ops (passes already off in
the backend).  **MEASURED 2026-05-26:** only `cpnos-in-c/Makefile` carries them
(autoload-in-c:178 and rcbios-in-c/clang:40 have only `-disable-lsr`).  Removing
both from cpnos and rebuilding -> PROM1 payload 2028 -> 1396 B, line program
2028/2048 B = **byte-identical to baseline** (confirmed: the passes don't run
either way).  So the cleanup is a single-file change to cpnos, verified safe.
Deferred from this session only because rc700-gensmedet tracks dirty build
artifacts; land it as a focused single-file commit when that repo is clean.

**Task 2 — re-measure `-disable-lsr` per production target (the real payoff, ~1-2 h).**
Now that `isLSRCostLess` (register-count-first) + `getNumberOfRegisters=3` are in
TTI, test whether TTI-guided LSR is acceptable without the flag:
- For each of cpnos PROM1, autoload PROM, rcbios BIOS: build **with vs without**
  `-disable-lsr`; compare `.text` size + MAME boot.
- Remove the flag where TTI-LSR size <= disabled-LSR size (the #177 win: a
  workaround obviated by the cost model).  cpnos is the prime suspect (2 KB cap,
  flag may be cargo-culted from a pre-TTI era).
- If TTI-LSR regresses, that localizes the missing cost: feed it into Task 4.

**MEASURED 2026-05-26 (cpnos):** removing `-disable-lsr` -> PROM1 line program
2028 -> **2030 B (+2 B, 20->18 free)**, but the **raw payload is identical
(2028 B both)** -- the +2 B is a *compression* artifact (LSR changed code content,
not length; ZX0 compressed 1396 -> 1401).  So LSR is **size-neutral on cpnos's
real code**; the flag's benefit is a marginal 2 B via compressibility.  At the
2 KB hard cap that 2 B is worth keeping, but the flag is NOT the meaningful
workaround the issue implied (contrast AES, where the docs show disabling LSR is
*+333 B* -- LSR genuinely helps u8-heavy code).  Still TODO: measure autoload +
BIOS (not 2 KB-capped, so LSR may matter differently there).  Net so far: the
`-disable-lsr` story is target-specific and small; do not expect a big #177 win
from removing it.

**Task 3 — replace the blunt global LICM/CSE disable with a gated decision (medium).**
The `disablePass` comment (line 225) states the intent: gate on per-function
optsize/minsize so `-O2` speed builds keep LICM/CSE where beneficial while `-Oz`
size builds skip them.  NOTE: since these are MIR passes that don't consult TTI,
the gate likely belongs in `Z80PassConfig` (opt-level / `hasMinSize` check around
the `disablePass` calls), **not** in `Z80TTIImpl`.  Validate across AES 13-config
(several configs exist precisely to measure LICM/CSE on/off).

**Task 4 — #184 i16 arithmetic width cost: keep HELD.**  Production-negative
(+9 B cpnos / +44 B AES).  Do not re-enable without a measured production-positive
case.  See the in-source comment block (Z80TargetTransformInfo.h:101-125).

**Task 5 — speculative hooks: do NOT implement without a measured need.**
The issue proposes `getMemoryOpCost`, `getCFInstrCost`, `isLegalAddImmediate`,
`getInliningThresholdMultiplier`.  Implement one ONLY if Task 2/3 shows a specific
pass making a wrong call that the hook would fix.  Lesson from #184: a *plausible*
cost hook can be net-negative on the production target; cost hooks must be
justified by a measured codegen win, not theory (`feedback_dig_deeper_before_parking`,
`lessons-2026-05-04-structural-fix-failures`).

## Validation methodology (binding for any #177 codegen-affecting change)

Per-production-target size + boot (the cost model affects size directly):
- cpnos PROM1 (2 KB hard cap — `project_rc702_2kb_prom_hard_limit`) + polypascal MAME boot.
- autoload PROM (2 KB cap) + boot banner.
- BIOS size.
- AES 13-config sweep (verifier PASS + size/tstate) — the configs are built to
  isolate LSR / LICM / CSE effects.
- lit suite + `cargo run -- clang`.

## What #177 does NOT do (set expectations)

- Does not retire the MIR/regalloc #180 peepholes (#23/#24/#20/#11...): those need
  the MIR/TRI cost model = **#38**.
- Does not change MachineLICM/MachineCSE behavior beyond the existing global
  disable (they don't read TTI).
- The big density lever for IX/IY remains **#38**, independent of #177.

## Recommended sequence

Task 1 (cleanup) -> Task 2 (the measurable payoff) -> Task 3 (proper LICM/CSE gate)
-> Task 5 only if Task 2/3 surface a concrete missing hook.  Task 4 stays held.
Expected realistic outcome: simpler production build flags (Tasks 1-2) and possibly
an opt-level-sensitive LICM/CSE decision (Task 3) — NOT the "~320 B AES shrink"
the issue projects (that was #128, already realized via the global disable).
