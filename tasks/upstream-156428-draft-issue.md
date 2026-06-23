# Draft: llvm/llvm-project issue — comment on / close #156428

**To file:** add as a comment on https://github.com/llvm/llvm-project/issues/156428
OR open a new issue with `Fixes #156428` in the body.

**Before posting:** replace the `[YOUR INTRO]` placeholder with 2-3 sentences
in your own words. The rest is ready.

---

## [YOUR INTRO]

*(2-3 sentences: what problem you encountered, which targets you confirmed it
on, why you looked at LiveVariables. Write in your own voice.)*

Fixes #156428.

---

## Root cause

**File:** `llvm/lib/CodeGen/LiveVariables.cpp`  
**Function:** `LiveVariables::HandlePhysRegUse`  
**Branch:** the `FindLastPartialDef` block, currently around lines 252–257
of upstream `main` (unchanged since `342de06a7692`):

```cpp
    // All of the sub-registers must have been defined before the use of Reg!
    MachineInstr *LastPartialDef = FindLastPartialDef(Reg);
    if (LastPartialDef) {
      LastPartialDef->addOperand(
          MachineOperand::CreateReg(Reg, /*IsDef=*/true, /*IsImp=*/true));
    }
```

`FindLastPartialDef(Reg)` returns the last instruction that defined *any one*
sub-register of `Reg`. The code then marks that instruction as implicitly
defining the *entire* super-register `Reg`.

**The comment already states the invariant that must hold before adding the
implicit-def: "All of the sub-registers must have been defined." The code never
checks this.**

This is correct on x86 — `AL` and `AH` cover `AX` completely, so the last of
them to be defined really does mean `AX` is fully defined. It is incorrect on
targets where the sub-registers of a pair are **independent**: writing one half
leaves the other untouched. On such targets a lone sub-register definition (say,
`$l` in the Z80 `HL` pair) cannot be used to claim the whole pair (`$hl`) is
defined — but the current code does exactly that.

---

## Why the x86 assumption breaks

The register pair model has two variants:

1. **Overlapping sub-registers (x86 model):** `AL`/`AH` together *cover*
   `AX`; defining both leaves no part of `AX` undefined. Once the last half is
   written, the super-register truly is fully defined.

2. **Independent sub-registers:** `L` and `H` in a pair like `HL` (Z80), or
   `r24`/`r25` in AVR's `r25r24`. Writing `L` does **not** affect the bits of
   `H`. A lone `L` def does not imply `HL` is fully defined.

`HandlePhysRegUse` was written for model 1. It silently misbehaves on
targets using model 2: it fires the `LastPartialDef` branch, finds the most
recent half-def, and attaches `implicit-def $super` — even though the other
half was never defined.

---

## Observable consequence: MachineCopyPropagation miscompile

The spurious `implicit-def $super` propagates into downstream passes. The
most dangerous consumer is `MachineCopyPropagation`.

MCP collects every `isDef()` operand and records the defined register as
clobbering all aliases. Given:

```
$h  = COPY $e                  ; (A) copy tracked by MCP
LD_L_n 0, implicit-def $l,
         implicit-def $hl      ; (B) spurious $hl def from LiveVariables
... use $h                     ; (C)
```

MCP sees `$hl` (which overlaps `$h`) as "defined" at (B). It concludes that
the `$h = COPY` at (A) is no longer live-out and eliminates (A). But `$h`
still holds the copied value that (C) reads — the copy deletion is incorrect.

On the Z80 backend this manifested as 275 instances across `$de` sub-registers
and 19 instances across `$hl` sub-registers being wrongly eliminated.

---

## Minimal reproducer (MIR, `-run-pass=livevars`)

These fixtures expose the bug directly, without full compilation. Both use the
Z80 target because it has in-tree support for independent sub-registers (`$h`
and `$l` are the two independent halves of `$hl`).

**Case 1 — only one half defined (bug fires here):**
```
# $l is defined. $h is NOT. The lone $l def must NOT gain implicit-def $hl.
bb.0:
    liveins: $c
    LD_L_C implicit-def $l, implicit $c
    RET implicit $hl
```

*Expected:* `LD_L_C` has no `implicit-def $hl` operand.  
*Actual (buggy):* `LD_L_C` gains `implicit-def $hl` even though `$h` was never defined.

**Case 2 — both halves defined (correct behavior must be preserved):**
```
# Both $l and $h are defined before $hl is used. The later def legitimately
# gains implicit-def $hl. This behavior must not regress.
bb.0:
    liveins: $c, $b
    LD_L_C implicit-def $l, implicit $c
    LD_H_B implicit-def $h, implicit $b
    RET implicit $hl
```

*Expected:* `LD_H_B` (the later half-def) gains `implicit-def $hl`.  
*Actual:* same — this case is already correct and must stay correct.

**AVR reproducer** (from @zlfn's comment on #156428):

The same pattern on AVR (`r24`/`r25` are independent halves of `r25r24`):

```
bb.0:
    liveins: $r22
    $r25 = MOVRdRr $r22        ; r25 gets r22's value — must survive
    $r24 = LDIRdK 42           ; only r24 is defined
    $r20 = MOVRdRr $r25        ; needs r25 from above
    RCALLk @use, implicit $r25r24, implicit killed $r20
```

*Expected:* the `$r25 = MOVRdRr` is not eliminated.  
*Actual (buggy):* LiveVariables adds `implicit-def $r25r24` to the `LDIRdK`
instruction; MCP then deletes the `$r25 = MOVRdRr` as dead.

---

## The invariant that must be enforced

The comment in `HandlePhysRegUse` already states the correct contract:
*"All of the sub-registers must have been defined before the use of Reg."*

The fix must enforce this: `implicit-def $super` should only be added to
`LastPartialDef` when **every** leaf sub-register of `Reg` has actually been
defined earlier in the block. When that condition does not hold, adding the
implicit-def is wrong and must be suppressed.

For the x86 model (sub-registers cover the super completely) the existing
behavior is correct and must be preserved: once both `AL` and `AH` are defined,
`FindLastPartialDef` correctly marks the later def as implicitly defining `AX`.

---

## Confirmed targets

| Target | Register pair example | Status |
|---|---|---|
| Out-of-tree DSP (original report) | `ACC = AH:AL` | Confirmed — original reporter @hongjia |
| Z80 (in-tree) | `HL = H:L`, `DE = D:E`, `BC = B:C` | Confirmed — @zlfn and this report |
| AVR (in-tree) | `r25r24 = r25:r24` | Confirmed — @zlfn (MIR reproducer above) |

---

## Scope of change

The fix is confined to `llvm/lib/CodeGen/LiveVariables.cpp`. It does not touch
any target-specific code. The change is conservative: it produces **fewer or
equal** implicit-defs compared to today. Specifically, it only suppresses
implicit-defs that the stated invariant says should not exist. For x86
(`AL`/`AH` covering `AX`) the output is identical to today.
