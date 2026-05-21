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

bool Z80ReorderTestDec::rewriteInMBB(MachineBasicBlock &MBB) {
  const TargetInstrInfo *TII = MBB.getParent()->getSubtarget().getInstrInfo();
  bool Changed = false;

  for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; /* incremented */) {
    Register CounterVReg;
    if (!isCopyToA(*MII, CounterVReg)) {
      ++MII;
      continue;
    }
    auto I0 = MII;

    // I1: DEC_A
    auto I1 = nextNonMeta(std::next(I0), MIE);
    if (I1 == MIE || I1->getOpcode() != Z80::DEC_A) {
      ++MII;
      continue;
    }

    // I2: vreg = COPY $a (save dec'd value).  Critical: I2's
    // destination MUST be a different vreg than CounterVReg --
    // otherwise CounterVReg is redefined to the POST-dec value,
    // and the "reload" in I3 is actually loading the POST value,
    // not the PRE value.  That changes the meaning of the test:
    // OR_A would test POST==0, not PRE==0, and my SUB-with-carry
    // rewrite (which tests PRE==0) would be incorrect.
    auto I2 = nextNonMeta(std::next(I1), MIE);
    if (I2 == MIE || !isCopyFromA(*I2)) {
      ++MII;
      continue;
    }
    if (I2->getOperand(0).getReg() == CounterVReg) {
      // POST-test shape (regalloc reused the vreg).  Not a PRE-test.
      // Skip this match -- the test semantics are different.
      ++MII;
      continue;
    }

    // I3: $a = COPY <same counter vreg>
    auto I3 = nextNonMeta(std::next(I2), MIE);
    Register I3SrcVReg;
    if (I3 == MIE || !isCopyToA(*I3, I3SrcVReg) ||
        I3SrcVReg != CounterVReg) {
      ++MII;
      continue;
    }

    // I4: OR A operating on the just-reloaded $a (test for zero).
    // In post-MachineScheduler MIR this appears as `OR_r $a` (the
    // parametric form with $a as the source operand); the assembler
    // emits the same byte as `OR A`.  Match either.
    auto I4 = nextNonMeta(std::next(I3), MIE);
    if (I4 == MIE) {
      ++MII;
      continue;
    }
    bool I4Matches = false;
    if (I4->getOpcode() == Z80::OR_A) {
      I4Matches = true;
    } else if (I4->getOpcode() == Z80::OR_r) {
      // OR_r's first operand is the source register.  Match when it's $a.
      if (I4->getNumOperands() >= 1 && I4->getOperand(0).isReg() &&
          I4->getOperand(0).getReg() == Z80::A) {
        I4Matches = true;
      }
    }
    if (!I4Matches) {
      ++MII;
      continue;
    }

    // I5: conditional Z branch (JR_Z_e or JP_Z_nn).
    auto I5 = nextNonMeta(std::next(I4), MIE);
    unsigned CarryOpc;
    if (I5 == MIE || (CarryOpc = getCarryFormOpcode(I5->getOpcode())) == 0) {
      ++MII;
      continue;
    }

    LLVM_DEBUG(dbgs() << DEBUG_TYPE << ": rewrite in " << MBB.getName()
                      << " starting at " << *I0);

    // Rewrite:
    //   1. Replace DEC_A with SUB_n 1 (defines A, sets CARRY).
    //   2. Erase I3 (redundant reload).
    //   3. Erase I4 (OR_A no longer needed; carry is set by SUB_n).
    //   4. Change I5 from JR_Z/JP_Z to JR_C/JP_C.

    DebugLoc DL = I1->getDebugLoc();
    // SUB_n's implicit uses ($a) and defs ($a, FLAGS) are declared in
    // Z80InstrCommon.td; BuildMI inserts them automatically.
    BuildMI(MBB, I1, DL, TII->get(Z80::SUB_n)).addImm(1);

    // Erase the old DEC_A, redundant LD_A_R (I3), and OR_A (I4).
    I1->eraseFromParent();
    I3->eraseFromParent();
    I4->eraseFromParent();

    // Change I5's opcode (in-place opcode swap, operands unchanged).
    const MCInstrDesc &CarryDesc = TII->get(CarryOpc);
    I5->setDesc(CarryDesc);

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
