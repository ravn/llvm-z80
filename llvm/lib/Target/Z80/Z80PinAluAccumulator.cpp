//===-- Z80PinAluAccumulator.cpp - Pin ALU accumulators to A --------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// ravn/llvm-z80#172 -- structural fix for the 8-bit ALU accumulator shuttle.
// Z80's 8-bit ALU writes A; a loop-carried accumulator placed in a non-A
// register forces `ld a,r; <alu>; ld r,a` round-trips every iteration.
//
// This pass runs AFTER PHI elimination (post-MachineScheduler, pre-RA), so the
// loop carrier is NOT a PHI -- it is a multiply-defined GR8 vreg threaded
// through A by COPYs (`$a = COPY %carrier; <A-ops>; %carrier = COPY $a`).  We
// find the connected component of GR8 vregs that thread through A together, and
// if that component's recurrence runs a genuine carrier ALU op (isCarrierAluOpc
// -- ADD/SUB/AND/OR/XOR, NOT DEC/INC which a counter uses and which have cheap
// non-A forms), we pin the whole component to the single-register AReg class so
// it stays resident in A.
//
// Guards against the over-constraint that regressed earlier attempts:
//   * a component is pinned only if no OTHER carrier component shares a block
//     (parallel accumulators -- AES mixColumns/subBytes -- are left alone);
//   * a component whose blocks contain a call is skipped (calls clobber A).
//
//===----------------------------------------------------------------------===//

#include "Z80PinAluAccumulator.h"
#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80InstrInfo.h"
#include "Z80Subtarget.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "z80-pin-alu-accumulator"

// Default OFF until the loop-carrier pin is proven net-positive on the AES
// corpus (see ravn/llvm-z80#172).  Flip on with
// -mllvm -enable-z80-pin-alu-accumulator=true.
static cl::opt<bool> EnablePinAluAccumulator(
    "enable-z80-pin-alu-accumulator", cl::init(false), cl::Hidden,
    cl::desc("Pin loop-carried 8-bit ALU accumulators to A by class "
             "(ravn/llvm-z80#172)."));

namespace {

class Z80PinAluAccumulator : public MachineFunctionPass {
public:
  static char ID;

  Z80PinAluAccumulator() : MachineFunctionPass(ID) {
    initializeZ80PinAluAccumulatorPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "Z80 Pin ALU Accumulator"; }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

char Z80PinAluAccumulator::ID = 0;

INITIALIZE_PASS(Z80PinAluAccumulator, DEBUG_TYPE, "Z80 Pin ALU Accumulator",
                false, false)

// Value-producing 8-bit ALU ops that REQUIRE A and have no cheaper non-A form.
// Their presence in a loop-carried recurrence marks a genuine accumulator.
// DEC_A/INC_A are deliberately excluded: a counter using them has the cheap
// `dec r` form, so it must stay out of A (freeing A for the real accumulator).
static bool isCarrierAluOpc(unsigned Opc) {
  switch (Opc) {
  case Z80::ADD_A_r:
  case Z80::ADD_A_A:
  case Z80::SUB_r:
  case Z80::AND_r:
  case Z80::AND_n:
  case Z80::OR_r:
  case Z80::OR_n:
  case Z80::XOR_r:
  case Z80::XOR_n:
    return true;
  default:
    return false;
  }
}

// Scan from an `$a = COPY %v` shuttle-in (forward) or a `%w = COPY $a`
// shuttle-out (backward) across the A-ops, returning the matching opposite
// shuttle's vreg if a carrier ALU op was seen in between (and setting HasAlu).
// Returns an invalid Register if the chain doesn't match.
static Register scanShuttle(MachineBasicBlock::iterator Start,
                            MachineBasicBlock *Par, bool Forward, bool &HasAlu) {
  bool SawAlu = false;
  if (Forward) {
    for (auto J = std::next(Start); J != Par->end(); ++J) {
      if (J->isMetaInstruction())
        continue;
      if (isCarrierAluOpc(J->getOpcode())) {
        SawAlu = true;
        HasAlu = true;
        continue;
      }
      if (J->getOpcode() == TargetOpcode::COPY && J->getOperand(1).isReg() &&
          J->getOperand(1).getReg() == Z80::A) {
        if (SawAlu && J->getOperand(0).isReg())
          return J->getOperand(0).getReg();
        return Register();
      }
      if (J->definesRegister(Z80::A, /*TRI=*/nullptr))
        return Register();
    }
    return Register();
  }
  // Backward from a `%w = COPY $a` def.
  for (auto J = std::next(MachineBasicBlock::reverse_iterator(Start));
       J != Par->rend(); ++J) {
    if (J->isMetaInstruction())
      continue;
    if (isCarrierAluOpc(J->getOpcode())) {
      SawAlu = true;
      HasAlu = true;
      continue;
    }
    if (J->getOpcode() == TargetOpcode::COPY && J->getOperand(0).isReg() &&
        J->getOperand(0).getReg() == Z80::A) {
      if (SawAlu && J->getOperand(1).isReg())
        return J->getOperand(1).getReg();
      return Register();
    }
    if (J->definesRegister(Z80::A, /*TRI=*/nullptr))
      return Register();
  }
  return Register();
}

// Collect the connected component of GR8 vregs that thread through A together
// with Seed.  Connections: plain vreg COPYs (carrier pieces) and A-shuttles
// (`$a = COPY V; <ALU>; W = COPY $a`).  Multi-def safe (post-PHI form).
static void collectComponent(Register Seed, MachineRegisterInfo &MRI,
                             DenseSet<Register> &Comp,
                             SmallPtrSetImpl<MachineBasicBlock *> &Blocks,
                             bool &HasAlu) {
  SmallVector<Register, 8> Work{Seed};
  Comp.insert(Seed);
  auto Add = [&](Register R) {
    if (R.isVirtual() &&
        Z80::GR8RegClass.hasSubClassEq(MRI.getRegClass(R)) &&
        Comp.insert(R).second)
      Work.push_back(R);
  };

  while (!Work.empty()) {
    Register V = Work.pop_back_val();

    for (MachineInstr &D : MRI.def_instructions(V)) {
      Blocks.insert(D.getParent());
      if (D.getOpcode() == TargetOpcode::COPY && D.getOperand(1).isReg()) {
        Register Src = D.getOperand(1).getReg();
        if (Src == Z80::A) {
          Register X = scanShuttle(D.getIterator(), D.getParent(),
                                   /*Forward=*/false, HasAlu);
          Add(X);
        } else {
          Add(Src);
        }
      }
    }

    for (MachineInstr &U : MRI.use_nodbg_instructions(V)) {
      Blocks.insert(U.getParent());
      if (U.getOpcode() != TargetOpcode::COPY || !U.getOperand(0).isReg())
        continue;
      Register Dst = U.getOperand(0).getReg();
      if (Dst == Z80::A) {
        Register W = scanShuttle(U.getIterator(), U.getParent(),
                                 /*Forward=*/true, HasAlu);
        Add(W);
      } else {
        Add(Dst);
      }
    }
  }
}

bool Z80PinAluAccumulator::runOnMachineFunction(MachineFunction &MF) {
  if (!EnablePinAluAccumulator)
    return false;

  MachineRegisterInfo &MRI = MF.getRegInfo();

  struct Carrier {
    DenseSet<Register> Comp;
    SmallPtrSet<MachineBasicBlock *, 4> Blocks;
  };
  SmallVector<Carrier, 4> Carriers;
  DenseSet<Register> Visited;

  for (unsigned i = 0, e = MRI.getNumVirtRegs(); i != e; ++i) {
    Register VReg = Register::index2VirtReg(i);
    if (MRI.reg_nodbg_empty(VReg) || Visited.count(VReg))
      continue;
    if (!Z80::GR8RegClass.hasSubClassEq(MRI.getRegClass(VReg)) ||
        MRI.getRegClass(VReg) == &Z80::ARegRegClass)
      continue;

    Carrier C;
    bool HasAlu = false;
    collectComponent(VReg, MRI, C.Comp, C.Blocks, HasAlu);
    for (Register R : C.Comp)
      Visited.insert(R);
    if (HasAlu)
      Carriers.push_back(std::move(C));
  }

  bool Changed = false;
  for (unsigned I = 0, E = Carriers.size(); I != E; ++I) {
    Carrier &C = Carriers[I];

    // Parallel-accumulator gate: skip if another carrier shares a block.
    bool Conflicts = false;
    for (unsigned J = 0; J != E && !Conflicts; ++J) {
      if (J == I)
        continue;
      for (MachineBasicBlock *B : Carriers[J].Blocks)
        if (C.Blocks.count(B)) {
          Conflicts = true;
          break;
        }
    }
    if (Conflicts)
      continue;

    // Call gate: calls clobber A.
    bool HasCall = false;
    for (MachineBasicBlock *B : C.Blocks) {
      for (MachineInstr &MI : *B)
        if (MI.isCall()) {
          HasCall = true;
          break;
        }
      if (HasCall)
        break;
    }
    if (HasCall)
      continue;

    for (Register V : C.Comp)
      if (V.isVirtual() &&
          Z80::GR8RegClass.hasSubClassEq(MRI.getRegClass(V)) &&
          MRI.getRegClass(V) != &Z80::ARegRegClass) {
        MRI.setRegClass(V, &Z80::ARegRegClass);
        Changed = true;
      }
  }

  return Changed;
}

MachineFunctionPass *llvm::createZ80PinAluAccumulatorPass() {
  return new Z80PinAluAccumulator;
}
