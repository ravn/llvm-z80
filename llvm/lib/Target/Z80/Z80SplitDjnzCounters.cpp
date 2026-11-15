//===-- Z80SplitDjnzCounters.cpp - Split DJNZ-loop counter live ranges ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// In countdown loops, greedy register allocation may allocate loop counters
// to registers other than B (such as C or D) because the register coalescer
// merges entry copies or because other virtual registers take B first.
//
// On Z80, DJNZ requires the down-counter to reside strictly in register B.
// This pass runs post-coalescer, immediately before greedy register allocation.
// For eligible countdown loops, it creates a fresh virtual register constrained
// to the BReg single-register class, inserts a preheader copy, and renames
// in-loop references to the fresh virtual register. This forces greedy register
// allocation to assign physical register B to the counter, enabling DJNZ folding.
//
//===----------------------------------------------------------------------===//

#include "Z80SplitDjnzCounters.h"
#include "Z80.h"
#include "Z80InstrInfo.h"
#include "Z80Subtarget.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "z80-split-djnz-counters"

namespace {

class Z80SplitDjnzCounters : public MachineFunctionPass {
public:
  static char ID;

  Z80SplitDjnzCounters() : MachineFunctionPass(ID) {
    initializeZ80SplitDjnzCountersPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Z80 Split DJNZ-loop counter live ranges";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

char Z80SplitDjnzCounters::ID = 0;

INITIALIZE_PASS(Z80SplitDjnzCounters, DEBUG_TYPE,
                "Z80 Split DJNZ-loop counter live ranges", false, false)

/// Find an 8-bit virtual register in GR8 that is decremented in Latch.
static Register findCounter8VReg(MachineBasicBlock &Latch,
                                 const MachineRegisterInfo &MRI) {
  for (MachineInstr &MI : Latch) {
    if (MI.getOpcode() != Z80::DEC_r)
      continue;
    if (MI.getNumOperands() < 1 || !MI.getOperand(0).isReg())
      continue;
    Register Reg = MI.getOperand(0).getReg();
    if (!Reg.isVirtual() || MRI.getRegClass(Reg) != &Z80::GR8RegClass)
      continue;
    return Reg;
  }
  return Register();
}

/// Find the target loop header branched or fallen through to by Latch.
static MachineBasicBlock *findLoopHeader(MachineBasicBlock &Latch) {
  for (const MachineInstr &Term : Latch.terminators()) {
    unsigned Opc = Term.getOpcode();
    if (Opc == Z80::JR_NZ_e || Opc == Z80::JP_NZ_nn) {
      if (Term.getNumOperands() > 0 && Term.getOperand(0).isMBB())
        return Term.getOperand(0).getMBB();
    }
    // Also handle inverted latches before rotation (JR_Z_e to exit, fallthrough to header).
    if (Opc == Z80::JR_Z_e || Opc == Z80::JP_Z_nn) {
      auto Next = std::next(Latch.getIterator());
      if (Next != Latch.getParent()->end())
        return &*Next;
    }
  }
  return nullptr;
}

/// Find unique preheader entering Header from outside the loop.
static MachineBasicBlock *findUniquePreheader(MachineBasicBlock &Header,
                                              MachineBasicBlock &Latch) {
  MachineBasicBlock *Preheader = nullptr;
  for (MachineBasicBlock *Pred : Header.predecessors()) {
    if (Pred == &Latch || Pred == &Header)
      continue;
    if (Preheader)
      return nullptr; // Multiple entry edges
    Preheader = Pred;
  }
  return Preheader;
}

/// Collect basic blocks that belong to the loop from Header to Latch.
static void collectLoopBlocks(MachineBasicBlock &Header,
                              MachineBasicBlock &Latch,
                              SmallPtrSetImpl<MachineBasicBlock *> &LoopBlocks) {
  LoopBlocks.insert(&Header);
  LoopBlocks.insert(&Latch);
  SmallVector<MachineBasicBlock *, 8> Worklist;
  Worklist.push_back(&Header);

  while (!Worklist.empty()) {
    MachineBasicBlock *MBB = Worklist.pop_back_val();
    for (MachineBasicBlock *Succ : MBB->successors()) {
      if (Succ == &Header)
        continue;
      if (LoopBlocks.insert(Succ).second)
        Worklist.push_back(Succ);
    }
  }
}

/// Check if From can reach To in the CFG.
static bool canReach(const MachineBasicBlock *From,
                     const MachineBasicBlock *To) {
  SmallPtrSet<const MachineBasicBlock *, 16> Visited;
  SmallVector<const MachineBasicBlock *, 16> Worklist;
  Worklist.push_back(From);
  while (!Worklist.empty()) {
    const MachineBasicBlock *Cur = Worklist.pop_back_val();
    if (Cur == To)
      return true;
    for (const MachineBasicBlock *Succ : Cur->successors()) {
      if (Visited.insert(Succ).second)
        Worklist.push_back(Succ);
    }
  }
  return false;
}

bool Z80SplitDjnzCounters::runOnMachineFunction(MachineFunction &MF) {
  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  // SM83 lacks the DJNZ instruction.
  if (STI.hasSM83())
    return false;

  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetInstrInfo &TII = *STI.getInstrInfo();
  bool Changed = false;

  for (MachineBasicBlock &Latch : MF) {
    Register Counter = findCounter8VReg(Latch, MRI);
    if (!Counter.isValid())
      continue;

    MachineBasicBlock *Header = findLoopHeader(Latch);
    if (!Header || !Header->isPredecessor(&Latch))
      continue;

    MachineBasicBlock *Preheader = findUniquePreheader(*Header, Latch);
    if (!Preheader)
      continue;

    // Collect all loop blocks between Header and Latch.
    SmallPtrSet<MachineBasicBlock *, 8> LoopBlocks;
    collectLoopBlocks(*Header, Latch, LoopBlocks);

    // Guard: if Counter has any use strictly post-loop (blocks that cannot
    // reach Header), splitting would leave the reader with the initial value.
    bool HasPostLoopUse = false;
    for (const MachineInstr &UseMI : MRI.use_nodbg_instructions(Counter)) {
      const MachineBasicBlock *UseMBB = UseMI.getParent();
      if (!LoopBlocks.contains(const_cast<MachineBasicBlock *>(UseMBB)) &&
          !canReach(UseMBB, Header)) {
        HasPostLoopUse = true;
        break;
      }
    }
    if (HasPostLoopUse)
      continue;

    // Create fresh virtual register in BReg.
    Register NewCounter = MRI.createVirtualRegister(&Z80::BRegRegClass);

    // Insert COPY of Counter into NewCounter at Preheader.
    auto InsertBefore = Preheader->getFirstTerminator();
    DebugLoc DL = (InsertBefore != Preheader->end())
                      ? InsertBefore->getDebugLoc()
                      : DebugLoc();
    BuildMI(*Preheader, InsertBefore, DL, TII.get(TargetOpcode::COPY),
            NewCounter)
        .addReg(Counter);

    // Rename all in-loop references of Counter to NewCounter.
    for (MachineBasicBlock *MBB : LoopBlocks) {
      for (MachineInstr &MI : *MBB) {
        for (MachineOperand &MO : MI.operands()) {
          if (MO.isReg() && MO.getReg() == Counter)
            MO.setReg(NewCounter);
        }
      }
    }

    Changed = true;
  }

  return Changed;
}

MachineFunctionPass *llvm::createZ80SplitDjnzCountersPass() {
  return new Z80SplitDjnzCounters;
}
