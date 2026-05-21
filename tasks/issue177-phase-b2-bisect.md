# Phase B2: bisect of the Phase-B bundle, ship clean cases, file the bug

Date: 2026-05-22 (session 73p Phase 2, follow-up to Phase B1).

## User redirect

> "i am fine with you accidentially introducing bugs, as long as you
> fix them correctly."

Phase B1 had parked #177 on discovering the bundle introduced
miscompiles.  Wrong reflex — the right move is to bisect the
bundle, ship the clean cases, and file the broken case with a
reproducer.

## Bisect sequence

Each step applied ONE component of the original bundle in isolation,
rebuilt clang+llc, ran the AES corpus.  Baseline = HEAD pre-bundle
(prefersVectorizedAddressing=false only).

| Bisect | Override | 05_Oz_static_stack | 09_Oz_prod_like | Other |
|---|---|---|---|---|
| A | `getCastInstrCost` (3 cases) | 2630 PASS | 2574 PASS | 04_O2 +113 B |
| B | `getArithmeticInstrCost` (all) | **2684 FAIL @ 100M ts** | 2618 +44 B | 02_Os/04_O2 ts=28 silent |
| B1 | -- only Mul -> Expensive | 2630 PASS | 2574 PASS | 04_O2 -144 B |
| B2 | -- only i16 -> 2 | **2684 FAIL @ 100M ts** | 2618 +44 B | full repro of B's pattern |

Conclusion: the **`i16 -> cost 2`** charge in `getArithmeticInstrCost`
is the sole cause of the bundle's miscompile.  Cast hooks (A),
Mul -> Expensive (B1), and getCmpSelInstrCost (untested but ruled
out by elimination -- B's pattern fully accounted for by B2) are all
clean.

## What the miscompile looks like at the asm level

The `i16 -> 2` charge causes IndVarSimplify (or similar) to narrow
the loop counter in `_aes_subBytes` from i16 to i8:

```
;;; baseline (.bss-free, push/pop save-restore across call)
ld   bc, $f       ; 16-bit counter
.loop:
ld   a, b
cpl
ld   d, a
ld   a, c
cpl
or   d            ; BC == 0xFFFF (i.e., decrement wrapped)?
ret  z
ld   d, 0
ld   e, c
ld   hl, ($0)     ; reloc: _gf_alog (i.e., function param)
add  hl, de
push hl           ; stack-save address across call
ld   l, (hl)
ld   h, 0
push bc
call _rj_sbox
pop  bc
pop  de
ld   (de), a
dec  bc
jr   .loop
```

```
;;; buggy (i16->2 narrows the IV to i8, BSS-spill for address)
ld   ($0), hl     ; reloc: .bss+0 -- save param to BSS
ld   c, $f        ; 8-bit counter
.loop:
ld   a, c
inc  a            ; C == 0xFF?  (i.e., -1 unsigned)
ret  z
ld   e, c
ld   d, 0
ld   hl, ($0)     ; reload param from BSS
add  hl, de
ld   ($0), hl     ; reloc: .bss+3 -- BSS-save address (NOT push/pop)
ld   l, (hl)
ld   h, 0
push af
call _rj_sbox
ld   de, ($0)     ; reload address from .bss+3
ld   (de), a
pop  af
dec  a
ld   c, a
jr   .loop
```

Both loops run 16 iterations.  Loop control is logically equivalent.
The BSS slots `.bss+0` (param) and `.bss+3` (saved address) don't
alias and `_rj_sbox` doesn't touch BSS.  So at the asm level, the
buggy version LOOKS correct.

## Why it actually fails at runtime

Open question — the asm-level scan above doesn't reveal the runtime
failure mode.  Hypotheses to investigate:

1. **BSS slot allocation across the whole module**: when i16->2
   changes IV widths globally, the `+static-stack` BSS allocator
   may pick slots that collide between functions in the same call
   chain.  The diff was only on `_aes_subBytes` -- but other
   functions (especially `_aes_expandEncKey`, `_aes_mixColumns`)
   would need separate inspection.
2. **Calling-convention violation**: with the narrow IV, calls
   may rely on a specific register being live across the call but
   `_rj_sbox` may not preserve it on a different IR shape.
3. **PHI handling**: the narrower IV may have introduced a PHI that
   regalloc handled correctly per-MBB but produces wrong cross-MBB
   value flow on `+static-stack`.

The 100M-tstate timeout strongly suggests an infinite loop somewhere
**other** than `_aes_subBytes` (whose loop control matches baseline).

## What ships in Phase B (post-bisect)

1. `getCastInstrCost` (full 3 cases as drafted in B1).
2. `getArithmeticInstrCost` with **only `Mul -> TCC_Expensive`**.

The full Phase-B bundle's i16/i32/i64 width logic is REMOVED.

## What's filed for later

**ravn/llvm-z80#184** (to be filed): "`getArithmeticInstrCost` returning
2 for i16 type causes miscompile under `+static-stack` (AES corpus
`05_Oz_static_stack` infinite loop)."  Reproducer: apply the i16=2
override in `Z80TargetTransformInfo.h`, build AES corpus, run
`05_Oz_static_stack`.  Asm diff included.

## Decision E gate (with Mul + Cast only)

- Lit Z80 suite: 105 PASS + 3 XFAIL (unchanged).
- AES corpus 13/13 PASS, no regressions, `04_O2` -25 B.
- Wider oracle (sieve/fannkuch/pi): byte-identical to baseline.
- cpnos PROM1 (clang): **2029 B / 2048 B (19 B free)** — **−1 B** vs
  prior 2030 B.  Production target byte-improvement.

## Methodological note

Phase B1 had me declaring "park #177" after seeing the miscompile.
That was the wrong call: a bisect across the three hooks takes ~15
minutes per cycle (build + AES sweep) and turns "the bundle is
broken" into "exactly which line of the bundle is broken."

Phase A's discipline ("validate before committing to multi-week
work") was the right one to apply HERE -- not "park on first
failure," but "bisect on first failure."  Closer kin to
[[feedback_no_commit_first_version]] than to [[feedback_state_certainty]].

## Cross-references

- Phase A: `issue177-phase-a-investigation.md`
- Phase B0 (wrong prediction): `issue177-phase-b0-investigation.md`
- Phase B1 (premature parking): `issue177-phase-b1-finding.md`
- Phase B2 (this doc): bisect + ship clean cases + file #184.
