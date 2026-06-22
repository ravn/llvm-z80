//===-- Z80RegisterInfo.cpp - Z80 Register Information --------------------===//
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

#include "Z80RegisterInfo.h"

#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80FrameLowering.h"
#include "Z80InstrInfo.h"
#include "Z80MachineFunctionInfo.h"
#include "Z80OpcodeUtils.h"
#include "Z80Subtarget.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/MC/MCContext.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/LiveRegMatrix.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/VirtRegMap.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"

#define DEBUG_TYPE "z80-reginfo"

// ravn/llvm-z80#115 + #27 S1 (session 73): runtime-toggleable instrumentation
// for `getRegAllocationHints`.  Off by default; lit tests for the pre-RA
// pointer-vreg cost-model work flip this on with `-z80-log-regalloc-hints`.
// Uses cl::opt rather than DEBUG_WITH_TYPE so it works in Release builds
// (LLVM_ENABLE_ASSERTIONS=OFF is the default here).
static llvm::cl::opt<bool> Z80LogRegallocHints(
    "z80-log-regalloc-hints", llvm::cl::Hidden, llvm::cl::init(false),
    llvm::cl::desc("Log every getRegAllocationHints query (Z80 #115/#27 S1)"));

// #112: un-reserve IY so it becomes an allocatable 4th 16-bit pair.
// Default OFF.  Three blockers are now resolved -- the encoder opcode-0 crash
// (GR16NoIR/GR16_BCDE discipline), the LEA_IX_FI missing-IY silent no-op (fixed
// below), and the #14 loop-carried-IY miscompile (Z80LateOptimization IX/IY
// transfer peephole dropping the back-edge IY update; fixed with a liveness
// guard) -- but the full oracle (session 73s) still shows IY-on miscompiles:
// (1) the i32-split-through-IY regalloc class (test_167/168: crc reduction
//     loops; allocator shuffles a split 32-bit value through expensive push/pop
//     IY round-trips and corrupts it -- needs cost-model work, NOT a peephole);
// (2) dynamic_alloca (frame-pointer class, test_48 FATAL all opt levels);
// (3) the AES corpus production target (C010=00).  Keep OFF until these close.
// See session73s-issue112-iy-unreserve-scope.md.
// Non-static so Z80NarrowNoIndex can gate on it (declared extern there).
namespace llvm {
cl::opt<bool> Z80UnreserveIY(
    "z80-unreserve-iy", cl::Hidden, cl::init(false),
    cl::desc("Make IY an allocatable 16-bit register (ravn/llvm-z80#112 "
             "bring-up; default off, has known residual regalloc miscompiles)"));

// #38/#112: IY is allocatable as a 4th 16-bit pair when the bring-up flag forces
// it, OR when this function is compiled for SIZE (-Os/-Oz) AND +static-stack is
// active.  Un-reserving IY is a measured size win (BIOS -23 B, autoload -11,
// cpnos -10, AES -145 B) at a small speed cost (~+0.1% tstates: an IY-held value
// is read via push iy; pop hl), so it is gated to size-opt and kept reserved for
// speed (-O2/-O3).  +static-stack is required for correctness (the byte-decompose
// legality fixes #112/#189/#201 are verified only under +static-stack).  Threaded
// through getReservedRegs, getLargestLegalSuperClass, and Z80NarrowNoIndex so all
// the leak-prevention engages together.
bool z80IsIYAllocatable(const MachineFunction &MF);
} // namespace llvm

#define GET_REGINFO_TARGET_DESC
#include "Z80GenRegisterInfo.inc"

using namespace llvm;

// Convenience aliases for shared opcode utilities.
static unsigned getStoreHLindOpcode(Register R) {
  return Z80::getStoreHLindOpcode(R);
}
static unsigned getLoadHLindOpcode(Register R) {
  return Z80::getLoadHLindOpcode(R);
}
static unsigned getCopyToAOpcode(Register R) {
  return Z80::getLD8RegOpcode(Z80::A, R);
}
static unsigned getCopyFromAOpcode(Register R) {
  return Z80::getLD8RegOpcode(R, Z80::A);
}

// Check if an 8-bit register is a sub-register of a 16-bit pair.
static bool isSubRegOf(Register Reg8, Register Reg16) {
  if (Reg16 == Z80::BC)
    return Reg8 == Z80::B || Reg8 == Z80::C;
  if (Reg16 == Z80::DE)
    return Reg8 == Z80::D || Reg8 == Z80::E;
  if (Reg16 == Z80::HL)
    return Reg8 == Z80::H || Reg8 == Z80::L;
  return false;
}

// Forward declaration — defined below in the liveness analysis section.
static bool isRegLiveAt(Register Reg, MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator MI,
                        const TargetRegisterInfo *TRI);

// Choose a temp register from {BC, DE} that doesn't overlap with the given
// 8-bit register. Prefers a register that is not live to avoid save/restore.
static Register chooseTempReg(Register Avoid8, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              const TargetRegisterInfo *TRI) {
  if (isSubRegOf(Avoid8, Z80::BC))
    return Z80::DE;
  if (isSubRegOf(Avoid8, Z80::DE))
    return Z80::BC;
  // Neither conflicts — pick a dead one if possible.
  auto NextIt = std::next(MI);
  if (!isRegLiveAt(Z80::BC, MBB, NextIt, TRI))
    return Z80::BC;
  if (!isRegLiveAt(Z80::DE, MBB, NextIt, TRI))
    return Z80::DE;
  return Z80::BC;
}

static unsigned getPushOpcode(Register R) { return Z80::getPushOpcode(R); }
static unsigned getPopOpcode(Register R) { return Z80::getPopOpcode(R); }

//===----------------------------------------------------------------------===//
// Z80RegisterInfo implementation
//===----------------------------------------------------------------------===//

Z80RegisterInfo::Z80RegisterInfo()
    : Z80GenRegisterInfo(/*RA=*/0, /*DwarfFlavor=*/0, /*EHFlavor=*/0,
                         /*PC=*/0, /*HwMode=*/0) {}

const MCPhysReg *
Z80RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  const auto &STI = MF->getSubtarget<Z80Subtarget>();
  if (MF->getFunction().hasFnAttribute("interrupt")) {
    if (STI.hasSM83())
      return SM83_Interrupt_CSR_SaveList;
    // With +shadow-regs, ISRs use EXX+EX AF,AF' for AF/BC/DE/HL save.
    // Only IX/IY need explicit PUSH/POP.
    if (STI.shadowRegs())
      return Z80_Interrupt_EXX_CSR_SaveList;
    return Z80_Interrupt_CSR_SaveList;
  }
  if (STI.hasSM83())
    return SM83_CSR_SaveList;
  if (MF->getFunction().getCallingConv() == CallingConv::Z80_AllReg)
    return Z80_AllReg_CSR_SaveList;

  // ravn/llvm-z80#131 callee-side: when the function declares
  // "z80-preserves-regs"="...", append the declared registers to the
  // default CSR save list.  PEI / Z80FrameLowering will then emit
  // push/pop for any declared register the body actually defines.
  // The attribute is a programmer assertion *and* a directive — the
  // caller-side code in Z80CallLowering reads it for RegMask narrowing;
  // here we make it real for the callee body too.
  const Function &F = MF->getFunction();
  if (F.hasFnAttribute("z80-preserves-regs")) {
    const Z80FunctionInfo *FI = MF->getInfo<Z80FunctionInfo>();
    if (!FI->ExtendedCSRBuilt) {
      // Start with the default CSR list (e.g. IX for sdcccall(1)).
      for (const MCPhysReg *P = Z80_CSR_SaveList; *P; ++P)
        FI->ExtendedCSRSaveList.push_back(*P);

      // Parse the comma-separated reg names and append.  Same mapping as
      // Z80CallLowering's parseReg helper.
      StringRef AttrVal =
          F.getFnAttribute("z80-preserves-regs").getValueAsString();
      SmallVector<StringRef, 8> Names;
      AttrVal.split(Names, ',', -1, /*KeepEmpty=*/false);
      auto parseReg = [](StringRef N) -> MCPhysReg {
        return StringSwitch<MCPhysReg>(N.trim().lower())
            .Case("a", Z80::A)
            .Case("b", Z80::B)
            .Case("c", Z80::C)
            .Case("d", Z80::D)
            .Case("e", Z80::E)
            .Case("h", Z80::H)
            .Case("l", Z80::L)
            .Case("af", Z80::AF)
            .Case("bc", Z80::BC)
            .Case("de", Z80::DE)
            .Case("hl", Z80::HL)
            .Case("ix", Z80::IX)
            .Case("iy", Z80::IY)
            .Default(MCPhysReg(0));
      };
      // PEI saves at the granularity of the listed regs.  Prefer the
      // 16-bit pair if both halves are listed (single push/pop instead
      // of two): collect pairs first, then add lone halves.
      SmallVector<MCPhysReg, 8> Want;
      bool WantPair[3] = {false, false, false}; // BC, DE, HL
      bool WantHalf[7] = {false}; // A, B, C, D, E, H, L
      auto halfIdx = [](MCPhysReg R) -> int {
        switch (R) {
        case Z80::A: return 0;
        case Z80::B: return 1;
        case Z80::C: return 2;
        case Z80::D: return 3;
        case Z80::E: return 4;
        case Z80::H: return 5;
        case Z80::L: return 6;
        default: return -1;
        }
      };
      for (StringRef N : Names) {
        MCPhysReg R = parseReg(N);
        if (!R) continue;
        if (R == Z80::BC) WantPair[0] = true;
        else if (R == Z80::DE) WantPair[1] = true;
        else if (R == Z80::HL) WantPair[2] = true;
        else if (int i = halfIdx(R); i >= 0) WantHalf[i] = true;
        else Want.push_back(R); // AF, IX, IY — kept as-is
      }
      // Pair completion: if both halves of a pair are listed, prefer
      // the pair (single PUSH BC vs PUSH AF + DEC SP + DEC SP).
      if (WantHalf[1] && WantHalf[2]) { WantPair[0] = true; WantHalf[1] = WantHalf[2] = false; }
      if (WantHalf[3] && WantHalf[4]) { WantPair[1] = true; WantHalf[3] = WantHalf[4] = false; }
      if (WantHalf[5] && WantHalf[6]) { WantPair[2] = true; WantHalf[5] = WantHalf[6] = false; }
      if (WantPair[0]) Want.push_back(Z80::BC);
      if (WantPair[1]) Want.push_back(Z80::DE);
      if (WantPair[2]) Want.push_back(Z80::HL);
      // Lone halves: promote to their pair anyway since Z80 push/pop
      // only operates on pairs.  This over-preserves the sister half
      // but matches the hardware granularity.
      if (WantHalf[0]) Want.push_back(Z80::AF);
      if (WantHalf[1] || WantHalf[2]) Want.push_back(Z80::BC);
      if (WantHalf[3] || WantHalf[4]) Want.push_back(Z80::DE);
      if (WantHalf[5] || WantHalf[6]) Want.push_back(Z80::HL);

      // Deduplicate while preserving order (sister-half promotion may
      // double up with explicit pair).
      llvm::SmallSet<MCPhysReg, 8> Seen;
      for (MCPhysReg R : Want) {
        if (Seen.insert(R).second) {
          // Skip if already in the default CSR (e.g. IX already there).
          bool InDefault = false;
          for (const MCPhysReg *P = Z80_CSR_SaveList; *P; ++P)
            if (*P == R) { InDefault = true; break; }
          if (!InDefault)
            FI->ExtendedCSRSaveList.push_back(R);
        }
      }

      FI->ExtendedCSRSaveList.push_back(0); // null-terminate
      FI->ExtendedCSRBuilt = true;
    }
    return FI->ExtendedCSRSaveList.data();
  }

  return Z80_CSR_SaveList;
}

const uint32_t *
Z80RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                      CallingConv::ID CC) const {
  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  if (STI.hasSM83())
    return SM83_CSR_RegMask;
  if (CC == CallingConv::Z80_AllReg)
    return Z80_AllReg_CSR_RegMask;
  return Z80_CSR_RegMask;
}

bool llvm::z80IsIYAllocatable(const MachineFunction &MF) {
  if (Z80UnreserveIY)
    return true;
  return MF.getFunction().hasOptSize() &&
         MF.getSubtarget<Z80Subtarget>().staticStack();
}

BitVector Z80RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const auto &STI = MF.getSubtarget<Z80Subtarget>();

  // Reserve the stack pointer
  Reserved.set(Z80::SP);

  // Reserve FLAGS: non-allocatable status register for dependency tracking
  Reserved.set(Z80::FLAGS);

  // IX and IY: always reserved on Z80.
  // Session 40 re-investigated #38: un-reserving IY produces ~387
  // test*opt FATAL `Unsupported instruction : <MCInst 0>` in the clang
  // test runner.  Root cause is *not* regalloc — pseudo expansion sites
  // call sub-register-keyed opcode lookups (`getSRLOpcode`, etc.) that
  // return 0 for IXH/IXL/IYH/IYL, then pass that 0 directly to
  // `BuildMI(..., TII.get(0))`, producing bare opcode-0 (PHI) MIs that
  // the encoder rejects.  See ravn/llvm-z80#112 for the audit and fix
  // design (single-register-class GR16NoIR exclusion on the affected
  // tied operands).  IY/IX stay reserved until #112 lands.
  Reserved.set(Z80::IX);
  if (!z80IsIYAllocatable(MF))  // #38/#112: allocatable under size-opt + static-stack
    Reserved.set(Z80::IY);
  if (STI.hasSM83()) {
    Reserved.set(Z80::IX);
    Reserved.set(Z80::IY);
  }

  // IXH/IXL/IYH/IYL are undocumented Z80 half-index registers.
  // Only available when FeatureUndocumented is enabled; absent on SM83.
  if (!STI.hasUndocumented() || STI.hasSM83()) {
    Reserved.set(Z80::IXH);
    Reserved.set(Z80::IXL);
    Reserved.set(Z80::IYH);
    Reserved.set(Z80::IYL);
  }

  // Shadow registers: only allocatable with +shadow-regs on Z80.
  // Reserved on SM83 (no shadow set) and when feature is disabled.
  if (STI.hasSM83() || !STI.shadowRegs()) {
    Reserved.set(Z80::AFp);
    Reserved.set(Z80::BCp);
    Reserved.set(Z80::DEp);
    Reserved.set(Z80::HLp);
    Reserved.set(Z80::Ap);
    Reserved.set(Z80::Fp);
    Reserved.set(Z80::Bp);
    Reserved.set(Z80::Cp);
    Reserved.set(Z80::Dp);
    Reserved.set(Z80::Ep);
    Reserved.set(Z80::Hp);
    Reserved.set(Z80::Lp);
  }

  if (STI.hasSM83()) {
    Reserved.set(Z80::I);
    Reserved.set(Z80::R);
  }

  return Reserved;
}

const TargetRegisterClass *
Z80RegisterInfo::getLargestLegalSuperClass(const TargetRegisterClass *RC,
                                           const MachineFunction &MF) const {
  if (RC->hasSuperClass(&Z80::Anyi8RegClass))
    return &Z80::Anyi8RegClass;
  // GR16NoIR (= DE/HL/BC, GR16 minus IX/IY) deliberately excludes the index
  // registers because its values are byte-decomposed and IX/IY have no
  // documented 8-bit sub-register ops.  When IY is allocatable (the #112
  // -z80-unreserve-iy bring-up path), do NOT re-widen GR16NoIR to GR16 here:
  // getLargestLegalSuperClass is the grow step in recomputeRegClass and in
  // greedy's live-range splitting, and widening a GR16NoIR spill/split temp to
  // GR16 lets the allocator park a byte-decomposed value back in IY (the
  // push/pop shuttle this exclusion exists to prevent -- a density regression
  // under +static-stack and a miscompile in the default config; ravn/llvm-z80
  // #189 / #27).  Spilling still works: SPILL_GR16/RELOAD_GR16 accept GR16 and
  // GR16NoIR is a subclass, so no widening is needed to spill.
  //
  // Gate on z80IsIYAllocatable: when IY is reserved (the production default),
  // GR16 and the IY-excluding subclasses have the same allocatable set
  // {DE,HL,BC}, so widening does not change allocation -- but the CLASS
  // distinction still affects coalescing, and refusing to widen unconditionally
  // regresses production code (extra spill churn from reduced coalescing
  // freedom).  Keeping the original widening when IY is reserved makes
  // production codegen byte-identical; the exclusion only matters when IY can
  // actually be chosen.
  //
  // When IY IS allocatable, do NOT widen ANY IY-excluding 16-bit subclass
  // (GR16NoIR = {DE,HL,BC}, GR16_BCDE = {DE,BC}, the single-pair classes) up
  // to GR16, which would re-introduce IX/IY.  Those values are constrained
  // because they are byte-decomposed (IX/IY have no documented 8-bit
  // sub-register ops) or otherwise IY-incompatible; widening lets greedy's
  // live-range splitting park such a value in IY and emit undocumented
  // IYH/IYL (ravn/llvm-z80 #189; the residual leak hit test_28/39/96 at
  // -Oz/-Os once +static-stack auto-IY made the path the default -- the
  // GR16NoIR-only guard missed GR16_BCDE, the i16 EQ/NE compare operand class).
  if (z80IsIYAllocatable(MF) && RC->hasSuperClass(&Z80::GR16RegClass) &&
      !RC->contains(Z80::IY))
    return RC;
  // Return GR16, not Anyi16.  Anyi16 includes SP which is never allocatable,
  // and the SPILL_GR16/RELOAD_GR16 pseudos only accept GR16.  Widening to
  // Anyi16 causes "Illegal virtual register" errors (#51).
  if (RC->hasSuperClass(&Z80::GR16RegClass))
    return &Z80::GR16RegClass;
  return RC;
}

bool Z80RegisterInfo::saveScavengerRegister(MachineBasicBlock &MBB,
                                            MachineBasicBlock::iterator I,
                                            MachineBasicBlock::iterator &UseMI,
                                            const TargetRegisterClass *RC,
                                            Register Reg) const {
  const TargetInstrInfo &TII = *MBB.getParent()->getSubtarget().getInstrInfo();
  DebugLoc DL;

  // Z80 can only PUSH/POP 16-bit register pairs.
  // For 8-bit registers, save the containing pair.
  unsigned PushOpc, PopOpc;
  if (Reg == Z80::BC || Reg == Z80::B || Reg == Z80::C) {
    PushOpc = Z80::PUSH_BC;
    PopOpc = Z80::POP_BC;
  } else if (Reg == Z80::DE || Reg == Z80::D || Reg == Z80::E) {
    PushOpc = Z80::PUSH_DE;
    PopOpc = Z80::POP_DE;
  } else if (Reg == Z80::HL || Reg == Z80::H || Reg == Z80::L) {
    PushOpc = Z80::PUSH_HL;
    PopOpc = Z80::POP_HL;
  } else if (Reg == Z80::AF || Reg == Z80::A) {
    PushOpc = Z80::PUSH_AF;
    PopOpc = Z80::POP_AF;
  } else {
    return false;
  }

  BuildMI(MBB, I, DL, TII.get(PushOpc));
  BuildMI(MBB, std::next(UseMI), DL, TII.get(PopOpc));
  return true;
}

//===----------------------------------------------------------------------===//
// FLAGS liveness helper
//===----------------------------------------------------------------------===//

// Check if the FLAGS register is live after MI.
// Scans forward: if an instruction reads FLAGS before any instruction writes
// FLAGS, then FLAGS is live. Conditional branches (JP cc, JR cc) are
// terminators with Uses = [FLAGS], so they are caught by readsRegister.
// Also checks successor blocks for FLAGS live-in.
static bool isFlagsLiveAfter(MachineBasicBlock::iterator MI,
                             const TargetRegisterInfo *TRI) {
  MachineBasicBlock &MBB = *MI->getParent();
  for (auto I = std::next(MI->getIterator()); I != MBB.end(); ++I) {
    if (I->readsRegister(Z80::FLAGS, TRI))
      return true;
    if (I->modifiesRegister(Z80::FLAGS, TRI))
      return false;
  }
  // Reached end of block — check if FLAGS is live-in to any successor.
  for (const MachineBasicBlock *Succ : MBB.successors()) {
    if (Succ->isLiveIn(Z80::FLAGS))
      return true;
  }
  return false;
}

//===----------------------------------------------------------------------===//
// Address computation helpers
//===----------------------------------------------------------------------===//

// Emit: PUSH IX; POP HL; LD <TempReg>,offset; ADD HL,<TempReg>
// After this, HL = IX + offset. TempReg must be BC or DE.
// If PreserveFlags is true, wraps ADD HL with PUSH AF/POP AF to preserve
// the FLAGS register. This is safe because no instruction between PUSH AF
// and POP AF modifies A (PUSH IX, POP HL, LD rr,nn don't touch A).
static void emitLargeOffsetAddr(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator InsertBefore,
                                const DebugLoc &DL, const TargetInstrInfo &TII,
                                int64_t Offset, Register TempReg,
                                bool PreserveFlags,
                                const TargetRegisterInfo *TRI) {
  assert((TempReg == Z80::BC || TempReg == Z80::DE) &&
         "Address computation temp must be BC or DE");

  BuildMI(MBB, InsertBefore, DL, TII.get(Z80::PUSH_IX));
  BuildMI(MBB, InsertBefore, DL, TII.get(Z80::POP_HL));

  unsigned LdOpc = (TempReg == Z80::BC) ? Z80::LD_BC_nn : Z80::LD_DE_nn;
  unsigned AddOpc = (TempReg == Z80::BC) ? Z80::ADD_HL_BC : Z80::ADD_HL_DE;

  BuildMI(MBB, InsertBefore, DL, TII.get(LdOpc)).addImm(Offset & 0xFFFF);
  if (PreserveFlags) {
    // A is not modified between this PUSH_AF and the POP_AF (only ADD HL,rr
    // runs); the save carries FLAGS across the ADD.  When A is dead its $a read
    // is don't-care -> mark it undef so the save does not trip
    // -verify-machineinstrs (ravn/llvm-z80#197; same as emitSPRelativeAddr).
    bool ADead = TRI && !isRegLiveAt(Z80::A, MBB, std::next(InsertBefore), TRI);
    MachineInstr *PushAF = BuildMI(MBB, InsertBefore, DL, TII.get(Z80::PUSH_AF));
    if (ADead)
      for (MachineOperand &MO : PushAF->operands())
        if (MO.isReg() && MO.isUse() && MO.getReg() == Z80::A)
          MO.setIsUndef(true);
  }
  BuildMI(MBB, InsertBefore, DL, TII.get(AddOpc));
  if (PreserveFlags)
    BuildMI(MBB, InsertBefore, DL, TII.get(Z80::POP_AF));
}

//===----------------------------------------------------------------------===//
// Register liveness analysis for frame index elimination
//===----------------------------------------------------------------------===//

// Check if Reg (or any overlapping sub/super-register) is live at MI by
// scanning forward through instructions. Replaces RegScavenger which is
// unreliable with forward frame index elimination (RS is not initialized
// per-BB in forward walk mode).
//
// We scan forward from MI until we either find:
// - An instruction that USES the register → it's live
// - An instruction that fully defines Reg (no use) → it's dead
// - The end of the basic block → check if live-out via successor live-ins
//
// Important: a partial def (e.g., defining H when querying HL) does NOT
// kill the entire register — the other half (L) may still be live.
// Only a def that fully covers Reg (Reg == DefReg, or Reg is a sub-register
// of DefReg) counts as a kill.
// Check whether ADJCALLSTACKUP will actually clobber Reg when expanded.
//
// ADJCALLSTACKUP carries implicit-def annotations for HL, A, SP, but the
// actual register side-effects depend on the expansion path chosen in
// Z80FrameLowering::eliminateCallFramePseudoInstr (which runs AFTER frame
// index elimination within PEI). The expansion paths are:
//
//   AdjAmount == 0         → erased entirely (no register effects)
//   SM83 && AdjAmount≤127  → ADD SP,e     (only SP/flags modified)
//   AdjAmount ≤ 16         → POP AF × N   (A/flags modified, HL untouched)
//   AdjAmount > 16         → LD HL,n; ADD HL,SP; LD SP,HL (HL/flags modified)
//
// If we naively trust the implicit-defs, isRegLiveAt() may conclude a
// register (e.g. HL) is dead when the pseudo won't actually modify it,
// causing SPILL/RELOAD expansion to skip saving the register.
static bool adjCallStackUpClobbersReg(const MachineInstr &MI, Register Reg,
                                      const TargetRegisterInfo *TRI) {
  assert(MI.getOpcode() == Z80::ADJCALLSTACKUP);
  int64_t AdjAmount = MI.getOperand(0).getImm() - MI.getOperand(1).getImm();

  if (AdjAmount == 0)
    return false;

  const auto &STI = MI.getMF()->getSubtarget<Z80Subtarget>();
  if (STI.hasSM83() && AdjAmount <= 127)
    // ADD SP,e: only SP and flags modified.
    return false;

  if (AdjAmount <= 16)
    // POP AF: clobbers A and flags; HL is untouched.
    return TRI->regsOverlap(Reg, Z80::A);

  // LD HL,n; ADD HL,SP; LD SP,HL: clobbers HL and flags.
  return TRI->regsOverlap(Reg, Z80::HL);
}

static bool isRegLiveAt(Register Reg, MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator MI,
                        const TargetRegisterInfo *TRI) {
  // Track the *register units* of Reg that are still "pending" — neither used
  // nor redefined yet on the forward scan from MI.  A def retires the units it
  // covers; a use of any still-pending unit means Reg is live.  If every unit
  // is redefined (without an intervening use) before the block end, Reg is
  // dead.  Otherwise the still-pending units fall back to the successor
  // live-ins.
  //
  // Per-unit tracking (rather than a single "full def covers Reg" test) is
  // required for 16-bit registers whose two halves are redefined by *separate*
  // defs, e.g. `$h = COPY ...` then `$l = COPY ...`.  A whole-register full-def
  // check never fires for that shape, so the stale pair was misreported live
  // via the successor live-ins of the *new* halves, emitting a borrow PUSH_HL
  // that reads an undefined $hl (ravn/llvm-z80#210).  Retiring units one at a
  // time lets both halves resolve to dead before the live-out fallback.
  SmallSet<MCRegUnit, 4> Pending;
  for (MCRegUnit U : TRI->regunits(Reg.asMCReg()))
    Pending.insert(U);

  for (auto I = MI, E = MBB.end(); I != E; ++I) {
    // ADJCALLSTACKDOWN is always erased without emitting any code.
    if (I->getOpcode() == Z80::ADJCALLSTACKDOWN)
      continue;

    // ADJCALLSTACKUP's implicit-defs don't always reflect reality —
    // the actual register clobbers depend on the expansion path.
    // Skip this pseudo if its expansion won't touch our register.
    if (I->getOpcode() == Z80::ADJCALLSTACKUP &&
        !adjCallStackUpClobbersReg(*I, Reg, TRI))
      continue;

    // A use of any pending unit means Reg is live (uses are checked before
    // defs so a read-modify-write instruction counts as a use).  An `undef`
    // use is a don't-care read (e.g. a call's `implicit undef $hl`, or a
    // restored-then-dead borrow value) and must NOT keep Reg live -- counting
    // it made the SP-relative spill expander emit an unnecessary borrow
    // PUSH_HL that itself read an undefined $hl (ravn/llvm-z80#197).
    for (const MachineOperand &MO : I->operands()) {
      if (!MO.isReg() || !MO.getReg().isValid() || !MO.isUse() || MO.isUndef())
        continue;
      for (MCRegUnit U : TRI->regunits(MO.getReg().asMCReg()))
        if (Pending.contains(U))
          return true;
    }
    // Defs retire the units they cover (their old value is now dead).
    for (const MachineOperand &MO : I->operands()) {
      if (!MO.isReg() || !MO.getReg().isValid() || !MO.isDef())
        continue;
      for (MCRegUnit U : TRI->regunits(MO.getReg().asMCReg()))
        Pending.erase(U);
    }
    if (Pending.empty())
      return false; // every unit redefined without use — dead
  }
  // No surviving use in the block — the still-pending units fall back to the
  // successor live-ins.  At O1+, values may be live across BB boundaries.
  for (const MachineBasicBlock *Succ : MBB.successors())
    for (const auto &LI : Succ->liveins())
      for (MCRegUnit U : TRI->regunits(LI.PhysReg))
        if (Pending.contains(U))
          return true;
  return false;
}

//===----------------------------------------------------------------------===//
// Large-offset SPILL/RELOAD expansion (IX-based frame pointer)
//===----------------------------------------------------------------------===//

// Expand SPILL_GR8 with large offset.
// Computes address in HL, stores SrcReg via LD (HL),r.
static void expandSpillGR8LargeOffset(MachineBasicBlock &MBB,
                                      MachineBasicBlock::iterator MI,
                                      const DebugLoc &DL,
                                      const TargetInstrInfo &TII,
                                      Register SrcReg, int64_t Offset,
                                      const TargetRegisterInfo *TRI) {
  bool SrcIsHL = isSubRegOf(SrcReg, Z80::HL);
  Register TempReg = chooseTempReg(SrcReg, MBB, MI, TRI);
  bool PreserveFlags = isFlagsLiveAfter(MI, TRI);
  auto NextIt = std::next(MachineBasicBlock::iterator(MI));

  // When spilling H or L, the source half has already been copied to A, so
  // the address borrow only needs to preserve the OTHER half OR the source
  // half if either is live past the spill.  Forcing the save unconditionally
  // made PUSH_HL read an undefined half when the other was dead (#212; same
  // shape #210 already addressed for the SP-relative variant).  If only the
  // source half is live, IMPLICIT_DEF the dead other half so the pair PUSH
  // does not trip -verify-machineinstrs.
  bool NeedSaveHL;
  bool OtherLive = false;
  Register Other;
  if (SrcIsHL) {
    Other = (SrcReg == Z80::H) ? Z80::L : Z80::H;
    OtherLive = isRegLiveAt(Other, MBB, NextIt, TRI);
    bool SrcLive = isRegLiveAt(SrcReg, MBB, NextIt, TRI);
    NeedSaveHL = OtherLive || SrcLive;
  } else {
    NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);
  }
  bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, TRI);
  bool NeedSaveAF = SrcIsHL && isRegLiveAt(Z80::A, MBB, NextIt, TRI);

  // For H/L: copy to A before clobbering HL.
  if (NeedSaveAF)
    BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_AF));
  if (SrcIsHL)
    BuildMI(MBB, MI, DL, TII.get(getCopyToAOpcode(SrcReg)));

  if (NeedSaveHL) {
    if (SrcIsHL && !OtherLive)
      BuildMI(MBB, MI, DL, TII.get(TargetOpcode::IMPLICIT_DEF), Other);
    BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
  }
  if (NeedSaveTemp)
    BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));

  emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, TempReg, PreserveFlags, TRI);

  if (SrcIsHL)
    BuildMI(MBB, MI, DL, TII.get(Z80::LD_HLind_A));
  else
    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(SrcReg)));

  if (NeedSaveTemp)
    BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
  if (NeedSaveHL)
    BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  if (NeedSaveAF)
    BuildMI(MBB, MI, DL, TII.get(Z80::POP_AF));
}

// Expand RELOAD_GR8 with large offset.
// Computes address in HL, loads via LD r,(HL).
static void expandReloadGR8LargeOffset(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MI,
                                       const DebugLoc &DL,
                                       const TargetInstrInfo &TII,
                                       Register DstReg, int64_t Offset,
                                       const TargetRegisterInfo *TRI) {
  bool DstIsHL = isSubRegOf(DstReg, Z80::HL);
  Register TempReg = chooseTempReg(DstReg, MBB, MI, TRI);
  bool PreserveFlags = isFlagsLiveAfter(MI, TRI);
  auto NextIt = std::next(MachineBasicBlock::iterator(MI));

  // When reloading into H or L, save HL only if the OTHER half is live
  // downstream (the half we are reloading into is about to be overwritten by
  // the LD DstReg,A copy that finishes this reload).  If both halves are
  // dead at NextIt (e.g. fastregalloc killed $hl on a prior XOR_CMP_EQ16),
  // saving HL would PUSH undef and trip -verify-machineinstrs
  // (ravn/llvm-z80#212).  When the sibling IS live (the consecutive-RELOAD
  // case the original comment described), isRegLiveAt sees the read and
  // returns true, so the save still fires.  In that case IMPLICIT_DEF the
  // destination half first -- its old value is dead (about to be
  // overwritten) and would otherwise leave the pair PUSH reading half-undef
  // (mirrors expandReloadGR8SPRelative for #210).
  bool NeedSaveHL;
  if (DstIsHL) {
    Register OtherHalf = (DstReg == Z80::H) ? Z80::L : Z80::H;
    NeedSaveHL = isRegLiveAt(OtherHalf, MBB, NextIt, TRI);
  } else {
    NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);
  }
  bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, TRI);
  bool NeedSaveAF = DstIsHL && isRegLiveAt(Z80::A, MBB, NextIt, TRI);

  if (NeedSaveAF)
    BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_AF));
  if (NeedSaveHL) {
    if (DstIsHL)
      BuildMI(MBB, MI, DL, TII.get(TargetOpcode::IMPLICIT_DEF), DstReg);
    BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
  }
  if (NeedSaveTemp)
    BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));

  emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, TempReg, PreserveFlags, TRI);

  if (DstIsHL) {
    // Can't load directly into H/L (HL holds the address).
    // Load into A, restore scratch, then copy A → DstReg.
    BuildMI(MBB, MI, DL, TII.get(Z80::LD_A_HLind));
    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
    BuildMI(MBB, MI, DL, TII.get(getCopyFromAOpcode(DstReg)));
    if (NeedSaveAF)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_AF));
  } else {
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(DstReg)));
    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  }
}

// Expand SPILL_GR16 with large offset.
// Computes address in HL, stores both bytes via LD (HL),lo; INC HL; LD (HL),hi.
static void expandSpillGR16LargeOffset(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MI,
                                       const DebugLoc &DL,
                                       const TargetInstrInfo &TII,
                                       Register SrcReg, int64_t Offset,
                                       const TargetRegisterInfo *TRI) {
  bool PreserveFlags = isFlagsLiveAfter(MI, TRI);
  auto NextIt = std::next(MachineBasicBlock::iterator(MI));

  if (SrcReg == Z80::HL) {
    // HL is both the value to store and the address register.
    // Strategy: push HL (data), compute address, pop data into temp.
    Register TempReg;
    if (!isRegLiveAt(Z80::DE, MBB, NextIt, TRI))
      TempReg = Z80::DE;
    else
      TempReg = Z80::BC;

    bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, TRI);

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));
    BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL)); // push data

    emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, TempReg, PreserveFlags, TRI);

    // Pop original HL value into TempReg.
    BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));

    Register TempLo = (TempReg == Z80::BC) ? Z80::C : Z80::E;
    Register TempHi = (TempReg == Z80::BC) ? Z80::B : Z80::D;
    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(TempLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(TempHi)));

    // Restore HL from TempReg if the spill didn't kill HL.
    if (!MI->getOperand(0).isKill()) {
      unsigned CopyH = (TempReg == Z80::BC) ? Z80::LD_H_B : Z80::LD_H_D;
      unsigned CopyL = (TempReg == Z80::BC) ? Z80::LD_L_C : Z80::LD_L_E;
      BuildMI(MBB, MI, DL, TII.get(CopyL));
      BuildMI(MBB, MI, DL, TII.get(CopyH));
    }

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
  } else if (SrcReg == Z80::BC || SrcReg == Z80::DE) {
    // BC or DE source. Use the other as temp.
    Register TempReg = (SrcReg == Z80::BC) ? Z80::DE : Z80::BC;
    Register SrcLo = (SrcReg == Z80::BC) ? Z80::C : Z80::E;
    Register SrcHi = (SrcReg == Z80::BC) ? Z80::B : Z80::D;

    bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);
    bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, TRI);

    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));

    emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, TempReg, PreserveFlags, TRI);

    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(SrcLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(SrcHi)));

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  } else {
    // IX or IY source (#28).  Z80 has no LD (HL),IX/IY or per-half
    // store, so we must transfer through a GR16 temp pair (BC or DE):
    // first PUSH IX/IY (so the value sits on the stack across the
    // address computation, which itself uses TempReg as scratch),
    // then POP into temp once HL holds the destination address.
    assert((SrcReg == Z80::IX || SrcReg == Z80::IY) &&
           "expandSpillGR16LargeOffset: unexpected SrcReg");
    unsigned PushSrc = (SrcReg == Z80::IX) ? Z80::PUSH_IX : Z80::PUSH_IY;

    Register TempReg;
    if (!isRegLiveAt(Z80::DE, MBB, NextIt, TRI))
      TempReg = Z80::DE;
    else if (!isRegLiveAt(Z80::BC, MBB, NextIt, TRI))
      TempReg = Z80::BC;
    else
      TempReg = Z80::DE;
    Register TempLo = (TempReg == Z80::BC) ? Z80::C : Z80::E;
    Register TempHi = (TempReg == Z80::BC) ? Z80::B : Z80::D;

    bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);
    bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, TRI);

    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));

    // PUSH IX/IY parks the value on the stack across the
    // emitLargeOffsetAddr call (which clobbers TempReg with the
    // offset).  emitLargeOffsetAddr balances its own internal
    // PUSH IX; POP HL pair, so the IX/IY value is still on top of
    // the stack when we POP it into the temp pair.
    BuildMI(MBB, MI, DL, TII.get(PushSrc));
    emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, TempReg, PreserveFlags, TRI);
    BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));

    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(TempLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(TempHi)));

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  }
}

// Expand RELOAD_GR16 with large offset.
// Computes address in HL, loads via LD lo,(HL); INC HL; LD hi,(HL).
static void expandReloadGR16LargeOffset(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        const DebugLoc &DL,
                                        const TargetInstrInfo &TII,
                                        Register DstReg, int64_t Offset,
                                        const TargetRegisterInfo *TRI) {
  bool PreserveFlags = isFlagsLiveAfter(MI, TRI);
  auto NextIt = std::next(MachineBasicBlock::iterator(MI));

  if (DstReg == Z80::HL) {
    // HL is the destination, but also used for address computation.
    // Load into a temp pair, then copy to HL.
    Register TempReg;
    if (!isRegLiveAt(Z80::BC, MBB, NextIt, TRI))
      TempReg = Z80::BC;
    else if (!isRegLiveAt(Z80::DE, MBB, NextIt, TRI))
      TempReg = Z80::DE;
    else
      TempReg = Z80::BC;

    bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, TRI);

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));

    emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, TempReg, PreserveFlags, TRI);

    Register TempLo = (TempReg == Z80::BC) ? Z80::C : Z80::E;
    Register TempHi = (TempReg == Z80::BC) ? Z80::B : Z80::D;
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(TempLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(TempHi)));

    // Copy temp to HL.
    unsigned CopyL = (TempReg == Z80::BC) ? Z80::LD_L_C : Z80::LD_L_E;
    unsigned CopyH = (TempReg == Z80::BC) ? Z80::LD_H_B : Z80::LD_H_D;
    BuildMI(MBB, MI, DL, TII.get(CopyL));
    BuildMI(MBB, MI, DL, TII.get(CopyH));

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
  } else if (DstReg == Z80::BC || DstReg == Z80::DE) {
    // BC or DE destination. Use DstReg itself as temp (old value is dead).
    bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);

    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));

    emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, DstReg, PreserveFlags, TRI);

    Register DstLo = (DstReg == Z80::BC) ? Z80::C : Z80::E;
    Register DstHi = (DstReg == Z80::BC) ? Z80::B : Z80::D;
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(DstLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(DstHi)));

    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  } else {
    // IX or IY destination (#28).  Z80 has no LD IX/IY,(HL) or per-half
    // load, so we must transfer through a GR16 temp pair (BC or DE),
    // then PUSH temp; POP IX/IY.  HL is also clobbered by the address
    // computation.  Save HL and the temp pair if either is live across
    // this RELOAD.
    assert((DstReg == Z80::IX || DstReg == Z80::IY) &&
           "expandReloadGR16LargeOffset: unexpected DstReg");
    unsigned PopOp = (DstReg == Z80::IX) ? Z80::POP_IX : Z80::POP_IY;

    Register TempReg;
    if (!isRegLiveAt(Z80::DE, MBB, NextIt, TRI))
      TempReg = Z80::DE;
    else if (!isRegLiveAt(Z80::BC, MBB, NextIt, TRI))
      TempReg = Z80::BC;
    else
      TempReg = Z80::DE;
    bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, TRI);
    bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);

    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));

    emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, TempReg, PreserveFlags, TRI);

    Register TempLo = (TempReg == Z80::BC) ? Z80::C : Z80::E;
    Register TempHi = (TempReg == Z80::BC) ? Z80::B : Z80::D;
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(TempLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(TempHi)));

    // Transfer temp -> IX/IY via PUSH/POP.
    BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));
    BuildMI(MBB, MI, DL, TII.get(PopOp));

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  }
}

//===----------------------------------------------------------------------===//
// SP-relative stack access (used when frame pointer is omitted)
//===----------------------------------------------------------------------===//

// Emit code to compute HL = SP_at_instruction + Offset.
// SPDelta accounts for PUSHes emitted by the CALLER before this point.
// PreserveFlags adds a SEPARATE PUSH AF/POP AF inside this function to
// protect FLAGS from ADD HL,SP. These are independent SP shifts:
//   - SPDelta: caller's PUSHes (e.g., NeedSaveAF, NeedSaveHL)
//   - PreserveFlags +2: this function's own PUSH AF for FLAGS preservation
//
// SM83 optimization: uses LDHL SP,e (0xF8) when the adjusted offset fits
// in a signed 8-bit range (-128..+127). This replaces the 2-instruction
// sequence (LD HL,nn + ADD HL,SP) with a single instruction.
// Emit a PUSH_AF that preserves only the flags across ADD_HL_SP.  A is not
// modified inside the PUSH_AF/POP_AF bracket, so when A is dead at this point
// its pushed value is don't-care; mark the $a read undef to satisfy
// -verify-machineinstrs (the live $flags read keeps the pair from being
// DCE'd).  When A is live we keep the real read so no pass propagates undef
// into the live value (ravn/llvm-z80#209 family).
static void emitFlagPreservingPushAF(MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator InsertBefore,
                                     const DebugLoc &DL,
                                     const TargetInstrInfo &TII, bool ADead) {
  MachineInstr *MI = BuildMI(MBB, InsertBefore, DL, TII.get(Z80::PUSH_AF));
  if (ADead)
    for (MachineOperand &MO : MI->operands())
      if (MO.isReg() && MO.isUse() && MO.getReg() == Z80::A)
        MO.setIsUndef(true);
}

static void emitSPRelativeAddr(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator InsertBefore,
                               const DebugLoc &DL, const TargetInstrInfo &TII,
                               int64_t Offset, int SPDelta,
                               bool PreserveFlags,
                               const TargetRegisterInfo *TRI) {
  int AdjOffset = Offset + SPDelta;
  if (PreserveFlags)
    AdjOffset += 2; // PUSH AF will shift SP by 2

  // A is not clobbered between the PUSH_AF and POP_AF below; the save exists
  // solely to carry FLAGS across ADD_HL_SP.  Determine whether A is dead here
  // so the don't-care $a read can be marked undef (see
  // emitFlagPreservingPushAF).
  bool ADead =
      PreserveFlags && TRI &&
      !isRegLiveAt(Z80::A, MBB, std::next(InsertBefore), TRI);

  // SM83: use LDHL SP,e if adjusted offset fits in signed 8-bit.
  // This replaces 2-instruction LD HL,nn + ADD HL,SP with a single LDHL SP,e.
  const auto &STI = MBB.getParent()->getSubtarget<Z80Subtarget>();
  if (STI.hasSM83() && AdjOffset >= -128 && AdjOffset <= 127) {
    if (PreserveFlags)
      emitFlagPreservingPushAF(MBB, InsertBefore, DL, TII, ADead);
    BuildMI(MBB, InsertBefore, DL, TII.get(Z80::LDHL_SP_e))
        .addImm(AdjOffset & 0xFF);
    if (PreserveFlags)
      BuildMI(MBB, InsertBefore, DL, TII.get(Z80::POP_AF));
    return;
  }

  // Z80 (and SM83 fallback for large offsets):
  // LD HL,nn; [PUSH AF;] ADD HL,SP; [POP AF;]
  BuildMI(MBB, InsertBefore, DL, TII.get(Z80::LD_HL_nn))
      .addImm(AdjOffset & 0xFFFF);
  if (PreserveFlags)
    emitFlagPreservingPushAF(MBB, InsertBefore, DL, TII, ADead);
  BuildMI(MBB, InsertBefore, DL, TII.get(Z80::ADD_HL_SP));
  if (PreserveFlags)
    BuildMI(MBB, InsertBefore, DL, TII.get(Z80::POP_AF));
}

// Expand SPILL_GR8 with SP-relative addressing.
static void expandSpillGR8SPRelative(MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator MI,
                                     const DebugLoc &DL,
                                     const TargetInstrInfo &TII,
                                     Register SrcReg, int64_t Offset,
                                     const TargetRegisterInfo *TRI) {
  bool SrcIsHL = isSubRegOf(SrcReg, Z80::HL);
  bool PreserveFlags = isFlagsLiveAfter(MI, TRI);
  auto NextIt = std::next(MachineBasicBlock::iterator(MI));
  int SPDelta = 0;

  // If source is H or L, copy to A before clobbering HL for address.
  bool NeedSaveAF = SrcIsHL && isRegLiveAt(Z80::A, MBB, NextIt, TRI);
  if (NeedSaveAF) {
    BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_AF));
    SPDelta += 2;
  }
  if (SrcIsHL)
    BuildMI(MBB, MI, DL, TII.get(getCopyToAOpcode(SrcReg)));

  bool NeedSaveHL;
  if (SrcIsHL) {
    // Spilling one half of HL: the source half has already been copied to A,
    // so the address borrow only needs to preserve the OTHER half — and only
    // if it (or the source half's register value) is live past the spill.
    // Forcing the save unconditionally made PUSH_HL read an undefined half
    // when the other half was dead (ravn/llvm-z80#210).
    Register Other = (SrcReg == Z80::H) ? Z80::L : Z80::H;
    bool OtherLive = isRegLiveAt(Other, MBB, NextIt, TRI);
    bool SrcLive = isRegLiveAt(SrcReg, MBB, NextIt, TRI);
    NeedSaveHL = OtherLive || SrcLive;
    if (NeedSaveHL) {
      // The source half still holds the (defined) spilled value; a dead other
      // half must be made defined so the pair PUSH doesn't read undef.
      if (!OtherLive)
        BuildMI(MBB, MI, DL, TII.get(TargetOpcode::IMPLICIT_DEF), Other);
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      SPDelta += 2;
    }
  } else {
    NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);
    if (NeedSaveHL) {
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      SPDelta += 2;
    }
  }

  emitSPRelativeAddr(MBB, MI, DL, TII, Offset, SPDelta, PreserveFlags, TRI);

  if (SrcIsHL)
    BuildMI(MBB, MI, DL, TII.get(Z80::LD_HLind_A));
  else
    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(SrcReg)));

  if (NeedSaveHL)
    BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  if (NeedSaveAF)
    BuildMI(MBB, MI, DL, TII.get(Z80::POP_AF));
}

// Expand RELOAD_GR8 with SP-relative addressing.
static void expandReloadGR8SPRelative(MachineBasicBlock &MBB,
                                      MachineBasicBlock::iterator MI,
                                      const DebugLoc &DL,
                                      const TargetInstrInfo &TII,
                                      Register DstReg, int64_t Offset,
                                      const TargetRegisterInfo *TRI) {
  bool DstIsHL = isSubRegOf(DstReg, Z80::HL);
  bool PreserveFlags = isFlagsLiveAfter(MI, TRI);
  auto NextIt = std::next(MachineBasicBlock::iterator(MI));
  int SPDelta = 0;

  bool NeedSaveAF = DstIsHL && isRegLiveAt(Z80::A, MBB, NextIt, TRI);
  if (NeedSaveAF) {
    BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_AF));
    SPDelta += 2;
  }

  bool NeedSaveHL;
  if (DstIsHL) {
    // Reloading into one half of HL: the address borrow clobbers the whole
    // pair, so only the OTHER half needs preserving, and only if it is live
    // past the reload.  The destination half's old value is dead (it is about
    // to be overwritten), so IMPLICIT_DEF it to keep the pair PUSH from
    // reading an undefined register (ravn/llvm-z80#210).
    Register Other = (DstReg == Z80::H) ? Z80::L : Z80::H;
    NeedSaveHL = isRegLiveAt(Other, MBB, NextIt, TRI);
    if (NeedSaveHL) {
      BuildMI(MBB, MI, DL, TII.get(TargetOpcode::IMPLICIT_DEF), DstReg);
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      SPDelta += 2;
    }
  } else {
    NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);
    if (NeedSaveHL) {
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      SPDelta += 2;
    }
  }

  emitSPRelativeAddr(MBB, MI, DL, TII, Offset, SPDelta, PreserveFlags, TRI);

  if (DstIsHL) {
    BuildMI(MBB, MI, DL, TII.get(Z80::LD_A_HLind));
    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
    BuildMI(MBB, MI, DL, TII.get(getCopyFromAOpcode(DstReg)));
    if (NeedSaveAF)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_AF));
  } else {
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(DstReg)));
    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  }
}

// Expand SPILL_GR16 with SP-relative addressing.
// Handles BC, DE, HL, and IX (which has no sub-registers).
static void expandSpillGR16SPRelative(MachineBasicBlock &MBB,
                                      MachineBasicBlock::iterator MI,
                                      const DebugLoc &DL,
                                      const TargetInstrInfo &TII,
                                      Register SrcReg, int64_t Offset,
                                      const TargetRegisterInfo *TRI) {
  bool PreserveFlags = isFlagsLiveAfter(MI, TRI);
  auto NextIt = std::next(MachineBasicBlock::iterator(MI));
  int SPDelta = 0;

  if (SrcReg == Z80::IX || SrcReg == Z80::IY) {
    // IX/IY have no accessible sub-registers.
    // Transfer to a temp pair via PUSH/POP, then store bytes.
    Register TempReg;
    if (!isRegLiveAt(Z80::DE, MBB, NextIt, TRI))
      TempReg = Z80::DE;
    else
      TempReg = Z80::BC;
    bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, TRI);
    bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);

    if (NeedSaveHL) {
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      SPDelta += 2;
    }
    if (NeedSaveTemp) {
      BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));
      SPDelta += 2;
    }

    // Transfer IX/IY to TempReg via PUSH/POP
    unsigned PushOpc = (SrcReg == Z80::IX) ? Z80::PUSH_IX : Z80::PUSH_IY;
    BuildMI(MBB, MI, DL, TII.get(PushOpc));
    SPDelta += 2;
    BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
    SPDelta -= 2;

    Register TempLo = (TempReg == Z80::BC) ? Z80::C : Z80::E;
    Register TempHi = (TempReg == Z80::BC) ? Z80::B : Z80::D;

    emitSPRelativeAddr(MBB, MI, DL, TII, Offset, SPDelta, PreserveFlags, TRI);

    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(TempLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(TempHi)));

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  } else if (SrcReg == Z80::HL) {
    // HL is both source and address register.
    Register TempReg;
    if (!isRegLiveAt(Z80::DE, MBB, NextIt, TRI))
      TempReg = Z80::DE;
    else
      TempReg = Z80::BC;
    bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, TRI);

    if (NeedSaveTemp) {
      BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));
      SPDelta += 2;
    }
    // Copy HL data to TempReg via LD r,r'
    Register TempLo = (TempReg == Z80::BC) ? Z80::C : Z80::E;
    Register TempHi = (TempReg == Z80::BC) ? Z80::B : Z80::D;
    unsigned CopyLo = (TempReg == Z80::BC) ? Z80::LD_C_L : Z80::LD_E_L;
    unsigned CopyHi = (TempReg == Z80::BC) ? Z80::LD_B_H : Z80::LD_D_H;
    BuildMI(MBB, MI, DL, TII.get(CopyLo));
    BuildMI(MBB, MI, DL, TII.get(CopyHi));

    emitSPRelativeAddr(MBB, MI, DL, TII, Offset, SPDelta, PreserveFlags, TRI);

    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(TempLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(TempHi)));

    if (!MI->getOperand(0).isKill()) {
      // Restore HL from TempReg
      unsigned RestL = (TempReg == Z80::BC) ? Z80::LD_L_C : Z80::LD_L_E;
      unsigned RestH = (TempReg == Z80::BC) ? Z80::LD_H_B : Z80::LD_H_D;
      BuildMI(MBB, MI, DL, TII.get(RestL));
      BuildMI(MBB, MI, DL, TII.get(RestH));
    }

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
  } else {
    // SrcReg is BC or DE.
    Register SrcLo = TRI->getSubReg(SrcReg, Z80::sub_lo);
    Register SrcHi = TRI->getSubReg(SrcReg, Z80::sub_hi);
    bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);

    if (NeedSaveHL) {
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      SPDelta += 2;
    }

    emitSPRelativeAddr(MBB, MI, DL, TII, Offset, SPDelta, PreserveFlags, TRI);

    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(SrcLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getStoreHLindOpcode(SrcHi)));

    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  }
}

// Expand RELOAD_GR16 with SP-relative addressing.
static void expandReloadGR16SPRelative(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MI,
                                       const DebugLoc &DL,
                                       const TargetInstrInfo &TII,
                                       Register DstReg, int64_t Offset,
                                       const TargetRegisterInfo *TRI) {
  bool PreserveFlags = isFlagsLiveAfter(MI, TRI);
  auto NextIt = std::next(MachineBasicBlock::iterator(MI));
  int SPDelta = 0;

  if (DstReg == Z80::IX || DstReg == Z80::IY) {
    // Load into temp pair, then transfer to IX/IY.
    Register TempReg;
    if (!isRegLiveAt(Z80::BC, MBB, NextIt, TRI))
      TempReg = Z80::BC;
    else if (!isRegLiveAt(Z80::DE, MBB, NextIt, TRI))
      TempReg = Z80::DE;
    else
      TempReg = Z80::BC;
    bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, TRI);
    bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);

    if (NeedSaveHL) {
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      SPDelta += 2;
    }
    if (NeedSaveTemp) {
      BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));
      SPDelta += 2;
    }

    emitSPRelativeAddr(MBB, MI, DL, TII, Offset, SPDelta, PreserveFlags, TRI);

    Register TempLo = (TempReg == Z80::BC) ? Z80::C : Z80::E;
    Register TempHi = (TempReg == Z80::BC) ? Z80::B : Z80::D;
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(TempLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(TempHi)));

    // Transfer TempReg to IX/IY via PUSH/POP
    BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));
    unsigned PopOpc = (DstReg == Z80::IX) ? Z80::POP_IX : Z80::POP_IY;
    BuildMI(MBB, MI, DL, TII.get(PopOpc));

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  } else if (DstReg == Z80::HL) {
    // Load into temp pair, then copy to HL.
    Register TempReg;
    if (!isRegLiveAt(Z80::BC, MBB, NextIt, TRI))
      TempReg = Z80::BC;
    else if (!isRegLiveAt(Z80::DE, MBB, NextIt, TRI))
      TempReg = Z80::DE;
    else
      TempReg = Z80::BC;
    bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, TRI);

    if (NeedSaveTemp) {
      BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));
      SPDelta += 2;
    }

    emitSPRelativeAddr(MBB, MI, DL, TII, Offset, SPDelta, PreserveFlags, TRI);

    Register TempLo = (TempReg == Z80::BC) ? Z80::C : Z80::E;
    Register TempHi = (TempReg == Z80::BC) ? Z80::B : Z80::D;
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(TempLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(TempHi)));

    // Copy temp to HL
    unsigned CopyL = (TempReg == Z80::BC) ? Z80::LD_L_C : Z80::LD_L_E;
    unsigned CopyH = (TempReg == Z80::BC) ? Z80::LD_H_B : Z80::LD_H_D;
    BuildMI(MBB, MI, DL, TII.get(CopyL));
    BuildMI(MBB, MI, DL, TII.get(CopyH));

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
  } else {
    // DstReg is BC or DE. Use it as its own temp for the load.
    bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);

    if (NeedSaveHL) {
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      SPDelta += 2;
    }

    emitSPRelativeAddr(MBB, MI, DL, TII, Offset, SPDelta, PreserveFlags, TRI);

    Register DstLo = TRI->getSubReg(DstReg, Z80::sub_lo);
    Register DstHi = TRI->getSubReg(DstReg, Z80::sub_hi);
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(DstLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(DstHi)));

    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
  }
}

//===----------------------------------------------------------------------===//
// eliminateFrameIndex
//===----------------------------------------------------------------------===//

bool Z80RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  // RS is unused — we use isRegLiveAt() for accurate liveness checks
  // instead of RegScavenger, which is unreliable with forward frame index
  // elimination (eliminateFrameIndicesBackwards=false).
  (void)RS;

  MachineFunction &MF = *MI->getMF();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const auto &STI2 = MF.getSubtarget<Z80Subtarget>();
  const TargetFrameLowering *TFI = getFrameLowering(MF);
  const TargetRegisterInfo *TRI = this;

  int Idx = MI->getOperand(FIOperandNum).getIndex();
  int64_t Offset = MFI.getObjectOffset(Idx);

  bool UseFP = TFI->hasFP(MF);

  // Z80 stack frame layout (with frame pointer):
  //   [parameters]     (IX+4, IX+5, ...)  - passed by caller
  //   [return address] (IX+2, IX+3)       - pushed by CALL
  //   [saved IX]       (IX+0, IX+1)       - pushed in prologue, IX points here
  //   [local var 1]    (IX-2, IX-1)       - allocated in prologue
  //   [local var 2]    (IX-4, IX-3)       <- SP after allocation
  //
  // Without frame pointer (SP-relative):
  //   [parameters]     (SP + StackSize + 2, ...)
  //   [return address] (SP + StackSize)
  //   [callee saves]   (SP + LocalSize, ...)
  //   [local var 1]    (SP + LocalSize - 2)
  //   [local var 2]    <- SP

  if (STI2.staticStack() && !UseFP) {
    // Static stack without frame pointer: BSS displacement only.
    // Two adjustments needed:
    // 1. PEI starts offsets from abs(LocalAreaOffset)=2 (return address
    //    space), but BSS has no return address.
    Offset -= TFI->getOffsetOfLocalArea(); // -(-2) = +2
    // 2. PEI includes CalleeSavedFrameSize in object offsets when CSR
    //    saves exist (they live on the real stack, not in BSS).
    const Z80FunctionInfo *FI = MF.getInfo<Z80FunctionInfo>();
    Offset += FI->getCalleeSavedFrameSize();
  } else if (UseFP) {
    Offset += 2; // Skip saved IX (also needed for static stack: IX = base+size)
  } else {
    // For callee-cleanup calls, if regalloc inserted this frame-index
    // instruction between CALL and ADJCALLSTACKUP, PEI's SPAdj still
    // includes the arg PUSHes even though the callee already popped them.
    // Detect this by scanning forward: if we hit ADJCALLSTACKUP before
    // any CALL, we're in the post-CALL region and must subtract the
    // callee-cleanup amount.
    int AdjustedSPAdj = SPAdj;
    if (SPAdj > 0) {
      auto It = std::next(MI->getIterator());
      auto End = MI->getParent()->end();
      for (; It != End; ++It) {
        if (It->getOpcode() == Z80::ADJCALLSTACKUP) {
          int CalleeAmount = It->getOperand(1).getImm();
          if (CalleeAmount > 0) {
            LLVM_DEBUG(dbgs() << "  CalleeSPAdj: SPAdj " << SPAdj << " -> "
                              << (SPAdj - CalleeAmount) << " (callee cleanup "
                              << CalleeAmount << ") for FI#" << Idx << "\n");
          }
          AdjustedSPAdj -= CalleeAmount;
          break;
        }
        if (It->isCall()) {
          LLVM_DEBUG(dbgs() << "  PreCALL region: SPAdj " << SPAdj
                            << " kept for FI#" << Idx << "\n");
          break; // Pre-CALL region, SPAdj is correct
        }
      }
    }
    Offset += MFI.getStackSize() + AdjustedSPAdj;
    // PEI's StackSize excludes LocalAreaOffset (return address size), but
    // regular objects' offsets include that bias. Compensate for non-fixed
    // objects. Fixed objects (negative Idx, e.g. stack args) use raw
    // SP-entry-relative offsets and don't need this adjustment.
    if (Idx >= 0)
      Offset -= TFI->getOffsetOfLocalArea(); // -(-2) = +2 for Z80
    LLVM_DEBUG(dbgs() << "  FI#" << Idx << " SPAdj=" << SPAdj << " AdjSPAdj="
                      << AdjustedSPAdj << " StackSize=" << MFI.getStackSize()
                      << " ObjOff=" << MFI.getObjectOffset(Idx)
                      << " FinalOffset=" << Offset << " in " << MF.getName()
                      << " bb." << MI->getParent()->getNumber() << "\n");
  }

  // Add any additional offset from the instruction operand
  // (for accessing bytes within a multi-byte stack slot)
  if (FIOperandNum + 1 < MI->getNumOperands() &&
      MI->getOperand(FIOperandNum + 1).isImm()) {
    Offset += MI->getOperand(FIOperandNum + 1).getImm();
    MI->removeOperand(FIOperandNum + 1);
  }

  unsigned Opc = MI->getOpcode();
  MachineBasicBlock &MBB = *MI->getParent();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc DL = MI->getDebugLoc();

  // --- Static stack direct BSS path ---
  // When +static-stack and IX is not the frame pointer, resolve frame
  // indices to BSS addresses (__sfrend_funcname + offset) directly.
  // This allows IX/IY to be used as allocatable data registers.
  if (STI2.staticStack() && !UseFP) {
    MCSymbol *EndSym = MF.getContext().getOrCreateSymbol(
        "__sfrend_" + MF.getName());

    // Helper: create a MachineOperand with EndSym + Offset.
    auto addBSSAddr = [&](MachineInstrBuilder &MIB) {
      auto *NewMI = MIB.addSym(EndSym).getInstr();
      NewMI->getOperand(NewMI->getNumExplicitOperands() - 1).setOffset(Offset);
    };

    // Helper: get LD A,r opcode for a given source register.
    auto getLdAFromReg = [](Register R) -> unsigned {
      switch (R.id()) {
      case Z80::B: return Z80::LD_A_B; case Z80::C: return Z80::LD_A_C;
      case Z80::D: return Z80::LD_A_D; case Z80::E: return Z80::LD_A_E;
      case Z80::H: return Z80::LD_A_H; case Z80::L: return Z80::LD_A_L;
      case Z80::IXH: return Z80::LD_A_IXH; case Z80::IXL: return Z80::LD_A_IXL;
      case Z80::IYH: return Z80::LD_A_IYH; case Z80::IYL: return Z80::LD_A_IYL;
      case Z80::A: return 0; // already in A
      default: return 0;
      }
    };
    // Helper: get LD r,A opcode for a given destination register.
    auto getLdRegFromA = [](Register R) -> unsigned {
      switch (R.id()) {
      case Z80::B: return Z80::LD_B_A; case Z80::C: return Z80::LD_C_A;
      case Z80::D: return Z80::LD_D_A; case Z80::E: return Z80::LD_E_A;
      case Z80::H: return Z80::LD_H_A; case Z80::L: return Z80::LD_L_A;
      case Z80::IXH: return Z80::LD_IXH_A; case Z80::IXL: return Z80::LD_IXL_A;
      case Z80::IYH: return Z80::LD_IYH_A; case Z80::IYL: return Z80::LD_IYL_A;
      case Z80::A: return 0; // already in A
      default: return 0;
      }
    };

    if (Opc == Z80::SPILL_GR8) {
      // Store 8-bit reg to BSS via A: LD A,r; LD (addr),A
      // Must save/restore A if it's live (not the source register).
      Register SrcReg = MI->getOperand(0).getReg();
      unsigned CopyOpc = getLdAFromReg(SrcReg);
      bool NeedSaveA = (CopyOpc != 0) &&
          isRegLiveAt(Z80::A, MBB, std::next(MI->getIterator()), this);
      if (NeedSaveA)
        BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_AF));
      if (CopyOpc)
        BuildMI(MBB, MI, DL, TII.get(CopyOpc));
      auto MIB = BuildMI(MBB, MI, DL, TII.get(Z80::LD_nnind_A));
      addBSSAddr(MIB);
      if (NeedSaveA)
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_AF));
      MI->eraseFromParent();
      return false;
    }
    if (Opc == Z80::RELOAD_GR8) {
      // Load 8-bit reg from BSS via A: LD A,(addr); LD r,A
      // Must save/restore A if it's live (not the destination register).
      Register DstReg = MI->getOperand(0).getReg();
      unsigned CopyOpc = getLdRegFromA(DstReg);
      bool NeedSaveA = (CopyOpc != 0) &&
          isRegLiveAt(Z80::A, MBB, std::next(MI->getIterator()), this);
      if (NeedSaveA)
        BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_AF));
      auto MIB = BuildMI(MBB, MI, DL, TII.get(Z80::LD_A_nnind));
      addBSSAddr(MIB);
      if (CopyOpc)
        BuildMI(MBB, MI, DL, TII.get(CopyOpc));
      if (NeedSaveA)
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_AF));
      MI->eraseFromParent();
      return false;
    }
    if (Opc == Z80::SPILL_GR16) {
      // 16-bit store to BSS: use direct addressing instructions.
      // LD (addr),HL = 3B, LD (addr),DE = 4B (ED 53), LD (addr),BC = 4B (ED 43)
      // LD (addr),IX = 4B (DD 22), LD (addr),IY = 4B (FD 22)
      Register SrcReg = MI->getOperand(0).getReg();
      unsigned StoreOpc = 0;
      if (SrcReg == Z80::HL) StoreOpc = Z80::LD_nnind_HL;
      else if (SrcReg == Z80::DE) StoreOpc = Z80::LD_nnind_DE;
      else if (SrcReg == Z80::BC) StoreOpc = Z80::LD_nnind_BC;
      else if (SrcReg == Z80::IX) StoreOpc = Z80::LD_nnind_IX;
      else if (SrcReg == Z80::IY) StoreOpc = Z80::LD_nnind_IY;
      if (StoreOpc) {
        auto MIB = BuildMI(MBB, MI, DL, TII.get(StoreOpc));
        addBSSAddr(MIB);
        MI->eraseFromParent();
        return false;
      }
      llvm_unreachable("Unexpected register for SPILL_GR16 in static-stack BSS mode");
    }
    if (Opc == Z80::RELOAD_GR16) {
      // 16-bit load from BSS: use direct addressing instructions.
      // LD HL,(addr) = 3B, LD DE,(addr) = 4B (ED 5B), LD BC,(addr) = 4B (ED 4B)
      // LD IX,(addr) = 4B (DD 2A), LD IY,(addr) = 4B (FD 2A)
      Register DstReg = MI->getOperand(0).getReg();
      unsigned LoadOpc = 0;
      if (DstReg == Z80::HL) LoadOpc = Z80::LD_HL_nnind;
      else if (DstReg == Z80::DE) LoadOpc = Z80::LD_DE_nnind;
      else if (DstReg == Z80::BC) LoadOpc = Z80::LD_BC_nnind;
      else if (DstReg == Z80::IX) LoadOpc = Z80::LD_IX_nnind;
      else if (DstReg == Z80::IY) LoadOpc = Z80::LD_IY_nnind;
      if (LoadOpc) {
        auto MIB = BuildMI(MBB, MI, DL, TII.get(LoadOpc));
        addBSSAddr(MIB);
        MI->eraseFromParent();
        return false;
      }
      llvm_unreachable("Unexpected register for RELOAD_GR16 in static-stack BSS mode");
    }
    if (Opc == Z80::SPILL_IMM8) {
      // Store immediate to BSS: LD A,imm; LD (addr),A = 5B
      // Must save/restore A if it's live — the SPILL_IMM8 pseudo has no
      // implicit-def of A (correct for IX-indexed LD (IX+d),n expansion),
      // so the register allocator may have placed a live value in A.
      int64_t Val = MI->getOperand(0).getImm();
      bool NeedSaveA =
          isRegLiveAt(Z80::A, MBB, std::next(MI->getIterator()), this);
      if (NeedSaveA)
        BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_AF));
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_A_n)).addImm(Val);
      auto MIB = BuildMI(MBB, MI, DL, TII.get(Z80::LD_nnind_A));
      addBSSAddr(MIB);
      if (NeedSaveA)
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_AF));
      MI->eraseFromParent();
      return false;
    }
    if (Opc == Z80::LEA_IX_FI) {
      // Compute BSS address into a register: LD rr, addr
      Register DstReg = MI->getOperand(0).getReg();
      unsigned LdOpc = 0;
      if (DstReg == Z80::HL) LdOpc = Z80::LD_HL_nn;
      else if (DstReg == Z80::DE) LdOpc = Z80::LD_DE_nn;
      else if (DstReg == Z80::BC) LdOpc = Z80::LD_BC_nn;
      else if (DstReg == Z80::IX) LdOpc = Z80::LD_IX_nn;
      if (LdOpc) {
        auto MIB = BuildMI(MBB, MI, DL, TII.get(LdOpc));
        addBSSAddr(MIB);
        MI->eraseFromParent();
        return false;
      }
    }
    if (Opc == Z80::ADD_HL_FI || Opc == Z80::SUB_HL_FI) {
      // Load 16-bit BSS value into temp, then ADD/SUB HL,temp.
      // LD BC/DE,(addr); ADD HL,BC/DE  (or AND A; SBC HL,BC/DE for sub)
      auto NextIt = std::next(MI);
      Register TempReg = !isRegLiveAt(Z80::BC, MBB, NextIt, this) ? Z80::BC
                       : !isRegLiveAt(Z80::DE, MBB, NextIt, this) ? Z80::DE
                                                                   : Z80::BC;
      bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, this);
      if (NeedSaveTemp)
        BuildMI(MBB, MI, DL, TII.get(Z80::getPushOpcode(TempReg)));
      unsigned LoadOpc = (TempReg == Z80::BC) ? Z80::LD_BC_nnind : Z80::LD_DE_nnind;
      auto MIB = BuildMI(MBB, MI, DL, TII.get(LoadOpc));
      addBSSAddr(MIB);
      if (Opc == Z80::ADD_HL_FI) {
        BuildMI(MBB, MI, DL,
                TII.get(TempReg == Z80::BC ? Z80::ADD_HL_BC : Z80::ADD_HL_DE));
      } else {
        BuildMI(MBB, MI, DL, TII.get(Z80::AND_A));
        BuildMI(MBB, MI, DL,
                TII.get(TempReg == Z80::BC ? Z80::SBC_HL_BC : Z80::SBC_HL_DE));
      }
      if (NeedSaveTemp)
        BuildMI(MBB, MI, DL, TII.get(Z80::getPopOpcode(TempReg)));
      MI->eraseFromParent();
      return false;
    }
    // Unknown opcode with frame index in static stack mode.
    // Fall through to normal handling.
  }

  // LEA_IX_FI: compute the actual address of a stack object into a register.
  if (Opc == Z80::LEA_IX_FI) {
    Register DstReg = MI->getOperand(0).getReg();

    if (!UseFP) {
      // SP-relative: LD HL, Offset; ADD HL, SP; then copy to DstReg.
      bool PreserveFlags = isFlagsLiveAfter(MI, this);
      int SPDelta = 0;

      if (DstReg == Z80::HL) {
        emitSPRelativeAddr(MBB, MI, DL, TII, Offset, 0, PreserveFlags, TRI);
      } else {
        bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, std::next(MI), this);
        if (NeedSaveHL) {
          BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
          SPDelta += 2;
        }
        emitSPRelativeAddr(MBB, MI, DL, TII, Offset, SPDelta, PreserveFlags, TRI);
        if (DstReg == Z80::DE) {
          const auto &STI = MF.getSubtarget<Z80Subtarget>();
          if (STI.hasSM83()) {
            BuildMI(MBB, MI, DL, TII.get(Z80::LD_D_H));
            BuildMI(MBB, MI, DL, TII.get(Z80::LD_E_L));
          } else {
            BuildMI(MBB, MI, DL, TII.get(Z80::EX_DE_HL));
          }
        } else if (DstReg == Z80::BC) {
          BuildMI(MBB, MI, DL, TII.get(Z80::LD_B_H));
          BuildMI(MBB, MI, DL, TII.get(Z80::LD_C_L));
        } else if (DstReg == Z80::IX) {
          BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
          BuildMI(MBB, MI, DL, TII.get(Z80::POP_IX));
        } else if (DstReg == Z80::IY) {
          // #112: IY destination (allocatable once un-reserved).  Without
          // this case LEA_IX_FI hit llvm_unreachable, which in a Release
          // build is a no-op -- the instruction was erased emitting NOTHING,
          // leaving IY undefined and a downstream spill reading garbage
          // (session 73s root-cause of the IY-allocatable miscompile).
          BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
          BuildMI(MBB, MI, DL, TII.get(Z80::POP_IY));
        } else {
          llvm_unreachable("Unexpected register for LEA_IX_FI");
        }
        if (NeedSaveHL)
          BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
      }
      MI->eraseFromParent();
      return false;
    }

    // IX-based (hasFP): existing logic
    if (Offset == 0) {
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_IX));
      if (DstReg == Z80::HL)
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
      else if (DstReg == Z80::DE)
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_DE));
      else if (DstReg == Z80::BC)
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_BC));
      else if (DstReg == Z80::IX)
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_IX));
      else if (DstReg == Z80::IY)  // #112
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_IY));
      else
        llvm_unreachable("Unexpected register for LEA_IX_FI");
    } else if (DstReg == Z80::HL) {
      bool PreserveFlags = isFlagsLiveAfter(MI, this);
      auto NextIt = std::next(MI);
      Register TempReg;
      if (!isRegLiveAt(Z80::BC, MBB, NextIt, this))
        TempReg = Z80::BC;
      else if (!isRegLiveAt(Z80::DE, MBB, NextIt, this))
        TempReg = Z80::DE;
      else
        TempReg = Z80::BC;
      bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, this);

      if (NeedSaveTemp)
        BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));
      emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, TempReg, PreserveFlags, TRI);
      if (NeedSaveTemp)
        BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
    } else if (DstReg == Z80::DE) {
      bool PreserveFlags = isFlagsLiveAfter(MI, this);
      bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, std::next(MI), this);

      if (NeedSaveHL)
        BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, Z80::DE, PreserveFlags, TRI);
      BuildMI(MBB, MI, DL, TII.get(Z80::EX_DE_HL));
      if (NeedSaveHL)
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
    } else if (DstReg == Z80::BC) {
      bool PreserveFlags = isFlagsLiveAfter(MI, this);
      bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, std::next(MI), this);

      if (NeedSaveHL)
        BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, Z80::BC, PreserveFlags, TRI);
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_B_H));
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_C_L));
      if (NeedSaveHL)
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
    } else if (DstReg == Z80::IX || DstReg == Z80::IY) {
      // #112: compute the address into HL (via a BC/DE scratch), then move it
      // into the index register via PUSH/POP.  Without this, an IX/IY dest
      // hit llvm_unreachable -> silent no-op in Release -> undefined index reg.
      bool PreserveFlags = isFlagsLiveAfter(MI, this);
      auto NextIt = std::next(MI);
      bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, this);
      Register TempReg = !isRegLiveAt(Z80::BC, MBB, NextIt, this)   ? Z80::BC
                         : !isRegLiveAt(Z80::DE, MBB, NextIt, this) ? Z80::DE
                                                                    : Z80::BC;
      bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, this);
      if (NeedSaveHL)
        BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      if (NeedSaveTemp)
        BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));
      emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, TempReg, PreserveFlags, TRI);
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
      BuildMI(MBB, MI, DL,
              TII.get(DstReg == Z80::IX ? Z80::POP_IX : Z80::POP_IY));
      if (NeedSaveTemp)
        BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
      if (NeedSaveHL)
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
    } else {
      llvm_unreachable("Unexpected register for LEA_IX_FI");
    }
    MI->eraseFromParent();
    return false;
  }

  // --- SP-relative mode (no frame pointer) ---
  // All SPILL/RELOAD must be expanded inline; IX+d is not available.
  if (!UseFP) {
    if (Opc == Z80::SPILL_IMM8) {
      int64_t Val = MI->getOperand(0).getImm();
      bool PreserveFlags = isFlagsLiveAfter(MI, this);
      bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, std::next(MI), this);
      int SPDelta = 0;

      if (NeedSaveHL) {
        BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
        SPDelta += 2;
      }
      emitSPRelativeAddr(MBB, MI, DL, TII, Offset, SPDelta, PreserveFlags, TRI);
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_HLind_n)).addImm(Val & 0xFF);
      if (NeedSaveHL)
        BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
      MI->eraseFromParent();
      return false;
    }

    if (Opc == Z80::SPILL_GR8) {
      Register SrcReg = MI->getOperand(0).getReg();
      expandSpillGR8SPRelative(MBB, MI, DL, TII, SrcReg, Offset, this);
      MI->eraseFromParent();
      return false;
    }

    if (Opc == Z80::RELOAD_GR8) {
      Register DstReg = MI->getOperand(0).getReg();
      expandReloadGR8SPRelative(MBB, MI, DL, TII, DstReg, Offset, this);
      MI->eraseFromParent();
      return false;
    }

    if (Opc == Z80::SPILL_GR16) {
      Register SrcReg = MI->getOperand(0).getReg();
      expandSpillGR16SPRelative(MBB, MI, DL, TII, SrcReg, Offset, this);
      MI->eraseFromParent();
      return false;
    }

    if (Opc == Z80::RELOAD_GR16) {
      Register DstReg = MI->getOperand(0).getReg();
      expandReloadGR16SPRelative(MBB, MI, DL, TII, DstReg, Offset, this);
      MI->eraseFromParent();
      return false;
    }

    // SP-relative mode: unfold ADD_HL_FI/SUB_HL_FI back to RELOAD + op.
    //
    // Without a frame pointer, IX+d addressing is unavailable, so we cannot
    // expand to the 8-bit IX-indexed ALU sequence.  Instead we unfold back
    // to the equivalent of RELOAD_GR16 + ADD_HL_rr / SUB_HL_rr:
    //
    //   [PUSH TempReg]          ; save if TempReg is live
    //   PUSH HL                 ; save running sum (HL is clobbered by reload)
    //   <reload SP-relative>    ; load stack variable into TempReg
    //   POP HL                  ; restore running sum
    //   ADD HL, TempReg         ; (or AND A / SBC HL, TempReg for SUB)
    //   [POP TempReg]           ; restore if was saved
    //
    // The PUSH/POP of HL shifts SP, so Offset is adjusted by SPAdj to
    // compensate for the extra stack entries between SP and the target slot.
    if (Opc == Z80::ADD_HL_FI || Opc == Z80::SUB_HL_FI) {
      auto NextIt = std::next(MI);
      Register TempReg = !isRegLiveAt(Z80::BC, MBB, NextIt, this)   ? Z80::BC
                         : !isRegLiveAt(Z80::DE, MBB, NextIt, this) ? Z80::DE
                                                                    : Z80::BC;
      bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, this);

      if (NeedSaveTemp)
        BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));

      int SPAdj = 2 + (NeedSaveTemp ? 2 : 0);
      expandReloadGR16SPRelative(MBB, MI, DL, TII, TempReg, Offset + SPAdj,
                                 this);

      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));

      if (Opc == Z80::ADD_HL_FI) {
        BuildMI(MBB, MI, DL,
                TII.get(TempReg == Z80::BC ? Z80::ADD_HL_BC : Z80::ADD_HL_DE));
      } else {
        BuildMI(MBB, MI, DL, TII.get(Z80::AND_A));
        BuildMI(MBB, MI, DL,
                TII.get(TempReg == Z80::BC ? Z80::SBC_HL_BC : Z80::SBC_HL_DE));
      }

      if (NeedSaveTemp)
        BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));

      MI->eraseFromParent();
      return false;
    }

    llvm_unreachable("Unexpected frame index instruction in SP-relative mode");
  }

  // --- Frame pointer mode (IX+d) ---

  // Small offset: fits in IX+d signed 8-bit displacement (-128 to +127).
  // For 16-bit SPILL/RELOAD, both Offset and Offset+1 must fit.
  bool Is16BitFI = (Opc == Z80::SPILL_GR16 || Opc == Z80::RELOAD_GR16 ||
                    Opc == Z80::ADD_HL_FI || Opc == Z80::SUB_HL_FI);
  int64_t MaxOffset = Is16BitFI ? Offset + 1 : Offset;

  if (Offset >= -128 && MaxOffset <= 127) {
    // Expand ADD_HL_FI/SUB_HL_FI to 8-bit IX-indexed ALU sequence (10 bytes).
    // This is the fast path — the offset fits in IX+d, so we can directly
    // use ADD A,(IX+d) / ADC A,(IX+d+1) to perform 16-bit addition byte
    // by byte without allocating a GR16_BCDE register pair.
    //
    //   LD A, L              ; 1B  low byte of running sum
    //   ADD A, (IX+d)        ; 3B  add low byte of stack variable
    //   LD L, A              ; 1B  store back
    //   LD A, H              ; 1B  high byte of running sum
    //   ADC A, (IX+d+1)      ; 3B  add high byte with carry
    //   LD H, A              ; 1B  store back → HL = sum + variable
    if (Opc == Z80::ADD_HL_FI) {
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_A_L));
      BuildMI(MBB, MI, DL, TII.get(Z80::ADD_A_IXd)).addImm(Offset);
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_L_A));
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_A_H));
      BuildMI(MBB, MI, DL, TII.get(Z80::ADC_A_IXd)).addImm(Offset + 1);
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_H_A));
      MI->eraseFromParent();
      return false;
    }
    if (Opc == Z80::SUB_HL_FI) {
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_A_L));
      BuildMI(MBB, MI, DL, TII.get(Z80::SUB_IXd)).addImm(Offset);
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_L_A));
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_A_H));
      BuildMI(MBB, MI, DL, TII.get(Z80::SBC_A_IXd)).addImm(Offset + 1);
      BuildMI(MBB, MI, DL, TII.get(Z80::LD_H_A));
      MI->eraseFromParent();
      return false;
    }

    MI->getOperand(FIOperandNum).ChangeToImmediate(Offset);

    // The frame-index displacement operand ($offset, at FIOperandNum+1) was
    // folded into the resolved offset and removed above (see the
    // "FIOperandNum + 1 ... removeOperand" block).  The SPILL/RELOAD FI
    // pseudos that survive to expandPostRAPseudo are declared *with* that
    // $offset operand (Z80InstrInfo.td), so dropping it leaves a 2-operand
    // instruction against a 3-operand MCInstrDesc and -verify-machineinstrs
    // reports "Too few operands" after PEI (#200).  Restore it as a 0
    // placeholder — its value is already folded into operand 1, and
    // expandPostRAPseudo reads only operand 1 — so this is codegen-neutral.
    switch (Opc) {
    case Z80::SPILL_GR16:
    case Z80::RELOAD_GR16:
    case Z80::SPILL_GR8:
    case Z80::RELOAD_GR8:
    case Z80::SPILL_IMM8:
      MI->addOperand(MachineOperand::CreateImm(0));
      break;
    default:
      break;
    }
    return false;
  }

  // Large offset: expand SPILL/RELOAD with IX-based address computation.
  if (Opc == Z80::SPILL_IMM8) {
    int64_t Val = MI->getOperand(0).getImm();
    auto NextIt = std::next(MI);
    Register TempReg =
        !isRegLiveAt(Z80::BC, MBB, NextIt, this) ? Z80::BC : Z80::DE;
    bool PreserveFlags = isFlagsLiveAfter(MI, this);
    bool NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, this);
    bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, this);

    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));
    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));
    emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, TempReg, PreserveFlags, TRI);
    BuildMI(MBB, MI, DL, TII.get(Z80::LD_HLind_n)).addImm(Val & 0xFF);
    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));
    if (NeedSaveHL)
      BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));
    MI->eraseFromParent();
    return false;
  }

  if (Opc == Z80::SPILL_GR8) {
    Register SrcReg = MI->getOperand(0).getReg();
    expandSpillGR8LargeOffset(MBB, MI, DL, TII, SrcReg, Offset, this);
    MI->eraseFromParent();
    return false;
  }

  if (Opc == Z80::RELOAD_GR8) {
    Register DstReg = MI->getOperand(0).getReg();
    expandReloadGR8LargeOffset(MBB, MI, DL, TII, DstReg, Offset, this);
    MI->eraseFromParent();
    return false;
  }

  if (Opc == Z80::SPILL_GR16) {
    Register SrcReg = MI->getOperand(0).getReg();
    expandSpillGR16LargeOffset(MBB, MI, DL, TII, SrcReg, Offset, this);
    MI->eraseFromParent();
    return false;
  }

  if (Opc == Z80::RELOAD_GR16) {
    Register DstReg = MI->getOperand(0).getReg();
    expandReloadGR16LargeOffset(MBB, MI, DL, TII, DstReg, Offset, this);
    MI->eraseFromParent();
    return false;
  }

  // Large offset fallback for ADD_HL_FI/SUB_HL_FI.
  //
  // When Offset+1 > 127 or Offset < -128, the displacement doesn't fit in
  // IX+d (signed 8-bit), so we cannot use the 10-byte IX-indexed ALU
  // expansion.  Instead we unfold back to the equivalent of a full 16-bit
  // RELOAD + ADD HL,rr, which requires a register pair (TempReg = BC or DE).
  //
  // This is strictly worse than not folding at all (~16B vs ~14B for the
  // unfold), because fold's purpose — avoiding a GR16_BCDE allocation — is
  // defeated by the unfold.  However, the final offset is unknown at fold
  // time (ISel/pre-StackColoring), so we cannot prevent folding for large
  // frames.  This path exists purely as a correctness fallback.
  //
  // In practice this is dead code: Z80 stack frames rarely exceed 127 bytes.
  //
  // Sequence (ADD_HL_FI, TempReg = BC):
  //   [PUSH BC]             ; save if BC is live
  //   PUSH HL               ; save running sum
  //   PUSH IX / POP HL      ;   HL = IX (frame pointer)
  //   LD BC, #Offset        ;   BC = large offset
  //   ADD HL, BC            ;   HL = IX + Offset = &variable
  //   LD C, (HL)            ;   load low byte
  //   INC HL
  //   LD B, (HL)            ;   load high byte → BC = variable value
  //   POP HL                ; restore running sum
  //   ADD HL, BC            ; HL += variable
  //   [POP BC]              ; restore if was saved
  if (Opc == Z80::ADD_HL_FI || Opc == Z80::SUB_HL_FI) {
    bool PreserveFlags = isFlagsLiveAfter(MI, this);
    auto NextIt = std::next(MI);
    Register TempReg = !isRegLiveAt(Z80::BC, MBB, NextIt, this)   ? Z80::BC
                       : !isRegLiveAt(Z80::DE, MBB, NextIt, this) ? Z80::DE
                                                                  : Z80::BC;
    bool NeedSaveTemp = isRegLiveAt(TempReg, MBB, NextIt, this);

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPushOpcode(TempReg)));

    // Save HL (the value to add/sub to).
    BuildMI(MBB, MI, DL, TII.get(Z80::PUSH_HL));

    // Compute address: HL = IX + Offset (clobbers HL)
    emitLargeOffsetAddr(MBB, MI, DL, TII, Offset, TempReg, PreserveFlags, TRI);

    // Load from (HL) into TempReg
    Register TempLo = (TempReg == Z80::BC) ? Z80::C : Z80::E;
    Register TempHi = (TempReg == Z80::BC) ? Z80::B : Z80::D;
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(TempLo)));
    BuildMI(MBB, MI, DL, TII.get(Z80::INC_HL));
    BuildMI(MBB, MI, DL, TII.get(getLoadHLindOpcode(TempHi)));

    // Restore HL.
    BuildMI(MBB, MI, DL, TII.get(Z80::POP_HL));

    // Perform the 16-bit operation.
    if (Opc == Z80::ADD_HL_FI) {
      BuildMI(MBB, MI, DL,
              TII.get(TempReg == Z80::BC ? Z80::ADD_HL_BC : Z80::ADD_HL_DE));
    } else {
      BuildMI(MBB, MI, DL, TII.get(Z80::AND_A));
      BuildMI(MBB, MI, DL,
              TII.get(TempReg == Z80::BC ? Z80::SBC_HL_BC : Z80::SBC_HL_DE));
    }

    if (NeedSaveTemp)
      BuildMI(MBB, MI, DL, TII.get(getPopOpcode(TempReg)));

    MI->eraseFromParent();
    return false;
  }

  llvm_unreachable("Large frame offset on non-SPILL/RELOAD instruction");
}

Register Z80RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = getFrameLowering(MF);
  return TFI->hasFP(MF) ? Z80::IX : Z80::SP;
}

bool Z80RegisterInfo::getRegAllocationHints(
    Register VirtReg, ArrayRef<MCPhysReg> Order,
    SmallVectorImpl<MCPhysReg> &Hints, const MachineFunction &MF,
    const VirtRegMap *VRM, const LiveRegMatrix *Matrix) const {
  // First, add default hints (copy-related).
  TargetRegisterInfo::getRegAllocationHints(VirtReg, Order, Hints, MF, VRM,
                                            Matrix);

  const MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetRegisterClass *RC = MRI.getRegClass(VirtReg);
  const auto &STI = MF.getSubtarget<Z80Subtarget>();

  // ravn/llvm-z80#115 + #27 S1 instrumentation (session 73): log every
  // hint query so we can see, in any pre-RA dump, which vregs are being
  // hinted, at what RC, and what opcodes consume them.  Used to debug
  // pointer-vreg pressure shapes (aes_mc_inv-class) before designing a
  // single-register-class pre-RA pass.  No behavioural change.  Off by
  // default; enable with `-debug-only=z80-regalloc-hint`.
  if (Z80LogRegallocHints) {
    dbgs() << "z80-regalloc-hint: VReg=" << printReg(VirtReg, this)
           << " RC=" << getRegClassName(RC) << " in "
           << MF.getName() << " uses=[";
    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    bool First = true;
    for (const MachineInstr &Use : MRI.use_nodbg_instructions(VirtReg)) {
      if (!First)
        dbgs() << ",";
      First = false;
      dbgs() << TII->getName(Use.getOpcode());
    }
    dbgs() << "]\n";
  }

  // For 16-bit registers: if the vreg is used by an instruction that
  // constrains its operand to GR16_BCDE (ADD HL,rr / SUB HL,rr etc.),
  // hint DE then BC. This avoids IX/IY which would need an expensive
  // PUSH/POP copy to BC/DE (4 bytes) since ADD HL,IX doesn't exist.
  if (RC->hasSubClassEq(&Z80::GR16RegClass) && STI.hasZ80()) {
    for (const MachineInstr &Use : MRI.use_nodbg_instructions(VirtReg)) {
      for (const MachineOperand &MO : Use.operands()) {
        if (!MO.isReg() || MO.getReg() != VirtReg)
          continue;
        // Check if this operand's constraint narrows to GR16_BCDE
        unsigned OpIdx = &MO - &Use.getOperand(0);
        if (OpIdx < Use.getNumExplicitOperands()) {
          const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
          const TargetRegisterClass *OpRC =
              Use.getRegClassConstraint(OpIdx, TII, this);
          if (OpRC && OpRC->hasSubClassEq(&Z80::GR16_BCDERegClass)) {
            // Hint DE, BC (the only registers that work for ADD/SUB HL,rr)
            if (is_contained(Order, Z80::DE) && !is_contained(Hints, Z80::DE))
              Hints.push_back(Z80::DE);
            if (is_contained(Order, Z80::BC) && !is_contained(Hints, Z80::BC))
              Hints.push_back(Z80::BC);
            goto done_16bit_hints;
          }
        }
      }
    }
    done_16bit_hints:;

    // ravn/llvm-z80#115 + #27 S2 (session 73): a soft `Hints.insert(begin, HL)`
    // for GR16 vregs used by LOAD8_IND/STORE8_IND was attempted here and
    // produced **zero** byte change on the AES corpus — 21 such hints fire
    // in aes_mc_inv alone, but greedy's copy-elim heuristic overrides every
    // one of them.  Confirms the prediction in `tasks/plan-115-27-regalloc-cluster.md`
    // that hint-flavored fixes for #27 don't work on this allocator path.
    // S3 (pre-RA single-register-class pointer-split pass) is the next
    // escalation.  Test-runner 685/42/56/207 unchanged either way.

    // i16 self-loop-counter hint (#99): when an i16 vreg is the
    // back-edge counter of a self-back-edge JR_NZ loop (def-by-DEC16
    // or matching INC16 paired with a 16-bit zero-test branching to
    // the same MBB), hint BC.  Without the hint the default GR16
    // order picks DE (often taken by the loop body's value reg) then
    // HL — and HL is the natural home for a pointer in the same
    // loop, forcing the counter and pointer to ping-pong.  Hinting
    // BC frees HL for the pointer and avoids the per-iteration
    // SPILL/RELOAD of HL across the store.
    if (is_contained(Order, Z80::BC)) {
      bool IsCounter = false;
      for (const MachineInstr &Def : MRI.def_instructions(VirtReg)) {
        if (Def.getOpcode() != Z80::DEC16 &&
            Def.getOpcode() != Z80::INC16)
          continue;
        const MachineBasicBlock *DefMBB = Def.getParent();
        // Check the parent MBB has a self-back-edge JR_NZ / JP_NZ.
        for (const MachineInstr &Term : DefMBB->terminators()) {
          unsigned Opc = Term.getOpcode();
          if (Opc != Z80::JR_NZ_e && Opc != Z80::JP_NZ_nn)
            continue;
          if (Term.getNumOperands() == 0 || !Term.getOperand(0).isMBB())
            continue;
          if (Term.getOperand(0).getMBB() == DefMBB) {
            IsCounter = true;
            break;
          }
        }
        if (IsCounter)
          break;
      }
      // Only emit BC for DEC16 (true countdown counter).  INC16-only
      // vregs are usually pointers being advanced; HL is better there.
      // The hint alone is a soft preference and greedy's copy-elim
      // heuristic typically routes the counter to HL anyway; the real
      // i16-counter fix lives in Z80SplitDjnzCounters which constrains
      // the per-loop counter vreg to the BCReg single-register class.
      // The hint here is a backstop for the basic regalloc.
      if (IsCounter) {
        for (const MachineInstr &Def : MRI.def_instructions(VirtReg)) {
          if (Def.getOpcode() == Z80::DEC16) {
            if (!is_contained(Hints, Z80::BC))
              Hints.insert(Hints.begin(), Z80::BC);
            break;
          }
        }
      }
    }
  }

  // Only hint B for 8-bit registers on Z80 (DJNZ is Z80-only).
  if (!RC->contains(Z80::B) || !STI.hasZ80())
    return false;

  // Check if this vreg looks like a loop counter: it's used in a COPY to A
  // that feeds a DEC_A, OR_A, JR_NZ sequence (the decrement-and-branch
  // pattern). If so, hinting B enables the DEC B; JR NZ → DJNZ peephole.
  for (const MachineInstr &Use : MRI.use_nodbg_instructions(VirtReg)) {
    if (Use.getOpcode() != TargetOpcode::COPY)
      continue;
    Register DstReg = Use.getOperand(0).getReg();
    if (DstReg != Z80::A)
      continue;
    // A is destination of a COPY from our vreg. Check if DEC_A follows.
    auto It = Use.getIterator();
    auto End = Use.getParent()->end();
    ++It;
    if (It == End || It->getOpcode() != Z80::DEC_A)
      continue;
    // Check if the MBB containing this DEC has a conditional NZ branch
    // back to itself (self-back-edge). A self-back-edge marks an
    // innermost single-MBB loop -- the safest place to fire DJNZ.
    // For nested do-while loops, the OUTER counter's dec/jr_nz lives
    // in the outer.latch MBB which branches back to the outer header
    // (a different MBB), not to itself; the INNER counter's dec/jr_nz
    // lives in a self-looping MBB. Issue #92.
    const MachineBasicBlock *DecMBB = Use.getParent();
    bool FeedsCondNZ = false;
    bool IsSelfBackEdge = false;
    // Scan all terminators, not just the last — an unconditional branch
    // after the conditional (non-fallthrough exit) would hide JP_NZ.
    for (auto TI = DecMBB->terminators().begin(),
              TE = DecMBB->terminators().end();
         TI != TE; ++TI) {
      unsigned TermOpc = TI->getOpcode();
      if (TermOpc == Z80::JR_NZ_e || TermOpc == Z80::JP_NZ_nn) {
        FeedsCondNZ = true;
        if (TI->getNumOperands() > 0 && TI->getOperand(0).isMBB() &&
            TI->getOperand(0).getMBB() == DecMBB)
          IsSelfBackEdge = true;
        break;
      }
    }
    if (IsSelfBackEdge) {
      // Innermost loop counter: hint B for DJNZ.
      LLVM_DEBUG(dbgs() << "  DJNZ hint: " << printReg(VirtReg, this)
                        << " is an innermost loop counter, hinting B\n");
      if (is_contained(Order, Z80::B) && !is_contained(Hints, Z80::B))
        Hints.insert(Hints.begin(), Z80::B);
    } else if (FeedsCondNZ) {
      // dec/jr_nz that branches to a DIFFERENT MBB: this is the latch of
      // a multi-block loop, typically the outer of a nested pair. Avoid
      // B so the inner self-looping counter can claim it.
      LLVM_DEBUG(dbgs() << "  DJNZ anti-hint: " << printReg(VirtReg, this)
                        << " is an outer/multi-block loop counter, avoiding B\n");
      static const MCPhysReg NonB[] = {Z80::D, Z80::E, Z80::H,
                                        Z80::L, Z80::C};
      for (MCPhysReg R : NonB) {
        if (is_contained(Order, R) && !is_contained(Hints, R))
          Hints.insert(Hints.begin(), R);
      }
    } else {
      // Outer loop counter (feeds unconditional branch or OR A; JR NZ
      // that the peephole already reduced): AVOID B so inner loops can
      // use it. Hint all non-B registers with priority.
      LLVM_DEBUG(dbgs() << "  DJNZ anti-hint: " << printReg(VirtReg, this)
                        << " is an outer loop counter, avoiding B\n");
      static const MCPhysReg NonB[] = {Z80::D, Z80::E, Z80::H,
                                        Z80::L, Z80::C};
      for (MCPhysReg R : NonB) {
        if (is_contained(Order, R) && !is_contained(Hints, R))
          Hints.insert(Hints.begin(), R);
      }
    }
    break;
  }
  return false;
}

StringRef Z80RegisterInfo::getRegAsmName(MCRegister Reg) const {
  switch (Reg.id()) {
  case Z80::A:
    return "a";
  case Z80::B:
    return "b";
  case Z80::C:
    return "c";
  case Z80::D:
    return "d";
  case Z80::E:
    return "e";
  case Z80::H:
    return "h";
  case Z80::L:
    return "l";
  case Z80::BC:
    return "bc";
  case Z80::DE:
    return "de";
  case Z80::HL:
    return "hl";
  case Z80::IX:
    return "ix";
  case Z80::IY:
    return "iy";
  case Z80::SP:
    return "sp";
  default:
    return "";
  }
}

// ============================================================================
// ravn/llvm-z80#23 Phase 2 (2026-06-08) -- tiered GR16 pressure limit.
// See header for rationale + path-not-taken on the full TableGen reshuffle.
// ============================================================================

static llvm::cl::opt<bool> TieredGR16Pressure(
    "z80-tiered-gr16-pressure", llvm::cl::Hidden, llvm::cl::init(true),
    llvm::cl::desc("Z80 #23 Phase 2: report GR16 pressure limit as 6 "
                   "(3 cheap pairs HL/DE/BC) instead of 12 (all 6 logical "
                   "pairs including IX/IY/AF).  Default ON.  AES -18 B at "
                   "-Oz; autoload unchanged (its over-hoisting is gated by "
                   "isReMaterializable bypass, not pressure -- Phase 3 "
                   "addresses that)."));

unsigned
Z80RegisterInfo::getRegPressureSetLimit(const MachineFunction &MF,
                                        unsigned Idx) const {
  unsigned Default = Z80GenRegisterInfo::getRegPressureSetLimit(MF, Idx);
  if (!TieredGR16Pressure)
    return Default;
  // Pressure set #10 is "GR16" per the TableGen-generated
  // PressureNameTable.  Hard-coding the index is fragile (would break
  // if the .td adds another GR16-equivalent class) but the alternative
  // (string-compare every call) is the hot path.  Phase 3 can revisit.
  // The 6 here is "3 cheap pairs × 2 register units per pair".
  StringRef Name = getRegPressureSetName(Idx);
  if (Name == "GR16")
    return std::min<unsigned>(Default, 6);
  return Default;
}
