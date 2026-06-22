# 3-pair-set LDIR/DJNZ baseline — compiler-comparison-corpus audit 2026-06-21

**Branch**: `3-pair-set-ldir-djnz`
(third writeup; follows
`3-pair-set-ldir-djnz-baseline-2026-06-21.md` (production) and
`3-pair-set-ldir-djnz-aes-audit-2026-06-21.md` (AES)).

**Goal**: complete the LDIR/DJNZ origin + cleanliness audit by
extending coverage to the compiler-comparison-corpus
(`rc700-gensmedet/tasks/compiler-comparison-corpus/`).  Confirm
whether the "3-pair-set is comprehensively effective" finding from
production + AES holds on a third independent workload.

**Outcome**: confirmed.  Across 6 corpus binaries: 3 LDIR sites (all
clean, all hand-written-runtime origin), 2 DJNZ sites (clean, both
compiler-rt helpers), 1 missed-DJNZ candidate in a compiler-rt
shift helper that uses EXX (making B-as-counter unsafe).
**No regalloc gap.**

## Methodology

Reused existing ELFs at
`tasks/compiler-comparison-corpus/sweep/llvm_z80_*.elf` (produced by
`sweep.sh`).  Disassembled each with `llvm-objdump`.  Same
counting + categorisation as Phase A and the AES audit.

## Per-binary LDIR/DJNZ counts

| Binary | LDIR family | DJNZ | Missed-DJNZ (8-bit) | 16-bit narrowable |
|--------|-------------|------|---------------------|-------------------|
| `llvm_z80_fannkuch` | 0 | 0 | 0 | 0 |
| `llvm_z80_licm_pessimize` | 0 | 0 | 0 | 0 |
| `llvm_z80_mm` | 2 | 0 | 0 | 0 |
| `llvm_z80_pi` | 1 | 2 | 1 | 0 |
| `llvm_z80_sieve` | 0 | 0 | 0 | 0 |
| `llvm_z80_word_fill` | 0 | 0 | 0 | 0 |
| **Aggregate** | **3** | **2** | **1** | **0** |

## LDIR sites — categorisation

### `llvm_z80_mm`: 2 LDIR/LDDR (hand-written `memmove` runtime)

The `mm` binary calls `memmove` for matrix transposition.  Both
sites are in the runtime `_memmove` stub (the same hand-written
asm that ships with the runtime; provides both forward LDIR and
reverse LDDR paths).

```asm
; reverse path (LDDR, downward copy for overlap)
10a: jr   c, $114    ; if forward, jump
10c: add  hl, bc     ; HL = src + count
10d: dec  hl         ; HL = src + count - 1
10e: ex   de, hl
10f: add  hl, bc     ; DE = dst + count - 1
110: dec  hl
111: lddr            ; downward copy
113: ret

; forward path
114: ex   de, hl     ; swap to canonical HL=src, DE=dst
115: ldir
117: ret
```

Both clean.  Hand-written; not in scope for regalloc audit.

### `llvm_z80_pi`: 1 LDIR (compiler-emitted, `_pi_init`)

```asm
_pi_init:
 133: ld   de, $376      ; dst
 136: ld   hl, $7d0      ; constant
 139: ld   ($374), hl    ; store the seed value
 13c: ld   hl, $374      ; src = the just-stored location
 13f: ld   bc, $22e      ; count = 558 bytes
 142: ldir
 144: ret
```

Initialises the pi-digit array with the seed pattern.  Compiler-
emitted from `__builtin_memcpy` or analogous; clean canonical
`LD HL,nn; LD DE,nn; LD BC,nn; LDIR` pattern.

## DJNZ sites — categorisation

### `llvm_z80_pi`: 2 DJNZ (both compiler-rt helpers)

```asm
__udiv32_skip:        ; 32-bit division helper
 1c7: exx
 1c8: djnz $18b       ; 32 iterations driven by B
 1ca: ret
```

```asm
___mulhi3_skip:       ; 16-bit multiplication helper
 33d: djnz $336       ; 16 iterations driven by B
 33f: ex   de, hl
```

Both compiler-rt routines hand-written for Z80; B used correctly
as the bit-counter.  Clean.

## Missed-DJNZ candidate -- pi compiler-rt shift helper

```asm
 357: ...              ; loop body using A, B, C, EXX-swapped pair
 362: sla  c
 364: rl   b
 366: exx
 367: rl   c
 369: rl   b
 36b: exx
 36c: dec  a           ; counter in A
 36d: jr   nz, $357    ; loop back
 36f: exx
```

The loop counter lives in A and decrements via `dec a; jr nz`
instead of `djnz`.  Net cost: 1 B per iteration's counter
check (3 B `dec a; jr nz` vs 2 B `djnz`), but A is used here
because B is part of the EXX'd pair.  Using B as the loop
counter would require avoiding EXX while B is live, which would
add far more bytes than the 1 B saved.

This is a **hand-tuned compiler-rt choice**, not a regalloc
miss.  Not a 3-pair-set gap.

## Cross-target totals (all three workloads)

| Workload | LDIR sites | LDIR clean | DJNZ sites | Missed-DJNZ |
|----------|-----------|------------|------------|-------------|
| Production (autoload) | 10 | 10/10 | 2 | 0 |
| Production (cpnos) | 2 | 2/2 | 0 | 0 |
| Production (rcbios) | 11 | 11/11 | 4 | 0 |
| AES `01_baseline_Oz` | 0 | n/a | 0 | 0 |
| AES `04_O2` | 0 | n/a | 0 | 0 |
| AES `05_Oz_static_stack` | 0 | n/a | 0 | 0 |
| AES `09_Oz_prod_like` | 0 | n/a | 1 | 0 |
| Corpus `fannkuch` | 0 | n/a | 0 | 0 |
| Corpus `licm_pessimize` | 0 | n/a | 0 | 0 |
| Corpus `mm` | 2 | 2/2 (hand-written) | 0 | 0 |
| Corpus `pi` | 1 | 1/1 | 2 | 1 (compiler-rt EXX) |
| Corpus `sieve` | 0 | n/a | 0 | 0 |
| Corpus `word_fill` | 0 | n/a | 0 | 0 |
| **Aggregate** | **26** | **26/26** | **9** | **1** (hand-tuned, not regalloc) |

26 LDIR sites all clean.  9 DJNZ sites all clean.  1 "missed-DJNZ"
that's actually a deliberate compiler-rt choice (EXX makes B
unsafe).  **Zero genuine 3-pair-set regalloc gaps across three
workloads.**

## What this confirms

The triple-workload audit makes the empirical case airtight:

- LDIR/LDDR/CPIR/CPDR emit the canonical
  `LD HL,nn; LD DE,nn; LD BC,nn; LDIR` pattern across every
  compiler-emitted site.
- DJNZ fires on every loop where it can; the loops where it
  doesn't are loops that genuinely cannot DJNZ (count-up,
  search, compound termination, EXX-conflict-with-B).
- The single-register-class machinery
  (`Z80SplitDjnzCounters` + `BReg` / `BCReg` / `GR16NoIR`),
  plus `getRegAllocationHints` + direct LDIR emission in
  GISel, are comprehensively effective.

## Phase D decision (final form)

The audit work is complete.  The "3-pair-set work" framing is
empirically refuted: the existing machinery already delivers
what was hypothesised to need improvement.  Two paths forward:

### Option X (close the branch as work-not-needed)

Three writeups on the branch document the empirical finding
across production + AES + corpus.  Merge to `main` as the
durable record; close the branch.

### Option Z (implement HLReg per #111)

Despite zero workload gaps, the documented lit XFAIL
(`issue-97a-bc-pingpong-i16-counter.ll`) exists, and the parked
#115 wants HLReg too.  Implementing HLReg closes those two
trackers and lands a long-asked-for register class.  ~1-2
sessions.

User has indicated Y (this audit) + Z in sequence -- proceeding
to Z next, with a thorough plan before any TableGen / ISel work.

## Files produced

Under `/tmp/3-pair-set/corpus/`:
- `llvm_z80_fannkuch.disas.s` (and the other 5 binaries)

Existing comparison-corpus ELFs reused.

## Cross-references

- `llvm-z80/tasks/3-pair-set-ldir-djnz-baseline-2026-06-21.md`
  -- Phase A (production audit)
- `llvm-z80/tasks/3-pair-set-ldir-djnz-aes-audit-2026-06-21.md`
  -- AES audit
- `llvm-z80/tasks/issue115-iy-unreserve-investigation-2026-06-21.md`
  -- parked HLReg design sketch (basis for the upcoming Z work)
- ravn/llvm-z80#7 -- meta tracker; all subtasks effectively delivered
  per this audit
- ravn/llvm-z80#111 -- HLReg for i16-counter pointer-arg (Z target)
- ravn/llvm-z80#115 -- HLReg/DEReg for IY-extraction (parked; Z
  partial-closes)
- `tasks/memory/feedback_production_hard_aes_soft.md`
- `tasks/memory/feedback_bsd_awk_only_on_macos.md`
  (new this round, after a `strtonum` failure)
