# Session 40 strand A — IY un-reservation diagnosis (2026-05-03)

## TL;DR

The session-39 framing of #38 ("deeper regalloc / register-class
issue, 11 runtime FAILs after #28 fix") was incomplete.  The
real failure surface is **systemic**: 387 test×opt-level FATALs
arise from clang's integrated assembler crashing on
`<MCInst 0>` (a bare opcode-0 MachineInstr produced by pseudo
expansions whose 8-bit sub-register lookup tables don't include
IXH/IXL/IYH/IYL).  At least one expansion site
(`Z80InstrInfo::expandPostRAPseudo` / LSHR16 / ASHR16) is the
root for many failures; an audit of the 42 sub-register
extraction sites in `Z80InstrInfo.cpp` is needed to identify
the rest.

The `<MCInst 0>` printing has been a confusing red herring —
opcode 0 is `TargetOpcode::PHI`, and 0-operand PHIs in post-RA
MIR look like a regalloc bug at first glance.  They are not;
they are **`BuildMI(MBB, I, DL, TII.get(0))`** calls where a
sub-register-keyed opcode lookup returned 0 and was passed
directly to `BuildMI` without a guard.

## Method

Branch `session-40-ix-iy-shadow`.  Single-line experiment:
remove `Reserved.set(Z80::IY)` from `getReservedRegs` (IX still
reserved).  Built clang/llc; ran lit + clang test suite.

## Failure surface (clang test runner)

| Bucket           | IY reserved | IY un-reserved | Δ           |
|------------------|------------:|---------------:|------------:|
| Total tests*opt  |         972 |            972 |       —     |
| Pass             |         573 |            286 |     -287    |
| Fail (runtime)   |          93 |             58 |     -35     |
| **Fatal (build)**|           0 |            422 |   **+422**  |
| Skip             |         206 |            206 |       —     |

(The Fatal column dominates the regression: 387 distinct
test×opt combinations, 120 unique tests after collapsing opt
levels.  Of those 120, ~100 are auto-generated `test_9X_edge`
tests; ~20 are named tests like `test_04_i32_bitwise`,
`test_05_i64_arithmetic`, `test_13_shift_ops`,
`test_24_multi_return`, `test_36_stack_pressure`,
`test_43_f32_inf_nan`, `test_55_i128_arithmetic`.)

## lit consequence

`ldir-aftermath.ll` flips to FAIL — code-quality regression,
not miscompile.  Greedy now picks IY for the dma pointer being
fed to LDIR; the post-LDIR store needs HL, so a `PUSH IY; POP HL`
extraction appears.  This is *correct* but worse than reserved-
IY codegen.

## Root cause of the FATALs

All FATALs share the message:

```
fatal error: error in backend: Unsupported instruction : <MCInst 0>
```

Reproduces minimally on `test_04_i32_bitwise` via:

```sh
build-macos/bin/clang --target=z80 -O2 -c -nostdlib -ffreestanding \
    z80-utils/test-runner/testcases/clang/test_04_i32_bitwise.c -o /tmp/t04.o
```

The `0` in `<MCInst 0>` is the opcode value, equal to
`TargetOpcode::PHI`.  Tracing through `Z80MCInstLower::lower`
shows the offending MachineInstr prints as bare `PHI` with zero
operands.  The pre-emit MIR contains:

```
$hl = COPY16_PUSHPOP $iy, implicit-def $l, implicit-def dead $h
LD_A_L implicit-def $a, implicit $l
AND_n 1, implicit-def $a, implicit-def $flags, implicit $a
ADD_A_C implicit-def $a, implicit-def $flags, implicit $a, implicit $c
LD_IXd_A -12, implicit $a
PHI                ; <-- bare opcode-0 MI #1
PHI                ; <-- bare opcode-0 MI #2
LD_A_E ...
```

The bare PHIs first appear after the **generic** post-RA
pseudo expansion pass (`postrapseudos`), which calls
`Z80InstrInfo::expandPostRAPseudo` to lower target pseudos.

Looking at the corresponding pre-postrapseudos MIR, the
position is held by:

```
SPILL_GR8 $a, -12 :: (store (s8) into %stack.13)
renamable $iy = LSHR16 killed renamable $iy(tied-def 0), implicit-def $flags
```

`LSHR16` (`Z80InstrInfo.cpp` line 1763) is expanded as:

```cpp
case Z80::LSHR16:
case Z80::ASHR16: {
  Register Reg = MI.getOperand(0).getReg();
  Register Hi = TRI->getSubReg(Reg, Z80::sub_hi);
  Register Lo = TRI->getSubReg(Reg, Z80::sub_lo);
  bool IsLogical = (MI.getOpcode() == Z80::LSHR16);
  BuildMI(MBB, MI, DL, get(IsLogical ? getSRLOpcode(Hi) : getSRAOpcode(Hi)));
  BuildMI(MBB, MI, DL, get(getRROpcode(Lo)));
  MI.eraseFromParent();
  return true;
}
```

For `Reg == IY`: `Hi == IYH`, `Lo == IYL`.

```cpp
static unsigned getSRLOpcode(Register Reg) {
  static const unsigned T[] = {Z80::SRL_A, Z80::SRL_B, Z80::SRL_C, Z80::SRL_D,
                               Z80::SRL_E, Z80::SRL_H, Z80::SRL_L};
  int I = Z80::gr8RegToIndex(Reg);
  return I >= 0 ? T[I] : 0;
}
```

`gr8RegToIndex(IYH)` returns `-1` (IYH is not in {A,B,C,D,E,H,L}).
`getSRLOpcode(IYH)` therefore returns `0`.  Same for
`getRROpcode(IYL)`.  Both `BuildMI(MBB, MI, DL, get(0))` calls
construct opcode-0 (i.e. PHI) MachineInstrs.  The encoder later
hits these and reports `<MCInst 0>` as unsupported.

Two bare PHIs per LSHR16 over IY × many such ops in `main` =
flood of "PHI"s in the MIR dump and one fatal at encode.

## Why this is a *systemic* finding

`Z80InstrInfo.cpp` has **42** call sites of
`TRI->getSubReg(<reg>, Z80::sub_lo|Z80::sub_hi)`.  Almost all of
them feed into 8-bit-keyed opcode lookups
(`getSRLOpcode`, `getRROpcode`, `getSRAOpcode`,
`getLD8RegOpcode`, etc.) that only accept {A,B,C,D,E,H,L}.

When IX/IY were reserved, none of these sites ever saw IXH/IXL/
IYH/IYL because regalloc never produced GR16 vregs allocated to
IX/IY.  Un-reserving IY exposes every site simultaneously.

This is exactly why session 39 saw "11 runtime FAILs" — the
clang -Os runs that didn't touch LSHR16 escaped, but enough
named tests use the affected pseudos to produce a large named-
test failure set and a much larger auto-generated edge-test
set.

## Why it's correct that this stays parked for now

Re-enabling IY for general allocation requires either:

1. **Defensively guard every BuildMI on a sub-register-keyed
   opcode lookup.**  When the opcode is 0, fall back to a path
   that doesn't use the sub-register directly (e.g. EX (SP),IY
   + manipulate via HL + EX back).  ~42 call sites; each
   needs a per-pseudo fallback.  Large surface.

2. **Extend the sub-register opcode lookups to know about
   IXH/IXL/IYH/IYL when `+undocumented` is enabled.**  Many
   undocumented Z80 instructions exist for the IX/IY halves
   (SRL IXH = DD CB 00 3C, etc.).  Already partially done in
   the codebase — needs an audit + fill in.  Smaller surface,
   gated on `+undocumented`.

3. **Restrict the GR16 register class on the affected pseudos
   so IX/IY can't be allocated to them.**  Define narrower
   register classes (`GR16NoIR`?) for tied operands of
   LSHR16/ASHR16/SHL16/etc., constrain by class so regalloc
   never chooses IY.  Smallest target source delta but very
   declarative; the cost is occasional extra COPYs into a
   permitted pair.

Option 3 looks most attractive: declarative, no per-site
defensive code, easy to extend if other sites are found.  It
matches the BReg/BCReg single-register-class technique used in
Phase 3 (#94 / #99) — same lever, applied as exclusion class.

The implementation cost is real but bounded.  Carrying the work
forward is justified by the IX/IY allocatability win.

## What to file

ravn/llvm-z80 issue: "Pseudo expansion sites assume GR16 sub-
registers are documented 8-bit registers".  Body:

  - Repro: un-reserve IY, build clang test runner — 387 FATAL
    `<MCInst 0>` failures.
  - Root: `BuildMI(..., TII.get(0))` from `getSRLOpcode(IYH)`
    returning 0 in LSHR16/ASHR16 expansion.
  - Site list: 42 sub-register extraction sites in
    `Z80InstrInfo.cpp`, of which LSHR16/ASHR16 (line 1763)
    are confirmed.  Audit needed for the remaining 41.
  - Fix design: option 3 above (register-class restriction).
  - Test: lit IR fixture demonstrating LSHR16 on IY → bare
    PHI at MIR level, with current behavior (XFAIL) and
    expected behavior (PASS).
  - Gates: makes #38 actionable; closes session 39's "deeper
    regalloc issue" framing.

## Strand A status at end of investigation

  - **Diagnosis confirmed**: not a regalloc/coalescer issue,
    but a backend completeness issue.
  - **Issue filed**: ravn/llvm-z80#112.
  - **Partial fix landed**: commit `28613369fa08` applies the
    GR16NoIR exclusion class to LSHR16/ASHR16 operands.
  - **Audit complete**: of the 42 sub-register extraction sites
    in `Z80InstrInfo.cpp`, only LSHR16/ASHR16 produce bare
    opcode-0 MIs.  Other sites either:
      - have explicit IR16 branches that route through
        PUSH HL; LD ...; POP IX/IY (ZEXT_GR8_GR16 line 802,
        SEXT_GR8_GR16 line 858, XOR_CMP_Z16 line 1357,
        SEXT16 line 1862);
      - use opcode lookup tables (`getLD8RegOpcode`,
        `getSUB`/`getXOR`/`getOR`/`getCP`/`getSBC`/`getADC`)
        that DO have IXH/IXL/IYH/IYL entries — emit *valid
        but undocumented* instructions instead of opcode 0
        (CMP16_ULT/CMP16_SBC_FLAGS/XOR_CMP_NE16/SM83_CMP_ZERO16,
        SM83_SADDO_HL_rr/SM83_SSUBO_HL_rr fall here);
      - have explicit `if (!Op) return false;` guards that
        fail-soft (ZEXT_GR8_GR16 fall-through path).
    See ravn/llvm-z80#112 closure rationale.
  - **Remaining problem class** (separate issue): pseudo
    expansions that emit IXH/IXL/IYH/IYL undocumented ops
    without checking `STI->hasUndocumented()`.  This is a
    policy violation, not an encoder crash, and is filed
    separately as a sibling of #112.
  - **Source restored**: `Reserved.set(Z80::IY)` reinstated;
    debug prints removed; lit baseline 84 PASS + 1 XFAIL
    (was 83+1 — +1 from new `issue-112-gr16noir-lshr.ll`
    regression-guard test).
