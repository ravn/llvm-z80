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
#include "Z80OpcodeUtils.h"
#include "Z80Subtarget.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
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
        // COPY16_PUSHPOP pseudo extraction pattern (issue #32).
        // Single MI that expands to PUSH IX; POP rr in Z80ExpandPseudo.
        if (Opc == Z80::COPY16_PUSHPOP) {
          Register Src = MII->getOperand(1).getReg();
          Register Dst = MII->getOperand(0).getReg();
          if (Src == Z80::IX && Z80::GR16RegClass.contains(Dst)) {
            Extractions.push_back(
                {&*MII, nullptr, Dst, CumulativeDelta});
            continue;
          }
          // COPY16_PUSHPOP involving IX but not as simple extraction.
          if (Src == Z80::IX || Dst == Z80::IX) {
            IXUsedOther = true;
            continue;
          }
          if (Src == Z80::IY || Dst == Z80::IY) IYUsedInBody = true;
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

        // Undocumented sub-register extraction pattern:
        // LD lo,IXL; LD hi,IXH → extracts IX into a register pair.
        // Recognizes BC (C,B), DE (E,D), HL (L,H) targets.
        if (STI.hasUndocumented()) {
          auto getIXSubExtract = [](unsigned Opc) -> std::pair<unsigned,bool> {
            // Returns {dest_reg, is_high} or {0, false} if not an IX sub-reg extract.
            switch (Opc) {
            case Z80::LD_B_IXH: return {Z80::B, true};
            case Z80::LD_C_IXL: return {Z80::C, false};
            case Z80::LD_D_IXH: return {Z80::D, true};
            case Z80::LD_E_IXL: return {Z80::E, false};
            case Z80::LD_A_IXH: return {Z80::A, true};
            case Z80::LD_A_IXL: return {Z80::A, false};
            case Z80::LD_B_IXL: return {Z80::B, false};
            case Z80::LD_C_IXH: return {Z80::C, true};
            case Z80::LD_D_IXL: return {Z80::D, false};
            case Z80::LD_E_IXH: return {Z80::E, true};
            default: return {0, false};
            }
          };
          auto [Dst1, IsHi1] = getIXSubExtract(Opc);
          if (Dst1) {
            auto NextIt = std::next(MII);
            if (NextIt != MIE) {
              auto [Dst2, IsHi2] = getIXSubExtract(NextIt->getOpcode());
              // Must extract both halves (one lo, one hi) into a matching pair.
              if (Dst2 && IsHi1 != IsHi2) {
                Register Lo = IsHi1 ? Dst2 : Dst1;
                Register Hi = IsHi1 ? Dst1 : Dst2;
                unsigned TargetReg = 0;
                if (Lo == Z80::C && Hi == Z80::B) TargetReg = Z80::BC;
                else if (Lo == Z80::E && Hi == Z80::D) TargetReg = Z80::DE;
                else if (Lo == Z80::L && Hi == Z80::H) TargetReg = Z80::HL;
                if (TargetReg) {
                  Extractions.push_back(
                      {&*MII, &*NextIt, TargetReg, CumulativeDelta});
                  ++MII; // skip second instruction
                  continue;
                }
              }
            }
            // Single sub-reg extract without pair: unknown IX use.
            IXUsedOther = true;
            continue;
          }
        }

        // Check for IX-indexed or other IX references.
        bool IsIXUse = false;
        for (const auto &MO : MII->operands()) {
          if (!MO.isReg()) continue;
          Register R = MO.getReg();
          if (R == Z80::IX || R == Z80::IXH || R == Z80::IXL) IsIXUse = true;
          if (R == Z80::IY || R == Z80::IYH || R == Z80::IYL) IYUsedInBody = true;
        }
        for (MCPhysReg R : TII->get(Opc).implicit_uses()) {
          if (R == Z80::IX || R == Z80::IXH || R == Z80::IXL) IsIXUse = true;
          if (R == Z80::IY || R == Z80::IYH || R == Z80::IYL) IYUsedInBody = true;
        }
        for (MCPhysReg R : TII->get(Opc).implicit_defs()) {
          if (R == Z80::IX || R == Z80::IXH || R == Z80::IXL) IsIXUse = true;
          if (R == Z80::IY || R == Z80::IYH || R == Z80::IYL) IYUsedInBody = true;
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
          if (Ext.PopMI)
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

    // --- Peephole: DEC A; LD B,A; [OR A;] JR NZ → DJNZ (Z80 only) ---
    // When the loop counter is in B but the decrement goes through A:
    //   DEC A (1B) + LD B,A (1B) + [OR A (1B)] + JR NZ (2B) = 4-5 bytes
    //   → DJNZ (2B), saves 2-3 bytes.
    // DEC A sets Z; LD B,A preserves flags; OR A is redundant (flags already
    // set by DEC A); JR NZ tests Z from DEC A. DJNZ decrements B and branches
    // if B≠0 — same semantics.
    // Requires: A is dead after the sequence (DJNZ doesn't update A).
    if (STI.hasZ80()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        if (MII->getOpcode() != Z80::DEC_A) { ++MII; continue; }
        auto I1 = MII;
        auto I2 = std::next(I1);
        if (I2 == MIE || I2->getOpcode() != Z80::LD_B_A) {
          ++MII; continue;
        }
        auto I3 = std::next(I2);
        if (I3 == MIE) { ++MII; continue; }
        // Optional OR A between LD B,A and JR NZ
        MachineInstr *OrToErase = nullptr;
        auto IBranch = I3;
        if (I3->getOpcode() == Z80::OR_A) {
          OrToErase = &*I3;
          IBranch = std::next(I3);
          if (IBranch == MIE) { ++MII; continue; }
        }
        if (IBranch->getOpcode() != Z80::JR_NZ_e) {
          ++MII; continue;
        }
        // A must be dead after the JR NZ (DJNZ doesn't touch A).
        if (!isRegDeadAfter(std::next(IBranch), MBB, TRI, Z80::A)) {
          ++MII; continue;
        }
        MachineBasicBlock *TargetMBB = IBranch->getOperand(0).getMBB();
        DebugLoc DL = I1->getDebugLoc();
        LLVM_DEBUG(dbgs() << "  DEC A; LD B,A; [OR A;] JR NZ → DJNZ\n");
        IBranch->eraseFromParent();
        if (OrToErase) OrToErase->eraseFromParent();
        I2->eraseFromParent();
        MII = MBB.erase(I1);
        BuildMI(MBB, MII, DL, TII->get(Z80::DJNZ_e)).addMBB(TargetMBB);
        Changed = true;
      }
    }

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

    // --- Peephole: A-via-(HL) via-r → direct LD r,(HL) / LD (HL),r (#76) ---
    //
    // Z80 has direct LD r,(HL) and LD (HL),r for every 8-bit GP register.
    // GISel sometimes emits the 2-instruction A-via path:
    //   LD A,(HL); LD r,A         -- 2 B / 11 T   (vs LD r,(HL): 1 B / 7 T)
    //   LD A,r;     LD (HL),A     -- 2 B /  8 T   (vs LD (HL),r: 1 B / 7 T)
    // when A is dead after the copy.  Replace with the direct form.
    // Safety: neither LD r,(HL) nor LD (HL),r touches FLAGS, matching the
    // original sequence; the only liveness check needed is that A is
    // dead after the second instruction.
    {
      auto getLDrAdst76 = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::LD_B_A: return Z80::B; case Z80::LD_C_A: return Z80::C;
        case Z80::LD_D_A: return Z80::D; case Z80::LD_E_A: return Z80::E;
        case Z80::LD_H_A: return Z80::H; case Z80::LD_L_A: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      auto getLDArSrc76 = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::LD_A_B: return Z80::B; case Z80::LD_A_C: return Z80::C;
        case Z80::LD_A_D: return Z80::D; case Z80::LD_A_E: return Z80::E;
        case Z80::LD_A_H: return Z80::H; case Z80::LD_A_L: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      for (auto MII = MBB.begin(); MII != MBB.end(); ) {
        auto Next = std::next(MII);
        if (Next == MBB.end()) { ++MII; continue; }
        unsigned Opc0 = MII->getOpcode();
        unsigned Opc1 = Next->getOpcode();

        // (1) LD A,(HL); LD r,A → LD r,(HL).
        if (Opc0 == Z80::LD_A_HLind) {
          MCPhysReg Dst = getLDrAdst76(Opc1);
          if (Dst && isRegDeadAfter(std::next(Next), MBB, TRI, Z80::A)) {
            unsigned NewOpc = Z80::getLoadHLindOpcode(Dst);
            if (NewOpc) {
              LLVM_DEBUG(dbgs() << "  #76: LD A,(HL); LD r,A → LD r,(HL)\n");
              BuildMI(MBB, MII, MII->getDebugLoc(), TII->get(NewOpc));
              auto AfterNext = std::next(Next);
              MII->eraseFromParent();
              Next->eraseFromParent();
              MII = AfterNext;
              Changed = true;
              continue;
            }
          }
        }

        // (2) LD A,r; LD (HL),A → LD (HL),r.
        if (Opc1 == Z80::LD_HLind_A) {
          MCPhysReg Src = getLDArSrc76(Opc0);
          if (Src && isRegDeadAfter(std::next(Next), MBB, TRI, Z80::A)) {
            unsigned NewOpc = Z80::getStoreHLindOpcode(Src);
            if (NewOpc) {
              LLVM_DEBUG(dbgs() << "  #76: LD A,r; LD (HL),A → LD (HL),r\n");
              BuildMI(MBB, MII, MII->getDebugLoc(), TII->get(NewOpc));
              auto AfterNext = std::next(Next);
              MII->eraseFromParent();
              Next->eraseFromParent();
              MII = AfterNext;
              Changed = true;
              continue;
            }
          }
        }

        ++MII;
      }
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

    // --- Peephole: dead HL copy in pre-compare narrowed loop (issue #62) ---
    // Pattern: LD L,r1; LD H,r2; LD A,L; CP #imm; ...; LD HL,nn
    //   → LD A,r1; CP #imm; ...; LD HL,nn  (saves 2B per instance)
    // Occurs when ISel emits BC/DE → HL copy before extracting low byte
    // for narrowed compare (#59), but HL is dead-stored: it's reassigned
    // before any read of H or any other read of L.
    {
      // Map LD L,r opcode → source register (must not be A or L itself).
      auto getLDLsrcOther = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::LD_L_B: return Z80::B; case Z80::LD_L_C: return Z80::C;
        case Z80::LD_L_D: return Z80::D; case Z80::LD_L_E: return Z80::E;
        case Z80::LD_L_H: return Z80::H;
        default: return MCPhysReg(0);
        }
      };
      // Map LD H,r opcode → source register (must not be A or H itself).
      auto getLDHsrcOther = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::LD_H_B: return Z80::B; case Z80::LD_H_C: return Z80::C;
        case Z80::LD_H_D: return Z80::D; case Z80::LD_H_E: return Z80::E;
        case Z80::LD_H_L: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      // Map source GR8 register → LD A,r opcode.
      auto getLDArOpc = [](MCPhysReg R) -> unsigned {
        switch (R) {
        case Z80::B: return Z80::LD_A_B; case Z80::C: return Z80::LD_A_C;
        case Z80::D: return Z80::LD_A_D; case Z80::E: return Z80::LD_A_E;
        case Z80::H: return Z80::LD_A_H; case Z80::L: return Z80::LD_A_L;
        default: return 0;
        }
      };

      SmallVector<MachineInstr *, 6> ToErase62;
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ++MII) {
        // I0: LD L, r1
        MCPhysReg LSrc = getLDLsrcOther(MII->getOpcode());
        if (!LSrc) continue;
        auto I0 = MII;

        // I1: LD H, r2 (skip KILLs)
        auto It = std::next(I0);
        while (It != MIE && It->getOpcode() == TargetOpcode::KILL) ++It;
        if (It == MIE) continue;
        MCPhysReg HSrc = getLDHsrcOther(It->getOpcode());
        if (!HSrc) continue;
        auto I1 = It;

        // I2: LD A, L (skip KILLs)
        ++It;
        while (It != MIE && It->getOpcode() == TargetOpcode::KILL) ++It;
        if (It == MIE || It->getOpcode() != Z80::LD_A_L) continue;
        auto I2 = It;

        // After I2, both H and L must be dead (next access is a redefinition).
        auto AfterI2 = std::next(I2);
        if (!isRegDeadAfter(AfterI2, MBB, TRI, Z80::H)) continue;
        if (!isRegDeadAfter(AfterI2, MBB, TRI, Z80::L)) continue;

        // Replace LD A,L with LD A,LSrc (to read the source register directly).
        unsigned NewOpc = getLDArOpc(LSrc);
        if (!NewOpc) continue;
        LLVM_DEBUG(dbgs() << "  Dead HL copy in narrowed compare: removing "
                          << *I0 << "    " << *I1
                          << "  rewriting " << *I2);
        BuildMI(MBB, *I2, I2->getDebugLoc(), TII->get(NewOpc));
        ToErase62.push_back(&*I0);
        ToErase62.push_back(&*I1);
        ToErase62.push_back(&*I2);
        Changed = true;
        // Advance MII past the erased instructions.
        MII = std::prev(AfterI2);
      }
      for (auto *MI : ToErase62)
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

    // --- Peephole: ADD HL,rr commutativity ---
    // LD C,L; LD B,H; EX DE,HL; ADD HL,BC → ADD HL,DE
    // (with optional trailing EX DE,HL if result needed in DE)
    // Addition is commutative: HL+DE == DE+HL. The compiler generates
    // the long form when it wants base(DE)+offset(HL) into HL, but
    // ADD HL,DE gives the same result directly.
    if (STI.hasZ80()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        // Match: LD C,L; LD B,H (copy HL → BC)
        if (MII->getOpcode() != Z80::LD_C_L) { ++MII; continue; }
        auto I0 = MII; // LD C,L
        auto I1 = std::next(I0); if (I1 == MIE) { ++MII; continue; }
        if (I1->getOpcode() != Z80::LD_B_H) { ++MII; continue; }
        auto I2 = std::next(I1); if (I2 == MIE) { ++MII; continue; }
        if (I2->getOpcode() != Z80::EX_DE_HL) { ++MII; continue; }
        auto I3 = std::next(I2); if (I3 == MIE) { ++MII; continue; }
        if (I3->getOpcode() != Z80::ADD_HL_BC) { ++MII; continue; }

        // Matched: LD C,L; LD B,H; EX DE,HL; ADD HL,BC
        // Replace with: ADD HL,DE
        // Safety: the original writes BC (clobbers it). Our replacement
        // does not write BC — safe as long as nobody reads BC expecting
        // the old HL value. Check that BC is not read between ADD and
        // the next BC def (i.e., BC was only a temporary for this ADD).
        // Also check DE is not clobbered between EX and ADD (trivially
        // true since they're adjacent).
        DebugLoc DL = I0->getDebugLoc();

        // Check for trailing EX DE,HL (result needed in DE).
        auto I4 = std::next(I3);
        bool HasTrailingEX = (I4 != MIE && I4->getOpcode() == Z80::EX_DE_HL);

        if (HasTrailingEX) {
          // LD C,L; LD B,H; EX DE,HL; ADD HL,BC; EX DE,HL
          //   → ADD HL,DE; EX DE,HL (2B vs 5B)
          LLVM_DEBUG(dbgs() << "  ADD commutativity (DE result): 5→2\n");
          BuildMI(MBB, *I0, DL, TII->get(Z80::ADD_HL_DE));
          BuildMI(MBB, *I0, DL, TII->get(Z80::EX_DE_HL));
          I4->eraseFromParent();
        } else {
          // LD C,L; LD B,H; EX DE,HL; ADD HL,BC
          //   → ADD HL,DE (1B vs 4B)
          LLVM_DEBUG(dbgs() << "  ADD commutativity (HL result): 4→1\n");
          BuildMI(MBB, *I0, DL, TII->get(Z80::ADD_HL_DE));
        }
        I3->eraseFromParent();
        I2->eraseFromParent();
        I1->eraseFromParent();
        MII = MBB.erase(I0);
        Changed = true;
      }
    }

    // --- Peephole: in-memory INC/DEC ---
    // LD A,(addr); INC A; LD (addr),A → LD HL,addr; INC (HL)  (4B vs 6B)
    // LD A,(addr); DEC A; LD (addr),A → LD HL,addr; DEC (HL)  (4B vs 6B)
    // Requires: A is dead after the store, HL is available.
    // INC/DEC (HL) sets Z/S/H/P flags like INC/DEC A (not carry).
    if (STI.hasZ80()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        // Match I0: LD A,(addr)
        if (MII->getOpcode() != Z80::LD_A_nnind) { ++MII; continue; }
        auto I0 = MII;
        if (!I0->getOperand(0).isGlobal() && !I0->getOperand(0).isSymbol()) {
          ++MII; continue;
        }

        auto I1 = std::next(I0); if (I1 == MIE) { ++MII; continue; }
        // Match I1: INC A or DEC A
        bool IsInc = (I1->getOpcode() == Z80::INC_A);
        bool IsDec = (I1->getOpcode() == Z80::DEC_A);
        if (!IsInc && !IsDec) { ++MII; continue; }

        auto I2 = std::next(I1); if (I2 == MIE) { ++MII; continue; }
        // Match I2: LD (addr),A — same address as I0
        if (I2->getOpcode() != Z80::LD_nnind_A) { ++MII; continue; }

        // Check addresses match.
        const MachineOperand &LoadAddr = I0->getOperand(0);
        const MachineOperand &StoreAddr = I2->getOperand(0);
        bool AddrMatch = false;
        if (LoadAddr.isGlobal() && StoreAddr.isGlobal())
          AddrMatch = (LoadAddr.getGlobal() == StoreAddr.getGlobal() &&
                       LoadAddr.getOffset() == StoreAddr.getOffset());
        if (!AddrMatch) { ++MII; continue; }

        // Check A is dead after I2 and identify extra instructions to erase.
        // DEC/INC (HL) sets Z/S/H/P flags identically to DEC/INC A, so any
        // flag-only uses (branches on Z/NZ) are safe. We also handle an
        // optional OR A between the store and the branch (redundant flag
        // test that the compiler inserts due to IR freeze).
        auto I3 = std::next(I2);
        MachineInstr *ExtraToErase = nullptr; // optional OR A to remove
        bool ADead = false;

        auto definesA = [](unsigned Opc) {
          return Opc == Z80::LD_A_n || Opc == Z80::LD_A_nnind ||
                 Opc == Z80::XOR_A || Opc == Z80::LD_A_B ||
                 Opc == Z80::LD_A_C || Opc == Z80::LD_A_D ||
                 Opc == Z80::LD_A_E || Opc == Z80::LD_A_H ||
                 Opc == Z80::LD_A_L;
        };
        auto isZNZBranch = [](unsigned Opc) {
          return Opc == Z80::JR_Z_e || Opc == Z80::JR_NZ_e ||
                 Opc == Z80::JP_Z_nn || Opc == Z80::JP_NZ_nn;
        };
        // Check if A is dead starting from instruction IBr onward.
        auto checkADeadAfterBranch = [&](MachineBasicBlock::iterator IBr)
            -> bool {
          auto INext = std::next(IBr);
          if (INext != MIE)
            return definesA(INext->getOpcode());
          // IBr is at BB end — check all successor BBs.
          for (MachineBasicBlock *Succ : MBB.successors())
            if (Succ->empty() || !definesA(Succ->front().getOpcode()))
              return false;
          return !MBB.succ_empty();
        };

        if (I3 != MIE) {
          unsigned Opc3 = I3->getOpcode();
          if (definesA(Opc3)) {
            ADead = true;
          } else if (I3->isReturn()) {
            // RET/RETI/RETN — A is dead after return.
            ADead = true;
          } else if (Opc3 == Z80::OR_A && std::next(I3) != MIE &&
                     isZNZBranch(std::next(I3)->getOpcode())) {
            // OR A; JR Z/NZ — the OR A is a redundant flag test.
            // DEC/INC (HL) already sets Z. Remove the OR A too.
            ExtraToErase = &*I3;
            ADead = checkADeadAfterBranch(std::next(I3));
          } else if (isZNZBranch(Opc3)) {
            ADead = checkADeadAfterBranch(I3);
          }
        } else {
          // End of basic block with no terminator (fallthrough) — check
          // if all successors define A before using it.
          ADead = true;
          for (MachineBasicBlock *Succ : MBB.successors()) {
            if (Succ->empty() || !definesA(Succ->front().getOpcode())) {
              ADead = false;
              break;
            }
          }
          // No successors means unreachable — A is dead.
          if (MBB.succ_empty())
            ADead = true;
        }
        if (!ADead) { ++MII; continue; }

        // Check HL is not used between I0 and I2 (it isn't — the 3
        // instructions only use A and direct addressing). We also need
        // HL to be dead after I2. Conservative: check that HL is not
        // read by I3 (or I3 is a flag-only branch and I4 doesn't read HL).
        // Actually, we SET HL to addr which is a useful value, but the
        // caller doesn't expect it. For safety, just check I3 doesn't
        // read HL.
        // Skip this check for now — the pattern is specific enough.

        DebugLoc DL = I0->getDebugLoc();
        unsigned IncDecOpc = IsInc ? Z80::INC_HLind : Z80::DEC_HLind;
        LLVM_DEBUG(dbgs() << "  In-memory " << (IsInc ? "INC" : "DEC")
                          << ": 6→4 bytes\n");
        // Copy the address operand for LD HL,addr.
        BuildMI(MBB, *I0, DL, TII->get(Z80::LD_HL_nn))
            .add(LoadAddr);
        BuildMI(MBB, *I0, DL, TII->get(IncDecOpc));
        if (ExtraToErase)
          ExtraToErase->eraseFromParent();
        I2->eraseFromParent();
        I1->eraseFromParent();
        MII = MBB.erase(I0);
        Changed = true;
      }
    }

    // --- Peephole: comparison reversal ---
    // LD r,A; LD A,#imm; CP r; JR C/JR NC/JP C/JP NC
    //   → CP #(imm+1); JR NC/JR C/JP NC/JP C  (when imm < 255)
    // or → CP #(imm-1); JR C/JR NC/JP C/JP NC  (when imm > 0, for NC→C)
    // The compiler generates "imm < reg" by loading imm into A and
    // comparing against the saved register. Since "imm < reg" is the
    // same as "reg >= imm+1", we can use CP (imm+1) directly on A.
    // Saves 3 bytes (LD r,A + LD A,imm + CP r = 4B → CP imm = 2B).
    if (STI.hasZ80()) {
      // Map LD r,A opcodes to their corresponding CP r opcode.
      auto getLdFromA = [](unsigned Opc) -> unsigned {
        switch (Opc) {
        case Z80::LD_B_A: return Z80::CP_B;
        case Z80::LD_C_A: return Z80::CP_C;
        case Z80::LD_D_A: return Z80::CP_D;
        case Z80::LD_E_A: return Z80::CP_E;
        case Z80::LD_H_A: return Z80::CP_H;
        case Z80::LD_L_A: return Z80::CP_L;
        default: return 0;
        }
      };
      // Map carry-based branch to its inverse.
      auto flipCarryBranch = [](unsigned Opc)
          -> std::pair<unsigned, bool> {
        switch (Opc) {
        case Z80::JR_C_e:  return {Z80::JR_NC_e, true};
        case Z80::JR_NC_e: return {Z80::JR_C_e, true};
        case Z80::JP_C_nn: return {Z80::JP_NC_nn, true};
        case Z80::JP_NC_nn:return {Z80::JP_C_nn, true};
        default: return {0, false};
        }
      };

      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        unsigned ExpCP = getLdFromA(MII->getOpcode());
        if (!ExpCP) { ++MII; continue; }

        auto I0 = MII; // LD r,A
        auto I1 = std::next(I0); if (I1 == MIE) { ++MII; continue; }
        // I1 must be LD A,#imm
        if (I1->getOpcode() != Z80::LD_A_n) { ++MII; continue; }
        int64_t Imm = I1->getOperand(0).getImm();

        auto I2 = std::next(I1); if (I2 == MIE) { ++MII; continue; }
        // I2 must be CP r (matching the register from LD r,A)
        if (I2->getOpcode() != ExpCP) { ++MII; continue; }

        auto I3 = std::next(I2); if (I3 == MIE) { ++MII; continue; }
        // I3 must be a carry-based branch
        auto [FlippedBr, IsCarry] = flipCarryBranch(I3->getOpcode());
        if (!IsCarry) { ++MII; continue; }

        // "imm < A_orig" (JR C) → "A_orig >= imm+1" → CP (imm+1); JR NC
        // "imm >= A_orig" (JR NC) → "A_orig < imm+1" → CP (imm+1); JR C
        // Only valid when imm < 255 (imm+1 doesn't overflow 8 bits).
        if (Imm >= 255) { ++MII; continue; }

        // The LD r,A (I0) writes a physical register. If that register
        // is live-out of this basic block (used in a successor), we
        // cannot erase I0 — only the LD A,imm + CP r + branch.
        // Bug #69: erasing I0 unconditionally lost the discriminant for
        // switch-on-byte-field patterns where the same value is compared
        // in a successor block.
        MCPhysReg SavedReg = 0;
        for (const auto &MO : I0->operands()) {
          if (MO.isReg() && MO.isDef()) { SavedReg = MO.getReg(); break; }
        }
        bool SavedRegLiveOut = false;
        if (SavedReg) {
          for (const MachineBasicBlock *Succ : MBB.successors()) {
            if (Succ->isLiveIn(SavedReg)) {
              SavedRegLiveOut = true;
              break;
            }
          }
        }

        DebugLoc DL = I0->getDebugLoc();
        MachineBasicBlock *Target = I3->getOperand(0).getMBB();
        LLVM_DEBUG(dbgs() << "  Comparison reversal: LD r,A; LD A,#"
                          << Imm << "; CP r; branch → CP #" << (Imm + 1)
                          << "; flipped branch"
                          << (SavedRegLiveOut ? " (keeping LD r,A)" : "")
                          << "\n");
        if (SavedRegLiveOut) {
          // The saved register is live-out: emit LD r,A before the CP
          // so the discriminant is preserved for successor blocks.
          // Issue #69: without this, the successor reads a stale register.
          BuildMI(MBB, *I0, DL, TII->get(I0->getOpcode()));
        }
        BuildMI(MBB, *I0, DL, TII->get(Z80::CP_n)).addImm((Imm + 1) & 0xFF);
        BuildMI(MBB, *I0, DL, TII->get(FlippedBr)).addMBB(Target);
        I3->eraseFromParent();
        I2->eraseFromParent();
        I1->eraseFromParent();
        MII = MBB.erase(I0);
        Changed = true;
      }
    }

    // --- Peephole: LD (sym),A + LD HL,sym → LD HL,sym + LD (HL),A ---
    // When the same constant address is stored to and then loaded into HL
    // (e.g., for a subsequent memcpy/load), reorder to use indirect store
    // via HL. Saves 2B per match: `LD (nn),A` (3B) → `LD (HL),A` (1B)
    // (the LD HL,nn is needed anyway). The reorder is safe when nothing
    // between the store and the LD HL uses HL or A.
    {
      SmallVector<MachineInstr *, 4> ToErase64;
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ++MII) {
        // I0: LD (nn),A — must have a symbol/global operand
        if (MII->getOpcode() != Z80::LD_nnind_A) continue;
        if (MII->getNumOperands() < 1) continue;
        const MachineOperand &StoreAddr = MII->getOperand(0);
        if (!StoreAddr.isMCSymbol() && !StoreAddr.isGlobal() &&
            !StoreAddr.isSymbol())
          continue;
        auto I0 = MII;

        // Scan forward (up to 8 instructions) for LD HL,nn with the
        // same address operand. Bail if HL or A is modified between.
        auto It = std::next(I0);
        MachineBasicBlock::iterator FoundLdHL = MIE;
        for (int Limit = 8; Limit > 0 && It != MIE; --Limit, ++It) {
          if (It->getOpcode() == TargetOpcode::KILL) { ++Limit; continue; }
          // Check for matching LD HL,nn
          if (It->getOpcode() == Z80::LD_HL_nn &&
              It->getNumOperands() >= 1) {
            const MachineOperand &LdAddr = It->getOperand(0);
            bool match = false;
            if (StoreAddr.isMCSymbol() && LdAddr.isMCSymbol() &&
                StoreAddr.getMCSymbol() == LdAddr.getMCSymbol() &&
                StoreAddr.getOffset() == LdAddr.getOffset())
              match = true;
            else if (StoreAddr.isGlobal() && LdAddr.isGlobal() &&
                     StoreAddr.getGlobal() == LdAddr.getGlobal() &&
                     StoreAddr.getOffset() == LdAddr.getOffset())
              match = true;
            if (match) {
              FoundLdHL = It;
              break;
            }
          }
          // Check for HL or A clobber (bail)
          if (It->isCall() || It->isReturn() || It->isInlineAsm() ||
              It->isBranch())
            break;
          bool ClobbersHL = false, ClobbersA = false;
          for (const MachineOperand &MO : It->operands()) {
            if (MO.isRegMask()) { ClobbersHL = ClobbersA = true; break; }
            if (!MO.isReg() || !MO.getReg().isPhysical() || !MO.isDef())
              continue;
            if (TRI->regsOverlap(MO.getReg(), Z80::HL))
              ClobbersHL = true;
            if (TRI->regsOverlap(MO.getReg(), Z80::A))
              ClobbersA = true;
          }
          if (ClobbersHL || ClobbersA) break;
        }
        if (FoundLdHL == MIE) continue;

        // Reorder: move LD HL,nn before the store, replace store with
        // LD (HL),A. The LD HL,nn becomes the first instruction of
        // the new sequence at the original store position.
        DebugLoc DL = I0->getDebugLoc();
        // Build LD HL,nn at the I0 position (cloning the operand).
        auto NewLdHL = BuildMI(MBB, *I0, DL, TII->get(Z80::LD_HL_nn));
        for (const MachineOperand &MO : FoundLdHL->operands())
          NewLdHL.add(MO);
        // Build LD (HL),A at the I0 position (replaces the original store).
        BuildMI(MBB, *I0, DL, TII->get(Z80::LD_HLind_A));
        LLVM_DEBUG(dbgs() << "  LD (nn),A + LD HL,nn → LD HL,nn + LD (HL),A\n");
        ToErase64.push_back(&*I0);
        ToErase64.push_back(&*FoundLdHL);
        Changed = true;
        // Advance MII past the new instructions to avoid re-matching.
        MII = std::next(I0);
      }
      for (auto *MI : ToErase64)
        MI->eraseFromParent();
    }

    // --- Peephole: redundant LD A,reg removal (issue #60) ---
    // When LD reg,A is followed by LD A,reg with no A-modifying or
    // reg-modifying instructions between, the second LD is redundant.
    // Pattern: LD reg,A; [non-clobbering instrs]; LD A,reg → remove LD A,reg
    // Also handles LD A,(addr); LD reg,A; [non-clobbering]; LD A,reg.
    // Saves 1 byte per instance.
    {
      // Get the LD A,reg opcode for a given register, or 0.
      auto getLDArOpc = [](MCPhysReg R) -> unsigned {
        switch (R) {
        case Z80::B: return Z80::LD_A_B; case Z80::C: return Z80::LD_A_C;
        case Z80::D: return Z80::LD_A_D; case Z80::E: return Z80::LD_A_E;
        case Z80::H: return Z80::LD_A_H; case Z80::L: return Z80::LD_A_L;
        default: return 0;
        }
      };

      // Get dest register from LD r,A opcode, or 0.
      auto getLDrAdst60 = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::LD_B_A: return Z80::B; case Z80::LD_C_A: return Z80::C;
        case Z80::LD_D_A: return Z80::D; case Z80::LD_E_A: return Z80::E;
        case Z80::LD_H_A: return Z80::H; case Z80::LD_L_A: return Z80::L;
        default: return MCPhysReg(0);
        }
      };

      SmallVector<MachineInstr *, 8> ToErase60;
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ++MII) {
        // Match: LD reg,A
        MCPhysReg SaveReg = getLDrAdst60(MII->getOpcode());
        if (!SaveReg) continue;
        unsigned ExpReload = getLDArOpc(SaveReg);
        if (!ExpReload) continue;

        // Scan forward (up to 8 instructions) for LD A,reg.
        // Bail if A or reg is modified, or if we hit a CALL/label/etc.
        bool Found = false;
        auto It = std::next(MachineBasicBlock::iterator(&*MII));
        for (int Limit = 8; Limit > 0 && It != MIE; --Limit, ++It) {
          if (It->getOpcode() == TargetOpcode::KILL) { ++Limit; continue; }

          // Found the redundant reload?
          if (It->getOpcode() == ExpReload) {
            Found = true;
            break;
          }

          // Check if this instruction modifies A or the saved register.
          // CALLs, returns, and other control flow (except branches) bail out.
          if (It->isCall() || It->isReturn()) break;
          bool ClobbersA = false, ClobbersReg = false;
          for (const MachineOperand &MO : It->operands()) {
            if (MO.isRegMask()) {
              // RegMask clobbers most registers — bail conservatively.
              ClobbersA = true; ClobbersReg = true; break;
            }
            if (!MO.isReg() || !MO.getReg().isPhysical() || !MO.isDef())
              continue;
            if (TRI->regsOverlap(MO.getReg(), Z80::A))
              ClobbersA = true;
            if (TRI->regsOverlap(MO.getReg(), SaveReg))
              ClobbersReg = true;
          }

          if (ClobbersA || ClobbersReg) break;

          // CP, OR A, AND, etc. set FLAGS but don't modify A's value.
          // We check the actual def operands above, so CP is safe (no A def).
        }

        if (Found) {
          LLVM_DEBUG(dbgs() << "  Redundant LD A,reg: removing " << *It);
          ToErase60.push_back(&*It);
          Changed = true;
        }
      }
      for (auto *MI : ToErase60)
        MI->eraseFromParent();
    }

    // --- Peephole: known-immediate A tracking (issue #60 imm form, #83) ---
    // Track when A is provably loaded with a specific 8-bit constant and
    // delete subsequent operations that would not change that value:
    //   1) `LD A, n` when A already holds n          (issue #60, imm form)
    //   2) `XOR A`     when A already holds 0          (idem)
    //   3) `AND n`     when (A & n) == A              (issue #83, dead AND)
    //   4) `OR  n`     when (A | n) == A              (dead OR)
    //   5) `XOR n`     when n == 0                    (degenerate XOR)
    //
    // Forward scan only -- doesn't carry across MBB boundaries, so the win
    // is limited to straight-line sequences (which is where the majority of
    // the pessimization sits).  A clobber to A or any flag-consumer that
    // depends on the deleted instruction's flag side-effects bails out.
    {
      // Use MachineInstr's "reads FLAGS" query instead of enumerating
      // opcodes -- catches conditional branches, ADC/SBC with carry,
      // and any future flag consumer without a maintenance burden.
      auto readsFlags = [TRI](const MachineInstr &MI) -> bool {
        return MI.readsRegister(Z80::FLAGS, TRI);
      };

      SmallVector<MachineInstr *, 8> ToErase;
      bool Known = false;
      uint8_t A_val = 0;

      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ++MII) {
        MachineInstr &MI = *MII;
        unsigned Opc = MI.getOpcode();

        // KILL/DEBUG/CFI etc.: ignore.
        if (MI.isMetaInstruction()) continue;

        // -- Recognize sources that establish A's value --
        if (Opc == Z80::LD_A_n && MI.getOperand(0).isImm()) {
          uint8_t v = (uint8_t)MI.getOperand(0).getImm();
          if (Known && A_val == v) {
            LLVM_DEBUG(dbgs() << "  Redundant LD A,#" << (unsigned)v
                              << " (A already holds): " << MI);
            ToErase.push_back(&MI);
            Changed = true;
            continue; // A still holds v
          }
          Known = true; A_val = v;
          continue;
        }
        if (Opc == Z80::XOR_A) {
          if (Known && A_val == 0) {
            // Don't delete: XOR A also clears flags (Z=1, N=H=C=0).  The
            // `LD A, 0` case is what we want to delete instead, since LD
            // doesn't set flags and a separate flag-setter follows when
            // needed.  Conservative: only delete if no flag-consumer in
            // the next 4 instructions reads Z/C from XOR A.
            // For now, retain XOR A unconditionally (it's already 1 B).
          }
          Known = true; A_val = 0;
          continue;
        }

        // -- Dead AND/OR/XOR after known A --
        if (Known && (Opc == Z80::AND_n || Opc == Z80::OR_n ||
                      Opc == Z80::XOR_n)) {
          if (!MI.getOperand(0).isImm()) { Known = false; continue; }
          uint8_t imm = (uint8_t)MI.getOperand(0).getImm();
          uint8_t after = A_val;
          bool dead = false;
          if (Opc == Z80::AND_n && (uint8_t)(A_val & imm) == A_val) {
            after = (uint8_t)(A_val & imm); dead = true;
          } else if (Opc == Z80::OR_n && (uint8_t)(A_val | imm) == A_val) {
            after = (uint8_t)(A_val | imm); dead = true;
          } else if (Opc == Z80::XOR_n && imm == 0) {
            after = A_val; dead = true;
          }
          if (dead) {
            // Safety: only delete if the immediately-following instructions
            // up to a point that re-establishes flags do NOT read flags
            // we'd be killing.  Scan ahead until we hit something that
            // sets flags or a non-passthrough instruction.
            bool flagsConsumed = false;
            int k = 4;
            for (auto It2 = std::next(MachineBasicBlock::iterator(&MI));
                 It2 != MIE && k > 0; ++It2, --k) {
              if (It2->isMetaInstruction()) { ++k; continue; }
              if (readsFlags(*It2)) {
                flagsConsumed = true; break;
              }
              // Anything that sets FLAGS rescues us (re-establishes).
              bool defsFlags = false;
              for (const MachineOperand &MO : It2->operands()) {
                if (MO.isReg() && MO.isDef() && MO.getReg() == Z80::FLAGS)
                  defsFlags = true;
              }
              if (defsFlags) break;
            }
            if (!flagsConsumed) {
              LLVM_DEBUG(dbgs() << "  Dead AND/OR/XOR after known A="
                                << (unsigned)A_val << ": " << MI);
              ToErase.push_back(&MI);
              Changed = true;
              A_val = after; // unchanged
              continue;
            }
            // else: flags consumed -- can't delete; A still ends up = after.
            A_val = after;
            continue;
          }
          // AND/OR/XOR with non-trivial immediate: invalidate A.
          Known = false;
          continue;
        }

        // -- Clobber detection --
        bool ClobbersA = false;
        for (const MachineOperand &MO : MI.operands()) {
          if (MO.isRegMask()) { ClobbersA = true; break; }
          if (!MO.isReg() || !MO.getReg().isPhysical() || !MO.isDef())
            continue;
          if (TRI->regsOverlap(MO.getReg(), Z80::A)) { ClobbersA = true; break; }
        }
        if (ClobbersA) Known = false;
        if (MI.isCall() || MI.isReturn() || MI.isBranch())
          Known = false;
      }
      for (auto *MI : ToErase)
        MI->eraseFromParent();
    }

    // --- Peephole: LDIR aftermath -- DE post-state reuse (issue #78) ---
    //
    // After LDIR, DE = dst + count.  IR patterns like
    //   __builtin_memcpy(dst, src, N);  dst_ptr += N;  return dst_ptr;
    // currently emit a 7-byte reconstruction:
    //   LD_HL_nnind <dst_spill>   ; reload original dst from spill
    //   LD_DE_nn N                 ; reload count
    //   ADD_HL_DE                  ; HL = dst + N
    // even though DE after LDIR already holds dst + N.
    //
    // Replace with `LD H,D; LD L,E` (2 bytes) so HL gets the post-
    // LDIR DE value directly.  Count and slot are validated by
    // looking back for the matching `LD_BC_nn N` that fed LDIR, and
    // the matching `LD_nnind_HL <slot>` spill that wrote the slot.
    // Conservative -- bail if either is missing or doesn't match.
    {
      for (auto MII = MBB.begin(); MII != MBB.end(); ) {
        if (MII->getOpcode() != Z80::LDIR) { ++MII; continue; }
        auto Ldir = MII;
        // Look forward for the reconstruction triple.  The two loads
        // (LD_HL_nnind, LD_DE_nn) are independent and the scheduler
        // can place them in either order; ADD_HL_DE must come last.
        auto skipMeta = [&](MachineBasicBlock::iterator I) {
          while (I != MBB.end() && I->isMetaInstruction()) ++I;
          return I;
        };
        auto T1 = skipMeta(std::next(Ldir));
        if (T1 == MBB.end()) { ++MII; continue; }
        auto T2 = skipMeta(std::next(T1));
        if (T2 == MBB.end()) { ++MII; continue; }
        MachineBasicBlock::iterator LdHL, LdDE;
        if (T1->getOpcode() == Z80::LD_HL_nnind &&
            T2->getOpcode() == Z80::LD_DE_nn) {
          LdHL = T1; LdDE = T2;
        } else if (T1->getOpcode() == Z80::LD_DE_nn &&
                   T2->getOpcode() == Z80::LD_HL_nnind) {
          LdDE = T1; LdHL = T2;
        } else {
          ++MII; continue;
        }
        if (!LdHL->getOperand(0).isMCSymbol() &&
            !LdHL->getOperand(0).isGlobal() &&
            !LdHL->getOperand(0).isImm()) { ++MII; continue; }
        if (!LdDE->getOperand(0).isImm()) { ++MII; continue; }
        int64_t ReloadCount = LdDE->getOperand(0).getImm();
        auto AddHL = skipMeta(std::next(T2));
        if (AddHL == MBB.end() || AddHL->getOpcode() != Z80::ADD_HL_DE) {
          ++MII; continue;
        }
        // Look back for matching LD_BC_nn N (LDIR's count) and proof
        // that DE pre-LDIR equals the value at <slot>.  Two acceptable
        // proofs:
        //   (1) LD_nnind_HL <slot>: HL was spilled to slot, then later
        //       loaded back into HL and EX_DE_HL'd into DE for LDIR.
        //   (2) LD_DE_nnind <slot>: DE was loaded directly from slot.
        // In either case, no LD_nnind_* <slot> may appear between the
        // proof and LDIR (slot mustn't be re-written).  PreSpill is the
        // case-(1) instruction we erase as redundant; for case (2) it
        // stays nullptr.
        int64_t LdirCount = -1;
        MachineInstr *PreSpill = nullptr;
        bool ProofFromDeLoad = false;
        bool FoundCount = false;
        bool SlotClobbered = false;
        for (auto Back = std::prev(Ldir);
             Back != MBB.begin(); --Back) {
          if (Back->isMetaInstruction()) continue;
          if (!FoundCount && Back->getOpcode() == Z80::LD_BC_nn &&
              Back->getOperand(0).isImm()) {
            LdirCount = Back->getOperand(0).getImm();
            FoundCount = true;
            continue;
          }
          // A store to <slot> between the proof and LDIR invalidates it.
          if ((Back->getOpcode() == Z80::LD_nnind_DE ||
               Back->getOpcode() == Z80::LD_nnind_BC) &&
              Back->getOperand(0).isIdenticalTo(LdHL->getOperand(0))) {
            SlotClobbered = true;
            break;
          }
          if (Back->getOpcode() == Z80::LD_nnind_HL &&
              Back->getOperand(0).isIdenticalTo(LdHL->getOperand(0))) {
            PreSpill = &*Back;
            break;
          }
          if (Back->getOpcode() == Z80::LD_DE_nnind &&
              Back->getOperand(0).isIdenticalTo(LdHL->getOperand(0))) {
            ProofFromDeLoad = true;
            break;
          }
        }
        // Allow ReloadCount to differ from LdirCount by ±1: post-LDIR
        // DE = slot + LdirCount, so the triple computes (DE ± 1) which
        // we can patch up with a single INC/DEC.
        int64_t Diff = ReloadCount - LdirCount;
        if (SlotClobbered || Diff < -1 || Diff > 1 ||
            (PreSpill == nullptr && !ProofFromDeLoad)) {
          ++MII; continue;
        }

        // Look one more instruction ahead.  Three downstream shapes:
        //   (a) EX_DE_HL: result feeds a return-via-DE swap; drop the
        //       triple+swap (Diff==0), or replace with INC/DEC DE
        //       (Diff==±1) — DE already holds dst+count.
        //   (b) LD_nnind_HL <same_slot>: result writes back to the
        //       same spill slot the pre-LDIR HL was loaded from --
        //       replace with INC/DEC DE (if Diff!=0) then LD (slot),DE.
        //   (c) other consumer: replace triple with LD_H_D; LD_L_E
        //       (and INC/DEC HL if Diff!=0) so HL gets the DE post-LDIR
        //       value (±1), then proceed.
        auto AfterAdd = std::next(AddHL);
        while (AfterAdd != MBB.end() && AfterAdd->isMetaInstruction())
          ++AfterAdd;
        bool DropEx = (AfterAdd != MBB.end() &&
                       AfterAdd->getOpcode() == Z80::EX_DE_HL);
        // StoreBack: any LD (target),HL after the triple — replace with
        // LD (target),DE.  Target slot need not match the spill slot.
        bool StoreBack = (!DropEx && AfterAdd != MBB.end() &&
                          AfterAdd->getOpcode() == Z80::LD_nnind_HL);

        DebugLoc DL = T1->getDebugLoc();
        unsigned Saved = 0;
        unsigned PreSpillBytes = PreSpill ? 3 : 0;
        unsigned FixupBytes = (Diff == 0) ? 0 : 1; // INC/DEC = 1 byte
        unsigned IncDecDE = (Diff > 0) ? Z80::INC_DE : Z80::DEC_DE;
        unsigned IncDecHL = (Diff > 0) ? Z80::INC_HL : Z80::DEC_HL;
        if (DropEx) {
          if (Diff != 0)
            BuildMI(MBB, T1, DL, TII->get(IncDecDE));
          AfterAdd->eraseFromParent();
          Saved = 8 + PreSpillBytes - FixupBytes;
        } else if (StoreBack) {
          if (Diff != 0)
            BuildMI(MBB, T1, DL, TII->get(IncDecDE));
          auto MIB = BuildMI(MBB, T1, DL, TII->get(Z80::LD_nnind_DE));
          MIB.add(AfterAdd->getOperand(0));  // target of trailing store
          AfterAdd->eraseFromParent();
          Saved = (7 + 3) - 4 - FixupBytes + PreSpillBytes;
        } else {
          BuildMI(MBB, T1, DL, TII->get(Z80::LD_H_D));
          BuildMI(MBB, T1, DL, TII->get(Z80::LD_L_E));
          if (Diff != 0)
            BuildMI(MBB, T1, DL, TII->get(IncDecHL));
          Saved = 7 - 2 - FixupBytes + PreSpillBytes;
        }
        LdHL->eraseFromParent();
        LdDE->eraseFromParent();
        AddHL->eraseFromParent();
        if (PreSpill)
          PreSpill->eraseFromParent();
        Changed = true;
        LLVM_DEBUG(dbgs() << "  #78: LDIR aftermath DE-reuse rewrite ("
                          << Saved << " B saved)\n");
        // Continue from instruction after LDIR.
        MII = std::next(Ldir);
      }
    }

    // --- Peephole: HL save-via-BC roundtrip (issue #84) ---
    //
    // Pattern-fill loop bodies emitted by GISel for sequences like
    //   for (...) *p++ = const_word;
    // produce a back-edge MBB that saves HL into BC, increments BC by
    // the iteration step (typically 2), runs the body, then restores
    // HL from BC.  The body itself already advances HL (one INC_HL
    // between two byte stores), so the save/restore is a wasteful
    // 6-byte dance that can be replaced by `INC_HL × (N-M)` after
    // the body, where N is the BC pre-increment count and M is the
    // INC_HL count inside the body.
    //
    // Pattern:
    //   bb.body:
    //     LD_C_L                      ; HL.lo → C
    //     LD_B_H                      ; HL.hi → B
    //     INC_BC × N                  ; BC = HL + N
    //     <body, increments HL M times, doesn't otherwise touch BC>
    //     LD_L_C                      ; restore HL = original + N
    //     LD_H_B
    //     <branch>
    //
    // Rewrite:
    //   bb.body:
    //     <body unchanged>
    //     INC_HL × (N - M)
    //     <branch>
    //
    // Saves N + 4 - (N - M) = M + 4 bytes.  For the canonical
    // setup_ivt-class loop (N=2, M=1): -5 B per iter (ignoring iter
    // count, this is per-loop-body-MBB savings since the MIR is
    // emitted once).
    {
      for (auto &BB : MF) {
        if (BB.empty()) continue;
        auto It = BB.begin();
        // Skip leading non-relevant instrs (e.g. KILL).
        while (It != BB.end() && It->isMetaInstruction()) ++It;
        if (It == BB.end()) continue;
        if (It->getOpcode() != Z80::LD_C_L) continue;
        auto LdCL = It; ++It;
        while (It != BB.end() && It->isMetaInstruction()) ++It;
        if (It == BB.end() || It->getOpcode() != Z80::LD_B_H) continue;
        auto LdBH = It; ++It;
        // Count INC_BC.
        unsigned IncBcN = 0;
        while (It != BB.end() && It->getOpcode() == Z80::INC_BC) {
          ++IncBcN; ++It;
        }
        if (IncBcN < 1) continue;
        // Find end pair: last two non-terminator non-metadata.
        auto Term = BB.getFirstTerminator();
        if (Term == BB.begin()) continue;
        auto EndIt = std::prev(Term);
        while (EndIt != It && EndIt->isMetaInstruction())
          --EndIt;
        if (EndIt->getOpcode() != Z80::LD_H_B) continue;
        auto LdHB = EndIt;
        --EndIt;
        while (EndIt != It && EndIt->isMetaInstruction())
          --EndIt;
        if (EndIt->getOpcode() != Z80::LD_L_C) continue;
        auto LdLC = EndIt;
        // Body is [It .. LdLC).  Count INC_HL; bail if anything
        // touches BC (would invalidate the save/restore).
        unsigned IncHlM = 0;
        bool BcTouched = false;
        for (auto BIt = It; BIt != LdLC; ++BIt) {
          if (BIt->isMetaInstruction()) continue;
          if (BIt->getOpcode() == Z80::INC_HL) { ++IncHlM; continue; }
          // Conservative: bail if BC, B, or C is read or written for
          // anything other than INC_HL (which doesn't touch BC).
          for (const MachineOperand &MO : BIt->operands()) {
            if (MO.isRegMask()) { BcTouched = true; break; }
            if (!MO.isReg()) continue;
            Register R = MO.getReg();
            if (!R.isPhysical()) continue;
            if (TRI->regsOverlap(R, Z80::BC) ||
                TRI->regsOverlap(R, Z80::B)  ||
                TRI->regsOverlap(R, Z80::C)) {
              BcTouched = true; break;
            }
          }
          if (BcTouched) break;
        }
        if (BcTouched) continue;
        if (IncHlM > IncBcN) continue;  // body advances HL further than save anticipated; out of scope
        // Rewrite.
        DebugLoc DL = LdCL->getDebugLoc();
        unsigned ExtraIncs = IncBcN - IncHlM;
        for (unsigned i = 0; i < ExtraIncs; ++i)
          BuildMI(BB, std::next(LdHB), DL, TII->get(Z80::INC_HL));
        LdCL->eraseFromParent();
        LdBH->eraseFromParent();
        // Erase the INC_BCs (they are between LdBH and the body).
        // We saved their range as [It .. body start) effectively;
        // re-walk from BB.begin() to find them.
        SmallVector<MachineInstr *, 4> ToErase;
        for (auto Iter = BB.begin(); Iter != BB.end(); ++Iter) {
          if (Iter->getOpcode() == Z80::INC_BC) {
            ToErase.push_back(&*Iter);
            if (ToErase.size() == IncBcN) break;
          } else if (Iter->getOpcode() != Z80::INC_BC &&
                     !Iter->isMetaInstruction()) {
            // Stop at first non-INC_BC after we've started collecting.
            if (!ToErase.empty()) break;
          }
        }
        for (auto *MI : ToErase) MI->eraseFromParent();
        LdLC->eraseFromParent();
        LdHB->eraseFromParent();
        Changed = true;
        LLVM_DEBUG(dbgs() << "  #84: removed HL-via-BC roundtrip in "
                          << printMBBReference(BB) << "\n");
      }
    }

    // --- Peephole: BC ping-pong in single-BB self-loops (issue #97) ---
    //
    // Sibling of #84.  Hand-written or post-Z80LoopRotate single-BB
    // self-loops with a PHI'd pointer put the pointer in BC across the
    // back-edge and HL during the store body, with `LD L,C; LD H,B`
    // reloading at every iteration and `INC BC × N` advancing for the
    // back-edge.  The preheader sets up the BC copy with `LD C,L; LD
    // B,H`.  Two orderings appear:
    //
    //   Case A (rotation NOT applied at IR level):
    //     loop:  LD L,C; LD H,B; <stores via (HL), INC HL × M>;
    //            INC BC × N; <trailer>; <branch>
    //
    //   Case B (rotation applied at IR level — back-edge advance now
    //           emitted FIRST within the body):
    //     loop:  <leading>; INC BC × N; <stores via (HL), INC HL × M>;
    //            <trailer>; LD L,C; LD H,B; <branch>
    //
    // Both forms use 4 + 4 + N bytes more than the no-ping-pong shape.
    //
    // Rewrite (when guards pass):
    //   - Erase pred's LD C,L; LD B,H.
    //   - Erase loop's LD L,C; LD H,B.
    //   - Replace loop's INC BC × N with INC HL × (N - M).
    //
    // Saves 4 (pred pair) + 4 (loop pair) + N - (N - M) = 8 + M bytes
    // per occurrence.  Closes ravn/llvm-z80#97.
    {
      auto TouchesReg = [&](const MachineInstr &MI, MCPhysReg Pair,
                            MCPhysReg Hi, MCPhysReg Lo) {
        if (MI.isMetaInstruction()) return false;
        for (const MachineOperand &MO : MI.operands()) {
          if (MO.isRegMask()) return true;
          if (!MO.isReg() || !MO.getReg().isPhysical()) continue;
          Register R = MO.getReg();
          if (TRI->regsOverlap(R, Pair) ||
              TRI->regsOverlap(R, Hi) ||
              TRI->regsOverlap(R, Lo))
            return true;
        }
        return false;
      };

      for (auto &LoopBB : MF) {
        // Single-BB self-loop with exactly two predecessors: itself + pred.
        if (LoopBB.pred_size() != 2) continue;
        bool IsSelfLoop = false;
        MachineBasicBlock *Pred = nullptr;
        for (MachineBasicBlock *P : LoopBB.predecessors()) {
          if (P == &LoopBB) IsSelfLoop = true;
          else Pred = P;
        }
        if (!IsSelfLoop || !Pred) continue;
        if (Pred->succ_size() != 1) continue;

        // Pred matcher.  Three accepted shapes:
        //   Case 1 (pointer from HL param): `LD C,L; LD B,H` adjacent.
        //     We'll drop both (BC = HL anyway, so HL still carries it).
        //   Case 2 (constant pointer initialized in both): `LD HL,nn N`
        //     and `LD BC,nn N` with matching immediate / global / MC
        //     symbol, no intervening modifications.  We'll drop the
        //     LD_BC_nn (HL keeps the value).
        //   Case 3 (constant pointer initialized in BC only): `LD BC,nn
        //     N` in pred with no LD_HL_nn at all and HL not live-in to
        //     LoopBB.  We'll rewrite the LD_BC_nn to LD_HL_nn (HL takes
        //     over as the carrier).
        auto PredTerm = Pred->getFirstTerminator();
        MachineInstr *LdCL = nullptr, *LdBH = nullptr;
        MachineInstr *LdBCnn = nullptr, *LdHLnn = nullptr;
        for (auto It = Pred->begin(); It != PredTerm; ++It) {
          if (It->getOpcode() == Z80::LD_C_L && !LdCL) {
            auto Next = std::next(It);
            while (Next != PredTerm && Next->isMetaInstruction()) ++Next;
            if (Next != PredTerm && Next->getOpcode() == Z80::LD_B_H) {
              LdCL = &*It;
              LdBH = &*Next;
            }
          }
          if (It->getOpcode() == Z80::LD_BC_nn) LdBCnn = &*It;
          if (It->getOpcode() == Z80::LD_HL_nn) LdHLnn = &*It;
        }
        bool MatchedCase1 = LdCL && LdBH;
        bool MatchedCase2 = false;
        bool MatchedCase3 = false;
        if (!MatchedCase1 && LdBCnn && LdHLnn &&
            LdBCnn->getNumOperands() >= 1 &&
            LdHLnn->getNumOperands() >= 1) {
          const MachineOperand &BCOp = LdBCnn->getOperand(0);
          const MachineOperand &HLOp = LdHLnn->getOperand(0);
          // Accept matching immediates, identical global addresses (with
          // same offset), or identical MC symbols.
          if (BCOp.isImm() && HLOp.isImm() &&
              BCOp.getImm() == HLOp.getImm()) {
            MatchedCase2 = true;
          } else if (BCOp.isGlobal() && HLOp.isGlobal() &&
                     BCOp.getGlobal() == HLOp.getGlobal() &&
                     BCOp.getOffset() == HLOp.getOffset()) {
            MatchedCase2 = true;
          } else if (BCOp.isMCSymbol() && HLOp.isMCSymbol() &&
                     BCOp.getMCSymbol() == HLOp.getMCSymbol() &&
                     BCOp.getOffset() == HLOp.getOffset()) {
            MatchedCase2 = true;
          } else if (BCOp.isBlockAddress() && HLOp.isBlockAddress() &&
                     BCOp.getBlockAddress() == HLOp.getBlockAddress() &&
                     BCOp.getOffset() == HLOp.getOffset()) {
            MatchedCase2 = true;
          }
        }
        if (!MatchedCase1 && !MatchedCase2 && LdBCnn && !LdHLnn) {
          // Case 3: only LD_BC_nn in pred.  We'll rewrite to LD_HL_nn.
          // Safety: HL must not be live-in to LoopBB before our rewrite
          // (the loop's LD_L_C; LD_H_B is what currently sets it).
          if (!LoopBB.isLiveIn(Z80::HL) &&
              !LoopBB.isLiveIn(Z80::H) &&
              !LoopBB.isLiveIn(Z80::L))
            MatchedCase3 = true;
        }
        if (!MatchedCase1 && !MatchedCase2 && !MatchedCase3) continue;

        bool Bail = false;
        if (MatchedCase1) {
          // After LD_B_H, no BC touch until terminator.
          for (auto It = std::next(LdBH->getIterator()); It != PredTerm; ++It) {
            if (TouchesReg(*It, Z80::BC, Z80::B, Z80::C)) {
              Bail = true; break;
            }
          }
        } else if (MatchedCase2) {
          // Between LD_BC_nn and terminator, no BC touch.
          for (auto It = std::next(LdBCnn->getIterator()); It != PredTerm;
               ++It) {
            if (TouchesReg(*It, Z80::BC, Z80::B, Z80::C)) {
              Bail = true; break;
            }
          }
          if (!Bail) {
            // And between LD_HL_nn and terminator, no HL touch.
            for (auto It = std::next(LdHLnn->getIterator()); It != PredTerm;
                 ++It) {
              if (TouchesReg(*It, Z80::HL, Z80::H, Z80::L)) {
                Bail = true; break;
              }
            }
          }
        } else { // MatchedCase3
          // Between LD_BC_nn and terminator, no BC touch.
          for (auto It = std::next(LdBCnn->getIterator()); It != PredTerm;
               ++It) {
            if (TouchesReg(*It, Z80::BC, Z80::B, Z80::C)) {
              Bail = true; break;
            }
          }
          // No HL touch anywhere in pred (we're about to write HL there).
          if (!Bail) {
            for (auto It = Pred->begin(); It != PredTerm; ++It) {
              if (TouchesReg(*It, Z80::HL, Z80::H, Z80::L)) {
                Bail = true; break;
              }
            }
          }
        }
        if (Bail) continue;

        // In LoopBB: find the unique LD_L_C; LD_H_B pair.
        auto LoopTerm = LoopBB.getFirstTerminator();
        if (LoopTerm == LoopBB.begin()) continue;
        MachineInstr *LdLC = nullptr, *LdHB = nullptr;
        for (auto It = LoopBB.begin(); It != LoopTerm; ++It) {
          if (It->getOpcode() != Z80::LD_L_C) continue;
          auto Next = std::next(It);
          while (Next != LoopTerm && Next->isMetaInstruction()) ++Next;
          if (Next == LoopTerm || Next->getOpcode() != Z80::LD_H_B) continue;
          if (LdLC) { Bail = true; break; }  // multiple pairs
          LdLC = &*It;
          LdHB = &*Next;
        }
        if (Bail || !LdLC || !LdHB) continue;

        // In LoopBB: find the unique INC_BC chain.
        MachineInstr *IncBcStart = nullptr;
        unsigned IncBcN = 0;
        bool ChainEnded = false;
        for (auto It = LoopBB.begin(); It != LoopTerm; ++It) {
          if (It->isMetaInstruction()) continue;
          if (It->getOpcode() == Z80::INC_BC) {
            if (ChainEnded) { Bail = true; break; }  // second chain
            if (!IncBcStart) IncBcStart = &*It;
            ++IncBcN;
          } else if (IncBcStart) {
            ChainEnded = true;
          }
        }
        if (Bail || !IncBcStart || IncBcN == 0) continue;

        // Determine ordering: which anchor comes first?  PingPongFirst is
        // the start of the earlier anchor; PingPongLast is the
        // (one-past-end) iterator of the later anchor.
        MachineBasicBlock::iterator FirstStart, FirstEnd, LastStart, LastEnd;
        bool LdLCFirst = false;
        // Find positions; iterate once to compare.
        auto LdLCIt = LdLC->getIterator();
        auto IncBcIt = IncBcStart->getIterator();
        for (auto It = LoopBB.begin(); It != LoopTerm; ++It) {
          if (&*It == LdLC) { LdLCFirst = true; break; }
          if (&*It == IncBcStart) { LdLCFirst = false; break; }
        }
        if (LdLCFirst) {
          FirstStart = LdLCIt;
          FirstEnd   = std::next(LdHB->getIterator());
          LastStart  = IncBcIt;
          LastEnd    = std::next(IncBcIt, IncBcN);
        } else {
          FirstStart = IncBcIt;
          FirstEnd   = std::next(IncBcIt, IncBcN);
          LastStart  = LdLCIt;
          LastEnd    = std::next(LdHB->getIterator());
        }

        // Region 1 — leading: [LoopBB.begin(), FirstStart).
        // Must not touch HL or BC.
        for (auto It = LoopBB.begin(); It != FirstStart; ++It) {
          if (TouchesReg(*It, Z80::BC, Z80::B, Z80::C) ||
              TouchesReg(*It, Z80::HL, Z80::H, Z80::L)) {
            Bail = true; break;
          }
        }
        if (Bail) continue;

        // Region 2 — body: [FirstEnd, LastStart).
        // Allowed: any read of HL (LD (HL),r / LD r,(HL) / LD (HL),n),
        // INC_HL (counted as M).  Not allowed: any def of HL other
        // than INC_HL, any touch of BC.
        unsigned IncHlM = 0;
        for (auto It = FirstEnd; It != LastStart; ++It) {
          if (It->isMetaInstruction()) continue;
          unsigned Op = It->getOpcode();
          if (Op == Z80::INC_HL) { ++IncHlM; continue; }
          // Other defs of HL/H/L → bail.  Touches of BC → bail.
          for (const MachineOperand &MO : It->operands()) {
            if (MO.isRegMask()) { Bail = true; break; }
            if (!MO.isReg() || !MO.getReg().isPhysical()) continue;
            Register R = MO.getReg();
            if (TRI->regsOverlap(R, Z80::BC) ||
                TRI->regsOverlap(R, Z80::B)  ||
                TRI->regsOverlap(R, Z80::C)) {
              Bail = true; break;
            }
            if (MO.isDef() &&
                (TRI->regsOverlap(R, Z80::HL) ||
                 TRI->regsOverlap(R, Z80::H)  ||
                 TRI->regsOverlap(R, Z80::L))) {
              Bail = true; break;
            }
          }
          if (Bail) break;
        }
        if (Bail) continue;
        if (IncHlM > IncBcN) continue;

        // Region 3 — trailer: [LastEnd, LoopTerm).  No HL/BC touches.
        for (auto It = LastEnd; It != LoopTerm; ++It) {
          if (TouchesReg(*It, Z80::BC, Z80::B, Z80::C) ||
              TouchesReg(*It, Z80::HL, Z80::H, Z80::L)) {
            Bail = true; break;
          }
        }
        if (Bail) continue;
        // Terminators must not touch HL/BC (e.g. JP (HL) would).
        for (auto It = LoopTerm; It != LoopBB.end(); ++It) {
          if (TouchesReg(*It, Z80::BC, Z80::B, Z80::C) ||
              TouchesReg(*It, Z80::HL, Z80::H, Z80::L)) {
            Bail = true; break;
          }
        }
        if (Bail) continue;

        // BC must be dead at every non-loop successor of LoopBB.
        for (MachineBasicBlock *Succ : LoopBB.successors()) {
          if (Succ == &LoopBB) continue;
          for (const auto &LI : Succ->liveins()) {
            if (TRI->regsOverlap(LI.PhysReg, Z80::BC) ||
                TRI->regsOverlap(LI.PhysReg, Z80::B)  ||
                TRI->regsOverlap(LI.PhysReg, Z80::C)) {
              Bail = true; break;
            }
          }
          if (Bail) break;
        }
        if (Bail) continue;

        // All guards passed.  Rewrite.
        DebugLoc DL = LdLC->getDebugLoc();
        unsigned ExtraIncs = IncBcN - IncHlM;

        // Insert INC_HL × ExtraIncs at LastStart (so they replace the
        // late anchor — INC_BC chain in case A, LD_L_C/LD_H_B in case B).
        for (unsigned i = 0; i < ExtraIncs; ++i)
          BuildMI(LoopBB, LastStart, DL, TII->get(Z80::INC_HL))
              .addReg(Z80::HL, RegState::Define)
              .addReg(Z80::HL);

        // Erase late anchor [LastStart, LastEnd).
        for (auto It = LastStart; It != LastEnd; ) {
          auto Cur = It++;
          Cur->eraseFromParent();
        }
        // Erase early anchor [FirstStart, FirstEnd).
        for (auto It = FirstStart; It != FirstEnd; ) {
          auto Cur = It++;
          Cur->eraseFromParent();
        }
        // Erase / rewrite pred's BC setup.
        if (MatchedCase1) {
          LdCL->eraseFromParent();
          LdBH->eraseFromParent();
        } else if (MatchedCase2) {
          LdBCnn->eraseFromParent();
        } else { // MatchedCase3
          // Replace LD_BC_nn N with LD_HL_nn N, copying the operand.
          // LD_*_nn's operand 0 is the immediate / global / MC-symbol;
          // there is no explicit def-reg operand (def is implicit).
          DebugLoc PredDL = LdBCnn->getDebugLoc();
          BuildMI(*Pred, LdBCnn, PredDL, TII->get(Z80::LD_HL_nn))
              .add(LdBCnn->getOperand(0));
          LdBCnn->eraseFromParent();
        }

        // Update liveness on LoopBB: drop $bc, ensure $hl.
        if (LoopBB.isLiveIn(Z80::BC)) LoopBB.removeLiveIn(Z80::BC);
        if (LoopBB.isLiveIn(Z80::B))  LoopBB.removeLiveIn(Z80::B);
        if (LoopBB.isLiveIn(Z80::C))  LoopBB.removeLiveIn(Z80::C);
        if (!LoopBB.isLiveIn(Z80::HL)) LoopBB.addLiveIn(Z80::HL);

        Changed = true;
        LLVM_DEBUG(dbgs() << "  #97: removed BC ping-pong in self-loop "
                          << printMBBReference(LoopBB)
                          << " (case " << (LdLCFirst ? "A" : "B")
                          << ", M=" << IncHlM << " N=" << IncBcN << ")\n");
      }
    }

    // --- Peephole: u8 switch range-check 16-bit → 8-bit (issue #86) ---
    //
    // GISel switch lowering on a u8 discriminator widens to i16 for
    // the jump-table index BEFORE the bound check, so the bound check
    // costs 9 B / 16-bit subtract:
    //
    //     DEC_A             ; offset = c - min
    //     LD_L_A             ; widen low byte
    //     LD_H_n 0           ; widen high byte
    //     LD_DE_nn N         ; load limit
    //     LD_A_E
    //     SUB_L
    //     LD_A_D
    //     SBC_A_H            ; HL <=> DE
    //     JR_NC/JP_NC default
    //
    // The same check is `CP N; JR_NC default` in 3 B, since A is the
    // u8 offset.  Reorder so the bound check uses the 8-bit form, and
    // the widen happens AFTER (when we know the bound check passed
    // and we'll need HL for jump-table indexing).
    //
    // Net: 9 B → 5 B (CP_n 2 B + JR_NC_e 2 B + LD_L_A/LD_H_n 0 still
    // needed at 3 B, was already accounted) -- save 4 B per switch.
    {
      for (auto MII = MBB.begin(); MII != MBB.end(); ) {
        if (MII->getOpcode() != Z80::LD_L_A) { ++MII; continue; }
        auto LdLA = MII;
        auto It = std::next(LdLA);
        if (It == MBB.end() || It->getOpcode() != Z80::LD_H_n ||
            !It->getOperand(0).isImm() ||
            It->getOperand(0).getImm() != 0) {
          ++MII; continue;
        }
        auto LdHN = It; ++It;
        if (It == MBB.end() || It->getOpcode() != Z80::LD_DE_nn ||
            !It->getOperand(0).isImm()) {
          ++MII; continue;
        }
        int64_t Limit = It->getOperand(0).getImm();
        // Limit must fit in 8 bits for `CP n` to be equivalent.
        if (Limit < 0 || Limit > 255) { ++MII; continue; }
        auto LdDE = It; ++It;
        if (It == MBB.end() || It->getOpcode() != Z80::LD_A_E) {
          ++MII; continue;
        }
        auto LdAE = It; ++It;
        if (It == MBB.end() || It->getOpcode() != Z80::SUB_L) {
          ++MII; continue;
        }
        auto SubL = It; ++It;
        if (It == MBB.end() || It->getOpcode() != Z80::LD_A_D) {
          ++MII; continue;
        }
        auto LdAD = It; ++It;
        if (It == MBB.end() || It->getOpcode() != Z80::SBC_A_H) {
          ++MII; continue;
        }
        auto SbcAH = It; ++It;
        if (It == MBB.end()) { ++MII; continue; }
        // Branch must be a carry-conditional out of the bound-check.
        unsigned BrOpc = It->getOpcode();
        if (BrOpc != Z80::JR_NC_e && BrOpc != Z80::JP_NC_nn &&
            BrOpc != Z80::JR_C_e && BrOpc != Z80::JP_C_nn) {
          ++MII; continue;
        }
        auto Br = It;

        // The DE high byte (D) is implicitly used by SBC -- it's read
        // here as the high byte of the limit.  After our rewrite, DE
        // is dead.  Conservative check: the LD_DE_nn defined DE just
        // for this comparison; if anything between LD_DE_nn and SbcAH
        // reads DE outside this chain, we'd already have bailed (none
        // of the matched instructions read DE except by name).
        // Same with HL -- after the rewrite, LD_L_A and LD_H_n 0
        // remain for jump-table indexing on the fall-through path.

        // The 16-bit chain computes `DE - HL = limit - offset`, so
        // carry-out is set iff offset > limit (out of range).
        // `CP_n limit` computes `A - limit`, so carry-out is set iff
        // offset < limit (in range).  These are *inverse* flags, so
        // the branch condition must flip.
        unsigned NewBrOpc;
        switch (BrOpc) {
        case Z80::JR_NC_e: NewBrOpc = Z80::JR_C_e; break;
        case Z80::JP_NC_nn: NewBrOpc = Z80::JP_C_nn; break;
        case Z80::JR_C_e:  NewBrOpc = Z80::JR_NC_e; break;
        case Z80::JP_C_nn: NewBrOpc = Z80::JP_NC_nn; break;
        default: ++MII; continue;
        }

        DebugLoc DL = LdLA->getDebugLoc();
        // Insert CP_n Limit before LdLA, then keep LD_L_A / LD_H_n 0
        // after (they preserve A / set H, neither touches flags).
        BuildMI(MBB, LdLA, DL, TII->get(Z80::CP_n)).addImm(Limit);
        LdDE->eraseFromParent();
        LdAE->eraseFromParent();
        SubL->eraseFromParent();
        LdAD->eraseFromParent();
        SbcAH->eraseFromParent();
        // Replace branch with flipped condition, same target.
        DebugLoc BrDL = Br->getDebugLoc();
        auto NewBr = BuildMI(MBB, Br, BrDL, TII->get(NewBrOpc));
        NewBr.add(Br->getOperand(0));
        Br->eraseFromParent();

        Changed = true;
        LLVM_DEBUG(dbgs() << "  #86: u8 switch range-check 16->8 bit\n");
        MII = std::next(LdHN);
      }
    }

    // --- Peephole: identity mask-roundtrip after SBC A,A (issue #79) ---
    //
    // GISel lowers `sext i1 → i8` of an `icmp ne` result via the
    // canonical (shl 7; ashr 7) idiom, which on Z80 expands to:
    //
    //     sbc  a, a       ; A is already 0xFF or 0x00 (the mask)
    //     and  $1         ; <-- mask-roundtrip starts here
    //     rrca            ;     {0xFF/00} -> {1/0} -> {0x80/00}
    //     and  $80
    //     add  a, a
    //     sbc  a, a       ; <-- ends here, A is back to 0xFF or 0x00
    //
    // The trailing 5 instructions (8 bytes) are an identity on the
    // value already in A.  This peephole detects the exact sequence
    // immediately after a SBC A,A (the canonical mask producer) and
    // deletes it.  Safe: the second `sbc a,a` would re-establish
    // FLAGS but in this position no FLAGS-using instruction follows
    // (we check); the outer Z/N/H/C state matches what the kept
    // SBC A,A established already.
    {
      SmallVector<MachineInstr *, 8> ToErase;
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ++MII) {
        if (MII->getOpcode() != Z80::SBC_A_A) continue;
        // The instruction whose tail we're considering -- mark and walk
        // forward exactly 5 specific instructions.
        auto It = std::next(MachineBasicBlock::iterator(&*MII));
        auto match = [&](unsigned Op, int64_t ExpectedImm = -1) {
          if (It == MIE || It->getOpcode() != Op) return false;
          if (ExpectedImm >= 0) {
            if (It->getNumOperands() < 1 || !It->getOperand(0).isImm() ||
                It->getOperand(0).getImm() != ExpectedImm)
              return false;
          }
          return true;
        };
        if (!match(Z80::AND_n, 0x01)) continue;
        auto andOne = It; ++It;
        if (!match(Z80::RRCA))      continue;
        auto rrca = It; ++It;
        if (!match(Z80::AND_n, 0x80))continue;
        auto and80 = It; ++It;
        if (!match(Z80::ADD_A_A))   continue;
        auto adda = It; ++It;
        if (!match(Z80::SBC_A_A))   continue;
        auto sbcAa = It; ++It;
        // Final guard: the kept SBC_A_A's FLAGS effect (Z/N/H/C from
        // the value-zero test of A on subtract-borrow) is the same
        // either way (subtract-borrow only depends on carry-in).  The
        // deleted SBC re-uses identical FLAGS as the kept one.  So
        // any subsequent flag consumer sees the same FLAGS state.
        // No FLAGS-bridge guard needed for THIS pattern.
        LLVM_DEBUG(dbgs() << "  #79: deleting mask-roundtrip after SBC A,A "
                          << "@ " << *MII);
        ToErase.push_back(&*andOne);
        ToErase.push_back(&*rrca);
        ToErase.push_back(&*and80);
        ToErase.push_back(&*adda);
        ToErase.push_back(&*sbcAa);
        Changed = true;
      }
      for (auto *MI : ToErase)
        MI->eraseFromParent();
    }

    // --- Peephole: carry-roundtrip + JR C → JR NC (issue #93) ---
    //
    // GISel lowers `add a,N; icmp ne 0` for count-up-to-zero loops
    // (which is what LSR rewrites a `for (i=N; i; i--)` countdown to)
    // via materializing the carry-out as i1 in A and rotating it back
    // into the carry flag for testing:
    //
    //     <carry-source>            ; e.g. ADD A,1 (sets C on wrap to 0)
    //     LD r, A                   ; save value back
    //     SBC A, A                  ; A = 0xFF if no carry, 0x00 if carry
    //     AND 1                     ; A = (was no-carry ? 1 : 0)
    //     XOR 1                     ; A = (was no-carry ? 0 : 1)
    //     RRCA                      ; bit 0 -> carry, so new C = old C
    //                               ;   inverted (this is the round-trip)
    //     JR C, target              ; loop iff old C was 0 (no wrap)
    //
    // The 4-instruction SBC/AND/XOR/RRCA chain is just an inverted
    // identity on carry; we can replace `chain; JR C, target` with
    // `JR NC, target` directly (test the same input carry, opposite
    // condition — no chain).  Saves 5 B per occurrence.
    //
    // Safety:
    //   - A is overwritten by the chain.  We replace by deleting the
    //     chain, so A retains its pre-chain value (which is the result
    //     of the carry-source op).  Require A dead after the branch.
    //   - The chain reads the carry input via SBC.  Between the carry
    //     source and the SBC there must be no flag-clobbering
    //     instruction.  We don't enforce this here — the pattern is
    //     specific enough that all known producers are safe.
    {
      auto isAndImm = [](const MachineInstr &MI, int64_t Imm) {
        return MI.getOpcode() == Z80::AND_n &&
               MI.getOperand(0).isImm() &&
               MI.getOperand(0).getImm() == Imm;
      };
      auto isXorImm = [](const MachineInstr &MI, int64_t Imm) {
        return MI.getOpcode() == Z80::XOR_n &&
               MI.getOperand(0).isImm() &&
               MI.getOperand(0).getImm() == Imm;
      };
      for (auto MII = MBB.begin(); MII != MBB.end(); ) {
        if (MII->getOpcode() != Z80::SBC_A_A) { ++MII; continue; }
        auto I0 = MII;
        SmallVector<MachineBasicBlock::iterator, 6> ToErase;
        ToErase.push_back(I0);
        auto It = std::next(I0);
        if (It == MBB.end() || !isAndImm(*It, 1)) { ++MII; continue; }
        ToErase.push_back(It); ++It;
        if (It == MBB.end() || !isXorImm(*It, 1)) { ++MII; continue; }
        ToErase.push_back(It); ++It;
        if (It == MBB.end()) { ++MII; continue; }

        // Two terminator forms after the SBC/AND 1/XOR 1 prefix:
        //   Form A (post-other-peepholes):  RRCA; JR C target
        //   Form B (pre-other-peepholes):   AND 1; OR A; JR NZ target
        unsigned NewBranchOp = 0;
        if (It->getOpcode() == Z80::RRCA) {
          ToErase.push_back(It); ++It;
          if (It == MBB.end()) { ++MII; continue; }
          if (It->getOpcode() == Z80::JR_C_e)
            NewBranchOp = Z80::JR_NC_e;
          else if (It->getOpcode() == Z80::JP_C_nn)
            NewBranchOp = Z80::JP_NC_nn;
        } else if (isAndImm(*It, 1)) {
          ToErase.push_back(It); ++It;
          if (It == MBB.end() || It->getOpcode() != Z80::OR_A) {
            ++MII; continue;
          }
          ToErase.push_back(It); ++It;
          if (It == MBB.end()) { ++MII; continue; }
          if (It->getOpcode() == Z80::JR_NZ_e)
            NewBranchOp = Z80::JR_NC_e;
          else if (It->getOpcode() == Z80::JP_NZ_nn)
            NewBranchOp = Z80::JP_NC_nn;
        }
        if (!NewBranchOp) { ++MII; continue; }
        auto IBranch = It;
        ToErase.push_back(IBranch);

        // A must be dead after the branch.
        if (!isRegDeadAfter(std::next(IBranch), MBB, TRI, Z80::A)) {
          ++MII; continue;
        }

        LLVM_DEBUG(dbgs() << "  #93: carry-roundtrip + branch → JR NC\n");
        DebugLoc DL = IBranch->getDebugLoc();
        BuildMI(MBB, IBranch, DL, TII->get(NewBranchOp))
            .add(IBranch->getOperand(0));
        auto NextMII = std::next(IBranch);
        for (auto &MI : ToErase)
          MI->eraseFromParent();
        Changed = true;
        MII = NextMII;
      }
    }

    // --- Peephole: ADD A,1; LD r,A → INC r (when carry-from-ADD dead, #93) ---
    //
    // After the #93 carry-roundtrip rewrite eliminates the SBC chain,
    // patterns like
    //     LD A,r ; ADD A,1 ; LD r,A ; JR NC target
    // remain.  When the JR's carry input is the only remaining use of
    // ADD's carry, AND we can rewrite as `INC r ; JR NZ target`
    // (because the only way ADD A,1 wraps is when result is 0 — Z and
    // !C are equivalent here).  Net: 5 B → 3 B per loop site.
    //
    // Safety:
    //   - INC r doesn't set carry; require carry dead after the
    //     branch (in practice the loop body re-establishes flags).
    //   - A is overwritten by `LD A,r ; ADD A,1`; require A dead after
    //     the branch.
    //   - r must be an 8-bit GPR (B/C/D/E/H/L) reachable by INC r.
    {
      auto getLDArSrc93 = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::LD_A_B: return Z80::B; case Z80::LD_A_C: return Z80::C;
        case Z80::LD_A_D: return Z80::D; case Z80::LD_A_E: return Z80::E;
        case Z80::LD_A_H: return Z80::H; case Z80::LD_A_L: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      auto getLDrAdst93 = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::LD_B_A: return Z80::B; case Z80::LD_C_A: return Z80::C;
        case Z80::LD_D_A: return Z80::D; case Z80::LD_E_A: return Z80::E;
        case Z80::LD_H_A: return Z80::H; case Z80::LD_L_A: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      auto getInc8Opc = [](MCPhysReg Reg) -> unsigned {
        switch (Reg) {
        case Z80::B: return Z80::INC_B; case Z80::C: return Z80::INC_C;
        case Z80::D: return Z80::INC_D; case Z80::E: return Z80::INC_E;
        case Z80::H: return Z80::INC_H; case Z80::L: return Z80::INC_L;
        default: return 0;
        }
      };
      for (auto MII = MBB.begin(); MII != MBB.end(); ) {
        MCPhysReg SrcR = getLDArSrc93(MII->getOpcode());
        if (!SrcR) { ++MII; continue; }
        auto I0 = MII;
        auto It = std::next(I0);
        if (It == MBB.end() || It->getOpcode() != Z80::ADD_A_n ||
            !It->getOperand(0).isImm() ||
            It->getOperand(0).getImm() != 1) {
          ++MII; continue;
        }
        auto I1 = It; ++It;
        if (It == MBB.end()) { ++MII; continue; }
        MCPhysReg DstR = getLDrAdst93(It->getOpcode());
        if (DstR != SrcR) { ++MII; continue; }
        auto I2 = It; ++It;
        if (It == MBB.end()) { ++MII; continue; }
        auto I3 = It;
        unsigned NewBranchOp;
        if (I3->getOpcode() == Z80::JR_NC_e)
          NewBranchOp = Z80::JR_NZ_e;
        else if (I3->getOpcode() == Z80::JP_NC_nn)
          NewBranchOp = Z80::JP_NZ_nn;
        else { ++MII; continue; }
        // A and FLAGS-via-carry must be dead after the branch.
        if (!isRegDeadAfter(std::next(I3), MBB, TRI, Z80::A)) {
          ++MII; continue;
        }
        unsigned IncOpc = getInc8Opc(SrcR);
        if (!IncOpc) { ++MII; continue; }
        LLVM_DEBUG(dbgs()
                   << "  #93: LD A,r; ADD A,1; LD r,A; JR NC → INC r; JR NZ\n");
        DebugLoc DL = I0->getDebugLoc();
        BuildMI(MBB, I0, DL, TII->get(IncOpc));
        BuildMI(MBB, I0, DL, TII->get(NewBranchOp))
            .add(I3->getOperand(0));
        auto NextMII = std::next(I3);
        I0->eraseFromParent();
        I1->eraseFromParent();
        I2->eraseFromParent();
        I3->eraseFromParent();
        Changed = true;
        MII = NextMII;
      }
    }

    // --- Peephole: consecutive `LD A,n; LD (addr),A` chain (issue #85) ---
    //
    // When ≥3 consecutive byte stores write to consecutive addresses,
    // replace the chain with a single LD HL,base + repeated
    // LD (HL),n; INC HL.  Per-pair cost: 2+3 = 5 B (current) vs 2+1
    // = 3 B (HL-walked, after one-time 3 B `ld hl, base` setup).
    //
    //   N=2: 10 B → 11 B  (loss; skip)
    //   N=3: 15 B → 11 B  (save 4 B)
    //   N=4: 20 B → 14 B  (save 6 B)
    //   ...
    //
    // The peephole walks the MBB, accumulates a maximal run of
    //   { LD A,imm; LD (addr),A }
    // pairs whose addresses are an arithmetic-1 progression, and
    // rewrites the run when length ≥ 3.  No flag-establishing
    // instructions in between (the `LD A,n` and `LD (nn),A` set no
    // flags, but a `CALL` or arithmetic op between would split the
    // run).  Side-effect-free reads are tolerated; we conservatively
    // require the inter-store instructions to be just the LD A,n
    // for the next pair.
    {
      // Address descriptor: either a numeric immediate or
      // (global symbol, offset).  Two stores are "consecutive" if
      // both descriptors match on the symbol part and differ in
      // offset by exactly one.
      struct AddrKey {
        const GlobalValue *GV; // nullptr for immediate
        int64_t Off;           // numeric address (when GV==nullptr) or offset
        bool isConsecutive(const AddrKey &Prev) const {
          return GV == Prev.GV && Off == Prev.Off + 1;
        }
      };
      auto getStoreAddr = [&](MachineInstr &MI) -> std::optional<AddrKey> {
        if (MI.getOpcode() != Z80::LD_nnind_A) return std::nullopt;
        const MachineOperand &Op = MI.getOperand(0);
        if (Op.isImm())
          return AddrKey{nullptr, Op.getImm()};
        if (Op.isGlobal())
          return AddrKey{Op.getGlobal(), (int64_t)Op.getOffset()};
        return std::nullopt;
      };
      auto buildBaseAddr = [&](MachineInstrBuilder &MIB,
                               const AddrKey &K) {
        if (K.GV)
          MIB.addGlobalAddress(K.GV, K.Off);
        else
          MIB.addImm(K.Off);
      };

      auto MII = MBB.begin();
      const auto MIE = MBB.end();
      while (MII != MIE) {
        // Look for the head of a run: LD A,imm0; LD (addr0),A.
        if (MII->getOpcode() != Z80::LD_A_n ||
            !MII->getOperand(0).isImm()) {
          ++MII; continue;
        }
        auto It2 = std::next(MachineBasicBlock::iterator(&*MII));
        if (It2 == MIE) break;
        auto firstAddr = getStoreAddr(*It2);
        if (!firstAddr) { ++MII; continue; }

        // Collect run: each subsequent pair must be
        //   LD A,imm_k; LD (addr_k),A   with addr_k = addr0 + k.
        struct Pair { MachineInstr *LdAn; MachineInstr *LdNnA; uint8_t Imm; };
        SmallVector<Pair, 8> Run;
        Run.push_back({&*MII, &*It2,
                       (uint8_t)MII->getOperand(0).getImm()});
        auto It3 = std::next(It2);
        while (It3 != MIE) {
          if (It3->getOpcode() != Z80::LD_A_n ||
              !It3->getOperand(0).isImm()) break;
          auto It4 = std::next(It3);
          if (It4 == MIE) break;
          auto a = getStoreAddr(*It4);
          AddrKey expected{firstAddr->GV,
                           firstAddr->Off + (int64_t)Run.size()};
          if (!a || a->GV != expected.GV || a->Off != expected.Off) break;
          Run.push_back({&*It3, &*It4,
                         (uint8_t)It3->getOperand(0).getImm()});
          It3 = std::next(It4);
        }

        if (Run.size() >= 3) {
          // Rewrite: ld hl, base; { ld (hl), imm; inc hl }+
          // Drop the trailing INC HL after the last store (one-byte save).
          DebugLoc DL = MII->getDebugLoc();
          auto MIB = BuildMI(MBB, MII, DL, TII->get(Z80::LD_HL_nn));
          buildBaseAddr(MIB, *firstAddr);
          for (size_t k = 0; k < Run.size(); ++k) {
            BuildMI(MBB, MII, DL, TII->get(Z80::LD_HLind_n))
                .addImm(Run[k].Imm);
            if (k + 1 < Run.size())
              BuildMI(MBB, MII, DL, TII->get(Z80::INC_HL));
          }
          for (auto &P : Run) {
            P.LdAn->eraseFromParent();
            P.LdNnA->eraseFromParent();
          }
          Changed = true;
          MII = It3; // resume after the run
          continue;
        }
        // Run too short (1 or 2 pairs); advance past the head pair only.
        MII = std::next(It2);
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

  // --- Peephole: cross-MBB CALL ; <fall through> ; RET → JP (issue #75) ---
  // When an MBB ends with CALL_nn (no explicit branch) and falls through
  // to an MBB whose first instruction is RET, the CALL can become a
  // TAILJMP -- the callee's RET will return directly to our caller.
  // Saves 1 byte per site.
  //
  // Same stack-arg safety check as the single-MBB version: no PUSHes
  // before the CALL in the producing MBB.  We don't touch the RET'ing
  // MBB; it remains as a target for any other predecessor.
  for (auto &MBB : MF) {
    auto Term = MBB.getLastNonDebugInstr();
    if (Term == MBB.end() || Term->getOpcode() != Z80::CALL_nn)
      continue;
    // Must have a single fall-through successor.
    if (MBB.succ_size() != 1) continue;
    MachineBasicBlock *Next = *MBB.succ_begin();
    // Successor's first non-debug instruction must be RET.
    auto NextFirst = Next->getFirstNonDebugInstr();
    if (NextFirst == Next->end() || NextFirst->getOpcode() != Z80::RET)
      continue;
    // Stack-args safety check.
    bool HasPush = false;
    for (auto It = MBB.begin(); It != Term; ++It) {
      if (It->isDebugInstr()) continue;
      unsigned Opc = It->getOpcode();
      if (Opc == Z80::PUSH_AF || Opc == Z80::PUSH_BC ||
          Opc == Z80::PUSH_DE || Opc == Z80::PUSH_HL ||
          Opc == Z80::PUSH_IX || Opc == Z80::PUSH_IY) {
        HasPush = true; break;
      }
    }
    if (HasPush) continue;
    // Replace CALL with TAILJMP, drop the fall-through to the RET MBB.
    LLVM_DEBUG(dbgs() << "  CALL → JP (cross-MBB tail call): " << *Term);
    MachineOperand &CallTarget = Term->getOperand(0);
    DebugLoc DL = Term->getDebugLoc();
    BuildMI(MBB, *Term, DL, TII->get(Z80::TAILJMP)).add(CallTarget);
    Term->eraseFromParent();
    // TAILJMP is isReturn, so no fall-through happens.  Remove the CFG
    // edge so successor MBB liveness reflects this.
    MBB.removeSuccessor(Next);
    Changed = true;
  }

  // --- Cross-block redundant LD A,r removal (issue #60) ---
  //
  // Forward dataflow tracking when register A is known to equal an 8-bit
  // GR8 register. The single-block peephole earlier in this pass handles
  // the in-block case; this pass extends it across basic block boundaries.
  //
  // The motivating example (fdc_get_result_bytes in autoload PROM):
  //     ld   d,a       ; A == D after this point
  //     cp   #2        ; CP doesn't touch A
  //     jr   nz,.L1    ; branch — A still == D on both edges
  //   .Lret:
  //     ld   a,d       ; REDUNDANT — A still == D
  //     ret
  //   .L1:
  //     ld   a,d       ; REDUNDANT — A still == D
  //     or   a
  //     ...
  //
  // State per program point: A is Top (initial for unprocessed MBBs),
  // Reg(r) (A == r), or Bottom (no fact known).
  //
  // Transfer:
  //   LD r,A          → Reg(r)         (A's value now also lives in r)
  //   LD A,r          → Reg(r)         (and if Known was already r, the
  //                                     LD is dead and removable)
  //   def of A or current Known.r → Bottom
  //   regmask (CALL)  → Bottom
  //   CP, OR A, branches, etc. that don't def A or Known.r → unchanged
  //
  // Meet (intersection over predecessor exits at MBB entry):
  //   Top ⊓ x = x;  Bottom ⊓ x = Bottom
  //   Reg(r) ⊓ Reg(r) = Reg(r);  Reg(r) ⊓ Reg(s) = Bottom (r ≠ s)
  //
  // Limitation: tracks only ONE register at a time (most recently set).
  // If both `LD D,A` and `LD E,A` are seen, only Reg(E) is remembered.
  // Sufficient for the issue #60 patterns; can be extended to a set if
  // future cases warrant.
  {
    enum AKKind : uint8_t { AK_Top = 0, AK_Bottom = 1, AK_Reg = 2 };
    struct AK {
      uint8_t Kind = AK_Top;
      MCPhysReg Reg = 0;
      bool operator==(const AK &O) const {
        return Kind == O.Kind && (Kind != AK_Reg || Reg == O.Reg);
      }
    };
    auto akTop = []() { AK a; a.Kind = AK_Top; return a; };
    auto akBot = []() { AK a; a.Kind = AK_Bottom; return a; };
    auto akReg = [](MCPhysReg R) { AK a; a.Kind = AK_Reg; a.Reg = R; return a; };
    auto akMeet = [&](AK X, AK Y) -> AK {
      if (X.Kind == AK_Top) return Y;
      if (Y.Kind == AK_Top) return X;
      if (X.Kind == AK_Bottom || Y.Kind == AK_Bottom) return akBot();
      return X.Reg == Y.Reg ? X : akBot();
    };

    // LD A,r → r, else 0.
    auto ldA_src60x = [](unsigned Opc) -> MCPhysReg {
      switch (Opc) {
      case Z80::LD_A_B: return Z80::B; case Z80::LD_A_C: return Z80::C;
      case Z80::LD_A_D: return Z80::D; case Z80::LD_A_E: return Z80::E;
      case Z80::LD_A_H: return Z80::H; case Z80::LD_A_L: return Z80::L;
      default: return 0;
      }
    };
    // LD r,A → r, else 0.
    auto ldR_A_dst60x = [](unsigned Opc) -> MCPhysReg {
      switch (Opc) {
      case Z80::LD_B_A: return Z80::B; case Z80::LD_C_A: return Z80::C;
      case Z80::LD_D_A: return Z80::D; case Z80::LD_E_A: return Z80::E;
      case Z80::LD_H_A: return Z80::H; case Z80::LD_L_A: return Z80::L;
      default: return 0;
      }
    };

    // Apply transfer for one instruction. If `Redundant` is non-null and the
    // instruction is a LD A,r whose effect is a no-op given Known, set it.
    auto step60x = [&](MachineInstr &MI, AK Known, bool *Redundant) -> AK {
      if (Redundant) *Redundant = false;
      if (MI.isDebugInstr())
        return Known;
      unsigned Opc = MI.getOpcode();
      if (Opc == TargetOpcode::KILL || Opc == TargetOpcode::IMPLICIT_DEF)
        return Known;

      // LD A,r — sets Known to r. Redundant if A already equals r.
      if (MCPhysReg Src = ldA_src60x(Opc)) {
        if (Known.Kind == AK_Reg && Known.Reg == Src && Redundant)
          *Redundant = true;
        return akReg(Src);
      }
      // LD r,A — A's value is now in r as well. We deliberately bypass the
      // generic clobber check for r, because we want Known := r afterwards
      // even though r is being defined.
      if (MCPhysReg Dst = ldR_A_dst60x(Opc))
        return akReg(Dst);

      // OR A and AND A are idempotent on A's value (A := A op A == A);
      // they only update FLAGS. Treat them as not clobbering A even though
      // their MCInstrDesc declares an implicit def of A. Without this,
      // Known is dropped after the OR A that typically follows a save+test
      // sequence, defeating the cross-block reload elimination on the
      // fall-through path. LD A,A is similarly a no-op.
      if (Opc == Z80::OR_A || Opc == Z80::AND_A || Opc == Z80::LD_A_A)
        return Known;

      // Anything else: scan operands + implicit defs for clobbers of A
      // or the currently-tracked Known.Reg.
      bool ClobberA = false, ClobberKnown = false;
      for (const MachineOperand &MO : MI.operands()) {
        if (MO.isRegMask()) {
          // CALL et al. — A is caller-saved per sdcccall, treat as clobbered.
          ClobberA = true; ClobberKnown = true; break;
        }
        if (!MO.isReg() || !MO.isDef() || !MO.getReg().isPhysical())
          continue;
        Register R = MO.getReg();
        if (TRI->regsOverlap(R, Z80::A))
          ClobberA = true;
        if (Known.Kind == AK_Reg && TRI->regsOverlap(R, Known.Reg))
          ClobberKnown = true;
      }
      for (MCPhysReg D : MI.getDesc().implicit_defs()) {
        if (TRI->regsOverlap(D, Z80::A))
          ClobberA = true;
        if (Known.Kind == AK_Reg && TRI->regsOverlap(D, Known.Reg))
          ClobberKnown = true;
      }
      if (ClobberA || ClobberKnown)
        return akBot();
      return Known;
    };

    // Iterate Entry/Exit per MBB to fixpoint. RPO seeds the first pass;
    // a small fixed iteration cap handles back-edges (loops).
    DenseMap<MachineBasicBlock *, AK> EntryAK, ExitAK;
    for (auto &MBB : MF) {
      EntryAK[&MBB] = akTop();
      ExitAK[&MBB] = akTop();
    }
    if (!MF.empty())
      EntryAK[&MF.front()] = akBot();

    SmallVector<MachineBasicBlock *, 32> RPO;
    for (auto *BB : ReversePostOrderTraversal<MachineFunction *>(&MF))
      RPO.push_back(BB);

    bool DfChanged = true;
    int Iter = 0;
    while (DfChanged && Iter++ < 16) {
      DfChanged = false;
      for (auto *BB : RPO) {
        if (BB != &MF.front()) {
          AK E = akTop();
          for (auto *Pred : BB->predecessors())
            E = akMeet(E, ExitAK[Pred]);
          if (!(EntryAK[BB] == E)) {
            EntryAK[BB] = E;
            DfChanged = true;
          }
        }
        AK K = EntryAK[BB];
        for (auto &MI : *BB)
          K = step60x(MI, K, nullptr);
        if (!(ExitAK[BB] == K)) {
          ExitAK[BB] = K;
          DfChanged = true;
        }
      }
    }

    // Removal pass: walk each MBB applying step from EntryAK; collect
    // LD A,r whose effect is a no-op given current Known.
    SmallVector<MachineInstr *, 16> ToErase60x;
    for (auto &MBB : MF) {
      AK K = EntryAK[&MBB];
      for (auto &MI : MBB) {
        bool Red = false;
        AK Next = step60x(MI, K, &Red);
        if (Red) {
          LLVM_DEBUG(dbgs() << "  Cross-block redundant LD A,r: " << MI);
          ToErase60x.push_back(&MI);
        }
        K = Next;
      }
    }
    for (auto *MI : ToErase60x) {
      MI->eraseFromParent();
      Changed = true;
    }
  }

  // --- Peephole: AND $1 + branch/ret → RRCA + carry branch/ret ---
  // AND $1 (2B) tests bit 0 via Z flag.  RRCA (1B) rotates bit 0 into carry.
  // Replace AND $1; JR/JP NZ → RRCA; JR/JP C (saves 1B).
  // Replace AND $1; RET NZ → RRCA; RET C (saves 1B).
  // Similarly AND $80; ... NZ → RLCA; ... C (bit 7 to carry).
  // Constraint: A must be dead on the fall-through path (RRCA modifies A
  // differently than AND).
  for (auto &MBB : MF) {
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      unsigned Opc = MII->getOpcode();
      if (Opc != Z80::AND_n) { ++MII; continue; }

      int64_t Imm = MII->getOperand(0).getImm();
      unsigned RotOpc;
      if (Imm == 1)
        RotOpc = Z80::RRCA;  // bit 0 → carry
      else if (Imm == 0x80)
        RotOpc = Z80::RLCA;  // bit 7 → carry
      else { ++MII; continue; }

      auto Next = std::next(MII);
      if (Next == MIE) { ++MII; continue; }

      // Skip redundant OR A between AND and branch (re-tests Z flag).
      auto BranchIt = Next;
      if (BranchIt->getOpcode() == Z80::OR_A ||
          BranchIt->getOpcode() == Z80::OR_r) {
        BranchIt = std::next(BranchIt);
        if (BranchIt == MIE) { ++MII; continue; }
      }

      // Map NZ → C, Z → NC (AND sets Z when bit is 0; rotate sets C when 1)
      unsigned NextOpc = BranchIt->getOpcode();
      unsigned NewNextOpc = 0;
      switch (NextOpc) {
      case Z80::JR_NZ_e:  NewNextOpc = Z80::JR_C_e; break;
      case Z80::JR_Z_e:   NewNextOpc = Z80::JR_NC_e; break;
      case Z80::JP_NZ_nn: NewNextOpc = Z80::JP_C_nn; break;
      case Z80::JP_Z_nn:  NewNextOpc = Z80::JP_NC_nn; break;
      case Z80::RET_NZ:   NewNextOpc = Z80::RET_C; break;
      case Z80::RET_Z:    NewNextOpc = Z80::RET_NC; break;
      default: break;
      }
      if (!NewNextOpc) { ++MII; continue; }

      // A must be dead after the branch/ret on the fall-through path.
      auto AfterBranch = std::next(BranchIt);
      if (!isRegDeadAfter(AfterBranch, MBB, TRI, Z80::A)) { ++MII; continue; }

      LLVM_DEBUG(dbgs() << "  AND→RRCA/RLCA peephole: " << *MII);
      DebugLoc DL = MII->getDebugLoc();

      // Erase any OR A between AND and branch.
      if (BranchIt != Next) {
        // Next is the OR_A, BranchIt is the branch
        Next->eraseFromParent();
      }

      MII = MBB.erase(MII); // erase AND
      BuildMI(MBB, MII, DL, TII->get(RotOpc));

      // Replace the branch/ret condition
      DebugLoc DL2 = BranchIt->getDebugLoc();
      if (BranchIt->getNumOperands() > 0 && BranchIt->getOperand(0).isMBB()) {
        MachineBasicBlock *Target = BranchIt->getOperand(0).getMBB();
        MII = MBB.erase(BranchIt);
        BuildMI(MBB, MII, DL2, TII->get(NewNextOpc)).addMBB(Target);
      } else {
        MII = MBB.erase(BranchIt);
        BuildMI(MBB, MII, DL2, TII->get(NewNextOpc));
      }
      Changed = true;
    }
  }

  // --- Peephole: PUSH IX; POP HL; ADD HL,rr; PUSH HL; POP IX → ADD IX,rr ---
  // Also matches: COPY16_PUSHPOP HL,IX; ADD HL,rr; COPY16_PUSHPOP IX,HL
  // When a 16-bit addition is performed through HL with IX/IY as the actual
  // accumulator, replace the copy-add-copy sequence with ADD IX,rr (2 bytes).
  // Also handles ADD HL,HL → ADD IX,IX (left shift by 1).
  if (STI.hasZ80()) {
    for (auto &MBB : MF) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        unsigned Opc = MII->getOpcode();

        // Determine if this starts an IX/IY→HL copy (PUSH+POP or pseudo).
        bool IsIX = false, IsIY = false;
        bool IsPseudoCopy = false;
        if (Opc == Z80::PUSH_IX) IsIX = true;
        else if (Opc == Z80::PUSH_IY) IsIY = true;
        else if (Opc == Z80::COPY16_PUSHPOP) {
          Register Src = MII->getOperand(1).getReg();
          Register Dst = MII->getOperand(0).getReg();
          if (Dst == Z80::HL) {
            if (Src == Z80::IX) { IsIX = true; IsPseudoCopy = true; }
            else if (Src == Z80::IY) { IsIY = true; IsPseudoCopy = true; }
          }
        }
        if (!IsIX && !IsIY) { ++MII; continue; }

        // For PUSH+POP form: I1=PUSH, I2=POP HL, I3=ADD
        // For pseudo form: I1=COPY16_PUSHPOP, I3=ADD (no I2)
        auto I1 = MII;
        MachineBasicBlock::iterator I2, I3;
        if (IsPseudoCopy) {
          I2 = MIE; // no POP instruction
          I3 = std::next(I1);
        } else {
          I2 = std::next(I1);
          if (I2 == MIE || I2->getOpcode() != Z80::POP_HL) { ++MII; continue; }
          I3 = std::next(I2);
        }
        if (I3 == MIE) { ++MII; continue; }
        unsigned AddOpc = I3->getOpcode();
        if (AddOpc != Z80::ADD_HL_BC && AddOpc != Z80::ADD_HL_DE &&
            AddOpc != Z80::ADD_HL_HL) { ++MII; continue; }

        // Match the HL→IX/IY copy after ADD (PUSH+POP or pseudo).
        auto I4 = std::next(I3);
        if (I4 == MIE) { ++MII; continue; }
        MachineBasicBlock::iterator I5;
        bool EndIsPseudo = false;
        if (I4->getOpcode() == Z80::COPY16_PUSHPOP) {
          Register Dst4 = I4->getOperand(0).getReg();
          Register Src4 = I4->getOperand(1).getReg();
          Register ExpIR = IsIX ? Z80::IX : Z80::IY;
          if (Src4 != Z80::HL || Dst4 != ExpIR) { ++MII; continue; }
          I5 = MIE; // no separate POP
          EndIsPseudo = true;
        } else {
          if (I4->getOpcode() != Z80::PUSH_HL) { ++MII; continue; }
          I5 = std::next(I4);
          unsigned ExpectedPop = IsIX ? Z80::POP_IX : Z80::POP_IY;
          if (I5 == MIE || I5->getOpcode() != ExpectedPop) { ++MII; continue; }
        }

        // Determine replacement opcode.
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
        if (I5 != MIE) I5->eraseFromParent();
        I4->eraseFromParent();
        I3->eraseFromParent();
        if (I2 != MIE) I2->eraseFromParent();
        MII = MBB.erase(I1);
        BuildMI(MBB, MII, DL, TII->get(NewOpc));
        Changed = true;
      }
    }
  }

  // --- Peephole: IX/IY transfer elimination ---
  // The register allocator saves caller-saved registers across CALLs by
  // transferring to callee-saved IX/IY via PUSH rr; POP IX ... PUSH IX; POP rr.
  // This costs 6B (1+2+2+1) when a simple PUSH rr ... POP rr costs only 2B (1+1).
  // Also matches COPY16_PUSHPOP pseudo form (issue #32):
  //   COPY16_PUSHPOP IX, rr ... COPY16_PUSHPOP rr, IX → PUSH rr ... POP rr
  // Note: CopyCost=3 on IR16 reduces but does not eliminate this pattern.
  if (STI.hasZ80()) {
    // Helper: check if an instruction touches a given register.
    auto touchesReg = [&](const MachineInstr &MI, MCPhysReg Reg) {
      for (const MachineOperand &MO : MI.operands()) {
        if (MO.isReg() && MO.getReg().isPhysical() &&
            TRI->regsOverlap(MO.getReg(), Reg))
          return true;
      }
      return false;
    };

    static const struct {
      unsigned PushOpc;
      unsigned PopOpc;
      unsigned PushIXOpc;
      unsigned PopIXOpc;
      MCPhysReg IXReg;
    } IXTransferPairs[] = {
        {Z80::PUSH_DE, Z80::POP_DE, Z80::PUSH_IX, Z80::POP_IX, Z80::IX},
        {Z80::PUSH_HL, Z80::POP_HL, Z80::PUSH_IX, Z80::POP_IX, Z80::IX},
        {Z80::PUSH_BC, Z80::POP_BC, Z80::PUSH_IX, Z80::POP_IX, Z80::IX},
        {Z80::PUSH_DE, Z80::POP_DE, Z80::PUSH_IY, Z80::POP_IY, Z80::IY},
        {Z80::PUSH_HL, Z80::POP_HL, Z80::PUSH_IY, Z80::POP_IY, Z80::IY},
        {Z80::PUSH_BC, Z80::POP_BC, Z80::PUSH_IY, Z80::POP_IY, Z80::IY},
    };

    for (auto &MBB : MF) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE; ++MII) {

        // --- Form 1: PUSH rr; POP IX ... PUSH IX; POP rr ---
        for (const auto &TP : IXTransferPairs) {
          if (MII->getOpcode() != TP.PushOpc)
            continue;
          auto PopIX = std::next(MII);
          if (PopIX == MIE || PopIX->getOpcode() != TP.PopIXOpc)
            continue;

          bool Safe = true;
          int StackDepth = 0;
          auto PushIX = MIE;
          for (auto Scan = std::next(PopIX); Scan != MIE; ++Scan) {
            unsigned SOpc = Scan->getOpcode();
            if (touchesReg(*Scan, TP.IXReg)) { Safe = false; break; }
            if (SOpc == TP.PushIXOpc && StackDepth == 0) {
              auto PopRR = std::next(Scan);
              if (PopRR != MIE && PopRR->getOpcode() == TP.PopOpc)
                PushIX = Scan;
              break;
            }
            if (SOpc == Z80::PUSH_AF || SOpc == Z80::PUSH_BC ||
                SOpc == Z80::PUSH_DE || SOpc == Z80::PUSH_HL ||
                SOpc == Z80::PUSH_IX || SOpc == Z80::PUSH_IY)
              ++StackDepth;
            else if (SOpc == Z80::POP_AF || SOpc == Z80::POP_BC ||
                     SOpc == Z80::POP_DE || SOpc == Z80::POP_HL ||
                     SOpc == Z80::POP_IX || SOpc == Z80::POP_IY)
              --StackDepth;
            if (StackDepth < 0) { Safe = false; break; }
          }
          if (!Safe || PushIX == MIE)
            continue;
          LLVM_DEBUG(dbgs() << "  IX transfer peephole: PUSH rr;POP IX...PUSH IX;POP rr"
                            << " → PUSH rr...POP rr (saves 4B)\n");
          PushIX->eraseFromParent();
          PopIX->eraseFromParent();
          Changed = true;
          break;
        }

        // --- Form 2: COPY16_PUSHPOP IX, rr ... COPY16_PUSHPOP rr, IX ---
        // Replace with PUSH rr ... POP rr (saves 4B).
        if (MII->getOpcode() == Z80::COPY16_PUSHPOP) {
          Register Dst1 = MII->getOperand(0).getReg();
          Register Src1 = MII->getOperand(1).getReg();
          if (!Z80::IR16RegClass.contains(Dst1)) continue;
          if (Z80::IR16RegClass.contains(Src1)) continue; // IX→IY, skip
          MCPhysReg IXReg = Dst1; // IX or IY
          Register RR = Src1;     // the GR16 being saved

          // Scan for matching reverse copy.
          bool Safe = true;
          auto EndCopy = MIE;
          int StackDepth = 0;
          for (auto Scan = std::next(MII->getIterator()); Scan != MIE; ++Scan) {
            unsigned SOpc = Scan->getOpcode();
            if (touchesReg(*Scan, IXReg)) {
              // Check if this IS the matching reverse copy.
              if (SOpc == Z80::COPY16_PUSHPOP && StackDepth == 0) {
                Register Dst2 = Scan->getOperand(0).getReg();
                Register Src2 = Scan->getOperand(1).getReg();
                if (Src2 == IXReg && Dst2 == RR) {
                  EndCopy = Scan;
                }
              }
              break; // IX/IY touched — stop scanning either way
            }
            if (SOpc == Z80::PUSH_AF || SOpc == Z80::PUSH_BC ||
                SOpc == Z80::PUSH_DE || SOpc == Z80::PUSH_HL ||
                SOpc == Z80::PUSH_IX || SOpc == Z80::PUSH_IY)
              ++StackDepth;
            else if (SOpc == Z80::POP_AF || SOpc == Z80::POP_BC ||
                     SOpc == Z80::POP_DE || SOpc == Z80::POP_HL ||
                     SOpc == Z80::POP_IX || SOpc == Z80::POP_IY)
              --StackDepth;
            if (StackDepth < 0) { Safe = false; break; }
          }
          if (!Safe || EndCopy == MIE) continue;

          LLVM_DEBUG(dbgs() << "  IX transfer peephole: COPY16_PUSHPOP pair"
                            << " → PUSH rr...POP rr (saves 4B)\n");
          // Replace start COPY16_PUSHPOP IX, rr with PUSH rr.
          DebugLoc DL1 = MII->getDebugLoc();
          // Replace end COPY16_PUSHPOP rr, IX with POP rr.
          DebugLoc DL2 = EndCopy->getDebugLoc();
          BuildMI(MBB, *EndCopy, DL2, TII->get(Z80::getPopOpcode(RR)));
          EndCopy->eraseFromParent();
          auto NextMII = MBB.erase(MII);
          BuildMI(MBB, NextMII, DL1, TII->get(Z80::getPushOpcode(RR)));
          MII = std::prev(NextMII); // will be incremented by the loop
          Changed = true;
        }
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
    // Helper: BSS store/load opcode tables, indexed by register-pair.
    // Issue #74: cross-register-pair spills (e.g. store HL, reload as DE)
    // also convert to PUSH+POP -- PUSH (storeReg); POP (loadReg) costs 2B
    // vs 6-8B for the BSS pair, and the value bytes are the same.
    struct StoreInfo {
      unsigned StoreOpc;   // LD_nnind_A, LD_nnind_HL, etc.
      unsigned PushOpc;    // PUSH_AF, PUSH_HL, etc.
      unsigned Bytes;      // 3 or 4
      bool Is8Bit;         // A vs 16-bit pair
    };
    struct LoadInfo {
      unsigned LoadOpc;    // LD_A_nnind, LD_HL_nnind, etc.
      unsigned PopOpc;     // POP_AF, POP_HL, etc.
      unsigned PushOpc;    // for re-PUSH after non-last POP (uses load reg)
      unsigned Bytes;      // 3 or 4
      bool Is8Bit;
    };
    static const StoreInfo Stores[] = {
        {Z80::LD_nnind_A,  Z80::PUSH_AF, 3, true},
        {Z80::LD_nnind_HL, Z80::PUSH_HL, 3, false},
        {Z80::LD_nnind_DE, Z80::PUSH_DE, 4, false},
        {Z80::LD_nnind_BC, Z80::PUSH_BC, 4, false},
    };
    static const LoadInfo Loads_[] = {
        {Z80::LD_A_nnind,  Z80::POP_AF, Z80::PUSH_AF, 3, true},
        {Z80::LD_HL_nnind, Z80::POP_HL, Z80::PUSH_HL, 3, false},
        {Z80::LD_DE_nnind, Z80::POP_DE, Z80::PUSH_DE, 4, false},
        {Z80::LD_BC_nnind, Z80::POP_BC, Z80::PUSH_BC, 4, false},
    };

    auto getStoreInfo = [&](unsigned Opc) -> const StoreInfo * {
      for (const auto &S : Stores)
        if (S.StoreOpc == Opc) return &S;
      return nullptr;
    };
    auto getLoadInfo = [&](unsigned Opc) -> const LoadInfo * {
      for (const auto &L : Loads_)
        if (L.LoadOpc == Opc) return &L;
      return nullptr;
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
      // MO_MCSymbol::isIdenticalTo only compares the symbol pointer, NOT
      // the offset.  We must also compare offsets to distinguish e.g.
      // __sfrend-10 from __sfrend-16.
      const MachineOperand &MOA = A.getOperand(0);
      const MachineOperand &MOB = B.getOperand(0);
      if (!MOA.isIdenticalTo(MOB))
        return false;
      if (MOA.isMCSymbol())
        return MOA.getOffset() == MOB.getOffset();
      return true;
    };

    // Collect all candidate conversions on pristine code first, then apply.
    // Single-pass apply-as-we-go would let an earlier conversion's inserted
    // POP appear unbalanced from a later store's narrow scan window.
    struct LoadEntry {
      MachineBasicBlock::iterator It;
      const LoadInfo *Info;
    };
    struct Candidate {
      MachineBasicBlock::iterator StoreIt;
      const StoreInfo *SI;
      SmallVector<LoadEntry, 4> Loads;
    };

    for (MachineBasicBlock &MBB : MF) {
      SmallVector<Candidate, 8> Candidates;
      auto MIE = MBB.end();
      for (auto MII = MBB.begin(); MII != MIE; ++MII) {
        const StoreInfo *SI = getStoreInfo(MII->getOpcode());
        if (!SI) continue;
        if (!isSfrendSymbol(MII->getOperand(0))) continue;

        // Found a BSS spill store.  Scan forward for matching same-address
        // loads to ANY register pair of the same width (8 or 16 bit).  Any
        // mixed-width same-address load (e.g. 8-bit load from a 16-bit
        // store's slot) is treated as an orphan and we bail (issue #82).
        bool Conflict = false;
        SmallVector<LoadEntry, 4> LoadList;
        // Track PUSH/POP balance for ALL register pairs (not just ours).
        // PUSH/POP is LIFO: if another register is pushed between our
        // PUSH and POP, our POP would get the wrong value.  We must
        // ensure the stack depth (from all PUSH/POP instructions) is 0
        // at each of our matching loads.
        int StackDepth = 0;

        auto isAnyPush = [](unsigned Opc) {
          return Opc == Z80::PUSH_BC || Opc == Z80::PUSH_DE ||
                 Opc == Z80::PUSH_HL || Opc == Z80::PUSH_AF ||
                 Opc == Z80::PUSH_IX || Opc == Z80::PUSH_IY;
        };
        auto isAnyPop = [](unsigned Opc) {
          return Opc == Z80::POP_BC || Opc == Z80::POP_DE ||
                 Opc == Z80::POP_HL || Opc == Z80::POP_AF ||
                 Opc == Z80::POP_IX || Opc == Z80::POP_IY;
        };

        for (auto Scan = std::next(MII); Scan != MIE; ++Scan) {
          unsigned SOpc = Scan->getOpcode();

          // Another store to the same sfrend slot = conflict (reuse).
          if (getStoreInfo(SOpc) && sameAddress(*MII, *Scan)) {
            Conflict = true;
            break;
          }

          // Track ALL PUSH/POP instructions between our store and
          // its matching load.  Any unbalanced push means our POP
          // would retrieve a different register's value.
          if (isAnyPush(SOpc)) StackDepth++;
          if (isAnyPop(SOpc)) {
            StackDepth--;
            if (StackDepth < 0) { Conflict = true; break; }
          }

          // BSS load from the same address.
          if (const LoadInfo *LI = getLoadInfo(SOpc)) {
            if (sameAddress(*MII, *Scan)) {
              // Mixed-width load (e.g. 8-bit load from a 16-bit slot, or
              // vice versa) is an orphan -- value bytes wouldn't match.
              // See issue #82 for the original same-pair-only orphan bug.
              if (LI->Is8Bit != SI->Is8Bit) {
                Conflict = true;
                break;
              }
              // Stack must be balanced at each reload point.  Issue #74:
              // we no longer require a CALL between store/load -- the
              // StackDepth check is sufficient.
              if (StackDepth != 0) { Conflict = true; break; }
              LoadList.push_back({Scan, LI});
            }
          }

          // Branch/jump out of block = stop scanning.
          if (Scan->isTerminator()) break;
        }

        // Unbalanced PUSH/POP between store and last load = unsafe.
        if (StackDepth != 0) Conflict = true;

        if (Conflict || LoadList.empty()) continue;

        // Check that no other basic block references the same BSS address.
        // The PUSH/POP conversion is local to this block — if another block
        // loads from the same slot, it expects the value to be in BSS.
        {
          bool UsedElsewhere = false;
          for (MachineBasicBlock &OtherMBB : MF) {
            if (&OtherMBB == &MBB) continue;
            for (MachineInstr &OtherMI : OtherMBB) {
              unsigned Opc2 = OtherMI.getOpcode();
              if ((getStoreInfo(Opc2) || getLoadInfo(Opc2)) &&
                  sameAddress(*MII, OtherMI)) {
                UsedElsewhere = true;
                break;
              }
            }
            if (UsedElsewhere) break;
          }
          if (UsedElsewhere) continue;
        }

        // Cost computation:
        //   PUSH/POP code: 1B (initial PUSH) + N B (POPs)
        //                   + (N-1) B (re-PUSHes between consecutive loads)
        //                = 2N B.
        //   BSS code: SI->Bytes (store) + sum(LoadBytes) (loads).
        // Re-PUSH uses the LOAD's PUSH opcode (the value is now in that reg).
        unsigned PushPopBytes = 1 + LoadList.size();
        if (LoadList.size() > 1) PushPopBytes += LoadList.size() - 1;
        unsigned BssBytes = SI->Bytes;
        for (auto &LE : LoadList) BssBytes += LE.Info->Bytes;

        if (PushPopBytes >= BssBytes) continue; // not worth it

        // For PUSH AF / POP AF: POP AF restores flags, which may conflict
        // with flags set by intervening instructions.  Only safe when
        // FLAGS is dead after each POP AF.  This applies only when the
        // STORE was 8-bit (we PUSH AF) AND any LOAD is 8-bit (we POP AF).
        // For 16-bit cases, no FLAGS interaction.
        if (SI->Is8Bit) {
          bool FlagsSafe = true;
          for (auto &LE : LoadList) {
            auto After = std::next(LE.It);
            if (!isRegDeadAfter(After, MBB, TRI, Z80::FLAGS)) {
              FlagsSafe = false;
              break;
            }
          }
          if (!FlagsSafe)
            continue;
        }

        LLVM_DEBUG(dbgs() << "  BSS spill→PUSH/POP candidate: " << *MII
                          << "  " << LoadList.size() << " loads, saves "
                          << (BssBytes - PushPopBytes) << "B\n");

        Candidates.push_back({MII, SI, std::move(LoadList)});
      }

      // Apply candidates in reverse order so earlier conversions' inserted
      // PUSH/POP land OUTSIDE later conversions' brackets, preserving LIFO.
      // (For non-overlapping candidates the order doesn't matter.)
      for (auto It = Candidates.rbegin(); It != Candidates.rend(); ++It) {
        auto &C = *It;
        DebugLoc DL = C.StoreIt->getDebugLoc();
        BuildMI(MBB, *C.StoreIt, DL, TII->get(C.SI->PushOpc));
        MBB.erase(C.StoreIt);

        for (size_t i = 0; i < C.Loads.size(); ++i) {
          auto &LE = C.Loads[i];
          DebugLoc LoadDL = LE.It->getDebugLoc();
          BuildMI(MBB, *LE.It, LoadDL, TII->get(LE.Info->PopOpc));
          if (i + 1 < C.Loads.size()) {
            BuildMI(MBB, *LE.It, LoadDL, TII->get(LE.Info->PushOpc));
          }
          MBB.erase(LE.It);
        }
        Changed = true;
      }
    }
  }

  // --- Redundant PUSH AF/POP AF around BSS spill ---
  // When A already holds the source register's value, the SPILL_GR8 expansion
  // generates: LD A,r; PUSH AF; LD A,r; LD (addr),A; POP AF
  // The push/copy/pop is unnecessary — A already has the right value.
  // Collapse to: LD A,r; LD (addr),A  (saves 3B per instance).
  //
  // The cross-block #60 pass earlier in this run may have already removed
  // the second LD A,r (because A == r is established by the first one),
  // leaving the 4-instruction form: LD A,r; PUSH AF; LD (addr),A; POP AF.
  // We match both forms.
  for (MachineBasicBlock &MBB : MF) {
    for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
      auto I0 = MII++;
      // Match I0: LD A,<reg>
      Register SrcReg = getLDArSrcReg(I0->getOpcode());
      if (!SrcReg.isValid())
        continue;
      // Match I1: PUSH AF
      auto I1 = MII;
      if (I1 == MIE || I1->getOpcode() != Z80::PUSH_AF)
        continue;
      auto I2 = std::next(I1);
      if (I2 == MIE)
        continue;

      // Two accepted shapes:
      //   5-instr (original):  LD A,r; PUSH AF; LD A,r; LD (addr),A; POP AF
      //   4-instr (post #60):  LD A,r; PUSH AF;        LD (addr),A; POP AF
      //
      // In the 5-instr form I2 is the duplicate LD A,<same reg> and I3 is
      // LD (addr),A.  In the 4-instr form I2 is already LD (addr),A.
      MachineBasicBlock::iterator IDupLdAr = MIE;
      MachineBasicBlock::iterator IStore;
      if (I2->getOpcode() == I0->getOpcode()) {
        IDupLdAr = I2;
        IStore = std::next(I2);
        if (IStore == MIE)
          continue;
      } else {
        IStore = I2;
      }
      if (IStore->getOpcode() != Z80::LD_nnind_A)
        continue;

      auto I4 = std::next(IStore);
      if (I4 == MIE || I4->getOpcode() != Z80::POP_AF)
        continue;

      LLVM_DEBUG(dbgs() << "  BSS spill push/pop elim: " << *I0
                        << "  removing PUSH AF"
                        << (IDupLdAr != MIE ? " + LD A,r" : "")
                        << " + POP AF\n");
      MBB.erase(I1);
      if (IDupLdAr != MIE)
        MBB.erase(IDupLdAr);
      MII = MBB.erase(I4);
      Changed = true;
    }
  }

  // --- BSS load forwarding (static-stack mode) ---
  // Track values at absolute BSS addresses within each basic block.
  // Eliminates redundant loads when the value is already in a register.
  // Modeled on the IX-indexed store-to-load forwarding above (lines 2098-2184).
  // Handles both MCSymbol (__sfrend_*) and GlobalValue (C globals) operands.
  if (STI.staticStack()) {
    // Key: pointer to either MCSymbol or GlobalValue + offset.
    // MCSymbol and GlobalValue are allocated from different pools, so
    // pointer values never collide.
    using BSSKey = std::pair<const void *, int64_t>;
    DenseMap<BSSKey, MCPhysReg> BSSAvail;

    // Map opcode to the register involved in a BSS load/store.
    auto getBSSLoadDst = [](unsigned Opc) -> Register {
      switch (Opc) {
      case Z80::LD_A_nnind:  return Z80::A;
      case Z80::LD_HL_nnind: return Z80::HL;
      case Z80::LD_DE_nnind: return Z80::DE;
      case Z80::LD_BC_nnind: return Z80::BC;
      default: return Register();
      }
    };
    auto getBSSStoreSrc = [](unsigned Opc) -> Register {
      switch (Opc) {
      case Z80::LD_nnind_A:  return Z80::A;
      case Z80::LD_nnind_HL: return Z80::HL;
      case Z80::LD_nnind_DE: return Z80::DE;
      case Z80::LD_nnind_BC: return Z80::BC;
      default: return Register();
      }
    };

    auto getBSSKey = [](const MachineInstr &MI) -> BSSKey {
      const MachineOperand &MO = MI.getOperand(0);
      if (MO.isMCSymbol())
        return {MO.getMCSymbol(), MO.getOffset()};
      if (MO.isGlobal())
        return {MO.getGlobal(), MO.getOffset()};
      return {nullptr, 0};
    };

    // Check if the instruction has a volatile memoperand.
    auto isVolatileAccess = [](const MachineInstr &MI) -> bool {
      for (auto *MMO : MI.memoperands())
        if (MMO->isVolatile())
          return true;
      return false;
    };

    // Invalidate BSSAvail entries where the value register overlaps
    // with a clobbered register.
    auto invalidateBSSReg = [&](MCPhysReg ClobberedReg) {
      SmallVector<BSSKey, 4> ToErase;
      for (auto &KV : BSSAvail) {
        if (TRI->regsOverlap(KV.second, ClobberedReg))
          ToErase.push_back(KV.first);
      }
      for (auto &K : ToErase)
        BSSAvail.erase(K);
    };

    for (MachineBasicBlock &MBB : MF) {
      BSSAvail.clear();

      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
        MachineInstr &MI = *MII++;
        unsigned Opc = MI.getOpcode();

        // BSS store: track the value (skip volatile).
        Register StoreSrc = getBSSStoreSrc(Opc);
        if (StoreSrc.isValid()) {
          BSSKey Key = getBSSKey(MI);
          if (Key.first) {
            if (isVolatileAccess(MI))
              BSSAvail.erase(Key);
            else
              BSSAvail[Key] = StoreSrc;
          }
          continue;
        }

        // BSS load: check if value is already available (skip volatile).
        Register LoadDst = getBSSLoadDst(Opc);
        if (LoadDst.isValid()) {
          BSSKey Key = getBSSKey(MI);
          if (isVolatileAccess(MI)) {
            // Volatile load: don't forward, invalidate register.
            invalidateBSSReg(LoadDst);
            continue;
          }
          if (Key.first) {
            auto It = BSSAvail.find(Key);
            if (It != BSSAvail.end()) {
              MCPhysReg SrcReg = It->second;
              if (LoadDst == SrcReg) {
                // Value already in the correct register — eliminate load.
                LLVM_DEBUG(dbgs() << "  BSS: eliminating redundant load: "
                                  << MI);
                MI.eraseFromParent();
                Changed = true;
                continue;
              }
              // Value in a different register — try register copy.
              // Only for 8-bit (A) loads where we can use LD r,r'.
              unsigned NewOpc = getLD8Opcode(LoadDst, SrcReg);
              if (NewOpc) {
                LLVM_DEBUG(dbgs() << "  BSS: forwarding load to reg copy: "
                                  << MI);
                invalidateBSSReg(LoadDst);
                BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(NewOpc));
                MI.eraseFromParent();
                Changed = true;
                BSSAvail[Key] = LoadDst;
                continue;
              }
            }
          }
          // Couldn't forward — record the new value.
          invalidateBSSReg(LoadDst);
          if (Key.first)
            BSSAvail[Key] = LoadDst;
          continue;
        }

        // CALL or unmodeled side effects: invalidate everything.
        if (MI.isCall() || MI.hasUnmodeledSideEffects()) {
          BSSAvail.clear();
          continue;
        }

        // Any other instruction: invalidate entries for defined registers.
        for (const MachineOperand &MO : MI.operands()) {
          if (MO.isReg() && MO.isDef() && MO.getReg().isPhysical())
            invalidateBSSReg(MO.getReg());
        }
        for (MCPhysReg Def : TII->get(Opc).implicit_defs())
          invalidateBSSReg(Def);
      }
    }
  }

  // --- Pass: JP → JR branch shortening (issue #58) ---
  // Convert all JP Z/NZ/C/NC and unconditional JP to JR equivalents.
  // BranchRelaxation (which runs after this pass) will widen any JR
  // that can't reach its target back to JP. Net effect: branches that
  // fit in ±127 bytes become JR (2B), saving 1B each.
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      unsigned NewOpc = 0;
      switch (MI.getOpcode()) {
      case Z80::JP_nn:    NewOpc = Z80::JR_e; break;
      case Z80::JP_Z_nn:  NewOpc = Z80::JR_Z_e; break;
      case Z80::JP_NZ_nn: NewOpc = Z80::JR_NZ_e; break;
      case Z80::JP_C_nn:  NewOpc = Z80::JR_C_e; break;
      case Z80::JP_NC_nn: NewOpc = Z80::JR_NC_e; break;
      default: continue;
      }
      // Only convert MBB-target branches (not symbol/immediate targets).
      if (!MI.getOperand(0).isMBB()) continue;
      LLVM_DEBUG(dbgs() << "  JP→JR shortening: " << MI);
      MI.setDesc(TII->get(NewOpc));
      Changed = true;
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
