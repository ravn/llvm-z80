# Static Frame Size Regression: Root Cause Z80IndexIV

**Date:** 2026-10-09  
**Baseline:** c863c55 (2026-07-01) — autoload PROM 2034 B, `compare_6bytes` 20 B  
**Current:** 2444 B, `compare_6bytes` 66 B  

## Executive Summary

The ~410 B regression from c863c55 to the current compiler is **not** caused by
`+static-frame` being broken. The root cause is `Z80IndexIV` creating IR that
raises register pressure in small loops, forcing the register allocator to spill
values it previously kept in registers.

Static frames are neutral in terms of code size (same result with or without
`+static-frame`). The actual culprit is a loop-IV transformation that turns
simple pointer-increment loops into Base+Index patterns, adding more simultaneously
live values than Z80 has register pairs.

## What Z80IndexIV Does

`Z80IndexIV` locates GEP instructions inside loops with SCEV of the form
`Base + Index` (index fits in 8 bits) and rewrites them as explicit index IVs.
Intended for IX+d indexed addressing: `LD A,(IX+d)` with 8-bit displacement.

## The Regression in compare_6bytes

`compare_6bytes(const byte *a, const byte *b)` — compares 6 bytes at two
pointer locations.

**IR produced by Z80IndexIV** (visible in ROM context but not isolated):
```llvm
%z80-indexiv.iv = phi i8 [0, entry], [%z80-indexiv.iv.next, loop]
%uglygep5 = getelementptr i8, ptr %scevgep4, i16 %zext(iv)  ; a[iv]
%uglygep  = getelementptr i8, ptr %scevgep,  i16 %zext(iv)  ; b[iv]
```

Simultaneously live: `scevgep4` (base a+1), `scevgep` (base b+1), `iv` (8-bit),
`uglygep5` (16-bit), `uglygep` (16-bit) + A for comparison = 5 wide values.
Z80 has BC, DE, HL (3 register pairs) → forced spills.

**Without Z80IndexIV** (c863c55 result, 20 B):
```asm
ld  c,l / ld b,h   ; put pointer a in BC
ld  l,e / ld h,d   ; put pointer b in HL
ld  e,$6           ; counter in E
; loop: ld a,(bc) / cp (hl) / inc hl / inc bc / dec e / jr nz
```
Only 3 simultaneously live wide values (BC=a, HL=b, E=counter) → no spills.

## Why the Optimization Misfires

Z80IndexIV is profitable when:
1. The loop body uses IX+d indexed addressing (`LD A,(IX+d)`)
2. The 8-bit index replaces a wider IV, reducing register pressure

For `compare_6bytes`, the transformation is **not** profitable because:
- The loop uses HL and DE directly (no IX+d)
- Creating an explicit index IV ADDS register pressure vs. pointer increment
- Z80IndexIV has no profitability check for this case

## Cascade Effect (Machine Code Outliner)

The forced RA spills produce IX-indexed stores in multiple functions.
The machine code outliner then extracts these common sequences:
```asm
_OUTLINED_FUNCTION_2:
  ld (ix+-2),e / ld (ix+-1),d / ret
```
Replacing inline sequences with `call $OUTLINED_FUNCTION_x` adds 3-byte CALL
overhead per occurrence. Result: `compare_6bytes` 20 B → 66 B (+46 B).

## Investigation Path (2026-10-09)

Tried fixing in `Z80FrameLowering.cpp`:
1. Profitability check for wide RA-spill slots (`isSpillSlotObjectIndex`) → neutral
2. `hasFPImpl` returning true for size builds → RA still spills regardless
3. `KeepWideOnStack` for Z80 → IX is more expensive than static (3 vs 6 B), 
   makes things worse

All approaches failed because `processFunctionBeforeFrameFinalized` runs AFTER
RA — the spills are already inserted. The fix must prevent RA from creating
spills in the first place, which requires either:

**A. Fix Z80IndexIV profitability**: don't create index IV when the loop
already has an efficient pointer-increment pattern and adding the IV increases
live values above available register pairs.

**B. Fix register pressure in Z80IndexIV result**: detect when the
Base+Index pattern forces spills that wouldn't exist with pointer increment,
and fall back.

**C. Raise Z80 spill cost for wide values**: make the RA see wide register pair
spills as more expensive (reflecting 6 bytes for IX+d access), discouraging
unnecessary spills. This is the cost-model approach (related to issue #38).

## Issue Filed

→ ravn/llvm-z80 issue #TODO (to be created)
