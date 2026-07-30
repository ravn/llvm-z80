# Session 2026-06-25 — dcc-corpus three-compiler comparison: root-cause investigation

**Date:** 2026-06-25
**Context:** Follow-up to the 2026-06-24 dcc-corpus oracle session that established
baselines.  This session root-caused the performance gaps between clang and dcc on the
dcc test suite, filed two new known-suboptimal-codegen entries (M5, M6), authored a
firmware-representative microbenchmark set, and SHIPPED one root-cause codegen fix
(16-bit signed compare-against-(-1) sign test — see "CRC fix" below).

---

## Setup: what the dcc repo is

`/Users/ravn/z80/dcc` is a third-party C89 compiler targeting CP/M Z80 (github.com/davidly/dcc).
It compiles via M80/L80 assembler+linker with a hand-written Z80 runtime (`DCCRTL.MAC`).
`dcc/scripts/compare3.sh` drives a three-way benchmark: dcc vs clang vs zsdcc.

Pre-built `.COM` + `.img` files (from the 2026-06-24 session) in
`dcc/build/compare3/` were used for T-state measurement throughout.

---

## T-state comparison: full baseline

Measured with `z88dk-ticks -pc 100 -end 0 -counter 2000000000`:

| Test       | dcc T-states | clang T-states | ratio  | note |
|------------|-------------|----------------|--------|------|
| sieve      | 28.5M       | 33.0M          | 1.16×  | clang slower |
| e          | 31.4M       | 40.1M          | 1.28×  | clang slower |
| nqueens    | 52.9M       | 66.9M          | 1.27×  | clang slower |
| ttt        | 7.0M        | 10.5M          | 1.50×  | clang slower |
| tqsort     | 57.9M       | 102.3M         | 1.77×  | clang slower |
| tbsearch   | 1.16M       | 1.20M          | 1.03×  | clang marginally slower |
| tstring    | 878M        | >2B (3.34B)    | 3.8×   | clang much slower |
| **tsetjmp**| 27.9K       | **23.5K**      | 0.84×  | **clang faster** |
| **tmalloch**| 35.2K      | **21.7K**      | 0.62×  | **clang faster** |
| fact       | 68.9K       | 222.9K         | 3.23×  | **bogus** — dcc wrong output |
| triangle   | 51.3K       | 139.4K         | 2.72×  | **bogus** — dcc wrong output |

### `fact` and `triangle` — invalid comparison

dcc's `printf` does not implement `%lu` or `%ld`; it prints the format specifier
literally ("factorial( lu ) = lu").  Confirmed via `z88dk-ticks -output`.  dcc avoids
all 32-bit decimal conversion (expensive on Z80 — divide-by-10 loop via `__udivsi3`
for each digit).  Clang correctly formats the number.  These "wins" are dcc
correctness bugs, not compiler quality.

### `tstring` — not hung, just slow

Counter fired at 2B T-states.  `z88dk-ticks -trace` background run completed and
showed final PC = **0x0dd8** (`jr z, $dfb` in `_strstr` outer scan loop — checking
`*hay == 0`).  Program was mid-scan through one of 1000 `strstr(ac, "gfe")` calls
(each scans ~4 KB).  Re-run with 4B counter confirmed program finishes at **3.34B
T-states** — not hung.

Three independent slowdowns in `cpm_stdlib.c`:
1. `strrchr` uses IY for `last` pointer — `push/pop iy` pairs waste ~42 T per
   iteration (root-caused in detail below → M6).
2. `strstr` outer loop reloads temporaries from BSS globals per byte
   (`ld de,($1d6c)` etc.) instead of keeping them in registers.
3. `rand()` calls `___mulsi3` (32-bit multiply, ~600 T) for each call vs dcc's
   cheaper PRNG algorithm.

---

## Root cause 1 — M5: scale-1 char-array loops miss pointer strength reduction

### Pattern (sieve/e/nqueens/ttt inner loops)

```c
for (k = start; k <= size; k += prime)
    flags[k] = 0;
```

**Clang inner loop (~90 T per iteration):**
```asm
ld hl, _flags   ; 10 T — reload base address every iteration
add hl, bc      ; 11 T — compute flags[k]
xor a           ;  4 T
ld (hl), a      ;  7 T — store 0
ld l, c         ;  4 T \
ld h, b         ;  4 T /  k→HL
add hl, de      ; 11 T — k += prime
ld c, l         ;  4 T \
ld b, h         ;  4 T /  HL→BC (k updated)
[signed 16-bit comparison, ~20-30 T]
```

**DCC inner loop (~39 T per iteration, HL=ptr, DE=prime, BC=&flags[SIZE]):**
```asm
ld (hl), 0     ; 10 T — store direct immediate
add hl, de     ; 11 T — ptr += prime
ld a, h        ;  4 T — high-byte comparison shortcut
cp b           ;  4 T — if H < B_bound, definitely in range
jp c, L17      ; 10 T — continue (taken most iterations)
```

The key waste: `ld hl, _flags` (10 T) reloads the global base address every iteration.
DCC avoids this by keeping a running pointer in HL and using a pre-loaded bound in BC.

**Hand-crafted pointer-form IR (manually written to test feasibility):**
compiles to ~57 T per iteration — confirming the backend CAN generate the pointer form;
the gap is in getting LSR to make the transformation.

### Why LSR won't fix it

`opt -passes=loop-reduce` on the sieve IR is a no-op — the IR is byte-identical
before and after.

**Root cause:** `isLSRCostLess` orders costs as `NumRegs` first:
```cpp
return std::tie(C1.NumRegs, C1.Insns, ...) < std::tie(C2.NumRegs, ...);
```

| Form | Live registers | NumBaseAdds |
|------|---------------|-------------|
| Integer-IV (current) | BC=k, DE=prime = **2** | 1 (GV+reg = illegal mode) |
| Pointer-IV (desired) | HL=ptr, DE=prime, BC=bound = **3** | 0 |

LSR sees 2 < 3 and keeps the integer form.  It correctly avoids register pressure but
fails to account for the 10-T `ld hl, _flags` reload per iteration — which is not
counted in `NumBaseAdds` (treated as a costless constant load, not an instruction).

`isLegalAddressingMode` already returns `false` for `GV + HasBaseReg` (Z80 can't fold
both into one instruction), but this only adds a `NumBaseAdds` cost which ranks below
`NumRegs` in `isLSRCostLess`.

**AVR oracle:** same `subi/sbci` base-reload pattern on AVR → generic LLVM LSR
cost model issue, not Z80-specific.

**Scale=1 vs scale>1 distinction:**
- Scale>1 (`short flags[]`, `i*size` in qsort): LSR reduces the multiply to
  pointer-increment (classic strength reduction).  tqsort measured +57 B without
  LSR (confirmed).
- Scale=1 (`char flags[]`, `k += prime`): **no multiplication to reduce**.  The
  only "strength" is eliminating the base-address reload — invisible to LSR.

### Fix options

A. **Z80GEPStrengthReduce pass** (Z80-specific, pre-RA): detect `gep global_gv, linear_iv`
   in loops; create a pointer phi.  Targets the exact case LSR misses.

B. **`isLSRCostLess` change**: weigh `NumBaseAdds` more heavily when the base add
   corresponds to an illegal addressing mode.  Risky — affects all loops.

**Filed as M5 in `tasks/known-suboptimal-codegen.md`.**

---

## Root cause 2 — M6: Z80LowerSelect pre-compute forces IY in pointer-scan loops

### Pattern (`strrchr` inner loop)

```c
char *strrchr(const char *s, int c) {
    const char *last = NULL;
    while (*s) { if (*s == (char)c) last = s; s++; }
    return last;
}
```

**Current assembly (from `cpm_stdlib.c` compiled by clang, `-Os`):**
```asm
; HL=s, B=c_char, C=cur_char, IY=last, DE=temp
.loop:
  ...comparison produces A=1 (match) or A=0 (no-match)...
  ld e, l          ; DE = s (optimistic pre-write)
  ld d, h          ;
  jp nz, .cont     ; if match: keep DE=s, jump to .cont
  push iy          ; if no-match: DE = IY (restore old last)
  pop de           ;   38 T wasted
.cont:
  inc hl           ; s++
  ld a, (hl)
  ld c, a
  or a
  push de          ; IY = new last (persist)
  pop iy           ;   38 T wasted
  jr nz, .loop
```

Per iteration wasted on IY moves: ~76 T (2× push/pop pairs).

### Root cause: virtual register physical assignment

`-print-after=virtregrewriter` shows the physical registers assigned:
- `$hl` = s (scan pointer)
- `$b` = c (char to find, loaded from stack via IX)
- `$c` = current char
- **`$iy` = last** ← the problem
- `$de` = select temporary (new_last)

**Why IY?**  Z80LowerSelect pre-computes `DE = s` (the true-value of the select)
BEFORE branching.  This means `old_last` must survive past that write.  With
`$hl`=s, `$b`=c, `$c`=cur_char already consuming HL, BC (both B and C as 8-bit
values), the only remaining 16-bit register for `last` is IY.

The no-match path then does `DE = IY` to restore old_last, and the "persist" step
does `IY = DE` to update IY with the selected value.  Four push/pop pairs per
iteration total.

### Verification: optimal hand-crafted IR

Restructuring the select as a "conditional update" CFG:
- Test condition
- **If match**: update `last = s` (only path that changes it)
- **If no-match**: fall through (last unchanged)

```llvm
loop:
  %last = phi ptr [null, entry], [%last2, cont]
  %match = icmp eq i8 %cur, %c8
  br i1 %match, label %update, label %cont
update:
  br label %cont
cont:
  %last2 = phi ptr [%s, update], [%last, loop]  ; <-- cheap: s or last, not pre-compute
  ...
```

**Assembly generated from this IR (no IY):**
```asm
; BC=s, L=c, DE=last — three registers, no IY
.loop:
  cp l             ;  4 T — compare cur char with c (L = c_char)
  jr nz, .cont     ; 12 T — no match: skip update
  ld e, c          ;  4 T — last = s (low byte)
  ld d, b          ;  4 T — last = s (high byte)
.cont:
  inc bc           ;  6 T — s++
  ld a, (bc)       ;  7 T — load next char
  or a             ;  4 T
  ret z            ;  5 T — if NUL, return (DE = last)
  jr .loop         ; 12 T
; total no-match: 37 T, match: 57 T
```

Inner loop: **37 T** (no-match, common case) vs ~80 T with IY.  2.2× speedup on
strrchr.  Impact on tstring: strrchr contributes ~316M of 3342M total T-states → fix
saves ~170M (~5% total speedup).

### Why the int-promotion angle is secondary

Even with `char c` (i8 parameter, IR already uses `icmp eq i8`), IY is still
allocated for `last`.  The issue is not the comparison width but the select's
pre-compute structure.  The comparison width DOES matter for register pressure
(i16 c adds a 4th live 16-bit value, confirmed by MIR analysis), but the IY
allocation persists either way due to the select lowering shape.

**InstCombine gap (related):** `icmp eq i16 (ashr_exact (shl i16 X, 8), 8), (sext i8 Y)`
→ `icmp eq i8 (trunc X), Y` is a missing rule.  Confirmed: InstCombine actually
WIDENS `sext(trunc(i16 c))` → `shl+ashr` (canonical form), then doesn't narrow the
resulting i16 comparison back to i8.  This would reduce register pressure further
(4 live i16 → 3 live i16 + 1 live i8) but does not by itself fix IY.

### Fix

Z80LowerSelect should detect `G_SELECT cond, new_val, phi_self` (conditional update
pattern — false-value is the incoming phi value) and emit:
```
test cond
if NOT cond: branch to SinkBB (fall-through, DE unchanged = old_last)
TrueBB: result = COPY new_val (ld d,h; ld e,l — 8 T)
SinkBB: result = DE (phi, no copy needed for false path)
```

This avoids pre-computing the true-value in the main block, eliminating IY.

**Filed as M6 in `tasks/known-suboptimal-codegen.md`.**

---

## LSR enabled/disabled: tqsort measurement

| Config | Size | T-states |
|--------|------|----------|
| LSR enabled (default) | 6941 B | 102.3M |
| LSR disabled (`-disable-lsr`) | 6998 B | 100.4M |

+57 B size penalty with LSR disabled (matches the known-suboptimal-codegen note
from 2026-06-24).  T-state difference is 1.9% (not the 38% claimed in the note —
that was from a different benchmark configuration or corpus).  LSR on tqsort saves
code size (by strength-reducing `i*size` multiply) but has near-zero T-state impact
on this specific test.

---

## Summary of actionable findings

| Finding | Impact | Fix path | Effort |
|---------|--------|----------|--------|
| M5: scale-1 GEP no strength reduction | 16–40% slower byte-array loops | Z80GEPStrengthReduce pass | High |
| M6: Z80LowerSelect IY in pointer-scan loops | 2.2× strrchr, 5% tstring | Z80LowerSelect conditional-update pattern | Medium |
| InstCombine: sext+icmp not narrowed | secondary register pressure | Missing InstCombine rule | Low (upstream) |
| dcc `%lu`/`%ld` gap | dcc correctness bug, not a clang issue | N/A | N/A |

Neither M5 nor M6 affects the four production firmware targets (rcbios, cpnos,
autoload, BIOS) — these lack tight inner loops over byte arrays.  Both are
relevant for general CP/M application code.

---

## Files changed this session

- `llvm-z80/tasks/known-suboptimal-codegen.md` — added M5 + M6 entries,
  updated last-modified date
- `llvm-z80/tasks/session-2026-06-25-dcc-corpus-investigation.md` — this file

---

## Firmware-representative microbenchmarks (dcc/tests/fw*.c)

After the dcc↔clang firmware-header compatibility attempt proved too invasive
(too many dcc C89 gaps: multi-line comments in `#define`, `(*p)++`, `%lu`,
function-bodies-in-macros, 8-char CP/M filenames), the better approach was a
NEW corpus of standalone C89 microbenchmarks extracting the firmware's actual
computation shapes — drop-in for `dcc/scripts/compare3.sh` (dcc/clang/zsdcc):

| test     | firmware source pattern                          |
|----------|--------------------------------------------------|
| fwdelay  | nested byte-counter do/while loops (rom.c delay) |
| fwfdc    | struct-as-byte-array fill + bitmask check (fdc_read_result) |
| fwsector | host-buffer tag match + 128 B memcpy (rwoper)    |
| fwbitops | bit-position→byte/mask arith + memset (bg_clear_from) |
| fwcoord  | modular screen coords + 16-bit multiply (xyadd)  |
| fwxlt    | sector-translate table lookup + DPB scan         |
| fwcrc    | CRC-16/CCITT inner bit loop                       |

Results (T-states; clang AFTER the CRC fix below):

| test     | dcc B | cl B | dcc T   | clang T | zsdcc T | clang vs dcc |
|----------|-------|------|---------|---------|---------|--------------|
| fwdelay  | 1920  | 3255 | 309.3M  | 49.3M   | 47.7M   | 6.3× faster  |
| fwfdc    | 2176  | 3371 | 18.1M   | 5.1M    | 9.3M    | 3.5× faster  |
| fwsector | 2560  | 3649 | 180.6M  | 45.6M   | (bf)    | 4.0× faster  |
| fwbitops | 2944  | 3867 | 19.0M   | 9.6M    | (bf)    | 2.0× faster  |
| fwcoord  | 2176  | 3567 | 6.5M    | 3.2M    | 3.8M    | 2.0× faster  |
| fwxlt    | 2176  | 3418 | 54.4M   | 11.9M   | 48.8M   | 4.6× faster  |
| fwcrc    | 2432  | 3426 | 301.8M  | 89.8M   | 105.2M  | 3.4× faster  |

clang wins T-states on every firmware pattern; dcc wins size on all (its DCCRTL
strips unused runtime; clang links the full CP/M stdlib).  `(bf)` = zsdcc build
failed (zcc + the test's idioms).  Two output DIFFs noted for later: fwfdc/zsdcc,
fwbitops/all — correctness mismatches to investigate (likely `%u`/promotion).

dcc `(*p)++` gap filed as ravn/dcc#2; `%lu`/`%ld` as davidly/dcc#33 (+ ravn/dcc#1
for the cannot-open-input filename fix).

---

## CRC fix — 16-bit signed compare-against-(-1) is a sign test (SHIPPED)

**Root cause.** The natural CRC idiom `if (crc & 0x8000) ...` (sign-bit isolate)
is canonicalised by the middle-end to `icmp sgt i16 crc, -1` (≡ `crc s>= 0` ≡
sign bit clear).  The Z80 16-bit signed-compare ISel
(`emitFusedCompareAndBranch`, `Z80InstructionSelector.cpp`) recognised
compare-against-**0** as a one-instruction sign test (`COPY hi; ADD A,A;
JR C/NC`) but had NO case for compare-against-**-1** — it fell through to a full
`LD HL,0xFFFF; SBC HL,rr` 16-bit subtraction plus `SBC A,A; AND 1` boolean
materialisation.  The inner CRC bit loop ballooned to ~20 instructions.  The
8-bit path already handled the `-1` form (the `LC == -1` case); the 16-bit path
did not — a plain omission.

**Fix.** Add the `isConstMinusOne(LHS)` case to the 16-bit signed path, emitting
the same sign test as compare-against-0:
  - `slt -1, X` (from `sgt X,-1`): X >= 0, sign clear → `ADD A,A; JR NC`
  - `sge -1, X` (from `sle X,-1`): X <  0, sign set   → `ADD A,A; JR C`

NOT fragile: it's a self-contained sign-bit emission (COPY hi → A; ADD A,A),
identical in structure to the existing against-0 case — no cross-instruction
carry-flag dependency (an earlier attempt to reuse `ADD HL,HL`'s carry for the
following branch was REVERTED as unsafe: ISel can't guarantee `ADD HL,HL` lands
immediately before the branch with carry intact — scheduling/regalloc decide).

**Effect.**
  - fwcrc (natural idiom, NO source rewrite): clang 203.6M → **89.8M** T-states
    (2.27×), now FASTER than zsdcc (105.2M) and dcc (301.8M).
  - Form A inner loop: 20+ insns → ~6 (sign test + `xor 0x21`/`xor 0x10`
    immediates + `ex de,hl`), matching the hand-optimal zsdcc shape.

**Oracle (all clean).**
  - Lit: CodeGen/Z80 174 pass + 6 XFAIL; new `icmp16-sign-test-minus-one.ll`;
    CostModel/Z80 3/3.
  - Runtime value oracle (`cargo run -- clang`): 878 pass / 0 fail / 0 fatal
    (+6 from new `test_242_crc16_sign_idiom.c`, host-verified CRC=0xFA16).
  - Production BYTE-IDENTICAL: autoload 1945 raw / 1481 ZX0, BIOS 5462,
    cpnos PROM1 payload 1986 raw.  Production-neutral; the win is confined to
    the sign-test pattern that production didn't previously hit suboptimally.

**Files:** `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` (the fix),
`llvm/test/CodeGen/Z80/icmp16-sign-test-minus-one.ll`,
`z80-utils/test-runner/testcases/clang/test_242_crc16_sign_idiom.c`.
