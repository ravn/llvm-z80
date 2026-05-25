# Session 73s — #27 per-pair 16-bit copy cost: measurement drill

**Date:** 2026-05-24
**Issue:** ravn/llvm-z80#27 ("Per-pair 16-bit register copy cost"); last open Cluster A item.
**Outcome:** NEGATIVE result. The per-pair-copy-cost lever is largely exhausted at HEAD. `copyPhysReg` already lowers each copy optimally; the dominant residual copy traffic is *necessary* base-pointer re-materialization, not waste. A prototype peephole was implemented, measured to fire 0× on production, and reverted.

## What #27 proposed

TableGen `CopyCost` is per-register-class, not per-pair, so the allocator can't see that `DE<->HL` (1 B via `EX DE,HL`) is cheaper than `GR16<->GR16` (2 B via 2× `LD r,r'`) or `GR16<->IX/IY` (3 B via PUSH/POP). The ask: make allocation decisions weigh actual per-pair byte cost.

## Findings

### 1. IX/IY copies do not occur in production
IX and IY are **always reserved** (`Z80RegisterInfo.cpp:266`, "stay reserved until #112"). The allocator only uses DE/HL/BC, so the entire IX/IY half of the #27 cost table (3 B PUSH/POP copies) never materializes. That part of #27 is gated on #38/#112 (un-reservation) and is moot until then.

### 2. `copyPhysReg` already lowers each surviving copy optimally
`Z80InstrInfo::copyPhysReg` (lines 237–256) already emits `EX DE,HL` (1 B) for `DE<->HL` copies whenever the source is dead — and it checks `computeRegisterLiveness` when the `KillSrc` flag isn't set, so it captures liveins too. All other GR16 copies use 2× `LD r,r'` (2 B), which is the cheapest option (no EX exists for BC). There is no per-copy lowering left on the table.

### 3. Measured copy population (AES `09_Oz_prod_like`, 2228 B `.text`)
- `ex de,hl`: 13 (already the 1 B form).
- `DE<->HL` via 2× LD: 29 copies (source was live, so EX is illegal — correct).
- `BC<->HL`: ~40 copies (the dominant cost, ~80 B), plus `BC<->DE` ~13.

The BC↔HL traffic dwarfs everything. Inspecting the MIR shows **why**: a table base (`__sfrend_aes_*`) is loaded once into BC/DE and then copied into HL repeatedly — once per `ADD HL,DE`/`ADD HL,BC`/`INC HL` against a different offset, e.g.

```
LD BC,(base)     ; 4 B, once
LD L,C; LD H,B   ; 2 B  -> HL = base
ADD HL,DE        ; HL = base + off0   (destroys HL)
...
LD L,C; LD H,B   ; 2 B  -> HL = base again
LD DE,7; ADD HL,DE
...
```

`ADD HL,rr` destroys HL, so the base must be re-materialized for each use. Holding it in BC/DE and copying to HL (2 B) is **cheaper** than reloading from memory (`LD HL,(base)`, 3 B). This is efficient, not wasteful — SDCC does the same.

### 4. Prototype peephole fired 0× on production
Hypothesis: a spill reload that lands in BC/DE and is then copied into HL, with the pair **dead** afterward, wastes 3 B (`LD BC,(addr)` 4 B ED-form + 2 B copy vs `LD HL,(addr)` 3 B). Implemented as a post-RA peephole (retarget the reload directly into HL when the source pair is dead after the copy), with a `.mir` unit test (which passed).

Instrumented fire-count on AES: **14 pattern matches, 0 fires.** In every matched site the source pair is **live** after the copy (the base-reuse pattern above — confirmed in MIR: the same BC/DE value is copied to HL again later). The "dead reload" shape the peephole targets does not occur in the AES corpus. cpnos PROM1 moved 2029→2028 B (within the ±1–2 B `#187` drift band — no real fire). Per the #180 audit principle (don't keep peepholes that earn no bytes on the test corpus), the peephole and its synthetic test were reverted.

## Conclusion

The per-pair-copy-cost framing of #27 is largely exhausted for the current (IX/IY-reserved, DE/HL/BC-only) allocator:
- The only per-pair *lowering* asymmetry that exists (`EX DE,HL`) is already exploited.
- The dominant copy cost (BC/DE↔HL base re-materialization) is forced by HL being the sole ALU/address pair under 3-pair pressure, and the current sequence is already the cheap option.

The genuinely-remaining win would require **reducing the need** for base re-materialization — i.e., allocation/scheduling that keeps a hot base alive in HL and arranges offset arithmetic around it, or rematerializing the base+offset more cheaply. That is regalloc-level work (the hard Cluster A residual), entangled with #110 (greedy ignores hints) and #115 — not a copy-cost-model tweak and not a peephole.

## Recommendation

- Keep #27 open but **reclassify** from "per-pair copy cost / CopyCost tuning" to "reduce base-pointer re-materialization under 3-pair pressure (regalloc-level; pairs with #110/#115)". The CopyCost knob itself offers no further win while IX/IY are reserved.
- Re-evaluate the IX/IY-copy half of #27 only after #38/#112 un-reserve IX/IY.

## Files touched
None committed (prototype reverted). Drill evidence: this writeup.
