While working on an out-of-tree z80 backend, I have verified the https://github.com/llvm/llvm-project/issues/156428 bug.

Problem is that the 16-bit registers on the Z80 can have each 8-bit half be set independently of the other, but llvm thinks that sets the other half too (probably because this is the case on x86) and this is also an issue on in-tree AVR.

On the Z80 the HL 16-bit register consists of H and L 8-bit, and LD L,0 does not alter H. It is not until the subsequent LD H,0 that HL is fully defined.   The fix implements this.

The fix is verified on AVR.

## Root cause

`LiveVariables::HandlePhysRegUse` takes `FindLastPartialDef` (the last instruction defining *any one* sub-register) and stamps it `implicit-def $super`. The comment asserts "all sub-registers must have been defined" but the code never checks it. Two failures on independent pairs (Z80 HL, AVR r25r24):

1. A lone half-def (only `$l` written) is wrongly marked as defining the whole pair.
2. When the two halves are defined by separate instructions, only the later def gets `implicit-def $super`, so the earlier half's def looks dead — MachineCopyPropagation then deletes its still-live copy.

Correct on x86 (AL/AH cover AX); wrong where halves are independent.

## Reproducers (`llc -run-pass=livevars,dead-mi-elimination`)

zlfn's exact AVR case from the issue: pristine deletes the still-live `$r25 = MOVRdRr` (miscompile); fix keeps it.
```
$r25 = MOVRdRr $r22
$r24 = LDIRdK 42
$r20 = MOVRdRr $r25
RCALLk @use, implicit $r25r24, implicit $r20
```
Pristine: `$r24 = LDIRdK 42, implicit-def $r25r24` → DeadMI deletes `$r25 = MOVRdRr`, later read undefined. Fix: `$r24 = LDIRdK 42, implicit $r25, implicit-def $r25r24` → copy survives.

## Fix (`LiveVariables.cpp`, two parts)

1. Add `implicit-def $super` only when every leaf sub-register has a def.
2. Add an `implicit` use of sibling halves defined by an earlier instruction so copy-prop keeps them live.

x86 output unchanged; produces fewer-or-equal spurious implicit-defs.

## Confirmed targets

| Target | Pair | Status |
|---|---|---|
| Out-of-tree DSP | ACC=AH:AL | original reporter @hongjia |
| Z80 | HL/DE/BC | this report, @zlfn |
| AVR | r25r24 | @zlfn |
