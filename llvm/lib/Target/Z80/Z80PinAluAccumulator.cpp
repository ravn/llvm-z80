//===-- Z80PinAluAccumulator.cpp - Pin ALU accumulators to A --------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// ravn/llvm-z80#172 -- structural fix for the 8-bit ALU accumulator
// shuttle.  For Z80's 8-bit ALU ops (XOR, AND, OR, ADD, SUB, ADC, SBC),
// the destination is always A.  When a loop carries an accumulator
// across iterations, regalloc that places it in a non-A register forces
// `ld a, r; ALU; ld r, a` round-trips around every ALU op -- a 12-ts
// per-iter cost that dominates the residual SDCC speed gap on AES.
//
// Hints don't move greedy regalloc on this path (verified in session
// 73n, see comment at Z80RegisterInfo.cpp:1891 from the #99 prior
// work).  The fix is structural: constrain the accumulator vreg to a
// single-register class containing only A.
//
// This pass mirrors Z80SplitDjnzCounters: find the accumulator vreg in
// a self-back-edge loop's MBB, insert a fresh `%new = COPY %old` at
// the loop preheader with %new in the AReg single-register class,
// then rename every in-MBB reference to %new.
//
// The COPY at the preheader becomes `LD A, r` (rarely necessary; A is
// typically free at loop entry) and post-RA copy propagation deletes
// it when the source vreg also lands in A.
//
//===----------------------------------------------------------------------===//

#include "Z80PinAluAccumulator.h"
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
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "z80-pin-alu-accumulator"

// Default off pending the liveness gating.  Session 73n's first
// implementation pins any GR8 vreg whose ALU-chain side matches, but
// when multiple chains' vregs are simultaneously live (e.g.,
// aes_mixColumns with three parallel XOR accumulators in nested
// blocks), forcing them all to A produces unsatisfiable constraints
// and either spill explosions (+285 B / +3.8 % ts on
// 01_baseline_Oz, 02_Os) or backend crashes (segfault on
// 05_Oz_static_stack).
//
// Needs: per-loop "primary accumulator" selection + interference
// check so only ONE vreg per overlapping live-range gets pinned.
// MachineLoopInfo + LiveIntervals would give this.  Not isolated
// this session.
//
// On `gf_alog_mini` standalone the pass does collapse part of the
// shuttle correctly (xor 27 / xor d chained in A without
// intermediate ld a, r).  Enable via `-mllvm
// -enable-z80-pin-alu-accumulator=true` for the standalone case.
static cl::opt<bool> EnablePinAluAccumulator(
    "enable-z80-pin-alu-accumulator", cl::init(false), cl::Hidden,
    cl::desc("Pin 8-bit ALU accumulator vregs to A by class "
             "(ravn/llvm-z80#172, off by default -- needs liveness "
             "gating before flipping on)"));

namespace {

class Z80PinAluAccumulator : public MachineFunctionPass {
public:
  static char ID;

  Z80PinAluAccumulator() : MachineFunctionPass(ID) {
    initializeZ80PinAluAccumulatorPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Z80 Pin ALU Accumulator";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

char Z80PinAluAccumulator::ID = 0;

INITIALIZE_PASS(Z80PinAluAccumulator, DEBUG_TYPE,
                "Z80 Pin ALU Accumulator", false, false)

// True iff Opc is one of the 8-bit ALU pseudos that reads A and writes A.
static bool isAluRMWOpc(unsigned Opc) {
  switch (Opc) {
  case Z80::XOR_r: case Z80::XOR_n:
  case Z80::AND_r: case Z80::AND_n:
  case Z80::OR_r:  case Z80::OR_n:
    return true;
  default:
    return false;
  }
}

// True iff Reg is a GR8 vreg whose sole use is `$a = COPY %Reg` AND that
// COPY is immediately followed (skipping meta) by an A-RMW ALU opcode.
// I.e., the vreg is the "in" half of the accumulator chain.
static bool isCopyToAFollowedByALU(Register Reg, const MachineRegisterInfo &MRI) {
  if (!Reg.isVirtual())
    return false;
  if (!Z80::GR8RegClass.hasSubClassEq(MRI.getRegClass(Reg)))
    return false;
  for (const MachineInstr &Use : MRI.use_nodbg_instructions(Reg)) {
    if (Use.getOpcode() != TargetOpcode::COPY)
      return false;
    if (!Use.getOperand(0).isReg() || Use.getOperand(0).getReg() != Z80::A)
      return false;
    auto J = std::next(Use.getIterator());
    auto E = Use.getParent()->end();
    while (J != E && J->isMetaInstruction())
      ++J;
    if (J == E || !isAluRMWOpc(J->getOpcode()))
      return false;
  }
  return true; // every use (and there is at least one) matches the pattern
}

// True iff Reg is a GR8 vreg whose sole def is `%Reg = COPY $a` AND that
// COPY is immediately preceded (skipping meta) by an A-RMW ALU opcode.
// I.e., the vreg is the "out" half of the accumulator chain.
static bool isCopyFromAAfterALU(Register Reg, const MachineRegisterInfo &MRI) {
  if (!Reg.isVirtual())
    return false;
  if (!Z80::GR8RegClass.hasSubClassEq(MRI.getRegClass(Reg)))
    return false;
  for (const MachineInstr &Def : MRI.def_instructions(Reg)) {
    if (Def.getOpcode() != TargetOpcode::COPY)
      return false;
    if (!Def.getOperand(1).isReg() || Def.getOperand(1).getReg() != Z80::A)
      return false;
    auto J = Def.getIterator();
    auto B = Def.getParent()->begin();
    if (J == B)
      return false;
    --J;
    while (J != B && J->isMetaInstruction())
      --J;
    if (!isAluRMWOpc(J->getOpcode()))
      return false;
  }
  return true;
}

bool Z80PinAluAccumulator::runOnMachineFunction(MachineFunction &MF) {
  if (!EnablePinAluAccumulator)
    return false;

  MachineRegisterInfo &MRI = MF.getRegInfo();
  bool Changed = false;

  // Walk every GR8 vreg and rewrite its register class to AReg if its
  // entire live range fits the accumulator pattern: every use is
  // `$a = COPY %v` followed by an ALU RMW, and every def is `%v = COPY $a`
  // preceded by an ALU RMW.  Constraining the class forces greedy
  // regalloc to allocate the vreg to A, collapsing the COPYs.
  for (unsigned i = 0, e = MRI.getNumVirtRegs(); i != e; ++i) {
    Register VReg = Register::index2VirtReg(i);
    if (MRI.reg_nodbg_empty(VReg))
      continue;
    if (!Z80::GR8RegClass.hasSubClassEq(MRI.getRegClass(VReg)))
      continue;
    if (MRI.getRegClass(VReg) == &Z80::ARegRegClass)
      continue; // already pinned

    // Pin when EITHER side of the chain matches.  Chains typically have
    // separate vregs on each side of A (with a PHI in between), so
    // requiring both would never fire.  Pin both halves independently.
    bool InMatches = isCopyToAFollowedByALU(VReg, MRI);
    bool OutMatches = isCopyFromAAfterALU(VReg, MRI);
    if (!InMatches && !OutMatches)
      continue;

    MRI.setRegClass(VReg, &Z80::ARegRegClass);
    Changed = true;
  }

  return Changed;
}

MachineFunctionPass *llvm::createZ80PinAluAccumulatorPass() {
  return new Z80PinAluAccumulator;
}
