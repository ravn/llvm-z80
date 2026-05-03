//===-- Z80SplitDjnzCounters.cpp - Split DJNZ-loop counter live ranges ---===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// #94 / #98 — sequential DJNZ-eligible loops at the same depth produce
// only one DJNZ before this pass.  Investigation in
// tasks/regalloc-sequential-djnz-investigation.md identified the cause:
// the register coalescer merges entry-edge PHI COPYs into the counter
// vregs, extending each counter's live range backwards from its loop's
// preheader to the function entry.  Two sequential loops then have
// counter live ranges that genuinely overlap on B.
//
// This pass runs after the register coalescer.  For each MBB that is a
// DJNZ-eligible self-loop (the back-edge JR_NZ branches to itself, and
// the `$a = COPY %v; DEC_A; %v = COPY $a` triplet appears in the body),
// it identifies the counter vreg and, if that vreg is used or defined
// outside the loop MBB, inserts a fresh COPY at the loop preheader and
// renumbers the in-loop uses to the new vreg.  The new vreg is created
// in the BReg single-register class — this constrains it to B by
// class, bypassing greedy's copy-elimination heuristic which would
// otherwise route the counter to %30's physreg.  Sequential loops thus
// independently get B for their counter (sharing B sequentially across
// non-overlapping ranges).
//
// Cost: one COPY per DJNZ-eligible loop preheader.  When the source
// vreg also lands in B (rare in practice), the COPY is `LD B, B` and
// is eliminated by post-RA copy propagation.  Otherwise it emits as
// `LD B, r` (1 byte) — paid back by the per-iteration DJNZ savings
// (1 byte vs DEC r; JR NZ).
//
//===----------------------------------------------------------------------===//

#include "Z80SplitDjnzCounters.h"
#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80InstrInfo.h"
#include "Z80Subtarget.h"

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

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

// A DJNZ-eligible self-loop MBB ends with `JR_NZ_e <self>` or
// `JP_NZ_nn <self>`.  The body contains a `COPY $a, %v; DEC_A;
// COPY %v, $a` triplet (with optional intermediate ops); %v is the
// counter vreg.  Returns the counter vreg or Register() if not found.
static Register findCounterVReg(MachineBasicBlock &MBB,
                                const MachineRegisterInfo &MRI) {
  for (auto It = MBB.begin(), E = MBB.end(); It != E; ++It) {
    // Match `$a = COPY %v` (gr8).
    if (It->getOpcode() != TargetOpcode::COPY)
      continue;
    if (!It->getOperand(0).isReg() ||
        It->getOperand(0).getReg() != Z80::A)
      continue;
    if (!It->getOperand(1).isReg())
      continue;
    Register V = It->getOperand(1).getReg();
    if (!V.isVirtual())
      continue;
    if (!Z80::GR8RegClass.hasSubClassEq(MRI.getRegClass(V)))
      continue;
    // Match `DEC_A` next (skipping meta).
    auto J = std::next(It);
    while (J != E && J->isMetaInstruction())
      ++J;
    if (J == E || J->getOpcode() != Z80::DEC_A)
      continue;
    // Find `%v = COPY $a` somewhere in the rest of the MBB.
    for (auto K = std::next(J); K != E; ++K) {
      if (K->getOpcode() != TargetOpcode::COPY)
        continue;
      if (!K->getOperand(0).isReg() ||
          K->getOperand(0).getReg() != V)
        continue;
      if (!K->getOperand(1).isReg() ||
          K->getOperand(1).getReg() != Z80::A)
        continue;
      return V;
    }
  }
  return Register();
}

// True iff MBB ends with a self-back-edge conditional NZ branch.
static bool isSelfBackEdgeNZLoop(const MachineBasicBlock &MBB) {
  for (const MachineInstr &Term : MBB.terminators()) {
    unsigned Opc = Term.getOpcode();
    if (Opc != Z80::JR_NZ_e && Opc != Z80::JP_NZ_nn)
      continue;
    if (Term.getNumOperands() == 0 || !Term.getOperand(0).isMBB())
      continue;
    if (Term.getOperand(0).getMBB() == &MBB)
      return true;
  }
  return false;
}

// Return the predecessor MBB that is NOT MBB itself, or nullptr if
// MBB has either zero non-self predecessors or multiple of them
// (we only handle the simple single-preheader shape).
static MachineBasicBlock *findUniquePreheader(MachineBasicBlock &MBB) {
  MachineBasicBlock *Found = nullptr;
  for (MachineBasicBlock *Pred : MBB.predecessors()) {
    if (Pred == &MBB)
      continue;
    if (Found)
      return nullptr; // multiple non-self predecessors
    Found = Pred;
  }
  return Found;
}

// True iff Reg has any def OR use outside MBB.
static bool hasAnyRefOutsideMBB(Register Reg, MachineBasicBlock &MBB,
                                const MachineRegisterInfo &MRI) {
  for (const MachineInstr &MI : MRI.reg_nodbg_instructions(Reg)) {
    if (MI.getParent() != &MBB)
      return true;
  }
  return false;
}

// True iff Reg has any USE outside MBB (a def outside is fine — typically
// the PHI entry-edge def at the loop preheader, which we *want* to split
// across).  A use outside means the post-loop value is observed; renaming
// in-loop uses to a fresh vreg would leave the outside reader looking at
// the never-decremented entry value (correctness bug — counter_used_after
// in djnz-comprehensive.ll exposes this).
static bool hasUseOutsideMBB(Register Reg, MachineBasicBlock &MBB,
                             const MachineRegisterInfo &MRI) {
  for (const MachineInstr &MI : MRI.use_nodbg_instructions(Reg)) {
    if (MI.getParent() != &MBB)
      return true;
  }
  return false;
}

bool Z80SplitDjnzCounters::runOnMachineFunction(MachineFunction &MF) {
  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  // DJNZ is Z80-only.
  if (!STI.hasZ80())
    return false;

  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetInstrInfo &TII = *STI.getInstrInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    if (!isSelfBackEdgeNZLoop(MBB))
      continue;

    Register Counter = findCounterVReg(MBB, MRI);
    if (!Counter.isValid())
      continue;

    // Only split when the counter has a def outside (the PHI entry-edge
    // def we want to break the live range across) and NO use outside
    // (otherwise renaming in-loop uses would leave the outside reader
    // looking at the never-decremented entry value).
    if (!hasAnyRefOutsideMBB(Counter, MBB, MRI))
      continue;
    if (hasUseOutsideMBB(Counter, MBB, MRI))
      continue;

    MachineBasicBlock *Preheader = findUniquePreheader(MBB);
    if (!Preheader)
      continue;

    // Create a fresh vreg in the BReg single-register class.  This
    // forces greedy to allocate B (or split / spill) regardless of
    // its copy-elimination heuristic.  See investigation doc.
    Register NewCounter = MRI.createVirtualRegister(&Z80::BRegRegClass);

    auto InsertBefore = Preheader->getFirstTerminator();
    DebugLoc DL = (InsertBefore != Preheader->end())
                      ? InsertBefore->getDebugLoc()
                      : DebugLoc();
    BuildMI(*Preheader, InsertBefore, DL, TII.get(TargetOpcode::COPY),
            NewCounter)
        .addReg(Counter);

    LLVM_DEBUG(dbgs() << "Z80SplitDjnzCounters: inserting COPY of "
                      << printReg(Counter, MRI.getTargetRegisterInfo())
                      << " into BReg vreg "
                      << printReg(NewCounter, MRI.getTargetRegisterInfo())
                      << " at end of "
                      << printMBBReference(*Preheader) << "\n");

    // Renumber every operand inside MBB that references Counter to
    // reference NewCounter instead.  This includes the back-edge
    // def `Counter = COPY $a` which becomes `NewCounter = COPY $a`.
    for (MachineInstr &MI : MBB) {
      for (MachineOperand &MO : MI.operands()) {
        if (MO.isReg() && MO.getReg() == Counter)
          MO.setReg(NewCounter);
      }
    }
    Changed = true;
  }

  return Changed;
}

MachineFunctionPass *llvm::createZ80SplitDjnzCountersPass() {
  return new Z80SplitDjnzCounters;
}
