//===-- Z80HighByteFirstBranch.cpp - High-byte-first loop exit test -------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Speed optimisation for the hot 16-bit unsigned loop exit test
// (ravn/llvm-z80#250 lever 2).  A byte-array pointer-walk kill loop exits on a
// full 16-bit unsigned compare:
//
//     CMP16_FLAGS %ptr, %end      ; ld a,l; sub c; ld a,h; sbc a,b   (16 T)
//     JP_C  loop                  ; ptr <  end -> keep looping
//     JP    exit                  ; ptr >= end -> done
//
// dcc runs ~1.28x faster here by comparing the HIGH byte first and taking the
// common "still well below the end" fast path in 8 T:
//
//     ld a,h ; cp b ; jp c,loop   ; ptr.hi <  end.hi -> loop  (fast path, 8 T)
//                     jp nz,exit  ; ptr.hi >  end.hi -> exit
//     ld a,l ; cp c ; jp c,loop   ; hi equal -> compare lo
//                     jp exit
//
// For a pointer that climbs by a small stride the high bytes differ on almost
// every iteration, so the fast path fires and the exit test drops 16 T -> 8 T.
// On the sieve benchmark this is the last ~1.5M-cycle lever between clang and
// dcc after the #250 start-pointer sink.
//
// It is SIZE-NEGATIVE (one extra compare + branch + a split block), so it is
// gated behind -z80-enable-hbf-branch (default OFF) and only rewrites a
// CMP16_FLAGS whose JP_C target is a loop back-edge (dominates the block) -- a
// hot loop latch, where the speed win pays for the bytes.  Uses only A + FLAGS,
// exactly like the CMP16_FLAGS it replaces, so it needs no extra register.
//
// Runs post-RA in addPreEmitPass, after BranchRelaxation/BranchCleanup (so the
// conditional exit is already a documented JP_cc) and before Z80ExpandPseudo
// (so the CMP16_FLAGS is still a single pseudo we can recognise).  It emits
// only absolute JPs, which never need relaxation.
//
//===----------------------------------------------------------------------===//

#include "Z80HighByteFirstBranch.h"
#include "Z80.h"
#include "Z80InstrInfo.h"
#include "Z80OpcodeUtils.h"
#include "Z80Subtarget.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "z80-high-byte-first-branch"

// Auto-on at -O2 only (ravn/llvm-z80#250 lever 2), same rule as the pointer-walk
// stack it complements.  If explicitly forced on at another level, branch width
// still adapts (jr at -Os/-Oz, hybrid at -O2, jp at -O3).  Override with
// `-z80-enable-hbf-branch[=false]`.
static cl::opt<bool> EnableZ80HighByteFirstBranch(
    "z80-enable-hbf-branch", cl::init(false), cl::Hidden,
    cl::desc("Force Z80 high-byte-first loop exit tests on/off (default: auto "
             "at -O2 only; ravn/llvm-z80#250 lever 2)"));

namespace {

class Z80HighByteFirstBranch : public MachineFunctionPass {
public:
  static char ID;

  Z80HighByteFirstBranch() : MachineFunctionPass(ID) {
    initializeZ80HighByteFirstBranchPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Z80 High-byte-first loop exit test";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineDominatorTreeWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

// One matched hot exit test: the four instructions of an expanded CMP16_FLAGS
// unsigned-less-than chain (ld a,lhs.lo; sub rhs.lo; ld a,lhs.hi; sbc a,rhs.hi)
// plus the two branches after it.  By the time this pass runs (addPreEmitPass)
// the generic ExpandPostRAPseudos has already lowered CMP16_FLAGS to these four
// instructions, so we match the lowered shape and recover the four 8-bit regs.
struct Match {
  MachineInstr *LdLo, *SubLo, *LdHi, *SbcHi; // the compare chain (to erase)
  MachineInstr *CondBr;                      // JP_C_nn / JR_C_e loop
  MachineInstr *UncondBr;                    // JP_nn / JR_e exit
  MachineBasicBlock *Loop;
  MachineBasicBlock *Exit;
  Register LhsLo, RhsLo, LhsHi, RhsHi; // compared as {LhsHi:LhsLo} < {RhsHi:RhsLo}
};

// The non-A GR8 physreg used by \p MI (the "r" of LD A,r / SUB r / SBC A,r).
static Register gr8UseOf(const MachineInstr &MI) {
  for (const MachineOperand &MO : MI.operands()) {
    if (!MO.isReg() || !MO.isUse() || !MO.getReg().isPhysical())
      continue;
    Register R = MO.getReg();
    if (R != Z80::A && Z80::gr8RegToIndex(R) >= 0)
      return R;
  }
  return Register();
}

} // end anonymous namespace

char Z80HighByteFirstBranch::ID = 0;

INITIALIZE_PASS_BEGIN(Z80HighByteFirstBranch, DEBUG_TYPE,
                      "Z80 High-byte-first loop exit test", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTreeWrapperPass)
INITIALIZE_PASS_END(Z80HighByteFirstBranch, DEBUG_TYPE,
                    "Z80 High-byte-first loop exit test", false, false)

// Match the exact hot-exit shape at the end of MBB:
//   CMP16_FLAGS %lhs, %rhs  (last non-terminator)
//   JP_C_nn Loop            (first terminator, its flags source is the CMP)
//   JP_nn   Exit            (second terminator)
// with Loop a back-edge (dominates MBB).  Fills \p M.
static bool matchHotExit(MachineBasicBlock &MBB, const MachineDominatorTree &MDT,
                         const TargetRegisterInfo &TRI, Match &M) {
  // Collect the trailing terminators.  This pass runs after BranchRelaxation,
  // so a short in-loop exit is JR_C_e/JR_e; a long one stays JP_C_nn/JP_nn.
  // Accept either -- both carry the target MBB as operand 0.
  auto TermIt = MBB.getFirstTerminator();
  if (TermIt == MBB.end())
    return false;
  MachineInstr *CondBr = &*TermIt;
  if (CondBr->getOpcode() != Z80::JP_C_nn &&
      CondBr->getOpcode() != Z80::JR_C_e)
    return false;
  if (!CondBr->getOperand(0).isMBB())
    return false;
  MachineBasicBlock *Loop = CondBr->getOperand(0).getMBB();

  // The exit edge is either an explicit second terminator (JP_nn/JR_e) or an
  // implicit fall-through to the layout-next block (the common in-loop shape:
  // `jr c, loop` then fall through to the exit).
  MachineInstr *UncondBr = nullptr;
  MachineBasicBlock *Exit = nullptr;
  auto Next = std::next(TermIt);
  if (Next != MBB.end()) {
    UncondBr = &*Next;
    if ((UncondBr->getOpcode() != Z80::JP_nn &&
         UncondBr->getOpcode() != Z80::JR_e) ||
        !UncondBr->getOperand(0).isMBB() || std::next(Next) != MBB.end())
      return false;
    Exit = UncondBr->getOperand(0).getMBB();
  } else {
    // Fall-through exit: the non-Loop CFG successor.
    for (MachineBasicBlock *S : MBB.successors())
      if (S != Loop)
        Exit = S;
    if (!Exit)
      return false;
  }

  // The four instructions right before the terminators must be an expanded
  // CMP16_FLAGS unsigned-less-than chain:
  //   LD A,LhsLo ; SUB RhsLo ; LD A,LhsHi ; SBC A,RhsHi
  auto Prev = [&](MachineBasicBlock::iterator It) {
    return It == MBB.begin() ? MBB.end() : std::prev(It);
  };
  auto ItSbc = Prev(TermIt);
  if (ItSbc == MBB.end())
    return false;
  auto ItLdHi = Prev(ItSbc);
  if (ItLdHi == MBB.end())
    return false;
  auto ItSub = Prev(ItLdHi);
  if (ItSub == MBB.end())
    return false;
  auto ItLdLo = Prev(ItSub);
  if (ItLdLo == MBB.end())
    return false;
  MachineInstr *SbcHi = &*ItSbc, *LdHi = &*ItLdHi, *SubLo = &*ItSub,
               *LdLo = &*ItLdLo;

  Register LhsLo = gr8UseOf(*LdLo), RhsLo = gr8UseOf(*SubLo);
  Register LhsHi = gr8UseOf(*LdHi), RhsHi = gr8UseOf(*SbcHi);
  if (!LhsLo || !RhsLo || !LhsHi || !RhsHi)
    return false;
  // Confirm the exact opcodes (guards against LD/ADD/LD/ADC and friends).
  if (LdLo->getOpcode() != Z80::getLD8RegOpcode(Z80::A, LhsLo) ||
      SubLo->getOpcode() != Z80::getSUBOpcode(RhsLo) ||
      LdHi->getOpcode() != Z80::getLD8RegOpcode(Z80::A, LhsHi) ||
      SbcHi->getOpcode() != Z80::getSBCOpcode(RhsHi))
    return false;
  // Need CP opcodes for the high-byte-first rewrite.
  if (!Z80::getCPOpcode(RhsLo) || !Z80::getCPOpcode(RhsHi))
    return false;

  // Profitability gate: only rewrite a genuine loop back-edge (Loop dominates
  // MBB), where the size-negative fast path pays off.
  if (!MDT.dominates(Loop, &MBB))
    return false;

  M = {LdLo,  SubLo, LdHi,  SbcHi, CondBr,
       UncondBr, Loop, Exit, LhsLo, RhsLo, LhsHi, RhsHi};
  return true;
}

// Emit `LD A, <src8>` then `CP <cmp8>` at the end of \p BB (implicit-operand
// opcodes, mirroring the CMP16_FLAGS expansion in Z80InstrInfo.cpp).
static void emitByteCompare(MachineBasicBlock &BB, const DebugLoc &DL,
                            const TargetInstrInfo &TII, Register Src8,
                            Register Cmp8) {
  BuildMI(&BB, DL, TII.get(Z80::getLD8RegOpcode(Z80::A, Src8)));
  BuildMI(&BB, DL, TII.get(Z80::getCPOpcode(Cmp8)));
}

bool Z80HighByteFirstBranch::runOnMachineFunction(MachineFunction &MF) {
  // -O2-only default (ravn/llvm-z80#250): auto-on at -O2 (== Default opt level,
  // not opt-size); explicit -mllvm flag overrides at any level.
  bool Enabled = EnableZ80HighByteFirstBranch.getNumOccurrences() > 0
                     ? EnableZ80HighByteFirstBranch.getValue()
                     : (MF.getTarget().getOptLevel() == CodeGenOptLevel::Default &&
                        !MF.getFunction().hasOptSize());
  if (!Enabled)
    return false;
  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  if (!STI.hasZ80())
    return false;

  const TargetInstrInfo &TII = *STI.getInstrInfo();
  const TargetRegisterInfo &TRI = *STI.getRegisterInfo();
  auto &MDT = getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();

  // Branch-width policy by opt level (ravn/llvm-z80#250 lever 2):
  //   -Os/-Oz (size)  : all JR  (2 B, 12 T)          -- hbf ~+4 B
  //   -O2     (balance): JP only on the hot backedge  -- hbf ~+5 B
  //   -O3     (speed) : all JP (3 B, 10 T)           -- hbf ~+7 B
  const Function &F = MF.getFunction();
  bool Size = F.hasOptSize(); // -Os OR -Oz
  bool Aggressive = MF.getTarget().getOptLevel() == CodeGenOptLevel::Aggressive;

  // Precompute a running byte offset per block (conservative -- CMP16 etc. in
  // other blocks are not expanded yet, so this UNDER-estimates, which only makes
  // us prefer JP; safe).  Used to keep every JR we emit inside its +/-127 range.
  DenseMap<const MachineBasicBlock *, int64_t> BlockOff;
  int64_t Off = 0;
  for (MachineBasicBlock &B : MF) {
    BlockOff[&B] = Off;
    for (MachineInstr &I : B)
      Off += TII.getInstSizeInBytes(I);
  }
  auto JRInRange = [&](const MachineBasicBlock *Src,
                       const MachineBasicBlock *Dst) {
    // Distance from end-ish of Src to start of Dst; +32 slack for the not-yet-
    // expanded pseudos between them.  JR displacement is a signed 8-bit (+/-127).
    int64_t D = BlockOff[Dst] - BlockOff[Src];
    return D >= -100 && D <= 100;
  };
  // Pick the branch opcode: `Hot` = the fast backedge (JP at -O2/-O3);
  // everything else is JP only at -O3.  Fall back to the JP form when the JR
  // would be out of range.
  auto condBr = [&](MachineBasicBlock *Src, MachineBasicBlock *Dst, bool Hot,
                    unsigned JpOpc, unsigned JrOpc) {
    bool WantJp = Aggressive || (Hot && !Size);
    unsigned Opc = (WantJp || !JRInRange(Src, Dst)) ? JpOpc : JrOpc;
    BuildMI(Src, DebugLoc(), TII.get(Opc)).addMBB(Dst);
  };

  // Collect matches first (the rewrite creates new blocks / mutates the CFG).
  SmallVector<std::pair<MachineBasicBlock *, Match>, 4> Work;
  for (MachineBasicBlock &MBB : MF) {
    Match M;
    if (matchHotExit(MBB, MDT, TRI, M))
      Work.emplace_back(&MBB, M);
  }
  if (Work.empty())
    return false;

  for (auto &[MBB, M] : Work) {
    DebugLoc DL = M.SbcHi->getDebugLoc();
    Register LhsHi = M.LhsHi, LhsLo = M.LhsLo, RhsHi = M.RhsHi, RhsLo = M.RhsLo;

    // New tail block holds the low-byte compare (reached when the high bytes
    // are equal).  Place it right after MBB so MBB falls through to it.
    MachineBasicBlock *LoBB = MF.CreateMachineBasicBlock(MBB->getBasicBlock());
    MF.insert(std::next(MBB->getIterator()), LoBB);

    // Drop the expanded compare chain and the branch(es); rebuild MBB's tail.
    // The exit may have been an implicit fall-through (no UncondBr).
    M.LdLo->eraseFromParent();
    M.SubLo->eraseFromParent();
    M.LdHi->eraseFromParent();
    M.SbcHi->eraseFromParent();
    M.CondBr->eraseFromParent();
    if (M.UncondBr)
      M.UncondBr->eraseFromParent();

    // MBB: high byte.  ld a,lhs.hi ; cp rhs.hi ; j c,Loop ; j nz,Exit ; (->LoBB)
    emitByteCompare(*MBB, DL, TII, LhsHi, RhsHi);
    condBr(MBB, M.Loop, /*Hot=*/true, Z80::JP_C_nn, Z80::JR_C_e);
    condBr(MBB, M.Exit, /*Hot=*/false, Z80::JP_NZ_nn, Z80::JR_NZ_e);

    // LoBB: low byte.  ld a,lhs.lo ; cp rhs.lo ; j c,Loop ; j Exit
    // (the unconditional Exit branch is dropped by Z80RemoveJumpToNext when Exit
    // is the fall-through block; otherwise it survives as a real cold exit.)
    emitByteCompare(*LoBB, DL, TII, LhsLo, RhsLo);
    condBr(LoBB, M.Loop, /*Hot=*/false, Z80::JP_C_nn, Z80::JR_C_e);
    {
      bool WantJp = Aggressive;
      unsigned Opc =
          (WantJp || !JRInRange(LoBB, M.Exit)) ? Z80::JP_nn : Z80::JR_e;
      BuildMI(LoBB, DL, TII.get(Opc)).addMBB(M.Exit);
    }

    // Fix up the CFG.  MBB used to go to {Loop, Exit}; now it also falls
    // through to LoBB, and LoBB goes to {Loop, Exit}.
    MBB->addSuccessor(LoBB);
    LoBB->addSuccessor(M.Loop);
    LoBB->addSuccessor(M.Exit);

    // Recompute live-ins for the new block from its successors (post-RA).
    LivePhysRegs LiveRegs;
    computeAndAddLiveIns(LiveRegs, *LoBB);

    LLVM_DEBUG(dbgs() << "z80-hbf: rewrote exit test in " << MF.getName()
                      << " (" << printReg(LhsHi, &TRI) << printReg(LhsLo, &TRI)
                      << " < " << printReg(RhsHi, &TRI) << printReg(RhsLo, &TRI)
                      << ")\n");
  }
  return true;
}

MachineFunctionPass *llvm::createZ80HighByteFirstBranchPass() {
  return new Z80HighByteFirstBranch;
}
