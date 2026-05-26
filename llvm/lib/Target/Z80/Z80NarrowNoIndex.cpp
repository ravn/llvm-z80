//===-- Z80NarrowNoIndex.cpp - Keep IX/IY-incompatible values out of IX/IY ===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// #189 / #27 / #112 -- when IY is allocatable (the -z80-unreserve-iy bring-up
// path), the greedy allocator and the spiller may place a plain-GR16 value in
// IX/IY.  That is correct for a value used only as a whole 16-bit pair, but
// wrong for a value that is accessed a byte at a time: IX/IY's halves
// (IXL/IXH, IYL/IYH) have no documented 8-bit ops, so a byte access of an
// IX/IY-resident value becomes either a `push/pop` shuttle (density regression,
// and a miscompile in the default config when it perturbs SP-relative spill
// slots; #189) or -- worse -- an UNDOCUMENTED `xor iyh` / `ld iyl` etc. emitted
// without +undocumented.
//
// Most byte-decomposed 16-bit values are already created in GR16NoIR (= GR16
// minus IX/IY) by the TableGen instruction operand classes (LSHR16,
// XOR_CMP_EQ16, the shift-chain pseudos, ...), and getLargestLegalSuperClass no
// longer re-widens GR16NoIR to GR16 under -z80-unreserve-iy (see
// Z80RegisterInfo.cpp).  Two leaks remain that this pass closes, both plain
// GR16 vregs that the class machinery does not catch:
//
//   (a) Explicitly byte-decomposed: a vreg with a sub_lo/sub_hi operand
//       (e.g. the loop-carried i32 half in crc_one that arrives as COPY $de and
//       is only consumed via byte COPYs -- nothing constrains it to GR16NoIR).
//
//   (b) Implicitly byte-decomposed by a consumer: a vreg USED by an instruction
//       whose operand register class is GR16NoIR (e.g. the LD_r16_nn 0 constants
//       feeding XOR_CMP_EQ16 in popcount32).  The consumer byte-decomposes the
//       value at post-RA expansion, but the GR16NoIR operand constraint is not
//       enforced on a rematerialized constant, so it can be remat'd straight
//       into IY -> `ld iy,0; xor iyh`.
//
// For both, IX/IY is unconditionally the wrong home (no pressure/loop shape
// makes it preferable), so this is a LEGALITY statement -- a register class,
// not a cost.  The pass narrows such vregs to GR16NoIR pre-RA.
//
// Gated on Z80UnreserveIY: when IY is reserved (the production default), GR16
// and GR16NoIR have the same allocatable set {DE,HL,BC}, but the class
// distinction still affects coalescing, so narrowing unconditionally would
// perturb production codegen.  Running only when IY is allocatable keeps
// production bit-for-bit identical.
//
// See tasks/issue189-27-regalloc-cost-model-drill-2026-05-25.md.
//
//===----------------------------------------------------------------------===//

#include "Z80NarrowNoIndex.h"
#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80InstrInfo.h"
#include "Z80Subtarget.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "z80-narrow-no-index"

STATISTIC(NumNarrowed, "Number of GR16 vregs narrowed to GR16NoIR (kept out of IX/IY)");

// Defined in Z80RegisterInfo.cpp: the #112 IY bring-up flag.
namespace llvm {
extern cl::opt<bool> Z80UnreserveIY;
bool z80IsIYAllocatable(const MachineFunction &MF);
}

namespace {

class Z80NarrowNoIndex : public MachineFunctionPass {
public:
  static char ID;

  Z80NarrowNoIndex() : MachineFunctionPass(ID) {
    initializeZ80NarrowNoIndexPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Z80 Narrow IX/IY-incompatible GR16 values to GR16NoIR";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

char Z80NarrowNoIndex::ID = 0;

INITIALIZE_PASS(Z80NarrowNoIndex, DEBUG_TYPE,
                "Z80 Narrow IX/IY-incompatible GR16 values to GR16NoIR", false,
                false)

// True iff some use of Reg requires IX/IY exclusion: either Reg is named with a
// sub_lo/sub_hi sub-register (explicit byte access), or Reg is an explicit
// operand of an instruction whose register-class constraint at that operand is
// exactly GR16NoIR (implicit byte access inside a pseudo, e.g. XOR_CMP_EQ16).
static bool mustAvoidIndex(Register Reg, const MachineRegisterInfo &MRI,
                           const TargetInstrInfo &TII) {
  for (const MachineOperand &MO : MRI.reg_nodbg_operands(Reg)) {
    unsigned Sub = MO.getSubReg();
    if (Sub == Z80::sub_lo || Sub == Z80::sub_hi)
      return true;

    const MachineInstr &MI = *MO.getParent();
    if (MI.isCopy() || MI.isPHI() || MI.isRegSequence() ||
        MI.isImplicitDef() || MI.isSubregToReg())
      continue; // these carry no fixed operand class
    unsigned OpIdx = MO.getOperandNo();
    if (OpIdx >= MI.getNumOperands())
      continue;
    const TargetRegisterClass *OpRC = TII.getRegClass(MI.getDesc(), OpIdx);
    if (OpRC == &Z80::GR16NoIRRegClass)
      return true;
  }
  return false;
}

bool Z80NarrowNoIndex::runOnMachineFunction(MachineFunction &MF) {
  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  // IX/IY are Z80-only, and there is nothing to keep out of them when IY is
  // reserved -- running then would only perturb production coalescing.
  if (!STI.hasZ80() || !z80IsIYAllocatable(MF))  // #38: flag OR size-opt+static-stack
    return false;

  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetInstrInfo &TII = *STI.getInstrInfo();
  bool Changed = false;

  for (unsigned I = 0, E = MRI.getNumVirtRegs(); I != E; ++I) {
    Register Reg = Register::index2VirtReg(I);
    if (MRI.reg_nodbg_empty(Reg))
      continue;
    // Only the IX/IY-including class leaks; the many already-GR16NoIR vregs are
    // fine, and an IX/IY-mandatory vreg is no longer plain GR16 by this point.
    if (MRI.getRegClass(Reg) != &Z80::GR16RegClass)
      continue;
    if (!mustAvoidIndex(Reg, MRI, TII))
      continue;

    LLVM_DEBUG(dbgs() << "  narrowing " << printReg(Reg)
                      << " GR16 -> GR16NoIR\n");
    MRI.setRegClass(Reg, &Z80::GR16NoIRRegClass);
    ++NumNarrowed;
    Changed = true;
  }

  return Changed;
}

MachineFunctionPass *llvm::createZ80NarrowNoIndexPass() {
  return new Z80NarrowNoIndex;
}
