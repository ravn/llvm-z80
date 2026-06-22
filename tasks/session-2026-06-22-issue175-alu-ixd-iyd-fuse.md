# Session 2026-06-22 — Issue #175 (partial): fuse LD r,(IX/IY+d) + per-reg ALU

## TL;DR

Added a Z80LateOptimization post-RA peephole that fuses the post-PEI
spill-reload + register-ALU sequence

    LD <r>,(IX+d) ; <ALU>_<r>     // 4 B / 23 ts

into the single fused indexed-ALU form

    <ALU> a,(IX+d)                // 3 B / 19 ts

when `<r>` is dead after the ALU.  Mirror for `(IY+d)`.  Covers
`XOR/OR/AND/SUB/CP` × `B/C/D/E/H/L`.

**Yield (AES corpus, IX-frame configs 12 & 13):** **−8 B / −1484 ts each**.
8 fusion sites fire across AES; 4 candidates remain unfused (2 are
`LD A,(IX+d); ADD A,A` which would XOR/AND/etc against (IX+d) with itself
when dst=A — peephole correctly skips; 2 are `LD r,(IX+d); XOR r` where
`r` is still live after the ALU per `computeRegisterLiveness`).

**Production:** byte-identical (4 production targets all use `+static-stack`
which never enters IX-frame mode — no fusion sites exist).  Confirmed
on autoload PROM (2-byte timestamp delta only) and rcbios BIOS (5462 B,
matches baseline exactly).

## Why a peephole, not ISel

The original #175 proposal had ISel-time matching of `G_LOAD` whose
address is `G_PTR_ADD(COPY $ix, G_CONSTANT)`.  Empirical sweep showed
the AES corpus DOES NOT produce that shape: spill reloads come from
`G_FRAME_INDEX`, which goes through `LOAD_IDX8` pseudos that
`Z80ExpandPseudo` later rewrites to `LD_<r>_(IX+d)` AFTER register
allocation.  At ISel time, only `XOR_r %vreg` exists; the IX-relative
load isn't there yet.

Z80LateOptimization runs after `ExpandPostRAPseudos`, so it sees the
final per-register `XOR_<reg>` + `LD_<reg>_IXd` pair and can fuse them.

The peephole-only design also keeps the change localized and easy to
revert; the ISel layer is untouched.

## What didn't fuse and why

Of 12 candidate sites in AES:

- **8 XOR** sites fused (the −8 B / −1484 ts win).
- **2 `LD A,(IX+d); ADD A,A`** — dst is A; my peephole skips dst=A
  because `A ^ A` would be `0`, not the desired `A := A^mem`.  Adding
  ADD/ADC/SBC support would not help these because they load INTO A.
- **2 `LD r,(IX+d); XOR r`** where `r` is live after the XOR (used by
  a later instruction in the same loop body) — `computeRegisterLiveness`
  correctly returns `LQR_Live`, peephole refuses fold.

## Files changed

```
llvm/lib/Target/Z80/Z80LateOptimization.cpp       | +119 (peephole block)
llvm/test/CodeGen/Z80/issue-175-alu-ixd.mir       | new (7 cases: 5 GREEN + 2 negative)
```

## Verification

- **Lit:** 154 PASS + 6 XFAIL (Z80/), new `issue-175-alu-ixd.mir`
  PASSES.
- **test-runner clang:** 866/0/0/256 (PASS/FAIL/FATAL/SKIP).
- **AES sweep:** configs 1-11 byte-identical, configs 12 & 13 each
  −8 B aes_text / −1484 ts.  All 13 PASS verifier.
- **Production:** autoload PROM byte-identical (2-byte timestamp delta);
  rcbios BIOS 5462 B (baseline match).

## Cross-references

- Issue: ravn/llvm-z80#175 — HL-indirect half shipped 2026-06-09; this
  closes the IX/IY-indexed half.  Originally a `wontfix` candidate per
  the 2026-06-09 comment (motivating IX-frame mode lost its use case
  when #40 closed); user requested implementation anyway for the AES
  corpus configs 12/13 yield.
- Related TD definitions: `Z80InstrInfo.td` lines 378-454 (already had
  all 16 needed opcodes); selector wasn't using them post-RA.
- AES corpus baseline writeup: `rc700-gensmedet/tasks/aes256-corpus/`.
