//===- Z80TargetTransformInfo.h - Z80 specific TTI --------------*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines a TargetTransformInfo::Concept conforming object specific
// to the Z80 target machine. It uses the target's detailed information to
// provide more precise answers to certain TTI queries, while letting the
// target-independent and default TTI implementations handle the rest.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80TARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_Z80_Z80TARGETTRANSFORMINFO_H

#include "Z80TargetMachine.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/Support/BranchProbability.h"

namespace llvm {

class Z80TTIImpl : public BasicTTIImplBase<Z80TTIImpl> {
  using BaseT = BasicTTIImplBase<Z80TTIImpl>;

  friend BaseT;

  const Z80Subtarget *ST;
  const Z80TargetLowering *TLI;

  const Z80Subtarget *getST() const { return ST; }
  const Z80TargetLowering *getTLI() const { return TLI; }

public:
  explicit Z80TTIImpl(const Z80TargetMachine *TM, const Function &F)
      : BaseT(TM, F.getParent()->getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  // All div, rem, and divrem ops are libcalls, so any possible combination
  // exists.
  bool hasDivRemOp(Type *DataType, bool IsSigned) const override {
    return true;
  }

  bool isLSRCostLess(const TargetTransformInfo::LSRCost &C1,
                     const TargetTransformInfo::LSRCost &C2) const override {
    // Z80 has extreme register pressure (3 pairs). Penalize register count
    // most heavily, then instruction count.
    return std::tie(C1.NumRegs, C1.Insns, C1.AddRecCost, C1.NumIVMuls,
                    C1.NumBaseAdds, C1.ScaleCost, C1.ImmCost, C1.SetupCost) <
           std::tie(C2.NumRegs, C2.Insns, C2.AddRecCost, C2.NumIVMuls,
                    C2.NumBaseAdds, C2.ScaleCost, C2.ImmCost, C2.SetupCost);
  }

  BranchProbability getPredictableBranchThreshold() const override {
    return BranchProbability(0, 1);
  }

  bool isValidAddrSpaceCast(unsigned FromAS, unsigned ToAS) const override {
    return true;
  }

  // Z80 has extremely few registers. Tell the loop optimizer so it avoids
  // creating extra induction variables that cause spills.
  unsigned getNumberOfRegisters(unsigned ClassID) const {
    // ClassID 0 = scalar. Z80 has 3 allocatable 16-bit pairs (BC, DE, HL)
    // and 7 allocatable 8-bit regs (A, B, C, D, E, H, L), but pairs and
    // sub-registers overlap, so the effective count is very low.
    return 3;
  }

  TypeSize getRegisterBitWidth(TargetTransformInfo::RegisterKind K) const {
    return TypeSize::getFixed(8);
  }

  // Z80 CALL is 3 bytes + RET 1 byte = 4 bytes overhead per call.
  // Inlining a function with multiple call sites duplicates code and
  // increases register pressure (only 3 GP pairs), causing spills.
  // But single-call-site static functions should always be inlined:
  // eliminates CALL+RET overhead and the function body entirely,
  // with no code duplication.
  bool areInlineCompatible(const Function *Caller,
                           const Function *Callee) const override {
    // Tiny functions: always inline (body < CALL overhead).
    if (Callee->getInstructionCount() <= 2)
      return true;
    // Single-call-site internal functions: inline to eliminate the call.
    // hasOneUse() = one call site (the only reference to the function).
    if (Callee->hasInternalLinkage() && Callee->hasOneUse())
      return true;
    return false;
  }

};

} // end namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80TARGETTRANSFORMINFO_H
