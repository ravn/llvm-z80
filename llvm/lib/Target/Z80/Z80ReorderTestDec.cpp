//===-- Z80ReorderTestDec.cpp - SUB-based test-then-dec idiom ----------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// ravn/llvm-z80#179 -- rewrite a specific post-ISel MIR pattern emitted
// by the GISel selector for the IR shape:
//
//     %dec = sub i8 %x, 1
//     %cond = icmp eq i8 %x, 0
//     br i1 %cond, exit_bb, body_bb
//
// The selector lowers ops in IR-source order: DEC_A first (clobbers
// $a), then has to reload %x to test it.  Resulting 6-instruction MIR
// uses A as a transit, with a redundant reload:
//
//     [I0]  $a = COPY %x:gr8        ; load %x into A
//     [I1]  DEC_A                   ; A := %x - 1, clobbers A
//     [I2]  %y:gr8 = COPY $a        ; save dec'd to %y
//     [I3]  $a = COPY %x:gr8        ; RELOAD %x  <-- redundant
//     [I4]  OR_A                    ; test A == 0
//     [I5]  JR_Z/JP_Z exit          ; branch
//
// Rewrite uses Z80's SUB_n instruction which produces both the
// dec'd value AND the carry flag (CARRY = 1 iff pre-value was 0,
// because SUB 1 borrows from 0 to wrap to 0xFF):
//
//     [I0]  $a = COPY %x:gr8        ; load %x into A
//     [I1'] SUB_n 1                 ; A := %x - 1, CARRY = (%x was 0)
//     [I2]  %y:gr8 = COPY $a        ; save dec'd to %y
//     [I5'] JR_C/JP_C exit          ; branch on CARRY (same condition)
//
// Net: 4 instructions instead of 6.  -2 instructions, -2 bytes (DEC_A
// 1B + I3 1B + OR_A 1B = 3B replaced by SUB_n 2B), -5..-13 tstates per
// iter (depending on branch outcome).
//
// In gf_log/gf_alog inner loops (~120 K iters per AES workload) this
// is ~600 K ts saved = ~21 % of the total clang-vs-SDCC AES speed gap.
//
// Why this is a pre-RA MIR pass and not a peephole or a combiner:
//   - Peephole (post-RA) would be the wrong layer per the
//     root-cause-over-peephole directive.  This is missing-backend-
//     work in the regalloc/scheduler interaction, not a Z80-ISA-
//     specific late-opt pattern.
//   - GISel combiner would need to operate on the IR shape, but the
//     IR is already optimal -- the bug emerges in how ISel lowers
//     the two independent operations through implicit-use $a.  A
//     combiner can't see post-ISel MIR.
//   - Custom MachineSchedStrategy would be the most generic fix but
//     is ~1-3 weeks of work.  This pass mirrors Z80SplitDjnzCounters'
//     precedent for "narrow-scope pre-RA MIR pass that fixes a
//     specific regalloc/scheduler shape we can't fix generically".
//
// Safety:
//   - Both COPY-to-$a must reference the SAME source vreg.
//   - No instruction between I0 and I5 may modify the source vreg.
//   - The dec'd value %y must not need to retain DEC_A's specific
//     flag side-effects (we change DEC_A's flag semantics to SUB_n's,
//     which adds CARRY -- but the only flag consumer in the pattern
//     is the immediate Z-test which we're rewriting to a C-test).
//
//===----------------------------------------------------------------------===//

#include "Z80ReorderTestDec.h"
#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80InstrInfo.h"
#include "Z80Subtarget.h"

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "z80-reorder-test-dec"

namespace {

class Z80ReorderTestDec : public MachineFunctionPass {
public:
  static char ID;

  Z80ReorderTestDec() : MachineFunctionPass(ID) {
    initializeZ80ReorderTestDecPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Z80 Reorder Test-Then-Dec";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  bool rewriteInMBB(MachineBasicBlock &MBB);
};

} // end anonymous namespace

char Z80ReorderTestDec::ID = 0;

INITIALIZE_PASS(Z80ReorderTestDec, DEBUG_TYPE,
                "Z80 Reorder Test-Then-Dec", false, false)

// Map JR_Z/JP_Z to JR_C/JP_C (the same form, different condition).
static unsigned getCarryFormOpcode(unsigned ZOpc) {
  switch (ZOpc) {
  case Z80::JR_Z_e:  return Z80::JR_C_e;
  case Z80::JP_Z_nn: return Z80::JP_C_nn;
  default:           return 0;
  }
}

// True iff MI is `$a = COPY <vreg>`; if so, SrcVReg is set.
static bool isCopyToA(const MachineInstr &MI, Register &SrcVReg) {
  if (MI.getOpcode() != TargetOpcode::COPY)
    return false;
  if (!MI.getOperand(0).isReg() || MI.getOperand(0).getReg() != Z80::A)
    return false;
  if (!MI.getOperand(1).isReg())
    return false;
  Register S = MI.getOperand(1).getReg();
  if (!S.isVirtual())
    return false;
  SrcVReg = S;
  return true;
}

// True iff MI is `<vreg> = COPY $a`.
static bool isCopyFromA(const MachineInstr &MI) {
  if (MI.getOpcode() != TargetOpcode::COPY)
    return false;
  if (!MI.getOperand(0).isReg() || !MI.getOperand(0).getReg().isVirtual())
    return false;
  if (!MI.getOperand(1).isReg() || MI.getOperand(1).getReg() != Z80::A)
    return false;
  return true;
}

// Skip meta instructions (DBG_VALUE, KILL etc.) forward.
static MachineBasicBlock::iterator
nextNonMeta(MachineBasicBlock::iterator It, MachineBasicBlock::iterator End) {
  while (It != End && It->isMetaInstruction())
    ++It;
  return It;
}

// Match the shared shape: I0 = $a = COPY %src ; I1 = <ALU on A>;
// I2 = vreg = COPY $a (dest must NOT be %src); I3 = $a = COPY %src
// (same %src as I0); I4 = <flag-only test on A>; I5 = conditional
// branch on flags.  Returns true if shape matches; iterators are
// set to the matched instructions.  AluOpc filters which ALU at I1
// (DEC_A for P1, ADD_A_A for P2).  TestOpcMatcher tests whether
// I4 is the expected flag-only test for that variant.
template <typename TestOpcMatcher>
static bool matchSharedShape(MachineBasicBlock::iterator MII,
                             MachineBasicBlock::iterator MIE,
                             unsigned AluOpc,
                             TestOpcMatcher TestMatches,
                             MachineBasicBlock::iterator &I0,
                             MachineBasicBlock::iterator &I1,
                             MachineBasicBlock::iterator &I2,
                             MachineBasicBlock::iterator &I3,
                             MachineBasicBlock::iterator &I4,
                             MachineBasicBlock::iterator &I5,
                             Register &CounterVReg) {
  if (!isCopyToA(*MII, CounterVReg)) return false;
  I0 = MII;

  I1 = nextNonMeta(std::next(I0), MIE);
  if (I1 == MIE || I1->getOpcode() != AluOpc) return false;

  I2 = nextNonMeta(std::next(I1), MIE);
  if (I2 == MIE || !isCopyFromA(*I2)) return false;
  // Critical safety gate: I2's destination must differ from
  // CounterVReg.  Else the regalloc reused the vreg for the POST
  // value, and the "reload" in I3 actually loads POST -- different
  // test semantics than what the rewrite produces.
  if (I2->getOperand(0).getReg() == CounterVReg) return false;

  I3 = nextNonMeta(std::next(I2), MIE);
  Register I3Src;
  if (I3 == MIE || !isCopyToA(*I3, I3Src) || I3Src != CounterVReg)
    return false;

  I4 = nextNonMeta(std::next(I3), MIE);
  if (I4 == MIE || !TestMatches(*I4)) return false;

  I5 = nextNonMeta(std::next(I4), MIE);
  if (I5 == MIE) return false;
  return true;
}

// True iff MI is `OR_A` or `OR_r $a` (both encode the same byte).
static bool isOrATest(const MachineInstr &MI) {
  if (MI.getOpcode() == Z80::OR_A) return true;
  if (MI.getOpcode() == Z80::OR_r &&
      MI.getNumOperands() >= 1 && MI.getOperand(0).isReg() &&
      MI.getOperand(0).getReg() == Z80::A)
    return true;
  return false;
}

// True iff MI is RLCA (rotate left through carry, no carry-in
// dependency).  The post-ISel form here is plain RLCA; ADD_A_A's
// carry-out has the SAME bit (bit 7 of the input) as RLCA's
// carry-out, so the RLCA is redundant work after ADD_A_A on the
// same register provided no flag-clobbering instruction lies
// between them.
static bool isRLCA(const MachineInstr &MI) {
  return MI.getOpcode() == Z80::RLCA;
}

bool Z80ReorderTestDec::rewriteInMBB(MachineBasicBlock &MBB) {
  const TargetInstrInfo *TII = MBB.getParent()->getSubtarget().getInstrInfo();
  bool Changed = false;

  for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; /* incremented */) {
    MachineBasicBlock::iterator I0, I1, I2, I3, I4, I5;
    Register CounterVReg;
    unsigned CarryOpc = 0;

    // P1: DEC + OR_A test.  Rewrite to SUB_n 1; ...; JR_C.
    bool P1 = matchSharedShape(MII, MIE, Z80::DEC_A, isOrATest,
                               I0, I1, I2, I3, I4, I5, CounterVReg);
    if (P1) {
      CarryOpc = getCarryFormOpcode(I5->getOpcode());
      if (CarryOpc == 0) P1 = false;
    }

    // P2: ADD_A_A + RLCA bit-7 test.  Rewrite to ADD_A_A alone; use its
    // carry directly at the branch.  Both ADD_A_A and RLCA set CARRY :=
    // bit 7 of the input value, so when both operate on the same
    // freshly-loaded value, the second is redundant.  Safety: the
    // shared-shape's I2-destination-differs-from-source gate ensures
    // CounterVReg isn't redefined between the two loads.
    bool P2 = false;
    if (!P1) {
      P2 = matchSharedShape(MII, MIE, Z80::ADD_A_A, isRLCA,
                            I0, I1, I2, I3, I4, I5, CounterVReg);
      if (P2) {
        // I5 must be a flag-conditional branch (JR_C, JR_NC, JP_C, JP_NC).
        unsigned BrOpc = I5->getOpcode();
        if (BrOpc != Z80::JR_C_e && BrOpc != Z80::JR_NC_e &&
            BrOpc != Z80::JP_C_nn && BrOpc != Z80::JP_NC_nn) {
          P2 = false;
        }
      }
    }

    if (!P1 && !P2) {
      ++MII;
      continue;
    }

    LLVM_DEBUG(dbgs() << DEBUG_TYPE << ": rewrite ("
                      << (P1 ? "P1 DEC+OR" : "P2 ADD+RLCA")
                      << ") in " << MBB.getName()
                      << " starting at " << *I0);

    if (P1) {
      // Rewrite P1:
      //   1. Replace DEC_A with SUB_n 1 (defines A, sets CARRY).
      //   2. Erase I3 (redundant reload).
      //   3. Erase I4 (OR_A no longer needed; carry is set by SUB_n).
      //   4. Change I5 from JR_Z/JP_Z to JR_C/JP_C.
      DebugLoc DL = I1->getDebugLoc();
      BuildMI(MBB, I1, DL, TII->get(Z80::SUB_n)).addImm(1);
      I1->eraseFromParent();
      I3->eraseFromParent();
      I4->eraseFromParent();
      const MCInstrDesc &CarryDesc = TII->get(CarryOpc);
      I5->setDesc(CarryDesc);
    } else {
      // Rewrite P2: ADD_A_A is kept (it sets CARRY = bit 7 of input).
      //   1. Erase I3 (redundant reload of original value).
      //   2. Erase I4 (RLCA -- ADD_A_A's CARRY is already what we want).
      //   3. I5 (JR_C/NC or JP_C/NC) is kept unchanged -- it tests the
      //      same flag (CARRY = bit 7 of input).
      I3->eraseFromParent();
      I4->eraseFromParent();
    }

    // Advance MII past the rewritten region.  After the rewrites,
    // the structure is: I0, SubN, I2, I5'.  Continue scanning from
    // just past I5'.
    MII = std::next(I5);
    Changed = true;
  }

  return Changed;
}

bool Z80ReorderTestDec::runOnMachineFunction(MachineFunction &MF) {
  bool Changed = false;
  for (MachineBasicBlock &MBB : MF)
    Changed |= rewriteInMBB(MBB);
  return Changed;
}

MachineFunctionPass *llvm::createZ80ReorderTestDecPass() {
  return new Z80ReorderTestDec;
}
