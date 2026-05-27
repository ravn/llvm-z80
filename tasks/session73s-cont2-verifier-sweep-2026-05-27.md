# Session 73s (cont-2) — Cluster 3 verifier-surface sweep (#197) — 2026-05-27

Continued the issue-closeout Cluster 3 (verifier/#197).  Fixed **4 root-cause
classes** of the `-O2 -verify-machineinstrs` red surface, each fully gated.
All four committed to `main` (NOT pushed).

## Closed / fixed this session

| # | what | commit | byte impact |
|---|------|--------|-------------|
| #200 | SPILL/RELOAD FI pseudos kept 2-operand form after PEI -> restore folded `$offset` as 0 placeholder | `47db108` | neutral |
| #194 | 2 Z80LateOptimization peepholes left stale `$a` liveness (cross-block `LD A,r` removal -> addLiveIn; `LD A,#0`->`XOR A` -> undef use) | `46fbafb` | neutral |
| #209 | stack-reservation `PUSH AF` reads don't-care `$a`/`$flags` (frame lowering) -> mark undef | `99ee190` | neutral |
| #209 | `EX DE,HL` one-way-copy dest read is don't-care (`copyPhysReg`) -> mark dest use undef | `8ff4208` | **cpnos 2028->2022 B (-6 B)** |

#200 **CLOSED on GitHub**.  #209 **filed** (covers the PUSH AF + EX DE,HL
don't-care-read pair).  #194 fixed (its exact scope — late-opt-introduced —
is closed; can close on GitHub).

Lit: **123+5 -> 127+5** (4 new tests: issue-200, issue-194, issue-209 ×2).
Value oracle on every commit: test-runner `-diff-opt -native-oracle` default
799/0 + static-stack 793/0 (no divergences) throughout; cpnos byte-identical
for the 3 neutral fixes; for the EX DE,HL win cpnos polypascal-test PASS
(50.65 s) + AES enc/dec PASS (ts 11516046 unchanged).

## The systemic picture (zoom-out)

The #197 red surface is **two families**, not isolated bugs:

1. **Stale-def liveness** — a peephole removes/moves a physreg def without
   updating live-ins.  (#194 cross-block `LD A,r` removal.)  Fix: `addLiveIn`.
2. **Don't-care reads** — an instruction nominally reads a physreg whose value
   is irrelevant, emitted without `undef`.  Members found: `XOR A` (zeroing,
   late-opt), `PUSH AF` (stack reserve, frame lowering), `EX DE,HL` one-way
   copy (`copyPhysReg`).  Fix: `setIsUndef` on the don't-care use.

## REMAINING surface — pick up here

After the 4 fixes, `clang -O2 -ffreestanding -mllvm -verify-machineinstrs -c
aes256.c` still fails (verifier aborts at first function):

- **`aes_mixColumns`: `PUSH_HL implicit $hl` reads undef `$hl`.**  THIRD
  family, *not* a clean don't-care read.  **ROOT-CAUSED + filed as #210**;
  deliberately NOT fixed (non-trivial + high-risk frame-lowering, non-production
  codepath).  **Confirmed root cause:** SP-relative frame spill borrows HL
  (`PUSH_HL; LD HL,nn; ADD HL,SP; LD (HL),r; POP_HL`) gated by
  `NeedSaveHL = isRegLiveAt(Z80::HL, ...)`; that forward scan **over-reports HL
  live** — it hits the next SP-relative access (which references HL but
  *redefines* it before use) and concludes HL is live, cascading so every spill
  saves a dead HL and reads it undef.  Confirmed in bb.8 (`liveins: $a`, HL
  killed by a preceding `LOAD8_IND` then read by `PUSH_HL`).  **Verifier-metadata
  only / runtime-correct** (diff-oracle green; AES byte-correct).  **Does NOT
  affect production** — cpnos/BIOS/autoload all `+static-stack` (BSS direct
  addressing, no SP-relative HL borrow).  Fix needs robust HL-liveness (precompute
  per-position over the original block, or a provably-dead scratch) + full MAME
  oracle; a wrong HL-dead call skips a needed save -> miscompile.  Earlier
  findings (kept for context):
  - It is an **SP-relative frame-access bracket** that borrows HL for the
    address: `PUSH_HL; LD HL,nn; ADD HL,SP; LD (HL),r; [INC HL; LD (HL),r;]
    POP_HL` (the dump shows e.g. spilling `$e` at SP+7).  `aes_mixColumns` has
    `framePointerPolicy: none` and is NOT `+static-stack`, so every frame slot
    is reached SP-relative, borrowing HL.
  - The bracket appears **after ExpandPostRAPseudos** (`-print-after=postrapseudos`
    shows the PUSH_HL; `-stop-after=prologepilog` still shows the unexpanded
    `SPILL_GR8/16`).  So the emission is in `Z80InstrInfo::expandPostRAPseudo`
    (or a helper it calls) for the no-FP SP-relative case — NOT the
    `eliminateFrameIndex` SP-relative helpers in `Z80RegisterInfo.cpp` I first
    suspected.  (Caveat: the SPILL_GR8 case I read at Z80InstrInfo.cpp:950
    emits `LD (IX+d),r`, which doesn't match the observed SP-relative PUSH_HL
    bracket — so the exact path is still unconfirmed; reconcile the pass
    boundary first.)
  - Root mechanism: the PUSH_HL saves HL purely to borrow it as the
    address-scratch register, guarded by a liveness check.  The verifier sees
    HL undef at the PUSH, so either the guard **over-saves** (saves HL when it
    is actually dead -> reads undef; fixing it = elide the PUSH = a size win)
    or the block live-ins are **stale**.
  - **Next-session first drill:** trace ONE `SPILL_GR8 $e, <off>` in
    `aes_mixColumns` through FinalizeISel -> ExpandPostRAPseudos -> z80-scavenging
    with `-print-after-all` to pin the exact BuildMI(PUSH_HL) site; then decide
    (a) tighten the HL-save guard to skip when HL is fully dead (codegen change
    -> FULL value oracle incl. MAME boot, since cpnos/AES bytes may move) vs
    (b) reconcile the block live-ins.  Gate exactly like the EX DE,HL fix.
- Whatever surfaces after that (the verifier aborts at the first function, so
  the full remaining count is unknown until `aes_mixColumns` clears).

## Production-target check (this session, safe measurements)
- **BIOS clang = 5897 B — UNCHANGED** (was 5897).  The 4 fixes are byte-neutral
  for the BIOS (no EX DE,HL DCE opportunity there); no regression, no re-boot
  needed (byte-identical to last-verified).
- **cpnos PROM1 = 2022 B** (was 2028; -6 B from the EX DE,HL fix), polypascal PASS.
- **AES** enc/dec PASS, size + ts unchanged.

Plus the known **#112/#189** illegal-vreg class (GR16NoIR) — the first of the
three #197 classes named in #200's text — is gated separately (IY-unreserve
cost model).

## #197 status
3 of the originally-named classes (#200, #194, and the PUSH AF/EX DE,HL
don't-care reads) cleared.  #197 (flip `-verify` to a blocking CI lane) still
gated on: the `aes_mixColumns` PUSH_HL liveness-reconciliation class above +
#112/#189.  Once those clear, wire `-verify-machineinstrs` into a CI lane.
