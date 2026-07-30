//===-- Z80FrameLowering.cpp - Z80 Frame Information ----------------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Z80 implementation of TargetFrameLowering class.
//
// The Z80 stack grows downward. The stack pointer (SP) points to the last
// used stack location. PUSH decrements SP first, then stores. POP loads
// first, then increments SP.
//
//===----------------------------------------------------------------------===//

#include "Z80FrameLowering.h"

#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80MachineFunctionInfo.h"
#include "Z80OpcodeUtils.h"
#include "Z80RegisterInfo.h"
#include "Z80Subtarget.h"

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/ErrorHandling.h"

#define DEBUG_TYPE "z80-framelowering"

using namespace llvm;

// Emit a PUSH AF used purely to reserve 2 bytes of stack space.  PUSH AF reads
// $a and $flags, but for stack reservation their values are irrelevant — only
// SP moves, and PUSH does not modify them, so any live A/FLAGS survives.  Mark
// the implicit uses undef so -verify-machineinstrs does not report "Using an
// undefined physical register" when A/FLAGS are dead at the prologue (e.g. a
// function whose argument arrives in HL, not A).  Without this the -verify
// surface stays red on such functions (ravn/llvm-z80 #197 / #209).
static void emitStackReservePushAF(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MBBI,
                                   const DebugLoc &DL,
                                   const TargetInstrInfo &TII) {
  MachineInstr *MI = BuildMI(MBB, MBBI, DL, TII.get(Z80::PUSH_AF));
  for (MachineOperand &MO : MI->operands())
    if (MO.isReg() && MO.isUse())
      MO.setIsUndef(true);
}

Z80FrameLowering::Z80FrameLowering()
    : TargetFrameLowering(StackGrowsDown, /*StackAlignment=*/Align(1),
                          /*LocalAreaOffset=*/-2) {}

bool Z80FrameLowering::hasFPImpl(const MachineFunction &MF) const {
  const auto &STI = MF.getSubtarget<Z80Subtarget>();

  // SM83 has no IX register — always use SP-relative addressing.
  if (STI.hasSM83())
    return false;

  const MachineFrameInfo &MFI = MF.getFrameInfo();

  if (MFI.hasVarSizedObjects() || MFI.isFrameAddressTaken())
    return true;

  // Static stack without stack arguments: IX not needed as frame pointer.
  // The existing alloca/fixedObjects checks below will catch functions
  // that actually need IX (stack args).
  // (Previously guarded with `return true` due to IX constant propagation
  //  bug in Z80LateOptimization — now fixed.)

  if (MF.getTarget().Options.DisableFramePointerElim(MF))
    return true;
  // Use frame pointer when the function has local stack allocations
  // or accesses incoming stack arguments.  Skip it for functions that
  // only need callee-saved register push/pop (like simple ISRs).
  for (const auto &BB : MF.getFunction())
    for (const auto &I : BB)
      if (isa<AllocaInst>(&I))
        return true;
  return MFI.getNumFixedObjects() > 0;
}

bool Z80FrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  // Z80 uses PUSH to pass stack arguments, so the call frame is not
  // preallocated. Return false so ADJCALLSTACKDOWN/UP are preserved
  // for expansion in eliminateCallFramePseudoInstr.
  return false;
}

MachineBasicBlock::iterator Z80FrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc DL = MI->getDebugLoc();
  unsigned Opc = MI->getOpcode();

  if (Opc == Z80::ADJCALLSTACKDOWN) {
    // PUSHes in the call sequence will adjust SP downward.
    // Nothing to emit here.
    return MBB.erase(MI);
  }

  assert(Opc == Z80::ADJCALLSTACKUP && "Expected ADJCALLSTACKUP");

  int64_t Amount = MI->getOperand(0).getImm();
  int64_t CalleeAmount = MI->getOperand(1).getImm();
  Amount -= CalleeAmount;

  // Clean up stack after call: restore SP by adding Amount.
  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  switch (classifyACSU(Amount, STI.hasSM83())) {
  case ACSU_Erase:
    break;
  case ACSU_AddSPe:
    // SM83: ADD SP,e (2 bytes, doesn't clobber HL)
    BuildMI(MBB, MI, DL, TII.get(Z80::ADD_SP_e)).addImm(Amount & 0xFF);
    break;
  case ACSU_PopAF:
    // Small amounts: use POP AF (each pops 2 bytes, clobbers A but not HL).
    // More compact than INC SP (1 byte per 2 bytes vs 1 byte per 1 byte).
    for (unsigned i = 0, n = Amount / 2; i < n; ++i)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_AF));
    if (Amount % 2)
      BuildMI(MBB, MI, DL, TII.get(Z80::INC_SP));
    break;
  case ACSU_AddHLSP:
    // Larger amounts: LD HL, Amount; ADD HL, SP; LD SP, HL
    BuildMI(MBB, MI, DL, TII.get(Z80::LD_HL_nn)).addImm(Amount);
    BuildMI(MBB, MI, DL, TII.get(Z80::ADD_HL_SP));
    BuildMI(MBB, MI, DL, TII.get(Z80::LD_SP_HL));
    break;
  }

  return MBB.erase(MI);
}

void Z80FrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL;

  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  uint64_t StackSize = MFI.getStackSize();

  // __critical functions: emit DI at entry to disable interrupts.
  // For interrupt handlers, the Z80 hardware already disables interrupts.
  bool IsCritical = MF.getFunction().hasFnAttribute("z80_critical");
  if (IsCritical && !MF.getFunction().hasFnAttribute("interrupt")) {
    BuildMI(MBB, MBBI, DL, TII.get(Z80::DI));
  }

  // ISR with +shadow-regs: save AF/BC/DE/HL via EXX + EX AF,AF'.
  // This replaces PUSH AF + PUSH BC + PUSH DE + PUSH HL (4 bytes → 2 bytes).
  // Must be before any register use (including IX frame setup).
  // Skip for empty ISR stubs (only a RET/RETI) — no registers to save.
  const auto &STI0 = MF.getSubtarget<Z80Subtarget>();
  if (MF.getFunction().hasFnAttribute("interrupt") && STI0.shadowRegs() &&
      STI0.hasZ80()) {
    // Check if ISR body is empty: single block with only return instruction(s)
    bool IsEmptyISR = (MF.size() == 1);
    if (IsEmptyISR) {
      for (const auto &MI : MBB) {
        if (!MI.isTerminator() && !MI.isDebugInstr()) {
          IsEmptyISR = false;
          break;
        }
      }
    }
    if (!IsEmptyISR) {
      BuildMI(MBB, MBBI, DL, TII.get(Z80::EXX));
      BuildMI(MBB, MBBI, DL, TII.get(Z80::EX_AF_AF));
    }
  }

  if (hasFP(MF)) {
    // --- Frame pointer mode (IX = frame pointer) ---
    bool NeedsFP = MFI.getNumFixedObjects() > 0 || MFI.isFrameAddressTaken() ||
                   MFI.hasVarSizedObjects();

    if (!StackSize && !NeedsFP)
      return;

    // Static stack: locals in BSS instead of the stack.
    // Only if there are actual locals (StackSize > CalleeSavedFrameSize),
    // not just CSR saves which live on the real stack.
    const auto &STI = MF.getSubtarget<Z80Subtarget>();
    Z80FunctionInfo *FI = MF.getInfo<Z80FunctionInfo>();
    bool UseStaticFrame =
        STI.staticStack() &&
        StackSize > FI->getCalleeSavedFrameSize() &&
        MFI.getNumFixedObjects() == 0 && !MFI.hasVarSizedObjects();
    FI->setUseStaticFrame(UseStaticFrame);

    BuildMI(MBB, MBBI, DL, TII.get(Z80::PUSH_IX));

    if (UseStaticFrame) {
      // IX points to end of the static frame so negative offsets
      // (same as normal IX frame convention) land within the BSS area.
      // Use __sfrend_<name> which is emitted at __sframe_<name>+size.
      MCSymbol *EndSym = MF.getContext().getOrCreateSymbol(
          "__sfrend_" + MF.getName());
      BuildMI(MBB, MBBI, DL, TII.get(Z80::LD_IX_nn)).addSym(EndSym);
      // No SP adjustment — locals live in BSS.
    } else {
      // Standard: IX = SP, then adjust SP for locals.
      BuildMI(MBB, MBBI, DL, TII.get(Z80::LD_IX_nn)).addImm(0);
      BuildMI(MBB, MBBI, DL, TII.get(Z80::ADD_IX_SP));
    }

    if (StackSize > 0 && !UseStaticFrame) {
      unsigned PushCount = StackSize / 2;
      if (PushCount <= 4) {
        for (unsigned i = 0; i < PushCount; ++i)
          emitStackReservePushAF(MBB, MBBI, DL, TII);
        if (StackSize % 2)
          BuildMI(MBB, MBBI, DL, TII.get(Z80::DEC_SP));
      } else {
        // Large frame: PUSH HL; LD HL,-(size-2); ADD HL,SP; LD SP,HL;
        // restore HL from IX-based save location.  The PUSH_HL preserves HL
        // across the SP adjustment; when HL is not a live-in argument its
        // saved value is don't-care, so mark the $hl read undef to satisfy
        // -verify-machineinstrs (ravn/llvm-z80#210/#197).  Mirrors the
        // HL-liveness check on the no-FP large-frame path below.
        bool HLLiveIn = MBB.isLiveIn(Z80::HL) || MBB.isLiveIn(Z80::H) ||
                        MBB.isLiveIn(Z80::L);
        MachineInstr *PushHL = BuildMI(MBB, MBBI, DL, TII.get(Z80::PUSH_HL));
        if (!HLLiveIn)
          for (MachineOperand &MO : PushHL->operands())
            if (MO.isReg() && MO.isUse() && MO.getReg() == Z80::HL)
              MO.setIsUndef(true);
        BuildMI(MBB, MBBI, DL, TII.get(Z80::LD_HL_nn))
            .addImm(-(int64_t)(StackSize - 2) & 0xFFFF);
        BuildMI(MBB, MBBI, DL, TII.get(Z80::ADD_HL_SP));
        BuildMI(MBB, MBBI, DL, TII.get(Z80::LD_SP_HL));
        BuildMI(MBB, MBBI, DL, TII.get(Z80::LD_L_IXd)).addImm(-2);
        BuildMI(MBB, MBBI, DL, TII.get(Z80::LD_H_IXd)).addImm(-1);
      }
    }
  } else {
    // --- No frame pointer (IX is allocatable) ---
    // Callee-saved registers are already pushed by spillCalleeSavedRegisters.
    const auto &STI = MF.getSubtarget<Z80Subtarget>();
    if (STI.staticStack()) {
      // Static stack: locals live in BSS, no stack allocation needed.
      return;
    }
    // We only need to allocate space for locals (StackSize - CSSize).
    const Z80FunctionInfo *FI = MF.getInfo<Z80FunctionInfo>();
    uint64_t LocalSize = StackSize - FI->getCalleeSavedFrameSize();

    if (LocalSize == 0)
      return;

    // Skip past callee-saved PUSHes (inserted before prologue by PEI).
    while (MBBI != MBB.end() && MBBI->getFlag(MachineInstr::FrameSetup) &&
           MBBI->getOpcode() != Z80::ADJCALLSTACKDOWN) {
      ++MBBI;
    }

    if (STI.hasSM83() && LocalSize <= 128) {
      // SM83: ADD SP,e (2 bytes, doesn't clobber HL)
      BuildMI(MBB, MBBI, DL, TII.get(Z80::ADD_SP_e))
          .addImm(-(int64_t)LocalSize & 0xFF);
    } else {
      unsigned PushCount = LocalSize / 2;
      // Check if HL might hold an incoming argument (live-in to entry block).
      // If so, we must not clobber HL with the large-frame LD HL approach.
      bool HLLive =
          MBB.isLiveIn(Z80::HL) || MBB.isLiveIn(Z80::H) || MBB.isLiveIn(Z80::L);
      if (HLLive || PushCount <= 12) {
        // Use PUSH AF (1 byte per 2 bytes, doesn't clobber any registers).
        for (unsigned i = 0; i < PushCount; ++i)
          emitStackReservePushAF(MBB, MBBI, DL, TII);
        if (LocalSize % 2)
          BuildMI(MBB, MBBI, DL, TII.get(Z80::DEC_SP));
      } else {
        // Large frame, HL not live: LD HL, -LocalSize; ADD HL, SP; LD SP, HL
        BuildMI(MBB, MBBI, DL, TII.get(Z80::LD_HL_nn))
            .addImm(-(int64_t)LocalSize & 0xFFFF);
        BuildMI(MBB, MBBI, DL, TII.get(Z80::ADD_HL_SP));
        BuildMI(MBB, MBBI, DL, TII.get(Z80::LD_SP_HL));
      }
    }
  }
}

void Z80FrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc DL;
  if (MBBI != MBB.end())
    DL = MBBI->getDebugLoc();

  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  uint64_t StackSize = MFI.getStackSize();

  if (hasFP(MF)) {
    bool NeedsFP = MFI.getNumFixedObjects() > 0 || MFI.isFrameAddressTaken() ||
                   MFI.hasVarSizedObjects();

    if (StackSize || NeedsFP) {
      const auto &STI = MF.getSubtarget<Z80Subtarget>();
      const Z80FunctionInfo *FI = MF.getInfo<Z80FunctionInfo>();
      bool UseStaticFrame =
          STI.staticStack() &&
          StackSize > FI->getCalleeSavedFrameSize() &&
          MFI.getNumFixedObjects() == 0 && !MFI.hasVarSizedObjects();

      if (UseStaticFrame) {
        // Static stack: IX pointed to BSS, SP never adjusted.
        BuildMI(MBB, MBBI, DL, TII.get(Z80::POP_IX));
      } else {
        // Standard: restore SP from IX, then pop IX.
        BuildMI(MBB, MBBI, DL, TII.get(Z80::LD_SP_IX));
        BuildMI(MBB, MBBI, DL, TII.get(Z80::POP_IX));
      }
    }
  } else {
    // No frame pointer: deallocate locals only.
    const auto &STI2 = MF.getSubtarget<Z80Subtarget>();
    if (STI2.staticStack()) {
      // Static stack: locals in BSS, nothing to deallocate.
      // Callee-save restores (FrameDestroy POPs) happen normally.
    } else {
    // Callee-saved registers are restored by restoreCalleeSavedRegisters,
    // which inserts POPs (with FrameDestroy flag) before the return.
    // We must insert local deallocation BEFORE those callee-save restores,
    // since the stack layout is: [locals | callee-saves | ret-addr].
    const Z80FunctionInfo *FI = MF.getInfo<Z80FunctionInfo>();
    uint64_t LocalSize = StackSize - FI->getCalleeSavedFrameSize();

    if (LocalSize > 0) {
      // Walk MBBI backwards past callee-save restore POPs (FrameDestroy flag)
      // so we insert local deallocation before them.
      while (MBBI != MBB.begin()) {
        auto Prev = std::prev(MBBI);
        if (Prev->getFlag(MachineInstr::FrameDestroy))
          MBBI = Prev;
        else
          break;
      }

      const auto &STI = MF.getSubtarget<Z80Subtarget>();
      if (STI.hasSM83() && LocalSize <= 127) {
        // SM83: ADD SP,e (2 bytes, doesn't clobber HL)
        BuildMI(MBB, MBBI, DL, TII.get(Z80::ADD_SP_e)).addImm(LocalSize & 0xFF);
      } else if (LocalSize <= 4) {
        // Small: INC SP loop (1 byte each, no clobber).
        for (unsigned i = 0; i < LocalSize; ++i)
          BuildMI(MBB, MBBI, DL, TII.get(Z80::INC_SP));
      } else {
        // Check which registers are live at return to pick the best strategy.
        // Return instruction adds implicit uses for return value registers.
        auto RetIt = MBB.getLastNonDebugInstr();
        bool HLLive = false, ALive = false;
        if (RetIt != MBB.end()) {
          for (const auto &MO : RetIt->operands()) {
            if (!MO.isReg() || !MO.isUse())
              continue;
            Register Reg = MO.getReg();
            if (Reg == Z80::HL || Reg == Z80::H || Reg == Z80::L)
              HLLive = true;
            if (Reg == Z80::A || Reg == Z80::AF)
              ALive = true;
          }
        }

        if (!HLLive) {
          // HL free: LD HL,LocalSize; ADD HL,SP; LD SP,HL (5 bytes total).
          BuildMI(MBB, MBBI, DL, TII.get(Z80::LD_HL_nn))
              .addImm(LocalSize & 0xFFFF);
          BuildMI(MBB, MBBI, DL, TII.get(Z80::ADD_HL_SP));
          BuildMI(MBB, MBBI, DL, TII.get(Z80::LD_SP_HL));
        } else if (!ALive) {
          // A free: POP AF loop (1 byte per 2 bytes).
          unsigned PopCount = LocalSize / 2;
          for (unsigned i = 0; i < PopCount; ++i)
            BuildMI(MBB, MBBI, DL, TII.get(Z80::POP_AF));
          if (LocalSize % 2)
            BuildMI(MBB, MBBI, DL, TII.get(Z80::INC_SP));
        } else {
          // Both A and HL live (i32 return): INC SP loop as last resort.
          for (unsigned i = 0; i < LocalSize; ++i)
            BuildMI(MBB, MBBI, DL, TII.get(Z80::INC_SP));
        }
      }
    }
    } // !staticStack
  }

  // Z80 interrupt handlers: emit EI immediately before RETI, after all
  // register restores and frame teardown.  Z80's EI is delayed — it takes
  // effect after the NEXT instruction, which is RETI.  This means no
  // nested interrupt can fire between EI and RETI.
  // (SM83 RETI atomically enables interrupts, so no EI needed.)
  // Emit EI before the return instruction for:
  // - Interrupt handlers (Z80 only, not SM83): re-enable interrupts
  // - __critical functions: restore interrupts that were disabled at entry
  bool NeedsEI = false;
  if (MF.getFunction().hasFnAttribute("interrupt") &&
      !MF.getSubtarget<Z80Subtarget>().hasSM83())
    NeedsEI = true;
  if (MF.getFunction().hasFnAttribute("z80_critical") &&
      !MF.getFunction().hasFnAttribute("interrupt"))
    NeedsEI = true;

  // ISR with +shadow-regs: restore AF/BC/DE/HL via EX AF,AF' + EXX
  // before EI + RETI. Must be after all frame teardown (POP IX etc.)
  // but before EI. Skip for empty ISR stubs (matching prologue logic).
  {
    const auto &STI1 = MF.getSubtarget<Z80Subtarget>();
    if (MF.getFunction().hasFnAttribute("interrupt") && STI1.shadowRegs() &&
        STI1.hasZ80()) {
      bool IsEmptyISR = (MF.size() == 1);
      if (IsEmptyISR) {
        for (const auto &MI : MBB) {
          if (!MI.isTerminator() && !MI.isDebugInstr()) {
            IsEmptyISR = false;
            break;
          }
        }
      }
      if (!IsEmptyISR) {
        MachineBasicBlock::iterator RetI = MBB.getLastNonDebugInstr();
        BuildMI(MBB, RetI, DL, TII.get(Z80::EX_AF_AF));
        BuildMI(MBB, RetI, DL, TII.get(Z80::EXX));
      }
    }
  }

  if (NeedsEI) {
    MachineBasicBlock::iterator RetI = MBB.getLastNonDebugInstr();
    BuildMI(MBB, RetI, DL, TII.get(Z80::EI));
  }
}

void Z80FrameLowering::determineCalleeSaves(MachineFunction &MF,
                                            BitVector &SavedRegs,
                                            RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  // Note: when hasFP(MF), IX is reserved and manually saved/restored in
  // emitPrologue/emitEpilogue — do NOT add it to SavedRegs here, as that
  // would cause PEI to insert a redundant PUSH/POP via
  // spillCalleeSavedRegisters.
}

bool Z80FrameLowering::spillCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    ArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  if (CSI.empty())
    return true;

  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc DL;
  if (MI != MBB.end())
    DL = MI->getDebugLoc();

  // Track callee-saved frame size for prologue/epilogue local-only allocation.
  Z80FunctionInfo *FI = MF.getInfo<Z80FunctionInfo>();
  FI->setCalleeSavedFrameSize(CSI.size() * 2);

  // Spill registers using PUSH
  for (const CalleeSavedInfo &CS : CSI) {
    Register Reg = CS.getReg();
    unsigned Opcode = Z80::getPushOpcode(Reg);

    if (Opcode) {
      BuildMI(MBB, MI, DL, TII.get(Opcode)).setMIFlag(MachineInstr::FrameSetup);
    } else {
      llvm_unreachable("Unexpected CSR without push opcode");
    }
  }

  return true;
}

bool Z80FrameLowering::restoreCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    MutableArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  if (CSI.empty())
    return true;

  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc DL;
  if (MI != MBB.end())
    DL = MI->getDebugLoc();

  // Restore registers using POP in reverse order
  for (auto I = CSI.rbegin(), E = CSI.rend(); I != E; ++I) {
    Register Reg = I->getReg();
    unsigned Opcode = Z80::getPopOpcode(Reg);

    if (Opcode) {
      BuildMI(MBB, MI, DL, TII.get(Opcode))
          .setMIFlag(MachineInstr::FrameDestroy);
    } else {
      llvm_unreachable("Unexpected CSR without pop opcode");
    }
  }

  return true;
}
