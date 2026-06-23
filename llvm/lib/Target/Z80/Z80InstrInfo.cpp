//===-- Z80InstrInfo.cpp - Z80 Instruction Information --------------------===//
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

#include "Z80InstrInfo.h"

#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80MachineFunctionInfo.h"
#include "Z80OpcodeUtils.h"
#include "Z80RegisterInfo.h"
#include "Z80Subtarget.h"

#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "z80-instrinfo"

#define GET_INSTRINFO_CTOR_DTOR
#include "Z80GenInstrInfo.inc"

Z80InstrInfo::Z80InstrInfo(const Z80Subtarget &STI)
    : Z80GenInstrInfo(STI, *STI.getRegisterInfo(),
                      /*CFSetupOpcode=*/Z80::ADJCALLSTACKDOWN,
                      /*CFDestroyOpcode=*/Z80::ADJCALLSTACKUP),
      STI(&STI) {}

Register Z80InstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                           int &FrameIndex) const {
  switch (MI.getOpcode()) {
  case Z80::RELOAD_GR8:
  case Z80::RELOAD_GR16:
    if (MI.getOperand(1).isFI()) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
    break;
  }
  return 0;
}

Register Z80InstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                          int &FrameIndex) const {
  switch (MI.getOpcode()) {
  case Z80::SPILL_GR8:
  case Z80::SPILL_GR16:
    if (MI.getOperand(1).isFI()) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
    break;
  }
  return 0;
}

// Get opcode for LD r,n (load immediate to 8-bit register)
static unsigned getLoadImmR8Opcode(Register Reg) {
  static const unsigned Opcodes[] = {Z80::LD_A_n, Z80::LD_B_n, Z80::LD_C_n,
                                     Z80::LD_D_n, Z80::LD_E_n, Z80::LD_H_n,
                                     Z80::LD_L_n};
  int Idx = Z80::gr8RegToIndex(Reg);
  if (Idx >= 0) return Opcodes[Idx];
  // Undocumented: LD IXH/IXL/IYH/IYL,n
  switch (Reg.id()) {
  case Z80::IXH: return Z80::LD_IXH_n;
  case Z80::IXL: return Z80::LD_IXL_n;
  case Z80::IYH: return Z80::LD_IYH_n;
  case Z80::IYL: return Z80::LD_IYL_n;
  default: return 0;
  }
}

static unsigned getSUBOpcode(Register Reg) {
  static const unsigned T[] = {Z80::SUB_A, Z80::SUB_B, Z80::SUB_C, Z80::SUB_D,
                               Z80::SUB_E, Z80::SUB_H, Z80::SUB_L};
  int I = Z80::gr8RegToIndex(Reg);
  if (I >= 0) return T[I];
  switch (Reg.id()) {
  case Z80::IXH: return Z80::SUB_IXH; case Z80::IXL: return Z80::SUB_IXL;
  case Z80::IYH: return Z80::SUB_IYH; case Z80::IYL: return Z80::SUB_IYL;
  default: return 0;
  }
}

static unsigned getSBCOpcode(Register Reg) {
  static const unsigned T[] = {Z80::SBC_A_A, Z80::SBC_A_B, Z80::SBC_A_C,
                               Z80::SBC_A_D, Z80::SBC_A_E, Z80::SBC_A_H,
                               Z80::SBC_A_L};
  int I = Z80::gr8RegToIndex(Reg);
  if (I >= 0) return T[I];
  switch (Reg.id()) {
  case Z80::IXH: return Z80::SBC_A_IXH; case Z80::IXL: return Z80::SBC_A_IXL;
  case Z80::IYH: return Z80::SBC_A_IYH; case Z80::IYL: return Z80::SBC_A_IYL;
  default: return 0;
  }
}

static unsigned getADCOpcode(Register Reg) {
  static const unsigned T[] = {Z80::ADC_A_A, Z80::ADC_A_B, Z80::ADC_A_C,
                               Z80::ADC_A_D, Z80::ADC_A_E, Z80::ADC_A_H,
                               Z80::ADC_A_L};
  int I = Z80::gr8RegToIndex(Reg);
  if (I >= 0) return T[I];
  switch (Reg.id()) {
  case Z80::IXH: return Z80::ADC_A_IXH; case Z80::IXL: return Z80::ADC_A_IXL;
  case Z80::IYH: return Z80::ADC_A_IYH; case Z80::IYL: return Z80::ADC_A_IYL;
  default: return 0;
  }
}

static unsigned getADD8Opcode(Register Reg) {
  static const unsigned T[] = {Z80::ADD_A_A, Z80::ADD_A_B, Z80::ADD_A_C,
                               Z80::ADD_A_D, Z80::ADD_A_E, Z80::ADD_A_H,
                               Z80::ADD_A_L};
  int I = Z80::gr8RegToIndex(Reg);
  if (I >= 0) return T[I];
  switch (Reg.id()) {
  case Z80::IXH: return Z80::ADD_A_IXH; case Z80::IXL: return Z80::ADD_A_IXL;
  case Z80::IYH: return Z80::ADD_A_IYH; case Z80::IYL: return Z80::ADD_A_IYL;
  default: return 0;
  }
}

static unsigned getXOROpcode(Register Reg) {
  static const unsigned T[] = {Z80::XOR_A, Z80::XOR_B, Z80::XOR_C, Z80::XOR_D,
                               Z80::XOR_E, Z80::XOR_H, Z80::XOR_L};
  int I = Z80::gr8RegToIndex(Reg);
  if (I >= 0) return T[I];
  switch (Reg.id()) {
  case Z80::IXH: return Z80::XOR_IXH; case Z80::IXL: return Z80::XOR_IXL;
  case Z80::IYH: return Z80::XOR_IYH; case Z80::IYL: return Z80::XOR_IYL;
  default: return 0;
  }
}

static unsigned getOROpcode(Register Reg) {
  static const unsigned T[] = {Z80::OR_A, Z80::OR_B, Z80::OR_C, Z80::OR_D,
                               Z80::OR_E, Z80::OR_H, Z80::OR_L};
  int I = Z80::gr8RegToIndex(Reg);
  if (I >= 0) return T[I];
  switch (Reg.id()) {
  case Z80::IXH: return Z80::OR_IXH; case Z80::IXL: return Z80::OR_IXL;
  case Z80::IYH: return Z80::OR_IYH; case Z80::IYL: return Z80::OR_IYL;
  default: return 0;
  }
}

static unsigned getCPOpcode(Register Reg) {
  static const unsigned T[] = {Z80::CP_A, Z80::CP_B, Z80::CP_C, Z80::CP_D,
                               Z80::CP_E, Z80::CP_H, Z80::CP_L};
  int I = Z80::gr8RegToIndex(Reg);
  if (I >= 0) return T[I];
  switch (Reg.id()) {
  case Z80::IXH: return Z80::CP_IXH; case Z80::IXL: return Z80::CP_IXL;
  case Z80::IYH: return Z80::CP_IYH; case Z80::IYL: return Z80::CP_IYL;
  default: return 0;
  }
}

static unsigned getSRLOpcode(Register Reg) {
  static const unsigned T[] = {Z80::SRL_A, Z80::SRL_B, Z80::SRL_C, Z80::SRL_D,
                               Z80::SRL_E, Z80::SRL_H, Z80::SRL_L};
  int I = Z80::gr8RegToIndex(Reg);
  return I >= 0 ? T[I] : 0;
}

static unsigned getSRAOpcode(Register Reg) {
  static const unsigned T[] = {Z80::SRA_A, Z80::SRA_B, Z80::SRA_C, Z80::SRA_D,
                               Z80::SRA_E, Z80::SRA_H, Z80::SRA_L};
  int I = Z80::gr8RegToIndex(Reg);
  return I >= 0 ? T[I] : 0;
}

static unsigned getRROpcode(Register Reg) {
  static const unsigned T[] = {Z80::RR_A, Z80::RR_B, Z80::RR_C, Z80::RR_D,
                               Z80::RR_E, Z80::RR_H, Z80::RR_L};
  int I = Z80::gr8RegToIndex(Reg);
  return I >= 0 ? T[I] : 0;
}

// Get low and high 8-bit sub-registers of a 16-bit register pair.
// Includes undocumented IX/IY halves for +undocumented target.
static std::pair<Register, Register> getSubRegs16(Register Reg) {
  switch (Reg.id()) {
  case Z80::BC:
    return {Z80::C, Z80::B};
  case Z80::DE:
    return {Z80::E, Z80::D};
  case Z80::HL:
    return {Z80::L, Z80::H};
  case Z80::IX:
    return {Z80::IXL, Z80::IXH};
  case Z80::IY:
    return {Z80::IYL, Z80::IYH};
  default:
    llvm_unreachable("Not a GR16 register pair");
  }
}

// Get PUSH opcode for a 16-bit register
static unsigned getPUSHOpcode(Register Reg) { return Z80::getPushOpcode(Reg); }

static unsigned getPOPOpcode(Register Reg) { return Z80::getPopOpcode(Reg); }

void Z80InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator I,
                               const DebugLoc &DL, Register DestReg,
                               Register SrcReg, bool KillSrc,
                               bool RenamableDest, bool RenamableSrc) const {
  // Handle 8-bit register copies: LD r,r'
  if (Z80::GR8RegClass.contains(DestReg) && Z80::GR8RegClass.contains(SrcReg)) {
    unsigned Opcode = Z80::getLD8RegOpcode(DestReg, SrcReg);
    if (Opcode) {
      BuildMI(MBB, I, DL, get(Opcode));
      return;
    }
  }

  // EX DE,HL: single-byte swap for DE<->HL copies (Z80 only).
  // Note: EX DE,HL swaps both registers, so the source is also modified.
  // This is safe when the source is dead after this copy.
  // SM83 lacks EX DE,HL; fall through to the 2x LD path below.
  if (!STI->hasSM83() && ((DestReg == Z80::DE && SrcReg == Z80::HL) ||
                          (DestReg == Z80::HL && SrcReg == Z80::DE))) {
    bool SrcDead = KillSrc;
    if (!SrcDead) {
      // KillSrc may not be set for physical register liveins.
      // Check actual liveness after this point.
      auto Next = std::next(I);
      auto LQR =
          MBB.computeRegisterLiveness(STI->getRegisterInfo(), SrcReg, Next);
      SrcDead = (LQR == MachineBasicBlock::LQR_Dead);
    }
    if (SrcDead) {
      MachineInstr *MI = BuildMI(MBB, I, DL, get(Z80::EX_DE_HL)).getInstr();
      // EX DE,HL swaps both pairs.  As a one-way copy SrcReg -> DestReg it
      // moves DestReg's OLD value into SrcReg, which is dead here and
      // discarded.  A copy overwrites its destination, so DestReg's incoming
      // value is a don't-care; mark the implicit use of DestReg undef.
      // Otherwise, when DestReg's old value is itself undefined (e.g. a fresh
      // pair used only as the copy target), -verify-machineinstrs flags the EX
      // with "Using an undefined physical register" (#209-class don't-care
      // read; AES rj_sb_inv).  The SrcReg use stays real (it is the value
      // being copied).
      for (MachineOperand &MO : MI->operands())
        if (MO.isReg() && MO.isUse() && MO.getReg() == DestReg)
          MO.setIsUndef(true);
      return;
    }
  }

  // Handle 16-bit register copies between BC, DE, HL using two 8-bit LDs.
  // LD r,r' is 1 byte / 4 cycles each (2 bytes / 8 cycles total),
  // much faster than PUSH/POP (2 bytes / 21 cycles).
  // NOTE: IX/IY copies also try this path first. With +undocumented,
  // LD E,IXL (2B) + LD D,IXH (2B) = 4B, vs PUSH/POP = 3B. The LD
  // path is used because PUSH/POP modifies SP, which can break
  // SP-relative addressing and interrupt-sensitive contexts.
  if (Z80::GR16RegClass.contains(DestReg) &&
      Z80::GR16RegClass.contains(SrcReg)) {
    const TargetRegisterInfo *TRI = STI->getRegisterInfo();
    Register DstLo = TRI->getSubReg(DestReg, Z80::sub_lo);
    Register DstHi = TRI->getSubReg(DestReg, Z80::sub_hi);
    Register SrcLo = TRI->getSubReg(SrcReg, Z80::sub_lo);
    Register SrcHi = TRI->getSubReg(SrcReg, Z80::sub_hi);
    if (DstLo && DstHi && SrcLo && SrcHi) {
      // Without +undocumented, skip 8-bit LD for IX/IY sub-registers
      // (IXH/IXL/IYH/IYL ops are undocumented).  Fall through to PUSH/POP.
      bool NeedsUndoc = Z80::IR16RegClass.contains(DestReg) ||
                         Z80::IR16RegClass.contains(SrcReg);
      if (!NeedsUndoc || STI->hasUndocumented()) {
        unsigned LoOp = Z80::getLD8RegOpcode(DstLo, SrcLo);
        unsigned HiOp = Z80::getLD8RegOpcode(DstHi, SrcHi);
        if (LoOp && HiOp) {
          BuildMI(MBB, I, DL, get(LoOp));
          BuildMI(MBB, I, DL, get(HiOp));
          return;
        }
      }
    }
  }

  // Handle 16-bit register copies involving IX/IY using PUSH/POP sequence.
  // PUSH src; POP dest = 3B.  Used as fallback when the 8-bit LD path above
  // didn't fire (e.g. IX/IY sub-register LD failed to find valid opcodes),
  // and also for documented-only IX/IY copies without +undocumented.
  if ((Z80::GR16RegClass.contains(DestReg) ||
       Z80::IR16RegClass.contains(DestReg)) &&
      (Z80::GR16RegClass.contains(SrcReg) ||
       Z80::IR16RegClass.contains(SrcReg))) {
    // When IX/IY is involved without +undocumented, use COPY16_PUSHPOP pseudo
    // to keep the PUSH/POP as a single MI.  This prevents optimization passes
    // from inserting between them, avoiding SP corruption (issue #32).
    // The pseudo expands to adjacent PUSH/POP in Z80ExpandPseudo.
    if (!STI->hasUndocumented() &&
        (Z80::IR16RegClass.contains(DestReg) ||
         Z80::IR16RegClass.contains(SrcReg))) {
      BuildMI(MBB, I, DL, get(Z80::COPY16_PUSHPOP), DestReg)
          .addReg(SrcReg, getKillRegState(KillSrc));
      return;
    }
    unsigned PushOp = getPUSHOpcode(SrcReg);
    unsigned PopOp = getPOPOpcode(DestReg);
    if (PushOp && PopOp) {
      BuildMI(MBB, I, DL, get(PushOp));
      BuildMI(MBB, I, DL, get(PopOp));
      return;
    }
  }

  // Handle SP loads from HL, IX, IY
  if (DestReg == Z80::SP) {
    switch (SrcReg.id()) {
    case Z80::HL:
      BuildMI(MBB, I, DL, get(Z80::LD_SP_HL));
      return;
    case Z80::IX:
      BuildMI(MBB, I, DL, get(Z80::LD_SP_IX));
      return;
    case Z80::IY:
      BuildMI(MBB, I, DL, get(Z80::LD_SP_IY));
      return;
    default:
      // BC/DE → SP: route through HL (LD HL,src; LD SP,HL)
      // HL is clobbered, but stackrestore is typically at scope exit
      // where HL is dead.
      copyPhysReg(MBB, I, DL, Z80::HL, SrcReg, KillSrc);
      BuildMI(MBB, I, DL, get(Z80::LD_SP_HL));
      return;
    }
  }

  // Handle reading SP into HL, IX, IY
  // Z80 has no "LD HL,SP" so we use "LD reg,0; ADD reg,SP"
  // ADD rr,SP clobbers FLAGS (carry). If FLAGS is live, we wrap with
  // PUSH AF/POP AF and compensate the offset (+2) for the changed SP.
  if (SrcReg == Z80::SP) {
    const TargetRegisterInfo *TRI = STI->getRegisterInfo();
    auto FlagsLQ = MBB.computeRegisterLiveness(TRI, Z80::FLAGS, I);
    bool FlagsLive = (FlagsLQ != MachineBasicBlock::LQR_Dead);
    int SPComp = FlagsLive ? 2 : 0; // SP compensation for PUSH AF

    if (DestReg == Z80::HL) {
      if (FlagsLive)
        BuildMI(MBB, I, DL, get(Z80::PUSH_AF));
      BuildMI(MBB, I, DL, get(Z80::LD_HL_nn)).addImm(SPComp);
      BuildMI(MBB, I, DL, get(Z80::ADD_HL_SP));
      if (FlagsLive)
        BuildMI(MBB, I, DL, get(Z80::POP_AF));
      return;
    }
    if (DestReg == Z80::IX) {
      if (FlagsLive)
        BuildMI(MBB, I, DL, get(Z80::PUSH_AF));
      BuildMI(MBB, I, DL, get(Z80::LD_IX_nn)).addImm(SPComp);
      BuildMI(MBB, I, DL, get(Z80::ADD_IX_SP));
      if (FlagsLive)
        BuildMI(MBB, I, DL, get(Z80::POP_AF));
      return;
    }
    if (DestReg == Z80::IY) {
      if (FlagsLive)
        BuildMI(MBB, I, DL, get(Z80::PUSH_AF));
      BuildMI(MBB, I, DL, get(Z80::LD_IY_nn)).addImm(SPComp);
      BuildMI(MBB, I, DL, get(Z80::ADD_IY_SP));
      if (FlagsLive)
        BuildMI(MBB, I, DL, get(Z80::POP_AF));
      return;
    }
    // SP → BC or DE: Z80 has no ADD BC,SP / ADD DE,SP, so route through HL.
    // PUSH HL; LD HL,N; ADD HL,SP; LD r,H; LD r,L; POP HL
    // N compensates for PUSH HL (and PUSH AF if FLAGS is live).
    if (DestReg == Z80::BC || DestReg == Z80::DE) {
      unsigned LdHiOp = (DestReg == Z80::BC) ? Z80::LD_B_H : Z80::LD_D_H;
      unsigned LdLoOp = (DestReg == Z80::BC) ? Z80::LD_C_L : Z80::LD_E_L;
      // Only PUSH_HL when HL holds a value that needs preserving.  If either
      // half is undef, IMPLICIT_DEF it before PUSH_HL so the pair PUSH does
      // not read partial-undef (ravn/llvm-z80#239 site 6).
      auto HLQ = MBB.computeRegisterLiveness(TRI, Z80::H, I);
      auto LLQ = MBB.computeRegisterLiveness(TRI, Z80::L, I);
      bool HLive = (HLQ != MachineBasicBlock::LQR_Dead);
      bool LLive = (LLQ != MachineBasicBlock::LQR_Dead);
      bool NeedSaveHL = HLive || LLive;
      int HLComp = NeedSaveHL ? 2 : 0;
      if (FlagsLive)
        BuildMI(MBB, I, DL, get(Z80::PUSH_AF));
      if (NeedSaveHL) {
        if (!HLive)
          BuildMI(MBB, I, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::H);
        if (!LLive)
          BuildMI(MBB, I, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::L);
        BuildMI(MBB, I, DL, get(Z80::PUSH_HL));
      }
      BuildMI(MBB, I, DL, get(Z80::LD_HL_nn)).addImm(SPComp + HLComp);
      BuildMI(MBB, I, DL, get(Z80::ADD_HL_SP));
      BuildMI(MBB, I, DL, get(LdHiOp));
      BuildMI(MBB, I, DL, get(LdLoOp));
      if (NeedSaveHL)
        BuildMI(MBB, I, DL, get(Z80::POP_HL));
      if (FlagsLive)
        BuildMI(MBB, I, DL, get(Z80::POP_AF));
      return;
    }
  }

  // Handle 8-bit copies FROM IXH/IXL/IYH/IYL to a GR8 register.
  // With +undocumented: use direct LD (LD dest,IXL etc.) — 2 bytes, no SP change.
  // Without: route through PUSH IX/IY; POP HL to extract the byte.
  {
    bool SrcIsIXH = (SrcReg == Z80::IXH), SrcIsIXL = (SrcReg == Z80::IXL);
    bool SrcIsIYH = (SrcReg == Z80::IYH), SrcIsIYL = (SrcReg == Z80::IYL);
    bool SrcIsIndexHi = SrcIsIXH || SrcIsIYH;
    bool SrcIsIndexLo = SrcIsIXL || SrcIsIYL;

    if ((SrcIsIndexHi || SrcIsIndexLo) && Z80::GR8RegClass.contains(DestReg)) {
      // Try direct undocumented LD if available (works for A,B,C,D,E)
      if (STI->hasUndocumented()) {
        unsigned DirectOp = Z80::getLD8RegOpcode(DestReg, SrcReg);
        if (DirectOp) {
          BuildMI(MBB, I, DL, get(DirectOp));
          return;
        }
      }

      unsigned PushOp = (SrcIsIXH || SrcIsIXL) ? Z80::PUSH_IX : Z80::PUSH_IY;
      Register ExtractReg = SrcIsIndexHi ? Z80::H : Z80::L;

      if (DestReg == Z80::H || DestReg == Z80::L) {
        Register OtherReg = (DestReg == Z80::H) ? Z80::L : Z80::H;
        const TargetRegisterInfo *TRI = STI->getRegisterInfo();
        auto OtherLQ = MBB.computeRegisterLiveness(TRI, OtherReg, I);

        if (OtherLQ == MachineBasicBlock::LQR_Dead) {
          BuildMI(MBB, I, DL, get(PushOp));
          BuildMI(MBB, I, DL, get(Z80::POP_HL));
          if ((DestReg == Z80::H && SrcIsIndexLo) ||
              (DestReg == Z80::L && SrcIsIndexHi)) {
            unsigned LdOp = Z80::getLD8RegOpcode(DestReg, ExtractReg);
            BuildMI(MBB, I, DL, get(LdOp));
          }
        } else {
          Register ScratchReg = SrcIsIndexHi ? Z80::D : Z80::E;
          BuildMI(MBB, I, DL, get(Z80::PUSH_DE));
          BuildMI(MBB, I, DL, get(PushOp));
          BuildMI(MBB, I, DL, get(Z80::POP_DE));
          unsigned LdOp = Z80::getLD8RegOpcode(DestReg, ScratchReg);
          BuildMI(MBB, I, DL, get(LdOp));
          BuildMI(MBB, I, DL, get(Z80::POP_DE));
        }
      } else {
        // Dest is not H/L: extract IX/IY half via PUSH IX; POP HL; LD dest,H/L.
        // Gate the HL save/restore on liveness; IMPLICIT_DEF dead half before
        // PUSH_HL so the pair PUSH does not read partial-undef (#239 site 5a).
        const TargetRegisterInfo *TRI5a = STI->getRegisterInfo();
        auto HLQ5a = MBB.computeRegisterLiveness(TRI5a, Z80::H, I);
        auto LLQ5a = MBB.computeRegisterLiveness(TRI5a, Z80::L, I);
        bool HLive5a = (HLQ5a != MachineBasicBlock::LQR_Dead);
        bool LLive5a = (LLQ5a != MachineBasicBlock::LQR_Dead);
        bool NeedSaveHL5a = HLive5a || LLive5a;
        if (NeedSaveHL5a) {
          if (!HLive5a)
            BuildMI(MBB, I, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::H);
          if (!LLive5a)
            BuildMI(MBB, I, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::L);
          BuildMI(MBB, I, DL, get(Z80::PUSH_HL));
        }
        BuildMI(MBB, I, DL, get(PushOp));
        BuildMI(MBB, I, DL, get(Z80::POP_HL));
        unsigned LdOp = Z80::getLD8RegOpcode(DestReg, ExtractReg);
        BuildMI(MBB, I, DL, get(LdOp));
        if (NeedSaveHL5a)
          BuildMI(MBB, I, DL, get(Z80::POP_HL));
      }
      return;
    }
  }

  // Handle 8-bit copies TO IXH/IXL/IYH/IYL from a GR8 register.
  // With +undocumented: use direct LD (LD IXL,src etc.) — 2 bytes, no SP change.
  // Without: route through HL: save HL, PUSH IX/IY; POP HL, modify, PUSH HL; POP IX/IY.
  {
    bool DstIsIXH = (DestReg == Z80::IXH), DstIsIXL = (DestReg == Z80::IXL);
    bool DstIsIYH = (DestReg == Z80::IYH), DstIsIYL = (DestReg == Z80::IYL);
    bool DstIsIndexHi = DstIsIXH || DstIsIYH;
    bool DstIsIndexLo = DstIsIXL || DstIsIYL;

    if ((DstIsIndexHi || DstIsIndexLo) && Z80::GR8RegClass.contains(SrcReg)) {
      // Try direct undocumented LD if available (works for A,B,C,D,E)
      if (STI->hasUndocumented()) {
        unsigned DirectOp = Z80::getLD8RegOpcode(DestReg, SrcReg);
        if (DirectOp) {
          BuildMI(MBB, I, DL, get(DirectOp));
          return;
        }
      }

      unsigned PushIR = (DstIsIXH || DstIsIXL) ? Z80::PUSH_IX : Z80::PUSH_IY;
      unsigned PopIR = (DstIsIXH || DstIsIXL) ? Z80::POP_IX : Z80::POP_IY;
      Register TargetReg = DstIsIndexHi ? Z80::H : Z80::L;

      // Compute HL liveness once; shared by both sub-paths below.
      const TargetRegisterInfo *TRI5bc = STI->getRegisterInfo();
      auto HLQ5bc = MBB.computeRegisterLiveness(TRI5bc, Z80::H, I);
      auto LLQ5bc = MBB.computeRegisterLiveness(TRI5bc, Z80::L, I);
      bool HLive5bc = (HLQ5bc != MachineBasicBlock::LQR_Dead);
      bool LLive5bc = (LLQ5bc != MachineBasicBlock::LQR_Dead);
      bool NeedSaveHL5bc = HLive5bc || LLive5bc;

      if (SrcReg == Z80::H || SrcReg == Z80::L) {
        // Source is H/L: preserve it in A first, then save HL across the
        // PUSH IX; POP HL read-modify-write.  Gate and IMPLICIT_DEF dead half
        // before PUSH_HL (#239 site 5b).
        BuildMI(MBB, I, DL, get(Z80::PUSH_AF));
        unsigned LdASrc = Z80::getLD8RegOpcode(Z80::A, SrcReg);
        BuildMI(MBB, I, DL, get(LdASrc));
        if (NeedSaveHL5bc) {
          if (!HLive5bc)
            BuildMI(MBB, I, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::H);
          if (!LLive5bc)
            BuildMI(MBB, I, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::L);
          BuildMI(MBB, I, DL, get(Z80::PUSH_HL));
        }
        BuildMI(MBB, I, DL, get(PushIR));
        BuildMI(MBB, I, DL, get(Z80::POP_HL));
        unsigned LdTargetA = Z80::getLD8RegOpcode(TargetReg, Z80::A);
        BuildMI(MBB, I, DL, get(LdTargetA));
        BuildMI(MBB, I, DL, get(Z80::PUSH_HL));
        BuildMI(MBB, I, DL, get(PopIR));
        if (NeedSaveHL5bc)
          BuildMI(MBB, I, DL, get(Z80::POP_HL));
        BuildMI(MBB, I, DL, get(Z80::POP_AF));
        return;
      }

      // Source is not H/L: read-modify-write IX/IY via HL scratch.
      // Gate and IMPLICIT_DEF dead half before PUSH_HL (#239 site 5c).
      if (NeedSaveHL5bc) {
        if (!HLive5bc)
          BuildMI(MBB, I, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::H);
        if (!LLive5bc)
          BuildMI(MBB, I, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::L);
        BuildMI(MBB, I, DL, get(Z80::PUSH_HL));
      }
      BuildMI(MBB, I, DL, get(PushIR));
      BuildMI(MBB, I, DL, get(Z80::POP_HL));
      unsigned LdOp = Z80::getLD8RegOpcode(TargetReg, SrcReg);
      BuildMI(MBB, I, DL, get(LdOp));
      BuildMI(MBB, I, DL, get(Z80::PUSH_HL));
      BuildMI(MBB, I, DL, get(PopIR));
      if (NeedSaveHL5bc)
        BuildMI(MBB, I, DL, get(Z80::POP_HL));
      return;
    }
  }

  llvm_unreachable("Cannot copy between these registers");
}

// Get the indexed store opcode for LD (IX+d),r
static unsigned getStoreIXdOpcode(Register Reg) {
  static const unsigned T[] = {Z80::LD_IXd_A, Z80::LD_IXd_B, Z80::LD_IXd_C,
                               Z80::LD_IXd_D, Z80::LD_IXd_E, Z80::LD_IXd_H,
                               Z80::LD_IXd_L};
  int I = Z80::gr8RegToIndex(Reg);
  return I >= 0 ? T[I] : 0;
}

static unsigned getLoadIXdOpcode(Register Reg) {
  static const unsigned T[] = {Z80::LD_A_IXd, Z80::LD_B_IXd, Z80::LD_C_IXd,
                               Z80::LD_D_IXd, Z80::LD_E_IXd, Z80::LD_H_IXd,
                               Z80::LD_L_IXd};
  int I = Z80::gr8RegToIndex(Reg);
  return I >= 0 ? T[I] : 0;
}

void Z80InstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
    bool isKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
    MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (MI != MBB.end())
    DL = MI->getDebugLoc();

  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Always use SPILL pseudos for both physical and virtual registers.
  // The pseudos handle large frame offsets correctly (HL-indirect addressing)
  // and are expanded in expandPostRAPseudo after eliminateFrameIndex.
  if (Z80::GR8RegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, MI, DL, get(Z80::SPILL_GR8))
        .addReg(SrcReg, getKillRegState(isKill))
        .addFrameIndex(FrameIndex)
        .addImm(0)
        .addMemOperand(MF.getMachineMemOperand(
            MachinePointerInfo::getFixedStack(MF, FrameIndex),
            MachineMemOperand::MOStore, 1, MFI.getObjectAlign(FrameIndex)));
    return;
  }

  {
    const TargetRegisterInfo *TRI = STI->getRegisterInfo();
    if (RC->hasSuperClassEq(&Z80::GR16RegClass) ||
        TRI->getCommonSubClass(RC, &Z80::GR16RegClass) ||
        Z80::IR16RegClass.hasSubClassEq(RC)) {
      BuildMI(MBB, MI, DL, get(Z80::SPILL_GR16))
          .addReg(SrcReg, getKillRegState(isKill))
          .addFrameIndex(FrameIndex)
          .addImm(0)
          .addMemOperand(MF.getMachineMemOperand(
              MachinePointerInfo::getFixedStack(MF, FrameIndex),
              MachineMemOperand::MOStore, 2, MFI.getObjectAlign(FrameIndex)));
      return;
    }
  }

  llvm_unreachable("storeRegToStackSlot: unsupported register class");
}

void Z80InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        Register DestReg, int FrameIndex,
                                        const TargetRegisterClass *RC,
                                        Register VReg, unsigned SubReg,
                                        MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (MI != MBB.end())
    DL = MI->getDebugLoc();

  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Always use RELOAD pseudos for both physical and virtual registers.
  // The pseudos handle large frame offsets correctly (HL-indirect addressing)
  // and are expanded in expandPostRAPseudo after eliminateFrameIndex.
  if (Z80::GR8RegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, MI, DL, get(Z80::RELOAD_GR8))
        .addReg(DestReg, RegState::Define)
        .addFrameIndex(FrameIndex)
        .addImm(0)
        .addMemOperand(MF.getMachineMemOperand(
            MachinePointerInfo::getFixedStack(MF, FrameIndex),
            MachineMemOperand::MOLoad, 1, MFI.getObjectAlign(FrameIndex)));
    return;
  }

  {
    const TargetRegisterInfo *TRI = STI->getRegisterInfo();
    if (RC->hasSuperClassEq(&Z80::GR16RegClass) ||
        TRI->getCommonSubClass(RC, &Z80::GR16RegClass) ||
        Z80::IR16RegClass.hasSubClassEq(RC)) {
      BuildMI(MBB, MI, DL, get(Z80::RELOAD_GR16))
          .addReg(DestReg, RegState::Define)
          .addFrameIndex(FrameIndex)
          .addImm(0)
          .addMemOperand(MF.getMachineMemOperand(
              MachinePointerInfo::getFixedStack(MF, FrameIndex),
              MachineMemOperand::MOLoad, 2, MFI.getObjectAlign(FrameIndex)));
      return;
    }
  }

  llvm_unreachable("loadRegFromStackSlot: unsupported register class");
}

/// Map a conditional branch opcode to its inverse.
/// Works with both JR and JP forms.
bool Z80InstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 1 && "Invalid Z80 branch condition!");
  unsigned Opc = Cond[0].getImm();
  switch (Opc) {
  case Z80::JP_Z_nn:
    Cond[0].setImm(Z80::JP_NZ_nn);
    return false;
  case Z80::JP_NZ_nn:
    Cond[0].setImm(Z80::JP_Z_nn);
    return false;
  case Z80::JP_C_nn:
    Cond[0].setImm(Z80::JP_NC_nn);
    return false;
  case Z80::JP_NC_nn:
    Cond[0].setImm(Z80::JP_C_nn);
    return false;
  case Z80::JR_Z_e:
    Cond[0].setImm(Z80::JR_NZ_e);
    return false;
  case Z80::JR_NZ_e:
    Cond[0].setImm(Z80::JR_Z_e);
    return false;
  case Z80::JR_C_e:
    Cond[0].setImm(Z80::JR_NC_e);
    return false;
  case Z80::JR_NC_e:
    Cond[0].setImm(Z80::JR_C_e);
    return false;
  default:
    return true;
  }
}

int Z80InstrInfo::getSPAdjust(const MachineInstr &MI) const {
  unsigned Opc = MI.getOpcode();

  // ADJCALLSTACKDOWN is erased without physical SP change.
  // The actual SP adjustments come from individual PUSH instructions,
  // so we report 0 here to avoid double-counting.
  if (Opc == Z80::ADJCALLSTACKDOWN)
    return 0;

  // SPAdj uses the "offset correction" sign convention (StackGrowsDown):
  //   PUSH (SP decreases) → positive (offsets from SP need to increase)
  //   POP  (SP increases) → negative (offsets from SP need to decrease)
  // This matches the base class where ADJCALLSTACKDOWN returns positive.
  switch (Opc) {
  case Z80::PUSH_BC:
  case Z80::PUSH_DE:
  case Z80::PUSH_HL:
  case Z80::PUSH_AF:
  case Z80::PUSH_IX:
  case Z80::PUSH_IY:
    return 2;
  case Z80::POP_BC:
  case Z80::POP_DE:
  case Z80::POP_HL:
  case Z80::POP_AF:
  case Z80::POP_IX:
  case Z80::POP_IY:
    return -2;
  case Z80::INC_SP:
    return -1; // SP += 1 (pops 1 byte)
  case Z80::DEC_SP:
    return 1; // SP -= 1 (pushes 1 byte)
  default:
    return TargetInstrInfo::getSPAdjust(MI);
  }
}

// True when A's incoming value is dead at MI (no value flows in).  Used to
// decide whether a flag-only `AND A` / `SBC A,A` emitted by the expansion of
// MI reads a don't-care A: when A is dead the $a read must be marked undef to
// satisfy -verify-machineinstrs (ravn/llvm-z80 #197); when A is live we keep
// the real read so no pass propagates undef into the live value.
static bool aIsDeadInto(const MachineInstr &MI, const TargetRegisterInfo *TRI) {
  const MachineBasicBlock &MBB = *MI.getParent();
  LivePhysRegs LiveRegs(*TRI);
  LiveRegs.addLiveOuts(MBB);
  for (auto I = MBB.rbegin(); &*I != &MI; ++I)
    LiveRegs.stepBackward(*I);
  LiveRegs.stepBackward(MI); // live-in to MI
  return !LiveRegs.contains(Z80::A);
}

// Mark the $a *use* of a just-built flag-only A op (AND A / SBC A,A) undef.
static void markAReadUndef(MachineInstrBuilder &MIB) {
  for (MachineOperand &MO : MIB->operands())
    if (MO.isReg() && MO.isUse() && MO.getReg() == Z80::A)
      MO.setIsUndef(true);
}

bool Z80InstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  MachineBasicBlock &MBB = *MI.getParent();
  const TargetRegisterInfo *TRI = STI->getRegisterInfo();
  DebugLoc DL = MI.getDebugLoc();

  switch (MI.getOpcode()) {
  case Z80::LOAD8_IND: {
    // Expand to LD A,(BC), LD A,(DE), or LD A,(HL) based on allocated register.
    Register Addr = MI.getOperand(0).getReg();
    unsigned Opc;
    if (Addr == Z80::BC) Opc = Z80::LD_A_BCind;
    else if (Addr == Z80::DE) Opc = Z80::LD_A_DEind;
    else if (Addr == Z80::HL) Opc = Z80::LD_A_HLind;
    else if (Addr == Z80::IX) {
      // LD A,(IX+0) — IX indirect with zero displacement
      BuildMI(MBB, MI, DL, get(Z80::LD_A_IXd)).addImm(0);
      MI.eraseFromParent();
      return true;
    } else if (Addr == Z80::IY) {
      BuildMI(MBB, MI, DL, get(Z80::LD_A_IYd)).addImm(0);
      MI.eraseFromParent();
      return true;
    } else llvm_unreachable("Invalid register for LOAD8_IND");
    BuildMI(MBB, MI, DL, get(Opc));
    MI.eraseFromParent();
    return true;
  }

  case Z80::STORE8_IND: {
    // Expand to LD (BC),A, LD (DE),A, or LD (HL),A based on allocated register.
    Register Addr = MI.getOperand(0).getReg();
    unsigned Opc;
    if (Addr == Z80::BC) Opc = Z80::LD_BCind_A;
    else if (Addr == Z80::DE) Opc = Z80::LD_DEind_A;
    else if (Addr == Z80::HL) Opc = Z80::LD_HLind_A;
    else if (Addr == Z80::IX) {
      // LD (IX+0),A — IX indirect with zero displacement
      BuildMI(MBB, MI, DL, get(Z80::LD_IXd_A)).addImm(0);
      MI.eraseFromParent();
      return true;
    } else if (Addr == Z80::IY) {
      BuildMI(MBB, MI, DL, get(Z80::LD_IYd_A)).addImm(0);
      MI.eraseFromParent();
      return true;
    } else llvm_unreachable("Invalid register for STORE8_IND");
    BuildMI(MBB, MI, DL, get(Opc));
    MI.eraseFromParent();
    return true;
  }

  case Z80::LD_r8_n: {
    // LD_r8_n dst, imm -> LD_A_n/LD_B_n/etc. based on dst register
    Register DstReg = MI.getOperand(0).getReg();

    unsigned Opcode;
    switch (DstReg.id()) {
    case Z80::A:
      Opcode = Z80::LD_A_n;
      break;
    case Z80::B:
      Opcode = Z80::LD_B_n;
      break;
    case Z80::C:
      Opcode = Z80::LD_C_n;
      break;
    case Z80::D:
      Opcode = Z80::LD_D_n;
      break;
    case Z80::E:
      Opcode = Z80::LD_E_n;
      break;
    case Z80::H:
      Opcode = Z80::LD_H_n;
      break;
    case Z80::L:
      Opcode = Z80::LD_L_n;
      break;
    default:
      llvm_unreachable("Unexpected register for LD_r8_n");
    }

    BuildMI(MBB, MI, DL, get(Opcode)).add(MI.getOperand(1));
    MI.eraseFromParent();
    return true;
  }

  case Z80::LD_r16_nn: {
    // LD_r16_nn dst, imm -> LD_BC_nn/LD_DE_nn/LD_HL_nn based on dst register
    Register DstReg = MI.getOperand(0).getReg();

    unsigned Opcode;
    if (DstReg == Z80::BC)
      Opcode = Z80::LD_BC_nn;
    else if (DstReg == Z80::DE)
      Opcode = Z80::LD_DE_nn;
    else if (DstReg == Z80::HL)
      Opcode = Z80::LD_HL_nn;
    else if (DstReg == Z80::IX)
      Opcode = Z80::LD_IX_nn;
    else if (DstReg == Z80::IY)
      Opcode = Z80::LD_IY_nn;
    else
      llvm_unreachable("Unexpected register for LD_r16_nn");

    BuildMI(MBB, MI, DL, get(Opcode)).add(MI.getOperand(1));
    MI.eraseFromParent();
    return true;
  }

  case Z80::ZEXT_GR8_GR16: {
    // Zero extend 8-bit to 16-bit: LD lo,src; LD hi,0
    Register DstReg = MI.getOperand(0).getReg();
    Register SrcReg = MI.getOperand(1).getReg();

    // IX/IY destination without +undocumented: route through HL + PUSH/POP.
    // Direct LD IXL,src / LD IXH,0 are undocumented instructions.
    if (Z80::IR16RegClass.contains(DstReg) && !STI->hasUndocumented()) {
      LivePhysRegs LiveRegs(*TRI);
      LiveRegs.addLiveOuts(MBB);
      for (auto I = MBB.rbegin(); &*I != &MI; ++I)
        LiveRegs.stepBackward(*I);
      bool HLive = LiveRegs.contains(Z80::H);
      bool LLive = LiveRegs.contains(Z80::L);
      bool HLLive = HLive || LLive;
      if (HLLive) {
        // IMPLICIT_DEF any dead half before PUSH_HL so the pair PUSH does
        // not read partial-undef (ravn/llvm-z80#239 site 1).
        if (!HLive)
          BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::H);
        if (!LLive)
          BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::L);
        BuildMI(MBB, MI, DL, get(Z80::PUSH_HL));
      }
      // Copy source to L (skip if already there)
      if (SrcReg != Z80::L) {
        unsigned CopyOp = Z80::getLD8RegOpcode(Z80::L, SrcReg);
        if (!CopyOp)
          return false;
        BuildMI(MBB, MI, DL, get(CopyOp));
      }
      // LD H,0
      BuildMI(MBB, MI, DL, get(Z80::LD_H_n)).addImm(0);
      // Transfer HL → IX/IY
      BuildMI(MBB, MI, DL, get(Z80::PUSH_HL));
      BuildMI(MBB, MI, DL, get(Z80::getPopOpcode(DstReg)));
      if (HLLive)
        BuildMI(MBB, MI, DL, get(Z80::POP_HL));
      MI.eraseFromParent();
      return true;
    }

    Register LoReg = TRI->getSubReg(DstReg, Z80::sub_lo);
    Register HiReg = TRI->getSubReg(DstReg, Z80::sub_hi);
    if (!LoReg || !HiReg)
      return false;

    // Copy source to low byte (skip if already in place)
    if (SrcReg != LoReg) {
      unsigned CopyOp = Z80::getLD8RegOpcode(LoReg, SrcReg);
      if (!CopyOp)
        return false;
      BuildMI(MBB, MI, DL, get(CopyOp));
    }
    // Set high byte to 0
    unsigned ImmOp = getLoadImmR8Opcode(HiReg);
    if (!ImmOp)
      return false;
    BuildMI(MBB, MI, DL, get(ImmOp)).addImm(0);
    MI.eraseFromParent();
    return true;
  }

  case Z80::SEXT_GR8_GR16: {
    // Sign extend 8-bit to 16-bit:
    // LD A,src; LD lo,A; RLCA; SBC A,A; LD hi,A
    Register DstReg = MI.getOperand(0).getReg();
    Register SrcReg = MI.getOperand(1).getReg();

    // IX/IY destination without +undocumented: sign-extend into HL, then
    // PUSH HL; POP IX/IY.  LD IXL,A / LD IXH,A are undocumented.
    if (Z80::IR16RegClass.contains(DstReg) && !STI->hasUndocumented()) {
      LivePhysRegs LiveRegs(*TRI);
      LiveRegs.addLiveOuts(MBB);
      for (auto I = MBB.rbegin(); &*I != &MI; ++I)
        LiveRegs.stepBackward(*I);
      bool HLive = LiveRegs.contains(Z80::H);
      bool LLive = LiveRegs.contains(Z80::L);
      bool HLLive = HLive || LLive;
      if (HLLive) {
        // IMPLICIT_DEF any dead half before PUSH_HL so the pair PUSH does
        // not read partial-undef (ravn/llvm-z80#239 site 1 symmetry).
        if (!HLive)
          BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::H);
        if (!LLive)
          BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::L);
        BuildMI(MBB, MI, DL, get(Z80::PUSH_HL));
      }
      // Copy source to A (for sign-bit extraction)
      if (SrcReg != Z80::A) {
        unsigned CopyToA = Z80::getLD8RegOpcode(Z80::A, SrcReg);
        if (!CopyToA)
          return false;
        BuildMI(MBB, MI, DL, get(CopyToA));
      }
      // LD L,A (low byte = source)
      if (SrcReg != Z80::L)
        BuildMI(MBB, MI, DL, get(Z80::LD_L_A));
      // RLCA; SBC A,A → A = sign extension byte
      BuildMI(MBB, MI, DL, get(Z80::RLCA));
      { auto SbcAA = BuildMI(MBB, MI, DL, get(Z80::SBC_A_A)); markAReadUndef(SbcAA); }
      // LD H,A (high byte = sign extension)
      BuildMI(MBB, MI, DL, get(Z80::LD_H_A));
      // Transfer HL → IX/IY
      BuildMI(MBB, MI, DL, get(Z80::PUSH_HL));
      BuildMI(MBB, MI, DL, get(Z80::getPopOpcode(DstReg)));
      if (HLLive)
        BuildMI(MBB, MI, DL, get(Z80::POP_HL));
      MI.eraseFromParent();
      return true;
    }

    Register LoReg = TRI->getSubReg(DstReg, Z80::sub_lo);
    Register HiReg = TRI->getSubReg(DstReg, Z80::sub_hi);
    if (!LoReg || !HiReg)
      return false;

    // Copy source to A (for sign-bit extraction)
    if (SrcReg != Z80::A) {
      unsigned CopyToA = Z80::getLD8RegOpcode(Z80::A, SrcReg);
      if (!CopyToA)
        return false;
      BuildMI(MBB, MI, DL, get(CopyToA));
    }
    // Copy A to low byte
    if (LoReg != Z80::A) {
      unsigned CopyToLo = Z80::getLD8RegOpcode(LoReg, Z80::A);
      if (!CopyToLo)
        return false;
      BuildMI(MBB, MI, DL, get(CopyToLo));
    }
    // RLCA rotates bit 7 into carry
    BuildMI(MBB, MI, DL, get(Z80::RLCA));
    // SBC A,A: A = 0xFF if carry (negative), 0x00 if not
    { auto SbcAA = BuildMI(MBB, MI, DL, get(Z80::SBC_A_A)); markAReadUndef(SbcAA); }
    // Copy A (sign extension) to high byte
    if (HiReg != Z80::A) {
      unsigned CopyToHi = Z80::getLD8RegOpcode(HiReg, Z80::A);
      if (!CopyToHi)
        return false;
      BuildMI(MBB, MI, DL, get(CopyToHi));
    }
    MI.eraseFromParent();
    return true;
  }

  case Z80::SPILL_IMM8: {
    // SPILL_IMM8 val, offset -> LD (IX+d),n
    // Large offsets are handled in eliminateFrameIndex.
    int64_t Val = MI.getOperand(0).getImm();
    int64_t Offset = MI.getOperand(1).getImm();

    assert(Offset >= -128 && Offset <= 127 &&
           "Large offset should have been expanded in eliminateFrameIndex");
    BuildMI(MBB, MI, DL, get(Z80::LD_IXd_n)).addImm(Offset).addImm(Val & 0xFF);
    MI.eraseFromParent();
    return true;
  }

  case Z80::SPILL_GR8: {
    // SPILL_GR8 src, offset -> LD (IX+d),r
    // Large offsets are handled in eliminateFrameIndex.
    Register SrcReg = MI.getOperand(0).getReg();
    int64_t Offset = MI.getOperand(1).getImm();

    if (!SrcReg.isPhysical())
      return false;

    assert(Offset >= -128 && Offset <= 127 &&
           "Large offset should have been expanded in eliminateFrameIndex");
    unsigned Opcode = getStoreIXdOpcode(SrcReg);
    if (!Opcode) {
      // Undocumented IXH/IXL/IYH/IYL: route through A.
      // LD A,src; LD (IX+d),A — must save A if live.
      unsigned CopyToA = Z80::getLD8RegOpcode(Z80::A, SrcReg);
      if (!CopyToA) return false;
      LivePhysRegs LiveRegs(*TRI);
      LiveRegs.addLiveOuts(MBB);
      for (auto I = MBB.rbegin(); &*I != &MI; ++I)
        LiveRegs.stepBackward(*I);
      bool ALive = LiveRegs.contains(Z80::A);
      if (ALive)
        BuildMI(MBB, MI, DL, get(Z80::PUSH_AF));
      BuildMI(MBB, MI, DL, get(CopyToA));
      BuildMI(MBB, MI, DL, get(Z80::LD_IXd_A)).addImm(Offset);
      if (ALive)
        BuildMI(MBB, MI, DL, get(Z80::POP_AF));
      MI.eraseFromParent();
      return true;
    }
    BuildMI(MBB, MI, DL, get(Opcode)).addImm(Offset);
    MI.eraseFromParent();
    return true;
  }

  case Z80::RELOAD_GR8: {
    // RELOAD_GR8 dst, offset -> LD r,(IX+d)
    // Large offsets are handled in eliminateFrameIndex.
    Register DstReg = MI.getOperand(0).getReg();
    int64_t Offset = MI.getOperand(1).getImm();

    if (!DstReg.isPhysical())
      return false;

    assert(Offset >= -128 && Offset <= 127 &&
           "Large offset should have been expanded in eliminateFrameIndex");
    unsigned Opcode = getLoadIXdOpcode(DstReg);
    if (!Opcode) {
      // Undocumented IXH/IXL/IYH/IYL: route through A.
      // LD A,(IX+d); LD dst,A — must save A if live.
      unsigned CopyFromA = Z80::getLD8RegOpcode(DstReg, Z80::A);
      if (!CopyFromA) return false;
      LivePhysRegs LiveRegs(*TRI);
      LiveRegs.addLiveOuts(MBB);
      for (auto I = MBB.rbegin(); &*I != &MI; ++I)
        LiveRegs.stepBackward(*I);
      bool ALive = LiveRegs.contains(Z80::A);
      if (ALive)
        BuildMI(MBB, MI, DL, get(Z80::PUSH_AF));
      BuildMI(MBB, MI, DL, get(Z80::LD_A_IXd)).addImm(Offset);
      BuildMI(MBB, MI, DL, get(CopyFromA));
      if (ALive)
        BuildMI(MBB, MI, DL, get(Z80::POP_AF));
      MI.eraseFromParent();
      return true;
    }
    BuildMI(MBB, MI, DL, get(Opcode)).addImm(Offset);
    MI.eraseFromParent();
    return true;
  }

  case Z80::SPILL_GR16: {
    // SPILL_GR16 src, offset -> LD (IX+d),lo ; LD (IX+d+1),hi
    // With +static-stack: LD (addr),rr (3-4B) instead of 6B IX-indexed.
    Register SrcReg = MI.getOperand(0).getReg();
    int64_t Offset = MI.getOperand(1).getImm();

    if (!SrcReg.isPhysical())
      return false;

    assert(Offset >= -128 && Offset + 1 <= 127 &&
           "Large offset should have been expanded in eliminateFrameIndex");

    if (SrcReg == Z80::SP)
      llvm_unreachable("SP cannot be spilled via SPILL_GR16");

    // Static stack: use direct BSS addressing (3-4B vs 6B IX-indexed).
    // LD (addr),HL = 3B, LD (addr),DE/BC = 4B vs LD (IX+d),lo;LD (IX+d+1),hi = 6B.
    // Only when UseStaticFrame (IX = __sfrend_) AND offset is negative (local
    // in BSS). Positive offsets would be stack args on the real stack.
    MachineFunction &MF = *MBB.getParent();
    Z80FunctionInfo *FI = MF.getInfo<Z80FunctionInfo>();
    if (FI->getUseStaticFrame() && Offset < 0) {
      MCSymbol *EndSym = MF.getContext().getOrCreateSymbol(
          "__sfrend_" + MF.getName());
      if (SrcReg == Z80::HL) {
        // LD (addr),HL = 3B (vs 6B IX-indexed)
        auto *StoreMI = BuildMI(MBB, MI, DL, get(Z80::LD_nnind_HL))
            .addSym(EndSym).getInstr();
        StoreMI->getOperand(0).setOffset(Offset);
        MI.eraseFromParent();
        return true;
      }
      if (SrcReg == Z80::DE) {
        // LD (addr),DE = 4B (ED 53) vs 6B IX-indexed. Always wins.
        auto *StoreMI = BuildMI(MBB, MI, DL, get(Z80::LD_nnind_DE))
            .addSym(EndSym).getInstr();
        StoreMI->getOperand(0).setOffset(Offset);
        MI.eraseFromParent();
        return true;
      }
      if (SrcReg == Z80::BC) {
        // LD (addr),BC = 4B (ED 43) vs 6B IX-indexed. Always wins.
        auto *StoreMI = BuildMI(MBB, MI, DL, get(Z80::LD_nnind_BC))
            .addSym(EndSym).getInstr();
        StoreMI->getOperand(0).setOffset(Offset);
        MI.eraseFromParent();
        return true;
      }
      if (SrcReg == Z80::IX) {
        // LD (addr),IX = 4B (DD 22) vs 6B IX-indexed.
        auto *StoreMI = BuildMI(MBB, MI, DL, get(Z80::LD_nnind_IX))
            .addSym(EndSym).getInstr();
        StoreMI->getOperand(0).setOffset(Offset);
        MI.eraseFromParent();
        return true;
      }
      if (SrcReg == Z80::IY) {
        // LD (addr),IY = 4B (FD 22) vs 6B IX-indexed.
        auto *StoreMI = BuildMI(MBB, MI, DL, get(Z80::LD_nnind_IY))
            .addSym(EndSym).getInstr();
        StoreMI->getOperand(0).setOffset(Offset);
        MI.eraseFromParent();
        return true;
      }
    }

    Register LoReg = TRI->getSubReg(SrcReg, Z80::sub_lo);
    Register HiReg = TRI->getSubReg(SrcReg, Z80::sub_hi);
    if (!LoReg || !HiReg)
      return false;

    unsigned LoOp = getStoreIXdOpcode(LoReg);
    unsigned HiOp = getStoreIXdOpcode(HiReg);

    if (!LoOp || !HiOp) {
      if (SrcReg == Z80::IX || SrcReg == Z80::IY) {
        // IX/IY have no IX-indexed store opcodes for their halves.
        // Transfer via HL: PUSH IX/IY; POP HL; LD (IX+d),L; LD (IX+d+1),H
        unsigned PushOp = (SrcReg == Z80::IX) ? Z80::PUSH_IX : Z80::PUSH_IY;
        LivePhysRegs LiveRegs(*TRI);
        LiveRegs.addLiveOuts(MBB);
        for (auto I = MBB.rbegin(); &*I != &MI; ++I)
          LiveRegs.stepBackward(*I);
        bool HLive = LiveRegs.contains(Z80::H);
        bool LLive = LiveRegs.contains(Z80::L);
        bool NeedSaveHL = HLive || LLive;
        if (NeedSaveHL) {
          // IMPLICIT_DEF dead half before PUSH_HL (ravn/llvm-z80#239 site 2).
          if (!HLive)
            BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::H);
          if (!LLive)
            BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::L);
          BuildMI(MBB, MI, DL, get(Z80::PUSH_HL));
        }
        BuildMI(MBB, MI, DL, get(PushOp));
        BuildMI(MBB, MI, DL, get(Z80::POP_HL));
        BuildMI(MBB, MI, DL, get(Z80::LD_IXd_L)).addImm(Offset);
        BuildMI(MBB, MI, DL, get(Z80::LD_IXd_H)).addImm(Offset + 1);
        if (NeedSaveHL)
          BuildMI(MBB, MI, DL, get(Z80::POP_HL));
        MI.eraseFromParent();
        return true;
      }
      return false;
    }

    BuildMI(MBB, MI, DL, get(LoOp)).addImm(Offset);
    BuildMI(MBB, MI, DL, get(HiOp)).addImm(Offset + 1);
    MI.eraseFromParent();
    return true;
  }

  case Z80::RELOAD_GR16: {
    // RELOAD_GR16 dst, offset -> LD lo,(IX+d) ; LD hi,(IX+d+1)
    // Large offsets are handled in eliminateFrameIndex.
    Register DestReg = MI.getOperand(0).getReg();
    int64_t Offset = MI.getOperand(1).getImm();

    if (!DestReg.isPhysical())
      return false;

    assert(Offset >= -128 && Offset + 1 <= 127 &&
           "Large offset should have been expanded in eliminateFrameIndex");

    if (DestReg == Z80::SP)
      llvm_unreachable("SP cannot be reloaded via RELOAD_GR16");

    // Static stack: use direct BSS addressing (3-4B vs 6B IX-indexed).
    // Only when UseStaticFrame (IX = __sfrend_) AND offset is negative (local
    // in BSS). Positive offsets would be stack args on the real stack.
    MachineFunction &MF = *MBB.getParent();
    Z80FunctionInfo *FI = MF.getInfo<Z80FunctionInfo>();
    if (FI->getUseStaticFrame() && Offset < 0) {
      MCSymbol *EndSym = MF.getContext().getOrCreateSymbol(
          "__sfrend_" + MF.getName());
      if (DestReg == Z80::HL) {
        // LD HL,(addr) = 3B (vs 6B IX-indexed)
        auto *LoadMI = BuildMI(MBB, MI, DL, get(Z80::LD_HL_nnind))
            .addSym(EndSym).getInstr();
        LoadMI->getOperand(0).setOffset(Offset);
        MI.eraseFromParent();
        return true;
      }
      if (DestReg == Z80::DE) {
        // LD DE,(addr) = 4B (ED 5B) vs 6B IX-indexed. Always wins.
        auto *LoadMI = BuildMI(MBB, MI, DL, get(Z80::LD_DE_nnind))
            .addSym(EndSym).getInstr();
        LoadMI->getOperand(0).setOffset(Offset);
        MI.eraseFromParent();
        return true;
      }
      if (DestReg == Z80::BC) {
        // LD BC,(addr) = 4B (ED 4B) vs 6B IX-indexed. Always wins.
        auto *LoadMI = BuildMI(MBB, MI, DL, get(Z80::LD_BC_nnind))
            .addSym(EndSym).getInstr();
        LoadMI->getOperand(0).setOffset(Offset);
        MI.eraseFromParent();
        return true;
      }
      if (DestReg == Z80::IX) {
        // LD IX,(addr) = 4B (DD 2A) vs 6B IX-indexed.
        auto *LoadMI = BuildMI(MBB, MI, DL, get(Z80::LD_IX_nnind))
            .addSym(EndSym).getInstr();
        LoadMI->getOperand(0).setOffset(Offset);
        MI.eraseFromParent();
        return true;
      }
      if (DestReg == Z80::IY) {
        // LD IY,(addr) = 4B (FD 2A) vs 6B IX-indexed.
        auto *LoadMI = BuildMI(MBB, MI, DL, get(Z80::LD_IY_nnind))
            .addSym(EndSym).getInstr();
        LoadMI->getOperand(0).setOffset(Offset);
        MI.eraseFromParent();
        return true;
      }
    }

    Register LoReg = TRI->getSubReg(DestReg, Z80::sub_lo);
    Register HiReg = TRI->getSubReg(DestReg, Z80::sub_hi);
    if (!LoReg || !HiReg)
      return false;

    unsigned LoOp = getLoadIXdOpcode(LoReg);
    unsigned HiOp = getLoadIXdOpcode(HiReg);

    if (!LoOp || !HiOp) {
      if (DestReg == Z80::IX || DestReg == Z80::IY) {
        // IX/IY have no IX-indexed load opcodes for their halves.
        // Transfer via HL: LD L,(IX+d); LD H,(IX+d+1); PUSH HL; POP IX/IY
        unsigned PopOp = (DestReg == Z80::IX) ? Z80::POP_IX : Z80::POP_IY;
        LivePhysRegs LiveRegs(*TRI);
        LiveRegs.addLiveOuts(MBB);
        for (auto I = MBB.rbegin(); &*I != &MI; ++I)
          LiveRegs.stepBackward(*I);
        bool HLive = LiveRegs.contains(Z80::H);
        bool LLive = LiveRegs.contains(Z80::L);
        bool NeedSaveHL = HLive || LLive;
        if (NeedSaveHL) {
          // IMPLICIT_DEF dead half before PUSH_HL (ravn/llvm-z80#239 site 3).
          if (!HLive)
            BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::H);
          if (!LLive)
            BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::L);
          BuildMI(MBB, MI, DL, get(Z80::PUSH_HL));
        }
        BuildMI(MBB, MI, DL, get(Z80::LD_L_IXd)).addImm(Offset);
        BuildMI(MBB, MI, DL, get(Z80::LD_H_IXd)).addImm(Offset + 1);
        BuildMI(MBB, MI, DL, get(Z80::PUSH_HL));
        BuildMI(MBB, MI, DL, get(PopOp));
        if (NeedSaveHL)
          BuildMI(MBB, MI, DL, get(Z80::POP_HL));
        MI.eraseFromParent();
        return true;
      }
      return false;
    }

    BuildMI(MBB, MI, DL, get(LoOp)).addImm(Offset);
    BuildMI(MBB, MI, DL, get(HiOp)).addImm(Offset + 1);
    MI.eraseFromParent();
    return true;
  }

  case Z80::CMP16_FLAGS:
  case Z80::CMP16_ULT: {
    // 8-bit SUB/SBC chain for 16-bit unsigned comparison.
    // LD A,lhs_lo; SUB rhs_lo; LD A,lhs_hi; SBC A,rhs_hi
    // Does NOT clobber HL/DE. Only clobbers A and FLAGS.
    Register LHSReg = MI.getOperand(0).getReg();
    Register RHSReg = MI.getOperand(1).getReg();
    Register LhsLo = TRI->getSubReg(LHSReg, Z80::sub_lo);
    Register LhsHi = TRI->getSubReg(LHSReg, Z80::sub_hi);
    Register RhsLo = TRI->getSubReg(RHSReg, Z80::sub_lo);
    Register RhsHi = TRI->getSubReg(RHSReg, Z80::sub_hi);

    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Z80::A, LhsLo)));
    BuildMI(MBB, MI, DL, get(getSUBOpcode(RhsLo)));
    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Z80::A, LhsHi)));
    BuildMI(MBB, MI, DL, get(getSBCOpcode(RhsHi)));

    if (MI.getOpcode() == Z80::CMP16_ULT) {
      { auto SbcAA = BuildMI(MBB, MI, DL, get(Z80::SBC_A_A)); markAReadUndef(SbcAA); }
      BuildMI(MBB, MI, DL, get(Z80::AND_n)).addImm(1);
    }

    MI.eraseFromParent();
    return true;
  }

  case Z80::CMP16_SBC_FLAGS: {
    // Carry-chain continuation: all SBC (no initial SUB).
    // LD A,lhs_lo; SBC A,rhs_lo; LD A,lhs_hi; SBC A,rhs_hi
    // Carry-in from previous CMP16_FLAGS or CMP16_SBC_FLAGS.
    Register LHSReg = MI.getOperand(0).getReg();
    Register RHSReg = MI.getOperand(1).getReg();
    Register LhsLo = TRI->getSubReg(LHSReg, Z80::sub_lo);
    Register LhsHi = TRI->getSubReg(LHSReg, Z80::sub_hi);
    Register RhsLo = TRI->getSubReg(RHSReg, Z80::sub_lo);
    Register RhsHi = TRI->getSubReg(RHSReg, Z80::sub_hi);

    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Z80::A, LhsLo)));
    BuildMI(MBB, MI, DL, get(getSBCOpcode(RhsLo)));
    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Z80::A, LhsHi)));
    BuildMI(MBB, MI, DL, get(getSBCOpcode(RhsHi)));

    MI.eraseFromParent();
    return true;
  }

  case Z80::XOR_CMP_EQ16:
  case Z80::XOR_CMP_NE16: {
    // XOR-based 16-bit equality comparison.
    // Compares two GR16NoIR registers using byte-level XOR, produces 0/1 in A.
    // Does NOT clobber the source register pairs (unlike SBC HL,DE).
    // Only clobbers A and B.
    //
    // Sequence: LD A,lhs_hi; XOR rhs_hi; LD B,A; LD A,lhs_lo; XOR rhs_lo; OR B
    // Then normalize: EQ → SUB 1; SBC A,A; AND 1
    //                 NE → ADD 0xFF; SBC A,A; AND 1
    //
    // The pseudo's operand class is GR16NoIR (#113), so the sub-register
    // halves are guaranteed to be in {A,B,C,D,E,H,L} — no IXH/IXL/IYH/IYL.
    Register LHSReg = MI.getOperand(0).getReg();
    Register RHSReg = MI.getOperand(1).getReg();
    Register LHS_hi = TRI->getSubReg(LHSReg, Z80::sub_hi);
    Register LHS_lo = TRI->getSubReg(LHSReg, Z80::sub_lo);
    Register RHS_hi = TRI->getSubReg(RHSReg, Z80::sub_hi);
    Register RHS_lo = TRI->getSubReg(RHSReg, Z80::sub_lo);

    // XOR high bytes, save to B
    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Z80::A, LHS_hi)));
    BuildMI(MBB, MI, DL, get(getXOROpcode(RHS_hi)));
    BuildMI(MBB, MI, DL, get(Z80::LD_B_A));
    // XOR low bytes, OR with saved high result
    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Z80::A, LHS_lo)));
    BuildMI(MBB, MI, DL, get(getXOROpcode(RHS_lo)));
    BuildMI(MBB, MI, DL, get(Z80::OR_B));

    // Normalize to 0/1
    if (MI.getOpcode() == Z80::XOR_CMP_EQ16) {
      // A=0 (equal) → SUB 1 sets carry → SBC A,A → 0xFF → AND 1 → 1
      BuildMI(MBB, MI, DL, get(Z80::SUB_n)).addImm(1);
    } else {
      // A=0 (equal) → ADD 0xFF no carry → SBC A,A → 0 → AND 1 → 0
      BuildMI(MBB, MI, DL, get(Z80::ADD_A_n)).addImm(0xFF);
    }
    { auto SbcAA = BuildMI(MBB, MI, DL, get(Z80::SBC_A_A)); markAReadUndef(SbcAA); }
    BuildMI(MBB, MI, DL, get(Z80::AND_n)).addImm(1);

    MI.eraseFromParent();
    return true;
  }

  case Z80::SM83_CMP_ZERO16: {
    // Lightweight 16-bit zero test: LD A,lo; OR hi — sets Z if reg==0.
    // Only clobbers A (not B), saving register pressure in loops.
    Register SrcReg = MI.getOperand(0).getReg();
    Register Lo = TRI->getSubReg(SrcReg, Z80::sub_lo);
    Register Hi = TRI->getSubReg(SrcReg, Z80::sub_hi);

    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Z80::A, Lo)));
    BuildMI(MBB, MI, DL, get(getOROpcode(Hi)));

    MI.eraseFromParent();
    return true;
  }
  case Z80::SM83_CMP_Z16:
  case Z80::XOR_CMP_Z16: {
    // 16-bit XOR-based equality comparison — sets Z flag directly.
    // Sequence: LD A,lhs_hi; XOR rhs_hi; LD B,A; LD A,lhs_lo; XOR rhs_lo; OR B
    // After OR B: Z=1 if equal, Z=0 if not equal.
    //
    // Operand class is GR16NoIR (#113); halves are always documented 8-bit
    // registers, so the standard sub-register path is the only path.
    Register LHSReg = MI.getOperand(0).getReg();
    Register RHSReg = MI.getOperand(1).getReg();
    Register LHS_hi = TRI->getSubReg(LHSReg, Z80::sub_hi);
    Register LHS_lo = TRI->getSubReg(LHSReg, Z80::sub_lo);
    Register RHS_hi = TRI->getSubReg(RHSReg, Z80::sub_hi);
    Register RHS_lo = TRI->getSubReg(RHSReg, Z80::sub_lo);

    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Z80::A, LHS_hi)));
    BuildMI(MBB, MI, DL, get(getXOROpcode(RHS_hi)));
    BuildMI(MBB, MI, DL, get(Z80::LD_B_A));
    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Z80::A, LHS_lo)));
    BuildMI(MBB, MI, DL, get(getXOROpcode(RHS_lo)));
    BuildMI(MBB, MI, DL, get(Z80::OR_B));

    MI.eraseFromParent();
    return true;
  }

  case Z80::ADD16_tied: {
    // Generalized 16-bit add: dst = dst + rhs.
    // Select real instruction based on physical accumulator register.
    Register Acc = MI.getOperand(0).getReg();  // dst (tied to src)
    Register RHS = MI.getOperand(2).getReg();  // rhs (BC or DE)
    unsigned AddOpc = 0;
    if (Acc == Z80::HL) {
      AddOpc = (RHS == Z80::BC) ? Z80::ADD_HL_BC : Z80::ADD_HL_DE;
    } else if (Acc == Z80::IX) {
      AddOpc = (RHS == Z80::BC) ? Z80::ADD_IX_BC : Z80::ADD_IX_DE;
    } else if (Acc == Z80::IY) {
      AddOpc = (RHS == Z80::BC) ? Z80::ADD_IY_BC : Z80::ADD_IY_DE;
    } else {
      // BC or DE as accumulator: copy through HL.
      // PUSH <Acc>; POP HL; ADD HL,rr; PUSH HL; POP <Acc>
      // But if RHS == Acc's pair, we have a conflict. Handle carefully.
      BuildMI(MBB, MI, DL, get(Z80::getPushOpcode(Acc)));
      BuildMI(MBB, MI, DL, get(Z80::POP_HL));
      unsigned HLAdd = (RHS == Z80::BC) ? Z80::ADD_HL_BC : Z80::ADD_HL_DE;
      BuildMI(MBB, MI, DL, get(HLAdd));
      BuildMI(MBB, MI, DL, get(Z80::PUSH_HL));
      BuildMI(MBB, MI, DL, get(Z80::getPopOpcode(Acc)));
      MI.eraseFromParent();
      return true;
    }
    BuildMI(MBB, MI, DL, get(AddOpc));
    MI.eraseFromParent();
    return true;
  }

  case Z80::ADD16_acc: {
    // Non-tied accumulator add: $dst = $base + $rhs (#178).
    // $dst is HLI (HL, since IX/IY reserved); $base is GR16; $rhs is BC/DE.
    Register Dst = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    Register RHS = MI.getOperand(2).getReg();
    bool KillBase = MI.getOperand(1).isKill();
    // Move base into the accumulator unless regalloc already coincided.
    if (Dst != Base)
      copyPhysReg(MBB, MI, DL, Dst, Base, KillBase);
    unsigned AddOpc = 0;
    if (Dst == Z80::HL)
      AddOpc = (RHS == Z80::BC) ? Z80::ADD_HL_BC : Z80::ADD_HL_DE;
    else if (Dst == Z80::IX)
      AddOpc = (RHS == Z80::BC) ? Z80::ADD_IX_BC : Z80::ADD_IX_DE;
    else if (Dst == Z80::IY)
      AddOpc = (RHS == Z80::BC) ? Z80::ADD_IY_BC : Z80::ADD_IY_DE;
    else
      llvm_unreachable("ADD16_acc: $dst not in HLI");
    BuildMI(MBB, MI, DL, get(AddOpc));
    MI.eraseFromParent();
    return true;
  }

  case Z80::ADD_HL_rr: {
    // ADD HL,rr — select ADD_HL_BC or ADD_HL_DE based on allocated register.
    Register RHS = MI.getOperand(0).getReg();
    unsigned AddOpc;
    if (RHS == Z80::BC)
      AddOpc = Z80::ADD_HL_BC;
    else if (RHS == Z80::DE)
      AddOpc = Z80::ADD_HL_DE;
    else
      llvm_unreachable("ADD_HL_rr: unexpected register");
    BuildMI(MBB, MI, DL, get(AddOpc));
    MI.eraseFromParent();
    return true;
  }

  case Z80::SUB_HL_rr: {
    // 16-bit subtraction: HL = HL - rr (no borrow in).
    Register RHS = MI.getOperand(0).getReg();
    if (STI->hasSM83()) {
      // SM83: byte-by-byte SUB/SBC (no 16-bit SBC HL,rr instruction).
      // LD A,L; SUB lo; LD L,A; LD A,H; SBC A,hi; LD H,A
      auto [Lo, Hi] = getSubRegs16(RHS);
      BuildMI(MBB, MI, DL, get(Z80::LD_A_L));
      BuildMI(MBB, MI, DL, get(getSUBOpcode(Lo)));
      BuildMI(MBB, MI, DL, get(Z80::LD_L_A));
      BuildMI(MBB, MI, DL, get(Z80::LD_A_H));
      BuildMI(MBB, MI, DL, get(getSBCOpcode(Hi)));
      BuildMI(MBB, MI, DL, get(Z80::LD_H_A));
    } else {
      // Z80: AND A; SBC HL,rr — atomic to prevent FLAGS clobbering.
      unsigned SbcOpc = (RHS == Z80::BC) ? Z80::SBC_HL_BC : Z80::SBC_HL_DE;
      bool ADead = aIsDeadInto(MI, TRI);
      auto AndA = BuildMI(MBB, MI, DL, get(Z80::AND_A));
      if (ADead)
        markAReadUndef(AndA); // AND A only clears carry; A value don't-care
      BuildMI(MBB, MI, DL, get(SbcOpc));
    }
    MI.eraseFromParent();
    return true;
  }

  case Z80::SADD_HL_rr: {
    // Signed 16-bit addition (sets P/V for overflow on Z80).
    Register RHS = MI.getOperand(0).getReg();
    if (STI->hasSM83()) {
      // SM83: byte-by-byte ADD/ADC (no ADC HL,rr; no P/V flag).
      // LD A,L; ADD A,lo; LD L,A; LD A,H; ADC A,hi; LD H,A
      auto [Lo, Hi] = getSubRegs16(RHS);
      BuildMI(MBB, MI, DL, get(Z80::LD_A_L));
      BuildMI(MBB, MI, DL, get(getADD8Opcode(Lo)));
      BuildMI(MBB, MI, DL, get(Z80::LD_L_A));
      BuildMI(MBB, MI, DL, get(Z80::LD_A_H));
      BuildMI(MBB, MI, DL, get(getADCOpcode(Hi)));
      BuildMI(MBB, MI, DL, get(Z80::LD_H_A));
    } else {
      // Z80: AND A; ADC HL,rr — sets P/V for overflow detection.
      unsigned AdcOpc = (RHS == Z80::BC) ? Z80::ADC_HL_BC : Z80::ADC_HL_DE;
      bool ADead = aIsDeadInto(MI, TRI);
      auto AndA = BuildMI(MBB, MI, DL, get(Z80::AND_A));
      if (ADead)
        markAReadUndef(AndA); // AND A only clears carry; A value don't-care
      BuildMI(MBB, MI, DL, get(AdcOpc));
    }
    MI.eraseFromParent();
    return true;
  }

  case Z80::ADD_HL_rr_CO: {
    // ADD HL,rr; SBC A,A; AND 1 — carry out in A.
    Register RHS = MI.getOperand(0).getReg();
    unsigned AddOpc;
    if (RHS == Z80::BC)
      AddOpc = Z80::ADD_HL_BC;
    else if (RHS == Z80::DE)
      AddOpc = Z80::ADD_HL_DE;
    else
      llvm_unreachable("ADD_HL_rr_CO: unexpected register");
    BuildMI(MBB, MI, DL, get(AddOpc));
    { auto SbcAA = BuildMI(MBB, MI, DL, get(Z80::SBC_A_A)); markAReadUndef(SbcAA); }
    BuildMI(MBB, MI, DL, get(Z80::AND_n)).addImm(1);
    MI.eraseFromParent();
    return true;
  }

  case Z80::SUB_HL_rr_BO: {
    // 16-bit subtraction with borrow out: HL = HL - rr, A = borrow.
    Register RHS = MI.getOperand(0).getReg();
    if (STI->hasSM83()) {
      // SM83: byte-by-byte SUB/SBC + capture borrow.
      // LD A,L; SUB lo; LD L,A; LD A,H; SBC A,hi; LD H,A; SBC A,A; AND 1
      auto [Lo, Hi] = getSubRegs16(RHS);
      BuildMI(MBB, MI, DL, get(Z80::LD_A_L));
      BuildMI(MBB, MI, DL, get(getSUBOpcode(Lo)));
      BuildMI(MBB, MI, DL, get(Z80::LD_L_A));
      BuildMI(MBB, MI, DL, get(Z80::LD_A_H));
      BuildMI(MBB, MI, DL, get(getSBCOpcode(Hi)));
      BuildMI(MBB, MI, DL, get(Z80::LD_H_A));
    } else {
      // Z80: AND A; SBC HL,rr
      unsigned SbcOpc = (RHS == Z80::BC) ? Z80::SBC_HL_BC : Z80::SBC_HL_DE;
      bool ADead = aIsDeadInto(MI, TRI);
      auto AndA = BuildMI(MBB, MI, DL, get(Z80::AND_A));
      if (ADead)
        markAReadUndef(AndA); // AND A only clears carry; A value don't-care
      BuildMI(MBB, MI, DL, get(SbcOpc));
    }
    // Capture borrow out: SBC A,A; AND 1
    { auto SbcAA = BuildMI(MBB, MI, DL, get(Z80::SBC_A_A)); markAReadUndef(SbcAA); }
    BuildMI(MBB, MI, DL, get(Z80::AND_n)).addImm(1);
    MI.eraseFromParent();
    return true;
  }

  case Z80::ADC_HL_rr_CIO: {
    // 16-bit add with carry in/out: HL = HL + rr + carry_in, A = carry_out.
    Register RHS = MI.getOperand(0).getReg();
    Register CarryReg = MI.getOperand(1).getReg();
    // Restore carry flag from carry_in register: LD A,carry; RRCA
    if (CarryReg != Z80::A) {
      unsigned LdOpc = Z80::getLD8RegOpcode(Z80::A, CarryReg);
      assert(LdOpc && "unexpected carry register for ADC_HL_rr_CIO");
      BuildMI(MBB, MI, DL, get(LdOpc));
    }
    BuildMI(MBB, MI, DL, get(Z80::RRCA));
    if (STI->hasSM83()) {
      // SM83: byte-by-byte ADC (carry flag set by RRCA above).
      // LD A,L; ADC A,lo; LD L,A; LD A,H; ADC A,hi; LD H,A
      auto [Lo, Hi] = getSubRegs16(RHS);
      BuildMI(MBB, MI, DL, get(Z80::LD_A_L));
      BuildMI(MBB, MI, DL, get(getADCOpcode(Lo)));
      BuildMI(MBB, MI, DL, get(Z80::LD_L_A));
      BuildMI(MBB, MI, DL, get(Z80::LD_A_H));
      BuildMI(MBB, MI, DL, get(getADCOpcode(Hi)));
      BuildMI(MBB, MI, DL, get(Z80::LD_H_A));
    } else {
      // Z80: ADC HL,rr (reads carry from RRCA above).
      unsigned AdcOpc = (RHS == Z80::BC) ? Z80::ADC_HL_BC : Z80::ADC_HL_DE;
      BuildMI(MBB, MI, DL, get(AdcOpc));
    }
    // Capture carry out: SBC A,A; AND 1
    { auto SbcAA = BuildMI(MBB, MI, DL, get(Z80::SBC_A_A)); markAReadUndef(SbcAA); }
    BuildMI(MBB, MI, DL, get(Z80::AND_n)).addImm(1);
    MI.eraseFromParent();
    return true;
  }

  case Z80::SBC_HL_rr_BIO: {
    // 16-bit sub with borrow in/out: HL = HL - rr - borrow_in, A = borrow_out.
    Register RHS = MI.getOperand(0).getReg();
    Register BorrowReg = MI.getOperand(1).getReg();
    // Restore borrow flag from borrow_in register: LD A,borrow; RRCA
    if (BorrowReg != Z80::A) {
      unsigned LdOpc = Z80::getLD8RegOpcode(Z80::A, BorrowReg);
      assert(LdOpc && "unexpected borrow register for SBC_HL_rr_BIO");
      BuildMI(MBB, MI, DL, get(LdOpc));
    }
    BuildMI(MBB, MI, DL, get(Z80::RRCA));
    if (STI->hasSM83()) {
      // SM83: byte-by-byte SBC (borrow flag set by RRCA above).
      // LD A,L; SBC A,lo; LD L,A; LD A,H; SBC A,hi; LD H,A
      auto [Lo, Hi] = getSubRegs16(RHS);
      BuildMI(MBB, MI, DL, get(Z80::LD_A_L));
      BuildMI(MBB, MI, DL, get(getSBCOpcode(Lo)));
      BuildMI(MBB, MI, DL, get(Z80::LD_L_A));
      BuildMI(MBB, MI, DL, get(Z80::LD_A_H));
      BuildMI(MBB, MI, DL, get(getSBCOpcode(Hi)));
      BuildMI(MBB, MI, DL, get(Z80::LD_H_A));
    } else {
      // Z80: SBC HL,rr (reads borrow from RRCA above).
      unsigned SbcOpc = (RHS == Z80::BC) ? Z80::SBC_HL_BC : Z80::SBC_HL_DE;
      BuildMI(MBB, MI, DL, get(SbcOpc));
    }
    // Capture borrow out: SBC A,A; AND 1
    { auto SbcAA = BuildMI(MBB, MI, DL, get(Z80::SBC_A_A)); markAReadUndef(SbcAA); }
    BuildMI(MBB, MI, DL, get(Z80::AND_n)).addImm(1);
    MI.eraseFromParent();
    return true;
  }

  case Z80::ADD_A_r:
  case Z80::SUB_r:
  case Z80::AND_r:
  case Z80::OR_r:
  case Z80::XOR_r:
  case Z80::CP_r: {
    // 8-bit ALU pseudo: select concrete opcode based on allocated register.
    Register RHS = MI.getOperand(0).getReg();
    static const unsigned AluOpcodes[][7] = {
        // A,       B,       C,       D,       E,       H,       L
        {Z80::ADD_A_A, Z80::ADD_A_B, Z80::ADD_A_C, Z80::ADD_A_D, Z80::ADD_A_E,
         Z80::ADD_A_H, Z80::ADD_A_L}, // ADD_A_r
        {Z80::SUB_A, Z80::SUB_B, Z80::SUB_C, Z80::SUB_D, Z80::SUB_E, Z80::SUB_H,
         Z80::SUB_L}, // SUB_r
        {Z80::AND_A, Z80::AND_B, Z80::AND_C, Z80::AND_D, Z80::AND_E, Z80::AND_H,
         Z80::AND_L}, // AND_r
        {Z80::OR_A, Z80::OR_B, Z80::OR_C, Z80::OR_D, Z80::OR_E, Z80::OR_H,
         Z80::OR_L}, // OR_r
        {Z80::XOR_A, Z80::XOR_B, Z80::XOR_C, Z80::XOR_D, Z80::XOR_E, Z80::XOR_H,
         Z80::XOR_L}, // XOR_r
        {Z80::CP_A, Z80::CP_B, Z80::CP_C, Z80::CP_D, Z80::CP_E, Z80::CP_H,
         Z80::CP_L}, // CP_r
    };
    unsigned TableIdx;
    switch (MI.getOpcode()) {
    case Z80::ADD_A_r:
      TableIdx = 0;
      break;
    case Z80::SUB_r:
      TableIdx = 1;
      break;
    case Z80::AND_r:
      TableIdx = 2;
      break;
    case Z80::OR_r:
      TableIdx = 3;
      break;
    case Z80::XOR_r:
      TableIdx = 4;
      break;
    case Z80::CP_r:
      TableIdx = 5;
      break;
    default:
      llvm_unreachable("unexpected 8-bit ALU pseudo");
    }
    int RegIdx = Z80::gr8RegToIndex(Register(RHS));
    if (RegIdx >= 0) {
      BuildMI(MBB, MI, DL, get(AluOpcodes[TableIdx][RegIdx]));
    } else {
      // Undocumented IXH/IXL/IYH/IYL: lookup via individual helper
      unsigned Opc = 0;
      switch (MI.getOpcode()) {
      case Z80::ADD_A_r: Opc = getADD8Opcode(Register(RHS)); break;
      case Z80::SUB_r:   Opc = getSUBOpcode(Register(RHS)); break;
      case Z80::AND_r:
        switch (RHS.id()) {
        case Z80::IXH: Opc = Z80::AND_IXH; break; case Z80::IXL: Opc = Z80::AND_IXL; break;
        case Z80::IYH: Opc = Z80::AND_IYH; break; case Z80::IYL: Opc = Z80::AND_IYL; break;
        default: break;
        } break;
      case Z80::OR_r:
        switch (RHS.id()) {
        case Z80::IXH: Opc = Z80::OR_IXH; break; case Z80::IXL: Opc = Z80::OR_IXL; break;
        case Z80::IYH: Opc = Z80::OR_IYH; break; case Z80::IYL: Opc = Z80::OR_IYL; break;
        default: break;
        } break;
      case Z80::XOR_r:
        switch (RHS.id()) {
        case Z80::IXH: Opc = Z80::XOR_IXH; break; case Z80::IXL: Opc = Z80::XOR_IXL; break;
        case Z80::IYH: Opc = Z80::XOR_IYH; break; case Z80::IYL: Opc = Z80::XOR_IYL; break;
        default: break;
        } break;
      case Z80::CP_r:
        switch (RHS.id()) {
        case Z80::IXH: Opc = Z80::CP_IXH; break; case Z80::IXL: Opc = Z80::CP_IXL; break;
        case Z80::IYH: Opc = Z80::CP_IYH; break; case Z80::IYL: Opc = Z80::CP_IYL; break;
        default: break;
        } break;
      default: break;
      }
      assert(Opc && "unexpected register for 8-bit ALU pseudo");
      BuildMI(MBB, MI, DL, get(Opc));
    }
    MI.eraseFromParent();
    return true;
  }

  case Z80::CAPTURE_PV: {
    // Read P/V flag (bit 2 of F register) into A as 0 or 1.
    // PUSH AF; POP HL; LD A,L; RRCA; RRCA; AND 1
    // PUSH/POP/LD don't affect flags, so P/V is preserved until RRCA.
    BuildMI(MBB, MI, DL, get(Z80::PUSH_AF));
    BuildMI(MBB, MI, DL, get(Z80::POP_HL));
    BuildMI(MBB, MI, DL, get(Z80::LD_A_L));
    BuildMI(MBB, MI, DL, get(Z80::RRCA));
    BuildMI(MBB, MI, DL, get(Z80::RRCA));
    BuildMI(MBB, MI, DL, get(Z80::AND_n)).addImm(1);
    MI.eraseFromParent();
    return true;
  }

  case Z80::SM83_SADDO_HL_rr: {
    // SM83 signed 16-bit add with overflow detection.
    // HL = HL + rr, A = overflow (0 or 1).
    // overflow = (result_hi ^ lhs_hi) & (result_hi ^ rhs_hi), bit 7
    Register RHS = MI.getOperand(0).getReg();
    auto [Lo, Hi] = getSubRegs16(RHS);
    // Use a temp from the "other" register pair.
    Register Temp = (RHS == Z80::DE) ? Z80::B : Z80::D;
    // Save lhs_hi before the addition overwrites H.
    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Temp, Z80::H)));
    // HL = HL + rr (byte-by-byte)
    BuildMI(MBB, MI, DL, get(Z80::LD_A_L));
    BuildMI(MBB, MI, DL, get(getADD8Opcode(Lo)));
    BuildMI(MBB, MI, DL, get(Z80::LD_L_A));
    BuildMI(MBB, MI, DL, get(Z80::LD_A_H));
    BuildMI(MBB, MI, DL, get(getADCOpcode(Hi)));
    BuildMI(MBB, MI, DL, get(Z80::LD_H_A));
    // A = result_hi. Compute overflow:
    // T1 = result_hi ^ lhs_hi (Temp has lhs_hi)
    unsigned XorTemp = (Temp == Z80::B) ? Z80::XOR_B : Z80::XOR_D;
    BuildMI(MBB, MI, DL, get(XorTemp)); // A = result_hi ^ lhs_hi
    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Temp, Z80::A))); // Temp = T1
    BuildMI(MBB, MI, DL, get(Z80::LD_A_H)); // A = result_hi
    // T2 = result_hi ^ rhs_hi
    unsigned XorHi = (Hi == Z80::B) ? Z80::XOR_B : Z80::XOR_D;
    BuildMI(MBB, MI, DL, get(XorHi)); // A = result_hi ^ rhs_hi
    // A = T1 & T2
    unsigned AndTemp = (Temp == Z80::B) ? Z80::AND_B : Z80::AND_D;
    BuildMI(MBB, MI, DL, get(AndTemp)); // A = (res^lhs) & (res^rhs)
    // Bit 7 → bit 0
    BuildMI(MBB, MI, DL, get(Z80::RLCA));
    BuildMI(MBB, MI, DL, get(Z80::AND_n)).addImm(1);
    MI.eraseFromParent();
    return true;
  }

  case Z80::SM83_SSUBO_HL_rr: {
    // SM83 signed 16-bit sub with overflow detection.
    // HL = HL - rr, A = overflow (0 or 1).
    // overflow = (result_hi ^ lhs_hi) & (lhs_hi ^ rhs_hi), bit 7
    Register RHS = MI.getOperand(0).getReg();
    auto [Lo, Hi] = getSubRegs16(RHS);
    // Use temps from the "other" register pair.
    Register Temp1 = (RHS == Z80::DE) ? Z80::B : Z80::D; // lhs_hi
    Register Temp2 = (RHS == Z80::DE) ? Z80::C : Z80::E; // XOR intermediate
    // Save lhs_hi before the subtraction overwrites H.
    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Temp1, Z80::H)));
    // HL = HL - rr (byte-by-byte)
    BuildMI(MBB, MI, DL, get(Z80::LD_A_L));
    BuildMI(MBB, MI, DL, get(getSUBOpcode(Lo)));
    BuildMI(MBB, MI, DL, get(Z80::LD_L_A));
    BuildMI(MBB, MI, DL, get(Z80::LD_A_H));
    BuildMI(MBB, MI, DL, get(getSBCOpcode(Hi)));
    BuildMI(MBB, MI, DL, get(Z80::LD_H_A));
    // A = result_hi. Compute overflow:
    // T1 = result_hi ^ lhs_hi (Temp1 has lhs_hi)
    unsigned XorTemp1 = (Temp1 == Z80::B) ? Z80::XOR_B : Z80::XOR_D;
    BuildMI(MBB, MI, DL, get(XorTemp1)); // A = result_hi ^ lhs_hi
    BuildMI(MBB, MI, DL,
            get(Z80::getLD8RegOpcode(Temp2, Z80::A))); // Temp2 = T1
    // T2 = lhs_hi ^ rhs_hi (need lhs_hi again, still in Temp1)
    BuildMI(MBB, MI, DL,
            get(Z80::getLD8RegOpcode(Z80::A, Temp1))); // A = lhs_hi
    unsigned XorHi = (Hi == Z80::B) ? Z80::XOR_B : Z80::XOR_D;
    BuildMI(MBB, MI, DL, get(XorHi)); // A = lhs_hi ^ rhs_hi
    // A = T1 & T2
    unsigned AndTemp2 = (Temp2 == Z80::C) ? Z80::AND_C : Z80::AND_E;
    BuildMI(MBB, MI, DL, get(AndTemp2)); // A = (res^lhs) & (lhs^rhs)
    // Bit 7 → bit 0
    BuildMI(MBB, MI, DL, get(Z80::RLCA));
    BuildMI(MBB, MI, DL, get(Z80::AND_n)).addImm(1);
    MI.eraseFromParent();
    return true;
  }

  case Z80::LSHR16:
  case Z80::ASHR16: {
    // 16-bit shift right by 1.
    // LSHR16: SRL hi; RR lo  (logical)
    // ASHR16: SRA hi; RR lo  (arithmetic, preserves sign)
    Register Reg = MI.getOperand(0).getReg();
    Register Hi = TRI->getSubReg(Reg, Z80::sub_hi);
    Register Lo = TRI->getSubReg(Reg, Z80::sub_lo);

    bool IsLogical = (MI.getOpcode() == Z80::LSHR16);
    BuildMI(MBB, MI, DL, get(IsLogical ? getSRLOpcode(Hi) : getSRAOpcode(Hi)));
    BuildMI(MBB, MI, DL, get(getRROpcode(Lo)));
    MI.eraseFromParent();
    return true;
  }

  case Z80::CALL_IY: {
    // Z80: Indirect call through IY register.
    // Expand to: CALL __call_iy (runtime trampoline that does JP (IY))
    MachineFunction &MF = *MBB.getParent();
    MCContext &Ctx = MF.getContext();
    MCSymbol *Sym = Ctx.getOrCreateSymbol("__call_iy");
    auto NewMI = BuildMI(MBB, MI, DL, get(Z80::CALL_nn)).addSym(Sym);
    // Copy implicit operands (argument registers)
    for (const MachineOperand &MO : MI.implicit_operands()) {
      if (MO.isReg() && MO.isUse())
        NewMI.addReg(MO.getReg(), RegState::Implicit);
    }
    MI.eraseFromParent();
    return true;
  }

  case Z80::CALL_HL: {
    // SM83: Indirect call through HL register.
    // Expand to: CALL __call_hl
    // The runtime trampoline is just JP (HL) — no register shift needed.
    MachineFunction &MF = *MBB.getParent();
    MCContext &Ctx = MF.getContext();
    MCSymbol *Sym = Ctx.getOrCreateSymbol("__call_hl");
    auto NewMI = BuildMI(MBB, MI, DL, get(Z80::CALL_nn)).addSym(Sym);
    for (const MachineOperand &MO : MI.implicit_operands()) {
      if (MO.isReg() && MO.isUse())
        NewMI.addReg(MO.getReg(), RegState::Implicit);
    }
    MI.eraseFromParent();
    return true;
  }

  case Z80::RET_CLEANUP: {
    // Callee-cleanup return: pop return address, skip N bytes of stack args,
    // then return via indirect jump.
    unsigned Amount = MI.getOperand(0).getImm();
    if (Amount == 0) {
      BuildMI(MBB, MI, DL, get(Z80::RET));
    } else if (STI->hasSM83()) {
      // SM83: POP HL; ADD SP,e; JP (HL)
      BuildMI(MBB, MI, DL, get(Z80::POP_HL));
      BuildMI(MBB, MI, DL, get(Z80::ADD_SP_e)).addImm(Amount & 0xFF);
      BuildMI(MBB, MI, DL, get(Z80::JP_HLind));
    } else {
      // For any N ≥ 2: if HL is dead (not holding a return value), use the
      // EX (SP),HL trick — saves 2 B regardless of N (#146):
      //   POP HL                 ; HL = ret addr, SP → args
      //   INC SP × (N-2)         ; skip first N-2 arg bytes
      //   EX (SP),HL             ; (SP) = ret addr, HL = garbage
      //   RET                    ; jump to ret addr, SP past all args
      //
      // If HL is live (holds return value), fall back to the safe BC sequence:
      //   POP BC; INC SP × N; PUSH BC; RET   (small N ≤ 8)
      //   POP BC; LD HL,N; ADD HL,SP; LD SP,HL; PUSH BC; RET  (large N)
      // HL is safe to clobber when the return value doesn't live there.
      // sdcccall(1) places: i8/bool → A (or L); i16 → HL; i32/float → HLDE.
      // For i16 and i32 returns HL carries return data — skip the trick.
      // For void, i8, and all other types HL is free.
      const Function &F = MBB.getParent()->getFunction();
      const Type *RetTy = F.getReturnType();
      // sdcccall(1) return registers: i8/bool→A, i16/ptr→HL, i32/float→HLDE.
      // getPrimitiveSizeInBits() returns 0 for pointers; treat them as 16-bit.
      unsigned RetBits = RetTy->isPointerTy() ? 16
                                              : RetTy->getPrimitiveSizeInBits();
      bool HLDead = !RetTy->isVoidTy() ? (RetBits != 16 && RetBits != 32)
                                        : true;

      if (HLDead && Amount >= 2) {
        // POP HL; INC SP × (N-2); EX (SP),HL; RET  (ravn/llvm-z80#146)
        BuildMI(MBB, MI, DL, get(Z80::POP_HL));
        for (unsigned i = 0; i < Amount - 2; ++i)
          BuildMI(MBB, MI, DL, get(Z80::INC_SP));
        BuildMI(MBB, MI, DL, get(Z80::EX_SPind_HL));
        BuildMI(MBB, MI, DL, get(Z80::RET));
      } else if (Amount <= 8) {
        // Z80 small: POP BC; INC SP × N; PUSH BC; RET
        // Use BC (not HL) because HL holds a return value.
        BuildMI(MBB, MI, DL, get(Z80::POP_BC));
        for (unsigned i = 0; i < Amount; ++i)
          BuildMI(MBB, MI, DL, get(Z80::INC_SP));
        BuildMI(MBB, MI, DL, get(Z80::PUSH_BC));
        BuildMI(MBB, MI, DL, get(Z80::RET));
      } else {
        // Z80 large: POP BC; LD HL,N; ADD HL,SP; LD SP,HL; PUSH BC; RET
        BuildMI(MBB, MI, DL, get(Z80::POP_BC));
        BuildMI(MBB, MI, DL, get(Z80::LD_HL_nn)).addImm(Amount);
        BuildMI(MBB, MI, DL, get(Z80::ADD_HL_SP));
        BuildMI(MBB, MI, DL, get(Z80::LD_SP_HL));
        BuildMI(MBB, MI, DL, get(Z80::PUSH_BC));
        BuildMI(MBB, MI, DL, get(Z80::RET));
      }
    }
    MI.eraseFromParent();
    return true;
  }

  case Z80::SEXT16: {
    // Sign extension: 16-bit register → all sign bits (0x0000 or 0xFFFF)
    // LD A,src_hi; ADD A,A; SBC A,A; LD dst_lo,A; LD dst_hi,A
    Register DstReg = MI.getOperand(0).getReg();
    Register SrcReg = MI.getOperand(1).getReg();
    bool SrcIsIR = Z80::IR16RegClass.contains(SrcReg);
    bool DstIsIR = Z80::IR16RegClass.contains(DstReg);

    // Without +undocumented, IX/IY sub-registers (IXH/IXL/IYH/IYL) cannot be
    // accessed directly.  Route through HL via PUSH/POP.
    if ((SrcIsIR || DstIsIR) && !STI->hasUndocumented()) {
      LivePhysRegs LiveRegs(*TRI);
      LiveRegs.addLiveOuts(MBB);
      for (auto I = MBB.rbegin(); &*I != &MI; ++I)
        LiveRegs.stepBackward(*I);
      bool HLive = LiveRegs.contains(Z80::H);
      bool LLive = LiveRegs.contains(Z80::L);
      bool HLLive = HLive || LLive;

      // Step 1: Get source high byte into A.
      if (SrcIsIR) {
        // Extract via PUSH IX/IY; POP HL; LD A,H
        // (HL might be the destination — save it if live and not dst)
        if (HLLive && DstReg != Z80::HL) {
          // IMPLICIT_DEF dead half before PUSH_HL (ravn/llvm-z80#239 site 4).
          if (!HLive)
            BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::H);
          if (!LLive)
            BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::L);
          BuildMI(MBB, MI, DL, get(Z80::PUSH_HL));
        }
        BuildMI(MBB, MI, DL, get(Z80::getPushOpcode(SrcReg)));
        BuildMI(MBB, MI, DL, get(Z80::POP_HL));
        BuildMI(MBB, MI, DL, get(Z80::LD_A_H));
        if (HLLive && DstReg != Z80::HL)
          BuildMI(MBB, MI, DL, get(Z80::POP_HL));
      } else {
        Register SrcHi = TRI->getSubReg(SrcReg, Z80::sub_hi);
        BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Z80::A, SrcHi)));
      }

      // Step 2: ADD A,A; SBC A,A — compute sign extension byte
      BuildMI(MBB, MI, DL, get(Z80::ADD_A_A));
      { auto SbcAA = BuildMI(MBB, MI, DL, get(Z80::SBC_A_A)); markAReadUndef(SbcAA); }

      // Step 3: Write A to both bytes of destination.
      if (DstIsIR) {
        // Build result in HL, transfer via PUSH HL; POP IX/IY
        if (HLLive) {
          // IMPLICIT_DEF dead half before PUSH_HL (ravn/llvm-z80#239 site 5).
          if (!HLive)
            BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::H);
          if (!LLive)
            BuildMI(MBB, MI, DL, get(TargetOpcode::IMPLICIT_DEF), Z80::L);
          BuildMI(MBB, MI, DL, get(Z80::PUSH_HL));
        }
        BuildMI(MBB, MI, DL, get(Z80::LD_L_A));
        BuildMI(MBB, MI, DL, get(Z80::LD_H_A));
        BuildMI(MBB, MI, DL, get(Z80::PUSH_HL));
        BuildMI(MBB, MI, DL, get(Z80::getPopOpcode(DstReg)));
        if (HLLive)
          BuildMI(MBB, MI, DL, get(Z80::POP_HL));
      } else {
        Register DstLo = TRI->getSubReg(DstReg, Z80::sub_lo);
        Register DstHi = TRI->getSubReg(DstReg, Z80::sub_hi);
        BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(DstLo, Z80::A)));
        BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(DstHi, Z80::A)));
      }

      MI.eraseFromParent();
      return true;
    }

    Register SrcHi = TRI->getSubReg(SrcReg, Z80::sub_hi);
    Register DstHi = TRI->getSubReg(DstReg, Z80::sub_hi);
    Register DstLo = TRI->getSubReg(DstReg, Z80::sub_lo);

    // LD A, src_hi - read the sign byte
    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(Z80::A, SrcHi)));
    // ADD A,A - shift sign bit into carry
    BuildMI(MBB, MI, DL, get(Z80::ADD_A_A));
    // SBC A,A - A = 0xFF if carry (negative), 0x00 if no carry (positive)
    { auto SbcAA = BuildMI(MBB, MI, DL, get(Z80::SBC_A_A)); markAReadUndef(SbcAA); }
    // LD dst_lo, A; LD dst_hi, A
    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(DstLo, Z80::A)));
    BuildMI(MBB, MI, DL, get(Z80::getLD8RegOpcode(DstHi, Z80::A)));

    MI.eraseFromParent();
    return true;
  }

  case Z80::INC16:
  case Z80::DEC16: {
    // INC16/DEC16 pseudo: expand to the correct INC/DEC based on physical reg
    Register Reg = MI.getOperand(0).getReg();
    unsigned Opc;
    bool IsInc = (MI.getOpcode() == Z80::INC16);
    if (Reg == Z80::BC)
      Opc = IsInc ? Z80::INC_BC : Z80::DEC_BC;
    else if (Reg == Z80::DE)
      Opc = IsInc ? Z80::INC_DE : Z80::DEC_DE;
    else if (Reg == Z80::HL)
      Opc = IsInc ? Z80::INC_HL : Z80::DEC_HL;
    else if (Reg == Z80::IX)
      Opc = IsInc ? Z80::INC_IX : Z80::DEC_IX;
    else if (Reg == Z80::IY)
      Opc = IsInc ? Z80::INC_IY : Z80::DEC_IY;
    else
      return false;
    BuildMI(MBB, MI, DL, get(Opc));
    MI.eraseFromParent();
    return true;
  }

  default:
    return false;
  }
}

bool Z80InstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                 MachineBasicBlock *&TBB,
                                 MachineBasicBlock *&FBB,
                                 SmallVectorImpl<MachineOperand> &Cond,
                                 bool AllowModify) const {
  // Look at the last instructions of the block
  MachineBasicBlock::iterator I = MBB.end();
  while (I != MBB.begin()) {
    --I;

    if (I->isDebugInstr())
      continue;

    // Not a terminator - stop analyzing
    if (!I->isTerminator())
      break;

    // Handle unconditional branches
    if (I->getOpcode() == Z80::JP_nn || I->getOpcode() == Z80::JR_e) {
      if (TBB) {
        // Already have a target - this is the fallthrough case with conditional
        FBB = I->getOperand(0).getMBB();
      } else {
        TBB = I->getOperand(0).getMBB();
      }
      continue;
    }

    // Handle conditional branches
    if (I->getOpcode() == Z80::JP_Z_nn || I->getOpcode() == Z80::JP_NZ_nn ||
        I->getOpcode() == Z80::JP_C_nn || I->getOpcode() == Z80::JP_NC_nn ||
        I->getOpcode() == Z80::JR_Z_e || I->getOpcode() == Z80::JR_NZ_e ||
        I->getOpcode() == Z80::JR_C_e || I->getOpcode() == Z80::JR_NC_e) {
      if (!Cond.empty()) {
        // Already saw a conditional branch - can't analyze
        return true;
      }
      if (TBB) {
        // Unconditional branch was already seen (iterating backward).
        // Pattern: cond_br TrueTarget; jp FalseTarget
        // TBB currently holds FalseTarget from the unconditional JP.
        FBB = TBB;
      }
      TBB = I->getOperand(0).getMBB();
      Cond.push_back(MachineOperand::CreateImm(I->getOpcode()));
      continue;
    }

    // DJNZ combines DEC B (side effect) with JR NZ (branch). It can't be
    // removed/re-inserted by the branch framework without losing the DEC B,
    // so we report the block as unanalyzable. BranchRelaxation will still
    // check isBranchOffsetInRange (DJNZ has ±127, same as JR) and leave it
    // alone when in range.
    if (I->getOpcode() == Z80::DJNZ_e)
      return true;

    // Unknown terminator
    return true;
  }

  return false;
}

unsigned Z80InstrInfo::insertBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *TBB,
                                    MachineBasicBlock *FBB,
                                    ArrayRef<MachineOperand> Cond,
                                    const DebugLoc &DL, int *BytesAdded) const {
  assert(TBB && "insertBranch requires a target block");
  unsigned Count = 0;

  // Try to convert a conditional branch opcode to its JR form (2 bytes).
  // Returns the original JP opcode if no JR equivalent exists (P/M/PE/PO).
  auto toShortCond = [](unsigned Opc) -> unsigned {
    switch (Opc) {
    case Z80::JP_Z_nn:
    case Z80::JR_Z_e:
      return Z80::JR_Z_e;
    case Z80::JP_NZ_nn:
    case Z80::JR_NZ_e:
      return Z80::JR_NZ_e;
    case Z80::JP_C_nn:
    case Z80::JR_C_e:
      return Z80::JR_C_e;
    case Z80::JP_NC_nn:
    case Z80::JR_NC_e:
      return Z80::JR_NC_e;
    default:
      return Opc; // P/M/PE/PO — no JR form, keep JP
    }
  };

  if (Cond.empty()) {
    // Unconditional branch - emit JR (2 bytes), relaxed to JP if needed
    BuildMI(&MBB, DL, get(Z80::JR_e)).addMBB(TBB);
    ++Count;
    if (BytesAdded)
      *BytesAdded = 2;
  } else {
    // Conditional branch - emit JR cc (2 bytes) if possible, else JP cc (3)
    unsigned CondOpc = toShortCond(Cond[0].getImm());
    bool IsShort = CondOpc != Cond[0].getImm() || CondOpc == Z80::JR_Z_e ||
                   CondOpc == Z80::JR_NZ_e || CondOpc == Z80::JR_C_e ||
                   CondOpc == Z80::JR_NC_e;
    BuildMI(&MBB, DL, get(CondOpc)).addMBB(TBB);
    ++Count;
    if (BytesAdded)
      *BytesAdded = IsShort ? 2 : 3;

    if (FBB) {
      // Fallthrough branch
      BuildMI(&MBB, DL, get(Z80::JR_e)).addMBB(FBB);
      ++Count;
      if (BytesAdded)
        *BytesAdded += 2;
    }
  }

  return Count;
}

unsigned Z80InstrInfo::removeBranch(MachineBasicBlock &MBB,
                                    int *BytesRemoved) const {
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;
  int Removed = 0;

  while (I != MBB.begin()) {
    --I;

    if (I->isDebugInstr())
      continue;

    unsigned Opc = I->getOpcode();
    bool isJP = Opc == Z80::JP_nn || Opc == Z80::JP_Z_nn ||
                Opc == Z80::JP_NZ_nn || Opc == Z80::JP_C_nn ||
                Opc == Z80::JP_NC_nn;
    bool isJR = Opc == Z80::JR_e || Opc == Z80::JR_Z_e || Opc == Z80::JR_NZ_e ||
                Opc == Z80::JR_C_e || Opc == Z80::JR_NC_e;
    // DJNZ has a DEC B side effect. analyzeBranch marks DJNZ blocks as
    // unanalyzable, so BranchRelaxation won't call removeBranch for them.
    // Stop here to preserve the side effect if any other pass calls us.
    if (Opc == Z80::DJNZ_e)
      break;
    if (!isJP && !isJR)
      break;

    Removed += isJR ? 2 : 3;
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  if (BytesRemoved)
    *BytesRemoved = Removed;

  return Count;
}

unsigned Z80InstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  unsigned Opcode = MI.getOpcode();

  // ADD_HL_FI / SUB_HL_FI are expanded during eliminateFrameIndex (PEI),
  // which runs before BranchRelaxation. By the time branch distances matter,
  // these pseudos no longer exist. Return the FP-mode size (10B) as a
  // reasonable estimate for any pre-PEI pass that queries instruction size.
  // SP-relative mode expands to ~16B but is uncommon.
  if (Opcode == Z80::ADD_HL_FI || Opcode == Z80::SUB_HL_FI)
    return 10;

  // Inline-runtime pseudos expand to multiple MBBs after BranchRelaxation.
  // Report exact expanded sizes so BranchRelaxation can make precise decisions.
  // SM83 expansions are larger due to lacking ADC HL,HL / SBC HL,DE / EX DE,HL
  // / DJNZ and requiring byte-wise emulation + PUSH/POP AF for counter.
  {
    const auto &STI =
        MI.getParent()->getParent()->getSubtarget<Z80Subtarget>();
    bool IsSM83 = STI.hasSM83();
    switch (Opcode) {
    case Z80::MUL16:  return IsSM83 ? 24 : 20;
    case Z80::UDIV16: return IsSM83 ? 45 : 32;
    case Z80::UMOD16: return IsSM83 ? 45 : 31;
    case Z80::SDIV16: return IsSM83 ? 79 : 66;
    case Z80::SMOD16: return IsSM83 ? 78 : 64;
    default: break;
    }
  }

  // COPY16_PUSHPOP expands to PUSH src (1-2B) + POP dst (1-2B).
  // PUSH/POP GR16 = 1B each, PUSH/POP IX/IY = 2B each (DD/FD prefix).
  if (Opcode == Z80::COPY16_PUSHPOP) {
    Register Src = MI.getOperand(1).getReg();
    Register Dst = MI.getOperand(0).getReg();
    unsigned Size = 0;
    Size += Z80::IR16RegClass.contains(Src) ? 2 : 1; // PUSH
    Size += Z80::IR16RegClass.contains(Dst) ? 2 : 1; // POP
    return Size;
  }

  // Handle pseudo-instructions that have no encoding
  if (MI.isDebugInstr() || MI.isLabel() || MI.isPseudo())
    return 0;

  // COPY is a pseudo that gets eliminated
  if (Opcode == TargetOpcode::COPY)
    return 0;

  // PHI nodes get eliminated
  if (Opcode == TargetOpcode::PHI)
    return 0;

  // Get the MCInstrDesc for size info
  const MCInstrDesc &Desc = MI.getDesc();

  // If the instruction has a fixed size from TableGen, use it
  unsigned Size = Desc.getSize();
  if (Size != 0)
    return Size;

  // Default sizes for Z80 instructions
  switch (Opcode) {
  // Single-byte instructions (most 8-bit ops, register-to-register)
  case Z80::NOP:
  case Z80::HALT:
  case Z80::RET:
  case Z80::LD_A_A:
  case Z80::LD_A_B:
  case Z80::LD_A_C:
  case Z80::LD_A_D:
  case Z80::LD_A_E:
  case Z80::LD_A_H:
  case Z80::LD_A_L:
  case Z80::LD_B_A:
  case Z80::LD_B_B:
  case Z80::LD_B_C:
  case Z80::LD_B_D:
  case Z80::LD_B_E:
  case Z80::LD_B_H:
  case Z80::LD_B_L:
  case Z80::LD_C_A:
  case Z80::LD_C_B:
  case Z80::LD_C_C:
  case Z80::LD_C_D:
  case Z80::LD_C_E:
  case Z80::LD_C_H:
  case Z80::LD_C_L:
  case Z80::LD_D_A:
  case Z80::LD_D_B:
  case Z80::LD_D_C:
  case Z80::LD_D_D:
  case Z80::LD_D_E:
  case Z80::LD_D_H:
  case Z80::LD_D_L:
  case Z80::LD_E_A:
  case Z80::LD_E_B:
  case Z80::LD_E_C:
  case Z80::LD_E_D:
  case Z80::LD_E_E:
  case Z80::LD_E_H:
  case Z80::LD_E_L:
  case Z80::LD_H_A:
  case Z80::LD_H_B:
  case Z80::LD_H_C:
  case Z80::LD_H_D:
  case Z80::LD_H_E:
  case Z80::LD_H_H:
  case Z80::LD_H_L:
  case Z80::LD_L_A:
  case Z80::LD_L_B:
  case Z80::LD_L_C:
  case Z80::LD_L_D:
  case Z80::LD_L_E:
  case Z80::LD_L_H:
  case Z80::LD_L_L:
  case Z80::LD_A_HLind:
  case Z80::LD_HLind_A:
  case Z80::ADD_A_A:
  case Z80::ADD_A_B:
  case Z80::ADD_A_C:
  case Z80::ADD_A_D:
  case Z80::ADD_A_E:
  case Z80::ADD_A_H:
  case Z80::ADD_A_L:
  case Z80::SUB_A:
  case Z80::SUB_B:
  case Z80::SUB_C:
  case Z80::SUB_D:
  case Z80::SUB_E:
  case Z80::SUB_H:
  case Z80::SUB_L:
  case Z80::AND_A:
  case Z80::AND_B:
  case Z80::AND_C:
  case Z80::AND_D:
  case Z80::AND_E:
  case Z80::AND_H:
  case Z80::AND_L:
  case Z80::OR_A:
  case Z80::OR_B:
  case Z80::OR_C:
  case Z80::OR_D:
  case Z80::OR_E:
  case Z80::OR_H:
  case Z80::OR_L:
  case Z80::XOR_A:
  case Z80::XOR_B:
  case Z80::XOR_C:
  case Z80::XOR_D:
  case Z80::XOR_E:
  case Z80::XOR_H:
  case Z80::XOR_L:
  case Z80::CP_A:
  case Z80::CP_B:
  case Z80::CP_C:
  case Z80::CP_D:
  case Z80::CP_E:
  case Z80::CP_H:
  case Z80::CP_L:
  case Z80::INC_A:
  case Z80::INC_B:
  case Z80::INC_C:
  case Z80::INC_D:
  case Z80::INC_E:
  case Z80::INC_H:
  case Z80::INC_L:
  case Z80::DEC_A:
  case Z80::DEC_B:
  case Z80::DEC_C:
  case Z80::DEC_D:
  case Z80::DEC_E:
  case Z80::DEC_H:
  case Z80::DEC_L:
  case Z80::INC_BC:
  case Z80::INC_DE:
  case Z80::INC_HL:
  case Z80::INC_SP:
  case Z80::DEC_BC:
  case Z80::DEC_DE:
  case Z80::DEC_HL:
  case Z80::DEC_SP:
  case Z80::ADD_HL_BC:
  case Z80::ADD_HL_DE:
  case Z80::ADD_HL_HL:
  case Z80::ADD_HL_SP:
  case Z80::ADD_HL_rr: // pseudo → 1-byte ADD HL,rr
  case Z80::ADD_A_r:
  case Z80::SUB_r:
  case Z80::AND_r:
  case Z80::OR_r:
  case Z80::XOR_r:
  case Z80::CP_r: // 8-bit ALU pseudos → 1 byte
  case Z80::PUSH_AF:
  case Z80::PUSH_BC:
  case Z80::PUSH_DE:
  case Z80::PUSH_HL:
  case Z80::POP_AF:
  case Z80::POP_BC:
  case Z80::POP_DE:
  case Z80::POP_HL:
  case Z80::RLCA:
  case Z80::RRCA:
  case Z80::RLA:
  case Z80::RRA:
  case Z80::SCF:
  case Z80::CCF:
  case Z80::SBC_A_A:
    return 1;

  // Two-byte instructions (with immediate or CB prefix)
  case Z80::LD_A_n:
  case Z80::LD_B_n:
  case Z80::LD_C_n:
  case Z80::LD_D_n:
  case Z80::LD_E_n:
  case Z80::LD_H_n:
  case Z80::LD_L_n:
  case Z80::ADD_A_n:
  case Z80::SUB_n:
  case Z80::AND_n:
  case Z80::OR_n:
  case Z80::XOR_n:
  case Z80::CP_n:
  case Z80::SLA_A:
  case Z80::SLA_B:
  case Z80::SLA_C:
  case Z80::SLA_D:
  case Z80::SLA_E:
  case Z80::SLA_H:
  case Z80::SLA_L:
  case Z80::SRA_A:
  case Z80::SRA_B:
  case Z80::SRA_C:
  case Z80::SRA_D:
  case Z80::SRA_E:
  case Z80::SRA_H:
  case Z80::SRA_L:
  case Z80::SRL_A:
  case Z80::SRL_B:
  case Z80::SRL_C:
  case Z80::SRL_D:
  case Z80::SRL_E:
  case Z80::SRL_H:
  case Z80::SRL_L:
  case Z80::SBC_HL_BC:
  case Z80::SBC_HL_DE:
  case Z80::SBC_HL_HL:
  case Z80::SBC_HL_SP:
  case Z80::ADC_HL_BC:
  case Z80::ADC_HL_DE:
  case Z80::ADC_HL_HL:
  case Z80::ADC_HL_SP:
  case Z80::PUSH_IX:
  case Z80::PUSH_IY:
  case Z80::POP_IX:
  case Z80::POP_IY:
    return 2;

  // Three-byte instructions (JP nn, CALL nn, LD rr,nn)
  // Also SUB_HL_xx pseudos (AND A=1 + SBC HL,xx=2 = 3 bytes)
  case Z80::JP_nn:
  case Z80::JP_Z_nn:
  case Z80::JP_NZ_nn:
  case Z80::JP_C_nn:
  case Z80::JP_NC_nn:
  case Z80::CALL_nn:
  case Z80::LD_BC_nn:
  case Z80::LD_DE_nn:
  case Z80::LD_HL_nn:
  case Z80::LD_SP_nn:
  case Z80::SUB_HL_rr:  // AND A(1) + SBC HL,rr(2) = 3
  case Z80::SADD_HL_rr: // AND A(1) + ADC HL,rr(2) = 3
    return 3;

  case Z80::CMP16_FLAGS: // LD A,lo(1) + SUB lo(1) + LD A,hi(1) + SBC A,hi(1) =
                         // 4
  case Z80::CMP16_SBC_FLAGS: // LD A,lo(1) + SBC A,lo(1) + LD A,hi(1) + SBC
                             // A,hi(1) = 4
    return 4;

  case Z80::LOAD8_IND:  // LD A,(rr) = 1
  case Z80::STORE8_IND: // LD (rr),A = 1
    return 1;

  case Z80::ADD_HL_rr_CO: // ADD HL,rr(1) + SBC A,A(1) + AND n(2) = 4
    return 4;

  case Z80::SUB_HL_rr_BO: // AND A(1) + SBC HL,rr(2) + SBC A,A(1) + AND n(2) = 6
    return 6;

  case Z80::CMP16_ULT: // LD A,lo(1) + SUB lo(1) + LD A,hi(1) + SBC A,hi(1) +
                       // SBC A,A(1) + AND 1(2) = 7
    return 7;

  // CAPTURE_PV: PUSH AF(1) + POP HL(1) + LD A,L(1) + RRCA(1) + RRCA(1) + AND
  // n(2) = 7
  case Z80::CAPTURE_PV:
  case Z80::ADC_HL_rr_CIO: // LD A,r(1) + RRCA(1) + ADC HL,rr(2) + SBC A,A(1) +
                           // AND n(2) = 7
  case Z80::SBC_HL_rr_BIO: // LD A,r(1) + RRCA(1) + SBC HL,rr(2) + SBC A,A(1) +
                           // AND n(2) = 7
    return 7;

  // Zero test pseudo
  case Z80::SM83_CMP_ZERO16: // LD A,lo + OR hi = 2
    return 2;

  // XOR-based comparison pseudos
  case Z80::SM83_CMP_Z16: // LD+XOR+LD B,A+LD+XOR+OR B = 6
  case Z80::XOR_CMP_Z16:
    return 6;
  case Z80::XOR_CMP_EQ16: // 6 (XOR chain) + SUB 1(2) + SBC A,A(1) + AND 1(2) =
                          // 11
  case Z80::XOR_CMP_NE16:
    return 11;

  // SM83 signed overflow pseudos: 12 x 1-byte + 1 x 2-byte = 14 bytes
  case Z80::SM83_SADDO_HL_rr:
  case Z80::SM83_SSUBO_HL_rr:
    return 14;

  // Four-byte instructions (IX/IY indexed)
  case Z80::LD_IX_nn:
  case Z80::LD_IY_nn:
    return 4;

  default:
    // For unknown instructions, return a safe default
    return 3;
  }
}

bool Z80InstrInfo::isBranchOffsetInRange(unsigned BranchOpc,
                                         int64_t BrOffset) const {
  switch (BranchOpc) {
  case Z80::JP_nn:
  case Z80::JP_Z_nn:
  case Z80::JP_NZ_nn:
  case Z80::JP_C_nn:
  case Z80::JP_NC_nn:
    return true;
  case Z80::JR_e:
  case Z80::JR_Z_e:
  case Z80::JR_NZ_e:
  case Z80::JR_C_e:
  case Z80::JR_NC_e:
  case Z80::DJNZ_e:
    // JR/DJNZ use a signed 8-bit offset from PC after the 2-byte instruction.
    // BrOffset is from the start of the instruction, so adjust by +2.
    return (BrOffset - 2) >= -128 && (BrOffset - 2) <= 127;
  default:
    return false;
  }
}

MachineBasicBlock *
Z80InstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  case Z80::JP_nn:
  case Z80::JP_Z_nn:
  case Z80::JP_NZ_nn:
  case Z80::JP_C_nn:
  case Z80::JP_NC_nn:
  case Z80::JR_e:
  case Z80::JR_Z_e:
  case Z80::JR_NZ_e:
  case Z80::JR_C_e:
  case Z80::JR_NC_e:
  case Z80::DJNZ_e:
    return MI.getOperand(0).getMBB();
  default:
    llvm_unreachable("unexpected opcode in getBranchDestBlock");
  }
}

void Z80InstrInfo::insertIndirectBranch(MachineBasicBlock &MBB,
                                        MachineBasicBlock &NewDestBB,
                                        MachineBasicBlock &RestoreBB,
                                        const DebugLoc &DL, int64_t BrOffset,
                                        RegScavenger *RS) const {
  // On Z80, JP nn can reach any address in the 64KB space.
  BuildMI(&MBB, DL, get(Z80::JP_nn)).addMBB(&NewDestBB);
}

// ravn/llvm-z80#23 / #220 — Z80::shouldHoist OVERRIDE REVERTED 2026-06-08.
//
// The count-based shouldHoist heuristic landed in ravn/llvm-z80@3bd2aa1f0de8
// turned out to introduce a measurable codegen side effect on autoload-in-c
// EVEN when the threshold was set unboundedly high (the heuristic always
// allowed the hoist).  Investigation revealed that the previous matrix
// measurements attributing "+25 B / +18 B / +64 B autoload regression to
// LICM" were actually the heuristic's presence-cost; with the override
// reverted, autoload is BYTE-IDENTICAL to the pre-#23 baseline AND cpnos
// PROM1 is also byte-identical (the "cpnos -15 B win" was also a
// heuristic artifact).
//
// Final fix-all measurements (2026-06-08, no shouldHoist override at all):
//   - AES -Oz: -13 B text, -8.9 % tstates  -- LICM win preserved
//   - AES -O2: -118 B text, -9.2 % tstates  -- LICM win preserved
//   - autoload PROM: 1658 / 1658 / 0 B delta
//   - cpnos PROM1: 2029 / 2029 / 0 B delta
//   - rcbios BIOS: +7 B (CSE residual, unrelated to LICM)
//   - lit: 149 PASS + 4 XFAIL.  Runtime: 854 PASS / 0 FAIL.
//
// Simplest possible correct answer: don't override shouldHoist.  The
// default LICM cost model + TableGen pressure limits already produce
// the right behavior on the four production targets.

// ============================================================================
// ravn/llvm-z80#23 Phase 1 (2026-06-08) -- cost-query hooks.
//
// See tasks/plan-z80-cost-model-refinement-2026-06-08.md Phase 1.
// These hooks are defined but not yet consumed by any pass; Phase 3
// will wire them.  The -z80-use-tiered-cost-model cl::opt gates the
// consumers (default OFF).  Adding the hooks is codegen-neutral.
// ============================================================================

static cl::opt<bool> UseTieredCostModel(
    "z80-use-tiered-cost-model", cl::Hidden, cl::init(true),
    cl::desc("Z80 #23 Phase 4: master flag for the tiered cost-model "
             "refinement.  Default ON 2026-06-08 after Phase 3 chs 1+2 "
             "demonstrated AES -70 B (Oz) and autoload -13 B raw wins "
             "with no regression on cpnos/rcbios/lit/runtime.  Set "
             "false to restore pre-tiered-cost-model behavior for "
             "diagnosis without rebuilding clang."));

bool Z80InstrInfo::useTieredCostModel() const {
  return UseTieredCostModel;
}

unsigned Z80InstrInfo::getRematCost(const MachineInstr &MI) const {
  // Phase 1 default: the instruction's literal byte length.  Most
  // Z80 rematable instructions (`LD r, #imm` 2 B, `LD rr, #imm` 3 B,
  // `XOR A` 1 B, etc.) have constant size, so getInstSizeInBytes is
  // the right starting estimate.  Phase 3 may distinguish further
  // (e.g. a context-aware cost that factors in tstate count).
  return getInstSizeInBytes(MI);
}

unsigned Z80InstrInfo::getSpillCost(const TargetRegisterClass *RC,
                                    Z80SpillKind Kind) const {
  // Phase 1 defaults -- size in bytes per spill+reload PAIR (save +
  // restore added together).
  //
  // BSS / spill-slot:
  //   8-bit:  LD (nn),A + LD A,(nn) = 3 + 3 = 6
  //   16-bit: LD (nn),HL + LD HL,(nn) = 3 + 3 = 6 (HL fast path)
  //           LD (nn),DE/BC + LD DE/BC,(nn) = 4 + 4 = 8 (ED prefix)
  //
  // PUSH/POP (when applicable -- valid only if regalloc can find a
  // bracketing point and the register is in {AF,BC,DE,HL,IX,IY}):
  //   8-bit:  not a single op; spill the containing pair.
  //   16-bit: PUSH rr + POP rr = 1 + 1 = 2.
  //
  // IX/IY-indexed (when a stack slot is exposed via IX or IY frame
  // pointer):
  //   8-bit:  LD (IX+d),r + LD r,(IX+d) = 3 + 3 = 6 (with DD prefix)
  //   16-bit: requires 2x 8-bit accesses = 6 + 6 = 12 (heavy).
  //
  // Use the AnyGR16 default size of 6 for the BSS path; PushPop 2;
  // IXIYIndex 6 for 8-bit, 12 for 16-bit.  RC is unused in this
  // phase; Phase 3 will switch on RC->getID() to refine.
  (void)RC;
  switch (Kind) {
  case Z80SpillKind::BSS:       return 6;
  case Z80SpillKind::PushPop:   return 2;
  case Z80SpillKind::IXIYIndex: return 6;
  }
  return 6; // sane fallback
}

#include "llvm/CodeGen/MachineLoopInfo.h"

// ravn/llvm-z80#23 Phase 3 chapter 1+2 (2026-06-08): MachineLICM hoist
// veto for Z80's tiered register file.  Gated by
// `-z80-use-tiered-cost-model`.  Only vetoes rematerializable
// instructions (non-rematable hoists go through MachineLICM's regular
// pressure-checked path, which the Phase 2 GR16 limit override already
// aligns with regalloc reality).
//
// Two conditions trigger the veto:
//   ch1 (CALL-in-body):     loop body contains a CALL.  Hoisted vreg
//                           must survive the CALL; sdcccall(1)
//                           clobbers HL/DE/BC -> BSS spill, 6 B per
//                           save+reload.
//   ch2 (leaf high-pressure): count of preheader instructions that
//                           are ALSO rematerializable AND whose def
//                           is used in the loop reaches
//                           CheapPairBudget (3 = HL+DE+BC).  Catches
//                           autoload's define_sextants nested leaf
//                           loops with CSE-deduplicated small
//                           constants.
//
// ch2 narrows the ravn/llvm-z80#220 earlier attempt's overcount: that
// attempt counted ANY preheader def with in-loop use (initialization,
// COPYs, etc.), which led to overrefusing + presence-cost.  This
// narrower filter (rematable + used-in-loop) targets the LICM-bypass
// scenario specifically.
bool Z80InstrInfo::shouldHoist(const MachineInstr &MI,
                               const MachineLoop *FromLoop) const {
  if (!useTieredCostModel() || !FromLoop)
    return Z80GenInstrInfo::shouldHoist(MI, FromLoop);
  if (!isReMaterializable(MI))
    return Z80GenInstrInfo::shouldHoist(MI, FromLoop);

  // ch1: CALL in loop body.
  for (const MachineBasicBlock *MBB : FromLoop->blocks())
    for (const MachineInstr &I : *MBB)
      if (I.isCall())
        return false;

  // ch2: count rematable preheader defs with in-loop uses.
  const MachineBasicBlock *Preheader = FromLoop->getLoopPreheader();
  if (!Preheader)
    return Z80GenInstrInfo::shouldHoist(MI, FromLoop);
  const MachineFunction *MF = MI.getMF();
  if (!MF)
    return Z80GenInstrInfo::shouldHoist(MI, FromLoop);
  const MachineRegisterInfo &MRI = MF->getRegInfo();
  unsigned Count = 0;
  for (const MachineInstr &PreI : *Preheader) {
    if (PreI.isDebugInstr() || PreI.isPHI() || PreI.isCopy())
      continue;
    if (!isReMaterializable(PreI))
      continue;
    bool UsedInLoop = false;
    for (const MachineOperand &MO : PreI.defs()) {
      if (!MO.isReg()) continue;
      Register R = MO.getReg();
      if (!R.isVirtual()) continue;
      for (const MachineInstr &Use : MRI.use_nodbg_instructions(R)) {
        if (FromLoop->contains(Use.getParent())) {
          UsedInLoop = true;
          break;
        }
      }
      if (UsedInLoop) break;
    }
    if (UsedInLoop)
      ++Count;
  }
  const unsigned CheapPairBudget = 3;
  if (Count >= CheapPairBudget)
    return false;

  return Z80GenInstrInfo::shouldHoist(MI, FromLoop);
}
