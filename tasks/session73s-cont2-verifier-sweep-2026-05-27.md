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

- **`aes_mixColumns` bb.4/bb.8: `PUSH_HL implicit $hl` reads undef `$hl`.**
  This is a THIRD family, *not* a clean don't-care read: the
  `PUSH_HL; LD HL,nn; ADD HL,SP; ...; POP_HL` frame-index SP-relative
  address-computation bracket saves HL guarded by
  `NeedSaveHL = isRegLiveAt(Z80::HL, ...)`.  The guard decided HL needed
  saving (so we must keep the PUSH — can't blanket-undef it), but the
  verifier sees HL undef at that point.  So this is a **liveness
  reconciliation** issue (stale block live-ins, or `isRegLiveAt`/
  `computeRegisterLiveness` returning conservative/`Unknown`), in the
  frame-index lowering scratch-management helpers in `Z80RegisterInfo.cpp`
  (~10 PUSH_HL sites, lines 602-1000; the SP-relative ones around
  `emitLargeOffsetAddr` @905 + callers).
  - First drill: dump `aes_mixColumns` MIR before PEI; find whether HL is
    genuinely live across the bracket (then the bug is missing block
    live-ins) or genuinely dead (then `NeedSaveHL` is over-conservative and
    the PUSH should be elided, also a size win).
- Whatever surfaces after that (the verifier aborts at the first function, so
  the full remaining count is unknown until `aes_mixColumns` clears).

Plus the known **#112/#189** illegal-vreg class (GR16NoIR) — the first of the
three #197 classes named in #200's text — is gated separately (IY-unreserve
cost model).

## #197 status
3 of the originally-named classes (#200, #194, and the PUSH AF/EX DE,HL
don't-care reads) cleared.  #197 (flip `-verify` to a blocking CI lane) still
gated on: the `aes_mixColumns` PUSH_HL liveness-reconciliation class above +
#112/#189.  Once those clear, wire `-verify-machineinstrs` into a CI lane.
