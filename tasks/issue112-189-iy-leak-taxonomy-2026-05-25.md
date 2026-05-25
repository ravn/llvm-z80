# IX/IY-unreserve leak taxonomy & invariant analysis (#112 / #189 / #27)

**2026-05-25.**  Step-back after fixing two #189 leaks in a row exposed a third —
the classic whack-a-mole signature.  This sweep replaces "patch the next leak"
with "characterize the whole class and state the invariant."  Companion to
`issue189-27-regalloc-cost-model-drill-2026-05-25.md` (the mechanism writeup).

## The invariant un-reserving IX/IY requires

> A 16-bit value that is ever accessed a byte at a time must never be allocated
> to IX/IY.

Rationale: IX/IY's 8-bit halves (`IXL`/`IXH`, `IYL`/`IYH`) have **no documented
ops**.  So a byte access of an IX/IY-resident value can only become either
(a) an **undocumented** `xor iyh` etc. (illegal without `+undocumented`), or
(b) a `push/pop` **shuttle** through a GP pair — which is documented and
value-correct as a whole-pair move, but is pure density cost, and historically
(pre-fix) corrupted in the default config when the shuttle perturbed SP under
SP-relative spill-slot addressing (#189).

The backend expresses this invariant with the `GR16NoIR` register class
(= GR16 minus IX/IY).  The leaks below are all places where a byte-accessible
value is *not* in `GR16NoIR` and therefore reaches IX/IY.

## Method

Compiled every `llvm/test/CodeGen/Z80/*.ll` with `-mtriple=z80 -O2
-z80-unreserve-iy` (NO `+undocumented`, NO `+static-stack` — the default
IX-frame config, where #189 miscompiled).  Because `+undocumented` is off,
`IXL/IXH/IYL/IYH` are reserved, so **any `iyh`/`iyl` operand op in the output is
an undocumented-emission bug**; `push/pop iy` is documented and may be legit.

## Findings (post-fix: getLargest gate + Z80NarrowNoIndex pass)

Sweep over 22 files that touch IY under un-reserve:

| Class | What | Count after fix | Status |
|-------|------|-----------------|--------|
| **A** | Undocumented `IYH`/`IYL` operand ops (no `+undocumented`) | **0 across all 22 files** | **FIXED** |
| **B** | #189 SP-perturbation **miscompile** (byte-shuttle corrupts SP-relative slots) | 0 at tested witnesses | **FIXED + verified** |
| **C** | Value-correct `push/pop iy` **shuttles** (IY used as extra pair, byte-accessed via GP copy) | ~200 occ., ~20 files | **residual, density only** |

### Class A — undocumented emission: ELIMINATED suite-wide

Pre-fix, `popcount32` emitted `ld iy,0; xor iyh; xor iyl` (undocumented, and a
pointless `xor 0`).  Post-fix the undocumented-op count is **0 in every file**
(`xor/or/and/add/.../ld <iyh|iyl|ixh|ixl>` grep = 0).  This was the dangerous
class (illegal instructions silently emitted) and it is gone.

### Class B — #189 correctness: FIXED and verified in the miscompile-prone config

The two fixes keep byte-decomposed `GR16NoIR` values out of IY through
allocation, so the corrupting shuttle does not form.  Runtime witnesses in the
default (no `+static-stack`) config under `-z80-unreserve-iy`, all opt levels:
- `test_171` (i32 crc_one): `0x0044` (pre-fix) -> **`0xEF8D`** (correct).
- `test_172` (i64 reduction, heavy IY shuttling): **`0x7315`** (host-verified),
  all 6 opt levels.

Two diverse wide-integer witnesses pass in the exact config where #189 corrupted.

### Class C — residual shuttles: value-correct, density only (Phase-3 cost model)

`push/pop iy` remains in wide-integer / float code (`i128-support` 92,
`fixed-point` 21, `fcmp` 16, `spill-regclass` 13, `i64-support` 10, ...).  These
are **not** whole-pair `push iy; pop iy` (0 of those) — they are COPY16_PUSHPOP
moves between IY and a GP pair (e.g. `push iy; pop hl`).  IY is being used as a
4th/5th register pair for values the fixes did not route to `GR16NoIR`; when
such a value is byte-accessed, it is first moved whole-pair to HL/DE/BC (correct)
and decomposed there.  This is the **density face of #189** (the shuttle costs
bytes) and is the register-allocation **cost-model** question (is the extra pair
worth the shuttle?), not a correctness or legality blocker.

## The two fixes, and what they do / don't enforce

1. **`getLargestLegalSuperClass(GR16NoIR)` no longer re-widens to GR16** (gated on
   `-z80-unreserve-iy`).  The grow step in `recomputeRegClass`/splitting was
   undoing the `GR16NoIR` exclusion during coalescing.  Fixes the i32 crc case;
   production byte-identical (flag off -> original path).
2. **`Z80NarrowNoIndex` pre-RA pass** narrows a plain-`GR16` vreg to `GR16NoIR`
   when it is byte-decomposed (`sub_lo`/`sub_hi`) **or** used where `GR16NoIR` is
   required (e.g. a rematerialized `LD_r16_nn` constant feeding `XOR_CMP_EQ16`).
   Fixes the popcount32 undocumented-emission case.

Both are load-bearing and complementary (the pass establishes `GR16NoIR`; the
gate keeps it through allocation).  Together they **neutralize the consequences**
of the leak (no undocumented ops, no miscompile) but do **not** fully enforce the
invariant: Class C shows byte-accessible values still reach IY via origins
neither fix tracks (whole-pair at definition, byte-accessed only after a COPY
chain — not visible as a `sub_lo`/`sub_hi` operand or a `GR16NoIR`-typed use on
the IY-resident vreg itself).

## Where the invariant should ultimately live (the chokepoint)

To *fully* enforce it (and recover the Class-C density), the structural fix is to
make a 16-bit value `GR16NoIR` **at creation** whenever it can be byte-accessed,
rather than narrowing reactively pre-RA.  Candidate chokepoints, in order of
preference:
- **Legalizer / RegBankSelect:** when a 16-bit value is produced/consumed by any
  byte-wise operation (the legalizer already splits i16 logic into i8 lanes via
  UNMERGE/MERGE), give the merged/PHI/REG_SEQUENCE result `GR16NoIR`, so GR16
  (IX/IY-eligible) is reserved for values provably always whole-pair (pointers,
  ADD16 accumulators, whole-pair copies).
- Failing that, a **rematerialization/spill guard**: never remat or spill a
  value into IX/IY when any use constrains it to `GR16NoIR` (closes the Class-C
  origins the pre-RA pass misses).

The alternative is the Phase-3 cost-model conclusion: keep IX/IY out of the
general 16-bit pool and make them opt-in only where measured-beneficial (close to
the status quo of reserving them).

## Status / recommendation

- **Correctness + legality of un-reserve-IY: substantially achieved** across the
  swept suite by the two fixes (Class A eliminated; Class B fixed + verified on
  i32 and i64 in the miscompile config).  These were the hard gates.
- **Residual (Class C): density only**, the cost-model question — not a blocker.
- **Verification gap before actually flipping `-z80-unreserve-iy` default:**
  runtime-verify Class C correctness on `i128` / `fixed-point` / `fcmp` (only
  i32 + i64 verified here); the mechanism argument covers them but they are not
  yet runtime-tested.
- The two fixes are **production-safe** (gated; production byte-identical) and a
  correct, isolated increment.  They are worth landing on their own merits and
  worth presenting to @zlfn as the concrete first step of un-reserving IX/IY,
  with this taxonomy as the map of what remains (Class C density + the
  create-time `GR16NoIR` chokepoint).
