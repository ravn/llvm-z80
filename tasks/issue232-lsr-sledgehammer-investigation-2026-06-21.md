# Issue #232 — investigation 2026-06-21

**Goal**: identify what cost signal LSR is missing on production Z80 builds
that forces the `-mllvm -disable-lsr` sledgehammer.

**Method**: empirical A/B on the three production targets (autoload-in-c,
cpnos-in-c, rcbios-in-c).  Build each with LSR disabled (current state) vs
LSR enabled; capture raw text size, compressed text size (where ZX0
applies), final ROM size, and per-function size deltas.  Dump the
worst-regressing function's disassembly to identify the actual transform.

**Outcome**: the answer is NOT one of the three hypotheses proposed in the
issue body (a `NumRegs` lying / b formula sub-optimal / c upstream
IndVarSimplify widening).  It's a **fourth hypothesis** that the issue
body didn't consider: **(d) downstream compressor-friendliness loss**.
And one of the three flags is a stale no-op that can be removed today.

## Numbers

| Target | Raw text  Δ | Compressed Δ | ROM Δ | Compression |
|--------|-------------|--------------|-------|-------------|
| autoload | **−1 B** | **+13 B** | +13 B | ZX0 |
| cpnos    | **+2 B** | **+13 B** | +11 B | ZX0 |
| rcbios   |  0 B     | (none)       |  0 B  | none        |

Sign convention: positive = bigger with LSR on (i.e. `-disable-lsr` helps).

Method:
1. Build each target with the committed Makefile (LSR off).
2. Edit Makefile to drop `-mllvm -disable-lsr`, rebuild, measure.
3. Restore Makefile.

ROM and raw text sizes from `wc -c` on the output binaries.
Compressed text size from `wc -c clang/text_compressed.zx0` (autoload) /
`clang-prom1lineprog/payload.zx0` (cpnos).  Per-function sizes from
`llvm-nm --print-size --size-sort`.

## What LSR actually does on autoload's `_main_relocated`

This is the largest function (611 B baseline, 623 B LSR-on; +12 B).  It's
the dominant contributor to autoload's regression.  The asm diff
(addresses stripped to compare instruction streams) shows the
characteristic LSR strength-reduction pattern:

Baseline (LSR off), per-iteration access:
```
ld    (hl),a
inc   hl
ld    a,d
inc   a
ld    d,a
```

LSR on, per-iteration access (after a one-time IY-pointer setup):
```
ld    (iy+offset),a
inc   de
```

LSR has replaced the per-iteration pointer recomputation chain (5 insns,
7 B) with an IY-indexed store (1 insn, 3 B) plus a one-time setup:

```
ld    hl,base
add   hl,de
push  hl
pop   iy            ; transfer base+offset into IY (7 B one-time)
```

**Locally this is exactly what cost-model would say is good**: LSR has
made each loop iteration ~4 B smaller in exchange for ~7 B of fixed
setup.  Across the function, the trade-off nets out to +12 B of raw text
(the function has multiple loops where the setup overhead doesn't
amortise as well as the per-iteration savings).

The per-function delta table across the whole PROM:

| Function | Δ |
|----------|---|
| `_main_relocated` | +12 B |
| `_fdc_write_full_cmd` | +7 B |
| `_fdc_read_result` | +4 B |
| `_compare_6bytes` | −3 B |
| `_check_sysfile` | −3 B |
| `_fdc_write_when_ready` | −9 B |
| `_fdc_read_when_ready` | −9 B |
| **net raw**  | **−1 B** |

So LSR's effect on raw code size is essentially neutral: +23 B in three
functions, −24 B in four others.  Local strength-reduction wins balance
local setup overhead.

## Why ROM grows by +13 B if raw text changes by −1 B

The ROM growth is entirely the ZX0 compressed payload:

```
autoload baseline:  raw 1959 B   zx0 1490 B   ROM 1669 B
autoload LSR on:    raw 1958 B   zx0 1503 B   ROM 1682 B
                    Δ raw -1 B   Δ zx0 +13 B  Δ ROM +13 B
```

LSR's transformed code shape compresses worse under ZX0.  The mechanism
is consistent with the IY-indexed access pattern we see in the asm diff:

- Baseline uses repeated `inc hl; ld a, d; inc a` patterns -- 1-byte
  instructions on common register pairs, highly repetitive byte sequences
  that ZX0 finds easy to compress (literal-back-references win).
- LSR-on uses `ld (iy+N), a` -- the `0xFD` IY prefix combined with a
  per-use-site offset byte produces more diverse byte sequences across
  the loop body.  ZX0 has fewer back-reference opportunities.

This is the same effect the cpnos Makefile comment already documented:
> -disable-lsr is kept: LSR is IR-level (still runs), and on cpnos the
> flag is marginally beneficial via better ZX0 compressibility.

So the autoload Makefile annotation ("saves ~90 bytes") is stale, but
the underlying reason ("LSR hurts ZX0 compressibility") still applies.
The reduction from "~90 B" to "+13 B" is consistent with the cost-model
improvements landed since the original measurement (issue #177's
`isLegalAddImmediate`, the IY-leak fixes #189/#27/#112, etc.) which have
made the raw delta close to neutral.  The ZX0 delta is more stable
because it tracks byte-pattern statistics, not raw code count.

## Hypothesis verdict

The issue body proposed three hypotheses (a) `NumRegs` lying, (b)
formula scoring sub-optimal, (c) widening upstream of LSR.  None match.

**Hypothesis (d) — downstream compressor-friendliness loss.**  LSR's
transform is locally correct.  Per-(opcode, type) cost cannot model
"how well does this byte pattern compress under ZX0," because ZX0's
benefit is a *global* property of the entire byte stream, not a property
of any single instruction.

This is the same architectural limit that closed #184: per-(opcode,
type) scalar cost cannot encode properties that depend on context the
cost API doesn't carry.  For #184 the missing context was post-regalloc
register pressure.  For #232 the missing context is global byte-pattern
statistics of the compressed artifact.

**Implication**: no cost-model edit, no IndVarSimplify tweak, no
`-z80-experimental-tti-costs` extension will retire `-disable-lsr` on
autoload or cpnos.  The sledgehammer is the right tool because the
problem is structurally outside TTI's expressive power.

## Surprise finding: rcbios `-disable-lsr` is a stale no-op

rcbios is byte-identical with LSR on vs off:
- BIOS size: 5462 B in both configurations.
- Disassembly: 2006 lines, byte-identical at the instruction level
  (`llvm-objdump -d` confirms).

LSR has no effect on rcbios's IR at -Oz today.  The `-mllvm -disable-lsr`
flag in `rcbios-in-c/clang/Makefile:41` is purely a stale carry-over.
**It can be removed.**  Doing so simplifies the build and removes a
"why is this here?" puzzle for future readers.

This is the only actionable byte-quality finding of this investigation.

## What to do with #232

Three options:

1. **Close #232 as WONT-FIX (investigation complete)**.  The
   sledgehammer is genuinely needed on autoload + cpnos for a reason
   that's outside the TTI cost model.  Document the finding in the
   issue body via a comment.  Net effect on production: zero on
   autoload + cpnos; rcbios's stale flag becomes a separate tiny PR.
2. **Keep #232 open** as a permanent marker against future
   re-attempts.  Same effect as option 1 but with the issue staying in
   the open queue.
3. **Pivot #232** from "cost-model investigation" to "ZX0-aware
   codegen heuristic."  This would be a new direction: e.g. a Z80
   peephole or pre-emit pass that re-shapes IY-indexed sequences into
   the better-compressing baseline pattern when in a `-Oz` /
   `-fcompressible-text` mode.  Out of scope for the current TTI
   investigation; would need its own design + measurement.  Not
   recommended without a specific motivating workload.

Recommend option 1.

## What stays as durable output

- This writeup as the empirical record (numbers, methodology, asm diff
  excerpt).
- A small PR removing rcbios's stale `-disable-lsr` flag (tracker
  required; this is its own task #234 if we file it).
- Comment on #232 with the conclusion + numbers; no code edits to
  llvm-z80 itself.

## Files produced under /tmp/issue232/ (transient)

- `build-baseline.log` / `build-lsr-on.log` -- autoload build logs.
- `autoload-baseline.elf` / `autoload-lsr-on.elf` -- ELF artefacts.
- `cpnos-baseline.elf` / `cpnos-lsr-on.elf` -- ELF artefacts.
- `cpnos-baseline.payload.{bin,zx0}` /
  `cpnos-lsr-on.payload.{bin,zx0}` -- raw + compressed payload bytes.
- `rcbios-baseline.elf` / `rcbios-lsr-on.elf` -- ELF artefacts.
- `baseline.sizes.txt` / `lsr-on.sizes.txt` -- autoload per-function
  sizes.
- `cpnos-baseline.sizes.txt` / `cpnos-lsr-on.sizes.txt` -- ditto cpnos.
- `rcbios-baseline.sizes.txt` / `rcbios-lsr-on.sizes.txt` -- ditto
  rcbios.
- `main_relocated.{baseline,lsr-on}.s` -- worst-regressing function
  disassembly for the asm diff.
- `delta.txt` -- per-function delta table for autoload.
- `main.{b,p}.instr` and `main.{b,p}.opc` -- stripped instruction
  streams for the address-shift-aware diff.

These are not committed; they can be regenerated by the methodology
above.

## Cross-references

- ravn/llvm-z80#232 (this investigation closes the first-step question).
- `llvm-z80/tasks/session-2026-06-21-z80-tti-modelling-investigation.md`
  -- the broader TTI sweep + inverse-analysis writeup that motivated
  #232.
- ravn/llvm-z80#184 -- WONT-FIX; the architectural-limit twin of #232
  (per-(opcode, type) cost cannot encode regalloc pressure either).
- `rc700-gensmedet/cpnos-in-c/Makefile:113-118` -- the comment that
  already documented the ZX0 reason on cpnos.
