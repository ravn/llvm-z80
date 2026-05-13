# Session 65 (2026-05-13) — #149 i16 != -1 → INC+OR-bytes lowering

## Context

Fifth closure from the corpus-mining batch.  #149: `icmp {eq,ne} i16
r, -1` (where -1 ≡ 0xFFFF for i16) was lowering through the XOR-based
EQ/NE path (`ld a,hi; cpl; ld tmp,a; ld a,lo; cpl; or tmp; jr cc`,
8 B) when the equivalent `(r+1) {==,!=} 0` test via `INC rr` plus
byte-OR is 5 B.

## Implementation

In `Z80InstructionSelector.cpp`, added a special-case at the entry
of the i16 EQ/NE path: when one operand is constant 0xFFFF AND the
other has **a single non-debug use**, emit:

```
COPY <Tmp:gr16>, <var>      ; coalesced into <var>'s reg by regalloc
INC16 <Tmp>                  ; in-place; clobbers <var>'s reg
COPY $a, <Tmp>:sub_lo
COPY <HiReg:gr8>, <Tmp>:sub_hi
OR <HiReg>                    ; sets Z iff Tmp == 0 iff var was 0xFFFF
J{Z,NZ} <target>
```

Post-RA: ~5 B (`inc de; ld a,e; or d; jr cc`).

## Critical safety guard: single-use only

Initial implementation didn't gate on use count — INC rr mutates
the source register.  cpnos-rom's `_snios_rcvmsg_c` has the
multi-use shape:
```c
r = recv_byte_t();
if (r != TRANSPORT_TIMEOUT) goto got_first_ack;    // first use
... got_first_ack: ...
if (((uint8_t)r & 0x7F) == ENQ) ...                 // second use
```
Without the single-use check, INC DE clobbered r so the
`(r & 0x7F) == ENQ` check saw r+1.  Boot broke.

Added `MRI.hasOneNonDBGUse(...)` check.  Multi-use bails to the
existing XOR/CPL path (8 B).

## Result

cpnos-rom production: no fires (all witnesses are multi-use).  The
fix is general-purpose; lit fixture demonstrates single-use cases
fire correctly with 5-byte sequence.

| | Pre-#149 | Post-#149 |
|---|---:|---:|
| Per-occurrence (single-use) | 8 B | 5 B (−3 B) |
| Per-occurrence (multi-use) | 8 B | 8 B (bails to safe path) |
| cpnos payload | 1860 B | **1860 B** (no fire — witness is multi-use) |

## Verification

  - **lit suite**: 100/100 (98 PASS + 2 XFAIL).  New fixture
    `issue-149-i16-ne-minus-one.ll` covers 4 cases:
      - single-use NE  (fires)
      - single-use EQ  (fires)
      - multi-use NE   (bails to XOR/CPL — verified by `CHECK: cpl`)
      - negative K = 0xFFFE (no fold — wrong constant)
  - **z80-utils test-runner** clang Oz: 113 PASS / 0 FAIL.
  - **cpnos-rom payload**: 1860 B (unchanged — fix bails on multi-use).
  - **cpnos-polypascal-test**: PASS on both pio-irq and sio cells.

## False-alarm bisect during verification (memory rule added)

Initial polypascal-test runs failed at stage 1 with "FAIL: timeout
waiting for E> boot prompt".  Reproducible across pio-irq and sio.
Looked exactly like a real codegen regression.

Bisect path:
  1. Bailed the i16-minus-one optimisation entirely → still failed.
  2. Confirmed cpnos binary was 1860 B (same as pre-#149).
  3. Realised: if my fix bails completely, cpnos should be
     byte-identical — yet it's failing.
  4. Checked daemon state: `nc -z 127.0.0.1 4002` showed port
     closed, but the test had logged that MP/M was started.
  5. Ran `make _kill-mpm; sleep 2; retry` → **PASS**.

Was a stale MP/M daemon from an earlier test session that
the Makefile's `_kill-mpm` target didn't fully clean.

Added memory rule `feedback_polypascal_stage1_flake.md` to catch
this pattern in future sessions before wasting bisect cycles on
codegen.

## Files touched

  - `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` — i16 EQ/NE
    -1 special case at entry of the EQ/NE branch.
  - `llvm/test/CodeGen/Z80/issue-149-i16-ne-minus-one.ll` — new
    lit fixture (4 cases).
  - `tasks/session65-149-i16-ne-minus-one.md` — this doc.
  - `memory/feedback_polypascal_stage1_flake.md` — new rule.
  - `memory/MEMORY.md` — indexed the new rule under §6.

## Ninja rebuild tracking

  | # | Trigger | Files |
  |---:|---|---:|
  | 1 | First version of #149 (no use-count gate) | 5 |
  | 2 | Add single-use gate | 5 |

Total this session: 2× 5-file incrementals.  Normal pattern,
no full-rebuild events.

## Cumulative state

  - Session 61 closed #141 — cpnos 1904 → 1878 B (−26 B)
  - Session 62 closed #142 — cpnos 1878 → 1866 B (−12 B)
  - Session 63 closed #144 — cpnos 1866 B unchanged (general C)
  - Session 64 closed #147 — cpnos 1866 → 1860 B (−6 B)
  - Session 65 closed #149 — cpnos 1860 B unchanged (multi-use bail)

**Cumulative cpnos savings: −44 B.**

Two consecutive closures (#144, #149) have 0 cpnos impact but
are correct general-purpose fixes.  Pattern: the corpus-mining
estimate (~30 B / ~5 B) was based on the C-level pattern count,
but cpnos's specific shapes don't always hit our safety
preconditions (e.g., multi-use, intervening readers).

5 closed.  9 still open from the original corpus (plus 3 follow-up
issues: #150, #151, #152).

## Rules-checked

  - `feedback_compiler_bug_test`: lit fixture demonstrates fix.
  - `feedback_test_before_fix`: test failed pre-fix, passes post-fix.
  - `feedback_no_commit_first_version`: full value-oracle satisfied
    (after the false-alarm bisect resolved as MP/M flake).
  - `feedback_value_oracle_all_transport_cells`: both pio-irq and
    sio polypascal cells PASS.
  - `feedback_polypascal_stage1_flake`: NEW — added this session
    after stage-1-fail false alarm.
  - `feedback_ninja_clang_llc_together`: rebuilt clang AND llc.
