//===-- Z80LateOptimization.cpp - Z80 Late Optimization -------------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the Z80 late optimization pass.
//
// This pass performs IX-indexed store-to-load forwarding after pseudo
// instructions have been expanded. When a value is spilled to the stack via
// LD (IX+d),R and later reloaded via LD R',(IX+d), this pass replaces the
// reload with a direct LD R',R (or eliminates it if R'==R).
//
//===----------------------------------------------------------------------===//

#include "Z80LateOptimization.h"

#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80Subtarget.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "z80-late-opt"

using namespace llvm;

// Custom DenseMapInfo for IX offsets.  The default DenseMapInfo<int8_t> uses
// -1 and -2 as sentinel values, which collide with valid IX offsets.
// Using int as the key type with out-of-range sentinels avoids this.
struct IXOffsetInfo {
  static inline int getEmptyKey() { return 256; }
  static inline int getTombstoneKey() { return 257; }
  static unsigned getHashValue(int V) {
    return DenseMapInfo<int>::getHashValue(V);
  }
  static bool isEqual(int LHS, int RHS) { return LHS == RHS; }
};

// Get the source register for an IX-indexed store instruction.
// Returns the physical register being stored, or Register() if not an
// IX-indexed store.
static Register getStoreIXdSrcReg(unsigned Opc) {
  switch (Opc) {
  case Z80::LD_IXd_A:
    return Z80::A;
  case Z80::LD_IXd_B:
    return Z80::B;
  case Z80::LD_IXd_C:
    return Z80::C;
  case Z80::LD_IXd_D:
    return Z80::D;
  case Z80::LD_IXd_E:
    return Z80::E;
  case Z80::LD_IXd_H:
    return Z80::H;
  case Z80::LD_IXd_L:
    return Z80::L;
  default:
    return Register();
  }
}

// Get the destination register for an IX-indexed load instruction.
// Returns the physical register being loaded, or Register() if not an
// IX-indexed load.
static Register getLoadIXdDstReg(unsigned Opc) {
  switch (Opc) {
  case Z80::LD_A_IXd:
    return Z80::A;
  case Z80::LD_B_IXd:
    return Z80::B;
  case Z80::LD_C_IXd:
    return Z80::C;
  case Z80::LD_D_IXd:
    return Z80::D;
  case Z80::LD_E_IXd:
    return Z80::E;
  case Z80::LD_H_IXd:
    return Z80::H;
  case Z80::LD_L_IXd:
    return Z80::L;
  default:
    return Register();
  }
}

// Get the LD r,r' opcode for two 8-bit physical registers.
// Returns 0 if no direct LD exists.
static unsigned getLD8Opcode(Register Dst, Register Src) {
  // Map register to table index
  auto regIdx = [](Register R) -> int {
    switch (R.id()) {
    case Z80::A:
      return 0;
    case Z80::B:
      return 1;
    case Z80::C:
      return 2;
    case Z80::D:
      return 3;
    case Z80::E:
      return 4;
    case Z80::H:
      return 5;
    case Z80::L:
      return 6;
    default:
      return -1;
    }
  };

  static const unsigned LDOpcodes[7][7] = {
      //       A            B            C            D            E H L
      /*A*/ {Z80::LD_A_A, Z80::LD_A_B, Z80::LD_A_C, Z80::LD_A_D, Z80::LD_A_E,
             Z80::LD_A_H, Z80::LD_A_L},
      /*B*/
      {Z80::LD_B_A, Z80::LD_B_B, Z80::LD_B_C, Z80::LD_B_D, Z80::LD_B_E,
       Z80::LD_B_H, Z80::LD_B_L},
      /*C*/
      {Z80::LD_C_A, Z80::LD_C_B, Z80::LD_C_C, Z80::LD_C_D, Z80::LD_C_E,
       Z80::LD_C_H, Z80::LD_C_L},
      /*D*/
      {Z80::LD_D_A, Z80::LD_D_B, Z80::LD_D_C, Z80::LD_D_D, Z80::LD_D_E,
       Z80::LD_D_H, Z80::LD_D_L},
      /*E*/
      {Z80::LD_E_A, Z80::LD_E_B, Z80::LD_E_C, Z80::LD_E_D, Z80::LD_E_E,
       Z80::LD_E_H, Z80::LD_E_L},
      /*H*/
      {Z80::LD_H_A, Z80::LD_H_B, Z80::LD_H_C, Z80::LD_H_D, Z80::LD_H_E,
       Z80::LD_H_H, Z80::LD_H_L},
      /*L*/
      {Z80::LD_L_A, Z80::LD_L_B, Z80::LD_L_C, Z80::LD_L_D, Z80::LD_L_E,
       Z80::LD_L_H, Z80::LD_L_L},
  };

  int di = regIdx(Dst), si = regIdx(Src);
  if (di < 0 || si < 0)
    return 0;
  return LDOpcodes[di][si];
}

// Invalidate all AvailValues entries where the stored register overlaps
// with the given clobbered register.
static void invalidateReg(DenseMap<int, MCPhysReg, IXOffsetInfo> &AvailValues,
                          const TargetRegisterInfo *TRI,
                          MCPhysReg ClobberedReg) {
  SmallVector<int, 4> ToErase;
  for (auto &KV : AvailValues) {
    if (TRI->regsOverlap(KV.second, ClobberedReg))
      ToErase.push_back(KV.first);
  }
  for (int K : ToErase)
    AvailValues.erase(K);
}

namespace {

class Z80LateOptimization : public MachineFunctionPass {
public:
  static char ID;

  Z80LateOptimization() : MachineFunctionPass(ID) {
    llvm::initializeZ80LateOptimizationPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

// Get the destination register for a LD r,(HL) instruction,
// or Register() if not one.
static Register getLoadHLindDstReg(unsigned Opc) {
  switch (Opc) {
  case Z80::LD_A_HLind:
    return Z80::A;
  case Z80::LD_B_HLind:
    return Z80::B;
  case Z80::LD_C_HLind:
    return Z80::C;
  case Z80::LD_D_HLind:
    return Z80::D;
  case Z80::LD_E_HLind:
    return Z80::E;
  default:
    return Register();
  }
}

// Get the source register for a LD (HL),r instruction,
// or Register() if not one.
static Register getStoreHLindSrcReg(unsigned Opc) {
  switch (Opc) {
  case Z80::LD_HLind_A:
    return Z80::A;
  case Z80::LD_HLind_B:
    return Z80::B;
  case Z80::LD_HLind_C:
    return Z80::C;
  case Z80::LD_HLind_D:
    return Z80::D;
  case Z80::LD_HLind_E:
    return Z80::E;
  default:
    return Register();
  }
}

// Get the source register for a LD A,r instruction, or Register() if not one.
static Register getLDArSrcReg(unsigned Opc) {
  switch (Opc) {
  case Z80::LD_A_B:
    return Z80::B;
  case Z80::LD_A_C:
    return Z80::C;
  case Z80::LD_A_D:
    return Z80::D;
  case Z80::LD_A_E:
    return Z80::E;
  case Z80::LD_A_H:
    return Z80::H;
  case Z80::LD_A_L:
    return Z80::L;
  default:
    return Register();
  }
}

// Get the LD r,A opcode for a given register r. Returns 0 if invalid.
static unsigned getLDrAOpcode(Register R) {
  switch (R.id()) {
  case Z80::B:
    return Z80::LD_B_A;
  case Z80::C:
    return Z80::LD_C_A;
  case Z80::D:
    return Z80::LD_D_A;
  case Z80::E:
    return Z80::LD_E_A;
  case Z80::H:
    return Z80::LD_H_A;
  case Z80::L:
    return Z80::LD_L_A;
  default:
    return 0;
  }
}

// Get the DEC r opcode for a given register r. Returns 0 if invalid.
static unsigned getDECrOpcode(Register R) {
  switch (R.id()) {
  case Z80::B:
    return Z80::DEC_B;
  case Z80::C:
    return Z80::DEC_C;
  case Z80::D:
    return Z80::DEC_D;
  case Z80::E:
    return Z80::DEC_E;
  case Z80::H:
    return Z80::DEC_H;
  case Z80::L:
    return Z80::DEC_L;
  default:
    return 0;
  }
}

// Get the LD r,#imm opcode for a given register r. Returns 0 if invalid.
static unsigned getLDrnOpcode(Register R) {
  switch (R.id()) {
  case Z80::A:
    return Z80::LD_A_n;
  case Z80::B:
    return Z80::LD_B_n;
  case Z80::C:
    return Z80::LD_C_n;
  case Z80::D:
    return Z80::LD_D_n;
  case Z80::E:
    return Z80::LD_E_n;
  case Z80::H:
    return Z80::LD_H_n;
  case Z80::L:
    return Z80::LD_L_n;
  default:
    return 0;
  }
}

// Check if a physical register is dead at a given point by scanning forward
// in the basic block. Returns true if the register is not used before being
// fully redefined or the end of the basic block.
//
// Tracks accumulated partial defs across instructions: e.g. for Reg=DE,
// seeing LD E,A followed by LD D,A (with no intervening use of DE/D/E)
// counts as a full redefinition.
static bool isRegDeadAfter(MachineBasicBlock::iterator After,
                           MachineBasicBlock &MBB,
                           const TargetRegisterInfo *TRI, MCPhysReg Reg) {
  // Collect sub-registers that must all be defined for Reg to be fully dead.
  // For leaf registers (e.g. E with no sub-regs), this stays empty and only
  // direct/super-register defs matter.
  SmallVector<MCPhysReg, 4> SubRegs;
  SmallVector<bool, 4> SubRegDefined;
  for (MCSubRegIterator SR(Reg, TRI); SR.isValid(); ++SR) {
    SubRegs.push_back(*SR);
    SubRegDefined.push_back(false);
  }

  for (auto I = After, E = MBB.end(); I != E; ++I) {
    bool HasUse = false, HasFullDef = false;
    for (const MachineOperand &MO : I->operands()) {
      if (!MO.isReg() || !MO.getReg().isPhysical())
        continue;
      if (!TRI->regsOverlap(MO.getReg(), Reg))
        continue;
      if (MO.readsReg())
        HasUse = true;
      if (MO.isDef()) {
        MCPhysReg DefReg = MO.getReg();
        // Direct or super-register def covers the entire register.
        if (DefReg == Reg || TRI->isSuperRegister(Reg, DefReg))
          HasFullDef = true;
        // Track partial defs: mark which sub-registers are covered.
        for (unsigned i = 0, e = SubRegs.size(); i != e; ++i) {
          if (!SubRegDefined[i] && (DefReg == SubRegs[i] ||
                                    TRI->isSuperRegister(SubRegs[i], DefReg)))
            SubRegDefined[i] = true;
        }
      }
    }
    if (HasUse)
      return false; // Used — register is live
    if (HasFullDef)
      return true; // Fully redefined without use — register is dead
    // Check if accumulated partial defs now cover all sub-registers.
    if (!SubRegs.empty() &&
        llvm::all_of(SubRegDefined, [](bool d) { return d; }))
      return true;
  }
  // End of basic block — check if register is live-in to any successor.
  // Use regsOverlap to catch sub/super-register relationships
  // (e.g. Reg=E but successor has DE as live-in).
  for (MachineBasicBlock *Succ : MBB.successors()) {
    for (const auto &LI : Succ->liveins()) {
      if (TRI->regsOverlap(LI.PhysReg, Reg))
        return false;
    }
  }
  return true;
}

bool Z80LateOptimization::runOnMachineFunction(MachineFunction &MF) {
  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  const auto *TII = STI.getInstrInfo();
  const auto *TRI = STI.getRegisterInfo();
  bool Changed = false;

  // --- IX constant propagation + unused IX/IY setup removal ---
  // 1. If IX is only used to hold a constant (LD IX,nn) with optional
  //    DEC/INC modifications and PUSH IX; POP rr extractions, replace
  //    each extraction with a direct LD rr,adjusted_value and remove
  //    the IX instructions. This saves ~10B per function (issue #15).
  // 2. If IX has no uses at all, remove PUSH IX; LD IX; POP IX setup.
  // 3. If IY has no uses, remove PUSH IY; POP IY.
  if (STI.hasZ80()) {
    bool IXUsedAsPointer = false; // IX-indexed addressing (abort remat)
    bool IXUsedOther = false;     // Other IX use (abort remat)
    bool IYUsedInBody = false;
    MachineInstr *ProloguePushIX = nullptr, *LdIX = nullptr;
    MachineInstr *EpiloguePopIX = nullptr;
    MachineInstr *PushIY = nullptr, *PopIY = nullptr;

    // Track IX constant value modifications and extractions.
    struct IXModification {
      MachineInstr *MI;
      int Delta; // +1 for INC, -1 for DEC
    };
    struct IXExtraction {
      MachineInstr *PushMI;
      MachineInstr *PopMI;
      unsigned TargetReg; // e.g. Z80::BC, Z80::DE, Z80::HL
      int64_t AdjustmentAtPoint; // cumulative DEC/INC delta at this point
    };
    SmallVector<IXModification, 4> Modifications;
    SmallVector<IXExtraction, 4> Extractions;
    int64_t CumulativeDelta = 0;

    for (auto &MBB2 : MF) {
      for (auto MII = MBB2.begin(), MIE = MBB2.end(); MII != MIE; ++MII) {
        unsigned Opc = MII->getOpcode();
        // Track prologue/epilogue PUSH/POP IX.
        if (Opc == Z80::PUSH_IX && !ProloguePushIX && !LdIX) {
          ProloguePushIX = &*MII;
          continue;
        }
        if (Opc == Z80::LD_IX_nn && !LdIX) {
          LdIX = &*MII;
          continue;
        }
        if (Opc == Z80::POP_IX) {
          EpiloguePopIX = &*MII;
          continue;
        }
        // Track IY setup.
        if (Opc == Z80::PUSH_IY && !PushIY) { PushIY = &*MII; continue; }
        if (Opc == Z80::POP_IY) { PopIY = &*MII; continue; }

        // Classify IX uses.
        // INC/DEC IX inside a loop means IX is not a constant —
        // the delta changes every iteration.  Detect by checking if
        // this block has a back-edge (a successor with a number <= ours,
        // or is its own successor).
        if (Opc == Z80::DEC_IX || Opc == Z80::INC_IX) {
          for (auto *Succ : MBB2.successors()) {
            if (Succ->getNumber() <= MBB2.getNumber()) {
              IXUsedOther = true; // in a loop: delta is not constant
              break;
            }
          }
          int Delta = (Opc == Z80::INC_IX) ? +1 : -1;
          CumulativeDelta += Delta;
          Modifications.push_back({&*MII, Delta});
          continue;
        }
        // PUSH IX; POP rr extraction pattern.
        if (Opc == Z80::PUSH_IX) {
          auto NextIt = std::next(MII);
          if (NextIt != MIE) {
            unsigned NextOpc = NextIt->getOpcode();
            unsigned TargetReg = 0;
            if (NextOpc == Z80::POP_BC) TargetReg = Z80::BC;
            else if (NextOpc == Z80::POP_DE) TargetReg = Z80::DE;
            else if (NextOpc == Z80::POP_HL) TargetReg = Z80::HL;
            if (TargetReg) {
              Extractions.push_back(
                  {&*MII, &*NextIt, TargetReg, CumulativeDelta});
              ++MII; // skip the POP
              continue;
            }
          }
          // PUSH IX without matching POP rr — unknown use.
          IXUsedOther = true;
          continue;
        }

        // Check for IX-indexed or other IX references.
        bool IsIXUse = false;
        for (const auto &MO : MII->operands()) {
          if (!MO.isReg()) continue;
          Register R = MO.getReg();
          if (R == Z80::IX) IsIXUse = true;
          if (R == Z80::IY) IYUsedInBody = true;
        }
        for (MCPhysReg R : TII->get(Opc).implicit_uses()) {
          if (R == Z80::IX) IsIXUse = true;
          if (R == Z80::IY) IYUsedInBody = true;
        }
        for (MCPhysReg R : TII->get(Opc).implicit_defs()) {
          if (R == Z80::IX) IsIXUse = true;
          if (R == Z80::IY) IYUsedInBody = true;
        }
        if (TII->getInstSizeInBytes(*MII) >= 3) {
          StringRef Name = TII->getName(Opc);
          if (Name.contains("IX")) { IsIXUse = true; IXUsedAsPointer = true; }
          if (Name.contains("IY")) IYUsedInBody = true;
        }
        if (IsIXUse) IXUsedOther = true;
      }
    }

    bool IXUsedInBody = IXUsedAsPointer || IXUsedOther;

    // IX constant propagation: replace extractions with direct loads.
    if (!IXUsedAsPointer && !IXUsedOther && !Extractions.empty() &&
        ProloguePushIX && LdIX && EpiloguePopIX) {
      MachineOperand &ValOp = LdIX->getOperand(0);
      // Get the LD rr,nn opcode for the target register.
      auto getLdOpc = [](unsigned Reg) -> unsigned {
        if (Reg == Z80::BC) return Z80::LD_BC_nn;
        if (Reg == Z80::DE) return Z80::LD_DE_nn;
        if (Reg == Z80::HL) return Z80::LD_HL_nn;
        return 0;
      };

      bool CanPropagate = true;
      for (auto &Ext : Extractions) {
        if (!getLdOpc(Ext.TargetReg)) { CanPropagate = false; break; }
      }

      if (CanPropagate) {
        LLVM_DEBUG(dbgs() << "  IX constant propagation: replacing "
                          << Extractions.size() << " extractions\n");
        for (auto &Ext : Extractions) {
          unsigned LdOpc = getLdOpc(Ext.TargetReg);
          MachineBasicBlock &ExtMBB = *Ext.PushMI->getParent();
          DebugLoc ExtDL = Ext.PushMI->getDebugLoc();
          // Build LD rr, value+adjustment.
          if (ValOp.isImm()) {
            BuildMI(ExtMBB, *Ext.PushMI, ExtDL, TII->get(LdOpc))
                .addImm((ValOp.getImm() + Ext.AdjustmentAtPoint) & 0xFFFF);
          } else {
            // Symbol operand: copy and adjust offset.
            auto MIB = BuildMI(ExtMBB, *Ext.PushMI, ExtDL, TII->get(LdOpc));
            MIB.add(ValOp);
            auto *NewMI = MIB.getInstr();
            auto &NewOp = NewMI->getOperand(NewMI->getNumExplicitOperands() - 1);
            NewOp.setOffset(NewOp.getOffset() + Ext.AdjustmentAtPoint);
          }
          Ext.PopMI->eraseFromParent();
          Ext.PushMI->eraseFromParent();
        }
        for (auto &Mod : Modifications)
          Mod.MI->eraseFromParent();
        LdIX->eraseFromParent();
        LdIX = nullptr;
        // Now IX is unused — the block below will remove PUSH/POP IX.
        IXUsedInBody = false;
        Changed = true;
      }
    }

    if (!IXUsedInBody && ProloguePushIX && LdIX && EpiloguePopIX) {
      LLVM_DEBUG(dbgs() << "  Removing unused IX frame setup\n");
      EpiloguePopIX->eraseFromParent();
      LdIX->eraseFromParent();
      ProloguePushIX->eraseFromParent();
      Changed = true;
    } else if (!IXUsedInBody && ProloguePushIX && !LdIX && EpiloguePopIX) {
      LLVM_DEBUG(dbgs() << "  Removing unused IX save/restore\n");
      EpiloguePopIX->eraseFromParent();
      ProloguePushIX->eraseFromParent();
      Changed = true;
    }
    if (!IYUsedInBody && PushIY && PopIY) {
      LLVM_DEBUG(dbgs() << "  Removing unused IY save/restore\n");
      PopIY->eraseFromParent();
      PushIY->eraseFromParent();
      Changed = true;
    }
  }

  for (MachineBasicBlock &MBB : MF) {
    // --- Peephole: POP rr; PUSH rr → (remove both) ---
    // When a register pair is popped and immediately pushed back, the stack
    // state is unchanged (SP net effect = 0, same value on stack). If the
    // register pair is dead after the push (overwritten before next use),
    // both instructions are redundant. Common on SM83 where consecutive
    // stack accesses via LDHL SP,# each need push/pop HL around them.
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      static const struct {
        unsigned PopOpc;
        unsigned PushOpc;
        MCPhysReg Reg;
      } PopPushPairs[] = {
          {Z80::POP_BC, Z80::PUSH_BC, Z80::BC},
          {Z80::POP_DE, Z80::PUSH_DE, Z80::DE},
          {Z80::POP_HL, Z80::PUSH_HL, Z80::HL},
      };

      unsigned Opc = MII->getOpcode();
      bool Matched = false;
      for (const auto &PP : PopPushPairs) {
        if (Opc != PP.PopOpc)
          continue;
        auto NextIt = std::next(MII);
        if (NextIt == MIE || NextIt->getOpcode() != PP.PushOpc)
          break;
        auto AfterPush = std::next(NextIt);
        if (!isRegDeadAfter(AfterPush, MBB, TRI, PP.Reg))
          break;
        LLVM_DEBUG(dbgs() << "  Removing redundant POP+PUSH: " << *MII);
        NextIt->eraseFromParent();
        MII = MBB.erase(MII);
        Changed = true;
        Matched = true;
        break;
      }
      if (!Matched)
        ++MII;
    }

    // --- Peephole: LD A,r; DEC A; LD r,A; OR A; JR NZ → DEC r; JR NZ ---
    // Replaces a 5-instruction decrement-and-branch sequence (28T, 6B) with
    // DEC r; JR NZ (14T, 3B). DEC r sets Z flag correctly for JR NZ, and
    // stays within the analyzable branch framework. Works on Z80 and SM83.
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      // Match: LD A,r (identify counter register r)
      Register CounterReg = getLDArSrcReg(MII->getOpcode());
      if (!CounterReg.isValid()) {
        ++MII;
        continue;
      }
      auto I1 = MII;
      auto I2 = std::next(I1);
      if (I2 == MIE) {
        ++MII;
        continue;
      }
      auto I3 = std::next(I2);
      if (I3 == MIE) {
        ++MII;
        continue;
      }
      auto I4 = std::next(I3);
      if (I4 == MIE) {
        ++MII;
        continue;
      }
      auto I5 = std::next(I4);
      if (I5 == MIE) {
        ++MII;
        continue;
      }

      // Match: DEC A; LD r,A; OR A; JR NZ,target
      if (I2->getOpcode() != Z80::DEC_A ||
          I3->getOpcode() != getLDrAOpcode(CounterReg) ||
          I4->getOpcode() != Z80::OR_A || I5->getOpcode() != Z80::JR_NZ_e) {
        ++MII;
        continue;
      }

      // The original sequence leaves A = r-1. The replacement doesn't
      // touch A, so we must verify A is dead after the sequence.
      if (!isRegDeadAfter(std::next(I5), MBB, TRI, Z80::A)) {
        ++MII;
        continue;
      }

      MachineBasicBlock *TargetMBB = I5->getOperand(0).getMBB();
      DebugLoc DL = I1->getDebugLoc();
      unsigned DECOpc = getDECrOpcode(CounterReg);
      LLVM_DEBUG(dbgs() << "  Loop counter peephole: LD A,"
                        << printReg(CounterReg, TRI) << " sequence → DEC "
                        << printReg(CounterReg, TRI) << "; JR NZ\n");
      I5->eraseFromParent();
      I4->eraseFromParent();
      I3->eraseFromParent();
      I2->eraseFromParent();
      MII = MBB.erase(I1);
      BuildMI(MBB, MII, DL, TII->get(DECOpc));
      BuildMI(MBB, MII, DL, TII->get(Z80::JR_NZ_e)).addMBB(TargetMBB);
      Changed = true;
    }

    // NOTE: A previous "DEC r → DEC B remap" for tight self-loops was here
    // but had a correctness bug: it changed the DEC opcode without ensuring
    // the counter value was actually in B (the predecessor still loads into r).
    // Removed. The DEC B; JR NZ → DJNZ peephole below handles all cases
    // correctly when the register allocator places the counter in B (enabled
    // by the B-last GR8 allocation order + DJNZ register hint).

    // --- Peephole: DEC B; JR NZ → DJNZ (Z80 only) ---
    // DJNZ is a 2-byte instruction that decrements B and branches if non-zero.
    // Replaces DEC B (1 byte) + JR NZ (2 bytes) = 3 bytes with DJNZ (2 bytes).
    if (STI.hasZ80()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        if (MII->getOpcode() != Z80::DEC_B) {
          ++MII;
          continue;
        }
        auto NextIt = std::next(MII);
        if (NextIt == MIE || NextIt->getOpcode() != Z80::JR_NZ_e) {
          ++MII;
          continue;
        }
        MachineBasicBlock *TargetMBB = NextIt->getOperand(0).getMBB();
        DebugLoc DL = MII->getDebugLoc();
        LLVM_DEBUG(dbgs() << "  DEC B; JR NZ → DJNZ\n");
        NextIt->eraseFromParent();
        MII = MBB.erase(MII);
        BuildMI(MBB, MII, DL, TII->get(Z80::DJNZ_e)).addMBB(TargetMBB);
        Changed = true;
      }
    }

    // --- Peephole: XOR #0xFF → CPL ---
    // CPL (1 byte) is equivalent to XOR #0xFF (2 bytes) for the A register
    // value, but sets flags differently (CPL: H=1,N=1, others unchanged;
    // XOR: S,Z,P from result, H=1,N=0,C=0). Safe only when FLAGS is dead.
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      MachineInstr &MI = *MII;
      if (MI.getOpcode() == Z80::XOR_n && MI.getOperand(0).getImm() == 0xFF) {
        auto After = std::next(MII);
        if (isRegDeadAfter(After, MBB, TRI, Z80::FLAGS)) {
          LLVM_DEBUG(dbgs() << "  XOR #0xFF → CPL: " << MI);
          BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(Z80::CPL));
          MII = MBB.erase(MII);
          Changed = true;
          continue;
        }
      }
      ++MII;
    }

    // --- Peephole: LD A,#0 → XOR A ---
    // XOR A (1 byte) sets A to 0 just like LD A,#0 (2 bytes), but also
    // sets FLAGS (Z=1, S=0, H=0, P=1, N=0, C=0). Safe when FLAGS is dead.
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      MachineInstr &MI = *MII;
      if (MI.getOpcode() == Z80::LD_A_n && MI.getOperand(0).getImm() == 0) {
        auto After = std::next(MII);
        if (isRegDeadAfter(After, MBB, TRI, Z80::FLAGS)) {
          LLVM_DEBUG(dbgs() << "  LD A,#0 → XOR A: " << MI);
          BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(Z80::XOR_A));
          MII = MBB.erase(MII);
          Changed = true;
          continue;
        }
      }
      ++MII;
    }

    // --- Peephole: OR A; LD r,0; JR Z → OR A; LD r,A; JR Z ---
    // In select lowering: OR A; LD r,0; JR Z skip; LD r,imm; skip:
    // After OR A, if A==0 the JR Z is taken with r=0 (correct via LD r,A).
    // On the NZ fall-through, r is overwritten by the non-zero select arm.
    // LD r,A is 1B/4T vs LD r,#0 at 2B/7T.  Saves 1B and 3T per instance.
    //
    // Safety: only fire when the LD r,0 sequence is followed by a Z-flag
    // branch (JR Z/NZ or JP Z/NZ), confirming this is a select pattern.
    // Without the branch check, LD r,0 in non-select code would be
    // incorrectly replaced when A != 0.
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE; ++MII) {
      // Look for OR_A (tests A, sets Z if A==0).
      if (MII->getOpcode() != Z80::OR_A)
        continue;
      // Collect candidate LD r,0 instructions between OR A and a branch.
      SmallVector<std::pair<MachineBasicBlock::iterator, unsigned>, 2>
          Candidates; // {iterator, LDrA opcode}
      auto Scan = std::next(MII);
      bool FoundBranch = false;
      while (Scan != MIE) {
        unsigned SOpc = Scan->getOpcode();
        // Check for Z-flag conditional branch — confirms select pattern.
        if (SOpc == Z80::JR_Z_e || SOpc == Z80::JR_NZ_e ||
            SOpc == Z80::JP_Z_nn || SOpc == Z80::JP_NZ_nn) {
          FoundBranch = true;
          break;
        }
        // Check for LD r,#0 where r != A.
        Register Dst;
        if (SOpc == Z80::LD_B_n) Dst = Z80::B;
        else if (SOpc == Z80::LD_C_n) Dst = Z80::C;
        else if (SOpc == Z80::LD_D_n) Dst = Z80::D;
        else if (SOpc == Z80::LD_E_n) Dst = Z80::E;
        else if (SOpc == Z80::LD_H_n) Dst = Z80::H;
        else if (SOpc == Z80::LD_L_n) Dst = Z80::L;
        else break; // unknown instruction — stop scanning

        if (Scan->getOperand(0).getImm() != 0)
          break; // not loading zero

        unsigned LdRA = getLDrAOpcode(Dst);
        if (!LdRA) break;
        Candidates.push_back({Scan, LdRA});
        ++Scan;
      }
      // Only apply if we confirmed a Z-flag branch follows.
      if (FoundBranch) {
        for (auto &[It, LdRA] : Candidates) {
          LLVM_DEBUG(dbgs() << "  OR A; LD r,0 → LD r,A: " << *It);
          BuildMI(MBB, *It, It->getDebugLoc(), TII->get(LdRA));
          It = MBB.erase(It);
          Changed = true;
        }
      }
    }

    // --- Peephole: LD rr,nn; INC/DEC rr → LD rr,nn±1 ---
    // Fold a 16-bit increment/decrement into the preceding immediate load.
    // LD rr,nn (3B) + INC/DEC rr (1B) = 4B → LD rr,nn±1 (3B). Saves 1B.
    // INC/DEC rr doesn't set flags, so no flag dependency to worry about.
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      unsigned Opc = MII->getOpcode();
      unsigned LdOpc = 0;
      if (Opc == Z80::LD_BC_nn) LdOpc = Opc;
      else if (Opc == Z80::LD_DE_nn) LdOpc = Opc;
      else if (Opc == Z80::LD_HL_nn) LdOpc = Opc;
      else if (Opc == Z80::LD_IX_nn) LdOpc = Opc;
      else if (Opc == Z80::LD_IY_nn) LdOpc = Opc;
      if (!LdOpc) {
        ++MII;
        continue;
      }
      auto NextIt = std::next(MII);
      if (NextIt == MIE) { ++MII; continue; }
      int Delta = 0;
      unsigned NextOpc = NextIt->getOpcode();
      if ((Opc == Z80::LD_BC_nn && NextOpc == Z80::INC_BC) ||
          (Opc == Z80::LD_DE_nn && NextOpc == Z80::INC_DE) ||
          (Opc == Z80::LD_HL_nn && NextOpc == Z80::INC_HL) ||
          (Opc == Z80::LD_IX_nn && NextOpc == Z80::INC_IX) ||
          (Opc == Z80::LD_IY_nn && NextOpc == Z80::INC_IY))
        Delta = 1;
      else if ((Opc == Z80::LD_BC_nn && NextOpc == Z80::DEC_BC) ||
               (Opc == Z80::LD_DE_nn && NextOpc == Z80::DEC_DE) ||
               (Opc == Z80::LD_HL_nn && NextOpc == Z80::DEC_HL) ||
               (Opc == Z80::LD_IX_nn && NextOpc == Z80::DEC_IX) ||
               (Opc == Z80::LD_IY_nn && NextOpc == Z80::DEC_IY))
        Delta = -1;
      if (Delta) {
        MachineOperand &Op = MII->getOperand(0);
        LLVM_DEBUG(dbgs() << "  LD rr,nn; " << (Delta > 0 ? "INC" : "DEC")
                          << " → fold into load\n");
        if (Op.isImm()) {
          Op.setImm((Op.getImm() + Delta) & 0xFFFF);
        } else {
          // Symbol operand: adjust the offset.
          Op.setOffset(Op.getOffset() + Delta);
        }
        NextIt->eraseFromParent();
        Changed = true;
        continue; // re-check in case of chained INC/DEC
      }
      ++MII;
    }

    // --- Peephole: ALU #imm; ALU #imm → ALU #imm ---
    // When the same immediate ALU instruction appears consecutively, the
    // second is redundant for idempotent operations (AND, OR).
    // Most common case: AND #1; AND #1 after SBC A,A; AND #1 sequences.
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      MachineInstr &MI = *MII;
      auto NextIt = std::next(MII);
      if (NextIt != MIE && MI.getOpcode() == NextIt->getOpcode() &&
          (MI.getOpcode() == Z80::AND_n || MI.getOpcode() == Z80::OR_n) &&
          MI.getOperand(0).getImm() == NextIt->getOperand(0).getImm()) {
        LLVM_DEBUG(dbgs() << "  Removing redundant: " << *NextIt);
        NextIt->eraseFromParent();
        Changed = true;
        continue;
      }
      ++MII;
    }

    // --- Peephole: LD r,A; LD A,r2; ALU r → ALU r2 ---
    // When the register allocator routes a commutative ALU operation through
    // a temp register (LD E,A; LD A,D; OR E → OR D), eliminate the detour.
    // Saves 2 bytes per instance.
    {
      // Map ALU opcode → register it operates on (0 = not a commutative ALU op)
      auto getALUReg = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::OR_B: case Z80::AND_B: case Z80::XOR_B: return Z80::B;
        case Z80::OR_C: case Z80::AND_C: case Z80::XOR_C: return Z80::C;
        case Z80::OR_D: case Z80::AND_D: case Z80::XOR_D: return Z80::D;
        case Z80::OR_E: case Z80::AND_E: case Z80::XOR_E: return Z80::E;
        case Z80::OR_H: case Z80::AND_H: case Z80::XOR_H: return Z80::H;
        case Z80::OR_L: case Z80::AND_L: case Z80::XOR_L: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      // Get the same ALU op but with a different register
      auto getALUWithReg = [](unsigned Opc, MCPhysReg R) -> unsigned {
        // Determine ALU group
        unsigned Base;
        switch (Opc) {
        case Z80::OR_B: case Z80::OR_C: case Z80::OR_D:
        case Z80::OR_E: case Z80::OR_H: case Z80::OR_L: Base = 0; break;
        case Z80::AND_B: case Z80::AND_C: case Z80::AND_D:
        case Z80::AND_E: case Z80::AND_H: case Z80::AND_L: Base = 1; break;
        case Z80::XOR_B: case Z80::XOR_C: case Z80::XOR_D:
        case Z80::XOR_E: case Z80::XOR_H: case Z80::XOR_L: Base = 2; break;
        default: return 0;
        }
        static const unsigned Ops[3][6] = {
            {Z80::OR_B,  Z80::OR_C,  Z80::OR_D,  Z80::OR_E,  Z80::OR_H,  Z80::OR_L},
            {Z80::AND_B, Z80::AND_C, Z80::AND_D, Z80::AND_E, Z80::AND_H, Z80::AND_L},
            {Z80::XOR_B, Z80::XOR_C, Z80::XOR_D, Z80::XOR_E, Z80::XOR_H, Z80::XOR_L},
        };
        switch (R) {
        case Z80::B: return Ops[Base][0]; case Z80::C: return Ops[Base][1];
        case Z80::D: return Ops[Base][2]; case Z80::E: return Ops[Base][3];
        case Z80::H: return Ops[Base][4]; case Z80::L: return Ops[Base][5];
        default: return 0;
        }
      };
      // Get source register from LD A,r opcode
      auto getLDAsrc = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::LD_A_B: return Z80::B; case Z80::LD_A_C: return Z80::C;
        case Z80::LD_A_D: return Z80::D; case Z80::LD_A_E: return Z80::E;
        case Z80::LD_A_H: return Z80::H; case Z80::LD_A_L: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      // Get dest register from LD r,A opcode
      auto getLDrAdst = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::LD_B_A: return Z80::B; case Z80::LD_C_A: return Z80::C;
        case Z80::LD_D_A: return Z80::D; case Z80::LD_E_A: return Z80::E;
        case Z80::LD_H_A: return Z80::H; case Z80::LD_L_A: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      SmallVector<MachineInstr *, 8> ToErase2;
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ++MII) {
        // Match: LD r,A
        MCPhysReg TempReg = getLDrAdst(MII->getOpcode());
        if (!TempReg) continue;
        // Skip KILLs to LD A,r2
        auto It = std::next(MII);
        while (It != MIE && It->getOpcode() == TargetOpcode::KILL) ++It;
        if (It == MIE) continue;
        MCPhysReg SrcReg = getLDAsrc(It->getOpcode());
        if (!SrcReg || SrcReg == TempReg) continue;
        auto LdA = It;
        // Skip KILLs to ALU TempReg
        ++It;
        while (It != MIE && It->getOpcode() == TargetOpcode::KILL) ++It;
        if (It == MIE) continue;
        MCPhysReg AluReg = getALUReg(It->getOpcode());
        if (AluReg != TempReg) continue;
        unsigned NewOpc = getALUWithReg(It->getOpcode(), SrcReg);
        if (!NewOpc) continue;
        LLVM_DEBUG(dbgs() << "  Commutative ALU shortcut: "
                          << TII->getName(MII->getOpcode()) << "; "
                          << TII->getName(LdA->getOpcode()) << "; "
                          << TII->getName(It->getOpcode()) << " → "
                          << TII->getName(NewOpc) << "\n");
        BuildMI(MBB, *It, It->getDebugLoc(), TII->get(NewOpc));
        ToErase2.push_back(&*MII);
        ToErase2.push_back(&*LdA);
        ToErase2.push_back(&*It);
        Changed = true;
      }
      for (auto *MI : ToErase2)
        MI->eraseFromParent();
    }

    // --- Peephole: LD L,H; LD H,0; LD A,L → LD A,H ---
    // When extracting the high byte of HL into A, the compiler sometimes
    // routes through L (LD L,H; LD H,0; LD A,L) instead of directly (LD A,H).
    // This happens when ISel zero-extends the high byte into HL (for potential
    // 16-bit use) but the result is only consumed as an 8-bit value in A.
    // The peephole replaces the 3-instruction sequence (4B) with LD A,H (1B).
    // Safe when H and L are dead after (the LD H,0 overwrites H, and LD A,L
    // is the last use of L before it's overwritten or dead).
    {
      SmallVector<MachineInstr *, 4> ToErase;
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ++MII) {
        MachineInstr &MI = *MII;
        if (MI.getOpcode() != Z80::LD_L_H) continue;
        // Look for LD H,#0 (skipping KILL pseudos)
        auto It = std::next(MachineBasicBlock::iterator(&MI));
        while (It != MIE && It->getOpcode() == TargetOpcode::KILL) ++It;
        if (It == MIE || It->getOpcode() != Z80::LD_H_n ||
            It->getOperand(0).getImm() != 0) continue;
        MachineInstr *LdH = &*It;
        // Look for LD A,L (skipping KILL pseudos)
        ++It;
        while (It != MIE && It->getOpcode() == TargetOpcode::KILL) ++It;
        if (It == MIE || It->getOpcode() != Z80::LD_A_L) continue;
        MachineInstr *LdA = &*It;
        LLVM_DEBUG(dbgs() << "  High-byte extract: LD L,H; LD H,0; LD A,L → "
                             "LD A,H\n");
        BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(Z80::LD_A_H));
        ToErase.push_back(LdA);
        ToErase.push_back(LdH);
        ToErase.push_back(&MI);
        Changed = true;
      }
      for (auto *MI : ToErase)
        MI->eraseFromParent();
    }

    // --- Peephole: 16-bit increment overflow test ---
    // Pattern A (9 instr, 16B): LD HL,1; ADD HL,rr; SBC A,A; AND 1;
    //   LD lo,L; LD hi,H; XOR 1; AND 1; JR NZ → INC rr; LD A,hi; OR lo; JR NZ
    // Pattern B (6 instr, 10B): LD HL,1; ADD HL,rr; SBC A,A; AND 1;
    //   EX DE,HL; JR Z → INC rr; LD A,hi; OR lo; JR NZ (inverted sense)
    // Saves 11B (A) or 5B (B) per instance. Common in `while(++counter)` loops.
    if (STI.hasZ80()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        // Match: LD HL,1
        if (MII->getOpcode() != Z80::LD_HL_nn || !MII->getOperand(0).isImm() ||
            MII->getOperand(0).getImm() != 1) {
          ++MII;
          continue;
        }
        auto I0 = MII; // LD HL,1
        auto I1 = std::next(I0); if (I1 == MIE) { ++MII; continue; }
        auto I2 = std::next(I1); if (I2 == MIE) { ++MII; continue; }
        auto I3 = std::next(I2); if (I3 == MIE) { ++MII; continue; }

        // Match: ADD HL,rr; SBC A,A; AND 1
        unsigned AddOpc = I1->getOpcode();
        unsigned IncOpc = 0, LdAHiOpc = 0, OrLoOpc = 0;
        unsigned LdLoOpc = 0, LdHiOpc = 0;
        if (AddOpc == Z80::ADD_HL_BC) {
          IncOpc = Z80::INC_BC;
          LdAHiOpc = Z80::LD_A_B; OrLoOpc = Z80::OR_C;
          LdLoOpc = Z80::LD_C_L; LdHiOpc = Z80::LD_B_H;
        } else if (AddOpc == Z80::ADD_HL_DE) {
          IncOpc = Z80::INC_DE;
          LdAHiOpc = Z80::LD_A_D; OrLoOpc = Z80::OR_E;
          LdLoOpc = Z80::LD_E_L; LdHiOpc = Z80::LD_D_H;
        }
        if (!IncOpc ||
            I2->getOpcode() != Z80::SBC_A_A ||
            I3->getOpcode() != Z80::AND_n || I3->getOperand(0).getImm() != 1) {
          ++MII;
          continue;
        }

        auto I4 = std::next(I3); if (I4 == MIE) { ++MII; continue; }

        bool Matched = false;

        // Try Pattern A: LD lo,L; LD hi,H; XOR 1; AND 1; JR NZ
        {
          auto I5 = std::next(I4);
          auto I6 = (I5 != MIE) ? std::next(I5) : MIE;
          auto I7 = (I6 != MIE) ? std::next(I6) : MIE;
          auto I8 = (I7 != MIE) ? std::next(I7) : MIE;
          // Also handle optional OR_A before JR_NZ (redundant flag test).
          auto IBranch = I8;
          if (I8 != MIE && I8->getOpcode() == Z80::OR_A) {
            auto I9 = std::next(I8);
            if (I9 != MIE) IBranch = I9;
          }
          if (IBranch != MIE &&
              I4->getOpcode() == LdLoOpc && I5->getOpcode() == LdHiOpc &&
              I6->getOpcode() == Z80::XOR_n && I6->getOperand(0).getImm() == 1 &&
              I7->getOpcode() == Z80::AND_n && I7->getOperand(0).getImm() == 1 &&
              IBranch->getOpcode() == Z80::JR_NZ_e) {
            MachineBasicBlock *Target = IBranch->getOperand(0).getMBB();
            DebugLoc DL = I0->getDebugLoc();
            LLVM_DEBUG(dbgs() << "  16-bit INC+NZ (pattern A): 9→4 instr\n");
            BuildMI(MBB, *I0, DL, TII->get(IncOpc));
            BuildMI(MBB, *I0, DL, TII->get(LdAHiOpc));
            BuildMI(MBB, *I0, DL, TII->get(OrLoOpc));
            BuildMI(MBB, *I0, DL, TII->get(Z80::JR_NZ_e)).addMBB(Target);
            IBranch->eraseFromParent();
            if (I8 != IBranch) I8->eraseFromParent(); // OR_A
            I7->eraseFromParent();
            I6->eraseFromParent(); I5->eraseFromParent();
            I4->eraseFromParent(); I3->eraseFromParent();
            I2->eraseFromParent(); I1->eraseFromParent();
            MII = MBB.erase(I0);
            Changed = true;
            Matched = true;
          }
        }

        // Try Pattern B: EX DE,HL; JR Z (counter in DE, inverted branch)
        if (!Matched && I4->getOpcode() == Z80::EX_DE_HL &&
            AddOpc == Z80::ADD_HL_DE) {
          auto IBr = std::next(I4);
          if (IBr != MIE && IBr->getOpcode() == Z80::JR_Z_e) {
            MachineBasicBlock *Target = IBr->getOperand(0).getMBB();
            DebugLoc DL = I0->getDebugLoc();
            LLVM_DEBUG(dbgs() << "  16-bit INC+NZ (pattern B): 6→4 instr\n");
            BuildMI(MBB, *I0, DL, TII->get(Z80::INC_DE));
            BuildMI(MBB, *I0, DL, TII->get(Z80::LD_A_D));
            BuildMI(MBB, *I0, DL, TII->get(Z80::OR_E));
            BuildMI(MBB, *I0, DL, TII->get(Z80::JR_NZ_e)).addMBB(Target);
            IBr->eraseFromParent(); I4->eraseFromParent();
            I3->eraseFromParent(); I2->eraseFromParent();
            I1->eraseFromParent();
            MII = MBB.erase(I0);
            Changed = true;
            Matched = true;
          }
        }

        if (!Matched)
          ++MII;
      }
    }

    // --- Peephole: LD rr,#imm; LDHL SP,#; LD (HL),lo; INC HL; LD (HL),hi
    //             → LDHL SP,#; LD (HL),#lo; INC HL; LD (HL),#hi (SM83 only) ---
    // When a 16-bit constant is stored to the stack via a register pair,
    // replace with immediate stores to (HL). Saves 1 byte (8B → 7B) per
    // occurrence and frees the register pair.
    if (STI.hasSM83()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        MachineInstr &MI = *MII;
        unsigned Opc = MI.getOpcode();

        bool IsBC = (Opc == Z80::LD_BC_nn);
        bool IsDE = (Opc == Z80::LD_DE_nn);
        if (!IsBC && !IsDE) {
          ++MII;
          continue;
        }
        if (!MI.getOperand(0).isImm()) {
          ++MII;
          continue;
        }

        // Match 5 consecutive instructions.
        auto I2 = std::next(MII);
        if (I2 == MIE || I2->getOpcode() != Z80::LDHL_SP_e) {
          ++MII;
          continue;
        }
        auto I3 = std::next(I2);
        if (I3 == MIE) {
          ++MII;
          continue;
        }
        unsigned ExpLo = IsBC ? Z80::LD_HLind_C : Z80::LD_HLind_E;
        if (I3->getOpcode() != ExpLo) {
          ++MII;
          continue;
        }

        auto I4 = std::next(I3);
        if (I4 == MIE || I4->getOpcode() != Z80::INC_HL) {
          ++MII;
          continue;
        }
        auto I5 = std::next(I4);
        if (I5 == MIE) {
          ++MII;
          continue;
        }
        unsigned ExpHi = IsBC ? Z80::LD_HLind_B : Z80::LD_HLind_D;
        if (I5->getOpcode() != ExpHi) {
          ++MII;
          continue;
        }

        // Register pair must be dead after the store sequence.
        MCPhysReg PairReg = IsBC ? Z80::BC : Z80::DE;
        if (!isRegDeadAfter(std::next(I5), MBB, TRI, PairReg)) {
          ++MII;
          continue;
        }

        int64_t Imm = MI.getOperand(0).getImm();
        LLVM_DEBUG(dbgs() << "  Folding 16-bit const store: " << MI);

        // Replace LD (HL),lo → LD (HL),#imm_lo
        BuildMI(MBB, *I3, I3->getDebugLoc(), TII->get(Z80::LD_HLind_n))
            .addImm(Imm & 0xFF);
        I3->eraseFromParent();

        // Replace LD (HL),hi → LD (HL),#imm_hi
        BuildMI(MBB, *I5, I5->getDebugLoc(), TII->get(Z80::LD_HLind_n))
            .addImm((Imm >> 8) & 0xFF);
        I5->eraseFromParent();

        // Remove LD rr,#imm
        MII = MBB.erase(MII);
        Changed = true;
      }
    }

    // --- Peephole: consecutive LDHL SP,#N → INC/DEC HL (SM83 only) ---
    // When two LDHL SP,# instructions target adjacent offsets with only
    // non-HL-modifying instructions between them, replace the second LDHL
    // with INC HL or DEC HL. Saves 1 byte (2B → 1B) per occurrence.
    // Common in consecutive byte-at-a-time stack initialization.
    if (STI.hasSM83()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE; ++MII) {
        if (MII->getOpcode() != Z80::LDHL_SP_e)
          continue;
        if (!MII->getOperand(0).isImm())
          continue;
        int64_t Offset1 = MII->getOperand(0).getImm();

        // Scan forward to find the next LDHL SP,#. Bail if any
        // intervening instruction modifies HL, modifies SP, or has
        // unmodeled side effects. SP changes must be caught explicitly
        // because PUSH/POP don't declare SP in their Defs.
        auto It = std::next(MII);
        bool Clobbered = false;
        while (It != MIE && It->getOpcode() != Z80::LDHL_SP_e) {
          // PUSH/POP modify SP but don't declare it as Def.
          if (It->isCall() || It->isReturn() || It->hasUnmodeledSideEffects() ||
              It->getOpcode() == Z80::PUSH_BC ||
              It->getOpcode() == Z80::PUSH_DE ||
              It->getOpcode() == Z80::PUSH_HL ||
              It->getOpcode() == Z80::PUSH_AF ||
              It->getOpcode() == Z80::POP_BC ||
              It->getOpcode() == Z80::POP_DE ||
              It->getOpcode() == Z80::POP_AF ||
              It->getOpcode() == Z80::ADD_SP_e) {
            // POP_HL also modifies HL, but we catch it via Defs below.
            Clobbered = true;
            break;
          }
          // Check explicit and implicit defs for HL and SP.
          for (const MachineOperand &MO : It->operands()) {
            if (MO.isReg() && MO.isDef() && MO.getReg().isPhysical() &&
                (TRI->regsOverlap(MO.getReg(), Z80::HL) ||
                 TRI->regsOverlap(MO.getReg(), Z80::SP))) {
              Clobbered = true;
              break;
            }
          }
          if (Clobbered)
            break;
          for (MCPhysReg Def : TII->get(It->getOpcode()).implicit_defs()) {
            if (TRI->regsOverlap(Def, Z80::HL) ||
                TRI->regsOverlap(Def, Z80::SP)) {
              Clobbered = true;
              break;
            }
          }
          if (Clobbered)
            break;
          ++It;
        }
        if (Clobbered || It == MIE)
          continue;
        if (It->getOpcode() != Z80::LDHL_SP_e || !It->getOperand(0).isImm())
          continue;

        int64_t Offset2 = It->getOperand(0).getImm();
        int64_t Diff = Offset2 - Offset1;
        if (Diff != 1 && Diff != -1)
          continue;
        // LDHL sets FLAGS (H,C), INC/DEC HL does not. Verify FLAGS is dead.
        if (!isRegDeadAfter(std::next(It), MBB, TRI, Z80::FLAGS))
          continue;

        unsigned NewOpc = (Diff == 1) ? Z80::INC_HL : Z80::DEC_HL;
        LLVM_DEBUG(dbgs() << "  LDHL SP,#" << Offset2 << " → "
                          << (Diff == 1 ? "INC" : "DEC") << " HL\n");
        BuildMI(MBB, *It, It->getDebugLoc(), TII->get(NewOpc));
        It->eraseFromParent();
        Changed = true;
      }
    }

    // --- Peephole: fold constant into XOR compare (CMP_Z16 + imm) ---
    // When a XOR-based 16-bit compare uses a constant loaded into a register
    // pair, fold the constant into XOR immediate instructions.
    // LD rr,#imm; LD A,X; XOR rhi; LD B,A; LD A,Y; XOR rlo; OR B
    // → LD A,X; XOR #hi; LD B,A; LD A,Y; XOR #lo; OR B
    // Saves 1 byte (9B → 8B) per occurrence and frees the register pair.
    // Applies to both Z80 (XOR_CMP_Z16 for i32/i64) and SM83 (SM83_CMP_Z16).
    {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        MachineInstr &MI = *MII;
        unsigned Opc = MI.getOpcode();

        bool IsBC = (Opc == Z80::LD_BC_nn);
        bool IsDE = (Opc == Z80::LD_DE_nn);
        if (!IsBC && !IsDE) {
          ++MII;
          continue;
        }
        if (!MI.getOperand(0).isImm()) {
          ++MII;
          continue;
        }

        // Match 7 consecutive instructions.
        auto I2 = std::next(MII);
        if (I2 == MIE) {
          ++MII;
          continue;
        }
        // I2: LD A,X (load high byte of compared value)
        Register I2Src = getLDArSrcReg(I2->getOpcode());
        if (!I2Src.isValid() && I2->getOpcode() != Z80::LD_A_HLind) {
          ++MII;
          continue;
        }

        auto I3 = std::next(I2);
        if (I3 == MIE) {
          ++MII;
          continue;
        }
        unsigned ExpXorHi = IsBC ? Z80::XOR_B : Z80::XOR_D;
        if (I3->getOpcode() != ExpXorHi) {
          ++MII;
          continue;
        }

        auto I4 = std::next(I3);
        if (I4 == MIE || I4->getOpcode() != Z80::LD_B_A) {
          ++MII;
          continue;
        }

        auto I5 = std::next(I4);
        if (I5 == MIE) {
          ++MII;
          continue;
        }
        // I5: LD A,Y (load low byte of compared value)
        Register I5Src = getLDArSrcReg(I5->getOpcode());
        if (!I5Src.isValid() && I5->getOpcode() != Z80::LD_A_HLind) {
          ++MII;
          continue;
        }

        auto I6 = std::next(I5);
        if (I6 == MIE) {
          ++MII;
          continue;
        }
        unsigned ExpXorLo = IsBC ? Z80::XOR_C : Z80::XOR_E;
        if (I6->getOpcode() != ExpXorLo) {
          ++MII;
          continue;
        }

        auto I7 = std::next(I6);
        if (I7 == MIE || I7->getOpcode() != Z80::OR_B) {
          ++MII;
          continue;
        }

        // Ensure lhs registers don't overlap with the constant pair.
        MCPhysReg PairReg = IsBC ? Z80::BC : Z80::DE;
        if (I2Src.isValid() && TRI->regsOverlap(I2Src, PairReg)) {
          ++MII;
          continue;
        }
        if (I5Src.isValid() && TRI->regsOverlap(I5Src, PairReg)) {
          ++MII;
          continue;
        }

        // The constant pair must be dead after OR B.
        // For BC: B is overwritten by LD B,A (I4) with the XOR result (same
        // value in both original and folded code), so only C matters.
        // For DE: neither D nor E is overwritten, so both must be dead.
        if (IsBC) {
          if (!isRegDeadAfter(std::next(I7), MBB, TRI, Z80::C)) {
            ++MII;
            continue;
          }
        } else {
          if (!isRegDeadAfter(std::next(I7), MBB, TRI, Z80::DE)) {
            ++MII;
            continue;
          }
        }

        int64_t Imm = MI.getOperand(0).getImm();
        int64_t HiByte = (Imm >> 8) & 0xFF;
        int64_t LoByte = Imm & 0xFF;
        LLVM_DEBUG(dbgs() << "  Folding CMP_Z16 constant: " << MI);

        // Handle XOR rhi: replace with XOR #hi, or remove if hi == 0.
        if (HiByte != 0) {
          BuildMI(MBB, *I3, I3->getDebugLoc(), TII->get(Z80::XOR_n))
              .addImm(HiByte);
        } else {
          // XOR #0 is identity. Also fold LD A,X; LD B,A → LD B,X.
          // I2 is LD A,X, I4 is LD B,A. With XOR removed, this is LD B,X.
          unsigned LdBOpc = 0;
          if (I2Src.isValid())
            LdBOpc = getLD8Opcode(Z80::B, I2Src);
          else if (I2->getOpcode() == Z80::LD_A_HLind)
            LdBOpc = Z80::LD_B_HLind;
          if (LdBOpc) {
            // Skip LD B,B (self-move NOP when I2Src == B).
            if (!(I2Src.isValid() && I2Src == Z80::B))
              BuildMI(MBB, *I2, I2->getDebugLoc(), TII->get(LdBOpc));
            I2->eraseFromParent();
            I4->eraseFromParent();
          }
        }
        I3->eraseFromParent();

        // Handle XOR rlo: replace with XOR #lo, or remove if lo == 0.
        if (LoByte != 0) {
          BuildMI(MBB, *I6, I6->getDebugLoc(), TII->get(Z80::XOR_n))
              .addImm(LoByte);
        }
        I6->eraseFromParent();

        // Remove LD rr,#imm
        MII = MBB.erase(MII);
        Changed = true;
      }
    }

    // --- Peephole: LD A,(HL); INC/DEC HL → LD A,(HL+)/(HL-) (SM83 only) ---
    // SM83 has post-increment/decrement LD instructions that combine a load
    // or store with an HL adjustment in a single byte.
    // Patterns:
    //   LD A,(HL); INC HL → LD A,(HL+)   (2B → 1B)
    //   LD (HL),A; INC HL → LD (HL+),A   (2B → 1B)
    //   LD A,(HL); DEC HL → LD A,(HL-)   (2B → 1B)
    //   LD (HL),A; DEC HL → LD (HL-),A   (2B → 1B)
    //
    // Extended: when r != A and A is dead after the sequence:
    //   LD r,(HL); INC HL → LD A,(HL+); LD r,A   (2B → 2B, saves 4T)
    //   LD (HL),r; INC HL → LD A,r; LD (HL+),A   (2B → 2B, saves 4T)
    //   (same for DEC HL variants with HL-/HL-)
    if (STI.hasSM83()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        MachineInstr &MI = *MII;
        auto NextIt = std::next(MII);
        if (NextIt == MIE) {
          ++MII;
          continue;
        }

        unsigned Opc = MI.getOpcode();
        unsigned NextOpc = NextIt->getOpcode();

        // Direct r=A patterns: 2B → 1B (size + speed win)
        unsigned NewOpc = 0;
        if (NextOpc == Z80::INC_HL) {
          if (Opc == Z80::LD_A_HLind)
            NewOpc = Z80::LD_A_HLI;
          else if (Opc == Z80::LD_HLind_A)
            NewOpc = Z80::LD_HLI_A;
        } else if (NextOpc == Z80::DEC_HL) {
          if (Opc == Z80::LD_A_HLind)
            NewOpc = Z80::LD_A_HLD;
          else if (Opc == Z80::LD_HLind_A)
            NewOpc = Z80::LD_HLD_A;
        }

        if (NewOpc) {
          LLVM_DEBUG(dbgs() << "  LD+INC/DEC HL → LD (HL+/-): " << MI);
          BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(NewOpc));
          NextIt->eraseFromParent();
          MII = MBB.erase(MII);
          Changed = true;
          continue;
        }

        // Extended r!=A patterns: 2B → 2B (speed win only, saves 4T)
        // LD r,(HL); INC/DEC HL → LD A,(HL+/-); LD r,A  (requires A dead)
        // LD (HL),r; INC/DEC HL → LD A,r; LD (HL+/-),A  (requires A dead)
        if (NextOpc == Z80::INC_HL || NextOpc == Z80::DEC_HL) {
          bool IsInc = (NextOpc == Z80::INC_HL);

          Register LoadDst = getLoadHLindDstReg(Opc);
          Register StoreSrc = getStoreHLindSrcReg(Opc);
          // Exclude A: LD A,(HL) → LD A,(HL+) is handled directly,
          // and LD (HL),A → LD A,A; LD (HL+),A produces a useless LD A,A.
          if (LoadDst == Z80::A)
            LoadDst = Register();
          if (StoreSrc == Z80::A)
            StoreSrc = Register();

          // Skip if this load is part of a 16-bit HL load pattern that the
          // later peephole will fold more profitably (5B → 3B vs our 2B → 2B).
          // Pattern: LD C/E,(HL); INC HL; LD B/D,(HL); LD L,C/E; LD H,B/D
          if (LoadDst.isValid() && IsInc) {
            auto I3 = std::next(NextIt);
            if (I3 != MIE) {
              unsigned HiOpc = (LoadDst == Z80::C)   ? Z80::LD_B_HLind
                               : (LoadDst == Z80::E) ? Z80::LD_D_HLind
                                                     : 0;
              if (HiOpc && I3->getOpcode() == HiOpc) {
                ++MII;
                continue; // Let 16-bit HL load peephole handle it
              }
            }
          }

          if (LoadDst.isValid() || StoreSrc.isValid()) {
            auto AfterSeq = std::next(NextIt);
            if (isRegDeadAfter(AfterSeq, MBB, TRI, Z80::A)) {
              DebugLoc DL = MI.getDebugLoc();
              unsigned HLOpc = IsInc ? Z80::LD_A_HLI : Z80::LD_A_HLD;
              unsigned HLSOpc = IsInc ? Z80::LD_HLI_A : Z80::LD_HLD_A;

              if (LoadDst.isValid()) {
                // LD r,(HL); INC/DEC HL → LD A,(HL+/-); LD r,A
                LLVM_DEBUG(dbgs() << "  LD r,(HL)+INC/DEC → HL+/-: " << MI);
                BuildMI(MBB, MI, DL, TII->get(HLOpc));
                BuildMI(MBB, MI, DL, TII->get(getLDrAOpcode(LoadDst)));
              } else {
                // LD (HL),r; INC/DEC HL → LD A,r; LD (HL+/-),A
                LLVM_DEBUG(dbgs() << "  LD (HL),r+INC/DEC → HL+/-: " << MI);
                BuildMI(MBB, MI, DL, TII->get(getLD8Opcode(Z80::A, StoreSrc)));
                BuildMI(MBB, MI, DL, TII->get(HLSOpc));
              }

              NextIt->eraseFromParent();
              MII = MBB.erase(MII);
              Changed = true;
              continue;
            }
          }
        }

        ++MII;
      }

      // --- Peephole: 16-bit HL load via HL+ (SM83 only) ---
      // When loading a 16-bit value from (HL) into HL itself via BC or DE:
      //   LD lo,(HL); INC HL; LD hi,(HL); LD L,lo; LD H,hi  (5B)
      // → LD A,(HL+); LD H,(HL); LD L,A                     (3B, saves 2B)
      // LD A,(HL+) loads lo byte and increments HL in one instruction.
      // LD H,(HL) reads the hi byte (HL still points to hi) before writing H.
      // LD L,A completes the 16-bit value in HL.
      // Conditions: A dead after (clobbered), register pair dead after (not
      // loaded).
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        MachineInstr &MI = *MII;
        unsigned Opc = MI.getOpcode();

        // I1: LD C,(HL) or LD E,(HL)
        bool IsBC = (Opc == Z80::LD_C_HLind);
        bool IsDE = (Opc == Z80::LD_E_HLind);
        if (!IsBC && !IsDE) {
          ++MII;
          continue;
        }

        auto I2 = std::next(MII);
        if (I2 == MIE || I2->getOpcode() != Z80::INC_HL) {
          ++MII;
          continue;
        }
        auto I3 = std::next(I2);
        if (I3 == MIE) {
          ++MII;
          continue;
        }
        unsigned ExpHi = IsBC ? Z80::LD_B_HLind : Z80::LD_D_HLind;
        if (I3->getOpcode() != ExpHi) {
          ++MII;
          continue;
        }

        auto I4 = std::next(I3);
        if (I4 == MIE) {
          ++MII;
          continue;
        }
        unsigned ExpLdL = IsBC ? Z80::LD_L_C : Z80::LD_L_E;
        if (I4->getOpcode() != ExpLdL) {
          ++MII;
          continue;
        }

        auto I5 = std::next(I4);
        if (I5 == MIE) {
          ++MII;
          continue;
        }
        unsigned ExpLdH = IsBC ? Z80::LD_H_B : Z80::LD_H_D;
        if (I5->getOpcode() != ExpLdH) {
          ++MII;
          continue;
        }

        auto After = std::next(I5);
        if (!isRegDeadAfter(After, MBB, TRI, Z80::A)) {
          ++MII;
          continue;
        }
        MCPhysReg PairReg = IsBC ? Z80::BC : Z80::DE;
        if (!isRegDeadAfter(After, MBB, TRI, PairReg)) {
          ++MII;
          continue;
        }

        LLVM_DEBUG(dbgs() << "  16-bit HL load via HL+: " << MI);
        DebugLoc DL = MI.getDebugLoc();
        BuildMI(MBB, MI, DL, TII->get(Z80::LD_A_HLI));
        BuildMI(MBB, MI, DL, TII->get(Z80::LD_H_HLind));
        BuildMI(MBB, MI, DL, TII->get(Z80::LD_L_A));

        I5->eraseFromParent();
        I4->eraseFromParent();
        I3->eraseFromParent();
        I2->eraseFromParent();
        MII = MBB.erase(MII);
        Changed = true;
      }
    }

    // --- SM83 SP-relative store-to-load forwarding ---
    // On SM83, stack access uses LDHL SP,#N; LD (HL),r / LD (HL),#imm.
    // Track what values (register or immediate) are at each stack offset,
    // then forward to subsequent loads to eliminate redundant LDHL sequences.
    if (MF.getSubtarget<Z80Subtarget>().hasSM83()) {
      // Each slot can hold either a register value or an immediate.
      struct SlotVal {
        bool IsImm = false;
        MCPhysReg Reg = 0;
        uint8_t Imm = 0;
      };
      DenseMap<int, SlotVal, IXOffsetInfo> SPSlots;
      int SPDelta = 0;

      auto invalidateSlotReg = [&](const TargetRegisterInfo *TRI,
                                   MCPhysReg Reg) {
        SmallVector<int, 4> ToErase;
        for (auto &KV : SPSlots) {
          if (!KV.second.IsImm && TRI->regsOverlap(KV.second.Reg, Reg))
            ToErase.push_back(KV.first);
        }
        for (int K : ToErase)
          SPSlots.erase(K);
      };

      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        MachineInstr &MI = *MII;
        unsigned Opc = MI.getOpcode();

        // Track SP changes.
        if (Opc == Z80::PUSH_AF || Opc == Z80::PUSH_BC || Opc == Z80::PUSH_DE ||
            Opc == Z80::PUSH_HL) {
          SPDelta -= 2;
          // PUSH writes to SPDelta+0 and SPDelta+1, invalidate those slots.
          SPSlots.erase(SPDelta);
          SPSlots.erase(SPDelta + 1);
          ++MII;
          continue;
        }
        if (Opc == Z80::POP_AF || Opc == Z80::POP_BC || Opc == Z80::POP_DE ||
            Opc == Z80::POP_HL) {
          // Invalidate slots at the popped location (no longer on stack).
          SPSlots.erase(SPDelta);
          SPSlots.erase(SPDelta + 1);
          SPDelta += 2;
          for (const MachineOperand &MO : MI.operands()) {
            if (MO.isReg() && MO.isDef() && MO.getReg().isPhysical())
              invalidateSlotReg(TRI, MO.getReg());
          }
          ++MII;
          continue;
        }
        if (Opc == Z80::ADD_SP_e) {
          int8_t Adj = (int8_t)(MI.getOperand(0).getImm() & 0xFF);
          SPDelta += Adj;
          ++MII;
          continue;
        }

        // Match LDHL SP,#N followed by store or load pattern.
        // (Check before side-effects: LDHL inherits hasSideEffects=1
        // from the conservative Z80Inst base but is actually safe.)
        if (Opc == Z80::LDHL_SP_e) {
          int8_t Imm = (int8_t)(MI.getOperand(0).getImm() & 0xFF);
          int AbsOff = SPDelta + Imm;

          auto It1 = std::next(MII);
          if (It1 == MIE) {
            ++MII;
            continue;
          }

          // Helper: check if a SlotVal matches a new store value.
          auto slotMatches = [](const SlotVal &Slot, bool NewIsImm,
                                MCPhysReg NewReg, uint8_t NewImm) -> bool {
            if (Slot.IsImm != NewIsImm)
              return false;
            if (Slot.IsImm)
              return Slot.Imm == NewImm;
            return Slot.Reg == NewReg;
          };

          // Helper: try to eliminate a redundant 16-bit store sequence.
          // Returns true if eliminated (LDHL + 3 instructions erased).
          auto tryElimRedundantStore =
              [&](int AbsOff, bool LoIsImm, MCPhysReg LoReg, uint8_t LoImm,
                  bool HiIsImm, MCPhysReg HiReg, uint8_t HiImm,
                  MachineBasicBlock::iterator LDHL,
                  MachineBasicBlock::iterator S1,
                  MachineBasicBlock::iterator Mid,
                  MachineBasicBlock::iterator S2) -> bool {
            auto AvLo = SPSlots.find(AbsOff);
            auto AvHi = SPSlots.find(AbsOff + 1);
            if (AvLo == SPSlots.end() || AvHi == SPSlots.end())
              return false;
            if (!slotMatches(AvLo->second, LoIsImm, LoReg, LoImm) ||
                !slotMatches(AvHi->second, HiIsImm, HiReg, HiImm))
              return false;
            // Values match. Safe to remove if HL and FLAGS are dead after.
            auto AfterStore = std::next(S2);
            if (!isRegDeadAfter(AfterStore, MBB, TRI, Z80::HL) ||
                !isRegDeadAfter(AfterStore, MBB, TRI, Z80::FLAGS))
              return false;
            LLVM_DEBUG(dbgs() << "  SM83 eliminating redundant store SP+"
                              << AbsOff << "\n");
            S2->eraseFromParent();
            Mid->eraseFromParent();
            S1->eraseFromParent();
            MII = MBB.erase(LDHL);
            Changed = true;
            return true;
          };

          // --- 16-bit immediate store: LDHL; LD (HL),#lo; INC HL; LD (HL),#hi
          if (It1->getOpcode() == Z80::LD_HLind_n) {
            auto It2 = std::next(It1);
            if (It2 != MIE && It2->getOpcode() == Z80::INC_HL) {
              auto It3 = std::next(It2);
              if (It3 != MIE && It3->getOpcode() == Z80::LD_HLind_n) {
                uint8_t LoVal = (uint8_t)(It1->getOperand(0).getImm() & 0xFF);
                uint8_t HiVal = (uint8_t)(It3->getOperand(0).getImm() & 0xFF);
                // Try redundant store elimination.
                if (tryElimRedundantStore(AbsOff, true, 0, LoVal, true, 0,
                                          HiVal, MII, It1, It2, It3))
                  continue;
                SlotVal SLo, SHi;
                SLo.IsImm = true;
                SLo.Imm = LoVal;
                SHi.IsImm = true;
                SHi.Imm = HiVal;
                SPSlots[AbsOff] = SLo;
                SPSlots[AbsOff + 1] = SHi;
                LLVM_DEBUG(dbgs() << "  SM83 imm store SP+" << AbsOff << " <- #"
                                  << (int)SLo.Imm << ", SP+" << (AbsOff + 1)
                                  << " <- #" << (int)SHi.Imm << "\n");
                MII = std::next(It3);
                continue;
              }
            }
            // 8-bit immediate store
            SlotVal S;
            S.IsImm = true;
            S.Imm = (uint8_t)(It1->getOperand(0).getImm() & 0xFF);
            SPSlots[AbsOff] = S;
            MII = std::next(It1);
            continue;
          }

          // --- 16-bit register store: LDHL; LD (HL),rlo; INC HL; LD (HL),rhi
          Register StoreSrc1 = getStoreHLindSrcReg(It1->getOpcode());
          if (StoreSrc1.isValid()) {
            auto It2 = std::next(It1);
            if (It2 != MIE && It2->getOpcode() == Z80::INC_HL) {
              auto It3 = std::next(It2);
              if (It3 != MIE) {
                Register StoreSrc2 = getStoreHLindSrcReg(It3->getOpcode());
                if (StoreSrc2.isValid()) {
                  // Try redundant store elimination.
                  if (tryElimRedundantStore(AbsOff, false, StoreSrc1, 0, false,
                                            StoreSrc2, 0, MII, It1, It2, It3))
                    continue;
                  SlotVal SLo, SHi;
                  SLo.Reg = StoreSrc1;
                  SHi.Reg = StoreSrc2;
                  SPSlots[AbsOff] = SLo;
                  SPSlots[AbsOff + 1] = SHi;
                  LLVM_DEBUG(dbgs() << "  SM83 reg store SP+" << AbsOff
                                    << " <- " << printReg(StoreSrc1, TRI)
                                    << ", SP+" << (AbsOff + 1) << " <- "
                                    << printReg(StoreSrc2, TRI) << "\n");
                  MII = std::next(It3);
                  continue;
                }
              }
            }
            // 8-bit register store
            SlotVal S;
            S.Reg = StoreSrc1;
            SPSlots[AbsOff] = S;
            MII = std::next(It1);
            continue;
          }

          // --- HL+ register store: LDHL; LD A,r; LD (HL+),A; LD (HL),r2
          {
            Register SrcLo = getLDArSrcReg(It1->getOpcode());
            // Only B/C/D/E — H/L can't be source (LDHL clobbered HL).
            if (SrcLo.isValid() && SrcLo != Z80::H && SrcLo != Z80::L) {
              auto It2 = std::next(It1);
              if (It2 != MIE && It2->getOpcode() == Z80::LD_HLI_A) {
                auto It3 = std::next(It2);
                if (It3 != MIE) {
                  Register StoreSrc2 = getStoreHLindSrcReg(It3->getOpcode());
                  if (StoreSrc2.isValid()) {
                    // Try redundant store elimination.
                    if (tryElimRedundantStore(AbsOff, false, SrcLo, 0, false,
                                              StoreSrc2, 0, MII, It1, It2, It3))
                      continue;
                    SlotVal SLo, SHi;
                    SLo.Reg = SrcLo;
                    SHi.Reg = StoreSrc2;
                    SPSlots[AbsOff] = SLo;
                    SPSlots[AbsOff + 1] = SHi;
                    LLVM_DEBUG(dbgs() << "  SM83 HL+ store SP+" << AbsOff
                                      << " <- " << printReg(SrcLo, TRI)
                                      << ", SP+" << (AbsOff + 1) << " <- "
                                      << printReg(StoreSrc2, TRI) << "\n");
                    MII = std::next(It3);
                    continue;
                  }
                }
              }
            }
          }

          // --- 16-bit load: LDHL; LD lo,(HL); INC HL; LD hi,(HL)
          Register LoadDst1 = getLoadHLindDstReg(It1->getOpcode());
          if (LoadDst1.isValid()) {
            auto It2 = std::next(It1);
            if (It2 != MIE && It2->getOpcode() == Z80::INC_HL) {
              auto It3 = std::next(It2);
              if (It3 != MIE) {
                Register LoadDst2 = getLoadHLindDstReg(It3->getOpcode());
                if (LoadDst2.isValid()) {
                  auto AvLo = SPSlots.find(AbsOff);
                  auto AvHi = SPSlots.find(AbsOff + 1);
                  if (AvLo != SPSlots.end() && AvHi != SPSlots.end()) {
                    SlotVal &SLo = AvLo->second;
                    SlotVal &SHi = AvHi->second;
                    // Forwarding removes LDHL which sets HL and FLAGS.
                    // Verify both are dead after the load sequence.
                    auto AfterLoad = std::next(It3);
                    if (!isRegDeadAfter(AfterLoad, MBB, TRI, Z80::HL) ||
                        !isRegDeadAfter(AfterLoad, MBB, TRI, Z80::FLAGS)) {
                      // Can't forward — fall through to tracking update.
                      invalidateSlotReg(TRI, LoadDst1);
                      invalidateSlotReg(TRI, LoadDst2);
                      SPSlots[AbsOff] = {false, MCPhysReg(LoadDst1), 0};
                      SPSlots[AbsOff + 1] = {false, MCPhysReg(LoadDst2), 0};
                      MII = std::next(It3);
                      continue;
                    }
                    // Build replacement instructions.
                    DebugLoc DL = MI.getDebugLoc();
                    bool CanForward = true;
                    // For register sources: can't use H/L (LDHL clobbers).
                    if (!SLo.IsImm && (SLo.Reg == Z80::H || SLo.Reg == Z80::L))
                      CanForward = false;
                    if (!SHi.IsImm && (SHi.Reg == Z80::H || SHi.Reg == Z80::L))
                      CanForward = false;
                    // Check reg-reg copy feasibility and ordering.
                    if (CanForward && !SLo.IsImm && !SHi.IsImm) {
                      // Both register: check for circular dependency.
                      bool LoIsNop = (LoadDst1 == SLo.Reg);
                      bool HiIsNop = (LoadDst2 == SHi.Reg);
                      unsigned OpcLo =
                          LoIsNop ? 0 : getLD8Opcode(LoadDst1, SLo.Reg);
                      unsigned OpcHi =
                          HiIsNop ? 0 : getLD8Opcode(LoadDst2, SHi.Reg);
                      if (!LoIsNop && !OpcLo)
                        CanForward = false;
                      if (!HiIsNop && !OpcHi)
                        CanForward = false;
                      if (CanForward) {
                        bool HiFirst = TRI->regsOverlap(LoadDst1, SHi.Reg);
                        if (HiFirst && TRI->regsOverlap(LoadDst2, SLo.Reg))
                          CanForward = false; // Circular.
                        if (CanForward) {
                          LLVM_DEBUG(dbgs() << "  SM83 fwd 16-bit reg SP+"
                                            << AbsOff << "\n");
                          if (HiFirst) {
                            if (OpcHi)
                              BuildMI(MBB, MI, DL, TII->get(OpcHi));
                            if (OpcLo)
                              BuildMI(MBB, MI, DL, TII->get(OpcLo));
                          } else {
                            if (OpcLo)
                              BuildMI(MBB, MI, DL, TII->get(OpcLo));
                            if (OpcHi)
                              BuildMI(MBB, MI, DL, TII->get(OpcHi));
                          }
                          It3->eraseFromParent();
                          It2->eraseFromParent();
                          It1->eraseFromParent();
                          MII = MBB.erase(MII);
                          Changed = true;
                          invalidateSlotReg(TRI, LoadDst1);
                          invalidateSlotReg(TRI, LoadDst2);
                          SPSlots[AbsOff] = {false, MCPhysReg(LoadDst1), 0};
                          SPSlots[AbsOff + 1] = {false, MCPhysReg(LoadDst2), 0};
                          continue;
                        }
                      }
                    }
                    // At least one immediate: generate LD r,#imm for imm
                    // slots and LD r,src for register slots.
                    if (CanForward) {
                      // Pre-validate all opcodes before emitting anything,
                      // to avoid partially-emitted instructions on failure.
                      auto getSlotOpc = [&](Register Dst,
                                            SlotVal &S) -> unsigned {
                        if (S.IsImm)
                          return getLDrnOpcode(Dst);
                        return (Dst == S.Reg) ? ~0u : getLD8Opcode(Dst, S.Reg);
                      };
                      unsigned OpcLo = getSlotOpc(LoadDst1, SLo);
                      unsigned OpcHi = getSlotOpc(LoadDst2, SHi);
                      if (!OpcLo || !OpcHi)
                        CanForward = false;
                    }
                    if (CanForward) {
                      bool HiFirst = false;
                      if (!SHi.IsImm && TRI->regsOverlap(LoadDst1, SHi.Reg))
                        HiFirst = true;

                      LLVM_DEBUG(dbgs() << "  SM83 fwd 16-bit imm/reg SP+"
                                        << AbsOff << "\n");
                      auto emitLoad = [&](Register Dst, SlotVal &S) {
                        if (S.IsImm) {
                          BuildMI(MBB, MI, DL, TII->get(getLDrnOpcode(Dst)))
                              .addImm(S.Imm);
                        } else if (Dst != S.Reg) {
                          BuildMI(MBB, MI, DL,
                                  TII->get(getLD8Opcode(Dst, S.Reg)));
                        }
                      };
                      if (HiFirst) {
                        emitLoad(LoadDst2, SHi);
                        emitLoad(LoadDst1, SLo);
                      } else {
                        emitLoad(LoadDst1, SLo);
                        emitLoad(LoadDst2, SHi);
                      }
                      It3->eraseFromParent();
                      It2->eraseFromParent();
                      It1->eraseFromParent();
                      MII = MBB.erase(MII);
                      Changed = true;
                      invalidateSlotReg(TRI, LoadDst1);
                      invalidateSlotReg(TRI, LoadDst2);
                      SPSlots[AbsOff] = {false, MCPhysReg(LoadDst1), 0};
                      SPSlots[AbsOff + 1] = {false, MCPhysReg(LoadDst2), 0};
                      continue;
                    }
                  }
                  // Couldn't forward — update tracking.
                  invalidateSlotReg(TRI, LoadDst1);
                  invalidateSlotReg(TRI, LoadDst2);
                  SPSlots[AbsOff] = {false, MCPhysReg(LoadDst1), 0};
                  SPSlots[AbsOff + 1] = {false, MCPhysReg(LoadDst2), 0};
                  MII = std::next(It3);
                  continue;
                }
              }
            }
            // 8-bit load: LDHL; LD r,(HL)
            // Forwarding removes LDHL (sets HL/FLAGS), so both must be dead.
            auto After8 = std::next(It1);
            if (!isRegDeadAfter(After8, MBB, TRI, Z80::HL) ||
                !isRegDeadAfter(After8, MBB, TRI, Z80::FLAGS)) {
              invalidateSlotReg(TRI, LoadDst1);
              SPSlots[AbsOff] = {false, MCPhysReg(LoadDst1), 0};
              MII = std::next(It1);
              continue;
            }
            auto AvIt = SPSlots.find(AbsOff);
            if (AvIt != SPSlots.end()) {
              SlotVal &S = AvIt->second;
              DebugLoc DL = MI.getDebugLoc();
              bool Done = false;
              if (S.IsImm) {
                unsigned LdOpc = getLDrnOpcode(LoadDst1);
                if (LdOpc) {
                  LLVM_DEBUG(dbgs() << "  SM83 fwd 8-bit imm SP+" << AbsOff
                                    << " #" << (int)S.Imm << "\n");
                  BuildMI(MBB, MI, DL, TII->get(LdOpc)).addImm(S.Imm);
                  It1->eraseFromParent();
                  MII = MBB.erase(MII);
                  Changed = true;
                  Done = true;
                }
              } else if (S.Reg != Z80::H && S.Reg != Z80::L) {
                unsigned CopyOpc =
                    (LoadDst1 == S.Reg) ? 0 : getLD8Opcode(LoadDst1, S.Reg);
                if (LoadDst1 == S.Reg || CopyOpc) {
                  LLVM_DEBUG(dbgs()
                             << "  SM83 fwd 8-bit reg SP+" << AbsOff << "\n");
                  if (CopyOpc)
                    BuildMI(MBB, MI, DL, TII->get(CopyOpc));
                  It1->eraseFromParent();
                  MII = MBB.erase(MII);
                  Changed = true;
                  Done = true;
                }
              }
              if (Done) {
                invalidateSlotReg(TRI, LoadDst1);
                SPSlots[AbsOff] = {false, MCPhysReg(LoadDst1), 0};
                continue;
              }
            }
            invalidateSlotReg(TRI, LoadDst1);
            SPSlots[AbsOff] = {false, MCPhysReg(LoadDst1), 0};
            MII = std::next(It1);
            continue;
          }

          // LDHL not followed by a recognizable pattern — HL is clobbered.
          invalidateSlotReg(TRI, Z80::HL);
          ++MII;
          continue;
        }

        // Calls and unmodeled side effects clear everything.
        if (MI.isCall() || MI.hasUnmodeledSideEffects()) {
          SPSlots.clear();
          ++MII;
          continue;
        }

        // Any other instruction: invalidate entries for defined regs.
        for (const MachineOperand &MO : MI.operands()) {
          if (MO.isReg() && MO.isDef() && MO.getReg().isPhysical())
            invalidateSlotReg(TRI, MO.getReg());
        }
        for (MCPhysReg Def : TII->get(Opc).implicit_defs())
          invalidateSlotReg(TRI, Def);
        ++MII;
      }
    }

    // --- Store-to-load forwarding and register copy elimination ---
    // Map from IX offset to the physical register holding that value.
    DenseMap<int, MCPhysReg, IXOffsetInfo> AvailValues;

    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      MachineInstr &MI = *MII++;
      unsigned Opc = MI.getOpcode();

      // Case 1: IX-indexed store — LD (IX+d), R
      Register StoreSrc = getStoreIXdSrcReg(Opc);
      if (StoreSrc.isValid()) {
        int Offset = MI.getOperand(0).getImm();
        if (MI.memoperands_empty()) {
          // Spill expansion — track the value.
          AvailValues[Offset] = StoreSrc;
        } else {
          // User code (possibly volatile) — invalidate this slot.
          AvailValues.erase(Offset);
        }
        continue;
      }

      // Case 2: IX-indexed load — LD R', (IX+d)
      Register LoadDst = getLoadIXdDstReg(Opc);
      if (LoadDst.isValid()) {
        int Offset = MI.getOperand(0).getImm();

        if (MI.memoperands_empty()) {
          auto It = AvailValues.find(Offset);
          if (It != AvailValues.end()) {
            MCPhysReg SrcReg = It->second;
            if (LoadDst == SrcReg) {
              // LD R, (IX+d) where R already holds the value — no-op.
              // Don't invalidate anything: R's value doesn't change.
              LLVM_DEBUG(dbgs() << "  Eliminating redundant reload: " << MI);
              MI.eraseFromParent();
              Changed = true;
              continue;
            }
            // Replace LD R', (IX+d) with LD R', R_src.
            unsigned NewOpc = getLD8Opcode(LoadDst, SrcReg);
            if (NewOpc) {
              LLVM_DEBUG(dbgs() << "  Forwarding: " << MI << "  -> LD "
                                << printReg(LoadDst, TRI) << ", "
                                << printReg(SrcReg, TRI) << "\n");
              // R' gets a new value — invalidate other entries pointing to R'.
              invalidateReg(AvailValues, TRI, LoadDst);
              BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(NewOpc));
              MI.eraseFromParent();
              Changed = true;
              AvailValues[Offset] = LoadDst;
              continue;
            }
          }
        }
        // Couldn't forward — R' gets a new value from memory.
        // Invalidate entries pointing to R' (they're stale).
        invalidateReg(AvailValues, TRI, LoadDst);
        // R' now holds the value at offset d.
        if (MI.memoperands_empty())
          AvailValues[Offset] = LoadDst;
        continue;
      }

      // Case 3: LD (IX+d), n — immediate store to IX slot
      if (Opc == Z80::LD_IXd_n) {
        int Offset = MI.getOperand(0).getImm();
        AvailValues.erase(Offset);
        continue;
      }

      // Case 4: Calls and unmodeled side effects — clear everything.
      if (MI.isCall() || MI.hasUnmodeledSideEffects()) {
        AvailValues.clear();
        continue;
      }

      // Case 5: Any other instruction — invalidate entries for defined regs.
      for (const MachineOperand &MO : MI.operands()) {
        if (MO.isReg() && MO.isDef() && MO.getReg().isPhysical())
          invalidateReg(AvailValues, TRI, MO.getReg());
      }
      // Also check implicit defs from the instruction descriptor.
      for (MCPhysReg Def : TII->get(Opc).implicit_defs())
        invalidateReg(AvailValues, TRI, Def);
    }
  }

  // --- EXX shadow bank spill conversion ---
  // DISABLED: EXX swaps ALL of BC/DE/HL simultaneously, destroying any
  // live values in those registers. The conversion can only be safely
  // applied at points where ALL three pairs are dead (function entry/exit,
  // after CALL). Converting arbitrary IX spill/reload pairs mid-function
  // corrupts live register values.
  //
  // A correct implementation would need to:
  // 1. Only insert EXX at safe points (after CALL, at function boundaries)
  // 2. Batch multiple spills into a single EXX region
  // 3. Verify no live BC/DE/HL values cross the EXX boundary
  //
  // This is essentially a register bank scheduling problem.
#if 0
  // Convert 16-bit IX-indexed spill/reload pairs to EXX-based transfers.
  // LD (IX+d),L; LD (IX+d+1),H (6B) → PUSH HL; EXX; POP HL; EXX (4B)
  // LD L,(IX+d); LD H,(IX+d+1) (6B) → EXX; PUSH HL; EXX; POP HL (4B)
  // Saves 2 bytes per 16-bit pair access. Max 3 shadow pairs (BC',DE',HL').
  if (STI.hasZ80() && STI.shadowRegs() &&
      !MF.getFunction().hasFnAttribute("interrupt")) {
    // Map IX offset → shadow pair register. Max 3 pairs.
    struct SpillSlot {
      int8_t LoOffset; // IX offset of low byte
      unsigned PushOpc; // PUSH_HL, PUSH_DE, or PUSH_BC
      unsigned PopOpc;  // POP_HL, POP_DE, or POP_BC
    };
    SmallDenseMap<int8_t, SpillSlot> ShadowSlots;
    static const unsigned PushOps[] = {Z80::PUSH_HL, Z80::PUSH_DE, Z80::PUSH_BC};
    static const unsigned PopOps[] = {Z80::POP_HL, Z80::POP_DE, Z80::POP_BC};
    unsigned NextSlot = 0;

    // Helper: check if two adjacent instructions form a 16-bit IX store pair.
    // Returns the lo-byte IX offset, or INT8_MIN if not a pair.
    auto isIXStorePair = [](MachineInstr &MI1, MachineInstr &MI2,
                            MCPhysReg &LoReg, MCPhysReg &HiReg) -> int8_t {
      // LD (IX+d),lo; LD (IX+d+1),hi
      static const struct { unsigned Opc; MCPhysReg Reg; } StoreMap[] = {
        {Z80::LD_IXd_B, Z80::B}, {Z80::LD_IXd_C, Z80::C},
        {Z80::LD_IXd_D, Z80::D}, {Z80::LD_IXd_E, Z80::E},
        {Z80::LD_IXd_H, Z80::H}, {Z80::LD_IXd_L, Z80::L},
        {Z80::LD_IXd_A, Z80::A},
      };
      MCPhysReg R1 = 0, R2 = 0;
      int8_t Off1 = INT8_MIN, Off2 = INT8_MIN;
      for (auto &S : StoreMap) {
        if (MI1.getOpcode() == S.Opc) { R1 = S.Reg; Off1 = MI1.getOperand(0).getImm(); }
        if (MI2.getOpcode() == S.Opc) { R2 = S.Reg; Off2 = MI2.getOperand(0).getImm(); }
      }
      if (!R1 || !R2 || Off2 != Off1 + 1) return INT8_MIN;
      LoReg = R1; HiReg = R2;
      return Off1;
    };

    // Helper: check if two adjacent instructions form a 16-bit IX load pair.
    auto isIXLoadPair = [](MachineInstr &MI1, MachineInstr &MI2,
                           MCPhysReg &LoReg, MCPhysReg &HiReg) -> int8_t {
      static const struct { unsigned Opc; MCPhysReg Reg; } LoadMap[] = {
        {Z80::LD_B_IXd, Z80::B}, {Z80::LD_C_IXd, Z80::C},
        {Z80::LD_D_IXd, Z80::D}, {Z80::LD_E_IXd, Z80::E},
        {Z80::LD_H_IXd, Z80::H}, {Z80::LD_L_IXd, Z80::L},
        {Z80::LD_A_IXd, Z80::A},
      };
      MCPhysReg R1 = 0, R2 = 0;
      int8_t Off1 = INT8_MIN, Off2 = INT8_MIN;
      for (auto &S : LoadMap) {
        if (MI1.getOpcode() == S.Opc) { R1 = S.Reg; Off1 = MI1.getOperand(0).getImm(); }
        if (MI2.getOpcode() == S.Opc) { R2 = S.Reg; Off2 = MI2.getOperand(0).getImm(); }
      }
      if (!R1 || !R2 || Off2 != Off1 + 1) return INT8_MIN;
      LoReg = R1; HiReg = R2;
      return Off1;
    };

    // Helper: get PUSH/POP opcode for a register pair.
    auto getPushPop = [](MCPhysReg Lo, MCPhysReg Hi,
                         unsigned &Push, unsigned &Pop) -> bool {
      if ((Lo == Z80::L && Hi == Z80::H) || (Lo == Z80::H && Hi == Z80::L))
        { Push = Z80::PUSH_HL; Pop = Z80::POP_HL; return true; }
      if ((Lo == Z80::E && Hi == Z80::D) || (Lo == Z80::D && Hi == Z80::E))
        { Push = Z80::PUSH_DE; Pop = Z80::POP_DE; return true; }
      if ((Lo == Z80::C && Hi == Z80::B) || (Lo == Z80::B && Hi == Z80::C))
        { Push = Z80::PUSH_BC; Pop = Z80::POP_BC; return true; }
      return false; // Not a standard pair (e.g., A+L)
    };

    // First pass: identify all 16-bit IX spill pairs and assign shadow slots.
    for (MachineBasicBlock &MBB : MF) {
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ++MII) {
        auto NextII = std::next(MII);
        if (NextII == MIE) continue;
        MCPhysReg Lo, Hi;
        int8_t Off = isIXStorePair(*MII, *NextII, Lo, Hi);
        if (Off == INT8_MIN)
          Off = isIXLoadPair(*MII, *NextII, Lo, Hi);
        if (Off == INT8_MIN) continue;
        unsigned Push, Pop;
        if (!getPushPop(Lo, Hi, Push, Pop)) continue;

        if (ShadowSlots.count(Off) == 0 && NextSlot < 3) {
          ShadowSlots[Off] = {Off, PushOps[NextSlot], PopOps[NextSlot]};
          NextSlot++;
        }
      }
    }

    // Second pass: convert matched pairs.
    if (!ShadowSlots.empty()) {
      for (MachineBasicBlock &MBB : MF) {
        for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
          auto NextII = std::next(MII);
          if (NextII == MIE) { ++MII; continue; }
          MCPhysReg Lo, Hi;
          unsigned SrcPush, SrcPop;
          DebugLoc DL = MII->getDebugLoc();

          // Check for 16-bit store pair.
          int8_t Off = isIXStorePair(*MII, *NextII, Lo, Hi);
          if (Off != INT8_MIN && getPushPop(Lo, Hi, SrcPush, SrcPop)) {
            auto It = ShadowSlots.find(Off);
            if (It != ShadowSlots.end()) {
              // LD (IX+d),lo; LD (IX+d+1),hi → PUSH pair; EXX; POP shadow; EXX
              auto &Slot = It->second;
              auto InsertPt = std::next(NextII);
              NextII->eraseFromParent();
              MII = MBB.erase(MII);
              BuildMI(MBB, MII, DL, TII->get(SrcPush));
              BuildMI(MBB, MII, DL, TII->get(Z80::EXX));
              BuildMI(MBB, MII, DL, TII->get(Slot.PopOpc));
              BuildMI(MBB, MII, DL, TII->get(Z80::EXX));
              Changed = true;
              continue;
            }
          }

          // Check for 16-bit load pair.
          Off = isIXLoadPair(*MII, *NextII, Lo, Hi);
          if (Off != INT8_MIN && getPushPop(Lo, Hi, SrcPush, SrcPop)) {
            auto It = ShadowSlots.find(Off);
            if (It != ShadowSlots.end()) {
              // LD lo,(IX+d); LD hi,(IX+d+1) → EXX; PUSH shadow; EXX; POP pair
              auto &Slot = It->second;
              NextII->eraseFromParent();
              MII = MBB.erase(MII);
              BuildMI(MBB, MII, DL, TII->get(Z80::EXX));
              BuildMI(MBB, MII, DL, TII->get(Slot.PushOpc));
              BuildMI(MBB, MII, DL, TII->get(Z80::EXX));
              BuildMI(MBB, MII, DL, TII->get(SrcPop));
              Changed = true;
              continue;
            }
          }
          ++MII;
        }
      }
    }
  }
#endif // disabled EXX conversion

  // --- Peephole: CALL nn; RET → JP nn (tail call) ---
  // When a function's last action is CALL followed by RET, replace with JP.
  // The callee's RET returns directly to our caller.
  // ONLY safe when no stack arguments were pushed for the callee (JP doesn't
  // push a return address, so the callee's stack frame would be wrong if
  // stack args are present). We verify by checking that no PUSH instructions
  // appear in the MBB before the CALL.
  for (auto &MBB : MF) {
    auto Term = MBB.getLastNonDebugInstr();
    if (Term == MBB.end())
      continue;

    // Match RET or RET_CLEANUP 0.
    unsigned TermOpc = Term->getOpcode();
    bool IsRet = (TermOpc == Z80::RET);
    bool IsRetCleanup0 = (TermOpc == Z80::RET_CLEANUP &&
                          Term->getOperand(0).getImm() == 0);
    if (!IsRet && !IsRetCleanup0)
      continue;

    // Check for CALL_nn immediately before RET.
    auto CallIt = Term;
    if (CallIt == MBB.begin())
      continue;
    --CallIt;
    while (CallIt != MBB.begin() && CallIt->isDebugInstr())
      --CallIt;
    if (CallIt->getOpcode() != Z80::CALL_nn)
      continue;

    // Verify no PUSHes in this MBB before the CALL (stack args would make
    // the tail call unsafe — callee expects a return address at SP).
    bool HasPush = false;
    for (auto It = MBB.begin(); It != CallIt; ++It) {
      if (It->isDebugInstr())
        continue;
      unsigned Opc = It->getOpcode();
      if (Opc == Z80::PUSH_AF || Opc == Z80::PUSH_BC || Opc == Z80::PUSH_DE ||
          Opc == Z80::PUSH_HL || Opc == Z80::PUSH_IX || Opc == Z80::PUSH_IY) {
        HasPush = true;
        break;
      }
    }
    if (HasPush)
      continue;

    // Replace CALL nn; RET with TAILJMP (JP to external function).
    // TAILJMP is isReturn + isTerminator but NOT isBranch, so branch
    // relaxation and branch cleanup don't process it. Lowered to JP_nn
    // in the assembly printer.
    MachineOperand &CallTarget = CallIt->getOperand(0);
    LLVM_DEBUG(dbgs() << "  CALL; RET → JP (tail call): " << *CallIt);
    DebugLoc DL = CallIt->getDebugLoc();
    BuildMI(MBB, *CallIt, DL, TII->get(Z80::TAILJMP)).add(CallTarget);
    Term->eraseFromParent();
    CallIt->eraseFromParent();
    Changed = true;
  }

  // --- Peephole: PUSH IX; POP HL; ADD HL,rr; PUSH HL; POP IX → ADD IX,rr ---
  // When a 16-bit addition is performed through HL with IX/IY as the actual
  // accumulator, replace the 5-instruction copy-add-copy sequence (8-10 bytes)
  // with a single ADD IX,rr or ADD IY,rr (2 bytes).
  // Also handles ADD HL,HL → ADD IX,IX (left shift by 1).
  if (STI.hasZ80()) {
    for (auto &MBB : MF) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        unsigned Opc = MII->getOpcode();
        // Match: PUSH IX or PUSH IY
        bool IsIX = (Opc == Z80::PUSH_IX);
        bool IsIY = (Opc == Z80::PUSH_IY);
        if (!IsIX && !IsIY) { ++MII; continue; }

        auto I1 = MII;       // PUSH IX/IY
        auto I2 = std::next(I1);
        if (I2 == MIE || I2->getOpcode() != Z80::POP_HL) { ++MII; continue; }
        auto I3 = std::next(I2);
        if (I3 == MIE) { ++MII; continue; }
        // I3 must be ADD HL,BC or ADD HL,DE or ADD HL,HL
        unsigned AddOpc = I3->getOpcode();
        if (AddOpc != Z80::ADD_HL_BC && AddOpc != Z80::ADD_HL_DE &&
            AddOpc != Z80::ADD_HL_HL) { ++MII; continue; }
        auto I4 = std::next(I3);
        if (I4 == MIE || I4->getOpcode() != Z80::PUSH_HL) { ++MII; continue; }
        auto I5 = std::next(I4);
        unsigned ExpectedPop = IsIX ? Z80::POP_IX : Z80::POP_IY;
        if (I5 == MIE || I5->getOpcode() != ExpectedPop) { ++MII; continue; }

        // Determine replacement opcode
        unsigned NewOpc = 0;
        if (IsIX) {
          if (AddOpc == Z80::ADD_HL_BC) NewOpc = Z80::ADD_IX_BC;
          else if (AddOpc == Z80::ADD_HL_DE) NewOpc = Z80::ADD_IX_DE;
          else if (AddOpc == Z80::ADD_HL_HL) NewOpc = Z80::ADD_IX_IX;
        } else {
          if (AddOpc == Z80::ADD_HL_BC) NewOpc = Z80::ADD_IY_BC;
          else if (AddOpc == Z80::ADD_HL_DE) NewOpc = Z80::ADD_IY_DE;
          else if (AddOpc == Z80::ADD_HL_HL) NewOpc = Z80::ADD_IY_IY;
        }
        if (!NewOpc) { ++MII; continue; }

        LLVM_DEBUG(dbgs() << "  ADD IX/IY peephole: PUSH;POP;ADD;PUSH;POP → "
                          << TII->getName(NewOpc) << "\n");
        DebugLoc DL = I3->getDebugLoc();
        I5->eraseFromParent();
        I4->eraseFromParent();
        I3->eraseFromParent();
        I2->eraseFromParent();
        MII = MBB.erase(I1);
        BuildMI(MBB, MII, DL, TII->get(NewOpc));
        Changed = true;
      }
    }
  }

  // --- Peephole: BSS spill/reload → PUSH/POP across CALLs ---
  // With static-stack, register allocator spills use BSS direct addressing:
  //   LD (bss),A  (3B/13T)  +  LD A,(bss)  (3B/13T)  =  6B/26T per pair
  // PUSH AF (1B/11T) + POP AF (1B/10T) = 2B/21T — saves 4B and 5T per pair.
  // For GR16: LD (bss),rr (3-4B) + LD rr,(bss) (3-4B) → PUSH/POP (2B).
  //
  // Pattern: LD (sfrend),reg ... CALL ... LD reg,(sfrend) with same address.
  // If the value is loaded multiple times, insert PUSH after each POP to keep
  // the value on the stack for subsequent POPs (1B overhead per extra use,
  // still cheaper than 3B BSS reload).
  //
  // Safety: with static-stack there is no SP-relative addressing, so PUSH/POP
  // cannot break address calculations.  Only fire for __sfrend/__sframe symbols.
  if (STI.staticStack()) {
    // Helper: get PUSH/POP opcodes for a BSS store/load pair.
    struct SpillInfo {
      unsigned StoreOpc;   // LD_nnind_A, LD_nnind_HL, etc.
      unsigned LoadOpc;    // LD_A_nnind, LD_HL_nnind, etc.
      unsigned PushOpc;    // PUSH_AF, PUSH_HL, etc.
      unsigned PopOpc;     // POP_AF, POP_HL, etc.
      unsigned StoreBytes; // size of store instruction
      unsigned LoadBytes;  // size of load instruction
    };
    static const SpillInfo SpillPairs[] = {
        {Z80::LD_nnind_A,  Z80::LD_A_nnind,  Z80::PUSH_AF, Z80::POP_AF, 3, 3},
        {Z80::LD_nnind_HL, Z80::LD_HL_nnind, Z80::PUSH_HL, Z80::POP_HL, 3, 3},
        {Z80::LD_nnind_DE, Z80::LD_DE_nnind, Z80::PUSH_DE, Z80::POP_DE, 4, 4},
        {Z80::LD_nnind_BC, Z80::LD_BC_nnind, Z80::PUSH_BC, Z80::POP_BC, 4, 4},
    };

    auto getSpillInfo = [&](unsigned Opc) -> const SpillInfo * {
      for (const auto &SI : SpillPairs)
        if (SI.StoreOpc == Opc) return &SI;
      return nullptr;
    };

    auto isMatchingLoad = [&](unsigned StoreOpc, unsigned Opc) -> bool {
      for (const auto &SI : SpillPairs)
        if (SI.StoreOpc == StoreOpc && SI.LoadOpc == Opc) return true;
      return false;
    };

    auto isSfrendSymbol = [](const MachineOperand &MO) -> bool {
      if (!MO.isMCSymbol()) return false;
      StringRef Name = MO.getMCSymbol()->getName();
      if (!Name.starts_with("__sfrend") && !Name.starts_with("__sframe"))
        return false;
      return true;
    };

    auto sameAddress = [](const MachineInstr &A, const MachineInstr &B) -> bool {
      // Compare the address operand (operand 0 for both store and load).
      return A.getOperand(0).isIdenticalTo(B.getOperand(0));
    };

    for (MachineBasicBlock &MBB : MF) {
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ++MII) {
        const SpillInfo *SI = getSpillInfo(MII->getOpcode());
        if (!SI) continue;
        if (!isSfrendSymbol(MII->getOperand(0))) continue;

        // Found a BSS spill store.  Scan forward for CALLs and matching loads.
        // Count how many loads reference this same address after the store.
        // Also track whether any other store to the same address intervenes
        // (which would mean the slot is reused for a different value).
        bool HasCall = false;
        bool Conflict = false;
        int LoadCount = 0;
        int PushPopBytes = 1; // initial PUSH
        int BssBytes = SI->StoreBytes;
        SmallVector<MachineBasicBlock::iterator, 4> Loads;
        // Track PUSH/POP balance for the same register pair.
        // Our PUSH will be at the store position.  If an existing POP of
        // the same pair appears between our PUSH and the matching load,
        // it would steal our value (LIFO).  Similarly, if a previous
        // conversion inserted a PUSH/POP pair, we must not interleave.
        int StackDepth = 0;

        for (auto Scan = std::next(MII); Scan != MIE; ++Scan) {
          unsigned SOpc = Scan->getOpcode();

          // Another store to the same sfrend slot = conflict (reuse).
          if (SOpc == SI->StoreOpc && sameAddress(*MII, *Scan)) {
            Conflict = true;
            break;
          }

          // Track PUSH/POP of the same register pair between our store
          // and its matching load.  A negative depth means an existing
          // POP would consume our pushed value before we reach it.
          if (SOpc == SI->PushOpc) StackDepth++;
          if (SOpc == SI->PopOpc) {
            StackDepth--;
            if (StackDepth < 0) { Conflict = true; break; }
          }

          if (SOpc == Z80::CALL_nn)
            HasCall = true;

          // Matching load from the same address.
          if (isMatchingLoad(SI->StoreOpc, SOpc) && sameAddress(*MII, *Scan)) {
            if (!HasCall) continue; // load before any call — not a cross-call spill
            Loads.push_back(Scan);
            LoadCount++;
            PushPopBytes += 1; // POP
            BssBytes += SI->LoadBytes;
            // If there are more loads after this one, we need PUSH after POP.
            // We'll account for that below.
          }

          // Branch/jump out of block = stop scanning.
          if (Scan->isTerminator()) break;
        }

        // Unbalanced PUSH/POP between store and load = unsafe.
        if (StackDepth != 0) Conflict = true;

        if (Conflict || LoadCount == 0) continue;

        // Cost: PUSH (1B) + N*POP (N B) + (N-1)*re-PUSH ((N-1) B) = 2N B.
        // Original: store (S B) + N*load (N*L B).
        // Multi-load with re-PUSH: after each POP except the last, insert
        // PUSH to keep the value on the stack for subsequent POPs.
        // The LIFO safety check above prevents interference with other
        // conversions in the same MBB.
        PushPopBytes = 2 * LoadCount; // PUSH + N*POP + (N-1)*re-PUSH
        BssBytes = SI->StoreBytes + LoadCount * SI->LoadBytes;

        if (PushPopBytes >= BssBytes) continue; // not worth it

        // For PUSH AF / POP AF: POP AF restores flags, which may conflict
        // with flags set by the CALL or intervening instructions.
        // Only safe when FLAGS is dead after each POP AF.
        // Check BEFORE replacing the store to avoid leaving a dangling PUSH.
        if (SI->PopOpc == Z80::POP_AF) {
          bool FlagsSafe = true;
          for (auto &LoadIt : Loads) {
            auto After = std::next(LoadIt);
            // After erasing the load, the POP AF will be at this position.
            // Check if flags are dead after the load instruction.
            if (!isRegDeadAfter(After, MBB, TRI, Z80::FLAGS)) {
              FlagsSafe = false;
              break;
            }
          }
          if (!FlagsSafe)
            continue;
        }

        LLVM_DEBUG(dbgs() << "  BSS spill→PUSH/POP: " << *MII
                          << "  " << LoadCount << " loads, saves "
                          << (BssBytes - PushPopBytes) << "B\n");

        // Replace store with PUSH.
        DebugLoc DL = MII->getDebugLoc();
        BuildMI(MBB, *MII, DL, TII->get(SI->PushOpc));
        MII = MBB.erase(MII);

        // Replace each load with POP.  For all but the last, insert a
        // re-PUSH immediately after the POP to keep the value on the stack
        // for subsequent loads.
        for (int i = 0; i < LoadCount; ++i) {
          auto &LoadMI = *Loads[i];
          DebugLoc LoadDL = LoadMI.getDebugLoc();
          BuildMI(MBB, LoadMI, LoadDL, TII->get(SI->PopOpc));
          if (i < LoadCount - 1) {
            // Re-PUSH after POP: saves the value back for the next POP.
            BuildMI(MBB, LoadMI, LoadDL, TII->get(SI->PushOpc));
          }
          MBB.erase(Loads[i]);
        }

        Changed = true;
        // Restart scan from current position (MII was updated by erase).
        --MII;
      }
    }
  }

  return Changed;
}

} // namespace

char Z80LateOptimization::ID = 0;

INITIALIZE_PASS(Z80LateOptimization, DEBUG_TYPE, "Z80 Late Optimizations",
                false, false)

MachineFunctionPass *llvm::createZ80LateOptimizationPass() {
  return new Z80LateOptimization;
}
