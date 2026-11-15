//===-- Z80StorePhiSink.cpp - Sink G_STORE of G_PHI for CISC RMW ----------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// LLVM's generic mid-end (InstCombine and LICM) merges stores to identical
// addresses across conditional branches into a single G_STORE of a G_PHI at
// the join block (if.end), and promotes loop memory locations into loop-carried
// PHI registers with a store on every iteration.
//
// On Z80 (a CISC target with in-place memory operations like SET/RES/INC/DEC),
// this splits atomic load-modify-store sequences, forcing values through
// accumulator A and clobbering A across loop iterations.
//
// This pass reverses that transformation by sinking the G_STORE (and cloning
// a local G_LOAD) back into the predecessor branches when all incoming values
// are RMW operations on the same address.
//
//===----------------------------------------------------------------------===//

#include "Z80StorePhiSink.h"
#include "Z80.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "z80-store-phi-sink"

STATISTIC(NumStoresSunk, "Number of merged stores sunk into predecessor blocks");

using namespace llvm;

namespace {

class Z80StorePhiSink : public MachineFunctionPass {
public:
  static char ID;

  Z80StorePhiSink() : MachineFunctionPass(ID) {
    initializeZ80StorePhiSinkPass(*PassRegistry::getPassRegistry());
  }

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::IsSSA);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  bool trySinkStorePhi(MachineInstr &StoreMI, MachineRegisterInfo &MRI);
};

char Z80StorePhiSink::ID = 0;

} // end anonymous namespace

INITIALIZE_PASS(Z80StorePhiSink, DEBUG_TYPE,
                "Z80 Store PHI Sinking for CISC RMW", false, false)

MachineFunctionPass *llvm::createZ80StorePhiSinkPass() {
  return new Z80StorePhiSink();
}

bool Z80StorePhiSink::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  MachineRegisterInfo &MRI = MF.getRegInfo();
  bool Changed = false;

  // Process blocks. Use early inc range in case instructions are erased.
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : llvm::make_early_inc_range(MBB)) {
      if (MI.getOpcode() == TargetOpcode::G_STORE) {
        if (trySinkStorePhi(MI, MRI))
          Changed = true;
      }
    }
  }

  return Changed;
}

bool Z80StorePhiSink::trySinkStorePhi(MachineInstr &StoreMI,
                                     MachineRegisterInfo &MRI) {
  Register ValReg = StoreMI.getOperand(0).getReg();
  Register PtrReg = StoreMI.getOperand(1).getReg();

  if (!ValReg.isVirtual() || !PtrReg.isVirtual())
    return false;

  // Only handle 8-bit stores (Z80 in-place byte RMW operations: SET/RES/INC/DEC).
  if (MRI.getType(ValReg).getSizeInBits() != 8)
    return false;

  if (!StoreMI.hasOneMemOperand())
    return false;
  const MachineMemOperand &SMMO = **StoreMI.memoperands_begin();
  if (SMMO.isVolatile() || SMMO.getAddrSpace() != 0)
    return false;

  MachineInstr *PhiMI = MRI.getVRegDef(ValReg);
  if (!PhiMI || PhiMI->getOpcode() != TargetOpcode::G_PHI ||
      PhiMI->getParent() != StoreMI.getParent())
    return false;

  // Check uses of ValReg. It can be used by StoreMI, and optionally by a
  // loop-carried backedge PHI in a loop header.
  MachineInstr *LoopPhiMI = nullptr;
  unsigned NumUses = 0;
  for (MachineInstr &UseMI : MRI.use_nodbg_instructions(ValReg)) {
    ++NumUses;
    if (&UseMI == &StoreMI)
      continue;
    if (UseMI.getOpcode() == TargetOpcode::G_PHI) {
      LoopPhiMI = &UseMI;
    } else {
      return false; // Unknown extra use
    }
  }
  if (NumUses > 2 || (NumUses == 2 && !LoopPhiMI))
    return false;

  MachineBasicBlock *JoinBB = StoreMI.getParent();
  unsigned NumIncomings = (PhiMI->getNumOperands() - 1) / 2;
  if (NumIncomings < 2)
    return false;

  auto getUnderlyingPtr = [&](Register R) -> Register {
    while (R.isVirtual()) {
      MachineInstr *Def = MRI.getVRegDef(R);
      if (!Def || Def->getOpcode() != TargetOpcode::COPY)
        break;
      Register Src = Def->getOperand(1).getReg();
      if (!Src.isVirtual())
        break;
      R = Src;
    }
    return R;
  };
  Register BasePtr = getUnderlyingPtr(PtrReg);

  auto matchLoadOrLoopCarriedPhi =
      [&](Register CandReg) -> std::pair<MachineInstr *, MachineInstr *> {
    MachineInstr *Def = MRI.getVRegDef(CandReg);
    if (!Def)
      return {nullptr, nullptr};

    if (Def->getOpcode() == TargetOpcode::G_LOAD) {
      if (!Def->hasOneMemOperand())
        return {nullptr, nullptr};
      const MachineMemOperand &MMO = **Def->memoperands_begin();
      if (MMO.isVolatile() || MMO.getAddrSpace() != 0)
        return {nullptr, nullptr};
      if (getUnderlyingPtr(Def->getOperand(1).getReg()) != BasePtr)
        return {nullptr, nullptr};
      return {Def, nullptr};
    }

    if (Def->getOpcode() == TargetOpcode::G_PHI && Def == LoopPhiMI) {
      // Loop-carried PHI from LICM promotion:
      // Must have an incoming G_LOAD from BasePtr from preheader,
      // and an incoming ValReg from JoinBB.
      unsigned NumPhiIncomings = (Def->getNumOperands() - 1) / 2;
      MachineInstr *PreheaderLoad = nullptr;
      bool HasBackedgeVal = false;
      for (unsigned P = 0; P < NumPhiIncomings; ++P) {
        Register PhiInc = Def->getOperand(1 + 2 * P).getReg();
        if (PhiInc == ValReg) {
          HasBackedgeVal = true;
          continue;
        }
        MachineInstr *PhiIncDef = MRI.getVRegDef(PhiInc);
        if (PhiIncDef && PhiIncDef->getOpcode() == TargetOpcode::G_LOAD) {
          if (PhiIncDef->hasOneMemOperand() &&
              getUnderlyingPtr(PhiIncDef->getOperand(1).getReg()) == BasePtr) {
            PreheaderLoad = PhiIncDef;
          }
        }
      }
      if (PreheaderLoad && HasBackedgeVal)
        return {PreheaderLoad, Def};
    }

    return {nullptr, nullptr};
  };

  struct IncomingInfo {
    Register IncomingVal;
    MachineBasicBlock *PredBB;
    MachineInstr *OpMI;
    MachineInstr *LoadMI;
    MachineInstr *LoopPhi;
    Register LoadedReg;
  };
  SmallVector<IncomingInfo, 4> Incomings;
  SmallPtrSet<MachineInstr *, 4> OriginalLoads;

  for (unsigned I = 0; I < NumIncomings; ++I) {
    Register IncVal = PhiMI->getOperand(1 + 2 * I).getReg();
    MachineBasicBlock *PredBB = PhiMI->getOperand(2 + 2 * I).getMBB();

    // PredBB must have JoinBB as its sole successor (no critical edges).
    if (PredBB->succ_size() != 1 || *PredBB->succ_begin() != JoinBB)
      return false;

    if (!IncVal.isVirtual())
      return false;

    MachineInstr *OpMI = MRI.getVRegDef(IncVal);
    if (!OpMI || OpMI->getParent() != PredBB)
      return false;

    // Check if OpMI is an RMW candidate operation.
    unsigned Opc = OpMI->getOpcode();
    if (Opc != TargetOpcode::G_OR && Opc != TargetOpcode::G_AND &&
        Opc != TargetOpcode::G_XOR && Opc != TargetOpcode::G_ADD &&
        Opc != TargetOpcode::G_SUB)
      return false;

    Register Op0 = OpMI->getOperand(1).getReg();
    Register Op1 = OpMI->getOperand(2).getReg();

    auto matchLoadAndConst =
        [&](Register CandLoadReg,
            Register CandConstReg) -> std::tuple<MachineInstr *, MachineInstr *, Register> {
      MachineInstr *CDef = MRI.getVRegDef(CandConstReg);
      if (!CDef || CDef->getOpcode() != TargetOpcode::G_CONSTANT)
        return {nullptr, nullptr, Register()};

      auto [LDef, LoopPhi] = matchLoadOrLoopCarriedPhi(CandLoadReg);
      if (!LDef)
        return {nullptr, nullptr, Register()};

      return {LDef, LoopPhi, CandLoadReg};
    };

    auto [LoadMI, LoopPhi, LoadedReg] = matchLoadAndConst(Op0, Op1);
    if (!LoadMI)
      std::tie(LoadMI, LoopPhi, LoadedReg) = matchLoadAndConst(Op1, Op0);

    if (!LoadMI)
      return false;

    // Check that no store intervenes in PredBB before OpMI.
    bool HasInterveningStore = false;
    for (auto It = PredBB->begin(); It != MachineBasicBlock::iterator(OpMI);
         ++It) {
      if (It->mayStore() || It->hasUnmodeledSideEffects()) {
        HasInterveningStore = true;
        break;
      }
    }
    if (HasInterveningStore)
      return false;

    Incomings.push_back({IncVal, PredBB, OpMI, LoadMI, LoopPhi, LoadedReg});
    OriginalLoads.insert(LoadMI);
  }

  // If LoopPhiMI exists, verify all its uses are among Incomings' OpMIs.
  if (LoopPhiMI) {
    Register LoopPhiReg = LoopPhiMI->getOperand(0).getReg();
    for (MachineInstr &UseMI : MRI.use_nodbg_instructions(LoopPhiReg)) {
      bool IsKnownOpMI = llvm::any_of(
          Incomings, [&](const IncomingInfo &II) { return II.OpMI == &UseMI; });
      if (!IsKnownOpMI)
        return false;
    }
  }

  // All incoming paths are confirmed RMW operations on BasePtr. Perform the sink!
  for (const auto &Info : Incomings) {
    MachineBasicBlock *PredBB = Info.PredBB;
    MachineInstr *OpMI = Info.OpMI;
    MachineInstr *LoadMI = Info.LoadMI;

    // Clone LoadMI into PredBB right before OpMI.
    Register NewLoadDst =
        MRI.createGenericVirtualRegister(MRI.getType(Info.LoadedReg));
    BuildMI(*PredBB, OpMI, OpMI->getDebugLoc(), LoadMI->getDesc(), NewLoadDst)
        .addReg(PtrReg)
        .cloneMemRefs(*LoadMI);

    if (OpMI->getOperand(1).getReg() == Info.LoadedReg)
      OpMI->getOperand(1).setReg(NewLoadDst);
    else if (OpMI->getOperand(2).getReg() == Info.LoadedReg)
      OpMI->getOperand(2).setReg(NewLoadDst);

    // Insert G_STORE right after OpMI (before any terminator in PredBB).
    auto InsertPos = std::next(MachineBasicBlock::iterator(OpMI));
    while (InsertPos != PredBB->end() && InsertPos->isTerminator())
      break;

    BuildMI(*PredBB, InsertPos, StoreMI.getDebugLoc(), StoreMI.getDesc())
        .addReg(Info.IncomingVal)
        .addReg(PtrReg)
        .cloneMemRefs(StoreMI);
  }

  // Erase the merged store and phi from JoinBB.
  StoreMI.eraseFromParent();
  PhiMI->eraseFromParent();

  // If there was a loop-carried PHI, it is now dead.
  if (LoopPhiMI && MRI.use_nodbg_empty(LoopPhiMI->getOperand(0).getReg()))
    LoopPhiMI->eraseFromParent();

  // Clean up original loads if they have no remaining uses.
  for (MachineInstr *L : OriginalLoads) {
    Register Dst = L->getOperand(0).getReg();
    if (MRI.use_nodbg_empty(Dst))
      L->eraseFromParent();
  }

  ++NumStoresSunk;
  return true;
}
