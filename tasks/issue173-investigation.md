# Issue #173 investigation: bare BSS store + 4-instr A-preserving reload

Date: 2026-05-22 (session 73p Phase 2, follow-up to #177 partial ship).

## The pattern, real-world census

Catalog of `push af` instances in AES `aes256.c` at `05_Oz_static_stack`,
extracted programmatically:

| Pattern                                                  | Count |
|----------------------------------------------------------|------:|
| 4-instr SPILL  (`push af; ld a, R; ld (sfr), a; pop af`) |     1 |
| 4-instr RELOAD (`push af; ld a, (sfr); ld R, a; pop af`) |    16 |
| BARE   reload  (`ld a, (sfr)`)                            |    19 |
| BARE   store   (`ld (sfr), a`)                            |    20 |

So the dominant shape is **bare store + 4-instr reload**, not the
symmetric 4+4 case I first MVP'd.  The bare store happens because
A already held the spilled value (no transit-via-A needed); the
4-instr reload happens because A is live with a different value
when we need to retrieve the spilled byte.

## False stop #1: MVP scope was wrong

First MVP detected only the symmetric 4+4 same-MBB pattern.  AES
sweep showed byte-identical to baseline.  Initial conclusion (wrong):
"Path A as scoped doesn't fire on real AES; lower yield than
estimated; abandoning."

User redirect: "reinvestigate thoroughly."

## Pairing the pattern

Programmatic pairing of bare stores -> 4-instr reload templates with
matching slot, per-function, with conflict and CALL flagging:

| Function           | Pairs | Clean | CALL  | No-CALL | Conflict |
|--------------------|------:|------:|------:|--------:|---------:|
| `aes_subBytes`     |     1 |     1 |     1 |       0 |        0 |
| `aes_sb_inv`       |     1 |     1 |     1 |       0 |        0 |
| `aes_mixColumns`   |     2 |     2 |     0 |       2 |        0 |
| `aes_mc_inv`       |     3 |     3 |     0 |       3 |        0 |
| `aes_expDecKey`    |     4 |     1 |     1 |       0 |        3 |

11 pairs total, 8 of which are "clean" (no other slot accesses in
the gap).  Yield estimate: 8 × 6 B = 48 B if all converted.

## False stop #2: bailing everywhere on partner-defined

First fully-implemented peephole bailed on every single store.
Tracing showed `aes_subBytes`/`aes_sb_inv` bailed on "partner B is
defined between."  But the asm has no explicit `ld b, ...` between
the store and reload.

Root cause: the CALL instruction in MIR has implicit defs for all
caller-saved registers (B included).  My `recordDefs` lambda
iterated `MI.operands()` checking `MO.isReg() && MO.isDef()` --
which IS true for implicit-def operands on CALL.

Fix: exclude CALL clobbers AND implicit defs from the partner-defined
check.  Caller-saved values across a CALL are not architecturally
preserved anyway, so our PUSH/POP restoring those values is a
strict improvement (or no-op) over the ABI guarantee.

After this fix: `aes_subBytes` and `aes_sb_inv` fire; mixColumns
and mc_inv still bail because the slot IS accessed elsewhere
(those have multi-reload patterns the existing across-CALL peephole
already handles).

## Implementation: two-phase scan

Phase 1 -- bare store -> matched 4-instr reload template.
Phase 2 -- from after the template, scan forward until stack depth
returns to 0 (excluding the matched template's own PUSH/POP_AF
which will be deleted).  Insert POP rr at that point.

The two-phase tracking handles the common case where the matched
reload sits INSIDE another PUSH/POP bracket (e.g., the existing
across-CALL cross-class peephole that wraps buffer-address
arithmetic in `aes_subBytes`).

## Production yield (measured)

| Target                            | Before | After  | Delta  |
|-----------------------------------|-------:|-------:|-------:|
| AES `09_Oz_prod_like` (bin)       |  2574 |   2562 |   -12 |
| AES `09_Oz_prod_like` (tstates)   | 10.75M | 10.74M |  -0.11% |
| AES `05_Oz_static_stack`          |  2630 |   2630 |     0 |
| Other AES configs                 |   (n/a, peephole gated on +static-stack) | | |
| Wider oracle (sieve/fannkuch/pi)  | (byte-identical) | | |
| cpnos PROM1 (clang)               |  2029 |   2028 |    -1 (20 B free) |

12 B / 12.08 M ts AES production win + 1 B cpnos PROM1.

## What didn't catch what -- and why

The peephole fires on `aes_subBytes` and `aes_sb_inv` (both have
a single bare-store + single 4-instr-reload + CALL between).  It
correctly bails on:

- `aes_shiftRows`, `aes_sr_inv`: slot accessed multiple times between
  store and reload (multi-store or multi-reload pattern).
- `aes_mixColumns`, `aes_mc_inv`: slot accessed by an intermediate
  store/load to the same slot (real conflict).
- `aes_expandEncKey`, `aes_expDecKey`: bare store has no matching
  4-instr-reload template in the same MBB (cross-MBB needed).

Future extensions to catch these patterns:

1. **Multi-reload case**: extend to N matched reloads per single
   store, mirror the existing across-CALL peephole's "push after
   each pop except the last" approach.  Yield: maybe 2-4 B per
   additional reload site.

2. **Cross-MBB case**: model after `Z80LateOptimization`'s existing
   cross-MBB BSS-spill peephole (#132).  Substantial work; the
   yield estimate from the catalog suggests ~6-12 B per cross-MBB
   pair.

3. **Mirror pattern (4-instr-spill + bare-reload)**: pattern exists
   in synthesized code but appears less often in real AES.  Could
   add a sibling peephole.

## Methodological lessons

1. **Catalog before implementing**.  The 11-pair catalog tells me
   what to optimize and what to bail on; the abstract issue text
   (100-200 B estimate) was an order of magnitude high because
   it assumed every push-af is a convertible pair.

2. **Don't conflate CALL clobbers with semantic definitions**.  CALL
   implicit defs are clobbers, not value-producing definitions.
   Exclude them from "register has been modified" checks.

3. **Two-phase stack tracking**.  Real-world spill+reload patterns
   are nested inside other stack manipulations (push hl/pop de
   bracketing the call).  Single-pass scan that requires "balanced
   at reload" misses these; two-phase scan that finds the next
   balanced point catches them.

4. **The user redirect was load-bearing**.  My first abandoned
   conclusion ("Path A doesn't fire") was wrong.  The catalog
   showed 8 firable pairs.  Bisect-style depth pays off.

## Status

Issue #173 partially closed:
- Path A (bare-store + 4-instr-reload + same MBB): **shipped**.
- Path A extensions (multi-reload, cross-MBB, mirror pattern): open.

Next-yield levers if this work continues:
- Multi-reload extension (~10-30 B AES).
- Cross-MBB extension (~10-30 B AES).
- Mirror pattern (4-instr-spill + bare-reload, ~5-15 B).
