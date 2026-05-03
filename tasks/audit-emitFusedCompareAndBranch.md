# Audit: `Z80InstructionSelector::emitFusedCompareAndBranch`

Closes ravn/llvm-z80#118.

`llvm/lib/Target/Z80/Z80InstructionSelector.cpp:703-1254` (~550
LOC).  Read-only audit of constant-RHS fold opportunities in the
ISel-time fused compare-and-branch helper, sibling to the
session-41 post-RA work on #116.

## Method

  - Read every branch of the function and catalog the shapes it
    handles, with byte cost where relevant.
  - Cross-reference against rcbios + cpnos-rom 2026-05-03
    disassemblies (`rc700-gensmedet/rcbios-in-c/bios.lis`,
    `rc700-gensmedet/cpnos-rom/clang/cpnos.lis`).
  - For every "missing" shape suggested in #118, either confirm
    a real fire site exists or note zero current sites.
  - Apply structural-first lens: anything better done in a
    GISel combiner / IR transform / regalloc layer is flagged
    as such, not as a peephole opportunity.

## Coverage today (what's already handled)

### Adjacent normalizations (entry-time, lines 716-836)

  - **zext/sext narrowing** (lines 716-748).  Both operands
    derived from same-size smaller type → compare narrowed.
    EQ/NE: always.  Unsigned: both zext.  Signed: both sext.
  - **`icmp eq/ne (add (zext i8), C), (zext i8)`** (lines 754-836)
    with overflow guard via `JP C` to fallthrough/target.
    Saves the 16-bit add path entirely when both operands fit
    in i8 + an i8 constant adder.

### i8 unsigned (lines 885-976)

| Shape | Emitted | Bytes | Note |
|---|---|---:|---|
| EQ/NE with 0 | `OR A` | 1 | line 912-914 |
| EQ/NE with const ≠ 0 / ULT/UGE with const | `CP n` | 2 | line 915-918 |
| Variable, RHS is single-use load | `CP (HL)` | 1 | lines 920-959 (HL constraint) |
| Variable / variable | `CP r` | 1 | line 967-975 |

### i8 signed (lines 977-1035)

| Shape | Emitted | Bytes | Note |
|---|---|---:|---|
| SLT/SGE 0 | `RLCA` + carry-branch | 1 | line 994-1002 |
| SGT X, -1  (= SLT -1, X) | `RLCA` + carry-branch | 1 | line 1003-1013 |
| Other constant RHS | `XOR 0x80; CP (C^0x80)` | 4 | line 1014-1021 |
| Variable / variable | `XOR 0x80; ...; XOR 0x80; CP r` | 7 | line 1022-1034 |

### i16 EQ/NE (lines 1038-1179)

| Shape | Emitted | Bytes | Note |
|---|---|---:|---|
| Z80 small-const RHS (0-255) + variable HighByteZero, C=0 | `LD A,L; OR A` | 2 | line 1078-1085 |
| Z80 small-const RHS (0-255) + variable HighByteZero, C≠0 | `LD A,L; CP n` | 3 | line 1078-1085 |
| Z80 small-const RHS (0-255), general (HighByteUnknown), C=0 | `LD A,L; OR H` | 2 | line 1086-1091 |
| Z80 small-const RHS (0-255), general, C≠0 | `LD A,L; SUB n; OR H` | 4 | line 1086-1091 |
| Z80 const RHS general (any 16-bit immediate) | `LD A,h; XOR Hi; LD t,A; LD A,l; XOR Lo; OR t` | 5–7 | line 1131-1146; XOR n omitted if byte=0 |
| Z80 variable / variable | XOR-byte chain via sub-register copies | 6+ | line 1148-1178 |
| SM83 RHS=0 | `SM83_CMP_ZERO16` pseudo | 3 | line 1101-1105 |
| SM83 variable / variable (or const) | `SM83_CMP_Z16` pseudo | 6 | line 1106-1114 |

### i16 signed (lines 1180-1239)

| Shape | Emitted | Bytes | Note |
|---|---|---:|---|
| SLT/SGE with RHS=0 | `LD A, hi; ADD A,A` (sign bit→carry) | 2 | line 1188-1201 |
| SLT/SGE with LHS=0  (after normalization SGT/SLE X,0) | non-neg-mask AND nonzero-test | 9 | line 1202-1229 |
| Generic | `emitSigned16BitCompare` helper + `OR A` | varies | line 1230-1238 |

### i16 unsigned ULT/UGE (line 1240-1246)

| Shape | Emitted | Bytes | Note |
|---|---|---:|---|
| Any (constant or variable RHS) | `CMP16_FLAGS` pseudo | 5–7 | line 1240-1245 |

### Width > 16

  - Returns `false` (delegates to non-fused emit32/emit64
    helpers).

## Gaps observed (ranked by structural fit + concrete fire sites)

### Gap 1 — i16 ULT/UGE with small-const RHS + provably-high-zero variable

**Shape:** `icmp ult i16 %x, C` where `C ≤ 255` and `%x` has its
high byte provably zero (e.g. zext from i8, computed via
`isHighByteProvablyZero`).

**Currently emitted:** falls to `CMP16_FLAGS` pseudo (5-7B
including the constant load into BC/DE).

**Could emit:** `LD A, l; CP C` (3B), branch on carry.  Same
shape as the EQ/NE small-const + HighByteZero case at line 1078
already does, just adapted for ULT/UGE.

**Savings per fire:** ~2-4 bytes.

**rcbios + cpnos-rom fire sites:**

  - rcbios disassembly: zero direct sites today.  Most i16
    ULT/UGE compares in BIOS use 16-bit values that genuinely
    need the full 16-bit compare (sector counts, addresses).
  - cpnos.bin: zero direct sites.

**ROI:** **Low.**  The pattern requires `zext i8 → i16` then
`ult const ≤ 255`, which is rare in C source.  More common is
the inverse: i8 variable compared to small constant directly,
already handled.

**Disposition:** **Skip / wontfix.**  Filed sibling tracking
issue #122 for completeness, but no plan to land.  The
infrastructure (`isHighByteProvablyZero`) already exists, so
the implementation cost is small (~30 LOC); the absence of fire
sites makes it not worth the maintenance.

### Gap 2 — i8 EQ/NE with constant 1 / 0xFF

**Shape:** `icmp eq i8 %x, 1` or `icmp eq i8 %x, 0xFF`.

**Currently emitted:** `CP 1` / `CP 255` (2B).

**Alternatives considered:**

  - `DEC A; OR A` (2B) for EQ 1.  Same byte count.  Clobbers A
    (CP doesn't).  **Net: not better.**
  - `INC A; OR A` (2B) for EQ 0xFF.  Same byte count.  Clobbers
    A.  **Net: not better.**

**rcbios fire sites:** 8 `CP $1` instances (e.g. `dc83`, `dcaf`,
`dd20`, etc.).  All EQ comparisons (followed by `JR Z`).

**Disposition:** **No-op.**  Not actually a savings opportunity.

### Gap 3 — Chained EQ/NE branches with shared LHS

**Shape:** `if (x == 1) ...; else if (x == 2) ...; else if
(x == 3) ...` — emitted as three independent compare-and-branch
sequences.

**Currently emitted:**

```asm
LD A, x      ; (1B per chain element — A reloaded each branch)
CP 1         ; 2B
JR Z, ...    ; 2B
LD A, x      ; redundant 1B reload
CP 2         ; 2B
JR Z, ...    ; 2B
...
```

The `LD A, x` reload is wasteful — A is set by the previous CP,
but the high-level structure of multiple branches makes ISel
emit each chain element independently.

**Currently mitigated:** post-RA peephole pattern #39 in the
late-opt audit ("Cross-block redundant `LD A,r` removal,
dataflow") removes some of these reloads after regalloc.

**rcbios fire sites:** 3 confirmed switch-style chains in
`bios.lis` around `dd1c`, `e120`, `e2f5`.  Each chain has ~5
elements; the post-RA peephole already removes the inner
reloads.

**Disposition:** **Already covered at post-RA.**  No ISel-time
work needed.  Verified the late-opt audit pattern #39 fires on
the existing chains.

### Gap 4 — i8/i16 EQ/NE with constant 0 already optimized, no second pass needed

**Shape:** any `icmp eq/ne %x, 0` already routes through the
fast path (`OR A` for i8, `LD A,L; OR H` for i16).  No gap.

### Gap 5 — Sign-bit comparisons via constant shifts

**Shape:** `if ((x >> 7) & 1)` on i8 — equivalent to sign-bit
test.

**Currently:** handled by the existing SLT/SGE 0 fold (line
994-1002).  Combiner / earlier IR transformations normalize the
shift+and into icmp.

**Disposition:** **Already covered.**

### Gap 6 — i16 ULT/UGE with constant where variable is genuinely 16-bit

**Shape:** `icmp ult i16 %x, 100` where `%x` is a real 16-bit
value (no high-byte-zero proof).

**Currently:** `CMP16_FLAGS` with `LD DE/BC, 100` constant load
(7-8B total).

**Alternatives considered:**

  - For very small const (≤ 255) and unknown-high variable, a
    branch-cascading pattern (`LD A, hi; OR A; JP NZ taken;
    LD A, lo; CP C; JP C/NC ...`) could save 2-3B but introduces
    multi-block branch.

**rcbios fire sites:** ~5 `SBC HL, BC/DE` sites that suggest
i16 ULT/UGE compares.  Each is a genuine 16-bit comparison
(track / sector / address); the constant is rarely small.

**Disposition:** **Skip.**  Multi-block branch is structurally
worse for LLVM's CFG, and the savings are marginal on real
sites.

## Bigger picture: structural fit

The function already handles the common constant-RHS shapes
well, and the few gaps either:

  - have no fire sites in real code (Gap 1, 6),
  - are not actually savings (Gap 2),
  - or are already mitigated at the post-RA layer (Gap 3, 5).

Per the structural-first principle, **no new ISel-time fold is
warranted at this time.**  The function is in a good local
optimum; further gains require structural moves (combiners,
regalloc, IR-level transforms) rather than more ISel branches.

The ISel function is also already large at ~550 LOC.  Adding
another conditional branch for a marginal savings would push
it toward unmanageable.  The structural lens prefers
declarative TableGen patterns or post-RA peepholes (when
they're correct invariants of regalloc state) over conditional
branches in the manually-written ISel helper.

## Concrete follow-ups filed

  - **#122** — i16 ULT/UGE with small-const + provably-high-
    zero variable.  Filed for tracking, not for current-cycle
    work.  Closes Gap 1.

No other follow-ups warranted by this audit.

## Cross-references

  - `tasks/late-opt-audit-2026-05-02.md` — post-RA peephole
    catalog (pattern #39 covers Gap 3 above).
  - `tasks/session41-summary.md` Carry-forward — original
    audit park; cited line 3205 was a different fold (G_OR/G_XOR
    `OR/XOR (HL)` load fusion), not the compare-and-branch
    function — that prior reference is corrected here.
  - `llvm/test/CodeGen/Z80/issue-116-i16-eqne-sbc-hl.ll` —
    sibling test on the post-RA path.
  - rcbios disassembly: `rc700-gensmedet/rcbios-in-c/bios.lis`
    @ 5929 B as of 2026-05-03 evening.
  - cpnos-rom disassembly:
    `rc700-gensmedet/cpnos-rom/clang/cpnos.lis` @ 1777 B
    payload.
