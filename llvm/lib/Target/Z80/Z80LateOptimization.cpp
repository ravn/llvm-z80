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
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "z80-late-opt"

using namespace llvm;

// ravn/llvm-z80#205 follow-up (experimental, default OFF).  Rewrite a K=2
// pattern-fill seed `LD HL,VAL; LD (nn),HL; ... ; LD HL,nn; ... ; LDIR` so the
// seed is written through (HL) in reversed byte order, landing HL on the fill
// base -- which is the LDIR source -- and folding away the separate `LD HL,nn`
// source setup.  Saves 1 byte/site UNCOMPRESSED (9 B value+seed+src-load ->
// 8 B).  VAL may be a constant (two imm byte stores) or a symbol (MO_HI/MO_LO
// byte-half stores, lowered to Addr16_High/Low relocations in Z80MCInstLower).
//
// CAVEAT -- why this is default OFF and not adopted by any production target:
// the reversed seed replaces the original's repetitive address bytes
// (`21 5f ed 22 00 ea 21 00 ea`, lots of repeated ea/00/21) with less
// compressible ones (`21 01 ea 36 ed 2b 36 5f`).  On a ZX0-compressed PROM
// (cpnos PROM1, autoload) the worse compressibility costs MORE than the 1 byte
// saved -- measured cpnos PROM1 2032 -> 2033 B, a net REGRESSION -- so the win
// only materialises on uncompressed targets.  Kept behind the flag as correct,
// tested infrastructure (and the first user of the MO_LO/MO_HI byte-half
// symbol-store lowering) pending an uncompressed workload that benefits.
static cl::opt<bool> EnableReverseFillSeed(
    "z80-reverse-fill-seed", cl::Hidden, cl::init(false),
    cl::desc("Z80: rewrite K=2 LDIR fill seed as a reversed (HL) byte store "
             "that lands HL on the fill base (saves 1 byte/site uncompressed; "
             "can regress ZX0-compressed PROMs -- default off)"));

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

// --- Shared predicates for the BSS-spill -> PUSH/POP peepholes -------------
// There are two such peepholes (the single-block one and the cross-MBB one);
// both must apply the SAME safety conditions before converting a frame-slot
// store + matching reload to PUSH/POP, because both drop the memory store.
// These predicates were previously duplicated as per-peephole lambdas and
// drifted -- e.g. the loop-carried "read before store" guard lived in the
// single-block peephole (#195) but was missing from the cross-block one,
// causing #202.  Defining them once here is the single source of truth so a
// guard cannot exist in one peephole and be absent from the other (#203).
static bool z80IsAnyBssLoad(unsigned O) {
  return O == Z80::LD_A_nnind || O == Z80::LD_HL_nnind ||
         O == Z80::LD_DE_nnind || O == Z80::LD_BC_nnind;
}
static bool z80IsAnyBssStore(unsigned O) {
  return O == Z80::LD_nnind_A || O == Z80::LD_nnind_HL ||
         O == Z80::LD_nnind_DE || O == Z80::LD_nnind_BC;
}
static bool z80IsAnyBssAccess(unsigned O) {
  return z80IsAnyBssLoad(O) || z80IsAnyBssStore(O);
}
// PUSH/POP of any register pair (#203: shared by the spill->PUSH/POP peepholes
// for stack-depth tracking; single source of truth so the opcode set can't
// drift between the four peepholes).
static bool z80IsAnyPush(unsigned O) {
  return O == Z80::PUSH_AF || O == Z80::PUSH_BC || O == Z80::PUSH_DE ||
         O == Z80::PUSH_HL || O == Z80::PUSH_IX || O == Z80::PUSH_IY;
}
static bool z80IsAnyPop(unsigned O) {
  return O == Z80::POP_AF || O == Z80::POP_BC || O == Z80::POP_DE ||
         O == Z80::POP_HL || O == Z80::POP_IX || O == Z80::POP_IY;
}
// Same frame slot: operand 0 of two BSS load/store MIs, symbol AND offset
// (MO_MCSymbol::isIdenticalTo ignores the offset, so compare it explicitly --
// distinguishes e.g. __sfrend-10 from __sfrend-16).
static bool z80SameBssAddr(const MachineInstr &A, const MachineInstr &B) {
  const MachineOperand &MA = A.getOperand(0);
  const MachineOperand &MB = B.getOperand(0);
  if (!MA.isIdenticalTo(MB))
    return false;
  if (MA.isMCSymbol())
    return MA.getOffset() == MB.getOffset();
  return true;
}
// Loop-carried guard: is the same slot accessed in MBB BEFORE the store at
// StoreIt?  Such a read is the signature of a loop-carried value whose home is
// this slot (read at the loop top via the back-edge, written here at the
// bottom); converting the store + reload to PUSH/POP drops the store, so the
// back-edge read sees a stale slot every iteration (#195 single-block,
// #202 cross-block).  Both peepholes must bail when this is true.
static bool z80SlotReadBeforeStoreInBlock(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator StoreIt) {
  for (auto P = MBB.begin(); P != StoreIt; ++P)
    if (z80IsAnyBssAccess(P->getOpcode()) && z80SameBssAddr(*StoreIt, *P))
      return true;
  return false;
}

// Address-taken guard (shared by all spill->PUSH/POP peepholes, #195/#204):
// collect every frame symbol/global that appears as an *immediate address*
// (i.e. in an instruction that is NOT a direct BSS load/store -- e.g.
// `LD HL, __sfrend_main` to take &local).  A slot whose base symbol is in this
// set may be read/written INDIRECTLY through a pointer the direct-access scans
// can't see, so its store must NOT be converted to PUSH/POP (the indirect read
// would get a never-written slot).  #195/test_27 (volatile m[3][3]) and #204
// (double-pointer swap: `&x` stored into `px`, x read via `*px` in a callee).
static void z80CollectAddrTakenFrameSyms(MachineFunction &MF,
                                         SmallPtrSetImpl<const void *> &Out) {
  for (MachineBasicBlock &MBB : MF)
    for (MachineInstr &MI : MBB) {
      if (z80IsAnyBssAccess(MI.getOpcode()))
        continue; // a direct memory operand is not "address taken"
      for (const MachineOperand &MO : MI.operands()) {
        if (MO.isMCSymbol())
          Out.insert(MO.getMCSymbol());
        else if (MO.isGlobal())
          Out.insert(MO.getGlobal());
      }
    }
}
// True iff StoreMI's slot (operand 0) has an address-taken base symbol.
static bool z80SlotAddrTaken(const MachineInstr &StoreMI,
                             const SmallPtrSetImpl<const void *> &Set) {
  const MachineOperand &A = StoreMI.getOperand(0);
  const void *Key = A.isMCSymbol() ? (const void *)A.getMCSymbol()
                    : (A.isGlobal() ? (const void *)A.getGlobal() : nullptr);
  return Key && Set.count(Key);
}
// "Is StoreMI's frame slot accessed by some OTHER instruction that would
// observe StoreMI's value?" -- the cross-region orphan guard shared by all
// four spill->PUSH/POP peepholes (#203, replacing four hand-mirrored copies).
// The peepholes differ only in what they exclude and whether they apply the
// #155 relaxation; those are explicit parameters so the one implementation
// cannot drift:
//   SkipBlocks : whole MBBs to skip (single-block peepholes handle same-block
//                conflicts via their own forward scan; the cross-MBB peephole
//                skips MBB_A and MBB_B).
//   SkipMIs    : specific instructions to skip (e.g. the store + its reload).
//   MDT/StoreMBB : when MDT is non-null, an access in a block that DOMINATES
//                StoreMBB is from a strictly-earlier, slot-coalesced lifetime
//                and is allowed (#155) -- it executes before StoreMBB's store.
static bool z80SlotUsedElsewhere(MachineFunction &MF,
                                 const MachineInstr &StoreMI,
                                 ArrayRef<const MachineBasicBlock *> SkipBlocks,
                                 ArrayRef<const MachineInstr *> SkipMIs,
                                 const MachineDominatorTree *MDT,
                                 const MachineBasicBlock *StoreMBB) {
  for (MachineBasicBlock &Other : MF) {
    if (llvm::is_contained(SkipBlocks, &Other))
      continue;
    bool DomSafe = MDT && StoreMBB && MDT->dominates(&Other, StoreMBB);
    for (MachineInstr &OI : Other) {
      if (llvm::is_contained(SkipMIs, &OI))
        continue;
      if (z80IsAnyBssAccess(OI.getOpcode()) && z80SameBssAddr(StoreMI, OI) &&
          !DomSafe)
        return true;
    }
  }
  return false;
}
// An explicit SP write (e.g. LD SP,HL for call-arg cleanup) between a spill
// store and its reload relocates the frame, so a PUSH/POP bracket spanning it
// would pop the wrong slot.  PUSH/POP are tracked separately via stack-depth;
// a CALL is net-SP-neutral (return addr pushed, then popped by RET); any OTHER
// def of SP is unsafe.  Shared by the spill->PUSH/POP peepholes (#203/#198).
static bool z80IsExplicitSPWrite(const MachineInstr &MI,
                                 const TargetRegisterInfo *TRI) {
  unsigned O = MI.getOpcode();
  return !z80IsAnyPush(O) && !z80IsAnyPop(O) && !MI.isCall() &&
         MI.modifiesRegister(Z80::SP, TRI);
}

// Map a zero-displacement IX/IY-indexed load/store opcode to the equivalent
// (HL)-indirect opcode, reporting which index register it uses (IdxReg) and
// whether it carries a trailing 8-bit value immediate (HasValImm, true only
// for `LD (IX+d),n` / `LD (IY+d),n`).  Returns 0 if the opcode is not a
// foldable indexed memory access.
//
// H/L-destination LOADS (LD_H_IXd / LD_L_IXd and IY twins) are intentionally
// excluded: rewriting them to `ld h,(hl)` / `ld l,(hl)` overwrites the HL
// pointer, which would corrupt any later (HL) access in the same fold window.
// Stores from H/L (LD_IXd_H / LD_IXd_L) are fine -- they only read H/L.
//
// The (HL) forms share the same low opcode byte as the indexed forms (the DD/
// FD prefix and the +d displacement byte are simply dropped), so each foldable
// indexed op has a 1:1 (HL) counterpart that is one byte shorter and needs no
// index register.
static unsigned z80IndexedZeroDispToHLIndirect(unsigned Opc, MCPhysReg &IdxReg,
                                               bool &HasValImm) {
  HasValImm = false;
  switch (Opc) {
  // Stores: (IX+0) <- r  ->  (HL) <- r
  case Z80::LD_IXd_B: IdxReg = Z80::IX; return Z80::LD_HLind_B;
  case Z80::LD_IXd_C: IdxReg = Z80::IX; return Z80::LD_HLind_C;
  case Z80::LD_IXd_D: IdxReg = Z80::IX; return Z80::LD_HLind_D;
  case Z80::LD_IXd_E: IdxReg = Z80::IX; return Z80::LD_HLind_E;
  case Z80::LD_IXd_H: IdxReg = Z80::IX; return Z80::LD_HLind_H;
  case Z80::LD_IXd_L: IdxReg = Z80::IX; return Z80::LD_HLind_L;
  case Z80::LD_IXd_A: IdxReg = Z80::IX; return Z80::LD_HLind_A;
  case Z80::LD_IYd_B: IdxReg = Z80::IY; return Z80::LD_HLind_B;
  case Z80::LD_IYd_C: IdxReg = Z80::IY; return Z80::LD_HLind_C;
  case Z80::LD_IYd_D: IdxReg = Z80::IY; return Z80::LD_HLind_D;
  case Z80::LD_IYd_E: IdxReg = Z80::IY; return Z80::LD_HLind_E;
  case Z80::LD_IYd_H: IdxReg = Z80::IY; return Z80::LD_HLind_H;
  case Z80::LD_IYd_L: IdxReg = Z80::IY; return Z80::LD_HLind_L;
  case Z80::LD_IYd_A: IdxReg = Z80::IY; return Z80::LD_HLind_A;
  // Immediate stores: (IX+0) <- n  ->  (HL) <- n
  case Z80::LD_IXd_n: IdxReg = Z80::IX; HasValImm = true; return Z80::LD_HLind_n;
  case Z80::LD_IYd_n: IdxReg = Z80::IY; HasValImm = true; return Z80::LD_HLind_n;
  // Loads: r <- (IX+0)  ->  r <- (HL)   (H/L destinations excluded above)
  case Z80::LD_B_IXd: IdxReg = Z80::IX; return Z80::LD_B_HLind;
  case Z80::LD_C_IXd: IdxReg = Z80::IX; return Z80::LD_C_HLind;
  case Z80::LD_D_IXd: IdxReg = Z80::IX; return Z80::LD_D_HLind;
  case Z80::LD_E_IXd: IdxReg = Z80::IX; return Z80::LD_E_HLind;
  case Z80::LD_A_IXd: IdxReg = Z80::IX; return Z80::LD_A_HLind;
  case Z80::LD_B_IYd: IdxReg = Z80::IY; return Z80::LD_B_HLind;
  case Z80::LD_C_IYd: IdxReg = Z80::IY; return Z80::LD_C_HLind;
  case Z80::LD_D_IYd: IdxReg = Z80::IY; return Z80::LD_D_HLind;
  case Z80::LD_E_IYd: IdxReg = Z80::IY; return Z80::LD_E_HLind;
  case Z80::LD_A_IYd: IdxReg = Z80::IY; return Z80::LD_A_HLind;
  default: return 0;
  }
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
    // ravn/llvm-z80 IY-peephole miscompile (2026-06-08): the tracking
    // above grabs the first PUSH_IY and any POP_IY in the function
    // regardless of basic-block position.  When +undocumented enables
    // a value-transfer pattern (HL -> IY -> HL across a loop), the
    // entry block ends up with PUSH_HL; POP_IY and the loop body
    // has PUSH_IY; POP_HL.  The peephole then matches the entry
    // POP_IY with the loop body's PUSH_IY and erases both, leaving
    // PUSH_HL (entry) and POP_HL (loop body) unpaired -- stack drifts
    // off the caller's frame after iteration 1.  Witnessed in AES
    // _aes_mixColumns with +undocumented; manifests as AES verify
    // FAIL + tstates collapse from 16.58M to 1.78M (program ends
    // early via stack-return corruption).
    //
    // Restrict the optimization to the safe-and-sufficient case: both
    // ops must live in the function's entry block.  A legitimate IY
    // save/restore (when present) usually has both PUSH_IY and POP_IY
    // in the entry block of compact functions, or the optimization
    // is conservatively skipped in multi-block functions.  Missing
    // some valid eliminations is acceptable; producing wrong code is
    // not.
    bool IYPairInEntry =
        PushIY && PopIY && PushIY->getParent() == &MF.front() &&
        PopIY->getParent() == &MF.front();
    if (!IYUsedInBody && IYPairInEntry) {
      LLVM_DEBUG(dbgs() << "  Removing unused IY save/restore\n");
      PopIY->eraseFromParent();
      PushIY->eraseFromParent();
      Changed = true;
    }
  }

  for (MachineBasicBlock &MBB : MF) {
    // (Peephole "POP rr; PUSH rr -> (remove both)" removed in session 73s
    // -- never fires on current production code.  Per ravn/llvm-z80#180 C2
    // re-test methodology: disable + measure; result was AES production
    // .text byte-identical (2228 B), cpnos PROM1 SHRINKS by 1 B
    // (2028 -> 2027 B; pipeline-ordering benefit from removing the
    // peephole), test-runner sweep zero per-test diff.  The pattern
    // targeted SM83's LDHL-SP boilerplate which no longer reaches this
    // pass at HEAD.  See tasks/session73s-issue2-retest.md.)

    // --- Peephole: LD A,r; DEC A; LD r,A; OR A; JR NZ → DEC r; JR NZ ---
    // Replaces a 5-instruction decrement-and-branch sequence (28T, 6B) with
    // DEC r; JR NZ (14T, 3B). DEC r sets Z flag correctly for JR NZ, and
    // stays within the analyzable branch framework. Works on Z80 and SM83.
    //
    // ravn/llvm-z80#221: use SkipPHIsLabelsAndDebug() instead of std::next()
    // to skip DBG_VALUE pseudos that interleave under `-g`.  Raw std::next()
    // lands on a debug pseudo, the opcode check fails, and the peephole
    // silently bails on `-g` builds.  Three production sites in autoload
    // (_delay's mid-counter, _compare_6bytes, _check_sysfile) hit this --
    // 3 B per site × 3 sites = 9 B win on autoload.
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      // Match: LD A,r (identify counter register r)
      Register CounterReg = getLDArSrcReg(MII->getOpcode());
      if (!CounterReg.isValid()) {
        ++MII;
        continue;
      }
      auto I1 = MII;
      auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
      if (I2 == MIE) { ++MII; continue; }
      auto I3 = MBB.SkipPHIsLabelsAndDebug(std::next(I2));
      if (I3 == MIE) { ++MII; continue; }
      auto I4 = MBB.SkipPHIsLabelsAndDebug(std::next(I3));
      if (I4 == MIE) { ++MII; continue; }
      auto I5 = MBB.SkipPHIsLabelsAndDebug(std::next(I4));
      if (I5 == MIE) { ++MII; continue; }

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
      // ravn/llvm-z80#221: range-erase [I1, std::next(I5)) so any DBG_VALUE
      // pseudos interleaved between the matched MIs (referencing the
      // about-to-be-erased $a value) are also removed.  Leaving them would
      // dangle on physreg state that the new DEC r; JR NZ doesn't produce.
      auto EraseEnd = std::next(I5);
      MII = MBB.erase(I1, EraseEnd);
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
    //
    // ravn/llvm-z80#221: use next_nodbg() instead of std::next() to skip
    // DBG_VALUE / DBG_LABEL pseudos that interleave under `-g`.  Raw
    // std::next() lands on a debug pseudo, the opcode check fails, and the
    // peephole silently bails on `-g` builds.
    if (STI.hasZ80()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        if (MII->getOpcode() != Z80::DEC_A) { ++MII; continue; }
        auto I1 = MII;
        auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
        if (I2 == MIE || I2->getOpcode() != Z80::LD_B_A) {
          ++MII; continue;
        }
        auto I3 = MBB.SkipPHIsLabelsAndDebug(std::next(I2));
        if (I3 == MIE) { ++MII; continue; }
        // Optional OR A between LD B,A and JR NZ
        MachineInstr *OrToErase = nullptr;
        auto IBranch = I3;
        if (I3->getOpcode() == Z80::OR_A) {
          OrToErase = &*I3;
          IBranch = MBB.SkipPHIsLabelsAndDebug(std::next(I3));
          if (IBranch == MIE) { ++MII; continue; }
        }
        if (IBranch->getOpcode() != Z80::JR_NZ_e) {
          ++MII; continue;
        }
        // A must be dead after the JR NZ (DJNZ doesn't touch A).
        if (!isRegDeadAfter(std::next(IBranch), MBB, TRI, Z80::A)) {
          ++MII; continue;
        }
        // FLAGS must also be dead after the JR NZ.  DEC A sets Z (which
        // JR NZ tests) and S/P/H from the result; DJNZ doesn't touch
        // FLAGS at all.  If any code past the branch consumes FLAGS, the
        // rewrite would observe the pre-DEC flag state instead.
        // ravn/llvm-z80#108 (site 1).
        if (!isRegDeadAfter(std::next(IBranch), MBB, TRI, Z80::FLAGS)) {
          ++MII; continue;
        }
        // ravn/llvm-z80#185: B must not be redefined anywhere in this
        // MBB between begin() and I1 (DEC_A).  If something else
        // clobbers B (e.g. body uses `ld b, h` for parallel BC
        // pointer arithmetic in aes_done at -Os with i16=2 cost),
        // then the LD_B_A we're about to erase is ESSENTIAL — it
        // restores the counter value into B before the test.
        // DJNZ would operate on the clobbered B and loop ~100+ extra
        // iterations, corrupting low memory.
        bool BClobberedInBody = false;
        for (auto It = MBB.begin(); It != I1; ++It) {
          if (It->modifiesRegister(Z80::B, TRI)) {
            BClobberedInBody = true;
            break;
          }
        }
        if (BClobberedInBody) { ++MII; continue; }
        MachineBasicBlock *TargetMBB = IBranch->getOperand(0).getMBB();
        DebugLoc DL = I1->getDebugLoc();
        LLVM_DEBUG(dbgs() << "  DEC A; LD B,A; [OR A;] JR NZ → DJNZ\n");
        // ravn/llvm-z80#221: erase [I1, IBranch] as a half-open range
        // (one past IBranch) so any intervening DBG_VALUE pseudos (which
        // tracked values through A that no longer exist after the rewrite)
        // are also removed.  Leaving them would dangle on physreg state
        // that the DJNZ doesn't produce.
        auto EraseEnd = std::next(IBranch);
        MII = MBB.erase(I1, EraseEnd);
        BuildMI(MBB, MII, DL, TII->get(Z80::DJNZ_e)).addMBB(TargetMBB);
        Changed = true;
        // Suppress unused-variable warning on OrToErase — the range erase
        // above already removed it (it lies within [I1, IBranch]).
        (void)OrToErase;
      }
    }

    // --- Peephole: DEC B; JR NZ → DJNZ (Z80 only) ---
    // DJNZ is a 2-byte instruction that decrements B and branches if non-zero.
    // Replaces DEC B (1 byte) + JR NZ (2 bytes) = 3 bytes with DJNZ (2 bytes).
    //
    // ravn/llvm-z80#221: use SkipPHIsLabelsAndDebug() instead of std::next()
    // to skip DBG_VALUE pseudos under `-g`.
    if (STI.hasZ80()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        if (MII->getOpcode() != Z80::DEC_B) {
          ++MII;
          continue;
        }
        auto NextIt = MBB.SkipPHIsLabelsAndDebug(std::next(MII));
        if (NextIt == MIE || NextIt->getOpcode() != Z80::JR_NZ_e) {
          ++MII;
          continue;
        }
        // FLAGS must be dead after the JR NZ.  DEC B sets Z (tested by
        // JR NZ) plus S/P/H from the result; DJNZ doesn't touch FLAGS.
        // ravn/llvm-z80#108 (site 2).
        if (!isRegDeadAfter(std::next(NextIt), MBB, TRI, Z80::FLAGS)) {
          ++MII;
          continue;
        }
        MachineBasicBlock *TargetMBB = NextIt->getOperand(0).getMBB();
        DebugLoc DL = MII->getDebugLoc();
        LLVM_DEBUG(dbgs() << "  DEC B; JR NZ → DJNZ\n");
        // Range-erase [MII, NextIt] (one past NextIt) so intervening
        // DBG_VALUE pseudos are removed too.  See #221 rationale.
        auto EraseEnd = std::next(NextIt);
        MII = MBB.erase(MII, EraseEnd);
        BuildMI(MBB, MII, DL, TII->get(Z80::DJNZ_e)).addMBB(TargetMBB);
        Changed = true;
      }
    }

    // --- Peephole RETIRED (#180, fully migrated to ISel): XOR #0xFF -> CPL ---
    // Both emitters of a flags-dead XOR_n 0xFF now produce CPL directly in the
    // instruction selector:
    //   * standalone `~x` (G_XOR x, 0xFF), all widths -- session-73q C1;
    //   * the i16 `== -1` / `!= -1` comparison fallback's byte inversions
    //     (Z80InstructionSelector.cpp, the CVal>=0 byte-XOR path) -- this
    //     change, ravn/llvm-z80#180/#149.
    // No flags-dead XOR_n 0xFF reaches post-RA anymore (verified: codegen
    // byte-identical across the Z80 lit suite at -O2/-Oz with this peephole
    // removed and the ISel CPL emit in place).  Flags-LIVE XOR_n 0xFF must keep
    // XOR (CPL doesn't set S/Z/P), which this peephole's FLAGS-dead guard
    // already refused -- so nothing is lost.

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
          MachineInstr *NewMI =
              BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(Z80::XOR_A));
          // XOR A sets A to 0 regardless of A's prior value, so its implicit
          // read of A is a don't-care.  LD A,#0 only *defined* A; A may be dead
          // here (e.g. a `return 0;` block whose predecessor's A is not live in
          // this block).  Mark the use undef so the read does not require A to
          // be live, otherwise -verify-machineinstrs reports "Using an
          // undefined physical register" at the XOR A (#194).
          for (MachineOperand &MO : NewMI->operands())
            if (MO.isReg() && MO.isUse() && MO.getReg() == Z80::A)
              MO.setIsUndef(true);
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
      // #180 C2 RE-TEST (session 73s): disabling this peephole grows AES
      // 09_Oz_prod_like .text by +18 B (2228 -> 2246).  PEEPHOLE IS LIVE.  Keep.
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

    // (Peephole "OR A; LD r,0; JR Z -> OR A; LD r,A; JR Z" removed in
    // session 73s -- never fires on current production targets.  Per
    // ravn/llvm-z80#180 C2 re-test methodology: disable + measure;
    // result was AES `-Oz +static-stack -disable-lsr -disable-licm
    // -disable-cse -ffunction-sections -fdata-sections` `.text` 2228 B
    // byte-identical, cpnos PROM1 2028 B byte-identical, test-runner
    // sweep zero per-test diff.  Same pattern as #15 / #11 retests:
    // peephole's input shape (select-lowering OR_A + LD r,0 + JR Z)
    // no longer appears in clang output post session-73p TTI/cost
    // changes.  See tasks/session73s-issue9-retest.md.)

    // --- Peephole: LD rr,nn; INC/DEC rr → LD rr,nn±1 ---
    // Fold a 16-bit increment/decrement into the preceding immediate load.
    // LD rr,nn (3B) + INC/DEC rr (1B) = 4B → LD rr,nn±1 (3B). Saves 1B.
    // INC/DEC rr doesn't set flags, so no flag dependency to worry about.
    //
    // Re-test in session 73s (#180 C2): disable -> cpnos PROM1 +4 B
    // (2027 -> 2031).  PEEPHOLE IS LIVE.  Keep.
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

    // (ALU #imm; ALU #imm idempotent collapse peephole removed in
    // session 73s -- never fires on current production code.  Per
    // ravn/llvm-z80#180 C2 re-test methodology: disable + measure;
    // result was -1 B cpnos PROM1 (pipeline-ordering side effect),
    // AES byte-identical, lit clean, test-runner zero per-test diff.
    // Same pattern as the #15 retest: peephole's input shape no
    // longer appears in clang output.  See tasks/session73s-issue11-retest.md.)

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
      // #180 C2 RE-TEST (session 73s): disabling this peephole grows AES
      // 09_Oz_prod_like .text by +4 B (2228 -> 2232).  PEEPHOLE IS LIVE.  Keep.
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
        // Safety (#159): the LD r,A save we're about to erase establishes
        // TempReg's value.  If TempReg is needed after the ALU instruction
        // (i.e., not dead), erasing the save leaves a later read observing
        // whatever was in TempReg before — silent miscompile.
        //
        // Use computeRegisterLiveness rather than just MO.isKill() (#161):
        // post-RA regalloc doesn't always mark the last use of a register
        // with a kill flag, even when the register IS provably dead after
        // that use.  The kill-flag-only check left 2 B on the table per
        // safe site (autoload-in-c's main_relocated had one such site).
        // computeRegisterLiveness queries actual liveness in the local
        // neighborhood.  Treat LQR_Unknown as live for safety.
        auto NextIt = std::next(It);
        auto Liveness = MBB.computeRegisterLiveness(TRI, TempReg, NextIt);
        if (Liveness != MachineBasicBlock::LQR_Dead) continue;
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

    // --- Peephole: LD r,(IX+d); <ALU> r → <ALU> a,(IX+d) (#175) ---
    // After PEI rewrites spill reloads to LD <r>,(IX+d), and the per-register
    // ALU op consumes <r> with A as accumulator, the two-MI sequence
    // (4 B / 23 ts) can become a single fused indexed-ALU (3 B / 19 ts).
    // Safe when r is dead after the ALU.  Skip dst=A: `LD A,(IX+d); XOR A`
    // is `A ^= (IX+d); A ^= A` which clears A -- rewriting to `XOR a,(IX+d)`
    // would XOR (IX+d) with the prior A, semantically different.
    {
      // Map per-register ALU opcode (XOR_E, OR_C, ...) -> (dst-register, base
      // operation).  Returns Register() if not a fusable ALU.
      auto getAluSrcReg = [](unsigned Opc) -> Register {
        switch (Opc) {
        case Z80::XOR_B: case Z80::OR_B: case Z80::AND_B: case Z80::SUB_B:
        case Z80::CP_B:  return Z80::B;
        case Z80::XOR_C: case Z80::OR_C: case Z80::AND_C: case Z80::SUB_C:
        case Z80::CP_C:  return Z80::C;
        case Z80::XOR_D: case Z80::OR_D: case Z80::AND_D: case Z80::SUB_D:
        case Z80::CP_D:  return Z80::D;
        case Z80::XOR_E: case Z80::OR_E: case Z80::AND_E: case Z80::SUB_E:
        case Z80::CP_E:  return Z80::E;
        case Z80::XOR_H: case Z80::OR_H: case Z80::AND_H: case Z80::SUB_H:
        case Z80::CP_H:  return Z80::H;
        case Z80::XOR_L: case Z80::OR_L: case Z80::AND_L: case Z80::SUB_L:
        case Z80::CP_L:  return Z80::L;
        default: return Register();
        }
      };
      auto getIXFusedOp = [](unsigned AluOpc) -> unsigned {
        switch (AluOpc) {
        case Z80::XOR_B: case Z80::XOR_C: case Z80::XOR_D: case Z80::XOR_E:
        case Z80::XOR_H: case Z80::XOR_L: return Z80::XOR_IXd;
        case Z80::OR_B:  case Z80::OR_C:  case Z80::OR_D:  case Z80::OR_E:
        case Z80::OR_H:  case Z80::OR_L:  return Z80::OR_IXd;
        case Z80::AND_B: case Z80::AND_C: case Z80::AND_D: case Z80::AND_E:
        case Z80::AND_H: case Z80::AND_L: return Z80::AND_IXd;
        case Z80::SUB_B: case Z80::SUB_C: case Z80::SUB_D: case Z80::SUB_E:
        case Z80::SUB_H: case Z80::SUB_L: return Z80::SUB_IXd;
        case Z80::CP_B:  case Z80::CP_C:  case Z80::CP_D:  case Z80::CP_E:
        case Z80::CP_H:  case Z80::CP_L:  return Z80::CP_IXd;
        default: return 0;
        }
      };
      auto getIYFusedOp = [](unsigned AluOpc) -> unsigned {
        switch (AluOpc) {
        case Z80::XOR_B: case Z80::XOR_C: case Z80::XOR_D: case Z80::XOR_E:
        case Z80::XOR_H: case Z80::XOR_L: return Z80::XOR_IYd;
        case Z80::OR_B:  case Z80::OR_C:  case Z80::OR_D:  case Z80::OR_E:
        case Z80::OR_H:  case Z80::OR_L:  return Z80::OR_IYd;
        case Z80::AND_B: case Z80::AND_C: case Z80::AND_D: case Z80::AND_E:
        case Z80::AND_H: case Z80::AND_L: return Z80::AND_IYd;
        case Z80::SUB_B: case Z80::SUB_C: case Z80::SUB_D: case Z80::SUB_E:
        case Z80::SUB_H: case Z80::SUB_L: return Z80::SUB_IYd;
        case Z80::CP_B:  case Z80::CP_C:  case Z80::CP_D:  case Z80::CP_E:
        case Z80::CP_H:  case Z80::CP_L:  return Z80::CP_IYd;
        default: return 0;
        }
      };
      auto getLoadIYdDstReg = [](unsigned Opc) -> Register {
        switch (Opc) {
        case Z80::LD_A_IYd: return Z80::A;
        case Z80::LD_B_IYd: return Z80::B;
        case Z80::LD_C_IYd: return Z80::C;
        case Z80::LD_D_IYd: return Z80::D;
        case Z80::LD_E_IYd: return Z80::E;
        case Z80::LD_H_IYd: return Z80::H;
        case Z80::LD_L_IYd: return Z80::L;
        default: return Register();
        }
      };
      for (auto MII = MBB.begin(); MII != MBB.end();) {
        auto I1 = MII;
        auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
        if (I2 == MBB.end()) { ++MII; continue; }
        unsigned LdOpc = I1->getOpcode();
        unsigned AluOpc = I2->getOpcode();
        Register LdDst = getLoadIXdDstReg(LdOpc);
        bool IsIX = LdDst.isValid();
        if (!IsIX)
          LdDst = getLoadIYdDstReg(LdOpc);
        Register AluSrc = getAluSrcReg(AluOpc);
        if (!LdDst.isValid() || !AluSrc.isValid() || LdDst != AluSrc) {
          ++MII; continue;
        }
        // Skip dst=A: A^A = 0 (not equivalent to XOR a,(IX+d)).
        if (LdDst == Z80::A) { ++MII; continue; }
        unsigned FusedOpc = IsIX ? getIXFusedOp(AluOpc) : getIYFusedOp(AluOpc);
        if (!FusedOpc) { ++MII; continue; }
        if (I1->getNumOperands() < 1 || !I1->getOperand(0).isImm()) {
          ++MII; continue;
        }
        int64_t Disp = I1->getOperand(0).getImm();
        auto AfterAlu = std::next(I2);
        auto Liveness = MBB.computeRegisterLiveness(TRI, LdDst, AfterAlu);
        if (Liveness != MachineBasicBlock::LQR_Dead) { ++MII; continue; }
        LLVM_DEBUG(dbgs() << "  #175: LD " << TRI->getName(LdDst)
                          << ",(" << (IsIX ? "IX" : "IY") << "+" << Disp
                          << "); " << TII->getName(AluOpc)
                          << " " << TRI->getName(LdDst) << " -> "
                          << TII->getName(FusedOpc) << " a,("
                          << (IsIX ? "IX" : "IY") << "+" << Disp << ")\n");
        BuildMI(MBB, I1, I1->getDebugLoc(), TII->get(FusedOpc)).addImm(Disp);
        auto AfterErase = std::next(I2);
        I1->eraseFromParent();
        I2->eraseFromParent();
        MII = AfterErase;
        Changed = true;
      }
    }

    // --- Peephole: LD L,H; LD H,0; LD A,L → LD A,H ---
    // When extracting the high byte of HL into A, the compiler sometimes
    // routes through L (LD L,H; LD H,0; LD A,L) instead of directly (LD A,H).
    // This happens when ISel zero-extends the high byte into HL (for potential
    // 16-bit use) but the result is only consumed as an 8-bit value in A.
    // The peephole replaces the 3-instruction sequence (4B) with LD A,H (1B).
    // Safe when H and L are dead after (the LD H,0 overwrites H, and LD A,L
    // is the last use of L before it's overwritten or dead).
    //
    // Re-test in session 73s (#180 C2): disable -> cpnos PROM1 +1 B
    // (2027 -> 2028).  PEEPHOLE IS LIVE.  Keep.
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
        // Liveness guard (ravn/llvm-z80#242): this fold rewrites the sequence to
        // a single `LD A,H` at the position of the original `LD L,H`, DELETING
        // the `LD H,0` and the `LD L,H`.  That is only correct if both H and L
        // are dead after the `LD A,L` — otherwise a later read sees the wrong
        // value: H keeps its pre-zero contents (the zeroing is gone) and L keeps
        // its old value (the copy is gone).  The historical comment claimed this
        // precondition but never checked it; at -O0 a 16-bit compare/op with a
        // zext-from-i8 operand expands to exactly `LD L,H; LD H,0; LD A,L; ...;
        // LD A,H`, where the trailing `LD A,H` reads the zeroed high byte.
        // Dropping the `LD H,0` then corrupts that read (e.g. 255u > 1u → false).
        auto AfterLdA = std::next(MachineBasicBlock::iterator(LdA));
        if (!isRegDeadAfter(AfterLdA, MBB, TRI, Z80::H) ||
            !isRegDeadAfter(AfterLdA, MBB, TRI, Z80::L))
          continue;
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
    //
    // Re-test in session 73s (#180 C2): disable -> cpnos PROM1 +7 B,
    // AES production +14 B.  PEEPHOLE IS LIVE.  Keep.
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

    // (16-bit increment overflow test peephole removed in session 73q
    // -- never fired on current production code.  ISel no longer emits
    // the `LD HL,1; ADD HL,rr; SBC A,A; AND 1; ...; JR NZ/Z` shape that
    // this peephole targeted, because the IR canonicalization driven by
    // session-73p Phase 2 #177 TTI hooks (+ InstCombine equality folds)
    // produces a `SBC A,A; RRCA; JR C` shape instead.  Verified via full
    // test-runner sweep (990/689/38/56/207 with zero per-test diff) and
    // cpnos PROM1 size (2028 B unchanged).  See #15 in #180 audit and
    // tasks/session73q-issue15-retest.md.)

    // --- Peephole: ADD HL,rr commutativity ---
    // LD C,L; LD B,H; EX DE,HL; ADD HL,BC → ADD HL,DE
    // (with optional trailing EX DE,HL if result needed in DE)
    // Addition is commutative: HL+DE == DE+HL. The compiler generates
    // the long form when it wants base(DE)+offset(HL) into HL, but
    // ADD HL,DE gives the same result directly.
    //
    // Re-test in session 73s (#180 C2): disable -> cpnos PROM1 +1 B
    // (2027 -> 2028).  Either real firing or pipeline noise; keep.
    if (STI.hasZ80()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        // Match: LD C,L; LD B,H (copy HL → BC)
        if (MII->getOpcode() != Z80::LD_C_L) { ++MII; continue; }
        auto I0 = MII; // LD C,L
        // ravn/llvm-z80#241: use SkipPHIsLabelsAndDebug to skip DBG_VALUE
        // pseudos that interleave under -g; raw std::next lands on them.
        auto I1 = MBB.SkipPHIsLabelsAndDebug(std::next(I0));
        if (I1 == MIE) { ++MII; continue; }
        if (I1->getOpcode() != Z80::LD_B_H) { ++MII; continue; }
        auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
        if (I2 == MIE) { ++MII; continue; }
        if (I2->getOpcode() != Z80::EX_DE_HL) { ++MII; continue; }
        auto I3 = MBB.SkipPHIsLabelsAndDebug(std::next(I2));
        if (I3 == MIE) { ++MII; continue; }
        if (I3->getOpcode() != Z80::ADD_HL_BC) { ++MII; continue; }

        // Matched: LD C,L; LD B,H; EX DE,HL; ADD HL,BC
        // Replace with: ADD HL,DE
        // Safety: the original writes BC (clobbers it).  Our replacement
        // does not write BC, so any downstream read of BC will see the
        // pre-block value instead of (old HL).  Bail if BC is live past
        // the rewrite point.  See ravn/llvm-z80#109.
        DebugLoc DL = I0->getDebugLoc();

        // Check for trailing EX DE,HL (result needed in DE).
        auto I4 = MBB.SkipPHIsLabelsAndDebug(std::next(I3));
        bool HasTrailingEX = (I4 != MIE && I4->getOpcode() == Z80::EX_DE_HL);

        // BC safety: must be dead after the rewrite point.  Without this
        // check, regalloc choices that reuse BC across the original block
        // (relying on BC's then-current value being the old HL) would
        // miscompile after the rewrite removes the BC write.  Empirically
        // GISel doesn't seem to produce such shapes, but the check is
        // cheap and matches the original safety comment that was
        // aspirational, not enforced (#109).
        auto AfterRewrite = HasTrailingEX
                                ? MBB.SkipPHIsLabelsAndDebug(std::next(I4))
                                : MBB.SkipPHIsLabelsAndDebug(std::next(I3));
        if (!isRegDeadAfter(AfterRewrite, MBB, TRI, Z80::BC)) {
          ++MII;
          continue;
        }

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
    //
    // #180 C2 RE-TEST (session 73s, finalized in resume): production targets
    // are byte-neutral when disabled (cpnos PROM1 2028, AES .text 2228 -- those
    // corpora contain no matching shape today), BUT inmem-incdec-positive.ll
    // REGRESSES: ISel still emits `LD A,(addr); INC/DEC A; LD (addr),A` and this
    // peephole is the sole remover (-2 B/site).  Same byte-neutral-but-live
    // pattern as #21/#79 -- the per-pattern lit canary is the decisive signal
    // when the production corpus is neutral.  PEEPHOLE IS LIVE.  Keep.
    if (STI.hasZ80()) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        // Match I0: LD A,(addr)
        if (MII->getOpcode() != Z80::LD_A_nnind) { ++MII; continue; }
        auto I0 = MII;
        if (!I0->getOperand(0).isGlobal() && !I0->getOperand(0).isSymbol()) {
          ++MII; continue;
        }

        // ravn/llvm-z80#241: SkipPHIsLabelsAndDebug to skip DBG_VALUE pseudos.
        auto I1 = MBB.SkipPHIsLabelsAndDebug(std::next(I0));
        if (I1 == MIE) { ++MII; continue; }
        // Match I1: INC A or DEC A
        bool IsInc = (I1->getOpcode() == Z80::INC_A);
        bool IsDec = (I1->getOpcode() == Z80::DEC_A);
        if (!IsInc && !IsDec) { ++MII; continue; }

        auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
        if (I2 == MIE) { ++MII; continue; }
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
        auto I3 = MBB.SkipPHIsLabelsAndDebug(std::next(I2));
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
          auto INext = MBB.SkipPHIsLabelsAndDebug(std::next(IBr));
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
          } else if (Opc3 == Z80::OR_A) {
            auto I3Next = MBB.SkipPHIsLabelsAndDebug(std::next(I3));
            if (I3Next != MIE && isZNZBranch(I3Next->getOpcode())) {
              // OR A; JR Z/NZ — OR A is a redundant flag test; DEC/INC (HL)
              // already sets Z. Remove the OR A too.
              ExtraToErase = &*I3;
              ADead = checkADeadAfterBranch(I3Next);
            }
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

        // The transformation emits `LD HL, addr; INC/DEC (HL)`, which
        // clobbers H and L (the LD HL,nn) and also implicitly reads
        // them (INC (HL) reads HL).  We must not fire if either H or L
        // is currently live across the original 3-instruction sequence
        // — otherwise we destroy a value the surrounding code relies on
        // (#104: c1 was held in H across an inlined check_true body
        //  that did `LD HL,_side_effect_counter; INC (HL)`, so the
        //  caller's c1 was lost).  The original sequence only uses A,
        //  so H and L are otherwise free.  Compute liveness post-I2 by
        //  walking back from MBB live-outs and skip the rewrite if H,
        //  L, or HL is still live there.
        {
          LivePhysRegs LiveRegs(*TRI);
          LiveRegs.addLiveOuts(MBB);
          for (auto Iter = MBB.rbegin();
               Iter != MBB.rend() && &*Iter != &*I2; ++Iter)
            LiveRegs.stepBackward(*Iter);
          if (LiveRegs.contains(Z80::H) || LiveRegs.contains(Z80::L) ||
              LiveRegs.contains(Z80::HL)) {
            ++MII;
            continue;
          }
        }

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
    //
    // Re-test in session 73s (#180 C2): disable -> cpnos PROM1 +2 B
    // (2027 -> 2029).  PEEPHOLE IS LIVE.  Keep.
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
        // ravn/llvm-z80#241: SkipPHIsLabelsAndDebug to skip DBG_VALUE pseudos.
        auto I1 = MBB.SkipPHIsLabelsAndDebug(std::next(I0));
        if (I1 == MIE) { ++MII; continue; }
        // I1 must be LD A,#imm
        if (I1->getOpcode() != Z80::LD_A_n) { ++MII; continue; }
        int64_t Imm = I1->getOperand(0).getImm();

        auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
        if (I2 == MIE) { ++MII; continue; }
        // I2 must be CP r (matching the register from LD r,A)
        if (I2->getOpcode() != ExpCP) { ++MII; continue; }

        auto I3 = MBB.SkipPHIsLabelsAndDebug(std::next(I2));
        if (I3 == MIE) { ++MII; continue; }
        // I3 must be a carry-based branch
        auto [FlippedBr, IsCarry] = flipCarryBranch(I3->getOpcode());
        if (!IsCarry) { ++MII; continue; }

        // "imm < A_orig" (JR C) → "A_orig >= imm+1" → CP (imm+1); JR NC
        // "imm >= A_orig" (JR NC) → "A_orig < imm+1" → CP (imm+1); JR C
        // Only valid when imm < 255 (imm+1 doesn't overflow 8 bits).
        if (Imm >= 255) { ++MII; continue; }

        // FLAGS must be dead after the branch.  The rewrite changes the
        // CP operand (different result), so Z/S/P/H differ between the
        // original and the rewrite.  Only carry is preserved-and-flipped
        // (the branch reads it correctly).  ravn/llvm-z80#108 (site 4).
        if (!isRegDeadAfter(MBB.SkipPHIsLabelsAndDebug(std::next(I3)),
                            MBB, TRI, Z80::FLAGS)) {
          ++MII; continue;
        }

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
    //
    // Re-test in session 73s (#180 C2): disable -> cpnos PROM1 +1 B
    // (2027 -> 2028).  Either real firing or pipeline noise; keep.
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
    //
    // Re-test in session 73s (#180 C2): disable -> cpnos PROM1 +1 B
    // (2027 -> 2028).  Either real firing or pipeline noise; keep.
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

      // #180 C2 RE-TEST (session 73s): AES 09_Oz_prod_like .text and cpnos
      // PROM1 are byte-neutral when this is disabled (those targets contain no
      // matching shapes today), but bool-store-no-mask.ll (#83) REGRESSES:
      // ISel still emits `ld a,#1; and #1` for `store i1 true` and this peephole
      // is the sole remover of the dead AND.  PEEPHOLE IS LIVE.  Keep.
      // (The #60-imm sub-case, known-zero-a.ll, now passes without this block --
      // handled upstream -- but the #83 path keeps the whole block live.)
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

    // (Peephole HL save-via-BC roundtrip, issue #84, removed in
    // session 73s -- never fires on current production code.  Per
    // ravn/llvm-z80#180 C2 re-test methodology: disable + measure;
    // result was cpnos PROM1 byte-identical, AES production target
    // byte-identical, test-runner sweep zero per-test diff
    // (990/690/37/56/207 unchanged).  Same pattern as session-73s
    // peephole #24 removal: GISel canonicalization no longer emits
    // the HL-via-BC save/restore shape for 
    // loops.  See tasks/session73s-issue23-retest.md.)


    // (Peephole BC ping-pong in single-BB self-loops, issue #97,
    // removed in session 73s -- never fires on current production
    // code.  Per ravn/llvm-z80#180 C2 re-test methodology:
    // disable + measure; result was AES production byte-identical,
    // cpnos PROM1 -1 B (pipeline-ordering benefit from removing
    // ~340 LOC dead peephole), test-runner sweep zero per-test diff
    // (990/690/37/56/207 unchanged).  Same pattern as #15 / #11 /
    // #9 / #2: Z80LoopRotate is disabled at default-on opt levels
    // and the hand-written self-loop shape this peephole targeted
    // no longer reaches this pass.  See tasks/session73s-issue24-retest.md.)


    // --- Peephole: u8 switch range-check 16-bit → 8-bit (issue #86) ---
    //
    // Re-test in session 73s (#180 C2): disable -> cpnos PROM1 +3 B
    // (2028 -> 2031).  PEEPHOLE IS LIVE.  Keep.
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

        // FLAGS must be dead after the branch.  The original chain
        // sets FLAGS via SBC A,H (16-bit SBC result); the rewrite
        // sets FLAGS via CP_n (8-bit subtract).  Carry is preserved
        // (and flipped via the branch swap), but Z/S/P/H bits differ.
        // ravn/llvm-z80#108 (site 5).
        if (!isRegDeadAfter(std::next(Br), MBB, TRI, Z80::FLAGS)) {
          ++MII; continue;
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
    //
    // Note (session 42, ravn/llvm-z80#120): an attempt to migrate this
    // to a GISel combiner ruled out the obvious approaches.  The
    // peephole is structurally correct because it operates POST-ISel
    // where it can rely on the target-specific invariant "after SBC
    // A,A on Z80, the value in A is a full mask" -- the truncate-and-
    // re-sign-extend tail is then a no-op on the physical register.
    // A GISel combiner runs PRE-ISel where this invariant doesn't
    // exist: Z80's BooleanContents is ZeroOrOne, so the i1 result of
    // G_ICMP only has its low bit defined and the (shl 7; ashr 7)
    // idiom is a meaningful widen, not an identity.  Eliding it at
    // the GISel layer produces wrong code for any consumer that needs
    // the full mask.  Migration would require either a target-specific
    // post-ISel combiner or splitting G_ICMP lowering into a "produce
    // mask" form whose i8 result is contractually full-mask.  Both
    // are larger surgery than the peephole.  See
    // tasks/issue-120-combiner-scoping-2026-05-03.md.
    {

      SmallVector<MachineInstr *, 8> ToErase;
      // #180 C2 RE-TEST (session 73s): AES 09_Oz_prod_like .text is byte-neutral
      // when disabled (no matching shape in that corpus), but mask-from-flag.ll
      // REGRESSES: ISel still emits the 5-instr `and $1; rrca; and $80; add a,a;
      // sbc a,a` mask-roundtrip for `(x!=y)?0xFF:0` and this is its sole remover
      // (-8 B/site).  PEEPHOLE IS LIVE.  Keep.
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
          // Liveness guard (#107, same anti-pattern as #104): the
          // rewrite emits `LD HL, base` and walks HL via `INC HL` for
          // every store after the first.  This clobbers H, L, and HL.
          // The original chain only uses A, so H/L are otherwise free.
          // Walk back from MBB live-outs to the chain head and bail if
          // H, L, or HL is live there — otherwise we'd destroy a value
          // the surrounding code relies on (e.g. the i16 ptr argument
          // arriving in HL via sdcccall).
          {
            LivePhysRegs LiveRegs(*TRI);
            LiveRegs.addLiveOuts(MBB);
            for (auto Iter = MBB.rbegin();
                 Iter != MBB.rend() && &*Iter != &*MII; ++Iter)
              LiveRegs.stepBackward(*Iter);
            if (LiveRegs.contains(Z80::H) || LiveRegs.contains(Z80::L) ||
                LiveRegs.contains(Z80::HL)) {
              MII = std::next(It2);
              continue;
            }
          }
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

    // --- Peephole #116: i16 EQ/NE byte-XOR → AND A; SBC HL,rr ---
    //
    // Variable-RHS i16 EQ/NE compare-and-branch is emitted by ISel as a
    // 6-byte byte-level XOR sequence:
    //
    //   LD A,X        ; X = sub_{hi,lo} of QPair (the "loaded" pair)
    //   XOR R1        ; R1 = sub_{hi,lo} of PPair (the "XOR'd" pair)
    //   LD T,A        ; T = some GR8 scratch
    //   LD A,Y        ; Y = the other half of QPair
    //   XOR R2        ; R2 = the other half of PPair
    //   OR T          ; combine; Z=1 iff QPair == PPair
    //   JR Z|NZ / JP Z|NZ
    //
    // When one of the pairs is already HL, AND A; SBC HL,otherpair (3 B,
    // 19 T) does the same compare in half the bytes and saves 5 T-states.
    //
    // Prior attempt at this transform via an ISel-time gate
    // (commit 33ceae174673) regressed rcbios bios.cim by +27 B because
    // SUB_HL_rr's HL-Def evicts long-lived values out of HL.  The
    // post-RA approach below only fires when the pattern's actual
    // physical-register placement *already* parks the value in HL,
    // sidestepping the regalloc-eviction problem.
    if (STI.hasZ80() && !STI.hasSM83()) {
      auto isXorR = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::XOR_B: return Z80::B; case Z80::XOR_C: return Z80::C;
        case Z80::XOR_D: return Z80::D; case Z80::XOR_E: return Z80::E;
        case Z80::XOR_H: return Z80::H; case Z80::XOR_L: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      auto isOrR = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::OR_B: return Z80::B; case Z80::OR_C: return Z80::C;
        case Z80::OR_D: return Z80::D; case Z80::OR_E: return Z80::E;
        case Z80::OR_H: return Z80::H; case Z80::OR_L: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      auto isLdAr = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::LD_A_B: return Z80::B; case Z80::LD_A_C: return Z80::C;
        case Z80::LD_A_D: return Z80::D; case Z80::LD_A_E: return Z80::E;
        case Z80::LD_A_H: return Z80::H; case Z80::LD_A_L: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      auto isLdrA = [](unsigned Opc) -> MCPhysReg {
        switch (Opc) {
        case Z80::LD_B_A: return Z80::B; case Z80::LD_C_A: return Z80::C;
        case Z80::LD_D_A: return Z80::D; case Z80::LD_E_A: return Z80::E;
        case Z80::LD_H_A: return Z80::H; case Z80::LD_L_A: return Z80::L;
        default: return MCPhysReg(0);
        }
      };
      auto pairOf = [](MCPhysReg R) -> MCPhysReg {
        switch (R) {
        case Z80::B: case Z80::C: return Z80::BC;
        case Z80::D: case Z80::E: return Z80::DE;
        case Z80::H: case Z80::L: return Z80::HL;
        default: return MCPhysReg(0);
        }
      };
      auto isHiByte = [](MCPhysReg R) -> bool {
        return R == Z80::B || R == Z80::D || R == Z80::H;
      };
      auto sbcHLOpc = [](MCPhysReg Pair) -> unsigned {
        switch (Pair) {
        case Z80::BC: return Z80::SBC_HL_BC;
        case Z80::DE: return Z80::SBC_HL_DE;
        default: return 0;
        }
      };

      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
        // I1: LD A,X
        MCPhysReg X = isLdAr(MII->getOpcode());
        if (!X) { ++MII; continue; }
        auto I1 = MII;
        // ravn/llvm-z80#221: use SkipPHIsLabelsAndDebug to skip DBG_VALUE
        // pseudos that interleave under -g; raw std::next lands on them.
        auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
        if (I2 == MIE) { ++MII; continue; }
        // I2: XOR R1
        MCPhysReg R1 = isXorR(I2->getOpcode());
        if (!R1) { ++MII; continue; }
        auto I3 = MBB.SkipPHIsLabelsAndDebug(std::next(I2));
        if (I3 == MIE) { ++MII; continue; }
        // I3: LD T,A
        MCPhysReg T = isLdrA(I3->getOpcode());
        if (!T) { ++MII; continue; }
        auto I4 = MBB.SkipPHIsLabelsAndDebug(std::next(I3));
        if (I4 == MIE) { ++MII; continue; }
        // I4: LD A,Y
        MCPhysReg Y = isLdAr(I4->getOpcode());
        if (!Y) { ++MII; continue; }
        auto I5 = MBB.SkipPHIsLabelsAndDebug(std::next(I4));
        if (I5 == MIE) { ++MII; continue; }
        // I5: XOR R2
        MCPhysReg R2 = isXorR(I5->getOpcode());
        if (!R2) { ++MII; continue; }
        auto I6 = MBB.SkipPHIsLabelsAndDebug(std::next(I5));
        if (I6 == MIE) { ++MII; continue; }
        // I6: OR T'
        MCPhysReg ORr = isOrR(I6->getOpcode());
        if (!ORr || ORr != T) { ++MII; continue; }

        // Validate the byte-XOR shape: the loaded pair (X,Y) must form a
        // single GR16; same for the XOR'd pair (R1,R2); the (hi,lo)
        // polarity of (X,R1) must match (i.e., both are hi or both lo);
        // and (Y,R2) must be the opposite polarity.
        MCPhysReg QPair = pairOf(X);
        MCPhysReg PPair = pairOf(R1);
        if (!QPair || !PPair) { ++MII; continue; }
        if (pairOf(Y) != QPair) { ++MII; continue; }
        if (pairOf(R2) != PPair) { ++MII; continue; }
        if (isHiByte(X) != isHiByte(R1)) { ++MII; continue; }
        if (isHiByte(Y) != isHiByte(R2)) { ++MII; continue; }
        if (isHiByte(X) == isHiByte(Y)) { ++MII; continue; }

        // T must not alias either pair, otherwise LD T,A overwrites
        // an operand we still need at I5/I6.
        if (TRI->regsOverlap(T, QPair) || TRI->regsOverlap(T, PPair)) {
          ++MII; continue;
        }

        // Identify which pair is HL (clean case) and which becomes
        // the SBC operand.  SBC operand must be BC or DE.
        MCPhysReg SbcRR;
        bool NeedMoveToHL = false; // true = neither-in-HL path (#117)
        MCPhysReg MoveToHL = 0;   // the pair to PUSH/POP into HL
        if (QPair == Z80::HL && (PPair == Z80::BC || PPair == Z80::DE)) {
          SbcRR = PPair;
        } else if (PPair == Z80::HL && (QPair == Z80::BC || QPair == Z80::DE)) {
          SbcRR = QPair;
        } else if ((QPair == Z80::BC || QPair == Z80::DE) &&
                   (PPair == Z80::BC || PPair == Z80::DE) &&
                   QPair != PPair) {
          // Neither side is HL but both are BC/DE (#117 extension).
          // Move QPair into HL via PUSH/POP (2 B) then SBC HL,PPair (2 B):
          // AND A + PUSH + POP + SBC = 5 B vs 6 B XOR sequence = −1 B.
          // Requires HL to be dead at the compare site (dead-before-I1) so
          // clobbering it with POP HL is safe.
          NeedMoveToHL = true;
          MoveToHL = QPair;
          SbcRR = PPair;
        } else {
          ++MII; continue;
        }

        // I7 must consume only the Z flag (JR Z/NZ or JP Z/NZ).  SBC
        // sets Z correctly for equality but produces different
        // values for C/N/P/V/S/H than the original byte-XOR sequence.
        auto I7 = MBB.SkipPHIsLabelsAndDebug(std::next(I6));
        if (I7 == MIE) { ++MII; continue; }
        unsigned BrOpc = I7->getOpcode();
        if (BrOpc != Z80::JR_Z_e && BrOpc != Z80::JR_NZ_e &&
            BrOpc != Z80::JP_Z_nn && BrOpc != Z80::JP_NZ_nn) {
          ++MII; continue;
        }

        // After the branch, A / T / HL / FLAGS must all be dead so
        // the replacement (which preserves A and T but writes HL+FLAGS)
        // is observably equivalent.
        auto AfterBr = MBB.SkipPHIsLabelsAndDebug(std::next(I7));
        if (!isRegDeadAfter(AfterBr, MBB, TRI, Z80::A)) { ++MII; continue; }
        if (!isRegDeadAfter(AfterBr, MBB, TRI, T)) { ++MII; continue; }
        if (!isRegDeadAfter(AfterBr, MBB, TRI, Z80::HL)) { ++MII; continue; }

        // #117: for the neither-in-HL path, HL must be dead at I1 so the
        // PUSH/POP HL that moves QPair into HL doesn't clobber a live HL
        // value.  Check H and L separately (pair query may return LQR_Unknown
        // when one half is partially determined).  Skip when either half is
        // not known dead.
        if (NeedMoveToHL) {
          auto HQ = MBB.computeRegisterLiveness(TRI, Z80::H, I1);
          auto LQ = MBB.computeRegisterLiveness(TRI, Z80::L, I1);
          if (HQ != MachineBasicBlock::LQR_Dead ||
              LQ != MachineBasicBlock::LQR_Dead) { ++MII; continue; }
        }

        LLVM_DEBUG(dbgs() << "  i16 EQ/NE byte-XOR -> "
                          << (NeedMoveToHL ? "PUSH/POP HL; " : "")
                          << "SBC HL," << TRI->getName(SbcRR) << "\n");

        // Replace I1..I6 with: [PUSH MoveToHL; POP HL;] AND A; SBC HL,rr.
        // AND A clears carry for the SBC; mark its $a read undef since A
        // was left dead by the byte-XOR sequence (#197).
        if (NeedMoveToHL) {
          unsigned PushOpc = Z80::getPushOpcode(MoveToHL);
          BuildMI(MBB, *I1, I1->getDebugLoc(), TII->get(PushOpc));
          BuildMI(MBB, *I1, I1->getDebugLoc(), TII->get(Z80::POP_HL));
        }
        auto AndA = BuildMI(MBB, *I1, I1->getDebugLoc(), TII->get(Z80::AND_A));
        for (MachineOperand &MO : AndA->operands())
          if (MO.isReg() && MO.isUse() && MO.getReg() == Z80::A)
            MO.setIsUndef(true);
        BuildMI(MBB, *I1, I1->getDebugLoc(), TII->get(sbcHLOpc(SbcRR)));

        // Erase I1..I6 (advance MII past the deleted range first).
        MII = std::next(I6);
        I1->eraseFromParent();
        I2->eraseFromParent();
        I3->eraseFromParent();
        I4->eraseFromParent();
        I5->eraseFromParent();
        I6->eraseFromParent();
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
    // Erasing a redundant LD A,r does not update block live-ins.  When the
    // removed LD was the reaching definition of $a for later uses in its block
    // -- i.e. $a flows in from the predecessors (EntryAK[MBB] == Reg(r), which
    // means every predecessor exits with A == r, so $a is live-out of them) --
    // $a becomes live-in to the block and the live-in list must say so, or
    // -verify-machineinstrs reports "Using an undefined physical register" at
    // the first $a reader (e.g. gf_log's ADD_A_A, #194).  For each block, walk
    // to the first event among {erased LD A,r, surviving $a-def}: if the erased
    // LD comes first, no surviving definition provides $a in-block, so $a is
    // live-in -- add it.
    auto defsPhysA = [&](const MachineInstr &MI) {
      for (const MachineOperand &MO : MI.operands())
        if (MO.isReg() && MO.isDef() && MO.getReg().isPhysical() &&
            TRI->regsOverlap(MO.getReg(), Z80::A))
          return true;
      for (MCPhysReg D : MI.getDesc().implicit_defs())
        if (TRI->regsOverlap(D, Z80::A))
          return true;
      return false;
    };
    for (auto &MBB : MF) {
      bool NeedALiveIn = false;
      for (auto &MI : MBB) {
        if (llvm::is_contained(ToErase60x, &MI)) {
          NeedALiveIn = true; // erased LD A,r reached before any surviving A-def
          break;
        }
        if (defsPhysA(MI))
          break; // $a is provided in-block before any erased LD A,r
      }
      if (NeedALiveIn && !MBB.isLiveIn(Z80::A))
        MBB.addLiveIn(Z80::A);
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

        // HL-liveness guard (ravn/llvm-z80#212: AES +static-stack -Os
        // miscompile).  The fold drops the ADD_HL_rr, which ALSO defined HL,
        // keeping the sum only in IX/IY.  That is correct only if HL is dead
        // after the closing IX/IY<-HL copy.  When IX/IY is allocatable
        // (-Os/-Oz + static-stack), the allocator can leave HL live across the
        // copy-add-copy (the same sum is reused by a following `XOR (HL)` /
        // `LD A,(HL)`); folding would then leave that consumer reading a stale
        // HL.  The closing copy READS $hl, so the allocator marks that read
        // `killed` exactly when HL is dead afterwards -- use that precise,
        // local signal (computeRegisterLiveness is coarser: it returns Unknown
        // near block boundaries, which would needlessly refuse safe folds and
        // shift unrelated code).  The $hl use is operand 1 of the closing
        // COPY16_PUSHPOP, or operand 0 of PUSH_HL.
        const MachineOperand &HLUse =
            EndIsPseudo ? I4->getOperand(1) : I4->getOperand(0);
        if (!HLUse.isKill()) {
          ++MII;
          continue;
        }

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

  // --- Peephole: fold (IX/IY+0) access back to (HL) when IX/IY mirrors HL ---
  // ravn/llvm-z80#243 residual.  The `hli` register class lets the allocator
  // place an HL-sized address (`STORE8_IND %addr:hli`) in IX/IY; when HL is
  // also free it ends up holding the SAME address, so codegen materialises the
  // index pointlessly and round-trips through HL:
  //
  //     push hl ; pop iy     ; COPY16_PUSHPOP $iy = $hl   (StartCopy: IY = HL)
  //     ld a,c              ; ...A-only work, HL untouched...
  //     cpl
  //     push iy ; pop hl     ; COPY16_PUSHPOP $hl = $iy   (reverse copy, no-op:
  //                          ;                             HL already == IY)
  //     and (hl)            ; HL still holds buf+idx
  //     ld 0(iy),a          ; -> can be ld (hl),a
  //
  // COPY16_PUSHPOP is opaque to machine-cp, so the round-trip survives to here.
  // When StartCopy `$idx = $hl` is followed -- with HL never re-defined and
  // $idx never re-defined -- only by the reverse copy `$hl = $idx` and/or
  // displacement-0 indexed accesses through $idx, then HL == $idx throughout
  // that window.  Rewrite each indexed access to its (HL) form, drop the
  // reverse copies (pure no-ops: HL already holds the value), and drop the
  // StartCopy materialisation.  The example above collapses to `and (hl);
  // ld (hl),a` -- 4 instructions / 6 bytes removed, no IX/IY traffic.
  if (STI.hasZ80()) {
    for (auto &MBB : MF) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE;) {
        // StartCopy must be `$ix/$iy = $hl`.
        if (MII->getOpcode() != Z80::COPY16_PUSHPOP) { ++MII; continue; }
        Register Dst = MII->getOperand(0).getReg();
        Register Src = MII->getOperand(1).getReg();
        if (Src != Z80::HL || !Z80::IR16RegClass.contains(Dst)) {
          ++MII;
          continue;
        }
        MCPhysReg Idx = Dst;
        auto StartCopy = MII;

        bool Ok = true;
        bool SawRewritable = false;
        SmallVector<MachineBasicBlock::iterator, 4> ReverseCopies;
        // (instruction, new (HL) opcode, carries value immediate)
        SmallVector<std::tuple<MachineBasicBlock::iterator, unsigned, bool>, 4>
            Rewrites;
        MachineBasicBlock::iterator LastTouch = StartCopy;

        for (auto Scan = std::next(StartCopy); Scan != MIE; ++Scan) {
          unsigned SOpc = Scan->getOpcode();

          // The reverse copy `$hl = $idx` is a no-op here (HL already holds
          // $idx's value) and is recognised BEFORE the HL-modification guard
          // below, since it does write HL.  Record it for deletion.
          if (SOpc == Z80::COPY16_PUSHPOP &&
              Scan->getOperand(0).getReg() == Z80::HL &&
              Scan->getOperand(1).getReg() == Idx) {
            ReverseCopies.push_back(Scan);
            LastTouch = Scan;
            continue;
          }

          // A foldable displacement-0 indexed access through $idx.
          MCPhysReg AccIdx = 0;
          bool HasValImm = false;
          unsigned HLOpc =
              z80IndexedZeroDispToHLIndirect(SOpc, AccIdx, HasValImm);
          if (HLOpc && AccIdx == Idx && Scan->getOperand(0).isImm() &&
              Scan->getOperand(0).getImm() == 0) {
            Rewrites.emplace_back(Scan, HLOpc, HasValImm);
            SawRewritable = true;
            LastTouch = Scan;
            continue;
          }

          // Any re-definition of $idx that does NOT also read it closes the
          // window cleanly: the StartCopy value is dead from here, and every
          // use we needed to handle is already behind us.  (If it also reads
          // $idx it is an unhandled use -- fall through to the bail below.)
          bool ReadsIdx = Scan->readsRegister(Idx, TRI);
          bool ModifiesIdx = Scan->modifiesRegister(Idx, TRI);
          if (ModifiesIdx && !ReadsIdx)
            break; // window closed; proceed with what we collected

          // Any other read/modify of $idx is an unhandled use -> give up.
          if (ReadsIdx || ModifiesIdx) { Ok = false; break; }

          // Re-defining HL (full, H, or L) breaks the HL == $idx invariant for
          // any subsequent indexed access -> give up.  (Plain reads of HL,
          // e.g. `and (hl)`, are fine.)
          if (Scan->modifiesRegister(Z80::HL, TRI)) { Ok = false; break; }
        }

        if (!Ok || !SawRewritable) { ++MII; continue; }

        // The rewrite drops the StartCopy that defines $idx, so $idx must be
        // dead after the last instruction we touched (a loop-carried index in
        // IY, live out via a back-edge, would otherwise lose its value).
        if (MBB.computeRegisterLiveness(TRI, Idx, std::next(LastTouch)) !=
            MachineBasicBlock::LQR_Dead) {
          ++MII;
          continue;
        }

        LLVM_DEBUG(dbgs() << "  (IX/IY+0)->(HL) fold: dropping "
                          << TRI->getName(Idx) << " materialisation, "
                          << Rewrites.size() << " indexed access(es) rewritten\n");

        // Rewrite each indexed access to its (HL) form.  The (HL) opcodes carry
        // their HL use / register def as IMPLICIT operands in the MCInstrDesc,
        // so BuildMI needs only the explicit value immediate (for LD (HL),n).
        for (auto &R : Rewrites) {
          MachineBasicBlock::iterator OldIt = std::get<0>(R);
          unsigned NewOpc = std::get<1>(R);
          bool HasValImm = std::get<2>(R);
          DebugLoc DL = OldIt->getDebugLoc();
          auto MIB = BuildMI(MBB, OldIt, DL, TII->get(NewOpc));
          if (HasValImm)
            MIB.addImm(OldIt->getOperand(1).getImm()); // the stored value n
          OldIt->eraseFromParent();
        }
        for (auto &RC : ReverseCopies)
          RC->eraseFromParent();
        MII = MBB.erase(StartCopy);
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
          // Same liveness requirement as Form 2: the rewrite drops the
          // IX/IY <- rr assignment, so IX/IY must be dead after the closing
          // POP rr.  A loop-carried value in IY (live out via a back-edge) is
          // not dead and must not lose its update.  ravn/llvm-z80#112 / #14.
          if (MBB.computeRegisterLiveness(TRI, TP.IXReg,
                                          std::next(std::next(PushIX))) !=
              MachineBasicBlock::LQR_Dead)
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

          // The rewrite drops the IX/IY <- rr write (the start copy), keeping
          // IX/IY at its pre-pattern value.  That is only legal if IX/IY is
          // dead after the closing copy.  A loop-carried value held in IY (live
          // out via a back-edge) is NOT dead here -- dropping its update silently
          // miscompiles the loop.  ravn/llvm-z80#112 / #14.
          if (MBB.computeRegisterLiveness(TRI, IXReg, std::next(EndCopy)) !=
              MachineBasicBlock::LQR_Dead)
            continue;

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

    // #203: shared slot-address predicate (see z80SameBssAddr).
    auto sameAddress = z80SameBssAddr;

    // A +static-stack slot whose base symbol is materialized into a register
    // (e.g. `LD HL, __sfrend_main`) can be accessed INDIRECTLY via pointer
    // arithmetic (array element, &local).  The direct-load orphan scan below
    // only recognizes direct `LD A,(nn)` accesses, so it cannot see such
    // reads -- converting the store+reload to PUSH/POP would drop a store
    // whose slot is still read through the pointer.  Collect every symbol/
    // global that appears in an instruction OTHER than a direct BSS load/store
    // (i.e. used as an immediate address, not a memory operand) and refuse the
    // conversion for those slots.  ravn/llvm-z80#195/test_27: a volatile
    // m[3][3] sum dropped m[0][0]'s store-back because its slot was read only
    // indirectly via `LD HL,__sfrend_main` + offset.
    SmallPtrSet<const void *, 4> AddrTakenSyms;
    z80CollectAddrTakenFrameSyms(MF, AddrTakenSyms);

    for (MachineBasicBlock &MBB : MF) {
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ++MII) {
        const SpillInfo *SI = getSpillInfo(MII->getOpcode());
        if (!SI) continue;
        if (!isSfrendSymbol(MII->getOperand(0))) continue;
        // Skip slots whose frame symbol is address-taken (shared guard).
        if (z80SlotAddrTaken(*MII, AddrTakenSyms))
          continue;

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
        // Track PUSH/POP balance for ALL register pairs (not just ours).
        // PUSH/POP is LIFO: if another register is pushed between our
        // PUSH and POP, our POP would get the wrong value.  We must
        // ensure the stack depth (from all PUSH/POP instructions) is 0
        // at each of our matching loads.
        int StackDepth = 0;

        // #203: shared predicates (single source of truth, no drift).
        auto isAnyPush = z80IsAnyPush;
        auto isAnyPop = z80IsAnyPop;

        // All BSS load opcodes — used to detect orphan loads from the
        // same slot to a register pair other than the spilled one.
        // Issue #82: converting our spill+matching-reload to PUSH/POP
        // while leaving such an orphan read from BSS produces a stale
        // load (the slot is never written since PUSH/POP went to the
        // stack, not the slot).  Bail when seen.
        auto isAnyBssLoad = z80IsAnyBssLoad;

        for (auto Scan = std::next(MII); Scan != MIE; ++Scan) {
          unsigned SOpc = Scan->getOpcode();

          // Another store to the same sfrend slot = conflict (reuse).
          if (SOpc == SI->StoreOpc && sameAddress(*MII, *Scan)) {
            Conflict = true;
            break;
          }

          // Orphan BSS load from the same slot to a different register
          // pair than the one we're spilling.  See #82.
          if (isAnyBssLoad(SOpc) &&
              !isMatchingLoad(SI->StoreOpc, SOpc) &&
              sameAddress(*MII, *Scan)) {
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

          // #203: shared explicit-SP-write guard (see z80IsExplicitSPWrite).
          if (z80IsExplicitSPWrite(*Scan, TRI)) {
            Conflict = true;
            break;
          }

          if (SOpc == Z80::CALL_nn)
            HasCall = true;

          // Matching load from the same address.
          if (isMatchingLoad(SI->StoreOpc, SOpc) && sameAddress(*MII, *Scan)) {
            // Stack must be balanced at each reload point.  Issue #74:
            // the prior `if (!HasCall) continue` skipped pure register-
            // pressure spills (no CALL between store/load) which is exactly
            // the case PUSH/POP wins on too -- the StackDepth check is
            // already the right safety guard.
            if (StackDepth != 0) { Conflict = true; break; }
            (void)HasCall;
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

        // Check that no other basic block references the same BSS address.
        // The PUSH/POP conversion is local to this block — if another block
        // accesses the same slot, it expects the value to be in BSS.
        // ravn/llvm-z80#198: this must match ANY register-class BSS access
        // (isAnyBssLoad/isAnyBssStore), not just SI->LoadOpc/StoreOpc (the
        // spilled pair's class).  rj_sb_inv spilled BC to a slot in bb.0 and
        // read it back as DE (`LD_DE_nnind`) in bb.1 — a cross-class reload
        // the old same-class-only check did not see, so the store→PUSH
        // conversion orphaned bb.1's read of a slot that PUSH never writes.
        // (The in-block orphan guard above is already class-agnostic; the
        // cross-block guard must be too — matching the sibling peephole.)
        // #203: shared orphan guard.  Single-block: skip the store block --
        // same-block conflicts are caught by the forward scan above.
        if (z80SlotUsedElsewhere(MF, *MII, {&MBB}, {}, nullptr, nullptr))
          continue;

        // Also bail if the slot is accessed (any register class) earlier in
        // THIS block, BEFORE the matched store.  The forward orphan scan only
        // covers accesses after the store; a read before it is the signature
        // of a loop-carried value whose home is this slot -- the slot is read
        // at the loop top (via the back-edge) and written at the bottom.
        // Converting the bottom store+reload to PUSH/POP drops the store, so
        // the top read sees a stale slot every iteration.  ravn/llvm-z80
        // #195/test_166: a popcount i32 loop hung because `LD (slot),BC` was
        // converted to PUSH/POP while the loop top still read it via
        // `LD HL,(slot)`.
        // Shared loop-carried guard (z80SlotReadBeforeStoreInBlock) -- the same
        // check the cross-block peephole uses, so the two can't drift (#203).
        if (z80SlotReadBeforeStoreInBlock(MBB, MII))
          continue;

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

        // Replace store with PUSH.  Anchor resumption to the inserted PUSH
        // (which is never erased) rather than decrementing the post-erase
        // iterator: if the matching load is the instruction immediately after
        // the store, erasing the store then the load would leave MII dangling
        // and `--MII` would dereference freed memory (ravn/llvm-z80#193).
        DebugLoc DL = MII->getDebugLoc();
        MachineInstr *PushMI = BuildMI(MBB, *MII, DL, TII->get(SI->PushOpc));
        auto StoreIt = MII;

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
        // Erase the store last; StoreIt stays valid through the load erasures.
        MBB.erase(StoreIt);

        Changed = true;
        // Resume scanning from the inserted PUSH; the outer loop's ++MII
        // advances past it.  Always a valid iterator.
        MII = PushMI->getIterator();
      }
    }
  }

  // --- Peephole: BSS spill cross-class transfer → PUSH/POP ---
  // Handles the case where the value is stored to a BSS slot and later
  // loaded into a DIFFERENT register pair (transfer via memory).
  // Pattern:
  //   LD (slot), rr_src       (3-4B)
  //   ... CALL ...
  //   LD rr_dst, (slot)        (3-4B)        ; rr_dst != rr_src
  // Replaces with:
  //   PUSH rr_src              (1-2B)
  //   ... CALL ...
  //   POP rr_dst               (1-2B)
  //
  // Same safety guards as the same-class peephole above:
  //   - sfrend/sframe symbols only (rules out global accesses)
  //   - Slot must not be re-stored or re-loaded between the pair
  //   - Stack must be balanced at the load point
  //   - Slot must not be used in any other basic block
  //   - Single-load only (multi-load cross-class is too complex)
  //
  // Savings per pair (16-bit):
  //   HL→DE/BC: 3+4 = 7B → 2B (save 5)
  //   DE→HL/BC, BC→HL/DE: 4+3-or-4 = 7-8B → 2B (save 5-6)
  //   HL→HL/DE→DE/BC→BC handled by the same-class peephole above.
  if (STI.staticStack()) {
    struct StoreClass { unsigned StoreOpc, PushOpc, Bytes; };
    struct LoadClass  { unsigned LoadOpc,  PopOpc,  Bytes; };
    static const StoreClass Stores[] = {
      {Z80::LD_nnind_HL, Z80::PUSH_HL, 3},
      {Z80::LD_nnind_DE, Z80::PUSH_DE, 4},
      {Z80::LD_nnind_BC, Z80::PUSH_BC, 4},
    };
    static const LoadClass Loads[] = {
      {Z80::LD_HL_nnind, Z80::POP_HL, 3},
      {Z80::LD_DE_nnind, Z80::POP_DE, 4},
      {Z80::LD_BC_nnind, Z80::POP_BC, 4},
    };

    auto getStoreInfo = [&](unsigned Opc) -> const StoreClass * {
      for (const auto &S : Stores)
        if (S.StoreOpc == Opc) return &S;
      return nullptr;
    };
    auto getLoadInfo = [&](unsigned Opc) -> const LoadClass * {
      for (const auto &L : Loads)
        if (L.LoadOpc == Opc) return &L;
      return nullptr;
    };

    // #203: shared predicates (single source of truth, no drift).
    auto isAnyBssLoad = z80IsAnyBssLoad;
    auto isAnyBssStore = z80IsAnyBssStore;
    auto isAnyPush = z80IsAnyPush;
    auto isAnyPop = z80IsAnyPop;

    auto isSfrendSymbol = [](const MachineOperand &MO) -> bool {
      if (!MO.isMCSymbol()) return false;
      StringRef Name = MO.getMCSymbol()->getName();
      return Name.starts_with("__sfrend") || Name.starts_with("__sframe");
    };

    // #203: shared slot-address predicate (see z80SameBssAddr).
    auto sameAddress = z80SameBssAddr;

    SmallPtrSet<const void *, 4> AddrTakenSyms;
    z80CollectAddrTakenFrameSyms(MF, AddrTakenSyms);
    for (MachineBasicBlock &MBB : MF) {
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ++MII) {
        const StoreClass *SC = getStoreInfo(MII->getOpcode());
        if (!SC) continue;
        if (!isSfrendSymbol(MII->getOperand(0))) continue;
        // Address-taken guard, shared so it can't drift (#195/#204): a slot read
        // indirectly via a pointer (its `&` is taken) must keep its memory store.
        if (z80SlotAddrTaken(*MII, AddrTakenSyms))
          continue;
        // Loop-carried guard, shared with the other spill->PUSH/POP peepholes
        // so it can't drift (#202/#203): bail if the slot is read before this
        // store in the block (back-edge reload signature -- dropping the store
        // would leave the loop top reading a stale slot).
        if (z80SlotReadBeforeStoreInBlock(MBB, MII))
          continue;

        // Scan forward for exactly ONE load from the same slot, with
        // no other accesses to the slot, and stack balanced.
        int StackDepth = 0;
        bool Conflict = false;
        MachineBasicBlock::iterator MatchedLoad = MIE;
        const LoadClass *LC = nullptr;

        for (auto Scan = std::next(MII); Scan != MIE; ++Scan) {
          unsigned SOpc = Scan->getOpcode();

          // Another store to same slot = conflict.
          if (isAnyBssStore(SOpc) && sameAddress(*MII, *Scan)) {
            Conflict = true; break;
          }

          // Track PUSH/POP balance.
          if (isAnyPush(SOpc)) ++StackDepth;
          if (isAnyPop(SOpc)) {
            --StackDepth;
            if (StackDepth < 0) { Conflict = true; break; }
          }

          // #203: shared explicit-SP-write guard (see z80IsExplicitSPWrite).
          if (z80IsExplicitSPWrite(*Scan, TRI)) {
            Conflict = true; break;
          }

          // Load from our slot?
          if (isAnyBssLoad(SOpc) && sameAddress(*MII, *Scan)) {
            // Already saw one load — bail (single-load only).
            if (MatchedLoad != MIE) { Conflict = true; break; }
            // Must be a 16-bit load (we only generate 16-bit pushes/pops).
            const LoadClass *LCi = getLoadInfo(SOpc);
            if (!LCi) { Conflict = true; break; }
            // Stack must be balanced at the load point.
            if (StackDepth != 0) { Conflict = true; break; }
            // Skip same-class case (handled by the peephole above).
            // Tracking by Push/Pop opcode pair: if PushOpc maps to PopOpc
            // for the same register, it's same-class.
            bool sameClass =
                (SC->PushOpc == Z80::PUSH_HL && LCi->PopOpc == Z80::POP_HL) ||
                (SC->PushOpc == Z80::PUSH_DE && LCi->PopOpc == Z80::POP_DE) ||
                (SC->PushOpc == Z80::PUSH_BC && LCi->PopOpc == Z80::POP_BC);
            if (sameClass) { Conflict = true; break; }
            MatchedLoad = Scan;
            LC = LCi;
          }

          if (Scan->isTerminator()) break;
        }

        if (Conflict || MatchedLoad == MIE || LC == nullptr) continue;

        // Cost check: PUSH(1-2B) + POP(1-2B) must be < store + load bytes.
        // PUSH/POP for HL/DE/BC are all 1B each; total 2B.
        // Store + load is 3+3, 3+4, 4+3, or 4+4 = 6/7/8 B.
        unsigned PushPopBytes = 2;  // BC/DE/HL all use 1B push and 1B pop
        unsigned BssBytes = SC->Bytes + LC->Bytes;
        if (PushPopBytes >= BssBytes) continue;

        // #203: shared orphan guard (skip the store block).
        if (z80SlotUsedElsewhere(MF, *MII, {&MBB}, {}, nullptr, nullptr))
          continue;

        LLVM_DEBUG(dbgs() << "  BSS spill cross-class→PUSH/POP: "
                          << *MII << "  → "
                          << LC->LoadOpc << " (saves "
                          << (BssBytes - PushPopBytes) << "B)\n");

        // Replace the store with PUSH.
        DebugLoc DLs = MII->getDebugLoc();
        BuildMI(MBB, *MII, DLs, TII->get(SC->PushOpc));
        // Replace the load with POP.
        DebugLoc DLl = MatchedLoad->getDebugLoc();
        BuildMI(MBB, *MatchedLoad, DLl, TII->get(LC->PopOpc));
        MBB.erase(MatchedLoad);
        MII = MBB.erase(MII);
        --MII;
        Changed = true;
      }
    }
  }

  // --- Peephole: Cross-MBB BSS-spill → PUSH/POP ---
  // Extends the in-MBB BSS-spill peepholes above to the case where the
  // matching LOAD lives in a successor MBB.  See ravn/llvm-z80#132.
  //
  // Pattern (from cpnos-rom SNIOS retry loops):
  //   MBB_A:  STORE rr,(slot); ...; CALL; ...; cond-br MBB_C; fallthrough
  //   MBB_B:  LOAD  rr,(slot); ...; back-edge or fallthrough
  //   MBB_C:  escape target with slot dead
  //
  // For each non-LOAD ("escape") successor of MBB_A we choose one of:
  //   - Prepend "inc sp; inc sp" in-place at MBB_C's head (cheapest)
  //     when MBB_A is MBB_C's sole predecessor — running the compensation
  //     only on this path.
  //   - Otherwise edge-split: insert a fresh compensation MBB just before
  //     MBB_C in layout, fall through into MBB_C, and rewrite MBB_A's
  //     explicit terminator operand from MBB_C to the new MBB.  Safe only
  //     when (a) MBB_A's terminator references MBB_C explicitly (no
  //     fall-through escape from MBB_A), and (b) MBB_C's layout-predecessor
  //     doesn't already fall through to MBB_C (would corrupt that path).
  // Both strategies have a 2 B compensation cost per escape edge.  The
  // slot must not be referenced anywhere outside MBB_A's STORE and
  // MBB_B's LOAD.
  if (STI.staticStack()) {
    struct CrossSpillInfo {
      unsigned StoreOpc, LoadOpc, PushOpc, PopOpc;
      unsigned StoreBytes, LoadBytes;
    };
    static const CrossSpillInfo Pairs[] = {
        {Z80::LD_nnind_A,  Z80::LD_A_nnind,  Z80::PUSH_AF, Z80::POP_AF, 3, 3},
        {Z80::LD_nnind_HL, Z80::LD_HL_nnind, Z80::PUSH_HL, Z80::POP_HL, 3, 3},
        {Z80::LD_nnind_DE, Z80::LD_DE_nnind, Z80::PUSH_DE, Z80::POP_DE, 4, 4},
        {Z80::LD_nnind_BC, Z80::LD_BC_nnind, Z80::PUSH_BC, Z80::POP_BC, 4, 4},
    };
    auto getCSI = [&](unsigned Opc) -> const CrossSpillInfo * {
      for (const auto &P : Pairs)
        if (P.StoreOpc == Opc)
          return &P;
      return nullptr;
    };
    auto isSfr = [](const MachineOperand &MO) {
      if (!MO.isMCSymbol())
        return false;
      StringRef N = MO.getMCSymbol()->getName();
      return N.starts_with("__sfrend") || N.starts_with("__sframe");
    };
    // #203: shared slot-address predicate (see z80SameBssAddr).
    auto sameAddr = z80SameBssAddr;
    // #203: shared predicates (single source of truth, no drift).
    auto isAnyBssLoad = z80IsAnyBssLoad;
    auto isAnyBssStore = z80IsAnyBssStore;
    auto isAnyPush = z80IsAnyPush;
    auto isAnyPop = z80IsAnyPop;

    // Per ravn/llvm-z80#143: track the NewMBBs we create via edge-split
    // compensation so subsequent fires can recognize them and bypass the
    // `Prev->canFallThrough() && Prev->isLayoutSuccessor(Succ)` bail
    // check.  The check was meant to protect UNRELATED fall-throughs into
    // MBB_C; an MBB we created ourselves earlier this iteration is not
    // an unrelated path and must not block us.
    SmallPtrSet<MachineBasicBlock *, 4> OurNewMBBs;
    SmallPtrSet<const void *, 4> AddrTakenSyms;
    z80CollectAddrTakenFrameSyms(MF, AddrTakenSyms);
    // Per ravn/llvm-z80#155: relax the `UsedElsewhere` gate to allow
    // external slot accesses that DOMINATE MBB_A (their stores/loads
    // execute strictly before MBB_A's STORE, so MBB_A's rewrite leaves
    // their values undisturbed).  Recompute the dominator tree each
    // outer iteration (after potential CFG mutations from earlier fires).
    std::unique_ptr<MachineDominatorTree> MDT;
    auto refreshMDT = [&]() {
      MDT = std::make_unique<MachineDominatorTree>(MF);
    };
    for (MachineBasicBlock &MBB_A : MF) {
      bool RestartOuter = true;
      while (RestartOuter) {
        RestartOuter = false;
        refreshMDT();
        for (auto MII = MBB_A.begin(), MIE = MBB_A.end(); MII != MIE; ++MII) {
          const CrossSpillInfo *CSI = getCSI(MII->getOpcode());
          if (!CSI)
            continue;
          if (!isSfr(MII->getOperand(0)))
            continue;

          // Loop-carried guard: if the slot is accessed earlier in MBB_A,
          // BEFORE this store, the store is the bottom of a loop-carried value
          // whose home is this slot (read at the loop top via the back-edge,
          // written here).  Converting this store + the successor-block reload
          // to PUSH/POP drops the memory store, so the back-edge read sees a
          // stale slot every iteration (#202: `do { v >>= 1; } while (v > 0)`
          // never updated v at -O0 +static-stack).  Shared with the
          // single-block peephole via z80SlotReadBeforeStoreInBlock so the two
          // can't drift (#203) -- this guard living in one but not the other
          // was exactly the #202 bug.  The forward scan below only covers
          // accesses AFTER the store, so this backward check is needed too.
          if (z80SlotReadBeforeStoreInBlock(MBB_A, MII))
            continue;
          // Address-taken guard (shared, #195/#204): a slot read indirectly via
          // a pointer must keep its memory store -- don't convert to PUSH/POP.
          if (z80SlotAddrTaken(*MII, AddrTakenSyms))
            continue;

          // Scan forward in MBB_A.  Must NOT find any other access to the
          // same slot (in-MBB matches are handled by the prior peepholes).
          // Track PUSH/POP balance; require StackDepth == 0 at terminator.
          int StackDepth = 0;
          bool BailLocal = false;
          for (auto S = std::next(MII); S != MIE; ++S) {
            unsigned O = S->getOpcode();
            if (isAnyPush(O))
              ++StackDepth;
            if (isAnyPop(O)) {
              --StackDepth;
              if (StackDepth < 0) {
                BailLocal = true;
                break;
              }
            }
            if ((isAnyBssLoad(O) || isAnyBssStore(O)) &&
                sameAddr(*MII, *S)) {
              BailLocal = true;
              break;
            }
            if (S->isTerminator())
              break;
          }
          if (BailLocal)
            continue;
          if (StackDepth != 0)
            continue;

          // Inspect successors.  Exactly one MBB_B must contain the
          // matching LOAD as its first slot-touching instruction with
          // balanced stack from entry to LOAD.  All other successors
          // are "escape MBBs"; for each, we either:
          //   - prepend "inc sp; inc sp" in-place when MBB_A is sole
          //     predecessor of the escape (cheapest: 2 B compensation)
          //   - or edge-split: insert a new compensation MBB just
          //     before the escape so its fall-through carries SP to
          //     the original escape target (also 2 B compensation,
          //     but requires the escape's layout-predecessor not to
          //     already fall-through to it; bail otherwise to keep
          //     cost predictable)
          // The escape MBB itself must not reference the slot.
          enum EscapeKind { ESC_PrependInPlace, ESC_InsertBefore };
          struct EscapeRec {
            MachineBasicBlock *MBB_C;
            EscapeKind Kind;
          };
          MachineBasicBlock *MBB_B = nullptr;
          MachineBasicBlock::iterator LoadIt;
          SmallVector<EscapeRec, 2> Escapes;
          bool BailSucc = false;
          for (MachineBasicBlock *Succ : MBB_A.successors()) {
            int SuccDepth = 0;
            bool SuccTouches = false;
            MachineBasicBlock::iterator FirstTouch = Succ->end();
            for (auto T = Succ->begin(); T != Succ->end(); ++T) {
              unsigned O = T->getOpcode();
              if (isAnyPush(O))
                ++SuccDepth;
              if (isAnyPop(O))
                --SuccDepth;
              if ((isAnyBssLoad(O) || isAnyBssStore(O)) &&
                  sameAddr(*MII, *T)) {
                SuccTouches = true;
                FirstTouch = T;
                break;
              }
            }
            if (!SuccTouches) {
              // Escape candidate.  Decide compensation strategy:
              if (Succ->pred_size() == 1 &&
                  *Succ->pred_begin() == &MBB_A) {
                // Cheap path: prepend in-place.
                Escapes.push_back({Succ, ESC_PrependInPlace});
                continue;
              }
              // Edge-split.  Only safe when MBB_A's terminator has an
              // explicit MBB operand referencing Succ (i.e. Succ is
              // reached by an explicit branch, not fall-through from
              // MBB_A) AND inserting a new MBB just before Succ in
              // layout won't break some OTHER MBB's fall-through into
              // Succ.
              bool HasExplicitEdge = false;
              for (auto Ti = MBB_A.getFirstTerminator();
                   Ti != MBB_A.end(); ++Ti) {
                for (const MachineOperand &MO : Ti->operands()) {
                  if (MO.isMBB() && MO.getMBB() == Succ) {
                    HasExplicitEdge = true;
                    break;
                  }
                }
                if (HasExplicitEdge)
                  break;
              }
              if (!HasExplicitEdge) {
                BailSucc = true;
                break;
              }
              MachineBasicBlock *Prev = Succ->getPrevNode();
              if (Prev && Prev->canFallThrough() &&
                  Prev->isLayoutSuccessor(Succ) &&
                  !OurNewMBBs.contains(Prev)) {
                // Some unrelated MBB falls-through to Succ; we can't
                // insert before Succ without breaking that path.  If
                // Prev is a peer-created NewMBB (peer 1's edge-split
                // compensation), allow the bail to be skipped -- peer 2
                // can chain its own NewMBB before Prev without
                // conflicting with the existing fall-through, because
                // peer 1's NewMBB came from THIS function's earlier
                // iteration and we own the layout (#143).
                BailSucc = true;
                break;
              }
              Escapes.push_back({Succ, ESC_InsertBefore});
              continue;
            }
            // Slot touched in Succ.  Must be the matching LOAD opcode.
            if (FirstTouch->getOpcode() != CSI->LoadOpc) {
              BailSucc = true;
              break;
            }
            if (SuccDepth != 0) {
              BailSucc = true;
              break;
            }
            if (MBB_B != nullptr) {
              // Two successors both contain a LOAD — bail.
              BailSucc = true;
              break;
            }
            MBB_B = Succ;
            LoadIt = FirstTouch;
          }
          LLVM_DEBUG(dbgs() << "[#132] MF=" << MF.getName()
                            << " MBB_A=BB#" << MBB_A.getNumber()
                            << " STORE=" << TII->getName(MII->getOpcode())
                            << " BailSucc=" << BailSucc
                            << " MBB_B="
                            << (MBB_B ? (int)MBB_B->getNumber() : -1)
                            << " Escapes=" << Escapes.size() << "\n");
          if (BailSucc || !MBB_B)
            continue;

          // ravn/llvm-z80#156: MBB_B must be reached only from MBB_A.
          // If MBB_B has other predecessors (typically a loop back-edge),
          // those paths enter MBB_B without having executed the STORE
          // (now PUSH) — the LOAD-rewritten-as-POP fires without a
          // matching PUSH, leaking 2 B off SP per back-edge traversal.
          // Caught originally in aes256.c gf_log under +static-stack:
          // entry block stored the K&R-promoted param, loop header
          // loaded it for the `atb != x` test, and the loop back-edge
          // re-entered the loop header from the loop body.  The peephole
          // happily rewrote STORE→PUSH at entry and LOAD→POP at loop
          // header; SP grew by 2 B per iteration until wrap, then a RET
          // popped 0x0000 as the return address and execution escaped.
          if (MBB_B->pred_size() != 1 ||
              *MBB_B->pred_begin() != &MBB_A)
            continue;

          // External slot accesses must either (a) not exist, or (b) be
          // in MBBs that DOMINATE MBB_A -- in which case their accesses
          // belong to a different lifetime (slot-coalesced by regalloc)
          // and execute before MBB_A's STORE, leaving the slot
          // undisturbed by our rewrite (#155).
          // #203: shared orphan guard with the #155 dominator relaxation --
          // skip MBB_A + MBB_B; allow accesses in blocks that dominate MBB_A
          // (a strictly-earlier slot-coalesced lifetime).
          if (z80SlotUsedElsewhere(MF, *MII, {&MBB_A, MBB_B}, {}, MDT.get(),
                                   &MBB_A))
            continue;

          // POP AF: FLAGS must be dead after the LOAD position.
          if (CSI->PopOpc == Z80::POP_AF) {
            auto After = std::next(LoadIt);
            if (!isRegDeadAfter(After, *MBB_B, TRI, Z80::FLAGS))
              continue;
          }

          // Cost gate.  PUSH/POP are 1 B each.  Compensation is normally
          // 2 B (inc sp; inc sp) per escape edge.  Per ravn/llvm-z80#138:
          // if a register class is dead at the escape MBB's live-in set,
          // emit `POP rr` (1 B) instead, saving 1 B per such escape.
          // Order: AF first (lowest impact if wrong), then HL/DE/BC.
          //
          // Probe each escape's MBB_C for a fully-dead pair.  An MCPhysReg
          // pair is "fully dead" iff none of its sub-registers are in
          // MBB_C's live-in set.
          SmallVector<unsigned, 4> EscPopOpc(Escapes.size(), 0u);
          unsigned CompCost = 0;
          for (size_t i = 0; i < Escapes.size(); ++i) {
            MachineBasicBlock *MBB_C = Escapes[i].MBB_C;
            struct { unsigned Op; MCPhysReg Hi, Lo; } Pairs[] = {
              {Z80::POP_AF, Z80::A, Z80::FLAGS},
              {Z80::POP_HL, Z80::H, Z80::L},
              {Z80::POP_DE, Z80::D, Z80::E},
              {Z80::POP_BC, Z80::B, Z80::C},
            };
            for (const auto &P : Pairs) {
              if (!MBB_C->isLiveIn(P.Hi) && !MBB_C->isLiveIn(P.Lo)) {
                EscPopOpc[i] = P.Op;
                break;
              }
            }
            CompCost += EscPopOpc[i] ? 1 : 2;
          }
          unsigned PushPopSave =
              (CSI->StoreBytes - 1) + (CSI->LoadBytes - 1);
          if (PushPopSave <= CompCost)
            continue;

          LLVM_DEBUG({
            dbgs() << "  Cross-MBB BSS spill→PUSH/POP: " << *MII
                   << "  load in BB#" << MBB_B->getNumber() << ", "
                   << Escapes.size() << " escape MBB(s), saves "
                   << (PushPopSave - CompCost) << "B\n";
          });

          // Rewrite MBB_A: STORE → PUSH.
          DebugLoc DLs = MII->getDebugLoc();
          BuildMI(MBB_A, *MII, DLs, TII->get(CSI->PushOpc));
          MBB_A.erase(MII);

          // Rewrite MBB_B: LOAD → POP.
          DebugLoc DLl = LoadIt->getDebugLoc();
          BuildMI(*MBB_B, *LoadIt, DLl, TII->get(CSI->PopOpc));
          MBB_B->erase(LoadIt);

          // Compensate each escape.  Uses POP rr (1 B) when a register
          // pair is dead at the escape (per #138 liveness probe above);
          // falls back to INC SP; INC SP (2 B) otherwise.
          for (size_t i = 0; i < Escapes.size(); ++i) {
            auto &E = Escapes[i];
            MachineBasicBlock *MBB_C = E.MBB_C;
            DebugLoc DLc;
            unsigned PopOpc = EscPopOpc[i];
            auto emitComp = [&](MachineBasicBlock *MBB,
                                MachineBasicBlock::iterator It) {
              if (PopOpc) {
                BuildMI(*MBB, It, DLc, TII->get(PopOpc));
              } else {
                BuildMI(*MBB, It, DLc, TII->get(Z80::INC_SP));
                BuildMI(*MBB, It, DLc, TII->get(Z80::INC_SP));
              }
            };
            if (E.Kind == ESC_PrependInPlace) {
              // Prepend in-place — MBB_A is MBB_C's sole predecessor so
              // this only runs on this path.
              emitComp(MBB_C, MBB_C->begin());
            } else {
              // Edge-split: create a new MBB just before MBB_C in
              // layout, fall through to MBB_C.  Rewrite MBB_A's
              // terminator operand from MBB_C to NewMBB.
              MachineBasicBlock *NewMBB = MF.CreateMachineBasicBlock();
              MF.insert(MBB_C->getIterator(), NewMBB);
              emitComp(NewMBB, NewMBB->end());
              NewMBB->addSuccessor(MBB_C);
              MBB_A.ReplaceUsesOfBlockWith(MBB_C, NewMBB);
              OurNewMBBs.insert(NewMBB); // ravn/llvm-z80#143
              // After this fire, NewMBB sits between MBB_C's old layout-
              // predecessor and MBB_C, falling-through into MBB_C.  If a
              // future peer also escapes to MBB_C, the bail check above
              // would normally block it (because Prev=NewMBB now satisfies
              // canFallThrough + isLayoutSuccessor).  The OurNewMBBs set
              // tells that check to ignore peer-created NewMBBs.
            }
          }

          Changed = true;
          RestartOuter = true;
          break;
        }
      }
    }
  }

  // --- Peephole: bare BSS store + 4-instr A-preserving reload → PUSH/POP rr ---
  // ravn/llvm-z80#173.
  //
  // Pattern (common in AES `aes_subBytes`, `aes_sb_inv`, `aes_mixColumns`,
  // `aes_mc_inv` at -Oz +static-stack):
  //
  //   ;; bare store (A already holds the value to spill):
  //   LD   (slot), A             ; 3 B
  //   ... [intermediate code, may include CALL, partner-half writes, etc.] ...
  //   ;; 4-instruction reload that preserves the current A:
  //   PUSH AF                    ; 1 B
  //   LD   A, (slot)             ; 3 B
  //   LD   R, A                  ; 1 B
  //   POP  AF                    ; 1 B
  //
  // Total: 9 B per spill+reload pair.
  //
  // Convert to (3 B per pair, saving 6 B):
  //
  //   LD   R, A                  ; 1 B  (copy A into R at the store point)
  //   PUSH rr                    ; 1 B  (where rr is the pair containing R)
  //   ... [intermediate code unchanged] ...
  //   POP  rr                    ; 1 B  (restores R from stack; partner-half restored too)
  //
  // Safety:
  //   - Slot accessed only by this bare-store and the matching reload-template.
  //   - Stack balance preserved at the matched reload site (excluding the
  //     matched reload's own PUSH AF / POP AF, which are part of the template).
  //   - The partner half of `rr` is not READ after the converted POP rr
  //     unless its value at that point is irrelevant (we restore it from
  //     stack, possibly losing intermediate modifications).  Conservative:
  //     bail if the partner half is defined anywhere between store and
  //     reload.  (Across-CALL case is covered by the same rule -- the CALL
  //     conventionally clobbers the partner, but our PUSH/POP keeps the
  //     pre-spill value intact, which is fine as long as no caller-saved
  //     read pattern relies on the post-CALL clobbered value.)
  if (STI.staticStack()) {
    struct StoreReload173 {
      unsigned PairReg;   // e.g. Z80::BC
      unsigned Reg8;      // e.g. Z80::C
      unsigned Partner8;  // e.g. Z80::B
      unsigned LdRegA;    // e.g. Z80::LD_C_A   (R := A)
      unsigned PushOpc;   // e.g. Z80::PUSH_BC
      unsigned PopOpc;    // e.g. Z80::POP_BC
    };
    static const StoreReload173 Variants[] = {
      {Z80::BC, Z80::B, Z80::C, Z80::LD_B_A, Z80::PUSH_BC, Z80::POP_BC},
      {Z80::BC, Z80::C, Z80::B, Z80::LD_C_A, Z80::PUSH_BC, Z80::POP_BC},
      {Z80::DE, Z80::D, Z80::E, Z80::LD_D_A, Z80::PUSH_DE, Z80::POP_DE},
      {Z80::DE, Z80::E, Z80::D, Z80::LD_E_A, Z80::PUSH_DE, Z80::POP_DE},
      {Z80::HL, Z80::H, Z80::L, Z80::LD_H_A, Z80::PUSH_HL, Z80::POP_HL},
      {Z80::HL, Z80::L, Z80::H, Z80::LD_L_A, Z80::PUSH_HL, Z80::POP_HL},
    };
    auto findByLdRegA = [&](unsigned Opc) -> const StoreReload173 * {
      for (const auto &V : Variants)
        if (V.LdRegA == Opc) return &V;
      return nullptr;
    };
    auto isSfr = [](const MachineOperand &MO) {
      if (!MO.isMCSymbol()) return false;
      StringRef N = MO.getMCSymbol()->getName();
      return N.starts_with("__sfrend") || N.starts_with("__sframe");
    };
    // #203: shared predicates (single source of truth, no drift).  The old
    // local isAnyBssAccess had an isStore out-param that no call site used.
    auto sameAddr = z80SameBssAddr;
    auto isAnyBssAccess = z80IsAnyBssAccess;

    SmallPtrSet<const void *, 4> AddrTakenSyms;
    z80CollectAddrTakenFrameSyms(MF, AddrTakenSyms);
    for (MachineBasicBlock &MBB : MF) {
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
        // Match bare store: LD_nnind_A on sfrend.  Skip if preceded by
        // an LD_A_<reg> (then it's a 4-instr spill template, handled by
        // the in-MBB peephole above when present).
        if (MII->getOpcode() != Z80::LD_nnind_A) { ++MII; continue; }
        if (!isSfr(MII->getOperand(0))) { ++MII; continue; }
        // Loop-carried guard, shared across the spill->PUSH/POP peepholes so it
        // can't drift (#202/#203): bail if the slot is read before this store
        // in the block (back-edge reload signature).  (Manual-increment loop:
        // must advance MII on the bail.)
        if (z80SlotReadBeforeStoreInBlock(MBB, MII)) { ++MII; continue; }
        // Address-taken guard (shared, #195/#204): slot read indirectly via a
        // pointer must keep its memory store.
        if (z80SlotAddrTaken(*MII, AddrTakenSyms)) { ++MII; continue; }
        if (MII != MBB.begin()) {
          auto Prev = std::prev(MII);
          unsigned PO = Prev->getOpcode();
          if (PO == Z80::LD_A_B || PO == Z80::LD_A_C || PO == Z80::LD_A_D ||
              PO == Z80::LD_A_E || PO == Z80::LD_A_H || PO == Z80::LD_A_L) {
            ++MII; continue;
          }
        }
        auto Store = MII;

        // Scan forward (within MBB only, MVP) for the matched 4-instr
        // reload template: PUSH_AF; LD_A_nnind same-slot; LD_r_A; POP_AF.
        // Track stack balance excluding this template's PUSH_AF/POP_AF.
        int StackDepth = 0;
        bool Bail = false;
        // Track which partner halves get defined; we only need to bail if
        // the *winner* variant's partner is defined.  Easier: collect a
        // set of defined 8-bit regs in the interval, then exclude variants
        // whose partner appears.
        SmallSet<unsigned, 8> DefinedRegs;
        // Track 8-bit registers READ in the interval.  The transform inserts
        // `LD R,A` at the store site, so R holds the spilled value throughout
        // the bracketed region; if the region READS R it would observe the
        // spilled value instead of R's original contents -> miscompile
        // (ravn/llvm-z80#192: the second XOR_CMP_EQ16 reads D as its zero
        // input, but #173 had put the first compare's result in D).
        SmallSet<unsigned, 8> ReadRegs;
        bool SeenCall = false;
        MachineBasicBlock::iterator R0 = MIE, R1 = MIE, R2 = MIE, R3 = MIE;
        const StoreReload173 *V = nullptr;

        auto isPush = [](unsigned O) {
          return O == Z80::PUSH_AF || O == Z80::PUSH_BC ||
                 O == Z80::PUSH_DE || O == Z80::PUSH_HL ||
                 O == Z80::PUSH_IX || O == Z80::PUSH_IY;
        };
        auto isPop = [](unsigned O) {
          return O == Z80::POP_AF || O == Z80::POP_BC ||
                 O == Z80::POP_DE || O == Z80::POP_HL ||
                 O == Z80::POP_IX || O == Z80::POP_IY;
        };

        auto recordDefs = [&](const MachineInstr &MI) {
          // Skip CALL clobbers (regmask + implicit defs).  Caller is not
          // permitted to rely on caller-saved values across a CALL anyway,
          // so our PUSH/POP restoring those values is semantically OK.
          if (MI.isCall()) return;
          for (const MachineOperand &MO : MI.operands()) {
            // Only count EXPLICIT defs.  Implicit defs are typically
            // ABI clobbers or call-result side effects that aren't
            // architectural value definitions.
            if (MO.isReg() && MO.isDef() && !MO.isImplicit()) {
              unsigned R = MO.getReg();
              if (R == Z80::A || R == Z80::B || R == Z80::C || R == Z80::D ||
                  R == Z80::E || R == Z80::H || R == Z80::L)
                DefinedRegs.insert(R);
              else if (R == Z80::BC) {
                DefinedRegs.insert(Z80::B); DefinedRegs.insert(Z80::C);
              } else if (R == Z80::DE) {
                DefinedRegs.insert(Z80::D); DefinedRegs.insert(Z80::E);
              } else if (R == Z80::HL) {
                DefinedRegs.insert(Z80::H); DefinedRegs.insert(Z80::L);
              }
            }
          }
        };

        auto recordReads = [&](const MachineInstr &MI) {
          // CALL reads are ABI clobbers, not architectural reads of our value.
          if (MI.isCall()) return;
          for (const MachineOperand &MO : MI.operands()) {
            if (!MO.isReg() || !MO.readsReg() || !MO.getReg().isPhysical())
              continue;
            unsigned R = MO.getReg();
            if (R == Z80::A || R == Z80::B || R == Z80::C || R == Z80::D ||
                R == Z80::E || R == Z80::H || R == Z80::L)
              ReadRegs.insert(R);
            else if (R == Z80::BC) {
              ReadRegs.insert(Z80::B); ReadRegs.insert(Z80::C);
            } else if (R == Z80::DE) {
              ReadRegs.insert(Z80::D); ReadRegs.insert(Z80::E);
            } else if (R == Z80::HL) {
              ReadRegs.insert(Z80::H); ReadRegs.insert(Z80::L);
            }
          }
        };

        // Phase 1: scan from Store to the matched 4-instr reload template.
        for (auto S = std::next(Store); S != MIE; ++S) {
          unsigned Op = S->getOpcode();

          if (isAnyBssAccess(Op) && sameAddr(*Store, *S)) {
            // Possible matched-reload template?
            if (Op == Z80::LD_A_nnind && S != std::next(Store) &&
                std::next(S) != MIE) {
              auto PA = std::prev(S);
              auto LR = std::next(S);
              if (PA->getOpcode() == Z80::PUSH_AF && std::next(LR) != MIE) {
                auto PF = std::next(LR);
                if (PF->getOpcode() == Z80::POP_AF) {
                  V = findByLdRegA(LR->getOpcode());
                  if (V) {
                    R0 = PA; R1 = S; R2 = LR; R3 = PF;
                    // Stack depth was incremented when we saw PA; the
                    // template will be deleted, so undo the increment.
                    --StackDepth;
                    break;
                  }
                }
              }
            }
            Bail = true; break;
          }

          if (isPush(Op)) ++StackDepth;
          if (isPop(Op)) {
            --StackDepth;
            if (StackDepth < 0) { Bail = true; break; }
          }
          if (S->isCall()) SeenCall = true;
          recordDefs(*S);
          recordReads(*S);
          if (S->isTerminator()) { Bail = true; break; }
        }

        if (Bail || R0 == MIE || !V) { ++MII; continue; }

        // Phase 2: from just after the matched template's POP_AF, scan
        // forward tracking stack depth until it returns to 0.  That's
        // where our `pop rr` goes (so the bracketing PUSH/POP balances).
        // The matched template's POP_AF itself doesn't contribute to
        // stack depth tracking (it's deleted along with the template).
        auto InsertPopBefore = MIE;
        bool BalanceBail = false;
        for (auto S = std::next(R3); S != MIE; ++S) {
          unsigned Op = S->getOpcode();
          if (StackDepth == 0) { InsertPopBefore = S; break; }
          if (isPush(Op)) ++StackDepth;
          if (isPop(Op)) {
            --StackDepth;
            if (StackDepth < 0) { BalanceBail = true; break; }
          }
          if (StackDepth == 0) {
            // Insert AFTER this pop -- so before the next instruction.
            InsertPopBefore = std::next(S);
            break;
          }
          recordDefs(*S);
          if (S->isTerminator()) { BalanceBail = true; break; }
        }
        if (BalanceBail || InsertPopBefore == MIE) { ++MII; continue; }

        // Partner half must not be defined between store and reload.
        // CALL clobbers per ABI but our PUSH/POP keeps the original; we
        // accept the CALL case because PUSH-saved values surviving the
        // CALL is not a semantic change relative to anything the caller
        // had right to observe.
        if (DefinedRegs.count(V->Partner8)) { ++MII; continue; }

        // The destination register R must not be READ in the interval between
        // the store and the matched reload: the transform inserts `LD R,A` at
        // the store site, so R would carry the spilled value during the region
        // instead of its original contents (ravn/llvm-z80#192).
        if (ReadRegs.count(V->Reg8)) { ++MII; continue; }

        // #203: shared orphan guard (skip the store + reload MIs; scan all
        // blocks, including this one).
        if (z80SlotUsedElsewhere(MF, *Store, {}, {&*Store, &*R1}, nullptr,
                                 nullptr)) { ++MII; continue; }

        LLVM_DEBUG(dbgs() << "  #173 bare-store + 4-instr-reload → "
                          << "LD r,A; PUSH/POP " << TRI->getName(V->PairReg)
                          << " (saves 6 B; CALL=" << (SeenCall ? "yes" : "no")
                          << ")\n");

        // Build replacements.
        // At the store site: replace `LD (slot), A` with `LD r, A; PUSH rr`.
        DebugLoc DLs = Store->getDebugLoc();
        BuildMI(MBB, Store, DLs, TII->get(V->LdRegA));
        BuildMI(MBB, Store, DLs, TII->get(V->PushOpc));
        // Insert POP rr at the discovered balanced point.
        DebugLoc DLr = R3->getDebugLoc();
        BuildMI(MBB, InsertPopBefore, DLr, TII->get(V->PopOpc));
        // Erase the original 4 reload-template instructions.
        MBB.erase(R0); MBB.erase(R1); MBB.erase(R2); MBB.erase(R3);
        // Erase the bare store.
        MII = MBB.erase(Store);
        Changed = true;
      }
    }
  }

  // --- Peephole #18/#206: `LD r, n` → `LD r, r'` when r' already holds n ---
  // #18 (original): `XOR A`/`LD A,n` seeds A; subsequent `LD r,n` with the
  // same n becomes `LD r,A` (1 B vs 2 B, saves 1 B/fire).
  // #206 (extension): track ALL seven GR8 registers (B,C,D,E,H,L,A).
  // When any tracked register r' already holds the wanted constant, emit the
  // cheaper `LD r,r'` (1 B) instead of `LD r,n` (2 B).  Source preference:
  // A first (so existing XOR-A chains keep the same code shape), then B..L
  // in the order they appear in the register file.
  // Tracking is strictly within one basic block; any def of a register
  // (including CALL RegMask clobbers) invalidates that register's entry.
  // ravn/llvm-z80#18 / ravn/llvm-z80#206.
  {
    // Map a `LD r, n` opcode to the destination physical register.
    auto ldNDst = [](unsigned Opc) -> MCPhysReg {
      switch (Opc) {
      case Z80::LD_A_n: return Z80::A;
      case Z80::LD_B_n: return Z80::B;
      case Z80::LD_C_n: return Z80::C;
      case Z80::LD_D_n: return Z80::D;
      case Z80::LD_E_n: return Z80::E;
      case Z80::LD_H_n: return Z80::H;
      case Z80::LD_L_n: return Z80::L;
      default:          return MCPhysReg(0);
      }
    };

    // All seven GR8 registers in source-preference order (A first).
    const MCPhysReg GR8Regs[] = {
        Z80::A, Z80::B, Z80::C, Z80::D, Z80::E, Z80::H, Z80::L};

    for (MachineBasicBlock &MBB : MF) {
      // KnownVal[i]: the 8-bit constant held in GR8Regs[i], or -1 if unknown.
      int64_t KnownVal[7];
      std::fill(std::begin(KnownVal), std::end(KnownVal), -1);

      // Seed A=0 from XOR A / SUB A at the top so the existing #18 behavior
      // for the most common case (xor a + zero-init sequence) is unchanged.
      auto setKnown = [&](MCPhysReg Reg, int64_t Val) {
        for (int i = 0; i < 7; ++i)
          if (GR8Regs[i] == Reg) { KnownVal[i] = Val & 0xFF; return; }
      };
      auto clearKnown = [&](MCPhysReg Reg) {
        for (int i = 0; i < 7; ++i)
          if (TRI->regsOverlap(GR8Regs[i], Reg)) KnownVal[i] = -1;
      };

      for (auto MII = MBB.begin(); MII != MBB.end();) {
        MachineInstr &MI = *MII;
        unsigned Opc = MI.getOpcode();

        // Check for `LD r, n` where some tracked register already holds n.
        if (MCPhysReg Dst = ldNDst(Opc)) {
          if (MI.getNumOperands() >= 1 && MI.getOperand(0).isImm()) {
            int64_t N = MI.getOperand(0).getImm() & 0xFF;
            for (int i = 0; i < 7; ++i) {
              if (GR8Regs[i] == Dst) continue; // don't LD r,r
              if (KnownVal[i] != N) continue;
              unsigned CopyOpc = Z80::getLD8RegOpcode(Dst, GR8Regs[i]);
              if (!CopyOpc) continue;
              BuildMI(MBB, MII, MI.getDebugLoc(), TII->get(CopyOpc));
              MII = MBB.erase(MII);
              Changed = true;
              // Dst now holds N — update tracker without clearing it.
              setKnown(Dst, N);
              goto next_mi_206;
            }
          }
          // No source found — record that Dst now holds N.
          if (MI.getNumOperands() >= 1 && MI.getOperand(0).isImm())
            setKnown(Dst, MI.getOperand(0).getImm());
          else
            clearKnown(Dst);
          ++MII;
          next_mi_206:;
          continue;
        }

        // Seed A=0 from XOR A / SUB A.
        if (Opc == Z80::XOR_A || Opc == Z80::SUB_A) {
          setKnown(Z80::A, 0);
          ++MII;
          continue;
        }

        // Invalidate any defined registers (including CALL clobbers).
        for (const MachineOperand &MO : MI.operands()) {
          if (MO.isRegMask()) {
            for (MCPhysReg R : GR8Regs)
              if (MO.clobbersPhysReg(R)) clearKnown(R);
          } else if (MO.isReg() && MO.getReg().isValid() && MO.isDef()) {
            clearKnown(MO.getReg());
          }
        }
        ++MII;
      }
    }
  }

  // --- Peephole: `mem |= 1<<N` / `mem &= ~(1<<N)` → SET/RES n,(HL) ---
  // Three-instruction sequence:
  //   LD_A_nnind <Sym>        (3 B)
  //   {OR_n,AND_n} K          (2 B)
  //   LD_nnind_A <Sym>        (3 B)   ; same address as load
  // For single-bit ops (popcount(K)==1 for OR, popcount(~K & 0xFF)==1
  // for AND), replace with:
  //   LD_HL_nn <Sym>           (3 B)
  //   {SET,RES}_b_(HL)         (2 B)
  // Saves 3 B per fire.  For two-bit ops, replace with two SET/RES
  // — saves 1 B.  More bits: no win, skip.  ravn/llvm-z80#147.
  //
  // Safety: A must be dead after the store (we don't preserve the
  // OR/AND result in A); HL must be dead at the load position (we
  // clobber HL).
  {
    static const std::pair<unsigned, unsigned> SetOps[8] = {
        {Z80::SET_0_HLind, 0}, {Z80::SET_1_HLind, 1},
        {Z80::SET_2_HLind, 2}, {Z80::SET_3_HLind, 3},
        {Z80::SET_4_HLind, 4}, {Z80::SET_5_HLind, 5},
        {Z80::SET_6_HLind, 6}, {Z80::SET_7_HLind, 7},
    };
    static const std::pair<unsigned, unsigned> ResOps[8] = {
        {Z80::RES_0_HLind, 0}, {Z80::RES_1_HLind, 1},
        {Z80::RES_2_HLind, 2}, {Z80::RES_3_HLind, 3},
        {Z80::RES_4_HLind, 4}, {Z80::RES_5_HLind, 5},
        {Z80::RES_6_HLind, 6}, {Z80::RES_7_HLind, 7},
    };
    auto sameAddrOp = [](const MachineOperand &A,
                         const MachineOperand &B) -> bool {
      if (A.isGlobal() && B.isGlobal())
        return A.getGlobal() == B.getGlobal() &&
               A.getOffset() == B.getOffset();
      if (A.isMCSymbol() && B.isMCSymbol())
        return A.getMCSymbol() == B.getMCSymbol() &&
               A.getOffset() == B.getOffset();
      return false;
    };

    for (MachineBasicBlock &MBB : MF) {
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
        auto LdIt = MII++;
        if (LdIt->getOpcode() != Z80::LD_A_nnind)
          continue;
        if (MII == MIE)
          continue;
        // Skip forward over insns that don't write A or touch
        // memory until we find the OR/AND.  Bail if an intervening
        // insn WRITES A (would clobber our reload), TOUCHES memory
        // (could read/write the target slot or HL), or is a
        // terminator/call.  A-READS are tolerated: ravn/llvm-z80#152
        // preserves A via `LD A,(HL)` after the address load.
        auto OpIt = MII;
        bool BailIntervening = false;
        bool HadAReader = false;
        while (OpIt != MIE) {
          unsigned O = OpIt->getOpcode();
          if (O == Z80::OR_n || O == Z80::AND_n)
            break;
          if (OpIt->isTerminator() || OpIt->isCall()) {
            BailIntervening = true;
            break;
          }
          // Detect actual memory access via MachineMemOperand
          // (mayLoad/mayStore are spuriously set on Z80 register
          // copies and can't be trusted here).
          if (!OpIt->memoperands_empty()) {
            BailIntervening = true;
            break;
          }
          for (const MachineOperand &MO : OpIt->operands()) {
            if (!MO.isReg() || !MO.getReg().isPhysical())
              continue;
            if (!TRI->regsOverlap(MO.getReg(), Z80::A))
              continue;
            if (MO.isDef()) {
              BailIntervening = true;
              break;
            }
            HadAReader = true;
          }
          if (BailIntervening)
            break;
          ++OpIt;
        }
        if (BailIntervening || OpIt == MIE)
          continue;
        unsigned Opc = OpIt->getOpcode();
        bool IsOr = (Opc == Z80::OR_n);
        bool IsAnd = (Opc == Z80::AND_n);
        if (!IsOr && !IsAnd)
          continue;
        auto StIt = std::next(OpIt);
        if (StIt == MIE || StIt->getOpcode() != Z80::LD_nnind_A)
          continue;
        if (!sameAddrOp(LdIt->getOperand(0), StIt->getOperand(0)))
          continue;

        // A must be dead after the store.
        auto AfterSt = std::next(StIt);
        if (!isRegDeadAfter(AfterSt, MBB, TRI, Z80::A))
          continue;
        // HL must be dead at the load (we clobber it with LD_HL_nn).
        if (!isRegDeadAfter(LdIt, MBB, TRI, Z80::H) ||
            !isRegDeadAfter(LdIt, MBB, TRI, Z80::L))
          continue;

        // Decode K and count active bits.
        int64_t K = OpIt->getOperand(0).getImm() & 0xFF;
        unsigned EffMask = IsOr ? (unsigned)K : ((~(unsigned)K) & 0xFF);
        unsigned Pop = llvm::popcount(EffMask);
        if (Pop == 0 || Pop > 2)
          continue;
        // With an intervening A-reader the rewrite also emits a 1 B
        // `LD A,(HL)` to preserve A.  Cost becomes 4 B + 2*Pop:
        //   Pop 1: 6 B → save 2 B (and 1 T-state).
        //   Pop 2: 8 B → break even on size but +14 T — skip.
        // ravn/llvm-z80#152.
        if (HadAReader && Pop != 1)
          continue;
        // Cost without reader: 3 B + 2*Pop B vs 8 B.
        // Pop 1: 5 B → save 3 B.
        // Pop 2: 7 B → save 1 B.

        DebugLoc DL = LdIt->getDebugLoc();
        // Emit LD_HL_nn with the same address operand at LdIt's
        // position (the new sequence's prologue).
        auto NewLd = BuildMI(MBB, *LdIt, DL, TII->get(Z80::LD_HL_nn));
        const MachineOperand &Addr = LdIt->getOperand(0);
        if (Addr.isGlobal())
          NewLd.addGlobalAddress(Addr.getGlobal(), Addr.getOffset());
        else if (Addr.isMCSymbol())
          NewLd.addSym(Addr.getMCSymbol(), Addr.getOffset());
        else {
          // Shouldn't happen given the sameAddrOp guard, but bail safely.
          NewLd.getInstr()->eraseFromParent();
          continue;
        }
        // With an A-reader: emit `LD A,(HL)` right after LD_HL_nn so
        // intervening A-readers see the loaded value.
        if (HadAReader)
          BuildMI(MBB, *LdIt, DL, TII->get(Z80::LD_A_HLind));
        // Emit SET/RES at OpIt's position (after any intervening
        // insns; without readers this is adjacent to LdIt).
        const auto *Table = IsOr ? SetOps : ResOps;
        for (unsigned Bit = 0; Bit < 8; ++Bit) {
          if (EffMask & (1u << Bit))
            BuildMI(MBB, *OpIt, DL, TII->get(Table[Bit].first));
        }
        // Erase the original three instructions.
        StIt->eraseFromParent();
        OpIt->eraseFromParent();
        MII = std::next(LdIt);
        LdIt->eraseFromParent();
        Changed = true;
      }
    }
  }

  // --- Peephole: CP/XOR with 1 or 0xFF → DEC_A/INC_A (when A dead) ---
  // Z80 has 1-byte equivalents of the equality tests A == 1 and
  // A == 0xFF when A's modified value isn't needed afterward:
  //   `DEC A`  (1 B) sets Z iff A was 1
  //   `INC A`  (1 B) sets Z iff A was 0xFF
  // vs `{CP,XOR}_n K` (2 B).  `OR A` (1 B) for A == 0 already fires
  // upstream.  This closes K ∈ {1, 0xFF}.  ravn/llvm-z80#148.
  //
  // Safety: the modified A must be dead along both branches:
  //   - the fall-through (isRegDeadAfter)
  //   - the taken branch (the target MBB must redefine A before
  //     reading it, OR the branch is a RET_cc with A not a return
  //     value).  Approximated by: the target's first non-debug
  //     instruction defines A, OR the branch is RET_cc and A is not
  //     in the function's live-out set.
  {
    auto isCpOrXor = [](unsigned Opc) {
      return Opc == Z80::CP_n || Opc == Z80::XOR_n;
    };
    auto isCondJp = [](unsigned Opc) {
      switch (Opc) {
      case Z80::JP_Z_nn:
      case Z80::JP_NZ_nn:
      case Z80::JR_Z_e:
      case Z80::JR_NZ_e:
        return true;
      default:
        return false;
      }
    };
    auto isCondRet = [](unsigned Opc) {
      return Opc == Z80::RET_Z || Opc == Z80::RET_NZ;
    };
    auto targetDeadA =
        [&TRI](MachineBasicBlock *TargetMBB) -> bool {
      if (!TargetMBB)
        return false;
      // Walk target's instructions: if A is defined before being
      // read, it was dead at entry; if it's read before defined,
      // it's live at entry.  If we reach a terminator without seeing
      // either, fall through to the MBB's liveouts.
      for (auto &MI : *TargetMBB) {
        if (MI.isDebugInstr())
          continue;
        // XOR_A is the canonical "clear A" idiom: `xor a` zeros A.  It
        // formally reads A (Uses=[A]) but the read value is irrelevant
        // — the instruction unconditionally sets A to 0.  Treat it as
        // a full def (A dead at entry to this MBB).
        if (MI.getOpcode() == Z80::XOR_A)
          return true;
        bool ReadsA = false, DefsA = false;
        for (const MachineOperand &MO : MI.operands()) {
          if (!MO.isReg() || !MO.getReg().isPhysical())
            continue;
          if (!TRI->regsOverlap(MO.getReg(), Z80::A))
            continue;
          if (MO.readsReg())
            ReadsA = true;
          if (MO.isDef())
            DefsA = true;
        }
        if (ReadsA)
          return false;
        if (DefsA)
          return true;
        if (MI.isCall())
          return false;
        // For a return terminator, check the MBB's liveouts.
        if (MI.isReturn()) {
          for (MachineBasicBlock *Succ : TargetMBB->successors()) {
            for (const auto &LI : Succ->liveins())
              if (TRI->regsOverlap(LI.PhysReg, Z80::A))
                return false;
          }
          // No successor with A live-in: also check the function's
          // return-value liveness via the RET instruction's implicit
          // operands.  If RET has an implicit-use of A (or AF), A
          // is a return value — live.
          for (const MachineOperand &MO : MI.operands()) {
            if (MO.isReg() && MO.isImplicit() && MO.readsReg() &&
                MO.getReg().isPhysical() &&
                TRI->regsOverlap(MO.getReg(), Z80::A))
              return false;
          }
          return true;  // A dead at return.
        }
        if (MI.isBranch())
          return false;  // give up at non-return terminator
      }
      return false;
    };

    for (MachineBasicBlock &MBB : MF) {
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
        auto OpIt = MII++;
        if (!isCpOrXor(OpIt->getOpcode()))
          continue;
        if (MII == MIE)
          continue;
        auto BrIt = MII;
        unsigned BrOpc = BrIt->getOpcode();
        bool IsJp = isCondJp(BrOpc);
        bool IsRet = isCondRet(BrOpc);
        if (!IsJp && !IsRet)
          continue;
        int64_t K = OpIt->getOperand(0).getImm() & 0xFF;
        unsigned NewOpc = 0;
        if (K == 1)
          NewOpc = Z80::DEC_A;
        else if (K == 0xFF)
          NewOpc = Z80::INC_A;
        else
          continue;
        // A must be dead after the branch on both paths.
        auto AfterBr = std::next(BrIt);
        if (!isRegDeadAfter(AfterBr, MBB, TRI, Z80::A))
          continue;
        if (IsJp) {
          if (!BrIt->getOperand(0).isMBB())
            continue;
          if (!targetDeadA(BrIt->getOperand(0).getMBB()))
            continue;
        }
        // ravn/llvm-z80#184 fix: the fall-through MBB also needs an
        // explicit A-dead check.  `isRegDeadAfter(AfterBr)` reaches
        // the end of the current MBB and only inspects Succ->liveins(),
        // which may not have been refreshed post-regalloc.  Walk the
        // fall-through MBB's instructions directly via targetDeadA.
        // This is needed for BOTH IsJp (where AfterBr falls through
        // to a body that uses A via push af) and IsRet (similar).
        if (MachineBasicBlock *Fall = MBB.getNextNode()) {
          if (!targetDeadA(Fall))
            continue;
        }
        // FLAGS must be dead after the branch too.  CP/XOR set C
        // (and other flags) but DEC_A / INC_A leave C unchanged.
        // Any downstream consumer of C — e.g. `JR C, X; SBC A, A` —
        // would see different flag state with our replacement.
        // Conservative: require FLAGS dead-after along both paths.
        if (!isRegDeadAfter(AfterBr, MBB, TRI, Z80::FLAGS))
          continue;
        if (IsJp) {
          if (!targetDeadA(BrIt->getOperand(0).getMBB()))
            continue;
          // Reuse the same "first non-debug insn defs/clobbers"
          // shape but on FLAGS instead of A.  Most flag-setting
          // arithmetic at MBB entry will count as a redefinition,
          // so FLAGS rarely flows across MBB boundaries.  Cheap
          // check: target's first non-debug, non-terminator insn
          // must DEFINE FLAGS before any read.
          MachineBasicBlock *TargetMBB = BrIt->getOperand(0).getMBB();
          bool TgtFlagsOK = false;
          for (auto &MI : *TargetMBB) {
            if (MI.isDebugInstr())
              continue;
            bool ReadsF = false, DefsF = false;
            for (const MachineOperand &MO : MI.operands()) {
              if (!MO.isReg() || !MO.getReg().isPhysical())
                continue;
              if (!TRI->regsOverlap(MO.getReg(), Z80::FLAGS))
                continue;
              if (MO.readsReg())
                ReadsF = true;
              if (MO.isDef())
                DefsF = true;
            }
            if (ReadsF)
              break;       // FLAGS live at target → unsafe
            if (DefsF) {
              TgtFlagsOK = true;
              break;
            }
            if (MI.isReturn() || MI.isCall()) {
              // RET/CALL don't consume FLAGS — they're effectively
              // a "dead" point for FLAGS unless a caller-side check
              // (which we can't model here) reads them.  Treat as
              // safe to be aggressive — most function entries
              // discard incoming flags.
              TgtFlagsOK = true;
              break;
            }
          }
          if (!TgtFlagsOK)
            continue;
        }
        // For RET_cc: rely on isRegDeadAfter — the standard sdcccall(1)
        // convention has A as return register, so a function returning
        // u8 will have A live at exit.  But if isRegDeadAfter(AfterBr,
        // ...) saw A as dead at MBB end (no further uses + no live-out
        // tracked), trust it.  This may be conservative but is safe.

        DebugLoc DL = OpIt->getDebugLoc();
        BuildMI(MBB, *OpIt, DL, TII->get(NewOpc));
        OpIt->eraseFromParent();
        Changed = true;
      }
    }
  }

  // --- Peephole: SBC A,A; AND 1; RRCA; SBC A,A round-trip elimination ---
  // Post-#144 chain after icmp sext-to-i16:
  //   sbc a, a    ; A = 0xFF/0x00 (i1 sign-extended)
  //   and 1       ; A = 0x01/0x00
  //   rrca        ; CF = bit 0
  //   sbc a, a    ; A = 0xFF/0x00 (round-trip back to first SBC's output)
  // The trailing `and 1; rrca; sbc a, a` triple is a no-op when the
  // first SBC A,A is in scope: it converts the i1-sign-extension to
  // {0,1} form and back.  Delete the triple.  ravn/llvm-z80#151.
  for (MachineBasicBlock &MBB : MF) {
    for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
      auto SbcAA = MII++;
      if (SbcAA->getOpcode() != Z80::SBC_A_A)
        continue;
      // Expect next three: AND_n 1, RRCA, SBC_A_A.
      auto AndIt = MII;
      if (AndIt == MIE || AndIt->getOpcode() != Z80::AND_n)
        continue;
      if ((AndIt->getOperand(0).getImm() & 0xFF) != 1)
        continue;
      auto RrcaIt = std::next(AndIt);
      if (RrcaIt == MIE || RrcaIt->getOpcode() != Z80::RRCA)
        continue;
      auto Sbc2It = std::next(RrcaIt);
      if (Sbc2It == MIE || Sbc2It->getOpcode() != Z80::SBC_A_A)
        continue;
      // Delete the three intermediate insns; the second SBC_A_A
      // already produced the same A value as the first, so the
      // chain (AND, RRCA, SBC) is a no-op.  We keep the first
      // SBC (it's the original i1 sign-extension) and let MII
      // continue from there.
      auto After = std::next(Sbc2It);
      Sbc2It->eraseFromParent();
      RrcaIt->eraseFromParent();
      AndIt->eraseFromParent();
      MII = After;
      Changed = true;
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

  // NOTE (ravn/llvm-z80#194): the cross-block redundant `LD A,r` removal above
  // extends A's live range across a block edge but leaves block live-ins stale
  // (gf_log's `ADD_A_A` reads `$a` with `$a` absent from %bb.2 live-ins, which
  // -verify-machineinstrs flags).  This is benign at runtime (the value is
  // correct).  A `fullyRecomputeLiveIns(MF)` here fixes the metadata, but
  // downstream block-placement reacts to the corrected live-ins and grows the
  // 2 KB-capped cpnos PROM by 2 B, and it does NOT make the module
  // verify-clean (PEI and other generic post-RA passes have their own
  // pre-existing staleness).  Deferred pending either a byte-neutral surgical
  // live-in update in the #60 removal or a coordinated verify-clean effort.

  // --- Peephole (experimental, #205 follow-up): reversed (HL) fill seed ---
  // Match the K=2 LDIR-fill setup the pattern-fill lowering emits (DE first):
  //   I5: LD_DE_nn  addr+2     ; LDIR destination (kept)
  //   I4: LD_HL_nn  VAL        ; value (constant or symbol)
  //   I3: LD_nnind_HL addr     ; seed store  (addr)
  //   I2: LD_HL_nn  addr       ; LDIR source  (== seed addr)
  //   I1: LD_BC_nn  len        ; LDIR count (kept)
  //       LDIR
  // and rewrite the value-load + seed-store + source-load (9 B) into a reversed
  // (HL) seed that lands HL on the base (8 B):
  //   LD HL,addr+1 ; LD (HL),hi ; DEC HL ; LD (HL),lo   (HL ends = addr = src)
  // HL is then the LDIR source, so the separate `LD HL,addr` is folded away.
  if (EnableReverseFillSeed) {
    auto sameAddr = [](const MachineOperand &A, const MachineOperand &B) {
      if (A.isImm() && B.isImm())
        return A.getImm() == B.getImm();
      if (A.isGlobal() && B.isGlobal())
        return A.getGlobal() == B.getGlobal() &&
               A.getOffset() == B.getOffset() &&
               A.getTargetFlags() == B.getTargetFlags();
      return false;
    };
    for (MachineBasicBlock &MBB : MF) {
      for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
           MII != MIE; ++MII) {
        if (MII->getOpcode() != Z80::LDIR || MII == MBB.begin())
          continue;
        auto I1 = std::prev(MII);
        if (I1 == MBB.begin() || I1->getOpcode() != Z80::LD_BC_nn)
          continue;
        auto I2 = std::prev(I1);
        if (I2 == MBB.begin() || I2->getOpcode() != Z80::LD_HL_nn)
          continue;
        auto I3 = std::prev(I2);
        if (I3 == MBB.begin() || I3->getOpcode() != Z80::LD_nnind_HL)
          continue;
        auto I4 = std::prev(I3);
        const MachineOperand ValOp = I4->getOperand(0);  // the fill value
        if (I4 == MBB.begin() || I4->getOpcode() != Z80::LD_HL_nn ||
            !(ValOp.isImm() || ValOp.isGlobal()))
          continue;
        // I5 = the LDIR-destination load (LD DE,dst), emitted before the seed
        // by the pattern-fill lowering; it is kept as-is.
        auto I5 = std::prev(I4);
        if (I5->getOpcode() != Z80::LD_DE_nn)
          continue;
        // Seed-store address must equal the LDIR source address.
        if (!sameAddr(I3->getOperand(0), I2->getOperand(0)))
          continue;

        const MachineOperand Addr = I2->getOperand(0);  // base (== seed addr)
        DebugLoc DL = I2->getDebugLoc();

        // LD HL,addr+1
        auto SrcLd = BuildMI(MBB, *I2, DL, TII->get(Z80::LD_HL_nn));
        if (Addr.isImm())
          SrcLd.addImm(Addr.getImm() + 1);
        else
          SrcLd.addGlobalAddress(Addr.getGlobal(), Addr.getOffset() + 1,
                                 Addr.getTargetFlags());

        // Emit `LD (HL),<hi/lo of VAL>`.  A constant VAL splits into two imm
        // bytes; a symbol VAL uses MO_HI/MO_LO byte-half operands (lowered to
        // the Addr16_High / Addr16_Low relocation in Z80MCInstLower).
        auto emitByteStore = [&](bool HiHalf) {
          auto MIB = BuildMI(MBB, *I2, DL, TII->get(Z80::LD_HLind_n));
          if (ValOp.isImm()) {
            int64_t V = ValOp.getImm();
            MIB.addImm(HiHalf ? ((V >> 8) & 0xFF) : (V & 0xFF));
          } else {
            MIB.addGlobalAddress(ValOp.getGlobal(), ValOp.getOffset(),
                                 HiHalf ? Z80::MO_HI : Z80::MO_LO);
          }
        };
        // LD (HL),hi ; DEC HL ; LD (HL),lo   -> HL = addr
        emitByteStore(/*HiHalf=*/true);
        BuildMI(MBB, *I2, DL, TII->get(Z80::DEC_HL));
        emitByteStore(/*HiHalf=*/false);

        LLVM_DEBUG(dbgs() << "  reversed fill-seed\n");
        // Drop the value load (I4), the absolute seed store (I3) and the
        // separate source load (I2); the LD DE,dst (I5) and LD BC,len (I1) are
        // kept, and HL now lands on the base via the reversed (HL) seed above.
        I4->eraseFromParent();
        I3->eraseFromParent();
        I2->eraseFromParent();
        Changed = true;
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
