//===-- Z80RegisterInfo.h - Z80 Register Information Impl -------*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Z80 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80REGISTERINFO_H
#define LLVM_LIB_TARGET_Z80_Z80REGISTERINFO_H

#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "Z80GenRegisterInfo.inc"

namespace llvm {

class Z80Subtarget;

class Z80RegisterInfo : public Z80GenRegisterInfo {
public:
  Z80RegisterInfo();

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  const TargetRegisterClass *
  getLargestLegalSuperClass(const TargetRegisterClass *RC,
                            const MachineFunction &) const override;

  bool requiresRegisterScavenging(const MachineFunction &MF) const override {
    return true;
  }

  bool saveScavengerRegister(MachineBasicBlock &MBB,
                             MachineBasicBlock::iterator I,
                             MachineBasicBlock::iterator &UseMI,
                             const TargetRegisterClass *RC,
                             Register Reg) const override;

  // Use forward frame index replacement so PEI tracks per-instruction SPAdj
  // inside call sequences (via InsideCallSequence). The backward walk lacks
  // this tracking, causing incorrect offsets when PUSHes shift SP.
  bool eliminateFrameIndicesBackwards() const override { return false; }

  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;

  bool getRegAllocationHints(Register VirtReg, ArrayRef<MCPhysReg> Order,
                             SmallVectorImpl<MCPhysReg> &Hints,
                             const MachineFunction &MF,
                             const VirtRegMap *VRM = nullptr,
                             const LiveRegMatrix *Matrix = nullptr) const override;

  // Return the name of a register for inline assembly
  StringRef getRegAsmName(MCRegister Reg) const override;

  // ravn/llvm-z80#23 Phase 2 (2026-06-08): override the auto-generated
  // GR16 pressure limit to reflect Z80's practical 3-pair budget
  // (HL/DE/BC) for short-lived values.  TableGen reports 12 register
  // units for GR16 (counting IX/IY/AF), but regalloc empirically uses
  // only HL/DE/BC for short-lived hoist candidates -- IX/IY incur a
  // 1-byte FD/DD prefix per use.  When MachineLICM consults the
  // higher limit, it over-hoists invariants that regalloc then
  // spills to BSS (the autoload-in-c witness; AES gets the benefit
  // because its leaf-loop hoists DO fit in HL).
  //
  // Gated by `-mllvm -z80-tiered-gr16-pressure` (default ON, since
  // measurement shows AES -18 B at -Oz with no production regressions).
  // Set OFF to restore pre-Phase-2 behavior.
  //
  // Path not taken (Phase 2 alternative considered): full TableGen
  // sub-class reshuffle (GR16NoIR / IR16 each its own pressure set).
  // TableGen merges pressure sets when one class is a strict subset
  // of another, and GR16NoIR ⊂ GR16, so generating separate sets
  // requires breaking that subset relationship -- invasive change to
  // ISel patterns.  This override is the simpler "tell LICM what
  // regalloc actually does" route; revisit the structural change if
  // measurement shows it needed.
  unsigned getRegPressureSetLimit(const MachineFunction &MF,
                                  unsigned Idx) const override;
};

} // namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80REGISTERINFO_H
