# llvm-z80 codegen-fix plan

Working document for systematically improving the Z80 backend.  Goal:
land upstream-ready patches with reproducers + lit tests + measured
size/speed deltas.  46 open issues (as of 2026-05-01) categorized,
clustered, and mapped to LLVM passes + Z80 idioms.

## Strategy

Two pillars:

1. **Tightness** — bytes saved per call site × occurrences in real code.
   Default to `-Oz`-friendly emission everywhere.

2. **Speed** — T-states for hot paths.  CONOUT (rcbios/cpnos) is hottest:
   per-character serial output, called from CCP/BDOS/applications;
   measured against ZX Spectrum's `RST 0x10` 38-T-state baseline.

A pattern qualifies for upstream if EITHER axis improves substantially
without regressing the other.  Where they conflict, gate on `-Oz` vs
`-O2`/`-O3`.

## Z80 idiom corpus (canonical sequences)

References: ZX Spectrum demoscene (Bonk/Lifeforce, Adam Klotblixt's
"Cracking the Code", Patrik Rak's compilers), SDCC's z80 peeprules,
sccz80/z88dk patterns, Hisoft Pascal-80 codegen.  Each idiom below
either directly maps to one of the open issues or is the building
block for a fix.

### Block moves (LDIR / LDDR / LDI / LDD)

```
; memset-style fill of N bytes with constant byte:
ld   hl, dst              ; 3 B
ld   (hl), val            ; 2 B
ld   de, dst+1            ; 3 B
ld   bc, N-1              ; 3 B
ld   ldir                 ; 2 B
                          ; total 13 B,  ~21 T per byte
```

```
; pattern fill (every byte is a copy of previous):
ld   hl, dst              ; 3 B
ld   (hl), low(pattern)   ; 2 B
inc  hl                   ; 1 B
ld   (hl), hi(pattern)    ; 2 B
dec  hl                   ; 1 B
ld   de, dst+2            ; 3 B
ld   bc, 2*N - 2          ; 3 B
ldir                      ; 2 B
                          ; total 17 B  -- the seed-and-LDIR for #88
```

```
; LDDR variant: fill BACKWARD (HL/DE pre-decremented), useful when
; copy direction must avoid overwriting source on overlap:
ld   hl, src+N-1          ; LDDR copies (HL) → (DE), HL--, DE--, BC--
ld   de, dst+N-1
ld   bc, N
lddr                      ; covers #64
```

### Counted loops (DJNZ)

```
; for (b = N; b; --b) { body }
ld   b, N         ; 2 B
loop:
... body ...
djnz loop         ; 2 B  (8/13 T)  — branch + dec in one
```

vs current `dec a; ld r, a; or a; jr nz` (~6 B per iter overhead).
**DJNZ is THE Z80 loop primitive.**  B is reserved by convention; if
register pressure forces B to a value, save+restore around the loop.
Issues #7, #77.

### 8-bit comparison vs 16-bit pessimization

```
; u8 cmp:  if (a < N) ...
cp   N             ; 2 B   ✓
jr   c, ...        ; 2 B
                  ; total 4 B
```

vs widening to 16-bit for switch range checks (#86):
```
; u16 cmp on u8 value (BAD):
ld   l, e
ld   h, $0
ld   bc, N
or   a
sbc  hl, bc
jr   c, ...        ; ~10 B
```

**8-bit values stay 8-bit until forced wider.**

### Known-zero / known-value tracking (#60, #18)

```
xor a              ; 1 B  (A=0, flags set)
                  ; vs ld a, 0   (2 B, no flag side-effect)
```

After `xor a`, `out (n), a`, `ld (nn), a`, `inc/dec hl`, `push/pop` —
**A still holds 0**.  Compiler should track this as a known-value
constraint and drop redundant `ld a, 0` (this session: caught a
hand-asm instance in `isr_crt`; compiler can do the same via
known-bits across MBBs).

### Tail calls (CALL X; RET → JP X) (#75)

```
call X         ; 3 B
ret            ; 1 B  → 4 B
                ; vs:
jp   X         ; 3 B  → save 1 B per site
```

LLVM's existing tail-call peephole only fires when `call` and `ret`
are in the same MBB.  Cross-block (early-return-fall-through) misses.

### Direct-address loads (`ld bc,(nn)`, `ld de,(nn)`) (#80)

Z80 has direct loads only for `HL` (and BC/DE/SP via `ED` prefix):

```
ld   hl, (nn)      ; 3 B   (most common)
ld   bc, (nn)      ; 4 B   (ED 4B nn nn)
ld   de, (nn)      ; 4 B   (ED 5B nn nn)
ld   sp, (nn)      ; 4 B   (ED 7B nn nn)
```

Currently clang emits `ld hl,(nn); ld de,hl` (4 B) instead of
`ld de,(nn)` (4 B).  Same length but loses HL clobber!  At `-Oz`,
direct-form costs the same and frees a register.

### Mask-from-flag idiom (#79)

```
; (x != y) ? 0xFF : 0   →
sub  a, e          ; A = x - y
add  a, $ff        ; carry := (x != y)
sbc  a, a          ; A = -carry = 0xFF if x!=y else 0
                  ; total 4 B
```

vs current 7-instruction mask-build chain.

### `INC (HL)` / `DEC (HL)` for byte counters in memory

```
ld   hl, counter   ; 3 B
inc  (hl)          ; 1 B  (sets Z flag) — used in isr_crt's frame counter
```

vs `ld a, (hl); inc a; ld (hl), a` (4 B).  Already covered in some
patterns.

### Bit testing (#71)

```
; A & 0x80 → boolean
rlca               ; 1 B  (carry := high bit)
and  $1            ; 2 B
                  ; vs srl a; srl a; ...; and 1  (per shift amount)
```

`SRL A` followed by `AND 1` is the canonical form for "extract bit N
of a small value".  `BIT n,A` sets Z but doesn't clear other bits;
when followed by `JR Z`, BIT is shorter (2 B) than CP+JR.

### Loop-invariant address (#89, #90)

```
; constant 16-bit value (e.g. address-of-extern) used inside a loop:
ld   de, sym       ; HOIST out of loop  ← regalloc must not clobber DE
loop:
ld   (hl), e
inc  hl
ld   (hl), d
inc  hl
djnz loop
```

vs reloading `ld de, sym` every iteration.  This is the seed-LDIR
opportunity (#88) — but even when LDIR doesn't fit, simple LICM
inside a loop pays for itself.

### Byte extract from extern-symbol address (#90)

```
; (uint8_t)(extern_sym >> 8)
ld   a, high(sym)        ; 2 B  if assembler supports high() reloc
                         ; or:
ld   hl, sym             ; 3 B
ld   a, h                ; 1 B  → 4 B
```

vs current 10 B route through `ld de,sym; ld l,d; ld h,$0; ld a,l`.

### `EX DE, HL` register swap

```
ex   de, hl        ; 1 B   (DE↔HL)
                  ; vs push de; push hl; pop de; pop hl  (4 B)
```

Already used liberally; flag for regalloc to prefer it over spills
when DE↔HL swap suffices.

### 16-bit shift left x2 / x4 / x8 / x16

```
add  hl, hl        ; 1 B  (HL *= 2; carry := high bit)
                  ; chain for higher powers
```

Used in CELL(x,y) computation (display memory addressing).

## Issue inventory (46 open, 2026-05-01)

Categorized by where the fix lives in the LLVM pipeline.  `D=` is the
difficulty estimate (S=trivial peephole, M=pattern in tablegen / new
combiner rule, L=cross-pass / regalloc work, X=infrastructure).
`I=` is impact (per-site bytes × est. occurrences).

### A. Late peepholes (`Z80LateOptimization.cpp`) — pattern-match + rewrite after RA

| # | Title | D | I | Z80 idiom |
|---|---|---|---|---|
| 60 | Redundant `LD A,reg` when A already holds the value | M | medium | known-value tracking |
| 71 | SRL A → RRCA when followed by AND mask | S | small | rotate-vs-shift |
| 75 | `CALL X; RET → JP X` cross-MBB | S | small | tail-call |
| 76 | `ld a, (hl); ld r, a` — actually optimal on Z80 (no `ld r,(hl)` for r≠A); CLOSE this issue | — | — | — |
| 77 | 8-bit countdown: `dec a; ld r,a; or a; jr nz` → `djnz` (or `dec r; jr nz`) | M | high | DJNZ |
| 78 | LDIR's post-state DE = dst+count not used | M | medium | LDIR aftermath |
| 79 | `(x != y) ? 0xFF : 0` → `add a,$ff; sbc a,a` | S | small | mask-from-flag |
| 83 | Dead `and 1` after `ld a,1` for `_Bool` | S | low (2 hits) | known-bits |
| 84 | HL backup through BC for restore — useless when `inc hl` advances anyway | M | medium | regalloc/peephole |
| 85 | Sequential consecutive-address stores → HL-walked `ld (HL),v / inc HL` | M | medium | store-chain |
| 86 | u8 switch range-check uses 16-bit SUB/SBC | M | medium | 8-bit cmp |
| 88 | N×16-bit constant pattern fill loop → seed-and-LDIR | M | medium | LDIR seed |
| 90 | `(uint8_t)(extern_addr >> 8)` byte-arg call: 10 B → 5 B | M | small (1 hit) | extern HI-byte |

### B. Instruction selection (`Z80InstructionSelector.cpp`, `Z80InstrGISel.td`)

| # | Title | D | I | Note |
|---|---|---|---|---|
| 7 | DJNZ, LDIR, CPIR, CP (HL) — broad target-instruction-driven codegen | L | high | umbrella for #77/#88/#50 |
| 50 | Unroll memcpy/memmove into LDI chains for speed-critical paths | M | medium | speed-axis |
| 64 | Inline `memmove(const_dst, const_src, const_n)` to LDIR/LDDR | S | small | |
| 73, 87 | -Oz: 8-byte memcpy unrolls inline instead of CALL to runtime stub | M | medium | inverse of #50 |
| 80 | `ld bc,(nn)` / `ld de,(nn)` not used | S | small | direct-addr |

### C. Register allocation / spill (Z80 RA pipeline)

| # | Title | D | I | Note |
|---|---|---|---|---|
| 16 | PUSH/POP instead of IX-indexed spills across CALLs | L | medium | |
| 18 | Known-value register copy optimization | M | medium | |
| 27 | Per-pair 16-bit register copy cost | L | medium | |
| 53 | `+static-stack` allocates trivially-constant locals to BSS unnecessarily | M | medium | const-fold first |
| 74 | RA spills to BSS when push/pop short-lived | L | high | |
| 89 | Loop-invariant 16-bit constant reloaded every iter (DE clobbered for counter) | L | medium | sibling of #74 |

### D. Frame lowering (`Z80FrameLowering.cpp`)

| # | Title | D | I | Note |
|---|---|---|---|---|
| 12 | `hasFP=false` correct but larger: IX CSR overhead exceeds frame savings | L | medium | |
| 40 | Evaluate IX frame pointer vs static-stack BSS per-function | L | medium | |

### E. Calling convention (`Z80CallingConv.td`, `Z80CallLowering.cpp`)

| # | Title | D | I | Note |
|---|---|---|---|---|
| 4  | `__critical` equivalent (DI/EI wrapper) | M | low | function attribute |
| 42 | Built-in intrinsics for DI, EI, HALT, IM 2, LD I,A | S | low | inline asm cleanup |
| 43 | Custom CC for CP/M BIOS entry points (BC/DE/HL params, C return) | L | medium | |

### F. Correctness / latent bugs

| # | Title | D | I | Note |
|---|---|---|---|---|
| 28 | -O0 codegen failures in large functions | L | bug | |
| 36 | va_arg incorrect — printf broken | L | bug | |
| 37, 38, 39 | +undocumented IY mis-emissions | L | bug | undoc-default cleanup |
| 63 | bench_string fails at -O0 only (de=045C, expect 00FF) | L | bug | |
| 65 | Z80PostRACompareMerge: CP_n etc. wrongly treated as setsZForA | L | bug | latent miscompile |
| 67 | Pre-existing lit-test failures: cmp-eq-regpressure, fib, interrupt, shift-opt, spill-regclass | L | bug | |
| 68 | Cross-block #60 LD A,r removal interferes with downstream peepholes | M | bug | pass ordering |
| 69 | Switch on shifted byte field uses stale register for second case | M | bug | latent miscompile |
| 82 | Static-stack u16 loop counter desync between live reg and frame slot | L | bug | |

### G. Toolchain / infrastructure

| # | Title | D | I | Note |
|---|---|---|---|---|
| 15 | Loop index → pointer conversion for Z80 code density | M | low | maybe-opt |
| 20 | BSS spill across CALL: multi-value pattern not handled | L | medium | |
| 35 | No standard C library | X | — | not codegen |
| 70 | -fverbose-asm doesn't annotate with source comments | M | qol | |
| 81 | Integrated assembler rejects apostrophe in `ex af, af'` | S | bug | |

## Clustering — fixes that share infrastructure

Many issues become one-liners once the underlying machinery is in
place.  Tackling clusters in order delivers compounding wins.

### Cluster 1: 8-bit primacy
**#77, #86, #83.**  Stop widening u8 to u16.  Counter loops, switch
range checks, `_Bool` stores all share the "u8 should stay u8" thread.
Single fix in legalizer + peephole audit.

### Cluster 2: DJNZ + LDIR family
**#7 (umbrella), #50, #64, #77, #88, #78.**  Loop-idiom recognizer
(detect `for(B;B;--B) { body }`) + memset/memcpy lowering + LDIR
post-state propagation.  Probably one new pass: `Z80LoopIdiomPass`
running after MachineLICM, before RA.

### Cluster 3: Block moves of various N
**#73, #87 (small memcpy unroll vs runtime stub), #50 (LDI unroll for
speed), #64 (memmove via LDDR).**  Threshold table per-N + per-`-Oz`/
`-O2`/`-O3`.  Same place — instruction selection of `G_MEMCPY` /
`G_MEMSET` / `G_MEMMOVE`.

### Cluster 4: Known-value tracking
**#60, #18, #83, #79.**  Track per-instruction A/HL/DE/BC known-bits
and known-values across MBBs.  Then redundant `ld a, 0`, dead `and 1`,
`(x!=y) ? 0xFF : 0` mask materialization all become consequences.

### Cluster 5: Direct-address loads
**#80, #90.**  `ld bc,(nn)` / `ld de,(nn)` / extern HI-byte extraction.
Same instruction-selection fix family.

### Cluster 6: Loop body shape
**#84 (HL backup through BC), #85 (consecutive stores), #88 (seed-LDIR),
#89 (constant reloaded).**  All show up in pattern-fill loops.  Once
DJNZ + LDIR-seed land, these are largely subsumed.

### Cluster 7: Calling convention / frame
**#12, #16, #40, #43, #74.**  IX vs static-stack vs push/pop spills.
Sizing question — wants a per-function cost model.

## Implementation status (2026-05-02)

Clusters tackled:

- ✅ **Cluster 4 (known-value)**: #60 imm form + #83 — landed in
  `Z80LateOptimization.cpp` (~130 line known-A peephole).  cpnos-rom
  payload 1750 → 1746 B (-4 B).  #79 (mask-from-flag) untouched —
  out of scope for post-RA peephole.
- ✅ **Cluster 5 (direct-address)**: #90 — landed in
  `Z80InstructionSelector.cpp` G_TRUNC fold (~30 line peephole).
  cpnos-rom .init 646 → 643 B (-3 B).  #80 closed without code
  change — investigation showed `ld bc/de,(nn)` ED-prefix is 4 B,
  same as `ld hl,(nn); ex de,hl` — no real size win.
- ✅ **Cluster 1 (8-bit primacy) regression locks**: #77 do-while
  form already DJNZs (`djnz-u8-counter.ll` pins behavior); #86
  basic switches already use 8-bit `cp` (`u8-switch-cmp.ll`
  pins).  Open: #77 `while(n--)` form (counter forced into C
  with per-iter round-trip — small win, no cpnos-rom hits) and
  #86 sparse-switch-on-truncated-i16 (compiler retains i16 ops
  through the case chain; bigger win but harder fix needing
  legalizer/combiner work).
- ✅ **Cluster 2 (DJNZ + LDIR family)** — partially closed:
  - #78 (LDIR aftermath: DE post-state reuse) — landed in
    `Z80LateOptimization.cpp` post-LDIR triple peephole, three
    downstream shapes (StoreBack, DropEx, Other) plus ±1 fixup
    (INC/DEC) and order-independent matcher.  Lit
    `ldir-aftermath.ll`.  cpnos-rom READ-SEQ inner loop: -6 B/iter
    absorbed into payload alignment padding.
  - #88 (seed-LDIR pattern fill) — new IR pass `Z80LoopIdiomFill`
    rewrites K-byte (K∈{1,2,3,4}) constant-trip-count fill loops
    as `seed K bytes; memcpy(base+K, base, K*(N-1))`.  Both new-PM
    and legacy entry points.  Lit `loop-idiom-fill.ll` covers
    K=1/2/3 (jump-table/IVT shape) plus volatile negative.
    cpnos's `setup_ivt` is `volatile` and correctly skipped.
  - #64 (memmove inline) — `Z80LegalizerInfo` G_MEMMOVE
    `.libcall()` → `.custom()` with direction analysis (same
    pointer; G_PTR_ADD chains; common base).  Picks LDIR when
    dst≤src, LDDR when dst≥src, libcall otherwise.  Lit
    `memmove-inline.ll`.
  - Still open: #50 unrolled LDI for speed, plus the new #91
    follow-up for LDDR codegen quality with constant Size.

Not-yet-tackled:

- Cluster 3 (memcpy thresholds): #73, #87 — partially handled by
  InstCombine fold guard (475a65378517) but #50 still open
- Cluster 6: subsumed by 2+4 (most issues now closed)
- Cluster 7: large; do last
- Correctness: #28, #36, #82 still open

Cumulative cpnos-rom impact: payload 1750 → 1738 B (-12 B); init
647 → 633 B (-14 B).  Z80 lit suite: 68/68 PASS at every commit
(plus 1 XFAIL for #82).

## Implementation roadmap

Suggested order, tackling clusters from cheap-and-impactful first:

1. **Cluster 4 (known-value)** — small, broad benefit; unblocks #60/#83/#79
   - new pass `Z80KnownValueTracker.cpp` running between
     `Z80PostRACompareMerge` and `Z80LateOptimization`
   - ~2-day fix

2. **Cluster 5 (direct-address)** — instruction-selection patterns
   - 4 new tablegen patterns in `Z80InstrGISel.td`
   - lit tests: `direct-addr-bc.ll`, `direct-addr-de.ll`,
     `extern-addr-hi-byte.ll`
   - ~1-day fix

3. **Cluster 1 (8-bit primacy)** — legalizer audit
   - `Z80LegalizerInfo.cpp`: don't widen i8 cmp/sub when result is i8 or boolean
   - 3 new lit tests
   - ~2-day fix

4. **Cluster 2 (DJNZ + LDIR)** — loop idiom recognition
   - new pass `Z80LoopIdiomPass`
   - existing `djnz.ll` lit test as seed; add seed-LDIR / LDDR / pattern-fill
   - ~5-day fix

5. **Cluster 3 (memcpy / memset of various N)** — threshold + lowering
   - extend `Z80LegalizerInfo::selectMemcpy` (or wherever it lives)
   - add per-`Oz` thresholds
   - ~2-day fix

6. **Cluster 6 (loop body shape)** — mostly subsumed by #2 + #4

7. **Correctness bugs** (F category) — case-by-case, parallel to feature work

8. **Cluster 7 (calling conv / frame)** — large, do last

## Per-issue test strategy

Every fix gets a lit test in `llvm/test/CodeGen/Z80/<topic>.ll`,
following the convention that's already there.  Each test:

- Has the smallest C-equivalent IR that triggers the pessimization.
- Uses `; RUN: llc -mtriple=z80 -O2` (or `-Oz`) before the FileCheck.
- `; CHECK-NOT:` the bad pattern.
- `; CHECK:` the expected good emission.
- Optionally a `; XFAIL:` line if landing the test before the fix
  (so the pre-fix CI marks it as expected-fail and the fix flips it
  to pass).

Naming: one file per issue or one per cluster.  Suggested:

```
llvm/test/CodeGen/Z80/
  djnz.ll                          (existing)
  djnz-u8-counter.ll               (#77)
  ldir-pattern-fill.ll             (#88)
  ldir-aftermath-de.ll             (#78)
  lddr-memmove.ll                  (#64)
  memcpy-thresholds.ll             (#73, #87)
  cmp-u8-not-widened.ll            (#86)
  bool-no-mask.ll                  (#83)
  mask-from-flag.ll                (#79)
  known-zero-a.ll                  (#60 immediate-form)
  known-value-cross-mbb.ll         (#60 reg-form)
  direct-addr-bc.ll                (#80 ld bc, (nn))
  direct-addr-de.ll                (#80 ld de, (nn))
  extern-addr-hi-byte.ll           (#90)
  tail-call-cross-mbb.ll           (#75)
  hl-no-bc-backup.ll               (#84)
  store-chain-walk.ll              (#85)
  rrca-and-mask.ll                 (#71, existing as lshr-rrca.ll)
  loop-licm-extern-addr.ll         (#89)
  bool-and-1-dead.ll               (#83 detail)
  switch-shifted-byte.ll           (#69 — correctness)
  cp-not-setszfora.ll              (#65 — correctness)
  static-stack-u16-loop.ll         (#82 — correctness, exists as XFAIL)
  va-arg-printf.ll                 (#36 — correctness)
  o0-large-func.ll                 (#28 — correctness)
  ix-cost-model.ll                 (#12, #40)
  pushpop-spill.ll                 (#74)
  custom-cc-bc-de-hl.ll            (#43)
  cstdlib-builtins.ll              (#42)
  apostrophe-ex-af-af.ll           (#81 — assembler)
```

## Speed-axis (CONOUT hot path)

Specific to rcbios/cpnos's character-output loop: every byte from CCP
to the SIO traverses `bios_conout → impl_conout → console_putc →
display poke → cursor_right`.  Per-byte hot path matters at 9600 baud
(~1 ms per byte) and especially at sustained 76800 baud (~130 µs per
byte = 520 T at 4 MHz).

Speed wins not yet captured by tightness optimisations:

- **Function inlining at -O3** — the resident BIOS shim chain
  (`bios_conout_shim → impl_conout`) is 3 B + 88 B = 91 B with one
  CALL; merge for ~17 T saved per byte.
- **Branch ordering** — predict the common path (printable byte, no
  CR/LF/control) for fall-through; misprediction cost ~5 T extra.
- **DJNZ for the 80-byte-row scroll** — already LDIR via memcpy, fine.

These are incremental tunings, smaller than the size wins.  Worth
benchmarking but not lit-testable without a cycle counter.

## Per-fix upstream package

For each cluster (or issue, when not clusterable), the upstream PR
contains:

1. **Reproducer** — a self-contained `.c` showing the pessimization
   with `clang --target=z80 -Oz` (or appropriate flags).
2. **Current output** — assembly listing with size in bytes.
3. **Expected output** — the canonical Z80 sequence.
4. **Lit test** — added to `llvm/test/CodeGen/Z80/`.
5. **Fix** — the actual patch.
6. **Size delta** — measured against rcbios + cpnos-rom builds (the
   two known-good Z80 codebases in this workspace).

## Backlog (parked / later)

- **#91 LDDR setup quality.**  Tracked upstream as ravn/llvm-z80#91
  (constant-fold Size-1 and the two PtrAdds when Size is a G_CONSTANT
  inside the new G_MEMMOVE custom legalizer).  Polish only -- semantics
  are correct, this is just `~25 B → ~12 B` for memmove(dst, src, K)
  where the direction analysis fires.

## Infrastructure follow-ups (low priority, but cheap)

- [ ] **CI: GitHub Actions workflow for Z80 lit suite.**  Two
      pre-existing test stalenesses (`lshr-rrca.ll`, `switch-byte-field.ll`)
      landed broken on their introducing commits because the lit suite
      wasn't gated.  Adding `.github/workflows/lit.yml` would catch
      this class of regression at PR time.

      Sketch (gates only on Z80 target / test path changes):
      ```yaml
      on:
        pull_request:
          paths:
            - 'llvm/lib/Target/Z80/**'
            - 'llvm/test/CodeGen/Z80/**'
            - 'clang/cmake/caches/Z80.cmake'
        push:
          branches: [main]
      jobs:
        z80-lit:
          runs-on: ubuntu-latest
          steps:
            - uses: actions/checkout@v4
            - uses: hendrikmuhs/ccache-action@v1
              with: { key: z80-lit }
            - run: |
                cmake -C clang/cmake/caches/Z80.cmake -G Ninja -S llvm -B build \
                    -DCMAKE_BUILD_TYPE=Release \
                    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
                ninja -C build llc clang FileCheck not count llvm-config
                build/bin/llvm-lit -v llvm/test/CodeGen/Z80/
      ```

      Cost: free (public repo).  Wall-clock per PR: ~4-8 min cached,
      ~25-35 min cold.  The Z80-only cmake cache shrinks the build
      versus upstream LLVM's full suite.

      Optional matrix entry: also run `cargo run -- clang` from
      `z80-utils/test-runner/` for the integration suite -- needs
      z88dk-ticks pre-installed via the `llvm-z80-test` Docker image
      that CLAUDE.md describes.

## Open questions for upstream

- What's `jacobly0`'s plan for the Z80 backend?  Last activity ~2025;
  unclear if patches are accepted upstream.
- Is the repository `ravn/llvm-z80` or `jacobly0/llvm-z80` the right
  PR target?  (Memory: file issues in ravn/* fork; PRs separate.)
- Stability of the GISel pipeline — some fixes touch Combine.td which
  is regenerated; need to keep the stable pieces minimal.
