# 3-pair-set LDIR/DJNZ codegen baseline — AES corpus audit 2026-06-21

**Branch**: `3-pair-set-ldir-djnz` (follow-up to
`3-pair-set-ldir-djnz-baseline-2026-06-21.md`).

**Goal**: re-run Phase A's LDIR/DJNZ origin + cleanliness audit on the
AES-256 corpus (per the user's "AES is important too" framing).  Find
3-pair-set codegen gaps that AES surfaces but production hides.

**Outcome**: AES has even **less** 3-pair-set surface than production.
Zero LDIR sites in any of the 4 configs scanned.  One DJNZ site in
`09_Oz_prod_like` (clean).  No missed-DJNZ candidates.  The 3-pair-set
machinery is fully effective on this workload too -- there's nothing
to fix here either.

## Methodology

Used the existing AES corpus ELFs from `tasks/aes256-corpus/sweep/`
(produced by `flag_sweep.sh`).  Disassembled the 4 chosen configs
with `llvm-objdump -d --triple=z80` to
`/tmp/3-pair-set/aes/<config>.disas.s`.  Searched for
`ldir`/`lddr`/`cpir`/`cpdr` and `djnz` occurrences.  Cross-checked
for missed-DJNZ candidates via `dec [acdehl]; jr nz` and
`dec (bc|de|hl); jr nz` patterns.  Surveyed backward conditional
branches (loop tails) to characterise loop shapes that didn't
DJNZ.

### Configurations chosen

| Config | Flags | Why included |
|--------|-------|--------------|
| `01_baseline_Oz` | `-Oz` | Most permissive (no production knobs) |
| `04_O2` | `-O2` | Non-`-Oz` optimization mix |
| `05_Oz_static_stack` | `-Oz +static-stack` | Exposes #192-class issues |
| `09_Oz_prod_like` | `-Oz +static-stack -disable-lsr -disable-machine-licm -disable-machine-cse -ffunction-sections -fdata-sections` | Closest to production |

## AES codegen counts

| Config | LDIR | LDDR | CPIR | CPDR | DJNZ |
|--------|------|------|------|------|------|
| `01_baseline_Oz` | 0 | 0 | 0 | 0 | 0 |
| `04_O2` | 0 | 0 | 0 | 0 | 0 |
| `05_Oz_static_stack` | 0 | 0 | 0 | 0 | 0 |
| `09_Oz_prod_like` | 0 | 0 | 0 | 0 | 1 |
| **Aggregate** | **0** | **0** | **0** | **0** | **1** |

## Why zero LDIR

AES-256's cipher operations are byte-by-byte ALU work with table
lookups (sbox, rcon, gf_alog, gf_log).  There is no `memcpy` /
`memset` / `memmove` in the algorithm itself, and the test harness
doesn't include LDIR-shaped patterns either.  The compiler has no
opportunity to emit LDIR because there's no source-level pattern to
recognise as one.

(Production triplet's LDIR sites all originate from `__builtin_memcpy`
/ `__builtin_memset` for IVT initialisation, BSS clearing, message
copying, etc.  AES doesn't have those.)

## DJNZ -- the single site

`09_Oz_prod_like` has 1 DJNZ in `_main`, at byte offset `0x8d3`.
Context:

```asm
 8c7: ld   e, $11      ; constant
 8c9: ld   d, a        ; setup
 8ca: xor  a           ; accumulator = 0
 8cb: ld   b, $8       ; B = 8 (fixed trip count)
 8cd: add  a, a        ; ┐ multiplication-by-shift body:
 8ce: rl   d           ; │ shift D left through carry
 8d0: jr   nc, $8d3    ; │ skip add if low bit of D was 0
 8d2: add  a, e        ; │ accumulate
 8d3: djnz $8cd        ; ┘ 8 iterations
```

Classic 8-bit-fixed-count multiplication loop using DJNZ as the
shift counter.  B-pinned correctly; no waste.

## DJNZ -- the missed-opportunity candidates (also zero)

Scanned all 4 configs for `dec [acdehl]; jr nz, label` (8-bit
countdown using a non-B counter that should DJNZ) and
`dec (bc|de|hl); jr nz, label` (16-bit countdown potentially
narrowable).  **Zero candidates in any config.**

## Why DJNZ doesn't fire on the other AES loops

`09_Oz_prod_like` has 3 backward conditional branches (loop tails)
besides the one DJNZ.  Each is a loop shape that's NOT DJNZ-able:

### Loop 0x4a -- `gf_mul`-style count-up with overflow exit

```asm
 28: <loop body using B, C, L>
 ...
 49: inc  b              ; ↑ count UP
 4a: jr   nz, $28        ; loop until B wraps to 0 (256 iters)
 4c: ld   b, $0          ; ...then continue
```

This counts UP through B with overflow-to-zero termination.
Semantically equivalent to "do 256 iterations" but expressed as
count-up.  DJNZ counts DOWN; converting requires changing the
loop direction at the IR / source level.  Not a 3-pair-set
allocation issue.

### Loop 0x965 -- search loop (sbox lookup-by-value)

```asm
 951: ld   a, e
 952: cp   $10            ; exit if E == 16
 ...
 96d: jr   z, $951        ; loop back
```

Searches for a value match; exits on found OR on E reaching 16.
**This is CPIR territory** (per #7 subtask "CPIR for search/compare
loops") -- recognising this idiom and lowering to CPIR would be a
codegen improvement, but it's idiom recognition not register
allocation.  Off the 3-pair-set scope.

### Loop 0x9e8 -- compound-termination via OR

```asm
 9e1: inc  de
 9e2-9e7: <compute h XOR b, l XOR c, OR h>
 9e8: jr   z, $9ca        ; loop until (HL XOR BC) == 0
```

Termination on a computed compound condition (DE incrementing as IV,
HL/BC compared via XOR-OR).  Not a fixed-trip loop; can't DJNZ.

## Cross-target comparison

| Workload | LDIR sites | LDIR clean | DJNZ sites | Missed-DJNZ candidates |
|----------|-----------|------------|------------|------------------------|
| Production (autoload) | 10 | 10/10 | 2 | 0 |
| Production (cpnos) | 2 | 2/2 | 0 | 0 |
| Production (rcbios) | 11 | 11/11 | 4 | 0 |
| AES `01_baseline_Oz` | 0 | n/a | 0 | 0 |
| AES `04_O2` | 0 | n/a | 0 | 0 |
| AES `05_Oz_static_stack` | 0 | n/a | 0 | 0 |
| AES `09_Oz_prod_like` | 0 | n/a | 1 | 0 |

Combined: **23 + 1 = 24 LDIR+DJNZ sites scanned, all clean.  Zero
missed-DJNZ candidates across two workloads.**

## What this confirms

The 3-pair-set machinery (Z80SplitDjnzCounters + BReg/BCReg/GR16NoIR
single-register classes + getRegAllocationHints + direct LDIR emission
in GISel) is **comprehensively effective**.  Production and AES
together exercise:

- LDIR with constant-address operands (production)
- LDIR with calling-convention-supplied vregs (production ZX0)
- LDIR with hand-written-asm origin (production runtime stubs)
- DJNZ on variable-shift loops (both)
- DJNZ on fixed-trip-count multiplication (AES)
- Loops that *should not* DJNZ (count-up / search / compound)

The compiler gets the right answer in every case.

## What's left in the "3-pair-set work" framing

After this audit, the original framing collapses further:

- **(P1) LDIR source-vreg constraints**: production-clean, AES has no
  LDIR.  Moot.
- **(P2) DJNZ detector coverage**: production-clean, AES has 1 clean
  DJNZ + 3 loops that genuinely can't DJNZ.  No coverage gap found.
- **(P3) #110 hint priority generic-fix**: still architecturally
  relevant but no workload-driven evidence for it from this audit.

The only remaining "gap" is the documented lit-test XFAIL
(`issue-97a-bc-pingpong-i16-counter.ll`) referenced by **#111**.
That's a *synthetic* shape that doesn't appear in production OR AES,
but is the witness for the HLReg single-register class extension.

## Phase D decision options

### Option X: close the 3-pair-set branch as work-not-needed

Phase A + this writeup are the deliverable.  The branch confirms
empirically (across production + AES) that the existing 3-pair-set
machinery is comprehensively effective.  No code changes; merge the
two writeups back to `main`.

Cost: 15 min for a merge commit.  Net contribution: empirical
confirmation of the fork architecture quality.

### Option Y: widen further to compiler-comparison-corpus

Re-run the same scan on the comparison corpus (`pi`, `sieve`,
`fannkuch`, `gf_log`, etc.).  If those workloads also clean, the
"3-pair-set is done" claim becomes very strong.  If they find a gap,
that's a witness for Option Z or a new direction.

Cost: ~45 min.  Risk: still finds nothing.

### Option Z: implement HLReg per #111 (parallel-closes #115)

Despite Phase A + this audit finding zero production-or-AES gaps,
the documented lit XFAIL exists.  Implementing HLReg per #111's
sketch closes:

- The `issue-97a-bc-pingpong-i16-counter.ll` XFAIL (concrete
  deliverable).
- Partial-close of the parked #115 (the same HLReg class is its
  IY-extraction route).

Cost: ~1-2 sessions of careful TableGen + GISel work, test-first per
`feedback_test_before_fix`.

### Option W: drop the 3-pair-set work entirely, pivot to a different open issue

E.g. #235 (memcpy_z80 builtin), #175 (missing 8-bit ALU mem-operand
instructions, SDCC emits 50+/AES round), #221 (-g defeats DJNZ +
~20 other peepholes).  Different scope entirely; informed by the
audit's "this area is done" finding.

## Recommendation

**Option X + Option Z in sequence**.  Close the audit branch with the
two writeups (empirical record of "production + AES are clean"), then
pick up HLReg as the natural next step.  HLReg has two waiting
consumers (#111 + #115) and the design is already documented in the
parked #115 writeup.

If you'd rather pivot to a non-3-pair-set issue (Option W), #175
(8-bit ALU mem-operand) has the largest documented gap relative to
SDCC ("emits 0 vs SDCC emits 50+ per AES round") and is directly
visible in the AES corpus we just scanned.

## Files produced

Under `/tmp/3-pair-set/aes/`:
- `01_baseline_Oz.disas.s` (and the other 3 configs)

Existing AES ELFs reused from `tasks/aes256-corpus/sweep/`.

## Cross-references

- `llvm-z80/tasks/3-pair-set-ldir-djnz-baseline-2026-06-21.md`
  -- Phase A (production audit) with the same conclusion.
- `llvm-z80/tasks/issue115-iy-unreserve-investigation-2026-06-21.md`
  -- parked HLReg/DEReg design sketch (re-usable for Option Z).
- ravn/llvm-z80#7 -- meta tracker "Implement Z80 instruction-driven
  codegen: DJNZ, LDIR, CPIR, CP (HL)"; production+AES status confirms
  the existing implementation matches the goal.
- ravn/llvm-z80#111 -- HLReg for i16-counter pointer-arg (Option Z).
- ravn/llvm-z80#115 -- HLReg/DEReg for IY-extraction (parked; same
  HLReg infrastructure).
- ravn/llvm-z80#175 -- missing 8-bit ALU mem-operand (Option W
  candidate; AES-visible).
- `tasks/memory/feedback_production_hard_aes_soft.md` -- the priority
  frame driving this audit.
