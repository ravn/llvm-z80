# clang-z80 bug: jump-table range check off-by-one (max case → default at -O1+)

**Found:** 2026-07-22, while implementing `printf("%f")` for
`zcc +cpm -compiler=llvmz80` (nanoprintf's conversion switch exposed it).
**Status:** FIXED 2026-07-22 — `Z80LateOptimization.cpp` CP-narrowing peephole
now uses `CP_n (Limit+1)` (guard `Limit ≤ 254`).  Lit + runtime suites green;
new lit `switch-jumptable-max-case-bound.ll`, updated `issue-86-u8-switch-range.ll`
(was pinning the bug: `cp 29` → `cp 30`), runtime fixture `test_255_...`.
Note: `issue-86`'s `_specc` is a real production BIOS function — its case 0x1e
(30) was silently mis-routed to the default block before this fix.

## Symptom

A `switch` statement routes its **highest case value** to the `default` block
at `-O1`/`-O2`/`-Oz` (correct at `-O0` and on host clang). nanoprintf's
`npf_parse_format_spec("%x")` returns 0 (parse failure) on z80 → `printf("%x")`
prints the literal `%x`; `%X` (a lower char value) works.

## Minimal repro (`bugs/switchbug.c`)

```c
typedef struct { unsigned char conv; char adj; } spec_t;
int parse(const char *fmt, spec_t *s){
    const char *cur = fmt;
    int tmp = 0;
    s->conv = 0; s->adj = 32;
    ++cur;
    switch(*cur++){
        case 'i': case 'd': tmp = 1; goto finish;
        case 'o': tmp = 2; goto finish;
        case 'u': tmp = 3; goto finish;
        case 'X': s->adj = 0;
        case 'x': tmp = 6; goto finish;
        finish: s->conv = (unsigned char)tmp; break;
        case 'F': s->adj = 0;
        case 'f': s->conv = 20; break;
        default: return 0;
    }
    return (int)(cur - fmt);
}
```

`parse("%x")` → ret=0, conv=0 (WRONG, should be ret=2, conv=6).
`parse("%X")` → ret=2, conv=6 (correct).

- **-O0**: correct. **-O1/-O2/-Oz**: wrong.
- **host clang (native x86)**: correct.
- **plain `unsigned x > 50`**: correct — so it is NOT a general UGT bug.

## Root cause (from `--target=z80 -S -O2`)

The switch lowers to a dense jump table (min case `'F'`=70, max case `'x'`=120,
51 entries, indices 0..50). The range guard emitted is:

```asm
    ld   a,(hl)      ; a = switch char
    add  a,186       ; normalize: 'F'(70)->0 ... 'x'(120)->50
    cp   50          ; <-- BUG: should be 51
    ld   l,a
    ld   h,0
    jr   nc,.LBB0_10 ; nc == unsigned >= 50  -> index 50 goes to default
```

`jr nc` after `cp 50` tests `index >= 50`, sending index **50** (`'x'`, the
last valid table slot) to the default block. The jump table itself is correct:
it has 51 entries and index 50 → the `'x'` handler (`.LBB0_5`). Only the bound
check is off by one.

The generic switch lowering produces a bound `index UGT Range` with
`Range = Last-First = 50` (i.e. `index > 50` → default), which the Z80 backend
must emit as `cp 51; jr nc` (`index >= 51`). It instead emits `cp 50; jr nc`
(`index >= 50`) — it lowered the unsigned `>` bound as `>=`, dropping the `+1`.

## Scope / impact

Narrow mechanism (jump-table bound only; ordinary comparisons are fine), but it
miscompiles **any** `switch` whose maximum case value maps to the last dense
jump-table index — a common shape in parsers/dispatchers. Silent wrong-answer
at the default optimization levels. Exposed here by nanoprintf (`'x'` is the max
conversion char).

## Fix location (candidate)

The Z80 lowering of the `BR_JT` / jump-table range comparison (the `index UGT
Range` bound). Whichever code turns that unsigned-greater-than-constant bound
into `cp N; jr nc` is using `N = Range` (= max index) instead of `N = Range+1`
(= entry count). Likely in `Z80ISelLowering` (SelectionDAG BR_JT / switch bound)
or the GlobalISel equivalent; confirm which path is active for the Z80 switch
lowering, then correct the constant to the exclusive bound (entry count).

## Verification plan (once fixed)

- `switchbug.c` returns correct at -O1/-O2/-Oz.
- New lit test pinning `cp 51` (entry-count bound) for a max-case switch.
- Runtime fixture: `switch` dispatch over all case chars incl. the maximum.
- Rebuild clang+llc+lld; re-run lit + test-runner suites.
- Re-verify nanoprintf `%x` on z80, then unblock Design B (`printf("%f")`).

## Relationship to printf %f

This is the true blocker for Design B (self-contained nanoprintf `printf`).
Fixing it in the backend is on the project's core mission AND unblocks %f, and
fixes a broad class of switch miscompiles — strictly better than working around
it in nanoprintf.
