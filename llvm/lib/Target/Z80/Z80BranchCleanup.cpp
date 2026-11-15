//===-- Z80BranchCleanup.cpp - Z80 Branch Cleanup -------------------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Cleans up branches after BranchRelaxation and Z80ExpandPseudo.
//
// BranchRelaxation turns an out-of-range JR cc into JR !cc around a block with
// only a JP. The pair becomes JP cc. A conditional branch to a block with only
// a RET becomes RET cc. A conditional branch around a block with only a tail
// call becomes a conditional tail call.
//
// Every change removes bytes. No JR goes out of range afterwards.
//
//===----------------------------------------------------------------------===//

#include "Z80BranchCleanup.h"

#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80InstrInfo.h"
#include "Z80OpcodeUtils.h"
#include "Z80Subtarget.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

#define DEBUG_TYPE "z80-branch-cleanup"

STATISTIC(NumTrampolinesCollapsed,
          "Number of JR_CC/JP trampolines collapsed into JP_CC");
STATISTIC(NumCondReturns, "Number of branches folded into RET cc");
STATISTIC(NumCondTailCalls,
          "Number of branches folded into conditional tail calls");
STATISTIC(NumJumpsToNext, "Number of jumps to the next block removed");
STATISTIC(NumInvertedLoopsRotated, "Number of inverted loop latches rotated");
STATISTIC(NumDJNZFolded, "Number of DEC B + JR NZ folded to DJNZ");

using namespace llvm;

namespace {

static unsigned getOppositeBranchOpcode(unsigned Opc) {
  switch (Opc) {
  case Z80::JR_Z_e:
    return Z80::JR_NZ_e;
  case Z80::JR_NZ_e:
    return Z80::JR_Z_e;
  case Z80::JR_C_e:
    return Z80::JR_NC_e;
  case Z80::JR_NC_e:
    return Z80::JR_C_e;
  case Z80::JP_Z_nn:
    return Z80::JP_NZ_nn;
  case Z80::JP_NZ_nn:
    return Z80::JP_Z_nn;
  case Z80::JP_C_nn:
    return Z80::JP_NC_nn;
  case Z80::JP_NC_nn:
    return Z80::JP_C_nn;
  default:
    return 0;
  }
}

class Z80BranchCleanup : public MachineFunctionPass {
public:
  static char ID;

  Z80BranchCleanup() : MachineFunctionPass(ID) {
    initializeZ80BranchCleanupPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // namespace

/// The JP cc with the condition of the conditional JR \p BrOpc, or 0.
static unsigned getCondJPOpcode(unsigned BrOpc) {
  switch (BrOpc) {
  case Z80::JR_NZ_e:
    return Z80::JP_NZ_nn;
  case Z80::JR_Z_e:
    return Z80::JP_Z_nn;
  case Z80::JR_NC_e:
    return Z80::JP_NC_nn;
  case Z80::JR_C_e:
    return Z80::JP_C_nn;
  default:
    return 0;
  }
}

/// The only instruction of \p MBB besides debug instructions, if its opcode is
/// \p Opc.
static MachineInstr *getOnlyInstr(MachineBasicBlock &MBB, unsigned Opc) {
  auto I = MBB.getFirstNonDebugInstr();
  if (I == MBB.end() || I->getOpcode() != Opc ||
      MBB.getLastNonDebugInstr() != I)
    return nullptr;
  return &*I;
}

/// Whether a terminator of \p MBB can go to \p Succ, or \p MBB falls through
/// to it.
static bool stillReaches(MachineBasicBlock &MBB, MachineBasicBlock *Succ) {
  for (MachineInstr &MI : MBB.terminators()) {
    // The targets of a jump table are not operands of the jump.
    if (MI.isIndirectBranch())
      return true;
    for (const MachineOperand &MO : MI.operands())
      if (MO.isMBB() && MO.getMBB() == Succ)
        return true;
  }
  auto Last = MBB.getLastNonDebugInstr();
  return MBB.isLayoutSuccessor(Succ) &&
         (Last == MBB.end() || !Last->isBarrier());
}

/// Turn each conditional branch of \p MBB to a block with only a RET into
/// RET cc.
static bool foldBranchesToRet(MachineBasicBlock &MBB, const Z80InstrInfo &TII) {
  bool Changed = false;
  for (MachineInstr &Br : make_early_inc_range(MBB.terminators())) {
    unsigned Opc = Z80::getCondRetOpcode(Br.getOpcode());
    if (!Opc)
      continue;
    MachineBasicBlock *RetMBB = Br.getOperand(0).getMBB();
    MachineInstr *Ret = getOnlyInstr(*RetMBB, Z80::RET);
    if (!Ret)
      continue;
    TII.insertCondForm(MBB, Br, Opc, *Ret, Br.getDebugLoc());
    Br.eraseFromParent();
    if (!stillReaches(MBB, RetMBB)) {
      MBB.removeSuccessor(RetMBB);
      if (RetMBB->pred_empty() && !RetMBB->hasAddressTaken() &&
          !RetMBB->isEHPad() && !RetMBB->isEntryBlock())
        RetMBB->eraseFromParent();
    }
    ++NumCondReturns;
    Changed = true;
  }
  return Changed;
}

/// Turn the conditional branch at the end of \p MBB, when it jumps over a
/// block with only \p Opc, into that instruction under the opposite condition.
/// \p GetCondOpc gives the conditional form for a branch opcode.
static bool foldBranchOver(MachineBasicBlock &MBB, const Z80InstrInfo &TII,
                           unsigned Opc, unsigned (*GetCondOpc)(unsigned)) {
  auto Br = MBB.getLastNonDebugInstr();
  if (Br == MBB.end() || !Br->isConditionalBranch())
    return false;
  SmallVector<MachineOperand, 1> Cond = {
      MachineOperand::CreateImm(Br->getOpcode())};
  if (TII.reverseBranchCondition(Cond))
    return false;
  unsigned CondOpc = GetCondOpc(Cond[0].getImm());
  if (!CondOpc)
    return false;

  MachineBasicBlock *Next = MBB.getNextNode();
  if (!Next || Next->pred_size() != 1 ||
      !Next->isLayoutSuccessor(Br->getOperand(0).getMBB()))
    return false;
  MachineInstr *MI = getOnlyInstr(*Next, Opc);
  if (!MI)
    return false;

  TII.insertCondForm(MBB, Br, CondOpc, *MI, Br->getDebugLoc());
  Br->eraseFromParent();
  if (MI->isBranch()) {
    MachineBasicBlock *Target = MI->getOperand(0).getMBB();
    MBB.replaceSuccessor(Next, Target);
    Next->removeSuccessor(Target);
  } else {
    MBB.removeSuccessor(Next);
  }
  Next->eraseFromParent();
  return true;
}

static bool removeJumpToNext(MachineBasicBlock &MBB) {
  auto Last = MBB.getLastNonDebugInstr();
  if (Last == MBB.end() ||
      (Last->getOpcode() != Z80::JR_e && Last->getOpcode() != Z80::JP_nn) ||
      !Last->getOperand(0).isMBB() ||
      !MBB.isLayoutSuccessor(Last->getOperand(0).getMBB()))
    return false;
  Last->eraseFromParent();
  return true;
}

/// Fold DEC B + JR NZ, loop into DJNZ loop.
///
/// WHAT: Fuses an 8-bit down-counter decrement on B and a following
/// conditional relative branch (JR NZ) into a single 2-byte DJNZ instruction.
///
/// WHY: DJNZ e combines DEC B and JR NZ e into 2 bytes and 13 T-states
/// (saving 1 byte and 3 T-states per iteration over 1-byte DEC B [4T] +
/// 2-byte JR NZ [12T]).
///
/// GOTCHA:
/// - Z80 only: SM83 lacks DJNZ, so check !STI.hasSM83().
/// - DJNZ does NOT modify any flags, whereas DEC B sets all ALU flags except
///   carry (Z, S, P/V, H, N). Therefore, the fold is only valid if FLAGS is
///   dead after the branch (both into Target and into the fallthrough block).
/// - Intervening instructions between DEC B and JR NZ are allowed only if
///   they have no dependencies on B or FLAGS and no side effects (e.g. INC HL).
///
/// WORKED EXAMPLE (from sum_bytes in djnz-loop.ll):
///   Before:
///     .LBB0_1:
///       add a, (hl)
///       inc hl
///       dec b           ; 1B, 4T
///       jr  nz, .LBB0_1 ; 2B, 12T
///   After:
///     .LBB0_1:
///       add a, (hl)
///       inc hl
///       djnz .LBB0_1    ; 2B, 13T (saves 1 byte and 3 T-states)
static bool foldToDJNZ(MachineBasicBlock &MBB, const Z80InstrInfo &TII,
                       const TargetRegisterInfo *TRI) {
  // Guard: SM83 lacks the DJNZ instruction entirely.
  if (MBB.getParent()->getSubtarget<Z80Subtarget>().hasSM83())
    return false;

  // Find the conditional branch JR NZ.
  MachineInstr *Branch = nullptr;
  for (MachineInstr &MI : MBB.terminators()) {
    if (MI.getOpcode() == Z80::JR_NZ_e) {
      Branch = &MI;
      break;
    }
  }
  // Guard: require a JR NZ terminator targeting a basic block.
  if (!Branch || !Branch->getOperand(0).isMBB())
    return false;

  MachineBasicBlock *Target = Branch->getOperand(0).getMBB();

  // Guard: target block must not expect live-in FLAGS from the loop latch.
  if (Target->isLiveIn(Z80::FLAGS))
    return false;

  // Guard: fallthrough / successor must not read FLAGS left by the loop.
  if (Z80::isLiveAt(MBB, std::next(Branch->getIterator()), Z80::FLAGS, TRI))
    return false;

  // Scan backward from Branch to find the matching DEC B.
  MachineInstr *DecB = nullptr;
  auto It = Branch->getIterator();
  while (It != MBB.begin()) {
    --It;
    if (It->isDebugInstr())
      continue;

    // Found DEC B: register operand is B.
    if (It->getOpcode() == Z80::DEC_r && It->getOperand(0).isReg() &&
        It->getOperand(0).getReg() == Z80::B) {
      DecB = &*It;
      break;
    }

    // Guard: intervening instructions cannot touch FLAGS (would break compare).
    if (It->modifiesRegister(Z80::FLAGS, TRI) ||
        It->readsRegister(Z80::FLAGS, TRI))
      return false;

    // Guard: intervening instructions cannot read or write B (ordering hazard).
    if (It->modifiesRegister(Z80::B, TRI) || It->readsRegister(Z80::B, TRI))
      return false;

    // Guard: calls, terminators, or unmodeled side effects cannot be bypassed.
    if (It->isCall() || It->isTerminator() || It->hasUnmodeledSideEffects())
      return false;
  }

  // Guard: no matching DEC B found in this basic block.
  if (!DecB)
    return false;

  // Replace Branch with DJNZ e, and erase DEC B.
  BuildMI(MBB, Branch, Branch->getDebugLoc(), TII.get(Z80::DJNZ_e))
      .addMBB(Target);
  Branch->eraseFromParent();
  DecB->eraseFromParent();
  return true;
}

/// Rotate an inverted loop layout produced by MachineBlockPlacement back to
/// canonical natural order.
///
/// WHAT: Reorders basic blocks so that the loop header immediately follows
/// the preheader (eliding the preheader's unconditional jump) and inverts the
/// latch branch so it jumps back to the header (enabling DJNZ folding).
///
/// WHY: Generic MachineBlockPlacement prioritizes loop back-edge fallthrough
/// by placing the latch before the header and jumping conditionally to exit on
/// zero (JR Z exit). On Z80, this inversion costs 4 bytes (2B preheader JR,
/// 2B exit JR) and prevents DJNZ (which only branches taken on non-zero).
///
/// GOTCHA:
/// - Only rotate when the latch's fallthrough is genuinely the header.
/// - Ensure all blocks in the tail chain [Header ... LoopTail] belong strictly
///   to the loop body (no external branches).
///
/// WORKED EXAMPLE (from parse_packet_loop in rotate-inverted-loop.ll):
///   Before:
///     Preheader (bb.1): ... JR_e bb.4 (Header)
///     LoopChain1 (bb.2, if.then7): INC B, JR_e bb.3
///     Latch (bb.3, if.end12): DEC C, INC HL, JR_Z_e bb.7 (Exit) -> fallthrough bb.4
///     Header (bb.4, for.body): CP (HL), JR_Z_e bb.2 -> fallthrough bb.5
///     LoopChain2 (bb.5, if.else): INC (IX), JR_e bb.3
///   After rotating [bb.4, bb.5] right after Preheader:
///     Preheader (bb.1): ... (JR_e bb.4 removed, falls through to bb.4)
///     Header (bb.4, for.body): CP (HL), JR_Z_e bb.2 -> fallthrough bb.5
///     LoopChain2 (bb.5, if.else): INC (IX), JR_e bb.3
///     LoopChain1 (bb.2, if.then7): INC B, JR_e bb.3
///     Latch (bb.3, if.end12): DEC C, INC HL, JR_NZ_e bb.4, JR_e bb.7
static bool rotateInvertedLoops(MachineFunction &MF, const Z80InstrInfo &TII) {
  bool Changed = false;

  for (auto MBI = MF.begin(), MBE = MF.end(); MBI != MBE; ++MBI) {
    MachineBasicBlock &Preheader = *MBI;

    // Guard: Preheader must end with an unconditional jump (JR_e or JP_nn).
    if (Preheader.empty())
      continue;
    MachineInstr &PreTerm = Preheader.back();
    unsigned PreOpc = PreTerm.getOpcode();
    if (PreOpc != Z80::JR_e && PreOpc != Z80::JP_nn)
      continue;
    if (PreTerm.getNumOperands() == 0 || !PreTerm.getOperand(0).isMBB())
      continue;
    MachineBasicBlock *Header = PreTerm.getOperand(0).getMBB();
    // Guard: target must be a different block.
    if (!Header || Header == &Preheader)
      continue;

    // Guard: Header must appear strictly after Preheader in layout.
    if (Header->getNumber() <= Preheader.getNumber())
      continue;

    // Guard: Header must have at least two predecessors (preheader and latch).
    if (Header->pred_size() < 2 || !Header->isPredecessor(&Preheader))
      continue;

    // Guard: Latch must immediately precede Header in layout (fallthrough back-edge).
    auto HeaderIt = Header->getIterator();
    if (HeaderIt == MF.begin())
      continue;
    MachineBasicBlock *Latch = &*std::prev(HeaderIt);
    if (!Header->isPredecessor(Latch) || Latch->getNumber() <= Preheader.getNumber())
      continue;

    // Guard: Latch must fall through to Header as its back-edge.
    if (!Latch->isLayoutSuccessor(Header))
      continue;

    // Find the conditional branch in Latch targeting the exit block.
    MachineInstr *CondBr = nullptr;
    for (MachineInstr &MI : Latch->terminators()) {
      if (MI.isConditionalBranch()) {
        CondBr = &MI;
        break;
      }
    }
    // Guard: Latch must have a conditional branch to an exit block.
    if (!CondBr || CondBr->getNumOperands() == 0 || !CondBr->getOperand(0).isMBB())
      continue;

    MachineBasicBlock *Exit = CondBr->getOperand(0).getMBB();
    if (Exit == Header)
      continue;

    unsigned CondOpc = CondBr->getOpcode();
    unsigned InvertedOpc = getOppositeBranchOpcode(CondOpc);
    // Guard: branch condition must be invertible.
    if (!InvertedOpc)
      continue;

    // Find contiguous tail chain [Header ... LoopTail] belonging to loop body.
    MachineBasicBlock *LoopTail = Header;
    auto NextIt = std::next(Header->getIterator());
    while (NextIt != MF.end()) {
      MachineBasicBlock *Cand = &*NextIt;
      bool InLoop = false;
      for (MachineBasicBlock *Succ : Cand->successors()) {
        if (Succ == Latch || Succ == Header ||
            (Succ->getNumber() > Preheader.getNumber() &&
             Succ->getNumber() <= Latch->getNumber())) {
          InLoop = true;
          break;
        }
      }
      if (!InLoop)
        break;
      LoopTail = Cand;
      ++NextIt;
    }

    // Splice [Header ... LoopTail] to immediately follow Preheader.
    auto SpliceBegin = Header->getIterator();
    auto SpliceEnd = std::next(LoopTail->getIterator());
    auto InsertPt = std::next(Preheader.getIterator());

    MF.splice(InsertPt, SpliceBegin, SpliceEnd);
    MF.RenumberBlocks();

    // 1. Remove Preheader's unconditional jump (now falls through to Header).
    PreTerm.eraseFromParent();

    // 2. Invert Latch's branch: jump back to Header on inverted condition.
    DebugLoc DL = CondBr->getDebugLoc();
    BuildMI(*Latch, CondBr->getIterator(), DL, TII.get(InvertedOpc))
        .addMBB(Header);
    CondBr->eraseFromParent();

    // 3. If Latch does not fall through to Exit, insert unconditional jump to Exit.
    if (!Latch->isLayoutSuccessor(Exit)) {
      BuildMI(*Latch, Latch->end(), DL, TII.get(Z80::JR_e))
          .addMBB(Exit);
    }

    Changed = true;
    ++NumInvertedLoopsRotated;
    break; // Break and let caller handle renumbered loop or next iteration
  }

  return Changed;
}

bool Z80BranchCleanup::runOnMachineFunction(MachineFunction &MF) {
  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  const auto *TII = STI.getInstrInfo();
  const auto *TRI = STI.getRegisterInfo();
  bool Changed = false;

  while (rotateInvertedLoops(MF, *TII))
    Changed = true;

  for (MachineBasicBlock &MBB : MF) {
    if (foldToDJNZ(MBB, *TII, TRI)) {
      ++NumDJNZFolded;
      Changed = true;
    }
  }

  for (MachineBasicBlock &MBB : MF) {
    if (foldBranchOver(MBB, *TII, Z80::JP_nn, getCondJPOpcode)) {
      ++NumTrampolinesCollapsed;
      Changed = true;
    }
  }

  // We should fold tail calls first. Folding the RET first gives the less
  // efficient RET cc / JP f.
  for (MachineBasicBlock &MBB : MF) {
    if (foldBranchOver(MBB, *TII, Z80::TCRETURN_nn,
                       Z80::getCondTailCallOpcode)) {
      ++NumCondTailCalls;
      Changed = true;
    }
  }
  for (MachineBasicBlock &MBB : MF)
    Changed |= foldBranchesToRet(MBB, *TII);
  // We should fold branches to a RET first. A branch over a RET folds only
  // when no other branch reaches that RET block.
  for (MachineBasicBlock &MBB : MF) {
    if (foldBranchOver(MBB, *TII, Z80::RET, Z80::getCondRetOpcode)) {
      ++NumCondReturns;
      Changed = true;
    }
  }
  for (MachineBasicBlock &MBB : MF) {
    if (removeJumpToNext(MBB)) {
      ++NumJumpsToNext;
      Changed = true;
    }
  }

  return Changed;
}

char Z80BranchCleanup::ID = 0;

INITIALIZE_PASS(Z80BranchCleanup, DEBUG_TYPE,
                "Clean up Z80 branches after relaxation", false, false)

MachineFunctionPass *llvm::createZ80BranchCleanupPass() {
  return new Z80BranchCleanup();
}
