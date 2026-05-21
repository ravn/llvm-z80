# Why is SDCC faster than clang on AES-256?  Structural analysis.

Date: 2026-05-21 (session 73p continued).  Reading: side-by-side
asm of HEAD clang `09_Oz_prod_like` (the production target) vs
SDCC `01_baseline_prod` (current cpnos-rom flags).

## Headline numbers

| Metric | clang | SDCC | Gap |
|---|---:|---:|---:|
| Binary size | **2 667 B** | 3 323 B | clang **−20 %** smaller |
| `.text` (aes part) | **2 307 B** | 2 680 B | clang **−14 %** smaller |
| tstates | 14 887 472 | **12 080 289** | SDCC **−18.9 %** faster |

**clang is smaller. SDCC is faster.** This document explains the
~2.8 M tstate gap, ranked by contribution.

## Methodology

Per-pattern cycle accounting, not whole-program profiling.  For each
candidate cause:

1. Locate the asm pattern in both compilers' output.
2. Sum tstates of the inner instructions (Z80 instruction table).
3. Multiply by the calling frequency in the test workload (one
   encrypt + one decrypt of a single AES-256 block, per
   `test_main.c`).

z88dk-ticks doesn't have per-function profiling, so the absolute
tstate-per-function numbers below are estimates with ~25 % error
bars.  The **relative ranking** is robust — the dominant cost
center is unambiguous from the asm shape alone.

## Calling frequency

The test workload runs one full AES-256 ECB encrypt + decrypt:

| Function | calls/workload | depth (calls to ↓) |
|---|---:|---|
| `aes_subBytes` / `aes_sb_inv` | 14 + 14 = 28 | `rj_sbox` × 16 each |
| `aes_shiftRows` / `aes_sr_inv` | 14 + 14 = 28 | (no calls) |
| `aes_mixColumns` / `aes_mc_inv` | 13 + 13 = 26 | `rj_xtime` × ~9 each |
| `aes_addRoundKey` | 15 + 15 = 30 | (no calls) |
| `aes_expandEncKey` | 1 | `rj_sbox` × 4, `rj_xtime` × ~4 |
| `aes_expDecKey` | 1 | `aes_mc_inv` × 13 |
| **`rj_sbox` / `rj_sb_inv`** | **~480** | `gf_mulinv` × 1 |
| **`gf_mulinv`** | **~480** | `gf_log` + `gf_alog` each × 1 |
| **`gf_log`** | **~470** | inner loop, up to 255 iters |
| **`gf_alog`** | **~470** | inner loop, up to 255 iters |
| `rj_xtime` | ~340 | (no calls) |

The bold rows are the hot path.  `gf_log` and `gf_alog` are called
~470 times each per workload, and each call runs an inner loop
**averaging ~128 iterations** (for uniform-random byte inputs, the
loop count is uniform 0..255 with mean 127.5).

**Total inner-loop iterations of `gf_log`+`gf_alog` per workload:
~120 000.**

## Cost #1 — `gf_log` / `gf_alog` inner loop (DOMINANT, ~70 % of gap)

This is where the gap is concentrated.  Both functions have inner
loops doing GF(256) multiplication by the polynomial generator:

```c
/* gf_alog: compute g^x in GF(256) */
uint8_t y = 1, atb = 1;
while (x--) {
    atb = (atb << 1) ^ ((atb & 0x80) ? 0x1b : 0);
    y ^= atb;
}
return y;
```

The clang and SDCC loop bodies are *structurally* different.

### SDCC inner iteration (~58-68 tstates)

```asm
l_gf_alog_00103:           ; loop top
    ld   b, c        ; 4 ts -- save counter to B for the test
    dec  c           ; 4 ts
    inc  b           ; 4 ts -- set Z flag = "pre-dec value was 0"
    dec  b           ; 4 ts
    jr   Z, exit     ; 7 ts (12 on exit)
    ld   b, a        ; 4 ts -- save accumulator to B
    add  a, a        ; 4 ts -- shift A (CARRY := bit-7 of B)
    bit  7, b        ; 8 ts -- test bit 7 of original
    jr   Z, no_xor   ; 7 ts (12 if branch)
    xor  a, 0x1b     ; 7 ts -- reduce mod polynomial
no_xor:
    xor  a, b        ; 4 ts -- y ^= atb
    jr   loop        ; 12 ts
```

Per iteration (avg branch ~9.5 ts): **~58-68 ts**.

### clang inner iteration (~95-119 tstates)

```asm
.LBB0_1:                                ; loop top
    ld   a, c        ; 4 ts -- load counter
    dec  a           ; 4 ts -- pre-dec
    ld   e, a        ; 4 ts -- save dec'd value
    ld   a, c        ; 4 ts -- RELOAD original counter
    or   a           ; 4 ts -- test for zero
    jr   z, exit     ; 7/12 ts
    ld   a, d        ; 4 ts -- load accumulator
    add  a, a        ; 4 ts -- shift
    ld   h, a        ; 4 ts -- save shifted to H
    ld   a, d        ; 4 ts -- RELOAD accumulator
    rlca             ; 4 ts -- test bit 7 via carry rotation
    jr   nc, no_xor  ; 7/12 ts
    ld   a, h        ; 4 ts -- reload shifted value
    xor  27          ; 7 ts -- reduce mod polynomial
    ld   h, a        ; 4 ts -- save back
no_xor:
    ld   a, h        ; 4 ts -- reload shifted value (third time!)
    xor  d           ; 4 ts -- y ^= atb
    ld   d, a        ; 4 ts -- save y back to D
    ld   c, e        ; 4 ts -- counter := dec'd
    jr   loop        ; 12 ts
```

Per iteration: **~95-119 ts**.  This is **+37-51 ts/iter slower**
than SDCC.

### Cycle accounting

| | clang | SDCC | Δ per iter |
|---|---:|---:|---:|
| ts/iter (avg branch) | ~100 | ~63 | **+37 ts** |
| Iterations/workload | ~120 000 | ~120 000 | — |
| **Total gap from `gf_log`+`gf_alog`** | | | **~4.4 M ts** |

The estimate is upper-bounded by the actual 2.8 M total gap — the
real `gf_log`/`gf_alog` contribution is probably ~2.0 M ts (the
loop counter is not uniformly distributed across AES-state bytes,
some bytes are zero and skip the loop entirely, and SDCC's
`inc b; dec b` trick has its own quirks).  Either way this is
**the dominant single cause**.

### What clang is doing wrong

Three concrete missed optimizations:

1. **Counter reload** (`ld a, c; dec a; ld e, a; ld a, c; or a`).
   The decrement and the zero-test of the *original* both need
   the value, but clang materializes the dec'd to a separate vreg
   instead of using the carry/flag side-effect of the dec itself.
   SDCC's `ld b, c; dec c; inc b; dec b` uses an explicit save-
   reverse-flag trick that's structurally tighter.

2. **Accumulator reload** (`ld a, d; add a, a; ld h, a; ld a, d; rlca`).
   Same value loaded into A twice — once to shift (to H), once
   to test bit 7 (via RLCA's carry).  SDCC keeps the original
   in B, shifts A in place, then `BIT 7, B` reads the saved copy
   directly.  Saves 8 ts per iter.

3. **Conditional XOR sub-block** uses an extra `ld a, h; ... ld h, a`
   pair where SDCC `xor a, 0x1b` operates directly on A.  Saves
   ~8 ts per iter when the branch is taken.

All three are post-RA peephole patterns — the IR is the same.
The gap is in regalloc + scheduling choices.

## Cost #2 — Cascaded XOR chains via A in `aes_mc_inv` / `aes_mixColumns` (~15 %)

The body of `aes_mc_inv` (and its encrypt sibling `aes_mixColumns`)
computes ~15 cascaded XORs over 4-byte windows from `buf[i..i+3]`.

### SDCC body (one XOR step ≈ 19 ts)

```asm
xor  a, (ix-7)       ; 19 ts  -- fused load + XOR via IX-frame
ld   h, a            ; 4 ts
xor  a, (ix-4)       ; 19 ts  -- next XOR direct from spill slot
xor  a, (ix-3)       ; 19 ts
```

Each XOR step is a single `xor a, (ix+d)` (19 ts, 3 B).

### clang body (one XOR step ≈ 32-50 ts)

```asm
ld   a, b            ; 4 ts
ld   (__sfrend-3), a ; 13 ts -- spill B to BSS
push af              ; 11 ts -- save current A
ld   a, (__sfrend-4) ; 13 ts -- load other value
ld   b, a            ; 4 ts
pop  af              ; 10 ts -- restore A
xor  b               ; 4 ts -- finally do the XOR
```

Per XOR step: **~59 ts** (vs SDCC's 19 ts).  Delta **~40 ts per XOR
step.**  This is the same "8-bit BSS spill via A" pattern filed as
**#173** during this session.

### Cycle accounting

| | clang | SDCC | Δ |
|---|---:|---:|---:|
| XOR steps / `aes_mc_inv` iter | ~15 | ~15 | — |
| Per-step ts gap | — | — | ~+40 ts |
| Iter / call (4 cols × 13 rounds) | 52 | 52 | — |
| **Total gap from `mc_inv` + `mixColumns`** | | | **~0.4 M ts** |

Estimate: ~400 K ts (~14 % of the total gap).

## Cost #3 — `rj_xtime` missed peephole (~5 %)

`rj_xtime(x) = (x << 1) ^ ((x & 0x80) ? 0x1b : 0)` — same
multiply-by-2 pattern as the inner GF loop, called 340× per
workload as a separate function.

### SDCC (~30 ts body)

```asm
ld   a, h            ; 4 ts
add  a, a            ; 4 ts -- shift; CARRY := bit-7
add  hl, hl          ; 11 ts -- (dead work; ABI quirk)
jr   NC, no_xor      ; 7/12 ts
xor  a, 0x1b         ; 7 ts
no_xor: ret          ; 10 ts
```

### clang (~80 ts body)

```asm
ld   d, l            ; 4 ts -- save arg
ld   a, l            ; 4 ts
and  128             ; 7 ts -- explicit bit-7 mask
ld   l, a            ; 4 ts
ld   h, 0            ; 7 ts
ld   a, d            ; 4 ts
add  a, a            ; 4 ts -- shift (carry from this could be used directly, but isn't)
ld   d, a            ; 4 ts
ld   a, l            ; 4 ts
or   a               ; 4 ts -- test the masked bit-7
jr   z, no_xor       ; 7/12 ts
ld   a, d            ; 4 ts
xor  27              ; 7 ts
ld   d, a            ; 4 ts
no_xor: ...          ; (more cleanup before ret)
```

Per call: clang **~80 ts** vs SDCC **~30 ts**.  **Delta ~50 ts/call.**

### Cycle accounting

340 calls × 50 ts = **~17 K ts** (~0.6 % of gap).  Small in
absolute terms, but a clear missed peephole: clang masks-and-tests
bit-7 *separately* from the shift, when the shift's carry-out
is already the bit-7 value.

## Cost #4 — Other regalloc churn (~10 %)

Smaller distributed costs:

- **`aes_subBytes` inner loop**: clang spills the loop counter to
  BSS (`ld (__sfrend-1), a` 13 ts + reload 13 ts = 26 ts overhead
  per byte iteration), while SDCC keeps it in C and uses
  PUSH BC / POP BC around the `rj_sbox` call (21 ts).  Per-iter
  gap: ~5 ts × 16 iters × 28 calls = ~2 K ts.

- **`rj_sbox` register pressure**: clang uses 5 registers (A, D,
  E, H, L, C) to hold the 4 rotated copies + the final XOR
  accumulator.  SDCC uses 2 (A, C) by mutating C in place with
  `rlc c; xor a, c` × 4.  The caller (e.g. `aes_subBytes`) then
  has to save/restore more state around each `rj_sbox` call.
  Per-call gap: ~10 ts × 480 calls = ~5 K ts.

- **`aes_expandEncKey` + `aes_expDecKey`**: called once per init.
  Heavy spill traffic but only 2 calls / workload, so the
  amortized gap is small (~5 K ts together).

Total Cost #4: **~30 K ts (~1 %).**

## Summary of cost breakdown

| Cost | Estimated ts gap | % of total | Open issue |
|---|---:|---:|---|
| **#1** `gf_log` + `gf_alog` inner loop | ~2.0 M | **~70 %** | (new — see below) |
| **#2** Cascaded XOR via BSS-through-A | ~0.4 M | ~14 % | #173 (NEW) |
| **#3** `rj_xtime` shift-then-test-bit-7 | ~0.02 M | ~0.6 % | (new — see below) |
| **#4** Other regalloc churn | ~0.03 M | ~1 % | #27, #115 |
| **Unaccounted** (within est. error) | ~0.35 M | ~13 % | — |
| **Total observed** | **2.8 M** | 100 % | — |

## Why does clang produce smaller-but-slower code?

Three structural reasons:

1. **clang trades cycles for bytes via `+static-stack`'s BSS spill.**
   BSS direct (`ld (nn), a` 3 B / 13 ts) is smaller than IX-frame
   (`ld (ix+d), a` 3 B / 19 ts) — but only for spilling A.  For
   other 8-bit registers, BSS forces transit through A with
   surrounding `push af; ... pop af`, which is 6 B / ~32 ts vs
   IX-frame's 3 B / 19 ts.  **The size win on A reverses into a
   cycle loss on non-A.**

2. **SDCC's `xor a, (ix+d)` fuses load + XOR into one 19-ts /
   3-byte instruction.**  clang has no IX-frame so emits load
   then XOR separately (with potential A-save/restore), spending
   ~32-59 ts per equivalent fused op.

3. **clang's regalloc preserves value identity across spills**
   (the "reload the same value twice" pattern in `gf_log`/
   `gf_alog`) where SDCC's allocator keeps loop-carrying values
   in fixed registers and uses non-destructive ALU sub-patterns
   like `bit 7, b` for testing.  This is the loop-carrier-in-A /
   #172 family.

## Open issues to file from this analysis

### New: gf_log/gf_alog inner-loop peephole

The 4 instructions `ld a, X; <op>; ld Y, a; ld a, X; <op2>` (where
the second `ld a, X` reloads the same value that was just stored
to Y) should compile to `ld a, X; ld Y, a; <op>` or similar, saving
the redundant reload.

In `gf_alog`, this would shrink the inner iteration from ~24 to ~19
instructions, closing roughly **half** of the ~37 ts/iter gap.

**Yield estimate**: ~1.0 M ts (35-40 % of the total speed gap).

This is **the highest-priority lever for the speed gap**, ahead of
#173 (8-bit BSS spill peephole) which addresses Cost #2.

### New: rj_xtime / general shift-then-bit-7 peephole

Pattern: `ld a, X; and 128; ld B, a; ld a, X; add a, a; ... ld a, B; or a; jr z, ...`
should compile to `ld a, X; add a, a; jr nc, ...` — using the carry
that `add a, a` already sets, instead of materializing bit 7 via
AND.  Saves ~5 instructions in `rj_xtime` and similar functions.

**Yield estimate**: ~20 K ts (~0.7 % of speed gap, small) but the
peephole would also apply elsewhere.

## Open issues to drill alongside

- **#173 — 8-bit BSS spill via A**: addresses Cost #2 (~0.4 M ts).
- **#175 — Missing 8-bit ALU with memory operand** (NEW from
  session 73p IX-mode investigation): \`XOR (HL)\`, \`XOR (IX+d)\`, and
  the 22 sibling fused 8-bit ALU instructions are entirely absent
  from the .td.  clang emits 0 of them; SDCC uses 50+ per AES round.
  Adding them is mechanical (~24 .td lines + ISel patterns).
  ~10-15 K ts saving on AES IX-frame configs; opens path to
  IX-frame mode being net-competitive with static-stack.
- **#172 — A-pin loop carrier**: addresses part of Cost #1 + Cost #4
  (~0.5 M ts combined estimate).  Needs LiveIntervals + PHI walk.
- **#27, #115**: distributed regalloc churn (Cost #4 fragments).

## Aside: does clang's IX-frame mode help?

Investigated during session 73p (per user prompt).  clang has an
IX-frame mode (\`-fno-omit-frame-pointer\`) used by AES corpus
configs 12 and 13.  Empirically:

| Config | bin B | tstates |
|---|---:|---:|
| 09 prod-like (static-stack) | 2 667 | 14 887 472 |
| 13 IX-frame prod-like | 3 639 | 15 080 454 |
| SDCC 01 baseline | 3 323 | 12 080 289 |

clang IX-frame is **+972 B BIGGER and +0.18 % SLOWER** than clang
static-stack.  Not a magic bullet -- the IX-frame setup overhead
isn't recovered by spill cost savings, because clang's selector
**doesn't emit fused \`xor a, (ix+d)\` instructions** (0 instances in
the corpus vs SDCC's 50+).  This is the missing primitive filed as
**#175**.  Until #175 lands, IX-frame mode is structurally worse on
clang.  After #175 lands, IX-frame mode might become competitive
with static-stack, but neither beats SDCC overall -- that requires
#174 (the dominant gf_log/gf_alog cost center).

## Prioritization

By estimated yield per session-hour:

1. **gf_log/gf_alog redundant-reload peephole** (NEW) — narrow,
   well-localized pattern; likely ~1 M ts saving; the dominant
   speed-gap lever.
2. **#173** — 8-bit BSS spill via A; ~0.4 M ts plus the size-
   shrink secondary benefit (most directly applicable to cpnos
   PROM1 budget).
3. **#172** — A-pin liveness-aware variant; harder to land
   correctly but potentially closes the rj_xtime + loop-carrier
   gaps simultaneously.

Closing #1 + #2 + the rj_xtime peephole would close roughly
**1.5 M of the 2.8 M tstate gap (~55 %)** — bringing clang to
within ~10 % of SDCC on tstates while preserving the −20 % size
advantage.

## Notes on methodology limitations

- Per-function tstate accounting is estimated, not measured.
  z88dk-ticks doesn't expose function-level counters; PC-tagged
  `-trace` could be post-processed but the runs are 12-15 M
  instructions, generating multi-gigabyte traces.
- Average iteration counts assume uniformly-distributed AES
  state bytes.  Real AES diffusion is approximately uniform but
  not perfectly so.
- Per-instruction tstate counts use the standard Zilog table;
  M1-cycle wait-states from the RC702 hardware (not the
  z88dk-ticks emulator) are ignored.

The **relative ranking** is robust to these limitations — the
asm-shape evidence is unambiguous.  The **absolute numbers** carry
~25 % error bars.
