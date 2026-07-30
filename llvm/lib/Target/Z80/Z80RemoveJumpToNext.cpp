//===-- Z80RemoveJumpToNext.cpp - Drop jump-to-fall-through branch --------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// General late peephole: remove an unconditional branch whose target is the
// block's own layout-next successor -- a jump to the immediately-following
// address, which is identical to just falling through.
//
// LLVM's BranchFolding already does this, but it runs early in codegen; branches
// created AFTER it (Z80HighByteFirstBranch's split-block exit test, pseudo
// expansion) can leave a `jp <next>` behind that nothing removes -- verified on
// the sieve #250 kill loop (the redundant final `jp exit` survived to the
// binary, costing 3 bytes).  Running this pass LAST in addPreEmitPass (after
// Z80ExpandPseudo) catches jump-to-next from ANY source.
//
// Two shapes are handled via analyzeBranch:
//   * pure unconditional `JP/JR next`               -> remove the branch
//   * conditional + unconditional `JP_cc T; JP next` (T != next) where the
//     unconditional falls through -> keep only the conditional, drop the uncond
//
// Correctness-preserving and unconditional (a jump to the fall-through block is
// semantically a fall-through), so it is always on; a hidden flag disables it
// for debugging.
//
//===----------------------------------------------------------------------===//

#include "Z80RemoveJumpToNext.h"
#include "Z80.h"
#include "Z80InstrInfo.h"
#include "Z80Subtarget.h"

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "z80-remove-jump-to-next"

static cl::opt<bool> DisableZ80RemoveJumpToNext(
    "z80-disable-remove-jump-to-next", cl::init(false), cl::Hidden,
    cl::desc("Disable the Z80 remove-jump-to-fall-through peephole"));

namespace {

class Z80RemoveJumpToNext : public MachineFunctionPass {
public:
  static char ID;

  Z80RemoveJumpToNext() : MachineFunctionPass(ID) {
    initializeZ80RemoveJumpToNextPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Z80 Remove jump to fall-through";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

char Z80RemoveJumpToNext::ID = 0;

INITIALIZE_PASS(Z80RemoveJumpToNext, DEBUG_TYPE,
                "Z80 Remove jump to fall-through", false, false)

bool Z80RemoveJumpToNext::runOnMachineFunction(MachineFunction &MF) {
  if (DisableZ80RemoveJumpToNext)
    return false;
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    MachineBasicBlock *TBB = nullptr, *FBB = nullptr;
    SmallVector<MachineOperand, 4> Cond;
    // AllowModify=false: just inspect.
    if (TII.analyzeBranch(MBB, TBB, FBB, Cond, /*AllowModify=*/false))
      continue; // unanalyzable (e.g. the hbf hi-block's two conditionals) -- skip

    if (Cond.empty() && TBB && !FBB && MBB.isLayoutSuccessor(TBB)) {
      // Pure unconditional jump to the next block: pure fall-through.
      TII.removeBranch(MBB);
      Changed = true;
      LLVM_DEBUG(dbgs() << "z80-remove-jump-to-next: dropped uncond jump in "
                        << MBB.getName() << "\n");
    } else if (!Cond.empty() && TBB && FBB && MBB.isLayoutSuccessor(FBB)) {
      // Conditional branch to TBB, then unconditional to FBB, where FBB is the
      // fall-through: keep only the conditional, drop the redundant uncond.
      DebugLoc DL = MBB.findBranchDebugLoc();
      TII.removeBranch(MBB);
      TII.insertBranch(MBB, TBB, /*FBB=*/nullptr, Cond, DL);
      Changed = true;
      LLVM_DEBUG(dbgs() << "z80-remove-jump-to-next: dropped fall-through uncond in "
                        << MBB.getName() << "\n");
    }
  }
  return Changed;
}

MachineFunctionPass *llvm::createZ80RemoveJumpToNextPass() {
  return new Z80RemoveJumpToNext;
}
