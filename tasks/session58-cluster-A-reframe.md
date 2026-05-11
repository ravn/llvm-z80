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
| #129 | Late-MIR peephole: `STORE_BSS rr; CALL; LOAD_BSS rr` → `PUSH rr; CALL; POP rr` | open, unimplemented | ~36 B per cpnos-rom build (16-bit), more with 8-bit AF spills |
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

## Recommendation for the next session

**Implement #129** (late-MIR peephole, BSS-spill→PUSH/POP around
CALL).  Single-session-scoped, target-local change, no regalloc
cost-model touches.  Existing similar peephole already lives in
`Z80LateOptimization.cpp:4727` (the PUSH-rr-through-IX simplifier),
so the surrounding pattern is already understood.

Required guards (already learned from earlier BSS-related peepholes):

  1. **Slot must be dead between store and load** — no other reads/
     writes reach the spill from outside the 3-instruction window.
     Reuse the cross-block check from the existing BSS load-forwarding
     code path.
  2. **Cross-register stack depth tracking** — track ALL PUSH/POP
     between (1) and (3), not just same-register, for LIFO
     correctness (lesson from #41).
  3. **MCSymbol offset comparison** — `MO_MCSymbol::isIdenticalTo`
     compares only the symbol pointer, not the offset; explicit
     `getOffset()` check needed (lesson from #41).
  4. **Skip when CALL itself depends on the BSS slot** (the spill
     is data flow, not a true spill).

Verification protocol (value oracle required per
`memory/feedback_no_commit_first_version.md`):

  - lit: existing CodeGen/Z80/ tests + add a new BSS-around-CALL
    fixture.
  - size baseline: rcbios + cpnos-rom + autoload-in-c.
  - z80-utils test-runner clang Oz suite.
  - cpnos-polypascal-test (the 4-cell matrix; this is the unique
    layout/regalloc value oracle for cpnos changes).

**Do NOT attempt #131 in the same session as #129** — #131 needs a
new clang attribute + RegMask wiring across CallLowering, which is
a larger surface.  Implement #129 first (cheap recovery), measure,
then design #131 (avoid the spill in the first place) as a separate
multi-session deliverable.

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
