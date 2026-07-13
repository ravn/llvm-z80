//===-- Z80PostRACompareMerge.cpp - Remove redundant flag-setting ops ------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// After register allocation, the backend often emits redundant OR A
// instructions to set the Z flag before a conditional branch, even when the
// preceding ALU instruction already set the Z flag correctly.
//
// Example:
//   xor e        ; already sets Z flag
//   or a         ; redundant — removed by this pass
//   jr z, .label
//
// This pass removes OR_A when the Z flag is already valid from a preceding
// instruction that defines FLAGS and operates on A.
//
// Cross-block: if ALL predecessors end with Z valid for A (no flag/A
// clobbering between the last flag-setter and block exit), the successor
// can start with ZFlagValid = true. This catches loop headers where both
// the entry and back-edge set Z for A.
//
//===----------------------------------------------------------------------===//

#include "Z80PostRACompareMerge.h"
#include "MCTargetDesc/Z80MCTargetDesc.h"

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "z80-post-ra-compare-merge"

class Z80PostRACompareMerge : public MachineFunctionPass {
public:
  static char ID;
  Z80PostRACompareMerge() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override { return "Z80PostRACompareMerge"; }
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char Z80PostRACompareMerge::ID = 0;

INITIALIZE_PASS(Z80PostRACompareMerge, DEBUG_TYPE,
                "Z80 Post-RA Redundant Compare Removal", false, false)

/// Returns true if MI defines the FLAGS register.
static bool definesFlags(const MachineInstr &MI) {
  for (const MachineOperand &MO : MI.operands()) {
    if (MO.isReg() && MO.isDef() && MO.getReg() == Z80::FLAGS)
      return true;
  }
  if (MI.getDesc().hasImplicitDefOfPhysReg(Z80::FLAGS))
    return true;
  return false;
}

/// Returns true if MI modifies the A register without setting FLAGS.
static bool modifiesAWithoutFlags(const MachineInstr &MI) {
  bool ModifiesA = false;
  for (const MachineOperand &MO : MI.operands()) {
    if (MO.isReg() && MO.isDef() && MO.getReg() == Z80::A)
      ModifiesA = true;
  }
  if (MI.getDesc().hasImplicitDefOfPhysReg(Z80::A))
    ModifiesA = true;
  return ModifiesA && !definesFlags(MI);
}

/// Returns true if MI sets the Z flag to reflect (A == 0) — i.e., the
/// instruction defines A and FLAGS together, and the Z flag of the result
/// is the standard "result-is-zero" indication. This is what `OR A`
/// computes, so any subsequent OR A is redundant.
///
/// CP r / CP n / CP (HL) / CP (IX+d) do NOT qualify: they leave A
/// unchanged and set Z based on (A - operand), i.e. (A == operand).
/// A subsequent OR A would test (A == 0), which is a different question.
/// Treating CP as Z-for-A here was a long-standing miscompile.
static bool setsZForA(const MachineInstr &MI) {
  if (!definesFlags(MI))
    return false;
  // POP_AF defines both $a and $flags, but the flags are the value RESTORED
  // from the stack (whatever PUSH_AF saved earlier), NOT a Z-reflects-(A==0)
  // result.  A subsequent `OR A` is therefore NOT redundant after POP_AF.
  // Example (from a static-stack reload-via-A that preserves A around the
  // clobber):  call __umodqi3 (A=i%7); push af; ld a,(slot); ld d,a; pop af;
  // or a; jr nz  -- the `or a` re-derives Z from A=i%7 and must survive.
  if (MI.getOpcode() == Z80::POP_AF)
    return false;
  for (const MachineOperand &MO : MI.operands()) {
    if (MO.isReg() && MO.isDef() && MO.getReg() == Z80::A)
      return true;
  }
  if (MI.getDesc().hasImplicitDefOfPhysReg(Z80::A))
    return true;
  return false;
}

/// Scan a block and return the ZFlagValid state at its exit.
/// Also optionally collect OR_A instructions to erase when ZFlagValid.
static bool scanBlock(MachineBasicBlock &MBB, bool ZFlagValid,
                      SmallVectorImpl<MachineInstr *> *ToErase) {
  for (MachineInstr &MI : MBB) {
    if (MI.getOpcode() == Z80::OR_A && ZFlagValid) {
      if (ToErase)
        ToErase->push_back(&MI);
      continue;
    }

    if (MI.isCall() || MI.isReturn() || MI.isInlineAsm() || MI.isPseudo()) {
      ZFlagValid = false;
      continue;
    }

    // Branches don't clobber flags — they read them. Don't reset here;
    // flags remain valid for the exit state (fall-through to successor).
    if (MI.isBranch())
      continue;

    if (definesFlags(MI)) {
      ZFlagValid = setsZForA(MI);
      continue;
    }

    if (modifiesAWithoutFlags(MI))
      ZFlagValid = false;
  }
  return ZFlagValid;
}

bool Z80PostRACompareMerge::runOnMachineFunction(MachineFunction &MF) {
  bool Changed = false;

  // Pass 1: compute ZFlagValidAtExit for each block (no erasure).
  DenseMap<MachineBasicBlock *, bool> ExitValid;
  for (MachineBasicBlock &MBB : MF)
    ExitValid[&MBB] = scanBlock(MBB, /*ZFlagValid=*/false, nullptr);

  // Pass 2: for each block, check if ALL predecessors have ZFlagValid
  // at exit. If so, start with ZFlagValid = true. Then erase redundant OR A.
  for (MachineBasicBlock &MBB : MF) {
    bool EntryValid = false;
    if (!MBB.pred_empty()) {
      EntryValid = true;
      for (MachineBasicBlock *Pred : MBB.predecessors()) {
        if (!ExitValid[Pred]) {
          EntryValid = false;
          break;
        }
      }
    }

    SmallVector<MachineInstr *, 4> ToErase;
    scanBlock(MBB, EntryValid, &ToErase);

    for (MachineInstr *MI : ToErase) {
      LLVM_DEBUG(dbgs() << "  Removing redundant: " << *MI);
      MI->eraseFromParent();
      Changed = true;
    }
  }

  return Changed;
}

MachineFunctionPass *llvm::createZ80PostRACompareMerge() {
  return new Z80PostRACompareMerge();
}
