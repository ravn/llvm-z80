# Upstream-submission writeup — LiveVariables spurious super-register implicit-def (#156428)

Status: fix landed in ravn/llvm-z80 main (`a32c4f33`, 2026-05-28).  This doc is
the target-agnostic submission text for llvm/llvm-project.  Per project policy
the fix is NOT filed/PR'd upstream by the agent — a human submits it.

## One-line

`LiveVariables::HandlePhysRegUse` can add a spurious super-register
`implicit-def` to an instruction that defines only one sub-register, on targets
where the sub-registers of a register tuple are independent — corrupting
liveness and enabling a `MachineCopyPropagation` miscompile.

## Where the bug is

`llvm/lib/CodeGen/LiveVariables.cpp`, `HandlePhysRegUse`, the
`FindLastPartialDef` branch:

```cpp
    // All of the sub-registers must have been defined before the use of Reg!
    MachineInstr *LastPartialDef = FindLastPartialDef(Reg);
    if (LastPartialDef) {
      LastPartialDef->addOperand(
          MachineOperand::CreateReg(Reg, /*IsDef=*/true, /*IsImp=*/true));
    }
```

`FindLastPartialDef(Reg)` returns the last instruction that defined *any one*
sub-register of `Reg`.  The code then marks that instruction as implicitly
defining the *whole* super-register `Reg`.  The comment asserts the invariant
"All of the sub-registers must have been defined" — but the code never checks
it.

## Why it is wrong

The implicit assumption is the x86 model: after `AL` and `AH` are both defined,
the last of them can be said to define `AX`/`EAX`.  That is only valid when the
sub-register defs together *cover* the super-register.

On a target where a register pair's halves are **independent** — writing one
half does not touch the other (e.g. the Z80 backend's `HL` = `H`:`L`, `DE`,
`BC`, etc.) — a single `LD L, n` defines only `L`.  If `HL` is then used,
`FindLastPartialDef` returns the `LD L` and the pass attaches
`implicit-def $hl` to it, falsely claiming the whole pair is defined though `H`
was never written.

Downstream effect (the observable miscompile): `MachineCopyPropagation`
collects every `isDef()` operand and calls `clobberRegister()`.  Given

```
$h = COPY $e            ; copy tracked
LD_L_n 0, implicit-def $l, implicit-def $hl   ; spurious $hl def
... use $h
```

MCP sees `$hl` (which overlaps `$h`) "defined" by the `LD_L_n`, decides the
`$h = COPY $e` is dead, and deletes it — but `$h` still holds the copied value.
In the Z80 backend this manifested across the test suite as 275 `DE` + 19 `HL`
instances.

## The fix

Only attach the super-register implicit-def when every *leaf* sub-register has
actually been defined (enforcing the invariant the comment already states):

```cpp
    MachineInstr *LastPartialDef = FindLastPartialDef(Reg);
    if (LastPartialDef) {
      bool AllSubRegsDefined = true;
      for (MCPhysReg SubReg : TRI->subregs(Reg)) {
        // Only require leaf sub-registers (no further sub-registers).
        if (TRI->subregs(SubReg).begin() == TRI->subregs(SubReg).end() &&
            !PhysRegDef[SubReg]) {
          AllSubRegsDefined = false;
          break;
        }
      }
      if (AllSubRegsDefined)
        LastPartialDef->addOperand(
            MachineOperand::CreateReg(Reg, /*IsDef=*/true, /*IsImp=*/true));
    }
```

This is a no-op for targets whose sub-register defs already cover the super
(x86 `AL`+`AH` → still adds `implicit-def $ax`); it only suppresses the
*spurious* def when the super-register is genuinely only partially defined.
It also makes x86 strictly more precise (using `AX` after defining only `AL`
no longer claims `AX` fully defined).

## Evidence it is the root cause (not a symptom patch)

The Z80 backend carried a post-rewrite workaround pass (`Z80FixupImplicitDefs`)
that removed the spurious super-register implicit-defs before MCP.  With this
LiveVariables fix in place and that workaround DISABLED, the Z80 differential
runtime oracle stays clean in both configs (default 799/0/50/207, +static-stack
793/0/50/213, zero divergences) — i.e. the fix prevents the MCP miscompile at
the source, so the workaround is no longer needed for correctness.

## Test

A target-independent observable repro needs a backend with independent
sub-registers; the witness is the Z80 lit test
`llvm/test/CodeGen/Z80/issue-156428-livevars-independent-subregs.mir`
(`-run-pass=livevars`): a lone `$l` def followed by a `$hl` use must NOT gain
`implicit-def $hl`; when both halves are defined it must.  For upstream, the
same can be expressed with any target exposing independent sub-registers, or as
a unit assertion on the invariant.

## Risk

`LiveVariables` feeds register allocation, so the change is gated on the full
Z80 value oracle (both configs, baseline-identical), cpnos production binary
(2023 B, size-stable), and lit (136+5).  The logic change is conservative
(strictly fewer/equal implicit-defs, only removing ones that violate the stated
invariant).
