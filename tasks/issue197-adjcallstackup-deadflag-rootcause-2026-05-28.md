# #197: dominant remaining verifier class root-caused — ADJCALLSTACKUP over-conservative Defs (2026-05-28)

## Summary

After this session cleared the SP-relative borrow (#210), prologue `PUSH_HL`,
CFG duplicate-successor, expansion live-ins, `AND_A` and `SBC A,A` classes, the
**dominant remaining `-verify -full` fatal class** (≈ the bulk of ~600 fatal
compiles, concentrated in the float tests — `test_46_f32_conv_edge` alone had
394 errors) is **call-return registers spuriously marked `dead`**, caused by
`ADJCALLSTACKUP`'s over-conservative `.td` `Defs`.

## Mechanism (reproduces in a 5-line function)

```c
float top(float a, float b, float c){ float t = a + b; return t * c; }
```
Post-greedy MIR:
```
CALL_nn &__addsf3, ..., implicit-def $de, implicit-def dead $hl, ...
ADJCALLSTACKUP 4, 4, ..., implicit-def $hl, implicit-def dead $a, implicit $sp
%27:gr16 = COPY $de
%28:gr16 = COPY $hl        ; reads $hl -> verifier: "undefined physical register"
```
- `__addsf3` returns the 32-bit float in **DE:HL**; both halves are used (the
  result `t` feeds `__mulsf3`).
- `ADJCALLSTACKUP` is declared `let Defs = [SP, HL, A]` in `Z80InstrInfo.td`
  (worst case: large frames expand to `LD HL,n; ADD HL,SP; LD SP,HL`, small to
  `POP AF`).  So **every** call-frame cleanup claims to clobber HL+A.
- Greedy regalloc therefore treats the CALL's `$hl` result as **dead**
  (clobbered by the following ADJCALLSTACKUP before the `COPY $hl`), and lets
  `COPY $hl` read ADJCALLSTACKUP's "definition".
- But `ADJCALLSTACKUP 4,4` has AdjAmount = 4 − 4 = **0** → erased in
  `eliminateCallFramePseudoInstr` (no code, clobbers nothing).  Its phantom
  `$hl` def vanishes, leaving `COPY $hl` reading the CALL's dead `$hl` → undef.

The `dead` flag is **added by greedy** (clean `implicit-def $hl` at
`finalize-isel`; `implicit-def dead $hl` at `-stop-after=greedy`).

## Why it's metadata-only

`diff-oracle` is green (default 799/0/50, +static-stack 793/0/50) — at runtime
HL genuinely holds the result and is used correctly; only the `dead`/liveness
*metadata* is wrong.  This is the same over-conservatism that
`adjCallStackUpClobbersReg()` (Z80RegisterInfo.cpp) was written to compensate
for inside `isRegLiveAt`, but here it corrupts regalloc's dead-flag analysis,
which `isRegLiveAt` cannot reach.

## Fix design (next focused session — regalloc-affecting, needs full gate)

Prune each `ADJCALLSTACKUP`'s implicit-def operands to its **actual** clobber
set, **before greedy**, using the existing `adjCallStackUpClobbersReg` logic
(AdjAmount = `$bytes − $prior`):
- AdjAmount == 0 → clobbers nothing (drop $hl, $a)
- SM83 && AdjAmount ≤ 127 → `ADD SP,e` → nothing (drop $hl, $a)
- AdjAmount ≤ 16 → `POP AF` → A only (keep $a, drop $hl)
- else → `LD HL,n; ADD HL,SP; LD SP,HL` → HL only (keep $hl, drop $a)

Implementation: a small pre-RA `MachineFunctionPass` inserted alongside the
existing `insertPass(&MachineSchedulerID, …)` Z80 passes (ReorderTestDec /
SplitDjnzCounters / NarrowNoIndex), iterating `ADJCALLSTACKUP` and calling
`MI.removeOperand`/`RemoveOperand` for the non-clobbered implicit-defs.  The
AdjAmount is final by then (set at ISel), so the prune is sound.

**Risk/upside:** regalloc-affecting → full value-oracle + cpnos/BIOS byte check
required.  Likely a *codegen improvement* (HL/A become allocatable across
non-clobbering call-frame brackets → fewer spills), which could even help the
production size budgets — measure.  Reuses the already-tested
`adjCallStackUpClobbersReg`, so the clobber logic itself is low-risk; the risk
is purely the allocation change, which the oracle gates.

## Other residual classes seen
- i1 → A `COPY` size mismatch (`$a = COPY %n:_(s1)`) in `icmp`-to-bool lowering.
- The full-suite verifier sweep *hung* ~10 min around `test_92_edge` /
  `test_96_iy_largeoffset_spill` — separate slowness/loop worth a look.
