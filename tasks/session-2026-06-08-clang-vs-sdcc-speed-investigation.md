# Clang AES K&R speed regression vs SDCC — investigation findings

**Status:** investigation in progress (started 2026-06-08, after icmp-narrow v2 merged).
**Trigger:** user "investigate why clang is slower than sdcc".
**Headline:** clang `09_Oz_prod_like` is 2581 B (−22 % vs SDCC 3323 B) and 18.21 M tstates (+51 % SLOWER than SDCC 12.08 M).  The speed gap was −11 % FASTER pre-revert (per session 73p Phase 1, 2026-05-21).  Today's gap is 7.5 M tstates.

## TL;DR

The +51 % speed gap on AES K&R is caused by a **multi-pass interaction in the middle-end** that fires on AVR but not on Z80:

1. **InstCombine on Z80** eliminates the `and i16 %atb, 255` mask in `gf_log`'s outside-user icmp as redundant; **InstCombine on AVR keeps it.**
2. **AggressiveInstCombine's Phase 2 (#163/#164 and-mask synthetic trunc root)** needs that `and 255` mask to recognize the narrowness signal; AVR has it, Z80 doesn't.  Phase 2 fires on AVR → phi narrows to i8 → speed wins.  Phase 2 misses on Z80 → phi stays at i16 → +51 % slower than SDCC.

The icmp-narrow sound gate (v1+v2 merged this session) doesn't help because the gate runs on the *post-InstCombine* IR, where the narrowness signal has already been removed.

## What we expected to find (and was partly wrong)

My first verdict was that the +51 % gap was caused by my v1 icmp-narrow gate omitting the and-mask outside-user path.  Implementing v2 disproved that: the and-mask path lands correctly, lit + matrix pass, but the AES sweep is byte-identical to v1 (and 2866 B post-revert → 2581 B v1/v2 is from other sound-gate effects, not from and-mask).

Second verdict was that my sound gate rejects gf_log because KnownBits cannot bound a cyclic phi (`atb` is an i16 phi whose only invariant is the source-level uint8_t domain).  That's also true — but it's the consequence, not the cause.  The deeper cause is that the IR shape reaching AggressiveInstCombine on Z80 doesn't even *trigger* the narrowing attempt.

## Pin-pointing the divergence

Method: `clang -mllvm -print-after-all -S -emit-llvm -Os` on `/tmp/gflog_kr.c` for both targets, diffing pass-by-pass.

Common trajectory:
- SROAPass: `atb` is i8 phi (good).
- First InstCombinePass: widens to i16 phi (both targets — int-promotion artefact).
- Many passes carry it as i16 (both targets identical).

Divergence at the FINAL InstCombinePass before AggressiveInstCombinePass:

**Z80 IR:**
```
%4 = phi i16 [ 1, %1 ], [ %17, %14 ]
%5 = and i16 %0, 255
%6 = icmp eq i16 %4, %5
...
%17 = xor i16 %4, %16          ; xor uses raw phi
```

**AVR IR:**
```
%4 = phi i16 [ 1, %1 ], [ %16, %8 ]
%5 = and i16 %4, 255           ; ← MARKER PRESERVED
%6 = and i16 %0, 255
%7 = icmp eq i16 %5, %6
...
%16 = xor i16 %5, %15          ; xor uses MASKED phi
```

AVR keeps the symmetric `and 255` masks on both icmp operands and on the xor's phi-side input.  Z80 simplifies them away, leaving the raw `%4` phi to be compared and xor'd directly.

After this InstCombine, the IR feeds AggressiveInstCombine:
- AVR: Phase 2 (#163/#164 synthetic trunc root) sees `(and i16 %4, 255)` with mask = 2^8 − 1.  Fires the synthetic trunc, walks the chain, narrows everything.  Final IR has `phi i8`.
- Z80: Phase 2 has nothing to trigger on (the `and 255` is gone).  Skips.  IR stays i16.

## Which pass removes the mask on Z80 (FOUND)

**`CorrelatedValuePropagationPass` (CVP).**  Per-pass trace shows the phi mask survives Z80's pipeline through `JumpThreadingPass` and is **gone after `CorrelatedValuePropagationPass`**.  AVR's CVP run on the equivalent stage **preserves** the mask.

But the divergence isn't CVP itself — it's the IR shape CVP receives:

- **Z80** IR entering CVP has the `if (z & 0x80) atb ^= 0x1b;` lowered to a **branch** (an if-then with a phi merge).
- **AVR** IR entering CVP has the same source lowered to a **select** (`%14 = select i1 %12, i8 %10, i8 %13`).

CVP uses LazyValueInfo (LVI), which can do **loop-carried range analysis on phis via per-edge ranges**.  On the branch form, LVI proves `%atb ≤ 255` via fixed-point iteration over the cycle (the branch gives it the per-edge range info it needs), then CVP drops the now-redundant `and i16 %4, 255` as a "useless mask".  On the select form, LVI doesn't get the same per-edge ranges and the simplification doesn't trigger.

### Confirmation experiment

Took AVR's pre-CVP IR (select form), rewrote the triple to `z80`, stripped the AVR-specific bits, and ran `opt -passes=correlated-propagation` with the **Z80 build of opt**:

- Input: has `%5 = and i16 %4, 255` and select form.
- Output: **PRESERVES the mask**.

So Z80's CVP on select-form input behaves like AVR's CVP.  It's the IR shape that determines CVP's behavior, not the triple.

### What drives the select-vs-branch divergence

`Z80TTIImpl::getPredictableBranchThreshold` returns `BranchProbability(0, 1)` (= 0 %), which tells SimplifyCFG that every branch on Z80 is "predictable" — so the cost model prefers branches over selects.  AVR doesn't override this hook and gets the LLVM default (~99 %), so SimplifyCFG prefers selects.  Set in `Z80TargetTransformInfo.cpp:62-64`, originally from zlfn's initial Z80 backend commit (`31997a6`, 2026-03-12).

## Why the original (unsound) #160/#165 worked despite this

`gf_log 153 -> 28 B` from the pre-revert era was achieved by the **icmp-narrow outside-user gate**, not Phase 2.  That gate's check was only `KnownBits(Other) <= NarrowBits` — it didn't need a graph-side mask marker at all.  So even after CVP had stripped the mask, the gate fired on `icmp eq (raw phi), (and arg, 255)` because the AND-on-arg side was provably narrow.

The sound v1+v2 (this morning's merges) adds the missing `KnownBits(GraphValue) <= NarrowBits` check.  That check requires the phi to be provably narrow, but KnownBits can't see through the cyclic phi (CVP/LVI's range analysis could, but TruncInstCombine doesn't use LVI).  So the sound gate correctly bails.

In other words: the original wins came from an unsound shortcut.  The sound version reveals that **Z80 never had the structural conditions for Phase 2 to fire on `gf_log`** — the marker CVP strips is precisely what Phase 2 needs.  AVR has it; Z80 doesn't.

## Fix landscape (now concrete)

Option | What | Cost | Risk
:-- | :-- | :-- | :--
1. Accept regression | Ship as-is | None | None
2. TruncInstCombine Phase 2 uses LVI | Don't pattern-match `(and X, MASK)` — query LVI for `getConstantRangeAtUse` and inject the synthetic trunc when the range fits in a legal narrow type | Medium (touch one pass, need lit + AES corpus) | Low–medium (LVI is generic, no target-specific risk)
3. Change Z80 `getPredictableBranchThreshold` | Set to e.g. `BranchProbability(99, 100)` (the default) so SimplifyCFG prefers selects | Tiny | **High** — affects every branch decision in the backend, could regress code size or speed elsewhere (the hook was set deliberately to 0 from day 1)
4. Frontend `!range` metadata on uint8_t-sourced values | Clang emits `!range !{i16 0, i16 256}` on uint8_t loads/parameters | Medium-large | Low (additive metadata)
5. Cyclic-phi KnownBits | Strengthen KnownBits in middle-end to handle cyclic phis | Large | Low (additive)

**Recommendation.** (2) is the cleanest pure-fix.  It generalizes Phase 2 from "match this pattern" to "narrow whenever the value's range proves narrowness", which subsumes the AES K&R shape and probably more.  Could be a future llvm-z80 patch and an upstream RFC at the same time.  (1) is the reasonable default if the user wants to focus on finishing firmware components.

## Option (2) was attempted and didn't work

Same-session continuation 2026-06-08 (user direction: "2"):

- Branch `phase4-phi-via-lvi`: wired `LazyValueInfo` through `AggressiveInstCombinePass::run → runImpl → TruncInstCombine` as an additional analysis.  Extended `canNarrowIcmpThroughGraph` with a KnownBits-then-LVI fallback: when KnownBits is conservative (cyclic phi etc.), query `LVI->getConstantRangeAtUse(...)` and admit if the range fits in the narrow width.
- Built clean.  Lit suite still 190 PASS + 4 XFAIL + 1 pre-existing FAIL.
- **AES corpus unchanged**: `09_Oz_prod_like` still 2581 B / 18.21 M tstates (byte-identical to main).
- Instrumented the LVI query with `errs() << ...`: on the post-CVP `gf_log` IR, LVI returns **full-set** for the `%4` phi.  The narrowness LVI proved during CVP's earlier run depended on the `(and i16 %4, 255)` mask being present in the IR; once CVP itself stripped that mask, LVI cannot recover the bound from the loop structure alone.  LVI's loop-carried per-edge analysis needs a starting constraint to propagate.

**Conclusion for option (2)**: the approach is sound and the wiring works, but the analysis doesn't have the power we need on the post-CVP IR shape.  This is a CONSEQUENCE of CVP's own optimization that stripped the marker — the narrowness signal is genuinely lost.

Branch discarded.  No commit.  Rough sketch preserved in `git reflog` and this writeup if anyone wants to revive it for a related shape (e.g. range-via-conditional inside a branch).

## What option (2) showed about the fix landscape

- Option (3) — change `getPredictableBranchThreshold` — would prevent the divergence by keeping selects, which keep the mask, which keeps the marker.  Still high-risk for other reasons.
- Option (4) — frontend `!range` metadata — would inject a narrowness signal that survives CVP.  Most robust.
- Option (5) — stronger middle-end phi analysis — could prove narrowness from the loop body's xor-with-i8-zext structure even without the mask, but this is research-grade work.
- A new candidate option (6): **a target/InstCombine fold that AVOIDS stripping the constraint when no downstream pass can recover it** — e.g., teach the CVP-style fold to check whether AggressiveInstCombine could use the mask.  Architectural concern: CVP would need to know about AggressiveInstCombine's needs, breaking abstraction.  Unattractive.

**Net.** The clean fixes all need either frontend cooperation (4) or middle-end research (5).  The local fixes (2, 3, 6) each have a structural reason they don't fully work.  Defaulting to option (1) — accept the AES K&R regression — is the honest reading.

## Implications for the icmp-narrow sound gate

The v1 + v2 sound gate (this morning's merges, ravn/llvm-z80 main `0dcf93b`) is sound and useful for the SHAPES it sees.  It just doesn't see the AES K&R shape, because the narrowness signal (`and 255`) has already been stripped by Z80's InstCombine.

So my earlier diagnosis ("KnownBits can't prove the cyclic phi narrow") was correct as far as it went, but the *operative* cause is that there's no upstream signal feeding the analysis in the first place.

The unsound original #160/#165 narrowed AES because — before Phase 2 was added — the icmp-narrow gate operated on the post-InstCombine IR where the `and 255` had been dropped, and the gate's checks (only on Other, not on GraphValue) happened to admit it.  Once we add the sound graph-side check, KnownBits on the (now mask-free) phi correctly says "high bits unknown", and the narrowing is correctly rejected.

## Options going forward

In order of investment:

1. **Accept the AES regression.** Clang `09_Oz_prod_like` 2581 B / 18.21 M tstates is the current shipping number; production is byte-identical to main.  AES is one workload, not on the firmware critical path; finishing the four production components is the project goal, not winning AES.

2. **Identify the offending InstCombine fold and either disable it on Z80 or strengthen Phase 2 to see through it.**  Cheap investigation (one focused diff against pristine LLVM should reveal it).  If the fix is target-local, it's a Z80-backend change; if it's a generic InstCombine improvement, it's upstream-eligible.

3. **Backend-side narrowing at isel.**  Z80InstructionSelector recognizes the wide-phi-with-narrow-trunc pattern and lowers as i8 throughout the loop.  Larger scope; needs its own soundness story (target-specific assumption that loop-carried i16 phis with i8 truncs are i8-domain — debatable, needs runtime witnesses).

4. **Frontend `!range` metadata** on uint8_t-sourced phis.  Cleanest architecturally; cross-target benefit (would help AVR + MSP430 + WebAssembly + … too).  Largest scope.

5. **Cyclic-phi KnownBits in middle-end.**  Recursive fixed-point analysis that proves narrowness on shapes like AES gf_log.  Largest scope; upstream-eligible as a generic improvement.

## What's been saved

- ravn/llvm-z80 main `0dcf93b` (commit `c4f52eb17a76` + merge): icmp-narrow v2 with and-mask outside-user path.  Sound and correct, AES-byte-identical to v1.
- This session writeup (`tasks/session-2026-06-08-clang-vs-sdcc-speed-investigation.md`).
- Pass-by-pass logs at `/tmp/z80_passes.log` and `/tmp/avr_passes.log` — NOT committed (regenerable from the gflog_kr.c source if needed).

## Reproduction

```bash
cat > /tmp/gflog_kr.c <<'EOF'
#include <stdint.h>
uint8_t gf_log(x) uint8_t x;
{
    uint8_t atb = 1, i = 0, z;
    do {
        if (atb == x) break;
        z = atb; atb <<= 1; if (z & 0x80) atb ^= 0x1b; atb ^= z;
    } while (++i > 0);
    return i;
}
EOF
CLANG=/Users/ravn/z80/llvm-z80/build-macos/bin/clang
$CLANG --target=z80 -Os -S -emit-llvm -Wno-deprecated-non-prototype -o - /tmp/gflog_kr.c | grep -E 'phi|icmp.*i16|and i16'
$CLANG --target=avr -mmcu=atmega328p -Os -S -emit-llvm -Wno-deprecated-non-prototype -o - /tmp/gflog_kr.c | grep -E 'phi|icmp.*i8|and i8'
```

Z80 output: i16 phi, i16 icmps, no `and i16 %4, 255` mask.
AVR output: i8 phi, i8 icmps.

## Open question for the next session

Run `opt -S -passes=instcombine` on the IR right BEFORE the final InstCombine, with `-mtriple=z80` and with `-mtriple=avr`, and diff the outputs.  That should isolate the exact transform that strips the mask on Z80.  Then the fix-design question becomes concrete: "is that fold soundness-justified on Z80, or is it a missed opportunity we can fix in InstCombine itself?"
