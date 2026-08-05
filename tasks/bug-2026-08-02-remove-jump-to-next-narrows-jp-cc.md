# Bug: Z80RemoveJumpToNext narrowed a range-widened JP_cc back to an out-of-range JR_cc

Date: 2026-08-02
Fixed in: `llvm/lib/Target/Z80/Z80RemoveJumpToNext.cpp`
Lit test: `llvm/test/CodeGen/Z80/remove-jump-to-next.mir` (`cond_then_fallthrough`)

## Symptom

z88dk classic stdlib test `test/suites/stdlib/qsort.c` compiled with
`+cpm -compiler=llvmz80 -O2` was rejected by the external z80asm stage:

    qsort.c:428: error: integer range: -$ac

`-$ac` = -172. The offending instruction was a conditional back-edge
`jr c,.LBB2_5` at a ~170-byte distance, far outside JR's +/-127 range.

## Root cause (bisected with `-print-after-all`)

The `jr c` was NOT a BranchRelaxation miss. The MIR pipeline order in
`addPreEmitPass` is:

    BranchRelaxation -> Z80BranchCleanup -> Z80HighByteFirstBranch
      -> Z80ExpandPseudo -> Z80RemoveJumpToNext

At -O2 `Z80HighByteFirstBranch` (an AI-authored speed pass, #250 lever 2)
splits the 16-bit loop-exit compare and correctly emits the low-byte
back-edge as `JP_C_nn` (it runs after relaxation and picks JP for the far
target). Dumps confirmed the branch was still `JP_C_nn %bb.5` after both
hbf and Z80ExpandPseudo.

`Z80RemoveJumpToNext` (also AI-authored) then handled the
"conditional + trailing unconditional-to-fall-through" shape with:

    TII.removeBranch(MBB);
    TII.insertBranch(MBB, TBB, /*FBB=*/nullptr, Cond, DL);

`Z80InstrInfo::insertBranch` re-materialises a conditional branch in its
SHORT `JR_cc` form. Because this pass runs AFTER BranchRelaxation, that
short JR is never range-checked again -> the range-widened `JP_C_nn`
was clobbered back into an out-of-range `jr c`.

## Fix

Erase ONLY the trailing unconditional branch; leave the conditional
instruction byte-for-byte. Never removeBranch()+insertBranch() in a
post-relaxation peephole.

    MachineBasicBlock::iterator LastBr = MBB.getLastNonDebugInstr();
    if (LastBr == MBB.end() || !LastBr->isUnconditionalBranch())
      continue;
    LastBr->eraseFromParent();

Size/behaviour: neutral-or-correct. Branches entering this pass are
already optimally sized by BranchRelaxation/BranchCleanup (JR in range,
JP out of range) or deliberately JP (hbf hot back-edge). Preserving them
is correct; the old narrowing was only ever a size win by accident and a
miscompile at range boundaries.

## Verification

- qsort.c raw asm: low-byte back-edge now `jp c,.LBB2_5`, 0 out-of-range jr.
- z88dk stdlib + string suites: compile+link GREEN under llvmz80.
- Dedicated runtime tests (`test/clang/runtime_qsort.sh`,
  `runtime_bsearch.sh`) PASS in ntvcm with an `__smallc` comparator
  (comparator function-pointer calling convention intact).
- Z80 lit suite: 209 passed, 5 XFAIL, 0 unexpected (208 exact-asm tests
  unchanged -> no unintended codegen drift).

## General lesson

Any Z80 MI peephole that runs after BranchRelaxation must preserve a
branch instruction's chosen opcode. removeBranch()+insertBranch()
re-materialises conditionals in the short JR form and silently reverts
range widening. Prefer surgical eraseFromParent() of the exact redundant
terminator.
