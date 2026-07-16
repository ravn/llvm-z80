//===-- Z80FrameLowering.h - Define frame lowering for Z80 ------*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Z80 declaration of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80FRAMELOWERING_H
#define LLVM_LIB_TARGET_Z80_Z80FRAMELOWERING_H

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class Z80FrameLowering : public TargetFrameLowering {
public:
  Z80FrameLowering();

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;

  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;
  // Ensure replaceFrameIndices runs even when there are no stack objects,
  // so that ADJCALLSTACKDOWN/UP pseudos are always eliminated.
  bool needsFrameIndexResolution(const MachineFunction &MF) const override {
    return MF.getFrameInfo().hasStackObjects() ||
           MF.getFrameInfo().adjustsStack();
  }

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

private:
  bool hasFPImpl(const MachineFunction &MF) const override;
};

// Classify the expansion path that eliminateCallFramePseudoInstr will choose
// for an ADJCALLSTACKUP with the given net adjustment and SM83 flag.
// adjCallStackUpClobbersReg (Z80RegisterInfo.cpp) must use the same logic to
// determine which registers the pseudo will modify; sharing this helper keeps
// the two sides in sync.
//
// Paths:
//   ACSU_Erase   - amount == 0: pseudo is erased, no register effects.
//   ACSU_AddSPe  - SM83 + amount <= 127: ADD SP,e (clobbers only SP/flags).
//   ACSU_PopAF   - amount <= 16: POP AF × N (clobbers A/flags, not HL).
//   ACSU_AddHLSP - amount > 16: LD HL,n; ADD HL,SP; LD SP,HL (clobbers HL/flags).
enum AdjCallStackUpPath { ACSU_Erase, ACSU_AddSPe, ACSU_PopAF, ACSU_AddHLSP };

inline AdjCallStackUpPath classifyACSU(int64_t Amount, bool IsSM83) {
  if (Amount == 0)
    return ACSU_Erase;
  if (IsSM83 && Amount <= 127)
    return ACSU_AddSPe;
  if (Amount <= 16)
    return ACSU_PopAF;
  return ACSU_AddHLSP;
}

} // namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80FRAMELOWERING_H
