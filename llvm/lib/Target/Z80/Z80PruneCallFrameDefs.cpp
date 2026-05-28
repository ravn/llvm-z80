//===-- Z80PruneCallFrameDefs.cpp - Prune ADJCALLSTACKUP implicit defs ----===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// ravn/llvm-z80 #197.  ADJCALLSTACKUP is declared `Defs = [SP, HL, A]` in the
// .td -- the *worst case* over all its expansions in
// Z80FrameLowering::eliminateCallFramePseudoInstr:
//
//   AdjAmount == 0            -> erased (no register effect)
//   SM83 && AdjAmount <= 127  -> ADD SP,e          (no HL/A clobber)
//   AdjAmount <= 16           -> POP AF x N         (clobbers A only)
//   else                      -> LD HL,n; ADD HL,SP; LD SP,HL  (clobbers HL only)
//
// Carrying HL+A on *every* instance is safe but pessimistic: the register
// allocator believes each call-frame cleanup clobbers HL and A.  For a float
// builtin returning its 32-bit result in DE:HL, greedy then marks the CALL's
// $hl result def *dead* (it looks clobbered by the following ADJCALLSTACKUP
// before the copy that consumes it).  When that ADJCALLSTACKUP is later erased
// (AdjAmount == 0) the phantom $hl def vanishes and the consuming COPY/SPILL
// reads an undefined $hl -- the dominant -verify-machineinstrs failure across
// the float corpus.
//
// This pre-RA pass prunes each ADJCALLSTACKUP's implicit-def $hl / $a operands
// down to the registers its own AdjAmount actually clobbers, so the allocator
// sees accurate liveness (HL/A allocatable across non-clobbering cleanups; the
// CALL's result def no longer falsely dead).  The AdjAmount is final by this
// point, and the prune is in the safe direction -- a def is only removed when
// the expansion provably does not write that register, mirroring the existing
// adjCallStackUpClobbersReg() logic in Z80RegisterInfo.cpp.  Runs after the
// machine scheduler and before greedy.
//
//===----------------------------------------------------------------------===//

#include "Z80PruneCallFrameDefs.h"
#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80InstrInfo.h"
#include "Z80Subtarget.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "z80-prune-callframe-defs"

namespace {

class Z80PruneCallFrameDefs : public MachineFunctionPass {
public:
  static char ID;

  Z80PruneCallFrameDefs() : MachineFunctionPass(ID) {
    initializeZ80PruneCallFrameDefsPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Z80 Prune Call-Frame Pseudo Defs";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

char Z80PruneCallFrameDefs::ID = 0;

INITIALIZE_PASS(Z80PruneCallFrameDefs, DEBUG_TYPE,
                "Z80 Prune Call-Frame Pseudo Defs", false, false)

bool Z80PruneCallFrameDefs::runOnMachineFunction(MachineFunction &MF) {
  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  const bool IsSM83 = STI.hasSM83();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      if (MI.getOpcode() != Z80::ADJCALLSTACKUP)
        continue;

      // Mirror adjCallStackUpClobbersReg (Z80RegisterInfo.cpp): the actual
      // clobber set of this specific instance, from its AdjAmount.
      int64_t AdjAmount = MI.getOperand(0).getImm() - MI.getOperand(1).getImm();
      bool ClobbersHL = false, ClobbersA = false;
      if (AdjAmount != 0 && !(IsSM83 && AdjAmount <= 127)) {
        if (AdjAmount <= 16)
          ClobbersA = true; // POP AF
        else
          ClobbersHL = true; // LD HL,n; ADD HL,SP; LD SP,HL
      }

      // Remove the implicit-def $hl / $a operands this instance does not make.
      // Walk backward so operand indices stay valid across removal.
      for (unsigned I = MI.getNumOperands(); I-- > 0;) {
        const MachineOperand &MO = MI.getOperand(I);
        if (!MO.isReg() || !MO.isDef() || !MO.isImplicit())
          continue;
        if ((MO.getReg() == Z80::HL && !ClobbersHL) ||
            (MO.getReg() == Z80::A && !ClobbersA)) {
          MI.removeOperand(I);
          Changed = true;
        }
      }
    }
  }

  return Changed;
}

MachineFunctionPass *llvm::createZ80PruneCallFrameDefsPass() {
  return new Z80PruneCallFrameDefs();
}
