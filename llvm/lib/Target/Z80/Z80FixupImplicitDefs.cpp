//===-- Z80FixupImplicitDefs.cpp - Fix super-register implicit-defs
//--------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Mitigation for LLVM bug: https://github.com/llvm/llvm-project/issues/156428
//
// Background
// ----------
// The LiveVariables pass (HandlePhysRegUse / FindLastPartialDef) adds
// `implicit-def $hl` to instructions that only define a sub-register
// such as L.  This is an LLVM core bug that affects any backend where
// sub-registers of a register pair are independent (i.e. writing one
// half does NOT clobber the other).  On Z80, writing L does NOT modify
// H -- the two halves of every register pair are independent.
//
// Why this matters
// ----------------
// MachineCopyPropagation (MCP) collects every `isDef()` operand into a
// Defs list and calls `clobberRegister()` for each one.  When MCP sees:
//
//   $h = COPY $e                    ; MCP tracks this copy
//   LD_L_n 0, implicit-def $l, implicit-def $hl
//
// it treats HL as clobbered, which overlaps with H, so it removes the
// $h copy as dead.  This is a miscompilation -- H still holds the value
// from COPY.  The same pattern affects DE (275 instances) and HL (19
// instances) across the test suite.
//
// What this pass does
// -------------------
// For each instruction, we collect the "intended" sub-register defs:
// MCInstrDesc static implicit defs and explicit physical register def
// operands that have a super-register.  If a runtime-added implicit-def
// is a super-register of any intended sub-register def, we remove it.
// This tells MCP that only the sub-register is defined, not the pair.
//
// The pass runs once before the first MCP (via addPostRewrite), which is
// sufficient because no later pass re-adds these implicit-defs.
//
// This workaround should be removed once the upstream bug is fixed.
//
//===----------------------------------------------------------------------===//

#include "Z80FixupImplicitDefs.h"
#include "Z80.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/MC/MCInstrDesc.h"

using namespace llvm;

#define DEBUG_TYPE "z80-fixup-implicit-defs"

namespace {

class Z80FixupImplicitDefs : public MachineFunctionPass {
public:
  static char ID;

  Z80FixupImplicitDefs() : MachineFunctionPass(ID) {
    initializeZ80FixupImplicitDefsPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Z80 Fixup Super-Register Implicit Defs";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().setNoVRegs();
  }
};

} // end anonymous namespace

char Z80FixupImplicitDefs::ID = 0;

INITIALIZE_PASS(Z80FixupImplicitDefs, DEBUG_TYPE,
                "Z80 Fixup Super-Register Implicit Defs", false, false)

/// Collect the set of physical sub-registers that the instruction is
/// designed to define.  This includes:
///  - MCInstrDesc static implicit defs (e.g. L from LD_L_n)
///  - Explicit physical register def operands (e.g. $l from COPY)
/// We exclude super-register defs (HL, DE, etc.) because those are
/// exactly what we want to remove if added spuriously by LiveVariables.
static SmallSet<MCPhysReg, 4>
getIntendedSubRegDefs(const MachineInstr &MI, const TargetRegisterInfo &TRI) {
  SmallSet<MCPhysReg, 4> Defs;

  // Static implicit defs from MCInstrDesc.
  for (MCPhysReg Reg : MI.getDesc().implicit_defs())
    Defs.insert(Reg);

  // Explicit physical register def operands (e.g. from COPY after
  // VirtRegRewriter).  Only collect sub-registers of register pairs,
  // not the pairs themselves — we want to detect when a sub-register
  // def has a spurious super-register implicit-def attached.
  for (const MachineOperand &MO : MI.operands()) {
    if (!MO.isReg() || !MO.isDef() || MO.isImplicit())
      continue;
    Register Reg = MO.getReg();
    if (Reg.isPhysical() && TRI.superregs(Reg.asMCReg()).begin() !=
                                TRI.superregs(Reg.asMCReg()).end())
      Defs.insert(Reg.asMCReg());
  }

  return Defs;
}

bool Z80FixupImplicitDefs::runOnMachineFunction(MachineFunction &MF) {
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      SmallSet<MCPhysReg, 4> SubRegDefs = getIntendedSubRegDefs(MI, *TRI);
      if (SubRegDefs.empty())
        continue;

      // Walk operands in reverse so removal doesn't invalidate indices.
      for (int I = MI.getNumOperands() - 1; I >= 0; --I) {
        MachineOperand &MO = MI.getOperand(I);
        if (!MO.isReg() || !MO.isDef() || !MO.isImplicit())
          continue;

        MCPhysReg Reg = MO.getReg().asMCReg();

        // Keep operands that are part of the intended defs.
        if (SubRegDefs.count(Reg))
          continue;

        // Check if this implicit-def is a super-register of any intended
        // sub-register def.  On Z80, this catches HL added by LiveVariables
        // when only L (or H) is defined.  Same for BC/DE/IX/IY pairs.
        bool IsSuperOfSubRegDef = false;
        for (MCPhysReg SD : SubRegDefs) {
          if (TRI->isSuperRegister(SD, Reg)) {
            IsSuperOfSubRegDef = true;
            break;
          }
        }

        if (!IsSuperOfSubRegDef)
          continue;

        LLVM_DEBUG(dbgs() << "Z80FixupImplicitDefs: removing implicit-def "
                          << printReg(Reg, TRI) << " from: " << MI);
        MI.removeOperand(I);
        Changed = true;
      }
    }
  }

  // Sub-register liveness repair for KILL pseudos (test_12 popcount16).
  // The rewriter can emit `dead $pair = KILL $pair, implicit-def $sub` that
  // revives only ONE half of a register pair even when the sibling half is
  // still read later in the SAME block.  In popcount16's loop the i16 `val`
  // gives `dead $hl = KILL $hl, implicit-def $l` (for the `val & 1` low-byte
  // read) followed by `LSHR16 $hl` (the `val >>= 1`, which needs $h); the
  // dead $hl def kills $h, so LSHR16 -> SRL_H reads an undefined $h.
  // fullyRecomputeLiveIns (below) only fixes block live-ins, not this
  // intra-block gap.  Revive any sibling sub-register of a KILL-defined pair
  // that is read downstream in the block before being redefined.  KILL emits
  // no code, so adding an implicit-def is metadata-only (byte-identical).
  for (MachineBasicBlock &MBB : MF) {
    for (MachineBasicBlock::iterator It = MBB.begin(), E = MBB.end(); It != E;
         ++It) {
      MachineInstr &MI = *It;
      if (!MI.isKill())
        continue;
      SmallVector<MCPhysReg, 2> PairDefs;
      SmallSet<MCPhysReg, 4> AlreadyDef;
      for (const MachineOperand &MO : MI.operands()) {
        if (!MO.isReg() || !MO.isDef() || !MO.getReg().isPhysical())
          continue;
        MCPhysReg R = MO.getReg().asMCReg();
        AlreadyDef.insert(R);
        if (TRI->subregs(R).begin() != TRI->subregs(R).end())
          PairDefs.push_back(R); // a register-pair super-register
      }
      for (MCPhysReg Pair : PairDefs) {
        for (MCPhysReg Sub : TRI->subregs(Pair)) {
          if (AlreadyDef.count(Sub))
            continue;
          bool ReadBeforeRedef = false;
          for (auto Scan = std::next(It); Scan != E; ++Scan) {
            if (Scan->readsRegister(Sub, TRI)) {
              ReadBeforeRedef = true;
              break;
            }
            if (Scan->definesRegister(Sub, TRI))
              break; // redefined before any read: genuinely dead at the KILL
          }
          if (!ReadBeforeRedef)
            continue;
          MI.addOperand(MachineOperand::CreateReg(Sub, /*isDef=*/true,
                                                  /*isImp=*/true));
          Changed = true;
          LLVM_DEBUG(dbgs() << "Z80FixupImplicitDefs: revived implicit-def "
                            << printReg(Sub, TRI) << " on KILL: " << MI);
        }
      }
    }
  }

  // After removing the spurious super-register implicit-defs above, recompute
  // block live-ins for the whole function.  LiveVariables (#156428) also leaves
  // *inconsistent block live-ins* for independent register-pair halves -- a
  // half can be over-claimed live-in (so a dead value's later read is reported
  // undef) or under-provided by a predecessor (so a live value's read is
  // reported undef).  LivePhysRegs is sub-register/reg-unit accurate, so a
  // fixpoint recompute (now that the implicit-defs are correct) yields
  // consistent live-ins.  This clears the residual -verify-machineinstrs
  // "undefined physical register" reads (AND A / SRL H / spilled counter /
  // IX-frame stores) and feeds correct liveness to the SP-relative spill
  // expander in PEI.  Metadata-only; runs before PEI.
  SmallVector<MachineBasicBlock *> Blocks;
  for (MachineBasicBlock &MBB : MF)
    Blocks.push_back(&MBB);
  fullyRecomputeLiveIns(Blocks);

  return Changed;
}

MachineFunctionPass *llvm::createZ80FixupImplicitDefsPass() {
  return new Z80FixupImplicitDefs;
}
