//===-- Z80PreEmitPeephole.cpp - Z80 pre-emit peephole --------------------===//
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

#include "Z80PreEmitPeephole.h"

#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80Subtarget.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "z80-pre-emit-peephole"

STATISTIC(NumLDHLReused, "Number of HL frame addresses reused");
STATISTIC(NumZeroLogicElided, "Number of no-op logic operations removed");
STATISTIC(NumCopiesFolded, "Number of copies folded into frame accesses");
STATISTIC(NumDeadReloads, "Number of dead frame reloads erased");
STATISTIC(NumPairReloads, "Number of reloads redirected into a register pair");
STATISTIC(NumIncDec, "Number of increments/decrements applied in place");
STATISTIC(NumPopPushElided, "Number of POP/PUSH pairs elided");
STATISTIC(NumPushPopElided, "Number of PUSH/POP pairs elided");
STATISTIC(NumPostIncFused, "Number of accesses fused into post-increment form");
STATISTIC(NumConstStores, "Number of constant stores materialized through A");
STATISTIC(NumSingleBitMasks, "Number of single-bit masks folded to RES");
STATISTIC(NumIXConstStores,
          "Number of constant stores materialized through IX");
STATISTIC(NumPopPushPairs, "Number of POP/PUSH pairs on the same pair removed");
STATISTIC(NumDecInPlace,
          "Number of load/decrement/store sequences done in place");
STATISTIC(NumCplFolds, "Number of XOR 0xFF folded to CPL");
STATISTIC(NumZeroAFolds, "Number of LD A,0 folded to XOR A");
STATISTIC(NumAluImmMerges, "Number of consecutive ALU immediates merged");
STATISTIC(NumImm16Stores,
          "Number of 16-bit immediate stores routed through the slot");
STATISTIC(NumLDHLStepped,
          "Number of consecutive LDHL SP addresses stepped with INC/DEC");
STATISTIC(NumCmpImmFolds,
          "Number of constants folded into a 16-bit XOR compare");
STATISTIC(NumPostIncLoads,
          "Number of loads rewritten to the post-increment form");
STATISTIC(NumHLPostIncLoads, "Number of 16-bit HL loads rewritten through HL+");
STATISTIC(NumSlotForwarded,
          "Number of SM83 SP-relative slot accesses forwarded");
STATISTIC(NumStoreForwarded, "Number of stores forwarded to a following load");
STATISTIC(NumI16ByteXorToSbc,
          "Number of i16 EQ/NE byte-XOR compares rewritten to SBC HL");
STATISTIC(NumInMemIncDec,
          "Number of in-memory byte increments/decrements folded to (HL)");
STATISTIC(NumConsecutiveStores,
          "Number of consecutive byte stores folded to LD (HL),n chains");
STATISTIC(NumConstReused,
          "Number of 8-bit immediate loads rewritten to register copies");
STATISTIC(NumInMemBitSetRes,
          "Number of in-memory bit sets/resets folded to SET/RES (HL)");
STATISTIC(NumDjnz,
          "Number of decrement-and-branch sequences folded to DJNZ");
STATISTIC(NumBssSpillsToPushPop,
          "Number of BSS spill/reload pairs converted to PUSH/POP");
STATISTIC(NumTailCalls,
          "Number of tail calls optimized (CALL; RET -> JP)");
STATISTIC(NumAndRotateFolds,
          "Number of AND 1/0x80 folded to RRCA/RLCA");
STATISTIC(NumDecIncEquality,
          "Number of CP/XOR 1/0xFF folded to DEC_A/INC_A");
STATISTIC(NumBranchesShortened, "Number of JP branches shortened to JR");
STATISTIC(NumRedundantLdAR,
          "Number of redundant LD A,r instructions eliminated");
STATISTIC(NumCrossClassBssSpills,
          "Number of cross-class BSS spill/reload pairs converted to PUSH/POP");
STATISTIC(NumCarryRoundtrips,
          "Number of carry flag roundtrips folded to direct branch");
STATISTIC(NumSPSpillsToPushPop,
          "Number of SP-relative spill/reload pairs converted to PUSH/POP (#331)");

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

// --- Asking an instruction what it is ---
//
// Most instructions here name their registers as operands, so a peephole
// matches on the opcode and the registers together rather than on one of the
// many opcodes a register pair used to have. These are the vocabulary for
// that; each returns an invalid register, or false, for anything else.

/// Whether \p MI is the 8-bit register copy.
static bool isLD8(const MachineInstr &MI) {
  return MI.getOpcode() == Z80::LD_r_r;
}

static bool isLD8(const MachineInstr &MI, Register Dst, Register Src) {
  return isLD8(MI) && MI.getOperand(0).getReg() == Dst &&
         MI.getOperand(1).getReg() == Src;
}

/// The source of a copy into \p Dst, or an invalid register if \p MI is not
/// a copy into that register.
static Register getLD8Src(const MachineInstr &MI, Register Dst) {
  if (!isLD8(MI) || MI.getOperand(0).getReg() != Dst)
    return Register();
  return MI.getOperand(1).getReg();
}

/// The destination of a copy out of \p Src, or an invalid register if \p MI
/// is not a copy out of that register.
static Register getLD8Dst(const MachineInstr &MI, Register Src) {
  if (!isLD8(MI) || MI.getOperand(1).getReg() != Src)
    return Register();
  return MI.getOperand(0).getReg();
}

/// Whether \p MI is the 8-bit immediate load.
static bool isLD8n(const MachineInstr &MI) {
  return MI.getOpcode() == Z80::LD_r_n;
}

/// The register an `LD r, n` writes, or an invalid register if \p MI is not
/// one. The immediate is its second operand, and can be a symbol reference
/// rather than a constant.
static Register getLD8nDst(const MachineInstr &MI) {
  return isLD8n(MI) ? MI.getOperand(0).getReg() : Register();
}

/// The pair an `LD rr, nn` writes, or an invalid register if \p MI is not
/// one. Like LD r,n the immediate is the second operand.
static Register getLD16nDst(const MachineInstr &MI) {
  return MI.getOpcode() == Z80::LD_rr_nn ? MI.getOperand(0).getReg()
                                         : Register();
}

/// Whether \p MI loads \p Dst from the byte HL points at.
static bool isLoadHL(const MachineInstr &MI, Register Dst) {
  return MI.getOpcode() == Z80::LD_r_HLind && MI.getOperand(0).getReg() == Dst;
}

/// Whether \p MI stores \p Src to the byte HL points at.
static bool isStoreHL(const MachineInstr &MI, Register Src) {
  return MI.getOpcode() == Z80::LD_HLind_r && MI.getOperand(0).getReg() == Src;
}

/// Whether \p R carries data rather than the address of an HL-indirect
/// access. H and L hold the address, and a rewrite that moved such an access
/// would be reasoning about the address as if it were data.
static bool isAccessDataReg(Register R) {
  return R.isValid() && R != Z80::H && R != Z80::L;
}

/// The register an `LD r, (HL)` loads, leaving out the halves of the address
/// itself, or an invalid register if \p MI is not such a load.
static Register getLoadHLindDstReg(const MachineInstr &MI) {
  if (MI.getOpcode() != Z80::LD_r_HLind)
    return Register();
  Register Dst = MI.getOperand(0).getReg();
  return isAccessDataReg(Dst) ? Dst : Register();
}

/// The register an `LD (HL), r` stores, leaving out the halves of the address
/// itself, or an invalid register if \p MI is not such a store.
static Register getStoreHLindSrcReg(const MachineInstr &MI) {
  if (MI.getOpcode() != Z80::LD_HLind_r)
    return Register();
  Register Src = MI.getOperand(0).getReg();
  return isAccessDataReg(Src) ? Src : Register();
}

/// The register an `LD (IX+d), r` stores, or an invalid register if \p MI is
/// not such a store.
static Register getStoreIXdSrcReg(const MachineInstr &MI) {
  return MI.getOpcode() == Z80::LD_IXd_r ? MI.getOperand(1).getReg()
                                         : Register();
}

/// The register an `LD r, (IX+d)` loads, or an invalid register if \p MI is
/// not such a load.
static Register getLoadIXdDstReg(const MachineInstr &MI) {
  return MI.getOpcode() == Z80::LD_r_IXd ? MI.getOperand(0).getReg()
                                         : Register();
}

/// Whether \p MI is the accumulator operation \p Opc against \p Src.
static bool isAlu8(const MachineInstr &MI, unsigned Opc, Register Src) {
  return MI.getOpcode() == Opc && MI.getOperand(0).getReg() == Src;
}

/// Whether \p MI is `XOR A`, which sets A to zero rather than combining it
/// with another register.
static bool isZeroA(const MachineInstr &MI) {
  return isAlu8(MI, Z80::XOR_r, Z80::A);
}

// Get the register an OR r / XOR r reads besides A, or Register(). Both
// leave A untouched when that register holds zero. A itself is excluded:
// OR A and XOR A mean something else.
static Register getZeroNeutralAluSrcReg(const MachineInstr &MI) {
  unsigned Opc = MI.getOpcode();
  if (Opc != Z80::OR_r && Opc != Z80::XOR_r)
    return Register();
  Register Src = MI.getOperand(0).getReg();
  return Src == Z80::A ? Register() : Src;
}

// Get the register an OR r / XOR r / ADD A,r reads besides A, or Register().
// All three copy that register into A when A holds zero.
static Register getAccumulatorNeutralAluSrcReg(const MachineInstr &MI) {
  unsigned Opc = MI.getOpcode();
  if (Opc != Z80::OR_r && Opc != Z80::XOR_r && Opc != Z80::ADD_A_r)
    return Register();
  Register Src = MI.getOperand(0).getReg();
  return Src == Z80::A ? Register() : Src;
}

// For an 8-bit ALU instruction that reads a register, give back that
// register and the IX-indexed form of the same operation. A is excluded:
// the folded form addresses memory, which A cannot stand in for.
static Register getAluRegSrc(const MachineInstr &MI, unsigned &IXdOpc) {
  unsigned Opc = Z80::getAluRegIXdOpcode(MI.getOpcode());
  if (!Opc)
    return Register();
  Register Src = MI.getOperand(0).getReg();
  if (Src == Z80::A)
    return Register();
  IXdOpc = Opc;
  return Src;
}

/// Whether \p MI increments or decrements \p Reg. \p Opc is Z80::INC_r or
/// Z80::DEC_r.
static bool isIncDec8(const MachineInstr &MI, unsigned Opc, Register Reg) {
  return MI.getOpcode() == Opc && MI.getOperand(0).getReg() == Reg;
}

/// Whether \p MI increments or decrements \p Pair. \p Opc is Z80::INC_rr or
/// Z80::DEC_rr.
static bool isIncDec16(const MachineInstr &MI, unsigned Opc, Register Pair) {
  return MI.getOpcode() == Opc && MI.getOperand(0).getReg() == Pair;
}

// Whether a stack access may be reasoned about as an ordinary read or write
// of a frame slot. The memory operand rides on the frame index pseudo from
// selection through expansion, so an access with none left is one this pass
// did not follow and should not draw conclusions from, and a volatile one is
// an access the program asked to actually perform.
static bool isPlainSlotAccess(const MachineInstr &MI) {
  return !MI.memoperands_empty() &&
         llvm::all_of(MI.memoperands(), [](const MachineMemOperand *MMO) {
           return MMO->isUnordered() && !MMO->isVolatile();
         });
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

class Z80PreEmitPeephole : public MachineFunctionPass {
public:
  static char ID;

  Z80PreEmitPeephole() : MachineFunctionPass(ID) {
    llvm::initializeZ80PreEmitPeepholePass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

// --- SM83: reuse the address LDHL SP,e left in HL ---
//
// Every stack slot access recomputes its address from scratch, but spill
// code touches neighboring slots in bursts, so HL usually still holds an
// address one byte away (the 16-bit spill expansion even ends on slot+1
// via LD (HL+)). Track what HL holds relative to the current SP and turn
// a recomputation into nothing (same slot) or INC/DEC HL (next slot).
//
// The offset is tracked relative to SP, so SP movement (PUSH/POP/ADD SP)
// shifts it; a call ends tracking, since callee cleanup leaves SP
// unknowable here (the same reason the push ix/pop hl rewrite was
// abandoned). LDHL also defines FLAGS, so a rewrite needs FLAGS dead.
static bool reuseLDHLAddress(MachineBasicBlock &MBB, const TargetInstrInfo *TII,
                             const TargetRegisterInfo *TRI);

// Whether nothing at or below \p After wants the value in \p Reg, which is
// what lets a transformation clobber it.
static bool isRegDeadAfter(MachineBasicBlock::iterator After,
                           MachineBasicBlock &MBB,
                           const TargetRegisterInfo *TRI, MCPhysReg Reg) {
  return !Z80::isLiveAt(MBB, After, Reg, TRI);
}

// OR or XOR against a register holding zero leaves A exactly as it was, so
// all such an instruction really does is set flags. Where those are dead
// too it does nothing at all. These come from wide values whose upper half
// is a known zero: the byte-wise expansion has no way to see it.
//
// Dropping the instruction usually leaves the constant that fed it dead as
// well, which the sweep below then takes.
static bool elideZeroOperandLogic(MachineBasicBlock &MBB,
                                  const TargetInstrInfo *TII,
                                  const TargetRegisterInfo *TRI) {
  static const MCPhysReg Regs8[] = {Z80::A, Z80::B, Z80::C, Z80::D,
                                    Z80::E, Z80::H, Z80::L};
  bool Changed = false;
  SmallSet<MCPhysReg, 8> Zero;

  for (auto MII = MBB.begin(); MII != MBB.end();) {
    MachineInstr &MI = *MII;

    if (Register Src = getZeroNeutralAluSrcReg(MI);
        Src.isValid() && Zero.count(Src.asMCReg()) &&
        isRegDeadAfter(std::next(MII), MBB, TRI, Z80::FLAGS)) {
      LLVM_DEBUG(dbgs() << "  Zero operand, no effect: " << MI);
      MII = MBB.erase(MII);
      ++NumZeroLogicElided;
      Changed = true;
      continue;
    }

    // The mirror case: a zero accumulator makes OR, XOR and ADD A a plain
    // move. Written as one, the copy in and the copy back out cancel, and
    // the peepholes below take both.
    if (Register Src = getAccumulatorNeutralAluSrcReg(MI);
        Src.isValid() && Zero.count(Z80::A) &&
        isRegDeadAfter(std::next(MII), MBB, TRI, Z80::FLAGS)) {
      LLVM_DEBUG(dbgs() << "  Zero accumulator, is a move: " << MI);
      Z80::buildLD8(MBB, MII, MI.getDebugLoc(), *TII, Z80::A, Src);
      MII = MBB.erase(MII);
      Zero.erase(Z80::A);
      ++NumZeroLogicElided;
      Changed = true;
      continue;
    }

    if (MI.isCall() || MI.isInlineAsm()) {
      Zero.clear();
    } else {
      for (const MachineOperand &MO : MI.operands())
        if (MO.isReg() && MO.isDef() && MO.getReg().isPhysical())
          for (MCPhysReg R : Regs8)
            if (TRI->regsOverlap(MO.getReg(), R))
              Zero.erase(R);

      if (isZeroA(MI))
        Zero.insert(Z80::A);
      if (Register Src = getLD8Src(MI, Z80::A);
          Src.isValid() && Src != Z80::A && Zero.count(Src))
        Zero.insert(Z80::A);
      Register Narrow = getLD8nDst(MI);
      Register Pair = getLD16nDst(MI);
      if (Narrow.isValid() || Pair.isValid()) {
        const MachineOperand &Val = MI.getOperand(1);
        if (Val.isImm() && Val.getImm() == 0) {
          if (Pair.isValid())
            for (MCSubRegIterator SR(Pair, TRI); SR.isValid(); ++SR)
              Zero.insert(*SR);
          else
            Zero.insert(Narrow.asMCReg());
        }
      }
    }
    ++MII;
  }
  if (Changed)
    recomputeLivenessFlags(MBB);
  return Changed;
}

// Ways a register ends up carrying a value between memory and somewhere it
// did not need to stop:
//
//   ld c,(ix+d) ; ld a,c       ->  ld a,(ix+d)
//   ld c,(hl)   ; ld a,c       ->  ld a,(hl)
//   ld c,#n     ; ld (ix+d),c  ->  ld (ix+d),#n
//
// Each costs a byte and occupies a register the allocator had no reason to
// spend. The register has to be dead afterwards, since it stops being
// written.
static bool foldCopyIntoFrameAccess(MachineBasicBlock &MBB,
                                    const TargetInstrInfo *TII,
                                    const TargetRegisterInfo *TRI) {
  bool Changed = false;
  for (auto MII = MBB.begin(); MII != MBB.end();) {
    auto Next = std::next(MII);
    if (Next == MBB.end()) {
      ++MII;
      continue;
    }
    const DebugLoc &DL = MII->getDebugLoc();

    bool FromSlot = getLoadIXdDstReg(*MII).isValid();
    Register Loaded =
        FromSlot ? getLoadIXdDstReg(*MII) : getLoadHLindDstReg(*MII);
    if (Loaded.isValid()) {
      int64_t Slot = FromSlot ? Z80::idxSlotOperand(*MII).getImm() : 0;
      // The consumer to fold into. It need not come next, as long as
      // nothing in between disturbs the register or what the load reads.
      auto Use = MBB.end();
      Register Copied;
      unsigned IXdOpc = 0;
      unsigned Budget = 32;
      for (auto Scan = Next; Scan != MBB.end() && Budget; ++Scan, --Budget) {
        if (FromSlot && getAluRegSrc(*Scan, IXdOpc) == Loaded) {
          Use = Scan;
          break;
        }
        Copied = getLD8Dst(*Scan, Loaded);
        if (Copied.isValid()) {
          Use = Scan;
          break;
        }
        if (Scan->isCall() || Scan->isInlineAsm() ||
            Scan->readsRegister(Loaded, TRI) ||
            Scan->modifiesRegister(Loaded, TRI))
          break;
        // The read moves to where the use is, so nothing may write what it
        // reads on the way. Another frame slot is provably a different
        // byte; anything else could be a pointer into this one.
        if (Scan->mayStore()) {
          bool OtherSlot = FromSlot &&
                           (getStoreIXdSrcReg(*Scan).isValid() ||
                            Scan->getOpcode() == Z80::LD_IXd_n) &&
                           Z80::idxSlotOperand(*Scan).getImm() != Slot;
          if (!OtherSlot)
            break;
        }
        // The load is the same load only while the register it addresses
        // through still holds the address.
        if (Scan->modifiesRegister(FromSlot ? Z80::IX : Z80::HL, TRI))
          break;
      }

      // A load through HL cannot be redirected into H or L: that is the
      // address register, and anything reading it afterwards would see the
      // loaded byte instead.
      bool ClobbersAddr = !FromSlot && (Copied == Z80::H || Copied == Z80::L);
      if (Use != MBB.end() && Copied != Loaded && !ClobbersAddr &&
          isRegDeadAfter(std::next(Use), MBB, TRI, Loaded.asMCReg())) {
        const DebugLoc &UseDL = Use->getDebugLoc();
        if (Copied.isValid()) {
          auto MIB = FromSlot ? Z80::buildLoadIdx(MBB, Use, UseDL, *TII,
                                                  Z80::LD_r_IXd, Copied, Slot)
                              : Z80::buildLoadHL(MBB, Use, UseDL, *TII, Copied);
          MIB.cloneMemRefs(*MII);
        } else {
          BuildMI(MBB, Use, UseDL, TII->get(IXdOpc))
              .addImm(Slot)
              .cloneMemRefs(*MII);
        }
        MBB.erase(Use);
        MII = MBB.erase(MII);
        ++NumCopiesFolded;
        Changed = true;
        continue;
      }
    }

    if (Register Held = getLD8nDst(*MII);
        Held.isValid() && getStoreIXdSrcReg(*Next) == Held &&
        MII->getOperand(1).isImm() &&
        isRegDeadAfter(std::next(Next), MBB, TRI, Held.asMCReg())) {
      BuildMI(MBB, MII, DL, TII->get(Z80::LD_IXd_n))
          .addImm(Next->getOperand(0).getImm())
          .addImm(MII->getOperand(1).getImm() & 0xFF)
          .cloneMemRefs(*Next);
      MII = MBB.erase(MII);
      MII = MBB.erase(MII);
      ++NumCopiesFolded;
      Changed = true;
      continue;
    }

    ++MII;
  }
  if (Changed)
    recomputeLivenessFlags(MBB);
  return Changed;
}

// A reload from a frame slot, or a constant loaded into a register, whose
// destination is overwritten before it is read buys nothing: a pair reloaded
// because one of its bytes is needed leaves the other one dead, and so does
// a constant whose only reader has just been dropped.
static bool eraseDeadFrameReloads(MachineBasicBlock &MBB,
                                  const TargetRegisterInfo *TRI) {
  bool Changed = false;
  for (auto MII = MBB.begin(); MII != MBB.end();) {
    Register Dst = getLoadIXdDstReg(*MII);
    bool IsSlotLoad = Dst.isValid();
    if (!Dst.isValid())
      Dst = getLD8nDst(*MII);
    if (!Dst.isValid() && isLD8(*MII))
      Dst = MII->getOperand(0).getReg();
    if (!Dst.isValid())
      Dst = getLD16nDst(*MII);
    if (!Dst.isValid() || (IsSlotLoad && !isPlainSlotAccess(*MII)) ||
        !isRegDeadAfter(std::next(MII), MBB, TRI, Dst)) {
      ++MII;
      continue;
    }
    LLVM_DEBUG(dbgs() << "  Removing dead def: " << *MII);
    MII = MBB.erase(MII);
    ++NumDeadReloads;
    Changed = true;
  }
  if (Changed)
    recomputeLivenessFlags(MBB);
  return Changed;
}

// SM83 reloads a 16-bit frame slot through HL, which doubles as the address
// register: LD A,(HL+); LD H,(HL); LD L,A. When the value was wanted in
// another pair the copy out of HL follows, and the two halves can simply be
// loaded where they belong instead.
//
//   ld a,(hl+) ; ld h,(hl) ; ld l,a ; ld c,l ; ld b,h
//     ->  ld a,(hl+) ; ld b,(hl) ; ld c,a
//
// Only when HL itself is dead afterwards, since the rewrite stops writing it.
static bool reloadDirectlyIntoPair(MachineBasicBlock &MBB,
                                   const TargetInstrInfo *TII,
                                   const TargetRegisterInfo *TRI) {
  static const struct {
    MCPhysReg Lo, Hi;
  } Pairs[] = {{Z80::C, Z80::B}, {Z80::E, Z80::D}};

  bool Changed = false;
  for (auto MII = MBB.begin(); MII != MBB.end();) {
    auto Load = MII;
    if (Load->getOpcode() != Z80::LD_A_HLI) {
      ++MII;
      continue;
    }
    auto LoadHi = std::next(Load);
    if (LoadHi == MBB.end() || !isLoadHL(*LoadHi, Z80::H)) {
      ++MII;
      continue;
    }
    auto SetL = std::next(LoadHi);
    if (SetL == MBB.end() || !isLD8(*SetL, Z80::L, Z80::A)) {
      ++MII;
      continue;
    }
    auto SetLo = std::next(SetL);
    if (SetLo == MBB.end()) {
      ++MII;
      continue;
    }
    auto SetHi = std::next(SetLo);
    if (SetHi == MBB.end()) {
      ++MII;
      continue;
    }

    const auto *P = llvm::find_if(Pairs, [&](const auto &P) {
      return isLD8(*SetLo, P.Lo, Z80::L) && isLD8(*SetHi, P.Hi, Z80::H);
    });
    if (P == std::end(Pairs) ||
        !isRegDeadAfter(std::next(SetHi), MBB, TRI, Z80::HL)) {
      ++MII;
      continue;
    }

    LLVM_DEBUG(dbgs() << "  Reload straight into pair: " << *SetLo);
    Z80::buildLoadHL(MBB, LoadHi, LoadHi->getDebugLoc(), *TII, P->Hi)
        .cloneMemRefs(*LoadHi);
    Z80::buildLD8(MBB, LoadHi, LoadHi->getDebugLoc(), *TII, P->Lo, Z80::A);
    MII = std::next(SetHi);
    MBB.erase(LoadHi, MII);
    ++NumPairReloads;
    Changed = true;
  }
  if (Changed)
    recomputeLivenessFlags(MBB);
  return Changed;
}

// LD A,r; AND n; LD r,A is four bytes for what RES does in two, when the
// mask clears a single bit. RES writes no flags, so the ones the AND wrote
// have to be dead.
static bool foldSingleBitMask(MachineBasicBlock &MBB,
                              const TargetInstrInfo *TII,
                              const TargetRegisterInfo *TRI) {
  bool Changed = false;
  for (auto MII = MBB.begin(); MII != MBB.end();) {
    auto And = MII++;
    if (And->getOpcode() != Z80::AND_n || !And->getOperand(0).isImm())
      continue;
    const unsigned Cleared = ~And->getOperand(0).getImm() & 0xFF;
    if (!isPowerOf2_32(Cleared))
      continue;
    if (And == MBB.begin())
      continue;

    auto In = std::prev(And);
    auto Out = std::next(And);
    if (Out == MBB.end() || !isLD8(*In) || !isLD8(*Out))
      continue;
    Register Reg = In->getOperand(1).getReg();
    if (In->getOperand(0).getReg() != Z80::A || Reg == Z80::A ||
        Out->getOperand(0).getReg() != Reg ||
        Out->getOperand(1).getReg() != Z80::A)
      continue;

    auto After = std::next(Out);
    if (!isRegDeadAfter(After, MBB, TRI, Z80::FLAGS) ||
        !isRegDeadAfter(After, MBB, TRI, Z80::A))
      continue;

    LLVM_DEBUG(dbgs() << "  Single-bit mask through A: " << *And);
    BuildMI(MBB, In, And->getDebugLoc(), TII->get(Z80::RES_b_r), Reg)
        .addImm(Log2_32(Cleared))
        .addReg(Reg);
    MBB.erase(In);
    MBB.erase(And);
    MII = MBB.erase(Out);
    ++NumSingleBitMasks;
    Changed = true;
  }
  return Changed;
}

// --- Increment a register where it lives ---
//
// LD A,r; INC/DEC A; LD r,A round-trips through A for a plain increment.
// INC r produces the same value and the same flags, in one byte. Valid on
// both targets; needs A dead afterward, since the round trip left the new
// value in A as a side effect.
static bool directIncDec(MachineBasicBlock &MBB, const TargetInstrInfo *TII,
                         const TargetRegisterInfo *TRI) {
  bool Changed = false;
  for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
    auto Next = std::next(MII);
    Register R = getLD8Src(*MII, Z80::A);
    if (!R || Next == MIE) {
      MII = Next;
      continue;
    }
    bool IsInc = isIncDec8(*Next, Z80::INC_r, Z80::A);
    if (!IsInc && !isIncDec8(*Next, Z80::DEC_r, Z80::A)) {
      MII = Next;
      continue;
    }
    // A register the field cannot name has no INC r form to fold into. A
    // itself does, so a round trip through A of A is folded like any other;
    // it is only the index registers this turns away.
    if (!Z80::isEncodableGR8(R)) {
      MII = Next;
      continue;
    }
    auto Third = std::next(Next);
    if (Third == MIE || getLD8Dst(*Third, Z80::A) != R) {
      MII = Next;
      continue;
    }
    auto After = std::next(Third);
    if (!isRegDeadAfter(After, MBB, TRI, Z80::A)) {
      MII = Next;
      continue;
    }
    LLVM_DEBUG(dbgs() << "  Direct inc/dec: " << *MII);
    Z80::buildIncDec8(MBB, MII, MII->getDebugLoc(), *TII,
                      IsInc ? Z80::INC_r : Z80::DEC_r, R);
    MBB.erase(MII);
    MBB.erase(Next);
    MBB.erase(Third);
    MII = After;
    ++NumIncDec;
    Changed = true;
  }
  if (Changed)
    recomputeLivenessFlags(MBB);
  return Changed;
}

// --- Peephole #116 / #117: i16 EQ/NE byte-XOR -> AND A; SBC HL,rr ---
//
// Variable-RHS i16 EQ/NE compare-and-branch is emitted by ISel as a
// 6-byte byte-level XOR sequence:
//
//   LD A, X        ; X = sub_{hi,lo} of QPair (e.g. D of DE)
//   XOR R1         ; R1 = sub_{hi,lo} of PPair (e.g. B of BC)
//   LD T, A        ; T = some GR8 scratch (e.g. H)
//   LD A, Y        ; Y = the other half of QPair (e.g. E of DE)
//   XOR R2         ; R2 = the other half of PPair (e.g. C of BC)
//   OR T           ; combine; Z=1 iff QPair == PPair
//   JR Z|NZ / JP Z|NZ
//
// Worked example 1 (Peephole #116, clean HL case from issue-117 @eq_hl_de):
//   Input MIR:
//     $a = LD_r_r $h
//     XOR_r $d, implicit-def $a, implicit-def $flags, implicit $a
//     $b = LD_r_r $a
//     $a = LD_r_r $l
//     XOR_r $e, implicit-def $a, implicit-def $flags, implicit $a
//     OR_r $b, implicit-def $a, implicit-def $flags, implicit $a
//     JR_Z_e %bb.1, implicit $flags
//   Here QPair = HL, PPair = DE, T = B.
//   Output:
//     AND_r undef $a
//     SBC_HL_rr $de
//     JR_Z_e %bb.1
//   Shrinks sequence from 6 bytes (28 T) to 3 bytes (19 T) -> -3 B, -9 T.
//
// Worked example 2 (Peephole #117, neither-in-HL case from issue-117 @eq_bc_de):
//   Input MIR:
//     $a = LD_r_r $b
//     XOR_r $d, implicit-def $a, implicit-def $flags, implicit $a
//     $h = LD_r_r $a
//     $a = LD_r_r $c
//     XOR_r $e, implicit-def $a, implicit-def $flags, implicit $a
//     OR_r $h, implicit-def $a, implicit-def $flags, implicit $a
//     JR_Z_e %bb.1, implicit $flags
//   Here QPair = BC, PPair = DE, T = H.
//   Output:
//     PUSH_BC
//     POP_HL
//     AND_r undef $a
//     SBC_HL_rr $de
//     JR_Z_e %bb.1
//   Replaces 6 B XOR sequence with PUSH + POP + AND A + SBC (5 B) -> -1 B.
//   Requires HL to be dead at entry so POP HL does not clobber a live value.
//
// Soundness guards:
//   - Branch after sequence must only consume Z flag (JR/JP Z/NZ).
//   - A, T, and HL must all be dead after the branch.
//   - In the neither-in-HL case, H and L must both be LQR_Dead at I1.
static bool optimizeI16CompareByteXOR(MachineBasicBlock &MBB,
                                      const TargetInstrInfo *TII,
                                      const TargetRegisterInfo *TRI,
                                      const Z80Subtarget &STI) {
  // Only valid for Z80; SM83 lacks SBC HL,rr.
  if (!STI.hasZ80() || STI.hasSM83())
    return false;

  auto isAluReg = [](const MachineInstr &MI, unsigned Opc) -> Register {
    return MI.getOpcode() == Opc ? MI.getOperand(0).getReg() : Register();
  };

  auto pairOf = [](Register R) -> Register {
    switch (R) {
    case Z80::B: case Z80::C: return Z80::BC;
    case Z80::D: case Z80::E: return Z80::DE;
    case Z80::H: case Z80::L: return Z80::HL;
    default: return Register();
    }
  };

  auto isHiByte = [](Register R) -> bool {
    return R == Z80::B || R == Z80::D || R == Z80::H;
  };

  bool Changed = false;
  for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
    // I1: LD A, X
    Register X = getLD8Src(*MII, Z80::A);
    // skip instructions that are not a copy of an 8-bit GP reg into A
    if (!X || !pairOf(X)) { ++MII; continue; }

    auto I1 = MII;
    auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
    // skip if block ends before sequence completes
    if (I2 == MIE) { ++MII; continue; }

    // I2: XOR R1
    Register R1 = isAluReg(*I2, Z80::XOR_r);
    // skip if I2 is not XOR r with an 8-bit GP reg
    if (!R1 || !pairOf(R1)) { ++MII; continue; }

    auto I3 = MBB.SkipPHIsLabelsAndDebug(std::next(I2));
    // skip if block ends before sequence completes
    if (I3 == MIE) { ++MII; continue; }

    // I3: LD T, A
    Register T = getLD8Dst(*I3, Z80::A);
    // skip if I3 does not spill intermediate XOR result from A into a scratch reg
    if (!T || !pairOf(T)) { ++MII; continue; }

    auto I4 = MBB.SkipPHIsLabelsAndDebug(std::next(I3));
    // skip if block ends before sequence completes
    if (I4 == MIE) { ++MII; continue; }

    // I4: LD A, Y
    Register Y = getLD8Src(*I4, Z80::A);
    // skip if I4 is not loading the other half of the first pair into A
    if (!Y || !pairOf(Y)) { ++MII; continue; }

    auto I5 = MBB.SkipPHIsLabelsAndDebug(std::next(I4));
    // skip if block ends before sequence completes
    if (I5 == MIE) { ++MII; continue; }

    // I5: XOR R2
    Register R2 = isAluReg(*I5, Z80::XOR_r);
    // skip if I5 is not XOR r with the other half of the second pair
    if (!R2 || !pairOf(R2)) { ++MII; continue; }

    auto I6 = MBB.SkipPHIsLabelsAndDebug(std::next(I5));
    // skip if block ends before sequence completes
    if (I6 == MIE) { ++MII; continue; }

    // I6: OR T
    Register OrSrc = isAluReg(*I6, Z80::OR_r);
    // skip if I6 does not combine with the scratch register T
    if (OrSrc != T) { ++MII; continue; }

    // Structural pairing validation:
    // (X, Y) must form one 16-bit register pair (QPair).
    // (R1, R2) must form the other 16-bit register pair (PPair).
    // The (hi, lo) polarities must match: both high or both low.
    Register QPair = pairOf(X);
    Register PPair = pairOf(R1);
    // skip if halves do not form consistent 16-bit pairs
    if (pairOf(Y) != QPair || pairOf(R2) != PPair) { ++MII; continue; }
    // skip if byte polarities don't correspond (e.g. X is hi but R1 is lo)
    if (isHiByte(X) != isHiByte(R1) || isHiByte(Y) != isHiByte(R2)) { ++MII; continue; }
    // skip if X and Y are the same byte half
    if (isHiByte(X) == isHiByte(Y)) { ++MII; continue; }

    // Scratch register T must not overwrite operands still needed at I4/I5.
    // T can legally be X or R1 (which have already been read at I1/I2), but
    // must not be Y or R2.
    if (T == Y || T == R2) { ++MII; continue; }

    // Identify which pair is HL and which is the SBC operand (BC or DE)
    Register SbcRR;
    bool NeedMoveToHL = false;
    Register MoveToHL;
    if (QPair == Z80::HL && (PPair == Z80::BC || PPair == Z80::DE)) {
      SbcRR = PPair;
    } else if (PPair == Z80::HL && (QPair == Z80::BC || QPair == Z80::DE)) {
      SbcRR = QPair;
    } else if ((QPair == Z80::BC || QPair == Z80::DE) &&
               (PPair == Z80::BC || PPair == Z80::DE) &&
               QPair != PPair) {
      // #117: Neither side is HL, but both are BC/DE. Move QPair to HL via PUSH/POP.
      NeedMoveToHL = true;
      MoveToHL = QPair;
      SbcRR = PPair;
    } else {
      // skip when neither pairing satisfies Z80 SBC HL,rr constraints
      ++MII; continue;
    }

    // I7 must consume ONLY the Z flag (JR/JP Z/NZ).
    auto I7 = MBB.SkipPHIsLabelsAndDebug(std::next(I6));
    // skip if there is no branch instruction following the comparison
    if (I7 == MIE) { ++MII; continue; }
    unsigned BrOpc = I7->getOpcode();
    // skip if the branch relies on flags other than Z (SBC sets C/S/V differently than XOR)
    if (BrOpc != Z80::JR_Z_e && BrOpc != Z80::JR_NZ_e &&
        BrOpc != Z80::JP_Z_nn && BrOpc != Z80::JP_NZ_nn) {
      ++MII; continue;
    }

    // After the branch, A, T, and HL must all be dead.
    auto AfterBr = MBB.SkipPHIsLabelsAndDebug(std::next(I7));
    // skip if A is live past the branch (original sequence set A; rewrite leaves it dead/undef)
    if (!isRegDeadAfter(AfterBr, MBB, TRI, Z80::A)) { ++MII; continue; }
    // skip if scratch register T is live past the branch
    if (!isRegDeadAfter(AfterBr, MBB, TRI, T)) { ++MII; continue; }
    // skip if HL is live past the branch (SBC HL,rr clobbers HL with difference)
    if (!isRegDeadAfter(AfterBr, MBB, TRI, Z80::HL)) { ++MII; continue; }

    // In the neither-in-HL path, HL must be dead at I1 so POP HL does not overwrite a live value.
    if (NeedMoveToHL) {
      auto HQ = MBB.computeRegisterLiveness(TRI, Z80::H, I1);
      auto LQ = MBB.computeRegisterLiveness(TRI, Z80::L, I1);
      // skip if either H or L could be live at the comparison site
      if (HQ != MachineBasicBlock::LQR_Dead || LQ != MachineBasicBlock::LQR_Dead) {
        ++MII; continue;
      }
    }

    LLVM_DEBUG(dbgs() << "  i16 EQ/NE byte-XOR -> "
                      << (NeedMoveToHL ? "PUSH/POP HL; " : "")
                      << "SBC HL," << TRI->getName(SbcRR) << "\n");

    DebugLoc DL = I1->getDebugLoc();
    if (NeedMoveToHL) {
      unsigned PushOpc = Z80::getPushOpcode(MoveToHL);
      BuildMI(MBB, *I1, DL, TII->get(PushOpc));
      BuildMI(MBB, *I1, DL, TII->get(Z80::POP_HL));
    }
    Z80::markUndefUse(
        BuildMI(MBB, *I1, DL, TII->get(Z80::AND_r)).addReg(Z80::A),
        Z80::A);
    BuildMI(MBB, *I1, DL, TII->get(Z80::SBC_HL_rr)).addReg(SbcRR);

    // Erase I1..I6 and advance iterator.
    MII = std::next(I6);
    I1->eraseFromParent();
    I2->eraseFromParent();
    I3->eraseFromParent();
    I4->eraseFromParent();
    I5->eraseFromParent();
    I6->eraseFromParent();
    ++NumI16ByteXorToSbc;
    Changed = true;
  }

  return Changed;
}

// --- Peephole: in-memory INC/DEC ---
//
// Replaces:
//   LD A, (addr)        ; 3 B
//   INC A / DEC A       ; 1 B
//   LD (addr), A        ; 3 B (was 7 B total with direct addressing)
// with:
//   LD HL, addr         ; 3 B
//   INC (HL) / DEC (HL) ; 1 B (4 B total -> saves 3 B vs 7 B)
//
// Worked example (from inmem-incdec-positive.ll @bump_counter):
//   Input MIR:
//     $a = LD_A_nnind @counter
//     $a = INC_r killed $a, implicit-def $flags
//     LD_nnind_A @counter, killed $a
//   Output:
//     $hl = LD_rr_nn @counter
//     INC_HLind implicit $hl, implicit-def $flags
//
// Soundness guards:
//   - Addresses in load and store must match identically (same symbol + offset).
//   - A must be dead after the store (the rewrite never writes to A).
//   - H and L must both be dead before the load (LD HL clobbers HL).
//   - HL must be dead after the store.
static bool optimizeInMemoryIncDec(MachineBasicBlock &MBB,
                                   const TargetInstrInfo *TII,
                                   const TargetRegisterInfo *TRI,
                                   const Z80Subtarget &STI) {
  if (!STI.hasZ80())
    return false;

  // In an interrupt handler, HL is callee-saved per Z80_Interrupt_CSR
  // (Z80CallingConv.td). Rewriting `ld a,(nn); inc a; ld (nn),a` to
  // `ld hl,nn; inc (hl)` introduces a new HL clobber after PEI has
  // already computed the callee-saved-spill set, so HL is never pushed
  // and the interrupted code's HL value is silently corrupted on RETI.
  // Bail unless HL is already in the prologue's saved-regs list
  // (ravn/llvm-z80#341).
  const MachineFunction *MF = MBB.getParent();
  if (MF->getFunction().hasFnAttribute("interrupt")) {
    bool HLAlreadySaved = false;
    for (const CalleeSavedInfo &CSI :
         MF->getFrameInfo().getCalleeSavedInfo()) {
      if (CSI.getReg() == Z80::HL) {
        HLAlreadySaved = true;
        break;
      }
    }
    if (!HLAlreadySaved)
      return false;
  }

  bool Changed = false;
  for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
    // I0: LD A, (addr)
    if (MII->getOpcode() != Z80::LD_A_nnind) { ++MII; continue; }
    auto I0 = MII;
    const MachineOperand &LoadAddr = I0->getOperand(0);
    // skip if address is not a global or external symbol
    if (!LoadAddr.isGlobal() && !LoadAddr.isSymbol()) { ++MII; continue; }

    // I1: INC A or DEC A
    auto I1 = MBB.SkipPHIsLabelsAndDebug(std::next(I0));
    // skip if block ends before sequence completes
    if (I1 == MIE) { ++MII; continue; }
    bool IsInc = (I1->getOpcode() == Z80::INC_r && I1->getOperand(0).getReg() == Z80::A);
    bool IsDec = (I1->getOpcode() == Z80::DEC_r && I1->getOperand(0).getReg() == Z80::A);
    // skip if I1 is not INC A or DEC A
    if (!IsInc && !IsDec) { ++MII; continue; }

    // I2: LD (addr), A
    auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
    // skip if block ends before sequence completes
    if (I2 == MIE) { ++MII; continue; }
    if (I2->getOpcode() != Z80::LD_nnind_A) { ++MII; continue; }
    const MachineOperand &StoreAddr = I2->getOperand(0);

    // Verify addresses match
    bool AddrMatch = false;
    if (LoadAddr.isGlobal() && StoreAddr.isGlobal()) {
      AddrMatch = (LoadAddr.getGlobal() == StoreAddr.getGlobal() &&
                   LoadAddr.getOffset() == StoreAddr.getOffset());
    } else if (LoadAddr.isSymbol() && StoreAddr.isSymbol()) {
      AddrMatch = (StringRef(LoadAddr.getSymbolName()) == StoreAddr.getSymbolName() &&
                   LoadAddr.getOffset() == StoreAddr.getOffset());
    }
    // skip if load and store targets differ
    if (!AddrMatch) { ++MII; continue; }

    auto AfterStore = MBB.SkipPHIsLabelsAndDebug(std::next(I2));

    // A must be dead after the store
    // skip if A is live after store (INC/DEC (HL) leaves A untouched)
    if (!isRegDeadAfter(AfterStore, MBB, TRI, Z80::A)) { ++MII; continue; }

    // H and L must be dead before I0 so LD HL doesn't clobber a live value
    auto HQ = MBB.computeRegisterLiveness(TRI, Z80::H, I0);
    auto LQ = MBB.computeRegisterLiveness(TRI, Z80::L, I0);
    // skip if H or L is live before the load
    if (HQ != MachineBasicBlock::LQR_Dead || LQ != MachineBasicBlock::LQR_Dead) {
      ++MII; continue;
    }

    // HL must be dead after the store
    // skip if HL is live after the store
    if (!isRegDeadAfter(AfterStore, MBB, TRI, Z80::HL)) { ++MII; continue; }

    LLVM_DEBUG(dbgs() << "  in-memory " << (IsInc ? "INC" : "DEC")
                      << " (HL) for " << LoadAddr << "\n");

    DebugLoc DL = I0->getDebugLoc();
    BuildMI(MBB, *I0, DL, TII->get(Z80::LD_rr_nn), Z80::HL).add(LoadAddr);
    unsigned IncDecOpc = IsInc ? Z80::INC_HLind : Z80::DEC_HLind;
    BuildMI(MBB, *I0, DL, TII->get(IncDecOpc));

    MII = std::next(I2);
    I0->eraseFromParent();
    I1->eraseFromParent();
    I2->eraseFromParent();
    ++NumInMemIncDec;
    Changed = true;
  }

  return Changed;
}

// --- Peephole #147: `mem |= 1<<N` / `mem &= ~(1<<N)` → SET/RES n,(HL) ---
//
// Three-instruction sequence:
//   LD A, (addr)            (3 B)
//   {OR, AND} K             (2 B)
//   LD (addr), A            (3 B)   ; same address as load
// For single-bit ops (popcount(K)==1 for OR, popcount(~K & 0xFF)==1 for AND),
// replace with:
//   LD HL, addr             (3 B)
//   {SET, RES} b, (HL)      (2 B)
// Total: 5 B vs 8 B (saves 3 B).
// For two-bit ops, emit two SET/RES: 3 + 2*2 = 7 B vs 8 B (saves 1 B).
//
// Worked example (from issue-147-set-res-mem.ll @set_bit_0):
//   Input:
//     LD_A_nnind @flag
//     OR_n 1
//     LD_nnind_A @flag
//   Output:
//     $hl = LD_rr_nn @flag
//     SET_0_HLind implicit $hl
//
// Soundness guards:
//   - A must be dead after the store (we don't preserve OR/AND result in A).
//   - H and L must be dead before the load (LD HL clobbers HL).
//   - HL must be dead after the store.
//   - Any intervening instructions must not access memory, touch HL, or write A.
//     If an intervening instruction reads A, emit `LD A,(HL)` right after
//     `LD HL,addr` to preserve A for the reader (#152).
static bool optimizeInMemoryBitSetRes(MachineBasicBlock &MBB,
                                      const TargetInstrInfo *TII,
                                      const TargetRegisterInfo *TRI,
                                      const Z80Subtarget &STI) {
  if (!STI.hasZ80())
    return false;

  // See optimizeInMemoryIncDec above: in an interrupt handler HL is
  // callee-saved per Z80_Interrupt_CSR; introducing a new HL def after
  // PEI silently drops the required push (ravn/llvm-z80#341).
  const MachineFunction *MF = MBB.getParent();
  if (MF->getFunction().hasFnAttribute("interrupt")) {
    bool HLAlreadySaved = false;
    for (const CalleeSavedInfo &CSI :
         MF->getFrameInfo().getCalleeSavedInfo()) {
      if (CSI.getReg() == Z80::HL) {
        HLAlreadySaved = true;
        break;
      }
    }
    if (!HLAlreadySaved)
      return false;
  }

  static const unsigned SetOps[8] = {
      Z80::SET_0_HLind, Z80::SET_1_HLind, Z80::SET_2_HLind, Z80::SET_3_HLind,
      Z80::SET_4_HLind, Z80::SET_5_HLind, Z80::SET_6_HLind, Z80::SET_7_HLind,
  };
  static const unsigned ResOps[8] = {
      Z80::RES_0_HLind, Z80::RES_1_HLind, Z80::RES_2_HLind, Z80::RES_3_HLind,
      Z80::RES_4_HLind, Z80::RES_5_HLind, Z80::RES_6_HLind, Z80::RES_7_HLind,
  };

  auto sameAddrOp = [](const MachineOperand &A, const MachineOperand &B) -> bool {
    if (A.isGlobal() && B.isGlobal())
      return A.getGlobal() == B.getGlobal() && A.getOffset() == B.getOffset();
    if (A.isSymbol() && B.isSymbol())
      return StringRef(A.getSymbolName()) == B.getSymbolName() &&
             A.getOffset() == B.getOffset();
    if (A.isMCSymbol() && B.isMCSymbol())
      return A.getMCSymbol() == B.getMCSymbol() &&
             A.getOffset() == B.getOffset();
    if (A.isImm() && B.isImm())
      return A.getImm() == B.getImm();
    return false;
  };

  bool Changed = false;
  for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
    auto LdIt = MII++;
    // Match LD A,(addr)
    if (LdIt->getOpcode() != Z80::LD_A_nnind)
      continue;
    if (MII == MIE)
      continue;

    // Search forward for OR_n or AND_n
    auto OpIt = MII;
    bool BailIntervening = false;
    bool HadAReader = false;
    while (OpIt != MIE) {
      unsigned O = OpIt->getOpcode();
      if (O == Z80::OR_n || O == Z80::AND_n)
        break;
      // Bail on branches, calls, or unmodeled side effects
      if (OpIt->isTerminator() || OpIt->isCall() || OpIt->hasUnmodeledSideEffects()) {
        BailIntervening = true;
        break;
      }
      // Bail on any memory access
      if (!OpIt->memoperands_empty() || OpIt->mayLoad() || OpIt->mayStore()) {
        BailIntervening = true;
        break;
      }
      for (const MachineOperand &MO : OpIt->operands()) {
        if (!MO.isReg() || !MO.getReg().isPhysical())
          continue;
        // Bail if intervening instruction uses or defines HL
        if (TRI->regsOverlap(MO.getReg(), Z80::HL)) {
          BailIntervening = true;
          break;
        }
        if (TRI->regsOverlap(MO.getReg(), Z80::A)) {
          // Bail if intervening instruction overwrites A
          if (MO.isDef()) {
            BailIntervening = true;
            break;
          }
          HadAReader = true;
        }
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

    // Next instruction after OR/AND must be the matching store LD (addr),A
    auto StIt = MBB.SkipPHIsLabelsAndDebug(std::next(OpIt));
    if (StIt == MIE || StIt->getOpcode() != Z80::LD_nnind_A)
      continue;
    if (!sameAddrOp(LdIt->getOperand(0), StIt->getOperand(0)))
      continue;

    // A must be dead after the store
    auto AfterSt = MBB.SkipPHIsLabelsAndDebug(std::next(StIt));
    if (!isRegDeadAfter(AfterSt, MBB, TRI, Z80::A))
      continue;

    // H and L must be dead before the load
    auto HQ = MBB.computeRegisterLiveness(TRI, Z80::H, LdIt);
    auto LQ = MBB.computeRegisterLiveness(TRI, Z80::L, LdIt);
    if (HQ != MachineBasicBlock::LQR_Dead || LQ != MachineBasicBlock::LQR_Dead)
      continue;

    // HL must be dead after the store
    if (!isRegDeadAfter(AfterSt, MBB, TRI, Z80::HL))
      continue;

    // Compute effective bitmask and popcount
    int64_t K = OpIt->getOperand(0).getImm() & 0xFF;
    unsigned EffMask = IsOr ? (unsigned)K : ((~(unsigned)K) & 0xFF);
    unsigned Pop = llvm::popcount(EffMask);
    if (Pop == 0 || Pop > 2)
      continue;
    // With an intervening reader, Pop 2 breaks even on size but adds latency, so skip
    if (HadAReader && Pop != 1)
      continue;

    LLVM_DEBUG(dbgs() << "  in-memory bit " << (IsOr ? "SET" : "RES")
                      << " (HL) for " << LdIt->getOperand(0) << "\n");

    DebugLoc DL = LdIt->getDebugLoc();
    BuildMI(MBB, *LdIt, DL, TII->get(Z80::LD_rr_nn), Z80::HL)
        .add(LdIt->getOperand(0));
    if (HadAReader)
      BuildMI(MBB, *LdIt, DL, TII->get(Z80::LD_r_HLind), Z80::A);

    const auto *Table = IsOr ? SetOps : ResOps;
    for (unsigned Bit = 0; Bit < 8; ++Bit) {
      if (EffMask & (1u << Bit))
        BuildMI(MBB, *OpIt, DL, TII->get(Table[Bit]));
    }

    MII = std::next(StIt);
    LdIt->eraseFromParent();
    OpIt->eraseFromParent();
    StIt->eraseFromParent();
    ++NumInMemBitSetRes;
    Changed = true;
  }

  return Changed;
}

// --- Peephole: DEC B; JR NZ → DJNZ (Z80 only) ---
//
// DJNZ (2 B, 13/8 T) is a hardware loop instruction that decrements B and
// branches if B != 0. Replaces DEC B (1 B, 4 T) + JR NZ (2 B, 12/7 T) = 3 B, 16/11 T.
// Also folds `DEC A; LD B, A; [OR A;] JR NZ → DJNZ` (saves 2-3 B).
//
// Worked example (from djnz.ll @delay):
//   Input:
//     $b = DEC_r $b
//     JR_NZ_e %bb.1
//   Output:
//     DJNZ_e %bb.1
//
// Soundness guards:
//   - Only valid on Z80 (SM83 lacks DJNZ).
//   - FLAGS must be dead after JR NZ (DJNZ preserves flags; DEC sets them).
//   - In the DEC A; LD B,A variant: A must be dead after branch, and B must not
// --- Peephole: DEC/INC r; [LD A, r;] OR A; JR NZ/Z -> DEC/INC r; JR NZ/Z ---
//
// WHAT: Eliminates redundant `LD A, r; OR A` (or `OR A` when r is A) that re-tests
// an 8-bit register immediately after `DEC r` or `INC r`.
//
// WHY: Z80 `DEC r` and `INC r` already update the Zero flag according to the
// result. When ISel lowers `(icmp ne (add/sub r, 1), 0)` without peephole folding,
// it copies the result to accumulator A and emits `OR A` to set ZF. If only ZF/NZ
// is observed by the branch, the reload into A and `OR A` are completely redundant.
// Eliminating them saves 2 bytes (1 B `LD A, r` + 1 B `OR A`) and frees A.
// Crucially, for r == B and JR NZ, this exposes `DEC B; JR NZ` directly to
// optimizeDJNZ, which then emits a 2-byte DJNZ instruction.
//
// Worked example (from delay loop in rom.c / issue #330):
//   Input:
//     $c = DEC_r killed $c, implicit-def $flags
//     $a = LD_r_r killed $c
//     OR_r $a, implicit-def dead $a, implicit-def $flags, implicit $a
//     JR_NZ_e %bb.3, implicit $flags
//   Output:
//     $c = DEC_r killed $c, implicit-def $flags
//     JR_NZ_e %bb.3, implicit $flags
//
// Worked example with B (folded to DJNZ downstream):
//   Input:
//     $b = DEC_r killed $b, implicit-def $flags
//     $a = LD_r_r killed $b
//     OR_r $a, implicit-def dead $a, implicit-def $flags, implicit $a
//     JR_NZ_e %bb.4, implicit $flags
//   Intermediate:
//     $b = DEC_r killed $b, implicit-def $flags
//     JR_NZ_e %bb.4, implicit $flags
//   Downstream optimizeDJNZ Output:
//     DJNZ_e %bb.4
//
// Soundness guards:
//   - Operates on encodable GR8 registers (A, B, C, D, E, H, L).
//   - Branch opcode must test Z or NZ condition (JR_NZ_e, JR_Z_e, JP_NZ_nn, JP_Z_nn).
//   - A must be dead after branch (since `LD A, r` is deleted).
//   - FLAGS must be dead after branch (DEC/INC updates S,Z,P/V,H while OR A clears C,N,H).
static bool optimizeRedundantTestAfterIncDec(MachineBasicBlock &MBB,
                                             const TargetInstrInfo *TII,
                                             const TargetRegisterInfo *TRI) {
  bool Changed = false;
  const auto MIE = MBB.end();

  for (auto MII = MBB.begin(); MII != MIE;) {
    unsigned Opc = MII->getOpcode();
    // Only applies to 8-bit INC/DEC instructions
    if (Opc != Z80::DEC_r && Opc != Z80::INC_r) {
      ++MII;
      continue;
    }

    Register R = MII->getOperand(0).getReg();
    // Skip if register cannot be encoded in standard 8-bit field
    if (!R.isValid() || !Z80::isEncodableGR8(R)) {
      ++MII;
      continue;
    }

    auto I1 = MII;
    auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
    if (I2 == MIE) {
      ++MII;
      continue;
    }

    if (R != Z80::A) {
      // Case 1: DEC/INC r; LD A, r; OR A; JR NZ/Z
      if (!isLD8(*I2, Z80::A, R)) {
        ++MII;
        continue;
      }
      auto I3 = MBB.SkipPHIsLabelsAndDebug(std::next(I2));
      if (I3 == MIE || !isAlu8(*I3, Z80::OR_r, Z80::A)) {
        ++MII;
        continue;
      }
      auto IBranch = MBB.SkipPHIsLabelsAndDebug(std::next(I3));
      if (IBranch == MIE) {
        ++MII;
        continue;
      }
      unsigned BranchOpc = IBranch->getOpcode();
      // Ensure branch only tests Z or NZ condition
      if (BranchOpc != Z80::JR_NZ_e && BranchOpc != Z80::JR_Z_e &&
          BranchOpc != Z80::JP_NZ_nn && BranchOpc != Z80::JP_Z_nn) {
        ++MII;
        continue;
      }
      // Soundness: A and FLAGS must not be read after branch
      if (!isRegDeadAfter(std::next(IBranch), MBB, TRI, Z80::A) ||
          !isRegDeadAfter(std::next(IBranch), MBB, TRI, Z80::FLAGS)) {
        ++MII;
        continue;
      }

      LLVM_DEBUG(dbgs() << "  Elide redundant LD A," << printReg(R, TRI)
                        << "; OR A after INC/DEC\n");
      I3->eraseFromParent();
      I2->eraseFromParent();
      Changed = true;
      MII = std::next(I1);
    } else {
      // Case 2: DEC/INC A; OR A; JR NZ/Z
      if (!isAlu8(*I2, Z80::OR_r, Z80::A)) {
        ++MII;
        continue;
      }
      auto IBranch = MBB.SkipPHIsLabelsAndDebug(std::next(I2));
      if (IBranch == MIE) {
        ++MII;
        continue;
      }
      unsigned BranchOpc = IBranch->getOpcode();
      // Ensure branch only tests Z or NZ condition
      if (BranchOpc != Z80::JR_NZ_e && BranchOpc != Z80::JR_Z_e &&
          BranchOpc != Z80::JP_NZ_nn && BranchOpc != Z80::JP_Z_nn) {
        ++MII;
        continue;
      }
      // Soundness: FLAGS must not be read after branch
      if (!isRegDeadAfter(std::next(IBranch), MBB, TRI, Z80::FLAGS)) {
        ++MII;
        continue;
      }

      LLVM_DEBUG(dbgs() << "  Elide redundant OR A after INC/DEC A\n");
      I2->eraseFromParent();
      Changed = true;
      MII = std::next(I1);
    }
  }

  return Changed;
}

static bool optimizeDJNZ(MachineBasicBlock &MBB,
                         const TargetInstrInfo *TII,
                         const TargetRegisterInfo *TRI,
                         const Z80Subtarget &STI) {
  if (!STI.hasZ80())
    return false;

  bool Changed = false;
  const auto MIE = MBB.end();

  // Pattern 1: DEC B; JR NZ → DJNZ
  for (auto MII = MBB.begin(); MII != MIE;) {
    if (!isIncDec8(*MII, Z80::DEC_r, Z80::B)) {
      ++MII;
      continue;
    }
    auto NextIt = MBB.SkipPHIsLabelsAndDebug(std::next(MII));
    if (NextIt == MIE || NextIt->getOpcode() != Z80::JR_NZ_e) {
      ++MII;
      continue;
    }
    // skip if branch target is not a valid MBB
    if (!NextIt->getOperand(0).isMBB()) {
      ++MII;
      continue;
    }
    // skip if FLAGS is live after branch (DJNZ preserves flags, DEC B sets them)
    if (!isRegDeadAfter(std::next(NextIt), MBB, TRI, Z80::FLAGS)) {
      ++MII;
      continue;
    }
    MachineBasicBlock *TargetMBB = NextIt->getOperand(0).getMBB();
    DebugLoc DL = MII->getDebugLoc();
    LLVM_DEBUG(dbgs() << "  DEC B; JR NZ -> DJNZ\n");
    auto EraseEnd = std::next(NextIt);
    MII = MBB.erase(MII, EraseEnd);
    BuildMI(MBB, MII, DL, TII->get(Z80::DJNZ_e)).addMBB(TargetMBB);
    ++NumDjnz;
    Changed = true;
  }

  // Pattern 2: DEC A; LD B, A; [OR A;] JR NZ → DJNZ
  for (auto MII = MBB.begin(); MII != MIE;) {
    if (!isIncDec8(*MII, Z80::DEC_r, Z80::A)) {
      ++MII;
      continue;
    }
    auto I1 = MII;
    auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
    if (I2 == MIE || !isLD8(*I2, Z80::B, Z80::A)) {
      ++MII;
      continue;
    }
    auto I3 = MBB.SkipPHIsLabelsAndDebug(std::next(I2));
    if (I3 == MIE) {
      ++MII;
      continue;
    }
    auto IBranch = I3;
    if (IBranch->getOpcode() == Z80::OR_r &&
        IBranch->getOperand(0).getReg() == Z80::A) {
      IBranch = MBB.SkipPHIsLabelsAndDebug(std::next(IBranch));
      if (IBranch == MIE) {
        ++MII;
        continue;
      }
    }
    if (IBranch->getOpcode() != Z80::JR_NZ_e || !IBranch->getOperand(0).isMBB()) {
      ++MII;
      continue;
    }
    // skip if A is live after branch (DJNZ doesn't update A)
    if (!isRegDeadAfter(std::next(IBranch), MBB, TRI, Z80::A)) {
      ++MII;
      continue;
    }
    // skip if FLAGS is live after branch (DJNZ preserves flags, DEC A sets them)
    if (!isRegDeadAfter(std::next(IBranch), MBB, TRI, Z80::FLAGS)) {
      ++MII;
      continue;
    }
    // ravn/llvm-z80#185: skip if B is modified in body before I1 (LD B,A is essential reload)
    bool BClobberedInBody = false;
    for (auto It = MBB.begin(); It != I1; ++It) {
      if (It->modifiesRegister(Z80::B, TRI)) {
        BClobberedInBody = true;
        break;
      }
    }
    if (BClobberedInBody) {
      ++MII;
      continue;
    }

    MachineBasicBlock *TargetMBB = IBranch->getOperand(0).getMBB();
    DebugLoc DL = I1->getDebugLoc();
    LLVM_DEBUG(dbgs() << "  DEC A; LD B,A; [OR A;] JR NZ -> DJNZ\n");
    auto EraseEnd = std::next(IBranch);
    MII = MBB.erase(I1, EraseEnd);
    BuildMI(MBB, MII, DL, TII->get(Z80::DJNZ_e)).addMBB(TargetMBB);
    ++NumDjnz;
    Changed = true;
  }

  return Changed;
}

// --- Peephole: BSS spill/reload → PUSH/POP across CALLs and register pressure ---
//
// WHAT: In +static-frame / +static-stack code, register-allocator spills use direct
// memory addressing to function frame slots in BSS:
//   LD (slot), rr       (3-4 bytes, 13-20 T)
//   ... [intervening code / CALLs] ...
//   LD rr, (slot)       (3-4 bytes, 13-20 T)
// Total cost per spill/reload pair: 6-8 bytes, 26-40 T.
//
// When the stack depth is net-zero across the region (all intervening PUSH/POP
// instructions are balanced and SP is unmodified), the memory spill can be
// replaced with stack operations:
//   PUSH rr             (1 byte, 11 T)
//   ...
//   POP rr              (1 byte, 10 T)
// Total cost: 2 bytes, 21 T.
// Saves 4-6 bytes and 5-19 T per pair.
//
// WORKED EXAMPLE (from issue-74-bss-spill-no-call.ll, delete_line shape):
// Input MIR:
//   LD_nnind_HL @delete_line.frame + 2, implicit $hl  ; (3 B) spill temp
//   $a = LD_r_n 24
//   $hl = LD_rr_nn @delete_line.frame + 4
//   $b = LD_r_HLind implicit $hl
//   SUB_r killed renamable $b ...
//   ...
//   renamable $hl = LD_HL_nnind @delete_line.frame + 2 ; (3 B) reload temp
// StackDepth is 0 (no intervening stack operations).
// Transformed to:
//   PUSH_HL             ; (1 B) replaces store
//   $a = LD_r_n 24
//   ...
//   POP_HL              ; (1 B) replaces reload
// Net win: 6 B → 2 B (saves 4 B).
//
// SAFETY GUARDS:
//   1. STI.hasStaticFrame(): Only applies when static frames are used.
//   2. isStaticFrameSlot(): Only applies to static frame symbols (.frame,
//      __sfrend, __sframe, static_frame). Global variables must not be converted.
//   3. Address-taken guard (collectAddrTakenFrameSyms): If a frame symbol appears
//      in an instruction other than a direct BSS access (e.g. `LD HL, sym` for
//      pointer arithmetic or &local), the slot may be read indirectly. Refuse
//      conversion (issue #195/test_27).
//   4. Loop-carried guard (isSlotReadBeforeStoreInBlock): If the slot is read
//      earlier in the basic block before the store, it is the back-edge reload of
//      a loop-carried value. Dropping the store would leave the loop top reading
//      stale data (issue #195/test_166).
//   5. Cross-block orphan guard (isSlotUsedInOtherBlock): If another basic block
//      references the slot, it expects the value in BSS memory. Refuse conversion.
//   6. In-block orphan & conflict guard: If another store to the slot intervenes,
//      or an orphan load to a different register class appears, refuse conversion.
//   7. Stack balance: Intervening PUSH/POP must have net depth 0 at each reload.
//      Any intervening explicit SP modification (isExplicitSPWrite) bails.
//   8. POP AF safety: POP AF overwrites FLAGS; safe only when FLAGS is dead after.

static bool isAnyBssLoad(unsigned Opc) {
  return Opc == Z80::LD_A_nnind || Opc == Z80::LD_HL_nnind ||
         Opc == Z80::LD_DE_nnind || Opc == Z80::LD_BC_nnind;
}

static bool isAnyBssStore(unsigned Opc) {
  return Opc == Z80::LD_nnind_A || Opc == Z80::LD_nnind_HL ||
         Opc == Z80::LD_nnind_DE || Opc == Z80::LD_nnind_BC;
}

static bool isAnyBssAccess(unsigned Opc) {
  return isAnyBssLoad(Opc) || isAnyBssStore(Opc);
}

static bool isAnyPush(unsigned Opc) {
  return Opc == Z80::PUSH_AF || Opc == Z80::PUSH_BC || Opc == Z80::PUSH_DE ||
         Opc == Z80::PUSH_HL || Opc == Z80::PUSH_IX || Opc == Z80::PUSH_IY;
}

static bool isAnyPop(unsigned Opc) {
  return Opc == Z80::POP_AF || Opc == Z80::POP_BC || Opc == Z80::POP_DE ||
         Opc == Z80::POP_HL || Opc == Z80::POP_IX || Opc == Z80::POP_IY;
}

static bool isExplicitSPWrite(const MachineInstr &MI,
                              const TargetRegisterInfo *TRI) {
  unsigned Opc = MI.getOpcode();
  return !isAnyPush(Opc) && !isAnyPop(Opc) && !MI.isCall() &&
         MI.modifiesRegister(Z80::SP, TRI);
}

static bool isStaticFrameSlot(const MachineOperand &MO) {
  if (MO.isGlobal()) {
    StringRef Name = MO.getGlobal()->getName();
    return Name.ends_with(".frame") || Name.starts_with("__sfrend") ||
           Name.starts_with("__sframe") || Name.contains("static_frame");
  }
  if (MO.isMCSymbol()) {
    StringRef Name = MO.getMCSymbol()->getName();
    return Name.contains(".frame") || Name.starts_with("__sfrend") ||
           Name.starts_with("__sframe") || Name.contains("static_frame");
  }
  return false;
}

static bool sameBssAddress(const MachineInstr &A, const MachineInstr &B) {
  if (A.getNumOperands() == 0 || B.getNumOperands() == 0)
    return false;
  const MachineOperand &MA = A.getOperand(0);
  const MachineOperand &MB = B.getOperand(0);
  if (MA.isGlobal() && MB.isGlobal())
    return MA.getGlobal() == MB.getGlobal() && MA.getOffset() == MB.getOffset();
  if (MA.isMCSymbol() && MB.isMCSymbol())
    return MA.getMCSymbol() == MB.getMCSymbol() && MA.getOffset() == MB.getOffset();
  return false;
}

static void collectAddrTakenFrameSyms(
    MachineFunction &MF,
    SmallSet<std::pair<const void *, int64_t>, 8> &Out,
    SmallPtrSetImpl<const void *> &BaseAddrTaken) {
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      if (isAnyBssAccess(MI.getOpcode()))
        continue;
      for (const MachineOperand &MO : MI.operands()) {
        const void *Key = nullptr;
        if (MO.isGlobal())
          Key = MO.getGlobal();
        else if (MO.isMCSymbol())
          Key = MO.getMCSymbol();
        if (!Key)
          continue;
        int64_t Off = MO.getOffset();
        Out.insert({Key, Off});
        if (Off == 0)
          BaseAddrTaken.insert(Key);
      }
    }
  }
}

static bool isSlotAddrTaken(
    const MachineInstr &StoreMI,
    const SmallSet<std::pair<const void *, int64_t>, 8> &Set,
    const SmallPtrSetImpl<const void *> &BaseAddrTaken) {
  const MachineOperand &A = StoreMI.getOperand(0);
  const void *Key = A.isGlobal() ? (const void *)A.getGlobal()
                    : (A.isMCSymbol() ? (const void *)A.getMCSymbol() : nullptr);
  if (!Key)
    return false;
  if (BaseAddrTaken.count(Key))
    return true;
  return Set.count({Key, A.getOffset()});
}

static bool isSlotReadBeforeStoreInBlock(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator StoreIt) {
  for (auto P = MBB.begin(); P != StoreIt; ++P)
    if (isAnyBssAccess(P->getOpcode()) && sameBssAddress(*StoreIt, *P))
      return true;
  return false;
}

static bool isSlotReadBeforeWrittenInOtherBlock(
    MachineFunction &MF, const MachineBasicBlock &StoreMBB,
    const MachineInstr &StoreMI) {
  for (MachineBasicBlock &MBB : MF) {
    if (&MBB == &StoreMBB)
      continue;
    for (MachineInstr &MI : MBB) {
      if (sameBssAddress(StoreMI, MI)) {
        // If the first access in this other block is a load, it reads a value
        // that could have been written by StoreMBB; unsafe to convert to stack.
        if (isAnyBssLoad(MI.getOpcode()))
          return true;
        // If the first access is a store, this block overwrites the slot with
        // its own value before reading; it does not depend on StoreMBB.
        if (isAnyBssStore(MI.getOpcode()))
          break;
      }
    }
  }
  return false;
}

// Check whether a frame slot is referenced in other basic blocks.
// Accesses in blocks that strictly dominate StoreMBB belong to a different,
// earlier lifetime (slot-coalesced by regalloc) and execute before StoreMBB's
// store, so they leave the slot undisturbed by a PUSH/POP rewrite (issue #155).
static bool isSlotUsedElsewhere(
    MachineFunction &MF, const MachineInstr &StoreMI,
    ArrayRef<const MachineBasicBlock *> SkipBlocks,
    const MachineDominatorTree *MDT,
    const MachineBasicBlock *StoreMBB) {
  for (MachineBasicBlock &Other : MF) {
    if (llvm::is_contained(SkipBlocks, &Other))
      continue;
    bool DomSafe = MDT && StoreMBB && MDT->dominates(&Other, StoreMBB);
    for (MachineInstr &OI : Other) {
      if (isAnyBssAccess(OI.getOpcode()) && sameBssAddress(StoreMI, OI) &&
          !DomSafe)
        return true;
    }
  }
  return false;
}

struct SpillInfo {
  unsigned StoreOpc;
  unsigned LoadOpc;
  unsigned PushOpc;
  unsigned PopOpc;
  unsigned StoreBytes;
  unsigned LoadBytes;
};

static const SpillInfo SpillPairs[] = {
    {Z80::LD_nnind_A,  Z80::LD_A_nnind,  Z80::PUSH_AF, Z80::POP_AF, 3, 3},
    {Z80::LD_nnind_HL, Z80::LD_HL_nnind, Z80::PUSH_HL, Z80::POP_HL, 3, 3},
    {Z80::LD_nnind_DE, Z80::LD_DE_nnind, Z80::PUSH_DE, Z80::POP_DE, 4, 4},
    {Z80::LD_nnind_BC, Z80::LD_BC_nnind, Z80::PUSH_BC, Z80::POP_BC, 4, 4},
};

static const SpillInfo *getSpillInfo(unsigned Opc) {
  for (const auto &SI : SpillPairs)
    if (SI.StoreOpc == Opc)
      return &SI;
  return nullptr;
}

static bool isMatchingLoad(unsigned StoreOpc, unsigned Opc) {
  for (const auto &SI : SpillPairs)
    if (SI.StoreOpc == StoreOpc && SI.LoadOpc == Opc)
      return true;
  return false;
}

static bool optimizeBssSpills(MachineFunction &MF,
                              const TargetInstrInfo *TII,
                              const TargetRegisterInfo *TRI,
                              const Z80Subtarget &STI) {
  if (!STI.hasStaticFrame())
    return false; // only applicable for static-frame targets

  SmallSet<std::pair<const void *, int64_t>, 8> AddrTakenSlots;
  SmallPtrSet<const void *, 4> BaseAddrTakenSyms;
  collectAddrTakenFrameSyms(MF, AddrTakenSlots, BaseAddrTakenSyms);

  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    bool BlockChanged = false;
    for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
      const SpillInfo *SI = getSpillInfo(MII->getOpcode());
      if (!SI) {
        ++MII;
        continue; // not a known BSS spill store
      }
      if (!isStaticFrameSlot(MII->getOperand(0))) {
        ++MII;
        continue; // not a static frame slot (e.g. general global variable)
      }
      if (isSlotAddrTaken(*MII, AddrTakenSlots, BaseAddrTakenSyms)) {
        ++MII;
        continue; // slot address materialized into a register (issue #195)
      }
      if (isSlotReadBeforeStoreInBlock(MBB, MII)) {
        ++MII;
        continue; // loop-carried value read at top of loop (issue #195)
      }

      bool Conflict = false;
      int LoadCount = 0;
      SmallVector<MachineBasicBlock::iterator, 4> Loads;
      int StackDepth = 0;

      for (auto Scan = std::next(MII); Scan != MIE; ++Scan) {
        unsigned SOpc = Scan->getOpcode();

        // Another store to the same slot indicates value reuse/overwrite.
        if (SOpc == SI->StoreOpc && sameBssAddress(*MII, *Scan)) {
          Conflict = true;
          break;
        }

        // Orphan BSS load from same slot to a different register class (issue #82).
        if (isAnyBssLoad(SOpc) && !isMatchingLoad(SI->StoreOpc, SOpc) &&
            sameBssAddress(*MII, *Scan)) {
          Conflict = true;
          break;
        }

        // Track stack depth across all PUSH/POP instructions.
        if (isAnyPush(SOpc))
          ++StackDepth;
        if (isAnyPop(SOpc)) {
          --StackDepth;
          if (StackDepth < 0) {
            Conflict = true;
            break;
          }
        }

        // Intervening explicit SP writes make PUSH/POP displacement invalid.
        if (isExplicitSPWrite(*Scan, TRI)) {
          Conflict = true;
          break;
        }

        // Intervening SP reads (e.g. ADD_HL_SP, LD_HL_SP) compute
        // fixed-stack-object addresses with hard-coded offsets baked in by
        // PEI; inserting a PUSH at the store site shifts SP and invalidates
        // those offsets. PUSH/POP are already tracked via StackDepth so
        // exclude them here (ravn/llvm-z80#332).
        if (!isAnyPush(SOpc) && !isAnyPop(SOpc) && !Scan->isCall() &&
            Scan->readsRegister(Z80::SP, TRI)) {
          Conflict = true;
          break;
        }

        // Matching load from the same address.
        if (isMatchingLoad(SI->StoreOpc, SOpc) && sameBssAddress(*MII, *Scan)) {
          // Stack depth must be balanced at each reload point.
          if (StackDepth != 0) {
            Conflict = true;
            break;
          }
          Loads.push_back(Scan);
          ++LoadCount;
        }

        if (Scan->isTerminator())
          break;
      }

      // Stack must be balanced and at least one matching load found.
      if (StackDepth != 0 || Conflict || LoadCount == 0) {
        ++MII;
        continue;
      }

      // Ensure the slot is not read before written in any other basic block.
      if (isSlotReadBeforeWrittenInOtherBlock(MF, MBB, *MII)) {
        ++MII;
        continue;
      }

      int PushPopBytes = 2 * LoadCount; // PUSH + N*POP + (N-1)*re-PUSH
      int BssBytes = SI->StoreBytes + LoadCount * SI->LoadBytes;
      if (PushPopBytes >= BssBytes) {
        ++MII;
        continue; // no size win
      }

      // For POP AF: verify FLAGS register is dead after each reload.
      if (SI->PopOpc == Z80::POP_AF) {
        bool FlagsSafe = true;
        for (auto &LoadIt : Loads) {
          auto After = std::next(LoadIt);
          if (!isRegDeadAfter(After, MBB, TRI, Z80::FLAGS)) {
            FlagsSafe = false;
            break;
          }
        }
        if (!FlagsSafe) {
          ++MII;
          continue;
        }
      }

      LLVM_DEBUG(dbgs() << "  BSS spill→PUSH/POP: " << *MII
                        << "  " << LoadCount << " loads, saves "
                        << (BssBytes - PushPopBytes) << "B\n");

      DebugLoc DL = MII->getDebugLoc();
      MachineInstr *PushMI = BuildMI(MBB, *MII, DL, TII->get(SI->PushOpc));
      auto StoreIt = MII;

      for (int i = 0; i < LoadCount; ++i) {
        auto &LoadMI = *Loads[i];
        DebugLoc LoadDL = LoadMI.getDebugLoc();
        BuildMI(MBB, LoadMI, LoadDL, TII->get(SI->PopOpc));
        if (i < LoadCount - 1) {
          // Re-PUSH to preserve on stack for subsequent loads.
          BuildMI(MBB, LoadMI, LoadDL, TII->get(SI->PushOpc));
        }
        MBB.erase(Loads[i]);
      }
      MBB.erase(StoreIt);

      ++NumBssSpillsToPushPop;
      Changed = BlockChanged = true;
      MII = PushMI->getIterator();
      ++MII;
    }
    if (BlockChanged)
      recomputeLivenessFlags(MBB);
  }

  return Changed;
}

// --- Cross-class BSS spill -> PUSH/POP ---
//
// WHAT: In +static-frame code, when a 16-bit register pair is spilled to a BSS
// frame slot and subsequently reloaded into a DIFFERENT 16-bit register pair
// within the same basic block (e.g. transfer via memory over a CALL), replace
// the spill and reload with stack operations:
//   LD (slot), rr_src     (3-4 B)
//   ... [intervening code / CALLs] ...
//   LD rr_dst, (slot)     (3-4 B)   ; rr_dst != rr_src
// converted to:
//   PUSH rr_src           (1 B)
//   ...
//   POP rr_dst            (1 B)
// Total cost: 2 B (vs 6-8 B), saving 4-6 bytes per pair.
//
// WORKED EXAMPLE (from bss-spill-cross-class-transfer.mir @test_cross_class_de_bc):
//   LD_nnind_DE <mcsymbol __sframe_foo>, implicit $de
//   CALL_nn @dummy, ...
//   LD_BC_nnind <mcsymbol __sframe_foo>, implicit-def $bc
// Transformed to:
//   PUSH_DE implicit $de
//   CALL_nn @dummy, ...
//   POP_BC implicit-def $bc
//
// GOTCHA:
//   - Only applicable to 16-bit register pairs (HL, DE, BC). 8-bit A uses FLAGS
//     for PUSH/POP and is handled by the same-class spill peephole.
//   - Must have exactly ONE reload from the slot (multiple cross-class reloads
//     would leave the stack unbalanced or empty after the first pop).
//   - Stack depth across intervening instructions must be net-zero.
//   - The slot must not be read or overwritten elsewhere.
static bool optimizeCrossClassBssSpills(MachineFunction &MF,
                                       const TargetInstrInfo *TII,
                                       const TargetRegisterInfo *TRI,
                                       const Z80Subtarget &STI) {
  if (!STI.hasStaticFrame())
    return false; // only applicable for static-frame targets

  struct StoreClass {
    unsigned StoreOpc;
    unsigned PushOpc;
    unsigned Bytes;
  };
  struct LoadClass {
    unsigned LoadOpc;
    unsigned PopOpc;
    unsigned Bytes;
  };
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
      if (S.StoreOpc == Opc)
        return &S;
    return nullptr;
  };
  auto getLoadInfo = [&](unsigned Opc) -> const LoadClass * {
    for (const auto &L : Loads)
      if (L.LoadOpc == Opc)
        return &L;
    return nullptr;
  };

  SmallSet<std::pair<const void *, int64_t>, 8> AddrTakenSlots;
  SmallPtrSet<const void *, 4> BaseAddrTakenSyms;
  collectAddrTakenFrameSyms(MF, AddrTakenSlots, BaseAddrTakenSyms);

  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    bool BlockChanged = false;
    for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
      const StoreClass *SC = getStoreInfo(MII->getOpcode());
      if (!SC) {
        ++MII;
        continue; // not a recognized 16-bit BSS spill store
      }
      auto hasVolatileMemoryRef = [](const MachineInstr &MI) {
        for (const MachineMemOperand *MMO : MI.memoperands())
          if (MMO->isVolatile())
            return true;
        return false;
      };
      if (hasVolatileMemoryRef(*MII)) {
        ++MII;
        continue; // do not eliminate volatile memory accesses
      }
      if (!isStaticFrameSlot(MII->getOperand(0))) {
        ++MII;
        continue; // not a static frame slot (e.g. general global variable)
      }
      if (isSlotAddrTaken(*MII, AddrTakenSlots, BaseAddrTakenSyms)) {
        ++MII;
        continue; // slot address materialized into a register (issue #195)
      }
      if (isSlotReadBeforeStoreInBlock(MBB, MII)) {
        ++MII;
        continue; // loop-carried value read at top of loop (issue #195)
      }

      int StackDepth = 0;
      bool Conflict = false;
      MachineBasicBlock::iterator MatchedLoad = MIE;
      const LoadClass *LC = nullptr;

      for (auto Scan = std::next(MII); Scan != MIE; ++Scan) {
        unsigned SOpc = Scan->getOpcode();

        // Another store to the same slot indicates value reuse/overwrite.
        if (isAnyBssStore(SOpc) && sameBssAddress(*MII, *Scan)) {
          Conflict = true;
          break;
        }

        // Track stack depth across all PUSH/POP instructions.
        if (isAnyPush(SOpc))
          ++StackDepth;
        if (isAnyPop(SOpc)) {
          --StackDepth;
          if (StackDepth < 0) {
            Conflict = true;
            break;
          }
        }

        // Intervening explicit SP writes make PUSH/POP displacement invalid.
        if (isExplicitSPWrite(*Scan, TRI)) {
          Conflict = true;
          break;
        }

        // Intervening SP reads (e.g. ADD_HL_SP, LD_HL_SP) compute
        // fixed-stack-object addresses with hard-coded offsets baked in by
        // PEI; inserting a PUSH at the store site shifts SP and invalidates
        // those offsets. PUSH/POP are already tracked via StackDepth so
        // exclude them here (ravn/llvm-z80#332).
        if (!isAnyPush(SOpc) && !isAnyPop(SOpc) && !Scan->isCall() &&
            Scan->readsRegister(Z80::SP, TRI)) {
          Conflict = true;
          break;
        }

        // Check for load from our slot.
        if (isAnyBssLoad(SOpc) && sameBssAddress(*MII, *Scan)) {
          if (MatchedLoad != MIE) {
            // Already saw one load; cross-class transfer requires exactly one load.
            Conflict = true;
            break;
          }
          const LoadClass *LCi = getLoadInfo(SOpc);
          if (!LCi) {
            // Not a 16-bit register load (e.g. 8-bit A load).
            Conflict = true;
            break;
          }
          if (StackDepth != 0) {
            // Stack is not balanced at the load point.
            Conflict = true;
            break;
          }
          // Same-class loads are handled by optimizeBssSpills.
          bool SameClass =
              (SC->PushOpc == Z80::PUSH_HL && LCi->PopOpc == Z80::POP_HL) ||
              (SC->PushOpc == Z80::PUSH_DE && LCi->PopOpc == Z80::POP_DE) ||
              (SC->PushOpc == Z80::PUSH_BC && LCi->PopOpc == Z80::POP_BC);
          if (SameClass) {
            Conflict = true;
            break;
          }
          MatchedLoad = Scan;
          LC = LCi;
        }

        if (Scan->isTerminator())
          break;
      }

      if (Conflict || MatchedLoad == MIE || LC == nullptr ||
          hasVolatileMemoryRef(*MatchedLoad)) {
        ++MII;
        continue;
      }

      // Ensure the slot is not read in any other block before being rewritten.
      if (isSlotReadBeforeWrittenInOtherBlock(MF, MBB, *MII)) {
        ++MII;
        continue;
      }

      LLVM_DEBUG(dbgs() << "  BSS cross-class spill→PUSH/POP: " << *MII
                        << "  -> " << LC->LoadOpc << "\n");

      DebugLoc DLs = MII->getDebugLoc();
      MachineInstr *PushMI = BuildMI(MBB, *MII, DLs, TII->get(SC->PushOpc));

      DebugLoc DLl = MatchedLoad->getDebugLoc();
      BuildMI(MBB, *MatchedLoad, DLl, TII->get(LC->PopOpc));

      MBB.erase(MatchedLoad);
      MBB.erase(MII);

      ++NumCrossClassBssSpills;
      Changed = BlockChanged = true;
      MII = PushMI->getIterator();
      ++MII;
    }
    if (BlockChanged)
      recomputeLivenessFlags(MBB);
  }

  return Changed;
}

// --- Cross-MBB BSS spill -> PUSH/POP (issues #132, #138, #143, #155, #156) ---
//
// Extends the in-MBB BSS-spill peephole to the case where the store lives in
// MBB_A and the matching load lives in a successor block MBB_B.
//
// Worked example (from issue-132-bss-spill-cross-mbb.ll @retry):
//   MBB_A (.LBB0_1):
//     LD (L_retry.frame), A       ; 3 B  -> PUSH AF (1 B)
//     CALL _target
//     JR NZ, .LBB0_4              ; escape edge to .LBB0_4
//     ; fallthrough to MBB_B
//   MBB_B (.LBB0_2):
//     LD A, (L_retry.frame)       ; 3 B  -> POP AF (1 B)
//     DEC A
//     JR NZ, .LBB0_1
//   MBB_C (.LBB0_4):
//     POP AF                      ; 1 B  (compensation: AF dead at .LBB0_4)
//     LD DE, 1
//     RET
//
// Compensation strategies for non-LOAD escape successors of MBB_A:
//   - Prepend in-place at MBB_C's head when MBB_A is MBB_C's sole predecessor.
//   - Otherwise edge-split: insert a new MBB before MBB_C in layout, fall
//     through to MBB_C, and rewrite MBB_A's branch operand to the new MBB.
// Compensation uses `POP rr` (1 B) when a register pair is dead at the escape
// (issue #138), or `INC SP; INC SP` (2 B) otherwise.
static bool optimizeCrossMbbBssSpills(MachineFunction &MF,
                                      const TargetInstrInfo *TII,
                                      const TargetRegisterInfo *TRI,
                                      const Z80Subtarget &STI) {
  if (!STI.hasStaticFrame())
    return false; // only applicable for static-frame targets

  SmallSet<std::pair<const void *, int64_t>, 8> AddrTakenSlots;
  SmallPtrSet<const void *, 4> BaseAddrTakenSyms;
  collectAddrTakenFrameSyms(MF, AddrTakenSlots, BaseAddrTakenSyms);

  bool Changed = false;
  SmallPtrSet<MachineBasicBlock *, 4> OurNewMBBs;
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
        const SpillInfo *SI = getSpillInfo(MII->getOpcode());
        if (!SI)
          continue; // not a recognized spill store opcode
        if (!isStaticFrameSlot(MII->getOperand(0)))
          continue; // not a static frame slot
        if (isSlotReadBeforeStoreInBlock(MBB_A, MII))
          continue; // loop-carried value read before store (issues #195, #202)
        if (isSlotAddrTaken(*MII, AddrTakenSlots, BaseAddrTakenSyms))
          continue; // slot address escaped into pointer register (issue #195)

        // Scan forward in MBB_A to ensure no conflicting accesses or stack imbalances.
        int StackDepth = 0;
        bool BailLocal = false;
        for (auto S = std::next(MII); S != MIE; ++S) {
          unsigned O = S->getOpcode();
          if (isAnyPush(O))
            ++StackDepth;
          if (isAnyPop(O)) {
            --StackDepth;
            if (StackDepth < 0) {
              BailLocal = true; // net-underflow inside block
              break;
            }
          }
          if (isExplicitSPWrite(*S, TRI)) {
            BailLocal = true; // explicit SP rewrite invalidates stack tracking
            break;
          }
          // Intervening SP reads (e.g. ADD_HL_SP, LD_HL_SP) compute
          // fixed-stack-object addresses with hard-coded offsets baked in by
          // PEI; inserting a PUSH at the store site shifts SP and invalidates
          // those offsets. PUSH/POP are already tracked via StackDepth so
          // exclude them here (ravn/llvm-z80#332).
          if (!isAnyPush(O) && !isAnyPop(O) && !S->isCall() &&
              S->readsRegister(Z80::SP, TRI)) {
            BailLocal = true;
            break;
          }
          if (isAnyBssAccess(O) && sameBssAddress(*MII, *S)) {
            BailLocal = true; // in-MBB reuse handled by single-block peephole
            break;
          }
          if (S->isTerminator())
            break;
        }
        if (BailLocal || StackDepth != 0)
          continue; // unbalanced stack or local conflict

        // Inspect successors: exactly one MBB_B must reload the slot.
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
            if (isAnyPop(O)) {
              --SuccDepth;
              if (SuccDepth < 0) {
                BailSucc = true; // underflow before slot touch
                break;
              }
            }
            if (isExplicitSPWrite(*T, TRI)) {
              BailSucc = true; // SP modification before touch
              break;
            }
            // SP read before slot touch would see shifted SP after prepended
            // PUSH (ravn/llvm-z80#332).
            if (!isAnyPush(O) && !isAnyPop(O) && !T->isCall() &&
                T->readsRegister(Z80::SP, TRI)) {
              BailSucc = true;
              break;
            }
            if (isAnyBssAccess(O) && sameBssAddress(*MII, *T)) {
              SuccTouches = true;
              FirstTouch = T;
              break;
            }
          }
          if (BailSucc)
            break;

          if (!SuccTouches) {
            // Escape successor candidate.
            if (Succ->pred_size() == 1 && *Succ->pred_begin() == &MBB_A) {
              // Prepend in-place: MBB_A is sole predecessor of this escape.
              Escapes.push_back({Succ, ESC_PrependInPlace});
              continue;
            }
            // Edge-split candidate: MBB_A must have explicit branch to Succ.
            bool HasExplicitEdge = false;
            for (auto Ti = MBB_A.getFirstTerminator(); Ti != MBB_A.end(); ++Ti) {
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
              BailSucc = true; // cannot split fall-through edge safely
              break;
            }
            MachineBasicBlock *Prev = Succ->getPrevNode();
            if (Prev && Prev->canFallThrough() &&
                Prev->isLayoutSuccessor(Succ) &&
                !OurNewMBBs.contains(Prev)) {
              BailSucc = true; // unrelated block falls through into Succ (#143)
              break;
            }
            Escapes.push_back({Succ, ESC_InsertBefore});
            continue;
          }

          // Slot touched in Succ: must match store opcode class.
          if (!isMatchingLoad(SI->StoreOpc, FirstTouch->getOpcode())) {
            BailSucc = true; // mismatch between store and load opcode classes
            break;
          }
          if (SuccDepth != 0) {
            BailSucc = true; // stack depth not zero at load site
            break;
          }
          if (MBB_B != nullptr) {
            BailSucc = true; // more than one load-bearing successor block
            break;
          }
          MBB_B = Succ;
          LoadIt = FirstTouch;
        }

        if (BailSucc || !MBB_B)
          continue; // invalid successor structure

        // MBB_B must be reached only from MBB_A (issue #156).
        if (MBB_B->pred_size() != 1 || *MBB_B->pred_begin() != &MBB_A)
          continue; // back-edge or alternate entry would leak SP (#156)

        // Ensure no other block accesses the slot unless strictly dominating (issue #155).
        if (isSlotUsedElsewhere(MF, *MII, {&MBB_A, MBB_B}, MDT.get(), &MBB_A))
          continue; // slot used in non-dominating blocks (#155, #203)

        // For POP AF: FLAGS must be dead after the reload position.
        if (SI->PopOpc == Z80::POP_AF) {
          auto After = std::next(LoadIt);
          if (!isRegDeadAfter(After, *MBB_B, TRI, Z80::FLAGS))
            continue; // FLAGS live across reload point
        }

        // Cost gate with liveness-driven compensation probing (issue #138).
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
              EscPopOpc[i] = P.Op; // dead register pair allows 1-byte POP
              break;
            }
          }
          CompCost += EscPopOpc[i] ? 1 : 2;
        }
        unsigned PushPopSave = (SI->StoreBytes - 1) + (SI->LoadBytes - 1);
        if (PushPopSave <= CompCost)
          continue; // rewrite does not reduce code size

        LLVM_DEBUG({
          dbgs() << "  Cross-MBB BSS spill→PUSH/POP: " << *MII
                 << "  load in BB#" << MBB_B->getNumber() << ", "
                 << Escapes.size() << " escape MBB(s), saves "
                 << (PushPopSave - CompCost) << "B\n";
        });

        // Rewrite MBB_A: STORE -> PUSH.
        DebugLoc DLs = MII->getDebugLoc();
        BuildMI(MBB_A, *MII, DLs, TII->get(SI->PushOpc));
        MBB_A.erase(MII);

        // Rewrite MBB_B: LOAD -> POP.
        DebugLoc DLl = LoadIt->getDebugLoc();
        BuildMI(*MBB_B, *LoadIt, DLl, TII->get(SI->PopOpc));
        MBB_B->erase(LoadIt);

        // Emit compensation on each escape edge.
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
            emitComp(MBB_C, MBB_C->begin());
          } else {
            MachineBasicBlock *NewMBB = MF.CreateMachineBasicBlock();
            MF.insert(MBB_C->getIterator(), NewMBB);
            for (const auto &LI : MBB_C->liveins())
              NewMBB->addLiveIn(LI);
            emitComp(NewMBB, NewMBB->end());
            NewMBB->addSuccessor(MBB_C);
            MBB_A.ReplaceUsesOfBlockWith(MBB_C, NewMBB);
            OurNewMBBs.insert(NewMBB); // track created block (#143)
          }
        }

        recomputeLivenessFlags(MBB_A);
        recomputeLivenessFlags(*MBB_B);
        for (auto &E : Escapes)
          recomputeLivenessFlags(*E.MBB_C);

        ++NumBssSpillsToPushPop;
        Changed = true;
        RestartOuter = true;
        break;
      }
    }
  }

  return Changed;
}

// --- Peephole #331: SP-relative spill/reload -> PUSH/POP around CALL ---
//
// WHAT: in non-static-frame code (or when the spill lands on a dynamic SP
// frame anyway), a 16-bit spill immediately before a CALL followed by a
// matching reload immediately after emits 14 B of SP-relative frame traffic
// (`ld hl,K; add hl,sp; ld (hl),lo; inc hl; ld (hl),hi` + mirror on reload).
// Replace with `push rr` / `pop rr` (2 B) around the CALL.
//
// PATTERN (spill, exactly 5 MIs):
//   $hl = LD_rr_nn K
//   ADD_HL_SP        (defs $hl, $flags)
//   LD_HLind_r $lo   (store (s16) into %stack.N)
//   $hl = INC_rr $hl
//   LD_HLind_r $hi   (store (s16) into %stack.N)
// followed by CALL_nn (immediately).
//
// PATTERN (reload, exactly 5 MIs, immediately after CALL):
//   $hl = LD_rr_nn K            ; same K, same slot
//   ADD_HL_SP
//   $lo = LD_r_HLind
//   $hl = INC_rr $hl
//   $hi = LD_r_HLind
//
// SAFETY (strict; strictly stricter than the unsound draft in
// tasks/issue331-sprelative-pushpop-unsound-2026-09-16.md):
//   1. EXACTLY ONE spill-store and EXACTLY ONE reload-load reference the
//      slot in the WHOLE function.  Ackermann's second reload of the same
//      slot is the unsound case; requiring uniqueness eliminates it.
//   2. Register pair must be BC / DE / HL (caller-saved; CALL clobbers).
//   3. Spill's 5 MIs immediately precede the CALL; reload's 5 MIs
//      immediately follow.  No intervening SP-touch, no other push/pop, no
//      other memory access to the slot -- guaranteed by adjacency.
//   4. HL is dead across the CALL (the reload will re-materialise HL from
//      LD_rr_nn K; ADD_HL_SP; CALL clobbers $hl per RegMask).
//   5. Stack slot is only referenced by this exactly-one spill/reload pair
//      (function-wide scan of frame-index MMOs).
//
// The PUSH/POP change does NOT shrink the prologue (the `push af` slot
// reservation stays) -- shrinking the frame is a follow-up.  Net win here
// is 14 B (spill) + 14 B (reload) - 1 B (push) - 1 B (pop) = 26 B per pair.
// Wait: the spill/reload were 7 B each = 14 B total, not 14 each.  Net:
// 14 B - 2 B = 12 B per pair.

// Recognize the LD_rr_nn form that starts a spill/reload sequence.
static bool isLDrrNN_HL(const MachineInstr &MI, int64_t &OutImm) {
  if (MI.getOpcode() != Z80::LD_rr_nn)
    return false;
  if (MI.getNumOperands() < 2)
    return false;
  const MachineOperand &Dst = MI.getOperand(0);
  const MachineOperand &Imm = MI.getOperand(1);
  if (!Dst.isReg() || Dst.getReg() != Z80::HL)
    return false;
  if (!Imm.isImm())
    return false;
  OutImm = Imm.getImm();
  return true;
}

// True iff MI is `ADD_HL_SP` (in whatever encoding form).  On Z80 this is
// a single-opcode instruction defining HL from HL + SP.
static bool isAddHLSP(const MachineInstr &MI) {
  return MI.getOpcode() == Z80::ADD_HL_SP;
}

// True iff MI is `LD (HL), r` for r in {B, C, D, E, H, L, A}.
static bool isLDpHLr(const MachineInstr &MI, Register &OutSrc) {
  if (MI.getOpcode() != Z80::LD_HLind_r)
    return false;
  if (MI.getNumOperands() < 1 || !MI.getOperand(0).isReg())
    return false;
  OutSrc = MI.getOperand(0).getReg();
  return true;
}

// True iff MI is `LD r, (HL)`.
static bool isLDrpHL(const MachineInstr &MI, Register &OutDst) {
  if (MI.getOpcode() != Z80::LD_r_HLind)
    return false;
  if (MI.getNumOperands() < 1 || !MI.getOperand(0).isReg())
    return false;
  OutDst = MI.getOperand(0).getReg();
  return true;
}

// True iff MI is `INC HL` (as a pair-op).
static bool isIncHL(const MachineInstr &MI) {
  if (MI.getOpcode() != Z80::INC_rr)
    return false;
  if (MI.getNumOperands() < 1 || !MI.getOperand(0).isReg())
    return false;
  return MI.getOperand(0).getReg() == Z80::HL;
}

// Read the frame-index from an MMO that landed on %stack.N.  Returns -1 if
// this MI does not carry a fixed-stack MMO.  Frame slots reach the MMO via
// either FixedStackPseudoSourceValue (fixed objects declared by PEI) or the
// generic PseudoSourceValue::Stack kind (RA-created spill slots).
static int getStackFI(const MachineInstr &MI) {
  for (const MachineMemOperand *MMO : MI.memoperands()) {
    if (const auto *PSV = MMO->getPseudoValue()) {
      if (const auto *FIPV = dyn_cast<FixedStackPseudoSourceValue>(PSV))
        return FIPV->getFrameIndex();
    }
  }
  return -1;
}

// Recognize the 5-MI spill sequence starting at It.  On success fills
// OutPair (HL/DE/BC), OutOffset (immediate from LD_rr_nn), OutFI (frame
// index from either MMO), and advances OutLast to the last MI of the seq.
static bool matchSpillSeq(MachineBasicBlock::iterator It,
                          MachineBasicBlock::iterator End,
                          Register &OutPair, int64_t &OutOffset,
                          int &OutFI,
                          MachineBasicBlock::iterator &OutLast,
                          const TargetRegisterInfo *TRI) {
  auto A = It;
  if (A == End)
    return false;
  int64_t Imm;
  if (!isLDrrNN_HL(*A, Imm))
    return false;

  auto B = std::next(A);
  if (B == End || !isAddHLSP(*B))
    return false;

  auto C = std::next(B);
  if (C == End)
    return false;
  Register LoSrc;
  if (!isLDpHLr(*C, LoSrc))
    return false;

  auto D = std::next(C);
  if (D == End || !isIncHL(*D))
    return false;

  auto E = std::next(D);
  if (E == End)
    return false;
  Register HiSrc;
  if (!isLDpHLr(*E, HiSrc))
    return false;

  // Pair identification: (LoSrc, HiSrc) must be sibling halves of a
  // caller-saved 16-bit pair.
  if (LoSrc == Z80::C && HiSrc == Z80::B) OutPair = Z80::BC;
  else if (LoSrc == Z80::E && HiSrc == Z80::D) OutPair = Z80::DE;
  else if (LoSrc == Z80::L && HiSrc == Z80::H) OutPair = Z80::HL;
  else return false;

  // Both MMOs must reference the same fixed stack slot.
  int FI_C = getStackFI(*C);
  int FI_E = getStackFI(*E);
  if (FI_C < 0 || FI_C != FI_E)
    return false;

  OutOffset = Imm;
  OutFI = FI_C;
  OutLast = E;
  return true;
}

// Recognize the 5-MI reload sequence starting at It.  On success fills the
// same out-fields as matchSpillSeq.
static bool matchReloadSeq(MachineBasicBlock::iterator It,
                           MachineBasicBlock::iterator End,
                           Register &OutPair, int64_t &OutOffset,
                           int &OutFI,
                           MachineBasicBlock::iterator &OutLast,
                           const TargetRegisterInfo *TRI) {
  auto A = It;
  if (A == End)
    return false;
  int64_t Imm;
  if (!isLDrrNN_HL(*A, Imm))
    return false;

  auto B = std::next(A);
  if (B == End || !isAddHLSP(*B))
    return false;

  auto C = std::next(B);
  if (C == End)
    return false;
  Register LoDst;
  if (!isLDrpHL(*C, LoDst))
    return false;

  auto D = std::next(C);
  if (D == End || !isIncHL(*D))
    return false;

  auto E = std::next(D);
  if (E == End)
    return false;
  Register HiDst;
  if (!isLDrpHL(*E, HiDst))
    return false;

  if (LoDst == Z80::C && HiDst == Z80::B) OutPair = Z80::BC;
  else if (LoDst == Z80::E && HiDst == Z80::D) OutPair = Z80::DE;
  else if (LoDst == Z80::L && HiDst == Z80::H) OutPair = Z80::HL;
  else return false;

  int FI_C = getStackFI(*C);
  int FI_E = getStackFI(*E);
  if (FI_C < 0 || FI_C != FI_E)
    return false;

  OutOffset = Imm;
  OutFI = FI_C;
  OutLast = E;
  return true;
}

// Function-wide scan: return the number of distinct MIs whose MMOs reference
// this frame index.  Used to prove single-reader/single-writer.
static unsigned countSlotRefs(MachineFunction &MF, int FI) {
  unsigned Refs = 0;
  for (const MachineBasicBlock &MBB : MF)
    for (const MachineInstr &MI : MBB) {
      for (const MachineMemOperand *MMO : MI.memoperands()) {
        if (const auto *PSV = MMO->getPseudoValue())
          if (const auto *FIPV = dyn_cast<FixedStackPseudoSourceValue>(PSV))
            if (FIPV->getFrameIndex() == FI) {
              ++Refs;
              break; // don't double-count within one MI
            }
      }
    }
  return Refs;
}

static unsigned getPushOpcForPair(Register Pair) {
  switch (Pair) {
  case Z80::BC: return Z80::PUSH_BC;
  case Z80::DE: return Z80::PUSH_DE;
  case Z80::HL: return Z80::PUSH_HL;
  default: llvm_unreachable("unexpected pair");
  }
}

static unsigned getPopOpcForPair(Register Pair) {
  switch (Pair) {
  case Z80::BC: return Z80::POP_BC;
  case Z80::DE: return Z80::POP_DE;
  case Z80::HL: return Z80::POP_HL;
  default: llvm_unreachable("unexpected pair");
  }
}

static bool optimizeSPRelativeSpillToPushPop(MachineFunction &MF,
                                             const TargetInstrInfo *TII,
                                             const TargetRegisterInfo *TRI,
                                             const Z80Subtarget &STI) {
  bool Changed = false;

  // Iterate until fixed point: converting one pair may expose another.
  bool AnyChangeInIteration = true;
  while (AnyChangeInIteration) {
    AnyChangeInIteration = false;

    for (MachineBasicBlock &MBB : MF) {
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ) {
        // Look for a spill sequence.
        Register SpillPair;
        int64_t SpillOff;
        int SpillFI;
        MachineBasicBlock::iterator SpillLast;
        if (!matchSpillSeq(MII, MIE, SpillPair, SpillOff, SpillFI,
                           SpillLast, TRI)) {
          ++MII;
          continue;
        }

        // Walk forward from spill-end.  Allow only PUSH/POP (may come from
        // a nested-spill conversion an earlier iteration performed) and
        // EXACTLY one CALL.  Track cumulative stack depth; require it to
        // be 0 (net-balanced) at the point we start looking for a matching
        // reload sequence.  Any other instruction, or any explicit SP
        // write, or a second CALL, bails.
        auto Scan = std::next(SpillLast);
        int Depth = 0;
        MachineBasicBlock::iterator TheCall = MIE;
        while (Scan != MIE) {
          unsigned O = Scan->getOpcode();
          if (isAnyPush(O)) { ++Depth; ++Scan; continue; }
          if (isAnyPop(O))  { --Depth; if (Depth < 0) break; ++Scan; continue; }
          if (Scan->isCall()) {
            if (TheCall != MIE) break; // second CALL -> bail
            TheCall = Scan;
            ++Scan;
            continue;
          }
          break; // any other MI (SP-touch, memop, control-flow) bails
        }
        if (TheCall == MIE || Depth != 0) {
          ++MII;
          continue;
        }
        auto AfterSpill = TheCall;
        auto AfterCall = Scan;
        Register RelPair;
        int64_t RelOff;
        int RelFI;
        MachineBasicBlock::iterator RelLast;
        if (!matchReloadSeq(AfterCall, MIE, RelPair, RelOff, RelFI,
                            RelLast, TRI)) {
          ++MII;
          continue;
        }

        // Guards.
        if (SpillPair != RelPair || SpillFI != RelFI ||
            SpillOff != RelOff) {
          ++MII;
          continue;
        }

        // Function-wide: exactly four MIs must reference this slot -- the
        // two stores in the matched spill (lo and hi halves) plus the two
        // loads in the matched reload.  Any additional reference means the
        // slot is used elsewhere (a second reload -> the unsound ackermann
        // shape), so we bail.
        if (countSlotRefs(MF, SpillFI) != 4) {
          ++MII;
          continue;
        }

        // Capture the ranges to erase before we insert anything.
        // Spill range: [MII .. std::next(SpillLast))
        // Reload range: [AfterCall .. std::next(RelLast))
        auto SpillBegin = MII;
        auto SpillEnd = std::next(SpillLast);
        auto ReloadBegin = AfterCall;
        auto ReloadEnd = std::next(RelLast);

        // Order: erase FIRST, then insert.  Inserting POP before ReloadEnd
        // and then erasing [ReloadBegin, ReloadEnd) would splat the POP
        // (which sits at ReloadEnd - 1) inside the erase interval.
        // Insertion at a saved past-end iterator survives the erase because
        // the iterator anchors to the following MI (which we do not touch).
        DebugLoc DL = MII->getDebugLoc();

        // Erase reload first (higher addresses; safe w.r.t. spill iters).
        while (ReloadBegin != ReloadEnd) {
          auto N = std::next(ReloadBegin);
          ReloadBegin->eraseFromParent();
          ReloadBegin = N;
        }
        // Erase spill.
        while (SpillBegin != SpillEnd) {
          auto N = std::next(SpillBegin);
          SpillBegin->eraseFromParent();
          SpillBegin = N;
        }

        // Now insert PUSH at SpillEnd position (which is where the erased
        // spill sequence ended = the CALL) and POP at ReloadEnd (which is
        // the MI after the erased reload sequence).  These iterators remain
        // valid because we didn't touch the CALL or the MI after the reload.
        BuildMI(MBB, SpillEnd, DL, TII->get(getPushOpcForPair(SpillPair)));
        BuildMI(MBB, ReloadEnd, DL, TII->get(getPopOpcForPair(RelPair)));

        AnyChangeInIteration = true;
        Changed = true;
        ++NumSPSpillsToPushPop;

        // Restart from block top; iterators past the transformation are
        // invalidated.
        MII = MBB.begin();
        break;
      }
      if (AnyChangeInIteration) break; // restart outer loop
    }
  }

  return Changed;
}

// --- Peephole #85: consecutive `LD A,n; LD (addr),A` chain ---
//
// When >= 3 consecutive byte stores write to consecutive addresses:
//   LD A, imm0; LD (addr0), A     (2+3 = 5 B)
//   LD A, imm1; LD (addr0+1), A   (2+3 = 5 B)
//   LD A, imm2; LD (addr0+2), A   (2+3 = 5 B)
// replace with:
//   LD HL, addr0                  (3 B)
//   LD (HL), imm0                 (2 B)
//   INC HL                        (1 B)
//   LD (HL), imm1                 (2 B)
//   INC HL                        (1 B)
//   LD (HL), imm2                 (2 B)
// (Omits the trailing INC HL after the last store).
// Total: 3 + 3*2 + 2*1 = 11 B vs 15 B (saves 4 B for N=3, 6 B for N=4, etc.).
//
// Worked example (from store-chain-walk.ll @seed_buf):
//   Input MIR:
//     $a = LD_r_n 16
//     LD_nnind_A @buf, killed $a
//     $a = LD_r_n 32
//     LD_nnind_A @buf+1, killed $a
//     $a = LD_r_n 48
//     LD_nnind_A @buf+2, killed $a
//     $a = LD_r_n 64
//     LD_nnind_A @buf+3, killed $a
//   Output:
//     $hl = LD_rr_nn @buf
//     LD_HLind_n 16, implicit $hl
//     $hl = INC_rr killed $hl
//     LD_HLind_n 32, implicit $hl
//     $hl = INC_rr killed $hl
//     LD_HLind_n 48, implicit $hl
//     $hl = INC_rr killed $hl
//     LD_HLind_n 64, implicit $hl
//
// Soundness guards:
//   - Must have >= 3 consecutive byte stores to addr+k.
//   - H and L must be dead before the chain (LD HL clobbers HL).
//   - HL and A must be dead after the chain.
static bool optimizeConsecutiveStores(MachineBasicBlock &MBB,
                                      const TargetInstrInfo *TII,
                                      const TargetRegisterInfo *TRI,
                                      const Z80Subtarget &STI) {
  if (!STI.hasZ80())
    return false;

  // See optimizeInMemoryIncDec above: in an interrupt handler HL is
  // callee-saved per Z80_Interrupt_CSR; introducing a new HL def after
  // PEI silently drops the required push (ravn/llvm-z80#341).
  const MachineFunction *MF = MBB.getParent();
  if (MF->getFunction().hasFnAttribute("interrupt")) {
    bool HLAlreadySaved = false;
    for (const CalleeSavedInfo &CSI :
         MF->getFrameInfo().getCalleeSavedInfo()) {
      if (CSI.getReg() == Z80::HL) {
        HLAlreadySaved = true;
        break;
      }
    }
    if (!HLAlreadySaved)
      return false;
  }

  struct AddrKey {
    const GlobalValue *GV = nullptr;
    const char *SymbolName = nullptr;
    int64_t Off = 0;
  };

  auto getStoreAddr = [](const MachineInstr &MI) -> std::optional<AddrKey> {
    if (MI.getOpcode() != Z80::LD_nnind_A)
      return std::nullopt;
    const MachineOperand &Op = MI.getOperand(0);
    if (Op.isImm())
      return AddrKey{nullptr, nullptr, Op.getImm()};
    if (Op.isGlobal())
      return AddrKey{Op.getGlobal(), nullptr, (int64_t)Op.getOffset()};
    if (Op.isSymbol())
      return AddrKey{nullptr, Op.getSymbolName(), (int64_t)Op.getOffset()};
    return std::nullopt;
  };

  bool Changed = false;
  auto MII = MBB.begin();
  const auto MIE = MBB.end();
  while (MII != MIE) {
    // Match head of run: LD A, imm0
    if (!isLD8n(*MII) || MII->getOperand(0).getReg() != Z80::A ||
        !MII->getOperand(1).isImm()) {
      ++MII;
      continue;
    }

    auto It2 = MBB.SkipPHIsLabelsAndDebug(std::next(MII));
    // skip if block ends before the store
    if (It2 == MIE)
      break;
    auto firstAddr = getStoreAddr(*It2);
    // skip if It2 is not a direct store of A to memory
    if (!firstAddr) {
      ++MII;
      continue;
    }

    struct Pair {
      MachineInstr *LdAn;
      MachineInstr *LdNnA;
      uint8_t Imm;
    };
    SmallVector<Pair, 8> Run;
    Run.push_back({&*MII, &*It2, (uint8_t)MII->getOperand(1).getImm()});

    auto It3 = MBB.SkipPHIsLabelsAndDebug(std::next(It2));
    while (It3 != MIE) {
      if (!isLD8n(*It3) || It3->getOperand(0).getReg() != Z80::A ||
          !It3->getOperand(1).isImm())
        break;
      auto It4 = MBB.SkipPHIsLabelsAndDebug(std::next(It3));
      if (It4 == MIE)
        break;
      auto a = getStoreAddr(*It4);
      if (!a)
        break;
      bool Matches = false;
      int64_t ExpectedOff = firstAddr->Off + (int64_t)Run.size();
      if (firstAddr->GV && a->GV == firstAddr->GV && a->Off == ExpectedOff)
        Matches = true;
      else if (firstAddr->SymbolName && a->SymbolName &&
               StringRef(a->SymbolName) == firstAddr->SymbolName &&
               a->Off == ExpectedOff)
        Matches = true;
      else if (!firstAddr->GV && !firstAddr->SymbolName && !a->GV &&
               !a->SymbolName && a->Off == ExpectedOff)
        Matches = true;

      if (!Matches)
        break;

      Run.push_back({&*It3, &*It4, (uint8_t)It3->getOperand(1).getImm()});
      It3 = MBB.SkipPHIsLabelsAndDebug(std::next(It4));
    }

    // Minimum 3 consecutive stores required for size win
    if (Run.size() >= 3) {
      // H and L must be dead before the chain (LD HL clobbers HL)
      auto HQ = MBB.computeRegisterLiveness(TRI, Z80::H, MII);
      auto LQ = MBB.computeRegisterLiveness(TRI, Z80::L, MII);
      if (HQ != MachineBasicBlock::LQR_Dead || LQ != MachineBasicBlock::LQR_Dead) {
        MII = std::next(It2);
        continue;
      }

      auto AfterLastStore = MBB.SkipPHIsLabelsAndDebug(std::next(MachineBasicBlock::iterator(Run.back().LdNnA)));
      // HL must be dead after the chain
      if (!isRegDeadAfter(AfterLastStore, MBB, TRI, Z80::HL)) {
        MII = std::next(It2);
        continue;
      }
      // A must be dead after the chain
      if (!isRegDeadAfter(AfterLastStore, MBB, TRI, Z80::A)) {
        MII = std::next(It2);
        continue;
      }

      LLVM_DEBUG(dbgs() << "  folding " << Run.size()
                        << " consecutive stores to (HL)\n");

      DebugLoc DL = MII->getDebugLoc();
      BuildMI(MBB, MII, DL, TII->get(Z80::LD_rr_nn), Z80::HL)
          .add(Run[0].LdNnA->getOperand(0));
      for (size_t k = 0; k < Run.size(); ++k) {
        BuildMI(MBB, MII, DL, TII->get(Z80::LD_HLind_n)).addImm(Run[k].Imm);
        if (k + 1 < Run.size())
          Z80::buildIncDec16(MBB, MII, DL, *TII, Z80::INC_rr, Z80::HL);
      }

      for (auto &P : Run) {
        P.LdAn->eraseFromParent();
        P.LdNnA->eraseFromParent();
      }
      ++NumConsecutiveStores;
      Changed = true;
      MII = It3;
      continue;
    }

    MII = std::next(It2);
  }

  return Changed;
}

// --- Peephole #18/#206: `LD r, n` → `LD r, r'` when r' already holds n ---
//
// When any tracked 8-bit register r' already holds constant n, emit the
// cheaper `LD r, r'` (1 B, 4 T) instead of `LD r, n` (2 B, 7 T), saving 1 B
// and 3 T per fire.
// Source preference: A first (so existing XOR A zero-init sequences keep their
// canonical shape), then B, C, D, E, H, L in register-file order.
// Tracking is strictly within one basic block; any def of a register
// (including calls and RegMask clobbers) invalidates that register's entry.
//
// Worked example (from issue-206-const-reuse-non-a.mir @single_reuse):
//   Input MIR:
//     $b = LD_r_n 7
//     $c = LD_r_n 7
//     RET implicit $b, implicit $c
//   Execution:
//     $b = LD_r_n 7 records KnownVal[B] = 7.
//     $c = LD_r_n 7 queries tracker for constant 7 -> finds B.
//   Output:
//     $b = LD_r_n 7
//     $c = LD_r_r $b
//     RET implicit $b, implicit $c
//
// Soundness guards:
//   - Only replaces LD r, n when Dst != Src (no self-copies `LD r, r`).
//   - Calls and unmodeled side effects clear all known constants.
//   - Any instruction defining or clobbering an overlapping register
//     invalidates that register's tracked constant.
static bool optimizeConstantReuse(MachineBasicBlock &MBB,
                                  const TargetInstrInfo *TII,
                                  const TargetRegisterInfo *TRI) {
  static const MCPhysReg GR8Regs[] = {
      Z80::A, Z80::B, Z80::C, Z80::D, Z80::E, Z80::H, Z80::L};

  bool Changed = false;
  int64_t KnownVal[7];
  std::fill(std::begin(KnownVal), std::end(KnownVal), -1);

  auto setKnown = [&](MCPhysReg Reg, int64_t Val) {
    for (int i = 0; i < 7; ++i) {
      if (GR8Regs[i] == Reg) {
        KnownVal[i] = Val & 0xFF;
        return;
      }
    }
  };

  auto clearKnown = [&](MCPhysReg Reg) {
    for (int i = 0; i < 7; ++i) {
      if (TRI->regsOverlap(GR8Regs[i], Reg))
        KnownVal[i] = -1;
    }
  };

  for (auto MII = MBB.begin(); MII != MBB.end();) {
    MachineInstr &MI = *MII;

    // Calls and unmodeled side effects may clobber arbitrary registers
    if (MI.isCall() || MI.hasUnmodeledSideEffects()) {
      std::fill(std::begin(KnownVal), std::end(KnownVal), -1);
      ++MII;
      continue;
    }

    // Match 8-bit immediate load: LD r, n
    if (isLD8n(MI)) {
      Register Dst = MI.getOperand(0).getReg();
      if (Dst.isPhysical() && MI.getOperand(1).isImm()) {
        int64_t N = MI.getOperand(1).getImm() & 0xFF;
        MCPhysReg FoundSrc = 0;
        for (int i = 0; i < 7; ++i) {
          // skip self-copy since LD r,r is a no-op that doesn't set value
          if (GR8Regs[i] == Dst)
            continue;
          // select first matching register in priority order (A first)
          if (KnownVal[i] == N) {
            FoundSrc = GR8Regs[i];
            break;
          }
        }
        if (FoundSrc) {
          LLVM_DEBUG(dbgs() << "  LD " << printReg(Dst, TRI) << ", " << N
                            << " -> LD " << printReg(Dst, TRI) << ", "
                            << printReg(FoundSrc, TRI) << "\n");
          DebugLoc DL = MI.getDebugLoc();
          Z80::buildLD8(MBB, MII, DL, *TII, Dst, FoundSrc);
          MII = MBB.erase(MII);
          clearKnown(Dst);
          setKnown(Dst, N);
          ++NumConstReused;
          Changed = true;
          continue;
        }
        // No source found — record that Dst now holds N
        clearKnown(Dst);
        setKnown(Dst, N);
        ++MII;
        continue;
      }
    }

    // Match zeroing of A: XOR A
    if (isZeroA(MI)) {
      clearKnown(Z80::A);
      setKnown(Z80::A, 0);
      for (const MachineOperand &MO : MI.operands()) {
        if (MO.isReg() && MO.isDef() && MO.getReg().isValid() &&
            MO.getReg().isPhysical() && MO.getReg() != Z80::A)
          clearKnown(MO.getReg());
      }
      ++MII;
      continue;
    }

    // Propagate constant across register copy: LD r, r'
    if (isLD8(MI)) {
      Register Dst = MI.getOperand(0).getReg();
      Register Src = MI.getOperand(1).getReg();
      if (Dst.isPhysical() && Src.isPhysical()) {
        int64_t SrcVal = -1;
        for (int i = 0; i < 7; ++i) {
          if (GR8Regs[i] == Src) {
            SrcVal = KnownVal[i];
            break;
          }
        }
        clearKnown(Dst);
        if (SrcVal != -1)
          setKnown(Dst, SrcVal);
        ++MII;
        continue;
      }
    }

    // Invalidate any defined or clobbered registers
    for (const MachineOperand &MO : MI.operands()) {
      if (MO.isRegMask()) {
        for (MCPhysReg R : GR8Regs) {
          if (MO.clobbersPhysReg(R))
            clearKnown(R);
        }
      } else if (MO.isReg() && MO.isDef() && MO.getReg().isValid() &&
                 MO.getReg().isPhysical()) {
        clearKnown(MO.getReg());
      }
    }
    for (MCPhysReg Def : TII->get(MI.getOpcode()).implicit_defs())
      clearKnown(Def);

    ++MII;
  }

  if (Changed)
    recomputeLivenessFlags(MBB);

  return Changed;
}

// --- Keep a saved register on the stack across untouched stretches ---
//
// Consecutive stack accesses each save and restore a live HL around
// themselves, producing POP rr ... PUSH rr with nothing in between that
// cares. When the stretch touches neither rr nor SP (removing the pair
// leaves SP two lower there, so any SP-relative access would slip), the
// value can simply stay on the stack. The adjacent-pair case is handled
// by the POP/PUSH peephole above; this is its windowed extension.
static bool elidePopPushAcrossStretch(MachineBasicBlock &MBB,
                                      const TargetInstrInfo *TII,
                                      const TargetRegisterInfo *TRI) {
  static const struct {
    unsigned PopOpc;
    unsigned PushOpc;
    MCPhysReg Reg;
  } Pairs[] = {
      {Z80::POP_BC, Z80::PUSH_BC, Z80::BC},
      {Z80::POP_DE, Z80::PUSH_DE, Z80::DE},
      {Z80::POP_HL, Z80::PUSH_HL, Z80::HL},
  };
  bool Changed = false;
  for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
    auto Next = std::next(MII);
    for (const auto &P : Pairs) {
      if (MII->getOpcode() != P.PopOpc)
        continue;
      unsigned Budget = 32;
      for (auto J = Next; J != MIE && Budget--; ++J) {
        if (J->getOpcode() == P.PushOpc) {
          // The pop also refills the register itself; anything reading it
          // after the push would see stale contents without the pair.
          if (!isRegDeadAfter(std::next(J), MBB, TRI, P.Reg))
            break;
          LLVM_DEBUG(dbgs() << "  Pop/push elision across stretch: " << *MII);
          MBB.erase(J);
          Next = MBB.erase(MII);
          ++NumPopPushElided;
          Changed = true;
          break;
        }
        // Push and pop model their SP movement through getSPAdjust, not
        // operands, so ask both ways: anything that moves or even reads SP
        // would see it two bytes short inside the shortened stretch.
        if (J->isCall() || J->isBranch() || J->isTerminator() ||
            J->isInlineAsm() || J->readsRegister(P.Reg, TRI) ||
            J->modifiesRegister(P.Reg, TRI) || TII->getSPAdjust(*J) != 0 ||
            J->readsRegister(Z80::SP, TRI) || J->modifiesRegister(Z80::SP, TRI))
          break;
      }
      break;
    }
    MII = Next;
  }
  if (Changed)
    recomputeLivenessFlags(MBB);
  return Changed;
}

// The mirror of the pass above: PUSH rr ... POP rr saves a pair across a
// stretch that never writes it, so the pair is doing nothing. What keeps the
// pass above from taking it is SP: everything the stretch reached through
// the stack was addressed with those two bytes already counted in. LDHL SP,e
// carries its own displacement, so taking two off each one puts the stretch
// back where it was.
//
// A displacement of less than two would have been addressing the saved value
// itself, which is not something the frame layout produces, and is refused
// rather than reasoned about.
static bool elidePushPopAcrossStretch(MachineBasicBlock &MBB,
                                      const TargetInstrInfo *TII,
                                      const TargetRegisterInfo *TRI) {
  static const struct {
    unsigned PushOpc;
    unsigned PopOpc;
    MCPhysReg Reg;
  } Pairs[] = {
      {Z80::PUSH_BC, Z80::POP_BC, Z80::BC},
      {Z80::PUSH_DE, Z80::POP_DE, Z80::DE},
      {Z80::PUSH_HL, Z80::POP_HL, Z80::HL},
  };
  bool Changed = false;
  for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
    auto Next = std::next(MII);
    for (const auto &P : Pairs) {
      if (MII->getOpcode() != P.PushOpc)
        continue;
      SmallVector<MachineInstr *, 8> Rebase;
      unsigned Budget = 16;
      for (auto J = Next; J != MIE && Budget--; ++J) {
        // LDHL SP,e writes HL, so it is only rebasable when HL is not the
        // pair being saved: for that one, the pop is what puts it back.
        if (J->getOpcode() == Z80::LDHL_SP_e && P.Reg != Z80::HL) {
          if (SignExtend64<8>(J->getOperand(0).getImm()) < 2)
            break;
          Rebase.push_back(&*J);
          continue;
        }
        if (J->getOpcode() == P.PopOpc) {
          for (MachineInstr *MI : Rebase) {
            int64_t Disp = SignExtend64<8>(MI->getOperand(0).getImm()) - 2;
            MI->getOperand(0).setImm(Disp & 0xFF);
          }
          LLVM_DEBUG(dbgs() << "  Push/pop elision across stretch: " << *MII);
          MBB.erase(J);
          Next = MBB.erase(MII);
          ++NumPushPopElided;
          Changed = true;
          break;
        }
        // The saved value only has to survive: a stretch that reads the pair
        // reads what it already holds. Push and pop model their SP movement
        // through getSPAdjust rather than operands, so ask both ways.
        if (J->isCall() || J->isBranch() || J->isTerminator() ||
            J->isInlineAsm() || J->modifiesRegister(P.Reg, TRI) ||
            TII->getSPAdjust(*J) != 0 || J->readsRegister(Z80::SP, TRI) ||
            J->modifiesRegister(Z80::SP, TRI))
          break;
      }
      break;
    }
    MII = Next;
  }
  if (Changed)
    recomputeLivenessFlags(MBB);
  return Changed;
}

// --- SM83: route (HL) accesses through A for the post-increment form ---
//
//   LD A,(HL); INC HL  -> LD A,(HL+)              (2B/4M -> 1B/2M)
//   LD (HL),A; INC HL  -> LD (HL+),A              (2B/4M -> 1B/2M)
//   LD r,(HL); INC HL  -> LD A,(HL+); LD r,A      (6M -> 5M, A dead)
//   LD (HL),r; INC HL  -> LD A,r; LD (HL+),A      (4M -> 3M, A dead)
//
// The non-A forms trade nothing in size and touch no flags; they only
// need A free to carry the value.
static bool fusePostIncAccess(MachineBasicBlock &MBB,
                              const TargetInstrInfo *TII,
                              const TargetRegisterInfo *TRI) {
  bool Changed = false;
  for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
    MachineInstr &MI = *MII;
    auto Next = std::next(MII);
    if (Next == MIE || !isIncDec16(*Next, Z80::INC_rr, Z80::HL)) {
      MII = Next;
      continue;
    }
    auto After = std::next(Next);
    const DebugLoc &DL = MI.getDebugLoc();
    // A pair the register allocator filled in one half only reaches here as a
    // byte store of the empty half, and the trip through A re-expresses that
    // read without giving it a value.
    SmallVector<MCRegister, 4> Empty;
    Z80::collectUndefReads(MI, TRI, Empty);
    bool AtStart = MII == MBB.begin();
    auto Prev = AtStart ? MBB.end() : std::prev(MII);

    if (isLoadHL(MI, Z80::A)) {
      BuildMI(MBB, MII, DL, TII->get(Z80::LD_A_HLI));
    } else if (isStoreHL(MI, Z80::A)) {
      BuildMI(MBB, MII, DL, TII->get(Z80::LD_HLI_A));
    } else if (Register Dst = getLoadHLindDstReg(MI);
               Dst && isRegDeadAfter(After, MBB, TRI, Z80::A)) {
      BuildMI(MBB, MII, DL, TII->get(Z80::LD_A_HLI));
      Z80::buildLD8(MBB, MII, DL, *TII, Dst, Z80::A);
    } else if (Register Src = getStoreHLindSrcReg(MI);
               Src && Src != Z80::A &&
               isRegDeadAfter(After, MBB, TRI, Z80::A)) {
      Z80::buildLD8(MBB, MII, DL, *TII, Z80::A, Src);
      BuildMI(MBB, MII, DL, TII->get(Z80::LD_HLI_A));
    } else {
      MII = Next;
      continue;
    }
    LLVM_DEBUG(dbgs() << "  Post-inc fuse: " << MI);
    Z80::markEmptyReads(AtStart ? MBB.begin() : std::next(Prev), MII, TRI,
                        Empty);
    MBB.erase(MII);
    MBB.erase(Next);
    MII = After;
    ++NumPostIncFused;
    Changed = true;
  }
  if (Changed)
    recomputeLivenessFlags(MBB);
  return Changed;
}

// --- SM83: buy constant stores a trip through A ---
//
// LD (HL),n costs two bytes per store and cannot post-increment; through A
// the store is one byte and fuses with a following INC HL into LD (HL+),A.
// Within a stretch where A carries nothing (no reads or writes), stores of
// one constant repeat often enough (array and struct initialization) that
// materializing the constant once pays for itself:
//   cost: XOR A = 1 byte (needs dead flags) or LD A,n = 2 bytes
//   gain: 1 byte per plain store, 2 bytes per store+INC HL pair
static bool materializeConstantStores(MachineBasicBlock &MBB,
                                      const TargetInstrInfo *TII,
                                      const TargetRegisterInfo *TRI) {
  bool Changed = false;

  auto WindowEnd = MBB.begin();
  while (WindowEnd != MBB.end()) {
    // Find the next maximal window with no genuine use or def of A.
    auto WindowBegin = WindowEnd;
    while (WindowBegin != MBB.end() &&
           (WindowBegin->isCall() || WindowBegin->isInlineAsm() ||
            WindowBegin->readsRegister(Z80::A, TRI) ||
            WindowBegin->modifiesRegister(Z80::A, TRI)))
      ++WindowBegin;
    WindowEnd = WindowBegin;
    SmallVector<MachineBasicBlock::iterator, 8> Stores;
    while (WindowEnd != MBB.end() && !WindowEnd->isCall() &&
           !WindowEnd->isInlineAsm() &&
           !WindowEnd->readsRegister(Z80::A, TRI) &&
           !WindowEnd->modifiesRegister(Z80::A, TRI)) {
      if (WindowEnd->getOpcode() == Z80::LD_HLind_n)
        Stores.push_back(WindowEnd);
      ++WindowEnd;
    }
    if (Stores.empty())
      continue;

    // The window not touching A is not enough: A may be carrying a value
    // straight through it to a reader beyond, which our constant would
    // clobber. Only proceed when A is dead past the window.
    if (!isRegDeadAfter(WindowEnd, MBB, TRI, Z80::A))
      continue;

    // Group the stores by constant and convert each group that profits.
    SmallVector<std::pair<int64_t, unsigned>, 4> Groups; // value, saving
    for (auto It : Stores) {
      int64_t V = It->getOperand(0).getImm() & 0xFF;
      bool Fused = std::next(It) != MBB.end() &&
                   isIncDec16(*std::next(It), Z80::INC_rr, Z80::HL);
      auto *G = llvm::find_if(Groups, [&](auto &P) { return P.first == V; });
      if (G == Groups.end())
        Groups.push_back({V, Fused ? 2u : 1u});
      else
        G->second += Fused ? 2 : 1;
    }
    // Convert only the best group: a second constant's LD A,n could land
    // between the first group's converted stores and corrupt what A holds.
    llvm::sort(Groups, [](auto &L, auto &R) { return L.second > R.second; });
    Groups.truncate(1);
    for (auto &G : Groups) {
      const int64_t Value = G.first;
      const unsigned Saving = G.second;
      auto FirstIt = *llvm::find_if(Stores, [&](auto It) {
        return (It->getOperand(0).getImm() & 0xFF) == Value;
      });
      bool FlagsDead = isRegDeadAfter(FirstIt, MBB, TRI, Z80::FLAGS);
      unsigned Cost = (Value == 0 && FlagsDead) ? 1 : 2;
      if (Saving <= Cost)
        continue;

      // Materialize the constant once, before its first store.
      const DebugLoc &DL = FirstIt->getDebugLoc();
      if (Value == 0 && FlagsDead)
        Z80::buildZeroA(MBB, FirstIt, DL, *TII);
      else
        Z80::buildLD8n(MBB, FirstIt, DL, *TII, Z80::A).addImm(Value);

      for (auto It : Stores) {
        if ((It->getOperand(0).getImm() & 0xFF) != Value)
          continue;
        auto NextIt = std::next(It);
        if (NextIt != MBB.end() && isIncDec16(*NextIt, Z80::INC_rr, Z80::HL)) {
          BuildMI(MBB, It, It->getDebugLoc(), TII->get(Z80::LD_HLI_A));
          MBB.erase(It);
          MBB.erase(NextIt);
        } else {
          Z80::buildStoreHL(MBB, It, It->getDebugLoc(), *TII, Z80::A);
          MBB.erase(It);
        }
      }
      ++NumConstStores;
      Changed = true;
      LLVM_DEBUG(dbgs() << "  A invest: constant " << Value << " saves "
                        << (Saving - Cost) << "B\n");
    }
  }

  if (Changed)
    recomputeLivenessFlags(MBB);
  return Changed;
}

// The IX-indexed store of an immediate is four bytes, one more than the
// store of A. Investing in A once, where nothing else needs it, pays for
// itself from the third store of a value on, or the second when the value
// is zero and XOR A can produce it.
static bool materializeIXConstantStores(MachineBasicBlock &MBB,
                                        const TargetInstrInfo *TII,
                                        const TargetRegisterInfo *TRI) {
  bool Changed = false;

  auto WindowEnd = MBB.begin();
  while (WindowEnd != MBB.end()) {
    auto Touches = [&](MachineBasicBlock::iterator It) {
      return It->isCall() || It->isInlineAsm() ||
             It->readsRegister(Z80::A, TRI) ||
             It->modifiesRegister(Z80::A, TRI);
    };
    auto WindowBegin = WindowEnd;
    while (WindowBegin != MBB.end() && Touches(WindowBegin))
      ++WindowBegin;
    WindowEnd = WindowBegin;
    SmallVector<MachineBasicBlock::iterator, 8> Stores;
    while (WindowEnd != MBB.end() && !Touches(WindowEnd)) {
      if (WindowEnd->getOpcode() == Z80::LD_IXd_n)
        Stores.push_back(WindowEnd);
      ++WindowEnd;
    }
    if (Stores.empty())
      continue;

    // A may be carrying a value straight through the window to a reader
    // beyond it, which the constant would destroy.
    if (!isRegDeadAfter(WindowEnd, MBB, TRI, Z80::A))
      continue;

    // Only the most repeated value: a second constant's LD A,n would land
    // between the first group's converted stores and corrupt what A holds.
    DenseMap<int64_t, unsigned> Counts;
    for (auto It : Stores)
      ++Counts[It->getOperand(1).getImm() & 0xFF];
    int64_t Value = 0;
    unsigned Saving = 0;
    for (auto &C : Counts)
      if (C.second > Saving) {
        Value = C.first;
        Saving = C.second;
      }

    auto FirstIt = *llvm::find_if(Stores, [&](auto It) {
      return (It->getOperand(1).getImm() & 0xFF) == Value;
    });
    bool FlagsDead = isRegDeadAfter(FirstIt, MBB, TRI, Z80::FLAGS);
    unsigned Cost = (Value == 0 && FlagsDead) ? 1 : 2;
    if (Saving <= Cost)
      continue;

    const DebugLoc &DL = FirstIt->getDebugLoc();
    if (Value == 0 && FlagsDead)
      Z80::buildZeroA(MBB, FirstIt, DL, *TII);
    else
      Z80::buildLD8n(MBB, FirstIt, DL, *TII, Z80::A).addImm(Value);

    for (auto It : Stores) {
      if ((It->getOperand(1).getImm() & 0xFF) != Value)
        continue;
      Z80::buildStoreIdx(MBB, It, It->getDebugLoc(), *TII, Z80::LD_IXd_r,
                         It->getOperand(0).getImm(), Z80::A);
      MBB.erase(It);
    }
    ++NumIXConstStores;
    Changed = true;
    LLVM_DEBUG(dbgs() << "  A invest (IX): constant " << Value << " saves "
                      << (Saving - Cost) << "B\n");
  }

  if (Changed)
    recomputeLivenessFlags(MBB);
  return Changed;
}

static bool reuseLDHLAddress(MachineBasicBlock &MBB, const TargetInstrInfo *TII,
                             const TargetRegisterInfo *TRI) {
  bool Changed = false;
  bool Known = false;  // Whether HL = SP + Off holds here.
  int64_t Off = 0;     // Relative to the CURRENT SP.
  bool AKnown = false; // Whether A holds the constant AVal here.
  int64_t AVal = 0;

  for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
    MachineInstr &MI = *MII;
    unsigned Opc = MI.getOpcode();
    auto Next = std::next(MII);

    // --- A-constant reuse: a store of a constant A already holds can go
    // through A (half the size), and with a following INC HL it fuses
    // into the post-increment store. Reloading the same constant into A
    // is dropped outright.
    if (Opc == Z80::LD_HLind_n && AKnown &&
        (MI.getOperand(0).getImm() & 0xFF) == (AVal & 0xFF)) {
      if (Next != MIE && isIncDec16(*Next, Z80::INC_rr, Z80::HL)) {
        LLVM_DEBUG(dbgs() << "  A reuse: fusing to (hl+) " << MI);
        BuildMI(MBB, MII, MI.getDebugLoc(), TII->get(Z80::LD_HLI_A));
        auto AfterInc = std::next(Next);
        MBB.erase(MII);
        MBB.erase(Next);
        MII = AfterInc;
        Off += 1; // LD (HL+),A moved HL exactly as the INC HL did.
        ++NumLDHLReused;
        Changed = true;
        continue;
      }
      LLVM_DEBUG(dbgs() << "  A reuse: store via A " << MI);
      Z80::buildStoreHL(MBB, MII, MI.getDebugLoc(), *TII, Z80::A);
      MII = MBB.erase(MII);
      ++NumLDHLReused;
      Changed = true;
      continue;
    }
    if (getLD8nDst(MI) == Z80::A && AKnown && MI.getOperand(1).isImm() &&
        (MI.getOperand(1).getImm() & 0xFF) == (AVal & 0xFF)) {
      // LD A,n leaves flags alone, so the reload can simply go.
      LLVM_DEBUG(dbgs() << "  A reuse: erasing reload " << MI);
      MII = MBB.erase(MII);
      ++NumLDHLReused;
      Changed = true;
      continue;
    }
    if (isZeroA(MI) && AKnown && (AVal & 0xFF) == 0 &&
        isRegDeadAfter(Next, MBB, TRI, Z80::FLAGS)) {
      LLVM_DEBUG(dbgs() << "  A reuse: erasing xor a " << MI);
      MII = MBB.erase(MII);
      ++NumLDHLReused;
      Changed = true;
      continue;
    }

    // Track what A holds. LD A,n can carry a link-time symbol byte instead
    // of an immediate; its value is unknown here, so it falls through to
    // the modifiesRegister case and invalidates the tracking.
    if (getLD8nDst(MI) == Z80::A && MI.getOperand(1).isImm()) {
      AKnown = true;
      AVal = MI.getOperand(1).getImm() & 0xFF;
    } else if (isZeroA(MI)) {
      AKnown = true;
      AVal = 0;
    } else if (MI.isCall() || MI.isInlineAsm() ||
               MI.modifiesRegister(Z80::A, TRI)) {
      AKnown = false;
    }

    if (Opc == Z80::LDHL_SP_e) {
      // The stored immediate is the masked byte of a signed displacement.
      int64_t N = SignExtend64<8>(MI.getOperand(0).getImm() & 0xFF);
      if (Known && isRegDeadAfter(Next, MBB, TRI, Z80::FLAGS)) {
        int64_t D = N - Off;
        if (D == 0) {
          LLVM_DEBUG(dbgs() << "  LDHL reuse: erasing " << MI);
          MII = MBB.erase(MII);
          ++NumLDHLReused;
          Changed = true;
          continue;
        }
        if (D == 1 || D == -1) {
          LLVM_DEBUG(dbgs() << "  LDHL reuse: inc/dec for " << MI);
          Z80::buildIncDec16(MBB, MII, MI.getDebugLoc(), *TII,
                             D == 1 ? Z80::INC_rr : Z80::DEC_rr, Z80::HL);
          MII = MBB.erase(MII);
          Off = N;
          ++NumLDHLReused;
          Changed = true;
          continue;
        }
      }
      Known = true;
      Off = N;
      MII = Next;
      continue;
    }

    if (isIncDec16(MI, Z80::INC_rr, Z80::HL)) {
      Off += 1;
      MII = Next;
      continue;
    }
    if (isIncDec16(MI, Z80::DEC_rr, Z80::HL)) {
      Off -= 1;
      MII = Next;
      continue;
    }

    switch (Opc) {
    case Z80::LD_HLI_A:
    case Z80::LD_A_HLI:
      Off += 1;
      break;
    case Z80::LD_HLD_A:
    case Z80::LD_A_HLD:
      Off -= 1;
      break;
    // HL is pinned to an absolute address, so SP movement shifts the
    // SP-relative offset in the opposite direction.
    case Z80::PUSH_AF:
    case Z80::PUSH_BC:
    case Z80::PUSH_DE:
    case Z80::PUSH_HL:
      Off += 2;
      break;
    case Z80::POP_AF:
    case Z80::POP_BC:
    case Z80::POP_DE:
      Off -= 2;
      break;
    case Z80::INC_SP:
      Off -= 1;
      break;
    case Z80::DEC_SP:
      Off += 1;
      break;
    case Z80::ADD_SP_e:
      Off -= SignExtend64<8>(MI.getOperand(0).getImm() & 0xFF);
      break;
    case Z80::LD_SP_HL: // SP := HL, so HL = SP + 0.
      Off = 0;
      break;
    default:
      // POP_HL loads an unknown value; calls leave SP itself unknowable
      // (callee cleanup); anything else touching HL or SP ends tracking.
      if (MI.isCall() || MI.isInlineAsm() ||
          MI.modifiesRegister(Z80::HL, TRI) ||
          MI.modifiesRegister(Z80::SP, TRI))
        Known = false;
      break;
    }
    MII = Next;
  }

  if (Changed)
    recomputeLivenessFlags(MBB);
  return Changed;
}

/// --- Tail call optimization: CALL nn; RET -> JP nn ---
///
/// WHAT:
/// When a function's last action is CALL_nn followed by RET (or RET_CLEANUP 0),
/// replace the CALL + RET pair with TAILJMP target (lowered to JP target).
///
/// WHY:
/// A CALL pushes a 2-byte return address to the stack, branches to the callee,
/// and upon return immediately executes RET which pops the caller's return
/// address. By jumping directly to the callee (JP), the callee's RET returns
/// directly to the original caller, saving:
///   - 1 byte in code size (JP is 3 bytes, CALL+RET is 3+1 = 4 bytes)
///   - 17 T-states (CALL=17T, RET=10T -> 27T; JP=10T, difference = 17T saved).
///
/// SOUNDNESS & ORDERING:
/// 1. Stack arguments: If the caller pushed arguments onto the stack for the
///    callee, SP is offset. A normal CALL would have the return address at
///    SP=SP_initial-2, with callee arguments below it. A bare JP would cause the
///    callee's RET to pop the top argument instead of the return address!
///    Guard: ensure NO PUSH instructions exist in the MBB before CALL.
/// 2. Epilogue: If the function has a stack frame (e.g., dynamic frame with
///    frame pointer or callee-saved registers), frame tear-down instructions
///    (POP IX, ADD SP, n) sit between the last logical action and RET. Because
///    this pass requires CALL_nn to be immediately adjacent to RET (or
///    cross-block fall-through directly to bare RET), functions with active
///    frame tear-down are naturally excluded without special casing.
/// 3. TAILJMP is marked isReturn + isTerminator + isBarrier + isCall, but NOT
///    isBranch. This prevents BranchRelaxation and BranchCleanup from mistaking
///    it for an internal control flow branch with MBB targets.
///
/// WORKED EXAMPLE:
/// Input:
///   _test_tailcall_simple:
///     call _callee_void
///     ret
/// Data structures:
///   Term: RET (opcode Z80::RET)
///   CallIt: CALL_nn (opcode Z80::CALL_nn, operand 0 = @_callee_void)
///   HasPush: false
/// Output:
///   _test_tailcall_simple:
///     TAILJMP @_callee_void  ; lowered to "jp _callee_void" in Z80AsmPrinter
static bool optimizeTailCalls(MachineFunction &MF, const TargetInstrInfo *TII,
                              const TargetRegisterInfo *TRI,
                              const Z80Subtarget &STI) {
  // Tail calls save 1 byte and 17 T-states, but change CALL; RET to JP.
  // When optimizing for minimum size (-Oz / minsize attribute), enable this
  // transformation unconditionally. For general builds, keep standard call/ret
  // to preserve full ABI stack frames and avoid regressing standard test suites.
  if (!MF.getFunction().hasMinSize())
    return false;

  bool Changed = false;

  // Single-MBB form: CALL_nn immediately followed by RET or RET_CLEANUP 0.
  for (auto &MBB : MF) {
    auto Term = MBB.getLastNonDebugInstr();
    if (Term == MBB.end())
      continue; // Skip empty blocks.

    // Match RET or RET_CLEANUP 0.
    unsigned TermOpc = Term->getOpcode();
    bool IsRet = (TermOpc == Z80::RET);
    bool IsRetCleanup0 = (TermOpc == Z80::RET_CLEANUP &&
                          Term->getOperand(0).getImm() == 0);
    if (!IsRet && !IsRetCleanup0)
      continue; // Block does not end with a zero-cleanup return.

    // Check for CALL_nn immediately before RET.
    auto CallIt = Term;
    if (CallIt == MBB.begin())
      continue; // No preceding instruction in block.
    --CallIt;
    while (CallIt != MBB.begin() && CallIt->isDebugInstr())
      --CallIt;
    if (CallIt->getOpcode() != Z80::CALL_nn)
      continue; // Preceding instruction is not a direct CALL_nn.

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
      continue; // Caller pushed stack arguments; unsafe to tail-call.

    // Replace CALL nn; RET with TAILJMP (JP to external function).
    MachineOperand &CallTarget = CallIt->getOperand(0);
    LLVM_DEBUG(dbgs() << "  CALL; RET -> JP (tail call): " << *CallIt);
    DebugLoc DL = CallIt->getDebugLoc();
    BuildMI(MBB, *CallIt, DL, TII->get(Z80::TAILJMP)).add(CallTarget);
    Term->eraseFromParent();
    CallIt->eraseFromParent();
    ++NumTailCalls;
    Changed = true;
  }

  // Cross-MBB form:
  // When an MBB ends with CALL_nn (no explicit branch) and falls through
  // to an MBB whose first instruction is RET, the CALL can become a
  // TAILJMP -- the callee's RET will return directly to our caller.
  for (auto &MBB : MF) {
    auto Term = MBB.getLastNonDebugInstr();
    if (Term == MBB.end() || Term->getOpcode() != Z80::CALL_nn)
      continue; // Block must end with CALL_nn.
    if (MBB.succ_size() != 1)
      continue; // Must have a single fall-through successor.
    MachineBasicBlock *Next = *MBB.succ_begin();
    auto NextFirst = Next->getFirstNonDebugInstr();
    if (NextFirst == Next->end() || NextFirst->getOpcode() != Z80::RET)
      continue; // Successor must begin with bare RET.

    // Stack-args safety check.
    bool HasPush = false;
    for (auto It = MBB.begin(); It != Term; ++It) {
      if (It->isDebugInstr())
        continue;
      unsigned Opc = It->getOpcode();
      if (Opc == Z80::PUSH_AF || Opc == Z80::PUSH_BC ||
          Opc == Z80::PUSH_DE || Opc == Z80::PUSH_HL ||
          Opc == Z80::PUSH_IX || Opc == Z80::PUSH_IY) {
        HasPush = true;
        break;
      }
    }
    if (HasPush)
      continue; // Caller pushed stack arguments; unsafe to tail-call.

    // Replace CALL with TAILJMP, drop the fall-through to the RET MBB.
    LLVM_DEBUG(dbgs() << "  CALL -> JP (cross-MBB tail call): " << *Term);
    MachineOperand &CallTarget = Term->getOperand(0);
    DebugLoc DL = Term->getDebugLoc();
    BuildMI(MBB, *Term, DL, TII->get(Z80::TAILJMP)).add(CallTarget);
    Term->eraseFromParent();
    // TAILJMP is isReturn, so no fall-through happens. Remove CFG edge.
    MBB.removeSuccessor(Next);
    ++NumTailCalls;
    Changed = true;
  }

  return Changed;
}

/// --- Peephole: CP/XOR with 1 or 0xFF → DEC_A/INC_A (when A dead) ---
/// Z80 has 1-byte equivalents of the equality tests A == 1 and
/// A == 0xFF when A's modified value isn't needed afterward:
///   `DEC A`  (1 B) sets Z iff A was 1
///   `INC A`  (1 B) sets Z iff A was 0xFF
/// vs `{CP,XOR}_n K` (2 B).  `OR A` (1 B) for A == 0 already fires.
/// This closes K ∈ {1, 0xFF}.  ravn/llvm-z80#148.
static bool optimizeDecIncEquality(MachineFunction &MF,
                                   const TargetInstrInfo *TII,
                                   const TargetRegisterInfo *TRI) {
  if (MF.empty())
    return false;

  bool Changed = false;

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

  auto targetDeadA = [&TRI](MachineBasicBlock *TargetMBB) -> bool {
    if (!TargetMBB)
      return false;
    // Walk target's instructions: if A is defined before being
    // read, it was dead at entry; if it's read before defined,
    // it's live at entry.  If we reach a terminator without seeing
    // either, fall through to the MBB's liveouts.
    for (const MachineInstr &MI : *TargetMBB) {
      if (MI.isDebugInstr())
        continue;
      // XOR_A is the canonical "clear A" idiom: `xor a` zeros A.
      if (isZeroA(MI))
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
    bool BlockChanged = false;
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
        NewOpc = Z80::DEC_r;
      else if (K == 0xFF)
        NewOpc = Z80::INC_r;
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
      // explicit A-dead check.
      if (MachineBasicBlock *Fall = MBB.getNextNode()) {
        if (!targetDeadA(Fall))
          continue;
      }
      // FLAGS must be dead after the branch too.  CP/XOR set C
      // (and other flags) but DEC_A / INC_A leave C unchanged.
      if (!isRegDeadAfter(AfterBr, MBB, TRI, Z80::FLAGS))
        continue;
      if (IsJp) {
        MachineBasicBlock *TargetMBB = BrIt->getOperand(0).getMBB();
        bool TgtFlagsOK = false;
        for (const MachineInstr &MI : *TargetMBB) {
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
            TgtFlagsOK = true;
            break;
          }
        }
        if (!TgtFlagsOK)
          continue;
      }

      DebugLoc DL = OpIt->getDebugLoc();
      Z80::buildIncDec8(MBB, *OpIt, DL, *TII, NewOpc, Z80::A);
      OpIt->eraseFromParent();
      ++NumDecIncEquality;
      Changed = BlockChanged = true;
    }
    if (BlockChanged)
      recomputeLivenessFlags(MBB);
  }
  return Changed;
}

// --- Pass: Redundant LD A,r removal across basic blocks (issue #60) ---
//
// When a value is saved into an 8-bit register via `LD r, A` and A is not
// subsequently modified, reloading A via `LD A, r` is redundant.
// Forward dataflow tracks whether A currently holds the value of register r.
//
// WHY it's there:
// Regalloc frequently emits `LD r, A` before a comparison or branch, and then
// redundantly reloads `LD A, r` in successor blocks (e.g. fdc_get_result_bytes).
// Because CP, OR A, branches, and other non-A-modifying instructions preserve
// A, the reload is a pure no-op.
//
// Soundness & Ordering:
// - Runs AFTER per-MBB peepholes and DJNZ optimization. In-block loop counters
//   (LD A,B; DEC A; LD B,A) must NOT have their initial load removed prematurely.
// - Erasing `LD A, r` makes A live across the block boundary. We explicitly add
//   A to MBB's live-in set if the erased load was the reaching def in that block,
//   and recompute kill/liveness flags for the block and its predecessors.
//
// Worked example (from redundant-ld-a-reg.ll @cross_block_chain):
//   entry:
//     CALL _compute
//     LD B, A             ; A and B hold same value
//     CP #2               ; CP sets flags, leaves A unchanged
//     JR NZ, .LBB0_2
//     LD A, B             ; <-- REDUNDANT, removed (saves 1 B)
//     RET
//   .LBB0_2:
//     LD A, B             ; <-- REDUNDANT, removed (saves 1 B)
//     OR A                ; OR A sets flags, leaves A unchanged
//     JR Z, .LBB0_4
//     LD A, B             ; <-- REDUNDANT, removed (saves 1 B)
//     LD (_g8), A
//     RET
static bool eliminateRedundantLdAR(MachineFunction &MF,
                                   const TargetRegisterInfo *TRI) {
  if (MF.empty())
    return false;

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

  auto isReg8 = [](Register R) {
    return R == Z80::B || R == Z80::C || R == Z80::D || R == Z80::E ||
           R == Z80::H || R == Z80::L;
  };

  auto step60 = [&](MachineInstr &MI, AK Known, bool *Redundant) -> AK {
    if (Redundant) *Redundant = false;
    if (MI.isDebugInstr())
      return Known;
    unsigned Opc = MI.getOpcode();
    if (Opc == TargetOpcode::KILL || Opc == TargetOpcode::IMPLICIT_DEF)
      return Known;

    // LD A, r - sets Known to r. Redundant if A already equals r.
    if (isLD8(MI)) {
      Register Dst = MI.getOperand(0).getReg();
      Register Src = MI.getOperand(1).getReg();
      if (Dst == Z80::A && isReg8(Src)) {
        if (Known.Kind == AK_Reg && Known.Reg == Src && Redundant)
          *Redundant = true;
        return akReg(Src.asMCReg());
      }
      // LD r, A - A's value is now also in r.
      if (Src == Z80::A && isReg8(Dst))
        return akReg(Dst.asMCReg());
      if (Dst == Z80::A && Src == Z80::A)
        return Known;
    }

    // OR A and AND A are idempotent on A's value; they only update FLAGS.
    if (isAlu8(MI, Z80::OR_r, Z80::A) || isAlu8(MI, Z80::AND_r, Z80::A))
      return Known;

    // CP r / CP n only sets flags, leaves A unchanged.
    if (Opc == Z80::CP_r || Opc == Z80::CP_n || Opc == Z80::CP_HLind)
      return Known;

    // Check for clobbers of A or the tracked register.
    bool ClobberA = false, ClobberKnown = false;
    for (const MachineOperand &MO : MI.operands()) {
      if (MO.isRegMask()) {
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

  // Dataflow fixpoint iteration.
  DenseMap<MachineBasicBlock *, AK> EntryAK, ExitAK;
  for (auto &MBB : MF) {
    EntryAK[&MBB] = akTop();
    ExitAK[&MBB] = akTop();
  }
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
        K = step60(MI, K, nullptr);
      if (!(ExitAK[BB] == K)) {
        ExitAK[BB] = K;
        DfChanged = true;
      }
    }
  }

  // Collection pass: identify redundant LD A, r instructions.
  SmallVector<MachineInstr *, 16> ToErase;
  for (auto &MBB : MF) {
    AK K = EntryAK[&MBB];
    for (auto &MI : MBB) {
      bool Red = false;
      AK Next = step60(MI, K, &Red);
      if (Red)
        ToErase.push_back(&MI);
      K = Next;
    }
  }

  if (ToErase.empty())
    return false;

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

  SmallPtrSet<MachineBasicBlock *, 8> AffectedMBBs;
  for (auto &MBB : MF) {
    bool NeedALiveIn = false;
    for (auto &MI : MBB) {
      if (llvm::is_contained(ToErase, &MI)) {
        NeedALiveIn = true;
        break;
      }
      if (defsPhysA(MI))
        break;
    }
    if (NeedALiveIn) {
      if (!MBB.isLiveIn(Z80::A))
        MBB.addLiveIn(Z80::A);
      AffectedMBBs.insert(&MBB);
      for (auto *Pred : MBB.predecessors())
        AffectedMBBs.insert(Pred);
    }
  }

  for (auto *MI : ToErase) {
    LLVM_DEBUG(dbgs() << "  Redundant LD A,r removed: " << *MI);
    AffectedMBBs.insert(MI->getParent());
    MI->eraseFromParent();
    ++NumRedundantLdAR;
  }

  for (auto *MBB : AffectedMBBs)
    recomputeLivenessFlags(*MBB);

  return true;
}

// --- Peephole: Carry Flag Roundtrip to Branch Elimination (issue #93) ---
//
// WHAT it does:
//   Folds roundtrips where a Carry Flag produced by arithmetic (such as
//   ADD HL,rr or ADD A,n) is captured into register A via SBC A,A; AND 1,
//   optionally inverted (XOR 1) and/or rotated back to CF (RRCA), and finally
//   tested by a conditional branch (JR/JP C, NC, Z, NZ).
//   When A and intermediate flags are dead after the branch, eliminates the
//   SBC/AND/[XOR]/[RRCA] sequence and branches directly on the original CF.
//
// WHY it's there:
//   GlobalISel lowers comparisons and overflow checks (e.g. `add i16 %t, 1;
//   icmp eq %t.next, 0` for `do { ... } while (++t);`) by capturing CF into an
//   s1 register in A via ADD_HL_rr_CO (SBC A,A; AND 1).
//   When the branch condition later tests this s1, it rotates bit 0 back into
//   CF via RRCA (or tests Z after XOR 1).
//   Any intervening instructions (such as `LD c,l; LD b,h` saving the counter)
//   do not alter FLAGS. Roundtripping CF -> A -> CF costs 4 to 6 bytes per loop!
//   Branching directly on CF saves 4 to 6 bytes per site.
//
// Worked example (from fdc_write_when_ready / test_u16_loop_overflow):
//   Before:
//     add hl, bc        ; sets CF on wrap to 0 (hl+bc >= 0x10000)
//     sbc a, a          ; A = CF ? 0xFF : 0x00
//     and 1             ; A = CF ? 0x01 : 0x00
//     ld c, l           ; flag-neutral counter save
//     ld b, h           ; flag-neutral counter save
//     xor 1             ; A = CF ? 0x00 : 0x01
//     rrca              ; CF_new = bit 0 of A (inverted original CF)
//     jr c, .loop       ; loop if CF_new == 1 (i.e. original CF == 0)
//   After:
//     add hl, bc        ; sets CF on wrap to 0
//     ld c, l           ; flag-neutral counter save
//     ld b, h           ; flag-neutral counter save
//     jr nc, .loop      ; loop directly if original CF == 0! (-6 bytes)
static bool optimizeCarryFlagRoundtrip(MachineFunction &MF,
                                       const TargetInstrInfo *TII,
                                       const TargetRegisterInfo *TRI) {
  bool Changed = false;

  auto isSbcAA = [](const MachineInstr &MI) {
    return MI.getOpcode() == Z80::SBC_A_r &&
           MI.getNumOperands() >= 1 &&
           MI.getOperand(0).isReg() &&
           MI.getOperand(0).getReg() == Z80::A;
  };

  auto isAndOne = [](const MachineInstr &MI) {
    return MI.getOpcode() == Z80::AND_n &&
           MI.getNumOperands() >= 1 &&
           MI.getOperand(0).isImm() &&
           (MI.getOperand(0).getImm() & 0xFF) == 1;
  };

  auto isXorOne = [](const MachineInstr &MI) {
    return MI.getOpcode() == Z80::XOR_n &&
           MI.getNumOperands() >= 1 &&
           MI.getOperand(0).isImm() &&
           (MI.getOperand(0).getImm() & 0xFF) == 1;
  };

  auto isOrA = [](const MachineInstr &MI) {
    return MI.getOpcode() == Z80::OR_r &&
           MI.getNumOperands() >= 1 &&
           MI.getOperand(0).isReg() &&
           MI.getOperand(0).getReg() == Z80::A;
  };

  for (MachineBasicBlock &MBB : MF) {
    bool BlockChanged = false;
    for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE;) {
      MachineInstr &SbcMI = *MII;
      if (!isSbcAA(SbcMI)) {
        ++MII;
        continue;
      }

      auto SbcIt = MII;
      auto AndIt = std::next(SbcIt);
      if (AndIt == MIE || !isAndOne(*AndIt)) {
        ++MII;
        continue;
      }

      // Verify that an instruction before SbcIt defines FLAGS and CF has not
      // been clobbered between that producer and SbcIt.
      auto ProdIt = SbcIt;
      bool FoundProducer = false;
      while (ProdIt != MBB.begin()) {
        --ProdIt;
        if (ProdIt->isDebugInstr())
          continue;
        if (ProdIt->definesRegister(Z80::FLAGS, TRI)) {
          FoundProducer = true;
          break;
        }
        // Intervening instructions before SbcIt must not read or clobber FLAGS.
        if (ProdIt->readsRegister(Z80::FLAGS, TRI))
          break;
      }
      if (!FoundProducer) {
        ++MII;
        continue;
      }

      SmallVector<MachineInstr *, 5> ToErase;
      ToErase.push_back(&*SbcIt);
      ToErase.push_back(&*AndIt);

      bool HasXorOne = false;
      bool HasRrca = false;
      MachineBasicBlock::iterator BranchIt = MBB.end();
      bool ValidChain = true;

      for (auto It = std::next(AndIt); It != MIE; ++It) {
        if (It->isDebugInstr())
          continue;

        if (!HasXorOne && !HasRrca && isXorOne(*It)) {
          HasXorOne = true;
          ToErase.push_back(&*It);
          continue;
        }

        if (!HasRrca && It->getOpcode() == Z80::RRCA) {
          HasRrca = true;
          ToErase.push_back(&*It);
          continue;
        }

        // OR A is redundant flag test from A often emitted before JR NZ/Z
        if (isOrA(*It)) {
          ToErase.push_back(&*It);
          continue;
        }

        if (It->isBranch()) {
          BranchIt = It;
          break;
        }

        // Mid instructions must be completely flag-neutral and A-neutral.
        if (It->readsRegister(Z80::FLAGS, TRI) ||
            It->definesRegister(Z80::FLAGS, TRI) ||
            It->readsRegister(Z80::A, TRI) ||
            It->definesRegister(Z80::A, TRI) ||
            It->isCall() || It->hasUnmodeledSideEffects()) {
          ValidChain = false;
          break;
        }
      }

      if (!ValidChain || BranchIt == MBB.end()) {
        ++MII;
        continue;
      }

      unsigned CurBranchOpc = BranchIt->getOpcode();
      unsigned NewBranchOpc = 0;

      if (HasRrca) {
        if (HasXorOne) {
          // CF is inverted
          switch (CurBranchOpc) {
          case Z80::JR_C_e:  NewBranchOpc = Z80::JR_NC_e; break;
          case Z80::JR_NC_e: NewBranchOpc = Z80::JR_C_e; break;
          case Z80::JP_C_nn: NewBranchOpc = Z80::JP_NC_nn; break;
          case Z80::JP_NC_nn: NewBranchOpc = Z80::JP_C_nn; break;
          default: break;
          }
        } else {
          // CF is identical
          switch (CurBranchOpc) {
          case Z80::JR_C_e:  NewBranchOpc = Z80::JR_C_e; break;
          case Z80::JR_NC_e: NewBranchOpc = Z80::JR_NC_e; break;
          case Z80::JP_C_nn: NewBranchOpc = Z80::JP_C_nn; break;
          case Z80::JP_NC_nn: NewBranchOpc = Z80::JP_NC_nn; break;
          default: break;
          }
        }
      } else if (HasXorOne) {
        // Zero flag from XOR 1:
        // CF=1 -> A=1 -> XOR 1 -> A=0 (Z)
        // CF=0 -> A=0 -> XOR 1 -> A=1 (NZ)
        switch (CurBranchOpc) {
        case Z80::JR_NZ_e:  NewBranchOpc = Z80::JR_NC_e; break;
        case Z80::JR_Z_e:   NewBranchOpc = Z80::JR_C_e; break;
        case Z80::JP_NZ_nn: NewBranchOpc = Z80::JP_NC_nn; break;
        case Z80::JP_Z_nn:  NewBranchOpc = Z80::JP_C_nn; break;
        default: break;
        }
      } else {
        // Zero flag from AND 1:
        // CF=1 -> A=1 (NZ)
        // CF=0 -> A=0 (Z)
        switch (CurBranchOpc) {
        case Z80::JR_NZ_e:  NewBranchOpc = Z80::JR_C_e; break;
        case Z80::JR_Z_e:   NewBranchOpc = Z80::JR_NC_e; break;
        case Z80::JP_NZ_nn: NewBranchOpc = Z80::JP_C_nn; break;
        case Z80::JP_Z_nn:  NewBranchOpc = Z80::JP_NC_nn; break;
        default: break;
        }
      }

      if (!NewBranchOpc) {
        ++MII;
        continue;
      }

      // Safety guard: A must be dead after the branch.
      if (!isRegDeadAfter(std::next(BranchIt), MBB, TRI, Z80::A)) {
        ++MII;
        continue;
      }

      // Safety guard: if there are instructions after BranchIt in MBB,
      // FLAGS must be dead after BranchIt.
      if (std::next(BranchIt) != MIE &&
          !isRegDeadAfter(std::next(BranchIt), MBB, TRI, Z80::FLAGS)) {
        ++MII;
        continue;
      }

      LLVM_DEBUG(dbgs() << "  Carry flag roundtrip fold: " << *BranchIt
                        << " -> opcode " << NewBranchOpc << "\n");

      if (NewBranchOpc != CurBranchOpc)
        BranchIt->setDesc(TII->get(NewBranchOpc));

      // Advance MII past SbcIt to next non-erased instruction or BranchIt
      auto ResumeIt = std::next(AndIt);
      while (ResumeIt != MIE && is_contained(ToErase, &*ResumeIt))
        ++ResumeIt;
      MII = ResumeIt;

      for (MachineInstr *MI : ToErase) {
        LLVM_DEBUG(dbgs() << "    erasing: " << *MI);
        MI->eraseFromParent();
      }

      ++NumCarryRoundtrips;
      Changed = BlockChanged = true;
    }

    if (BlockChanged)
      recomputeLivenessFlags(MBB);
  }

  return Changed;
}

// --- Pass: JP -> JR branch shortening (issue #58) ---
// Convert all unconditional JP and conditional JP (Z, NZ, C, NC) instructions
// with MachineBasicBlock targets to their 2-byte JR equivalents.
//
// WHY it's there:
// InstructionSelector emits 3-byte JP instructions (JP_nn, JP_cc_nn) for all
// branches. While BranchFolder rematerializes some branches as JR via
// insertBranch, branches that bypass BranchFolder (such as tight self-loops,
// unfolded blocks, or error handlers) retain the 3-byte JP encoding.
//
// Soundness & Ordering:
// BranchRelaxation runs immediately after this pass in addPreEmitPass.
// Any JR whose target is beyond +-127 bytes will be relaxed back to JP by
// BranchRelaxation. Converting greedily to JR here guarantees that every branch
// that CAN fit in +-127 bytes uses the compact 2-byte form, saving 1 byte each.
// Must run after tail-call optimization so tail jumps (TAILJMP / external JP)
// are already finalized.
//
// Worked example (from issue-58-branch-shortening.ll @halt):
//   .LBB0_1:
//     JP .LBB0_1     ; 3 B (ISel emit for self-loop `for (;;);`)
//   Rewritten to:
//     JR .LBB0_1     ; 2 B (saves 1 B; in-range displacement = -2)
static bool shortenBranches(MachineFunction &MF, const TargetInstrInfo *TII) {
  bool Changed = false;
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      unsigned NewOpc = 0;
      switch (MI.getOpcode()) {
      case Z80::JP_nn:    NewOpc = Z80::JR_e; break;
      case Z80::JP_Z_nn:  NewOpc = Z80::JR_Z_e; break;
      case Z80::JP_NZ_nn: NewOpc = Z80::JR_NZ_e; break;
      case Z80::JP_C_nn:  NewOpc = Z80::JR_C_e; break;
      case Z80::JP_NC_nn: NewOpc = Z80::JR_NC_e; break;
      default: continue; // not a conditional/unconditional relative-capable JP
      }
      // Only convert MBB-target branches, never function symbols or indirects.
      if (MI.getNumOperands() == 0 || !MI.getOperand(0).isMBB())
        continue; // skip non-MBB jump targets (e.g. TAILJMP, symbol targets)
      LLVM_DEBUG(dbgs() << "  JP->JR shortening: " << MI);
      MI.setDesc(TII->get(NewOpc));
      ++NumBranchesShortened;
      Changed = true;
    }
  }
  return Changed;
}

bool Z80PreEmitPeephole::runOnMachineFunction(MachineFunction &MF) {
  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  const auto *TII = STI.getInstrInfo();
  const auto *TRI = STI.getRegisterInfo();
  bool Changed = false;

  Changed |= optimizeDecIncEquality(MF, TII, TRI);

  for (MachineBasicBlock &MBB : MF) {
    // The peepholes written inline below move reads past the point where
    // register allocation recorded a value as dying, which leaves the kill and
    // dead flags describing a live range that ended too early. The peepholes
    // kept in their own functions each restate the flags before returning;
    // these share the one call at the end of the block.
    bool BlockChanged = false;

    // Dropping a no-op is what leaves the constant behind it dead, so this
    // comes before the sweep.
    Changed |= elideZeroOperandLogic(MBB, TII, TRI);
    // Before the rest: they all reason about what a register holds, and a
    // definition nobody reads only muddies that.
    Changed |= eraseDeadFrameReloads(MBB, TRI);
    Changed |= foldCopyIntoFrameAccess(MBB, TII, TRI);

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
        ++NumPopPushPairs;
        Changed = BlockChanged = true;
        Matched = true;
        break;
      }
      if (!Matched)
        ++MII;
    }

    // --- Peephole: LD A,r; DEC/INC A; LD r,A; [OR A;] [JR cc] -> DEC/INC r; [JR cc] ---
    // Replaces the 3-to-5 instruction register inc/dec round-trip through accumulator A:
    //   LD A, r; DEC A; LD r, A; OR A; JR NZ -> DEC r; JR NZ (or DJNZ)
    //   LD A, r; DEC A; LD r, A; JR Z/NZ -> DEC r; JR Z/NZ
    //   LD A, r; DEC A; LD r, A -> DEC r (when A is dead)
    //   LD A, r; INC A; LD r, A -> INC r (when A is dead)
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      // Match: LD A,r (identify counter register r)
      Register CounterReg = getLD8Src(*MII, Z80::A);
      if (!CounterReg.isValid() || CounterReg == Z80::A ||
          !Z80::isEncodableGR8(CounterReg)) {
        ++MII;
        continue;
      }
      auto I1 = MII;
      auto I2 = MBB.SkipPHIsLabelsAndDebug(std::next(I1));
      if (I2 == MIE) {
        ++MII;
        continue;
      }
      bool IsDec = isIncDec8(*I2, Z80::DEC_r, Z80::A);
      bool IsInc = isIncDec8(*I2, Z80::INC_r, Z80::A);
      if (!IsDec && !IsInc) {
        ++MII;
        continue;
      }
      auto I3 = MBB.SkipPHIsLabelsAndDebug(std::next(I2));
      if (I3 == MIE || !isLD8(*I3, CounterReg, Z80::A)) {
        ++MII;
        continue;
      }

      unsigned IncDecOpc = IsDec ? Z80::DEC_r : Z80::INC_r;
      DebugLoc DL = I1->getDebugLoc();

      // Look ahead for possible redundant [OR A] and branch
      auto I4 = MBB.SkipPHIsLabelsAndDebug(std::next(I3));
      if (I4 != MIE && isAlu8(*I4, Z80::OR_r, Z80::A)) {
        auto I5 = MBB.SkipPHIsLabelsAndDebug(std::next(I4));
        if (I5 != MIE &&
            (I5->getOpcode() == Z80::JR_NZ_e || I5->getOpcode() == Z80::JR_Z_e ||
             I5->getOpcode() == Z80::JP_NZ_nn || I5->getOpcode() == Z80::JP_Z_nn) &&
            isRegDeadAfter(std::next(I5), MBB, TRI, Z80::A) &&
            isRegDeadAfter(std::next(I5), MBB, TRI, Z80::FLAGS)) {
          // Case 1: LD A,r; DEC/INC A; LD r,A; OR A; JR/JP cc
          // Eliminate OR A because DEC/INC r sets Z flag identically
          I4->eraseFromParent();
          I3->eraseFromParent();
          I2->eraseFromParent();
          MII = MBB.erase(I1);
          Z80::buildIncDec8(MBB, MII, DL, *TII, IncDecOpc, CounterReg);
          ++NumDecInPlace;
          Changed = BlockChanged = true;
          continue;
        }
      }

      // Case 2: LD A,r; DEC/INC A; LD r,A; JR/JP cc (without OR A)
      if (I4 != MIE &&
          (I4->getOpcode() == Z80::JR_NZ_e || I4->getOpcode() == Z80::JR_Z_e ||
           I4->getOpcode() == Z80::JP_NZ_nn || I4->getOpcode() == Z80::JP_Z_nn) &&
          isRegDeadAfter(std::next(I4), MBB, TRI, Z80::A)) {
        I3->eraseFromParent();
        I2->eraseFromParent();
        MII = MBB.erase(I1);
        Z80::buildIncDec8(MBB, MII, DL, *TII, IncDecOpc, CounterReg);
        ++NumDecInPlace;
        Changed = BlockChanged = true;
        continue;
      }

      // Case 3: Standalone LD A,r; DEC/INC A; LD r,A (no immediate branch)
      if (isRegDeadAfter(std::next(I3), MBB, TRI, Z80::A)) {
        I3->eraseFromParent();
        I2->eraseFromParent();
        MII = MBB.erase(I1);
        Z80::buildIncDec8(MBB, MII, DL, *TII, IncDecOpc, CounterReg);
        ++NumDecInPlace;
        Changed = BlockChanged = true;
        continue;
      }

      ++MII;
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
          ++NumCplFolds;
          Changed = BlockChanged = true;
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
      if (getLD8nDst(MI) == Z80::A && MI.getOperand(1).isImm() &&
          MI.getOperand(1).getImm() == 0) {
        auto After = std::next(MII);
        if (isRegDeadAfter(After, MBB, TRI, Z80::FLAGS)) {
          LLVM_DEBUG(dbgs() << "  LD A,#0 → XOR A: " << MI);
          Z80::buildZeroA(MBB, MI, MI.getDebugLoc(), *TII);
          MII = MBB.erase(MII);
          ++NumZeroAFolds;
          Changed = BlockChanged = true;
          continue;
        }
      }
      ++MII;
    }

    // --- Peephole: AND $1 / $80 + branch/ret → RRCA / RLCA + carry branch/ret ---
    // AND $1 (2B) tests bit 0 via Z flag. RRCA (1B) rotates bit 0 into carry.
    // Replace AND $1; JR/JP NZ → RRCA; JR/JP C (saves 1B).
    // Replace AND $1; RET NZ → RRCA; RET C (saves 1B).
    // Similarly AND $80; ... NZ → RLCA; ... C (bit 7 to carry).
    // Constraint: A must be dead after the branch/ret on fall-through & target.
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      unsigned Opc = MII->getOpcode();
      if (Opc != Z80::AND_n) {
        ++MII;
        continue;
      }

      int64_t Imm = MII->getOperand(0).getImm() & 0xFF;
      unsigned RotOpc;
      if (Imm == 1)
        RotOpc = Z80::RRCA; // bit 0 → carry
      else if (Imm == 0x80)
        RotOpc = Z80::RLCA; // bit 7 → carry
      else {
        ++MII;
        continue;
      }

      auto Next = std::next(MII);
      if (Next == MIE) {
        ++MII;
        continue;
      }

      // Skip redundant OR A between AND and branch (re-tests Z flag).
      auto BranchIt = Next;
      if (isAlu8(*BranchIt, Z80::OR_r, Z80::A)) {
        BranchIt = std::next(BranchIt);
        if (BranchIt == MIE) {
          ++MII;
          continue;
        }
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
      if (!NewNextOpc) {
        ++MII;
        continue;
      }

      // A function returning a non-void value may use A for the return value.
      if ((NextOpc == Z80::RET_NZ || NextOpc == Z80::RET_Z) &&
          !MF.getFunction().getReturnType()->isVoidTy()) {
        ++MII;
        continue;
      }

      // A must be dead after the branch/ret on the fall-through and target paths.
      auto AfterBranch = std::next(BranchIt);
      if (!isRegDeadAfter(AfterBranch, MBB, TRI, Z80::A)) {
        ++MII;
        continue;
      }

      LLVM_DEBUG(dbgs() << "  AND→RRCA/RLCA peephole: " << *MII);
      DebugLoc DL = MII->getDebugLoc();

      if (BranchIt != Next)
        Next->eraseFromParent();

      BranchIt->setDesc(TII->get(NewNextOpc));

      BuildMI(MBB, MII, DL, TII->get(RotOpc));
      MII = MBB.erase(MII);
      ++NumAndRotateFolds;
      Changed = BlockChanged = true;
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
        ++NumAluImmMerges;
        Changed = BlockChanged = true;
        continue;
      }
      ++MII;
    }

    // --- Peephole: LD r, r (self-copy) → (remove) ---
    // A register copy to itself is a 1-byte, 4-T no-op that modifies no flags.
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      MachineInstr &MI = *MII;
      if (isLD8(MI) &&
          MI.getOperand(0).getReg() == MI.getOperand(1).getReg()) {
        LLVM_DEBUG(dbgs() << "  Removing redundant self-copy: " << MI);
        MII = MBB.erase(MII);
        Changed = BlockChanged = true;
        continue;
      }
      ++MII;
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
        Register Pair = getLD16nDst(MI);
        bool IsBC = Pair == Z80::BC;
        bool IsDE = Pair == Z80::DE;
        if (!IsBC && !IsDE) {
          ++MII;
          continue;
        }
        if (!MI.getOperand(1).isImm()) {
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
        if (!isStoreHL(*I3, IsBC ? Z80::C : Z80::E)) {
          ++MII;
          continue;
        }

        auto I4 = std::next(I3);
        if (I4 == MIE || !isIncDec16(*I4, Z80::INC_rr, Z80::HL)) {
          ++MII;
          continue;
        }
        auto I5 = std::next(I4);
        if (I5 == MIE) {
          ++MII;
          continue;
        }
        if (!isStoreHL(*I5, IsBC ? Z80::B : Z80::D)) {
          ++MII;
          continue;
        }

        // Register pair must be dead after the store sequence.
        MCPhysReg PairReg = IsBC ? Z80::BC : Z80::DE;
        if (!isRegDeadAfter(std::next(I5), MBB, TRI, PairReg)) {
          ++MII;
          continue;
        }

        int64_t Imm = MI.getOperand(1).getImm();
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
        ++NumImm16Stores;
        Changed = BlockChanged = true;
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

        LLVM_DEBUG(dbgs() << "  LDHL SP,#" << Offset2 << " → "
                          << (Diff == 1 ? "INC" : "DEC") << " HL\n");
        Z80::buildIncDec16(MBB, *It, It->getDebugLoc(), *TII,
                           Diff == 1 ? Z80::INC_rr : Z80::DEC_rr, Z80::HL);
        It->eraseFromParent();
        ++NumLDHLStepped;
        Changed = BlockChanged = true;
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
        Register Pair = getLD16nDst(MI);
        bool IsBC = Pair == Z80::BC;
        bool IsDE = Pair == Z80::DE;
        if (!IsBC && !IsDE) {
          ++MII;
          continue;
        }
        if (!MI.getOperand(1).isImm()) {
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
        Register I2Src = getLD8Src(*I2, Z80::A);
        if (!I2Src.isValid() && !isLoadHL(*I2, Z80::A)) {
          ++MII;
          continue;
        }

        auto I3 = std::next(I2);
        if (I3 == MIE) {
          ++MII;
          continue;
        }
        if (!isAlu8(*I3, Z80::XOR_r, IsBC ? Z80::B : Z80::D)) {
          ++MII;
          continue;
        }

        auto I4 = std::next(I3);
        if (I4 == MIE || !isLD8(*I4, Z80::B, Z80::A)) {
          ++MII;
          continue;
        }

        auto I5 = std::next(I4);
        if (I5 == MIE) {
          ++MII;
          continue;
        }
        // I5: LD A,Y (load low byte of compared value)
        Register I5Src = getLD8Src(*I5, Z80::A);
        if (!I5Src.isValid() && !isLoadHL(*I5, Z80::A)) {
          ++MII;
          continue;
        }

        auto I6 = std::next(I5);
        if (I6 == MIE) {
          ++MII;
          continue;
        }
        if (!isAlu8(*I6, Z80::XOR_r, IsBC ? Z80::C : Z80::E)) {
          ++MII;
          continue;
        }

        auto I7 = std::next(I6);
        if (I7 == MIE || !isAlu8(*I7, Z80::OR_r, Z80::B)) {
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

        int64_t Imm = MI.getOperand(1).getImm();
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
          bool Folded = false;
          if (I2Src.isValid()) {
            // Skip LD B,B (self-move NOP when I2Src == B).
            if (I2Src != Z80::B)
              Z80::buildLD8(MBB, *I2, I2->getDebugLoc(), *TII, Z80::B, I2Src);
            Folded = true;
          } else if (isLoadHL(*I2, Z80::A)) {
            Z80::buildLoadHL(MBB, *I2, I2->getDebugLoc(), *TII, Z80::B);
            Folded = true;
          }
          if (Folded) {
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
        ++NumCmpImmFolds;
        Changed = BlockChanged = true;
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

        // Direct r=A patterns: 2B → 1B (size + speed win)
        unsigned NewOpc = 0;
        if (isIncDec16(*NextIt, Z80::INC_rr, Z80::HL)) {
          if (isLoadHL(MI, Z80::A))
            NewOpc = Z80::LD_A_HLI;
          else if (isStoreHL(MI, Z80::A))
            NewOpc = Z80::LD_HLI_A;
        } else if (isIncDec16(*NextIt, Z80::DEC_rr, Z80::HL)) {
          if (isLoadHL(MI, Z80::A))
            NewOpc = Z80::LD_A_HLD;
          else if (isStoreHL(MI, Z80::A))
            NewOpc = Z80::LD_HLD_A;
        }

        // A pair the register allocator filled in one half only reaches here
        // as a byte access reading nothing, and the replacement re-expresses
        // that read without giving it a value.
        SmallVector<MCRegister, 4> Empty;
        Z80::collectUndefReads(MI, TRI, Empty);
        bool AtStart = MII == MBB.begin();
        auto Prev = AtStart ? MBB.end() : std::prev(MII);
        auto newRange = [&] { return AtStart ? MBB.begin() : std::next(Prev); };

        if (NewOpc) {
          LLVM_DEBUG(dbgs() << "  LD+INC/DEC HL → LD (HL+/-): " << MI);
          BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(NewOpc));
          Z80::markEmptyReads(newRange(), MII, TRI, Empty);
          NextIt->eraseFromParent();
          MII = MBB.erase(MII);
          ++NumPostIncLoads;
          Changed = BlockChanged = true;
          continue;
        }

        // Extended r!=A patterns: 2B → 2B (speed win only, saves 4T)
        // LD r,(HL); INC/DEC HL → LD A,(HL+/-); LD r,A  (requires A dead)
        // LD (HL),r; INC/DEC HL → LD A,r; LD (HL+/-),A  (requires A dead)
        if (isIncDec16(*NextIt, Z80::INC_rr, Z80::HL) ||
            isIncDec16(*NextIt, Z80::DEC_rr, Z80::HL)) {
          bool IsInc = isIncDec16(*NextIt, Z80::INC_rr, Z80::HL);

          Register LoadDst = getLoadHLindDstReg(MI);
          Register StoreSrc = getStoreHLindSrcReg(MI);
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
              Register HiReg = (LoadDst == Z80::C)   ? Z80::B
                               : (LoadDst == Z80::E) ? Z80::D
                                                     : Register();
              if (HiReg.isValid() && isLoadHL(*I3, HiReg)) {
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
                Z80::buildLD8(MBB, MI, DL, *TII, LoadDst, Z80::A);
              } else {
                // LD (HL),r; INC/DEC HL → LD A,r; LD (HL+/-),A
                LLVM_DEBUG(dbgs() << "  LD (HL),r+INC/DEC → HL+/-: " << MI);
                Z80::buildLD8(MBB, MI, DL, *TII, Z80::A, StoreSrc);
                BuildMI(MBB, MI, DL, TII->get(HLSOpc));
              }

              Z80::markEmptyReads(newRange(), MII, TRI, Empty);
              NextIt->eraseFromParent();
              MII = MBB.erase(MII);
              ++NumPostIncLoads;
              Changed = BlockChanged = true;
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

        // I1: LD C,(HL) or LD E,(HL)
        bool IsBC = isLoadHL(MI, Z80::C);
        bool IsDE = isLoadHL(MI, Z80::E);
        if (!IsBC && !IsDE) {
          ++MII;
          continue;
        }

        auto I2 = std::next(MII);
        if (I2 == MIE || !isIncDec16(*I2, Z80::INC_rr, Z80::HL)) {
          ++MII;
          continue;
        }
        auto I3 = std::next(I2);
        if (I3 == MIE) {
          ++MII;
          continue;
        }
        if (!isLoadHL(*I3, IsBC ? Z80::B : Z80::D)) {
          ++MII;
          continue;
        }

        auto I4 = std::next(I3);
        if (I4 == MIE) {
          ++MII;
          continue;
        }
        if (!isLD8(*I4, Z80::L, IsBC ? Z80::C : Z80::E)) {
          ++MII;
          continue;
        }

        auto I5 = std::next(I4);
        if (I5 == MIE) {
          ++MII;
          continue;
        }
        if (!isLD8(*I5, Z80::H, IsBC ? Z80::B : Z80::D)) {
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
        Z80::buildLoadHL(MBB, MI, DL, *TII, Z80::H);
        Z80::buildLD8(MBB, MI, DL, *TII, Z80::L, Z80::A);

        I5->eraseFromParent();
        I4->eraseFromParent();
        I3->eraseFromParent();
        I2->eraseFromParent();
        MII = MBB.erase(MII);
        ++NumHLPostIncLoads;
        Changed = BlockChanged = true;
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

      // A store of a register that holds nothing leaves nothing in the slot,
      // so the slot must not be described as holding what that register does.
      auto setSlot = [&](int Off, const SlotVal &S, bool Empty) {
        if (Empty)
          SPSlots.erase(Off);
        else
          SPSlots[Off] = S;
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
        if (Opc == Z80::LDHL_SP_e) {
          int8_t Imm = (int8_t)(MI.getOperand(0).getImm() & 0xFF);
          int AbsOff = SPDelta + Imm;

          auto It1 = std::next(MII);
          if (It1 == MIE) {
            ++MII;
            continue;
          }

          // The accesses this address is about to be used for have to be
          // ordinary frame traffic. The longest sequence matched below is
          // three instructions, so that is as far as this has to look.
          bool Plain = true;
          unsigned Look = 3;
          for (auto L = It1; L != MIE && Look; ++L, --Look)
            if (L->mayLoadOrStore() && !isPlainSlotAccess(*L))
              Plain = false;
          if (!Plain) {
            SPSlots.erase(AbsOff);
            SPSlots.erase(AbsOff + 1);
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
            ++NumSlotForwarded;
            Changed = BlockChanged = true;
            return true;
          };

          // --- 16-bit immediate store: LDHL; LD (HL),#lo; INC HL; LD (HL),#hi
          if (It1->getOpcode() == Z80::LD_HLind_n) {
            auto It2 = std::next(It1);
            if (It2 != MIE && isIncDec16(*It2, Z80::INC_rr, Z80::HL)) {
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
          Register StoreSrc1 = getStoreHLindSrcReg(*It1);
          if (StoreSrc1.isValid()) {
            auto It2 = std::next(It1);
            if (It2 != MIE && isIncDec16(*It2, Z80::INC_rr, Z80::HL)) {
              auto It3 = std::next(It2);
              if (It3 != MIE) {
                Register StoreSrc2 = getStoreHLindSrcReg(*It3);
                if (StoreSrc2.isValid()) {
                  // Try redundant store elimination.
                  if (tryElimRedundantStore(AbsOff, false, StoreSrc1, 0, false,
                                            StoreSrc2, 0, MII, It1, It2, It3))
                    continue;
                  SlotVal SLo, SHi;
                  SLo.Reg = StoreSrc1;
                  SHi.Reg = StoreSrc2;
                  setSlot(AbsOff, SLo, Z80::readsUndef(*It1, StoreSrc1));
                  setSlot(AbsOff + 1, SHi, Z80::readsUndef(*It3, StoreSrc2));
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
            setSlot(AbsOff, S, Z80::readsUndef(*It1, StoreSrc1));
            MII = std::next(It1);
            continue;
          }

          // --- HL+ register store: LDHL; LD A,r; LD (HL+),A; LD (HL),r2
          {
            Register SrcLo = getLD8Src(*It1, Z80::A);
            // Only B/C/D/E — H/L can't be source (LDHL clobbered HL).
            if (SrcLo.isValid() && SrcLo != Z80::H && SrcLo != Z80::L) {
              auto It2 = std::next(It1);
              if (It2 != MIE && It2->getOpcode() == Z80::LD_HLI_A) {
                auto It3 = std::next(It2);
                if (It3 != MIE) {
                  Register StoreSrc2 = getStoreHLindSrcReg(*It3);
                  if (StoreSrc2.isValid()) {
                    // Try redundant store elimination.
                    if (tryElimRedundantStore(AbsOff, false, SrcLo, 0, false,
                                              StoreSrc2, 0, MII, It1, It2, It3))
                      continue;
                    SlotVal SLo, SHi;
                    SLo.Reg = SrcLo;
                    SHi.Reg = StoreSrc2;
                    setSlot(AbsOff, SLo, Z80::readsUndef(*It1, SrcLo));
                    setSlot(AbsOff + 1, SHi, Z80::readsUndef(*It3, StoreSrc2));
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
          Register LoadDst1 = getLoadHLindDstReg(*It1);
          if (LoadDst1.isValid()) {
            auto It2 = std::next(It1);
            if (It2 != MIE && isIncDec16(*It2, Z80::INC_rr, Z80::HL)) {
              auto It3 = std::next(It2);
              if (It3 != MIE) {
                Register LoadDst2 = getLoadHLindDstReg(*It3);
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
                      bool CopyLo = !LoIsNop && Z80::canLD8(LoadDst1, SLo.Reg);
                      bool CopyHi = !HiIsNop && Z80::canLD8(LoadDst2, SHi.Reg);
                      if (!LoIsNop && !CopyLo)
                        CanForward = false;
                      if (!HiIsNop && !CopyHi)
                        CanForward = false;
                      if (CanForward) {
                        bool HiFirst = TRI->regsOverlap(LoadDst1, SHi.Reg);
                        if (HiFirst && TRI->regsOverlap(LoadDst2, SLo.Reg))
                          CanForward = false; // Circular.
                        if (CanForward) {
                          LLVM_DEBUG(dbgs() << "  SM83 fwd 16-bit reg SP+"
                                            << AbsOff << "\n");
                          auto emitCopyHi = [&] {
                            if (CopyHi)
                              Z80::buildLD8(MBB, MI, DL, *TII, LoadDst2,
                                            SHi.Reg);
                          };
                          auto emitCopyLo = [&] {
                            if (CopyLo)
                              Z80::buildLD8(MBB, MI, DL, *TII, LoadDst1,
                                            SLo.Reg);
                          };
                          if (HiFirst) {
                            emitCopyHi();
                            emitCopyLo();
                          } else {
                            emitCopyLo();
                            emitCopyHi();
                          }
                          It3->eraseFromParent();
                          It2->eraseFromParent();
                          It1->eraseFromParent();
                          MII = MBB.erase(MII);
                          ++NumSlotForwarded;
                          Changed = BlockChanged = true;
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
                      auto slotIsLoadable = [&](Register Dst, SlotVal &S) {
                        if (S.IsImm)
                          return Z80::isEncodableGR8(Dst);
                        return Dst == S.Reg || Z80::canLD8(Dst, S.Reg);
                      };
                      if (!slotIsLoadable(LoadDst1, SLo) ||
                          !slotIsLoadable(LoadDst2, SHi))
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
                          Z80::buildLD8n(MBB, MI, DL, *TII, Dst).addImm(S.Imm);
                        } else if (Dst != S.Reg) {
                          Z80::buildLD8(MBB, MI, DL, *TII, Dst, S.Reg);
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
                      ++NumSlotForwarded;
                      Changed = BlockChanged = true;
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
                if (Z80::isEncodableGR8(LoadDst1)) {
                  LLVM_DEBUG(dbgs() << "  SM83 fwd 8-bit imm SP+" << AbsOff
                                    << " #" << (int)S.Imm << "\n");
                  Z80::buildLD8n(MBB, MI, DL, *TII, LoadDst1).addImm(S.Imm);
                  It1->eraseFromParent();
                  MII = MBB.erase(MII);
                  ++NumSlotForwarded;
                  Changed = BlockChanged = true;
                  Done = true;
                }
              } else if (S.Reg != Z80::H && S.Reg != Z80::L) {
                bool NeedCopy =
                    LoadDst1 != S.Reg && Z80::canLD8(LoadDst1, S.Reg);
                if (LoadDst1 == S.Reg || NeedCopy) {
                  LLVM_DEBUG(dbgs()
                             << "  SM83 fwd 8-bit reg SP+" << AbsOff << "\n");
                  if (NeedCopy)
                    Z80::buildLD8(MBB, MI, DL, *TII, LoadDst1, S.Reg);
                  It1->eraseFromParent();
                  MII = MBB.erase(MII);
                  ++NumSlotForwarded;
                  Changed = BlockChanged = true;
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
      Register StoreSrc = getStoreIXdSrcReg(MI);
      if (StoreSrc.isValid()) {
        int Offset = Z80::idxSlotOperand(MI).getImm();
        if (isPlainSlotAccess(MI) && !Z80::readsUndef(MI, StoreSrc))
          AvailValues[Offset] = StoreSrc;
        else
          AvailValues.erase(Offset);
        continue;
      }

      // Case 2: IX-indexed load — LD R', (IX+d)
      Register LoadDst = getLoadIXdDstReg(MI);
      if (LoadDst.isValid()) {
        int Offset = Z80::idxSlotOperand(MI).getImm();

        if (isPlainSlotAccess(MI)) {
          auto It = AvailValues.find(Offset);
          if (It != AvailValues.end()) {
            MCPhysReg SrcReg = It->second;
            if (LoadDst == SrcReg) {
              // LD R, (IX+d) where R already holds the value — no-op.
              // Don't invalidate anything: R's value doesn't change.
              LLVM_DEBUG(dbgs() << "  Eliminating redundant reload: " << MI);
              MI.eraseFromParent();
              ++NumStoreForwarded;
              Changed = BlockChanged = true;
              continue;
            }
            // Replace LD R', (IX+d) with LD R', R_src.
            if (Z80::canLD8(LoadDst, SrcReg)) {
              LLVM_DEBUG(dbgs() << "  Forwarding: " << MI << "  -> LD "
                                << printReg(LoadDst, TRI) << ", "
                                << printReg(SrcReg, TRI) << "\n");
              // R' gets a new value — invalidate other entries pointing to R'.
              invalidateReg(AvailValues, TRI, LoadDst);
              Z80::buildLD8(MBB, MI, MI.getDebugLoc(), *TII, LoadDst, SrcReg);
              MI.eraseFromParent();
              ++NumStoreForwarded;
              Changed = BlockChanged = true;
              AvailValues[Offset] = LoadDst;
              continue;
            }
          }
        }
        // Couldn't forward — R' gets a new value from memory.
        // Invalidate entries pointing to R' (they're stale).
        invalidateReg(AvailValues, TRI, LoadDst);
        // R' now holds the value at offset d.
        if (isPlainSlotAccess(MI))
          AvailValues[Offset] = LoadDst;
        continue;
      }

      // Case 3: LD (IX+d), n — immediate store to IX slot
      if (Opc == Z80::LD_IXd_n) {
        int Offset = Z80::idxSlotOperand(MI).getImm();
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

    if (BlockChanged)
      recomputeLivenessFlags(MBB);
  }

  Changed |= optimizeBssSpills(MF, TII, TRI, STI);
  Changed |= optimizeCrossClassBssSpills(MF, TII, TRI, STI);
  Changed |= optimizeCrossMbbBssSpills(MF, TII, TRI, STI);
  Changed |= optimizeSPRelativeSpillToPushPop(MF, TII, TRI, STI);

  // Run last: earlier peepholes pattern-match LDHL-based slot accesses
  // (redundant store elimination keys slot identity on them), so the
  // address-reuse rewrite must not obscure those first.
  for (MachineBasicBlock &MBB : MF) {
    if (STI.hasSM83()) {
      Changed |= reuseLDHLAddress(MBB, TII, TRI);
      // After address reuse: it creates the INC HL neighbors these fuse with.
      Changed |= materializeConstantStores(MBB, TII, TRI);
      Changed |= fusePostIncAccess(MBB, TII, TRI);
      // After fusion: the reload it rewrites only takes its LD A,(HL+) form
      // once these two have run.
      Changed |= reloadDirectlyIntoPair(MBB, TII, TRI);
    }
    // The store itself costs the same either way, so the LD A,n that pays
    // for the shorter stores is pure added time: a size-for-speed trade,
    // which is the level that takes those.
    if (!STI.hasSM83() && MF.getFunction().hasMinSize())
      Changed |= materializeIXConstantStores(MBB, TII, TRI);
    Changed |= foldSingleBitMask(MBB, TII, TRI);
    Changed |= directIncDec(MBB, TII, TRI);
    Changed |= optimizeConstantReuse(MBB, TII, TRI);
    Changed |= optimizeI16CompareByteXOR(MBB, TII, TRI, STI);
    Changed |= optimizeInMemoryIncDec(MBB, TII, TRI, STI);
    Changed |= optimizeInMemoryBitSetRes(MBB, TII, TRI, STI);
    Changed |= optimizeConsecutiveStores(MBB, TII, TRI, STI);
    Changed |= optimizeRedundantTestAfterIncDec(MBB, TII, TRI);
    Changed |= optimizeDJNZ(MBB, TII, TRI, STI);
    Changed |= elidePopPushAcrossStretch(MBB, TII, TRI);
    Changed |= elidePushPopAcrossStretch(MBB, TII, TRI);
  }

  Changed |= optimizeTailCalls(MF, TII, TRI, STI);
  Changed |= eliminateRedundantLdAR(MF, TRI);
  Changed |= optimizeCarryFlagRoundtrip(MF, TII, TRI);
  Changed |= shortenBranches(MF, TII);

  return Changed;
}

} // namespace

char Z80PreEmitPeephole::ID = 0;

INITIALIZE_PASS(Z80PreEmitPeephole, DEBUG_TYPE,
                "Z80 pre-emit peephole optimization", false, false)

MachineFunctionPass *llvm::createZ80PreEmitPeepholePass() {
  return new Z80PreEmitPeephole;
}
