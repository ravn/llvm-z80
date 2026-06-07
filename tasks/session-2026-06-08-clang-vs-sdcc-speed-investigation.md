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

## Which InstCombine fold removes the mask on Z80

Not yet conclusively identified.  The plausible candidates:

- `icmp eq (and X, MASK), (and Y, MASK) → icmp eq X, Y` when MASK doesn't affect the comparison's truth value.  Fires when KnownBits proves the unmasked compare is equivalent.
- `(and X, MASK)` drop when KnownBits(X) ≤ MASK.  Fires on values like `%4` if KnownBits decides it's already narrow.
- A TTI-driven fold that prefers wider arithmetic on targets without a strong i8 preference.

Z80 vs AVR TTI surface that *might* be relevant:
- Both `n8:16` in data layout (8 and 16 both legal integers).
- Both `getRegisterBitWidth = 8` (Z80) / similar on AVR.
- `isZExtFree(i8, i16)`: Z80 returns `false` (explicit override at `Z80ISelLowering.cpp:221-223`); AVR inherits the base-class `false`.  **Equal.**
- Z80 has custom `isLSRCostLess`, `isLegalAddImmediate=|Imm|≤3`, `Mul=Expensive`, `getPredictableBranchThreshold=0`.  AVR has different TTI shape (need to confirm).

A direct AVR-side investigation could:
- Run `opt -passes=instcombine -S` on the same i16 IR with `-mtriple=avr` vs `-mtriple=z80` and see which fold differs.  Cheap to do; gives an exact answer.

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
