# Session 35 — close #97 (BC ping-pong in single-BB self-loops)

Date: 2026-05-02 (continuation of session 34's investigations).
Branch: `session-35-issue-97` (off `main`).

## TL;DR

Closes ravn/llvm-z80#97.  Lit suite 76 PASS + 1 XFAIL → **77 PASS + 1
XFAIL** (issue-97 file flips PASS; sub-case `issue-97a-bc-pingpong-i16-
counter.ll` filed as the harder follow-up XFAIL).  rcbios BIOS and
cpnos-rom payload **unchanged** (5920 B / 1708 B) — the peephole still
fires on hand-written Case 1 (`LD C,L; LD B,H` from HL-param) shapes
in PROM init code (PROM0 non-padding -1 B).

`Z80LoopRotate` was flipped on by default during measurement and
flipped back off: the rotation-around-CALL regression (rcbios +33 B,
cpnos-rom +4 B) is a separate spill-shape problem.  Documented as the
next gating constraint on #77a.

## Closed this session (with verification)

- **#97** BC ping-pong in single-BB self-loops.  Post-RA peephole in
  `Z80LateOptimization.cpp` after the existing #84 peephole (~250
  LOC).  Handles three pred shapes × two body orderings:

  | Pred shape (Case)              | Loop ordering (Case A/B)             |
  | ------------------------------ | ------------------------------------ |
  | 1: `LD C,L; LD B,H` (HL param) | A: `LD L,C; LD H,B` then `INC BC × N` |
  | 2: `LD HL,nn N; LD BC,nn N`    | B: `INC BC × N` then `LD L,C; LD H,B` |
  | 3: `LD BC,nn N` only           | (both)                               |

  Verified against `issue-97-bc-pingpong-singlebb.ll` (XFAIL → PASS),
  `hl-no-bc-backup.ll`, `static-stack-loop-counter-desync.ll`, and
  PROM init code (cpnos-rom `setup_ivt`).

  Pred Case 3 rewrites `LD_BC_nn N` to `LD_HL_nn N` in pred (HL becomes
  the carrier).  Guards: HL not live-in to LoopBB before the rewrite,
  no HL touch elsewhere in pred, BC dead at all non-loop successors.

  Body matcher counts INC_HL × M and INC_BC × N inside the
  ping-pong window; ExtraIncs = N − M new INC_HL inserted at the
  later anchor, both anchor pairs erased.  Full no-touch guard on
  the leading + trailing regions and on terminators.

## Filed this session

- **#97a** (issue-97a-bc-pingpong-i16-counter.ll, XFAIL): the i16-counter
  rotated-loop sub-case where the counter and pointer compete for HL.
  Closing it requires either a regalloc-level swap (counter → BC,
  pointer → HL) or a more invasive peephole that rewrites every
  DEC_HL / LD_A_L / OR_H counter reference.  Not a real-code shape
  in cpnos-rom or rcbios today (counters are i8 / DJNZ-eligible) so
  parked.

- **Rotation-around-CALL spill regression** (sub-issue under #77a):
  with `Z80LoopRotate` on, rotated loops that contain a CALL force
  regalloc to BSS-spill the loop carrier across the call.  Measured
  in `_netboot_mpm`'s inner banner loop: 28 B → 47 B (+19 B for the
  loop alone, +28 B for the function).  rcbios BIOS +33 B, cpnos-rom
  +4 B end-to-end.  Possible fixes: (a) post-RA peephole rewriting
  the spill-around-CALL shape, (b) regalloc cost-model tweak to
  rematerialize cheap loop carriers across the call, (c) keep rotation
  default-off until either lands.  Documented inline in
  `Z80LoopRotate.cpp` and in the `issue-77a-loop-rotate.ll` test
  header.

## Files touched

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp` — new peephole at the
  position of the existing #84 peephole.
- `llvm/lib/Target/Z80/Z80LoopRotate.cpp` — comment update (default
  stays false; documents the rotation-around-CALL gate).
- `llvm/test/CodeGen/Z80/issue-97-bc-pingpong-singlebb.ll` — drop
  XFAIL, drop the i16-counter case (extracted to its own file),
  rewrite header to describe the fix.
- `llvm/test/CodeGen/Z80/issue-97a-bc-pingpong-i16-counter.ll` (new) —
  XFAIL, single function, header describes the harder case.
- `llvm/test/CodeGen/Z80/issue-77a-loop-rotate.ll` — header refreshed
  to mention the new rotation-around-CALL gate (RUN lines unchanged).
- `llvm/tasks/session35-summary.md` (this file).

No `rc700-gensmedet` source changes.  cpnos-rom and rcbios sizes
verified unchanged from session 33 baseline.

## Pain points caught

- **Operand index for `LD_HL_nn`**: I used `RegState::Define` + a
  separate addReg(HL) when constructing the rewritten LD_HL_nn,
  producing `ld hl,hl`.  Operand 0 is the immediate / global / MC-
  symbol; the def is implicit in the opcode.  Caught immediately
  in the IVT-repro asm dump.  **(Easy)**, ~5 min.

- **Symbol vs. immediate compare**: my Case 2 matcher used `isImm()`
  only, which silently bailed on `__ivt_start`-style global / MC-
  symbol operands (the cpnos-rom shape).  Extended to `isGlobal`,
  `isMCSymbol`, `isBlockAddress` with offset comparison.  **(Medium)**
  — caught only when the lit suite passed but cpnos-rom size showed
  no improvement, which forced a closer look at the actual asm.

- **Two body orderings**: the test file's hand-rotated cases all had
  `LD_L_C` first; flipping `Z80LoopRotate` default-on exposed the
  inverse shape (INC_BC chain at top of body, LD_L_C at end).
  Restructured the matcher to find both anchors and walk three
  regions (leading / body / trailing) regardless of order.
  **(Medium)** — visible only after flipping the default.

- **Rotation-around-CALL was the real cost**: spent ~30 min chasing
  why cpnos-rom +4 B persisted after Case 3 landed, before realising
  the `_netboot_mpm` inner loop's BSS spill across `impl_conout` is a
  separate problem.  **(Medium)** — required diffing function-by-
  function asm.  Worth keeping the rotate default off until that
  shape is also handled.

## Still open

- **#77a** loop rotation: pass exists, gated off-by-default.  Waiting
  on rotation-around-CALL spill rewrite.
- **#97a** i16-counter sub-case (this session's filing).
- **#94 / #98 / #92 (closed) / #89 / #95** regalloc cluster — same
  family, deferred to a dedicated regalloc-hint session.
