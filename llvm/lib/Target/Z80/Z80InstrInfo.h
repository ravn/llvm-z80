//===-- Z80InstrInfo.h - Z80 Instruction Information ------------*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Z80 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80INSTRINFO_H
#define LLVM_LIB_TARGET_Z80_Z80INSTRINFO_H

#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "Z80GenInstrInfo.inc"

namespace llvm {

class Z80Subtarget;

namespace Z80 {
inline void markUndefUse(const MachineInstrBuilder &MIB, MCRegister Reg) {
  for (MachineOperand &MO : MIB.getInstr()->operands())
    if (MO.isReg() && MO.isUse() && MO.getReg() == Reg)
      MO.setIsUndef();
}

inline void emitHLSavePush(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator InsertBefore,
                           const DebugLoc &DL, const TargetInstrInfo &TII) {
  const TargetRegisterInfo *TRI =
      MBB.getParent()->getSubtarget().getRegisterInfo();
  MachineInstrBuilder Push =
      BuildMI(MBB, InsertBefore, DL, TII.get(Z80::PUSH_HL));
  for (MachineBasicBlock::iterator I = Push->getIterator();
       I != MBB.begin();) {
    --I;
    bool Touched = false;
    for (MachineOperand &MO : I->operands()) {
      if (!MO.isReg() || !MO.getReg().isValid() ||
          !TRI->regsOverlap(MO.getReg(), Z80::HL))
        continue;
      Touched = true;
      if (MO.isDef())
        MO.setIsDead(false);
      else if (MO.isKill())
        MO.setIsKill(false);
    }
    if (Touched)
      return;
  }
  if (!MBB.isLiveIn(Z80::HL) && !MBB.isLiveIn(Z80::L) &&
      !MBB.isLiveIn(Z80::H))
    markUndefUse(Push, Z80::HL);
}
} // namespace Z80

class Z80InstrInfo : public Z80GenInstrInfo {
public:
  Z80InstrInfo(const Z80Subtarget &STI);

  Register isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;

  Register isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  void storeRegToStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
      bool isKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void loadRegFromStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register DestReg,
      int FrameIndex, const TargetRegisterClass *RC, Register VReg,
      unsigned SubReg = 0,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;

  bool expandPostRAPseudo(MachineInstr &MI) const override;

  int getSPAdjust(const MachineInstr &MI) const override;

  unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

  bool isBranchOffsetInRange(unsigned BranchOpc,
                             int64_t BrOffset) const override;

  MachineBasicBlock *getBranchDestBlock(const MachineInstr &MI) const override;

  void insertIndirectBranch(MachineBasicBlock &MBB,
                            MachineBasicBlock &NewDestBB,
                            MachineBasicBlock &RestoreBB, const DebugLoc &DL,
                            int64_t BrOffset = 0,
                            RegScavenger *RS = nullptr) const override;

  // ravn/llvm-z80#23 Phase 1 (2026-06-08): cost-query hooks for the
  // cost-model refinement plan in
  // tasks/plan-z80-cost-model-refinement-2026-06-08.md.
  //
  // These hooks add NEW Z80-specific cost queries that future passes
  // (Phase 3) will consult to make hoist / CSE / spill decisions
  // aware of Z80's tiered register file.  Adding them is codegen-
  // neutral until Phase 3 consumers light up; the
  // `-z80-use-tiered-cost-model` cl::opt (default OFF) gates them.
  //
  // The defaults below match the values existing infrastructure
  // already implicitly assumes, so the no-op-control measurement
  // (see [[feedback_no_op_control_measurement]] memory rule) should
  // be byte-identical to the pre-Phase-1 baseline.

  /// Estimated size-in-bytes cost of rematerializing MI at a use
  /// site.  Default: getInstSizeInBytes(MI).  A "1-byte rematable"
  /// (e.g. `XOR A` to set A=0) is much cheaper than reloading from
  /// BSS (3 B); a "3-byte rematable" (e.g. `LD HL,#imm`) costs the
  /// same as a BSS reload, so the existing
  /// `MachineLICM::isTriviallyReMaterializable` bypass overstates
  /// its benefit in some cases.  Phase 3 will consult this in a
  /// cost-aware version of the bypass.
  unsigned getRematCost(const MachineInstr &MI) const;

  /// Which mechanism the regalloc would use to spill a value of
  /// register class RC across a code region.  Phase 3 will use
  /// this to let the regalloc pick the cheapest available
  /// mechanism rather than always reaching for BSS.
  enum class Z80SpillKind { BSS, PushPop, IXIYIndex };

  /// Estimated size-in-bytes cost of one spill+reload pair for
  /// register class RC via the given spill mechanism.
  /// Conservative defaults (Phase 1):
  ///   BSS:       3 B (LD A,(nn) or LD HL,(nn))
  ///   PushPop:   2 B (PUSH rr + POP rr)
  ///   IXIYIndex: 3 B (LD A,(IX+0) with FD/DD prefix)
  /// Phase 3 will widen this to a (bytes, tstates) tuple.
  unsigned getSpillCost(const TargetRegisterClass *RC,
                        Z80SpillKind Kind) const;

  /// Returns whether the Phase 1 cost-query hooks are currently
  /// enabled (cl::opt -z80-use-tiered-cost-model).  Convenience
  /// wrapper so consumers don't have to reach for the flag
  /// directly.  Default false.
  bool useTieredCostModel() const;

  /// ravn/llvm-z80#23 Phase 3 (2026-06-08): cost-aware hoist veto.
  /// MachineLICM consults this via TII->shouldHoist().  When the
  /// tiered cost model is enabled, refuse to hoist a rematable
  /// instruction out of a loop whose body contains a CALL --
  /// sdcccall(1) clobbers HL/DE/BC, so a hoisted vreg's live range
  /// crossing the call will spill to BSS, costing 6 B per save+reload
  /// pair vs the remat's natural cost (typically 2-3 B per use).
  ///
  /// Caveat: autoload-in-c's worst regression (define_sextants nested
  /// loops) is LEAF -- no CALL in body -- so this heuristic doesn't
  /// catch it.  Phase 3 chapter 2 will need a register-pressure-
  /// aware extension (e.g. "refuse cheap remats when the loop has
  /// already accumulated N CSE-deduplicated rematables").  This
  /// chapter ships the call-aware piece as foundation.
  bool shouldHoist(const MachineInstr &MI,
                   const MachineLoop *FromLoop) const override;

private:
  const Z80Subtarget *STI;
};

namespace Z80 {

enum AddressSpace : unsigned { AS_Memory = 0, AS_IO = 2, NumAddrSpaces };

enum TOF {
  MO_NO_FLAGS = 0,
  MO_LO,
  MO_ADDR16_LO = MO_LO,
  MO_HI,
  MO_ADDR16_HI = MO_HI,
  MO_HI_JT,
};

} // namespace Z80

} // namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80INSTRINFO_H
