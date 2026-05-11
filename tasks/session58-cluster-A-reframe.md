# Session 58 (2026-05-11) — Cluster A reframe after #128/#129/#131

## Context

Session 42's structural plan recommended a multi-session cost-model
investigation on #89 + #27 as the next Phase 3 deliverable — both
simple paths on #89 were ruled out empirically (see
`tasks/issue-89-investigation-2026-05-03.md`).  Between sessions 42
and 58, three new issues were filed on 2026-05-10 that target the
same underlying gap from different angles:

| Issue | Mechanism | Status | Estimated win |
|---|---|---|---|
| #128 | Disable MachineLICM + MachineCSE at `-Oz` | already wired into `rc700-gensmedet/cpnos-rom/Makefile` (Phase 62/63) | −141 B on `snios_c.o`, −134 B on cpnos payload |
| #129 | Late-MIR peephole: `STORE_BSS rr; CALL; LOAD_BSS rr` → `PUSH rr; CALL; POP rr` | **in-MBB form fully implemented** (Z80LateOptimization.cpp:4859 same-class since 2026-03-27; :5104 cross-class as of fc34593, current HEAD); residual is cross-MBB | exhaustive on in-MBB (zero residuals in cpnos.lis); cross-MBB extension blocked by SP-balance correctness — see "Correction" below |
| #131 | New `__attribute__((z80_preserves_regs(...)))` clang analog of SDCC's `__preserves_regs` | open, unimplemented | 70–110 B on SNIOS clang code, closes parity gap with SDCC |

## SNIOS-in-C as the concrete witness

Session 57's plain-C SNIOS port (`rc700-gensmedet/cpnos-rom/snios_c.c`,
17 functions, +426 B clang / +306 B SDCC over the asm version) is
now the densest evidence for this cluster.  Inspecting
`clang/cpnos.lis` for `_snios_sndmsg_force` (f1b9..f2c1, 265 B):

  - **Three inner loops** (`do { ... } while (--t)`, `do { ... } while
    (--i)`, `do { ... } while (k--)`) each containing one or two
    `xport_send_byte` / `recv_byte_t` CALLs.
  - Loop counters (`t`, `i`, `k`) spilled to BSS sframe slots —
    every iteration pays 3 B read + 3 B write through the slot
    instead of 1 B `dec r` + 1 B `djnz` or `dec r; jr nz`.
  - Running checksum `hcs` spilled to BSS — every `hcs += b` costs
    10 B (`push af; ld a,(sf+8); ld e,a; pop af; add a,e; ld (sf+8),a`)
    vs SDCC's 3 B (`ld a,e; add a,c; ld e,a`).

SDCC compiles the equivalent `try_send_frame` to ~119 B (counters
in `B`/`D`, checksum in `E`, pointer in `HL`/`BC`, push/pop the
clobbered pair around each CALL).  Per-loop gap: ~12–15 B × 3 loops
≈ the full 125 B function-level gap.

## Reframe

The original Phase 3 Cluster A framing (#89 + #27 + #38 carried,
"close one cluster to reach engagement-mode gate") is still valid,
but the **method** has shifted:

  - **#89's symptom** (loop-invariant DE reload) is partially
    mitigated by #128's LICM disable, since the in-loop remat
    Path 2 from session 42 traced back to LICM/coalescer interaction.
    The residual after #128 lives mostly in 8-bit counter+accumulator
    spill (the SNIOS pattern), which is #129's surface, not the
    cost-model surface session 42 was scoping.
  - **#27 (per-pair 16-bit copy cost)** has no concrete regression
    in the post-#57 SNIOS code either; the dominant cost is per-scalar
    8-bit spill cost, not per-pair copy cost.  #27 stays an RFE.
  - **#100 (rotation-around-CALL spill)** is the same family as
    #129 — both want to recover BSS-around-CALL into stack-around-CALL.
    #129 is the narrower, more concrete version.

## Correction (same session, after attempting implementation)

The above table's "open, unimplemented" claim for #129 was wrong.
On opening `Z80LateOptimization.cpp`:

  - **Lines 4859–5102** — same-class BSS-spill→PUSH/POP around CALL.
    Landed 2026-03-27 (commit `0c74b562655d`).  Handles AF, BC, DE,
    HL.  Includes the cross-MBB conservative bailout, LIFO stack-
    depth tracking, MCSymbol-offset disambiguation, orphan-load
    detection (#82 lesson), POP AF flag-safety check, and multi-load
    re-PUSH support.
  - **Lines 5104–5278** — cross-class BSS-spill→PUSH/POP (HL→DE,
    DE→HL, etc.).  Landed in commit `fc34593` (current llvm-z80
    HEAD, the most recent commit).

An awk scan of `cpnos.lis` for the in-MBB pattern (store, CALL,
matching load with no intervening terminator) finds **zero
residuals** in both 8-bit and 16-bit cases.  The in-MBB form of
#129 is exhaustively implemented.

## The actual residual: cross-MBB BSS-spill

The SNIOS code surfaces the cross-MBB shape:

```
MBB_A:                                    ; loop top + spill + call
  ...
  ld (sframe+8), a       ; STORE
  call _xport_recv_byte  ; CALL
  ld a,d; cpl; ld d,a; ...; or d
  jr nz, MBB_C            ; conditional exit → bypasses LOAD
  ; fallthrough to MBB_B

MBB_B:                                    ; reload + dec + back-edge
  ld a, (sframe+8)       ; LOAD (matches STORE)
  dec a
  ld d, a
  or a
  jr nz, MBB_A            ; loop back

MBB_C:                                    ; success path, slot dead
  ...
```

The existing peephole's scan stops at MBB_A's terminator (the
`jr nz, MBB_C`), so the matching LOAD in MBB_B is never seen.
A naïve cross-MBB extension that PUSHes in MBB_A and POPs in MBB_B
**breaks SP balance**: on the MBB_A → MBB_C escape edge, the
pushed value never gets popped — MBB_C inherits SP = SP_in − 2.

Correct cross-MBB extension requires one of:

  - **Edge splitting**: insert a new MBB on each MBB_A → MBB_C
    escape edge with a compensating `inc sp; inc sp` (2 B) or
    `pop af`/`pop hl` (1 B if the reg is dead at the edge).
    Adds 2 B per escape edge; net save is `(BssBytes − 2) − 2 ×
    EscapeEdges`.
  - **Single-predecessor MBB_C**: only fire when every escape
    target has MBB_A as its sole predecessor, so the compensating
    pop can be prepended in-place.

Both require `MachineDominatorTree` (currently not a dependency of
`Z80LateOptimization`).  Multi-load and multi-escape cases compound.
This is multi-session work, **not "single-session-scoped"** as the
original entry in this doc claimed.

## Recommendation for the next session — REVISED

  1. **Close #129 in code** — both forms in `Z80LateOptimization`
     (same-class + cross-class in-MBB) exhaustively handle the
     suggested implementation.  Add a comment to the issue
     pointing at the two peepholes and the empirical "zero in-MBB
     residual" finding.
  2. **#132 filed** for the cross-MBB BSS-spill→PUSH/POP extension
     with the SP-balance correctness analysis, SNIOS reproducer,
     and multi-session estimate.  Gated on adding
     `MachineDominatorTree`/`MachinePostDominatorTree` as
     dependencies of late-optimization (or a dedicated pre-late
     pass that owns the cross-MBB analysis).
  3. **Reposition #131 as the more leverage near-term win** —
     `z80_preserves_regs` attribute prevents the spill in the
     first place, sidestepping the cross-MBB correctness issue
     entirely.  Multi-session scope, but each piece (Attr.td
     entry, RegMask wiring in `Z80CallLowering`, lit fixture) is
     independently testable.

Verification protocol for any cross-MBB extension (per
`memory/feedback_no_commit_first_version.md`):

  - lit: CodeGen/Z80 fixture covering both successful and
    bailed-out cases (single-pred MBB_C, multi-pred MBB_C,
    multi-load, multi-escape).
  - size baseline: rcbios + cpnos-rom + autoload-in-c.
  - z80-utils test-runner clang Oz suite.
  - cpnos-polypascal-test (4-cell matrix; unique layout/regalloc
    value oracle for cpnos changes).
  - MAME boot of autoload + rcbios + cpnos-rom (the SP-balance
    failure modes are runtime crashes that lit/sizes won't catch).

## Engagement-mode gating

Per the roadmap (§10.2), Cluster A close is the engagement-mode gate.
With this reframe:

  - **Closing #129 alone** is sufficient under the loose reading
    ("cluster fundamentally addressed") — it closes the dominant
    residual gap.
  - **Closing #129 + #131** is sufficient under the strict reading.
  - #89 + #27 can be downgraded to RFE/long-tail once #129's
    measurements show the residual.

## Files

  - `rc700-gensmedet/cpnos-rom/snios_c.c` lines 129–209 (try_send_frame,
    snios_sndmsg_force) — the smallest self-contained C source
    exhibiting the cluster.
  - `rc700-gensmedet/cpnos-rom/clang/cpnos.lis` lines 1567–1805 —
    clang asm for `_snios_sndmsg_force` (265 B).
  - `rc700-gensmedet/cpnos-rom/sdcc/audit/snios_c.s` lines 480–598 —
    SDCC asm for `_try_send_frame` (~119 B).
