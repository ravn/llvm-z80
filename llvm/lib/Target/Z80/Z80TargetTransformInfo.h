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
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/IR/Instructions.h"
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
    // Prefer instruction count to the other metrics.
    return std::tie(C1.Insns, C1.NumRegs, C1.AddRecCost, C1.NumIVMuls,
                    C1.NumBaseAdds, C1.ScaleCost, C1.ImmCost, C1.SetupCost) <
           std::tie(C2.Insns, C2.NumRegs, C2.AddRecCost, C2.NumIVMuls,
                    C2.NumBaseAdds, C2.ScaleCost, C2.ImmCost, C2.SetupCost);
  }

  BranchProbability getPredictableBranchThreshold() const override {
    return BranchProbability(0, 1);
  }

  // Z80 has BC, DE, HL as GP pairs (IX/IY reserved by default). LSR's
  // Cost::RateFormula adds `(NumRegs - TTIRegNum)` to C.Insns when a
  // formula requires more IVs than registers available — the only
  // proxy LSR has for spill cost. Reporting 3 here pressures LSR to
  // choose fewer IVs for loops that would otherwise exceed the file.
  unsigned getNumberOfRegisters(unsigned ClassID) const override {
    return 3;
  }

  // LSR often detects that its own transformed solution is more expensive
  // than the un-transformed baseline (nested loops with multiple pointer
  // IVs). On register-rich targets the baseline-check is a safety net; on
  // Z80 it's load-bearing — opt in so LSR discards its own regression.
  bool shouldDropLSRSolutionIfLessProfitable() const override { return true; }

  // Z80 has DJNZ: an 8-bit decrement-and-branch-if-nonzero on B.
  // Reporting profitability here causes LSR::RateRegister to zero the
  // AddRecCost of a {C,+,-1} i8 ICmpZero IV, so LSR prefers the narrow
  // down-counter over a stride-matched wide {−C,+,1} IV. We do NOT
  // register the HardwareLoops IR pass — no llvm.loop.decrement
  // intrinsics are generated; this hook is purely a cost-model signal
  // consumed by LSR.
  bool isHardwareLoopProfitable(Loop *L, ScalarEvolution &SE,
                                AssumptionCache &AC,
                                TargetLibraryInfo *LibInfo,
                                HardwareLoopInfo &HWLoopInfo) const override {
    const SCEV *EC = SE.getSymbolicMaxBackedgeTakenCount(L);
    if (isa<SCEVCouldNotCompute>(EC))
      return false;
    if (SE.getUnsignedRangeMax(EC).getActiveBits() > 8)
      return false;
    HWLoopInfo.CountType = Type::getInt8Ty(L->getHeader()->getContext());
    HWLoopInfo.LoopDecrement =
        ConstantInt::get(HWLoopInfo.CountType, 1);
    HWLoopInfo.CounterInReg = true;
    return true;
  }

  bool isValidAddrSpaceCast(unsigned FromAS, unsigned ToAS) const override {
    return true;
  }

  // Z80 has only 3 GP register pairs (BC, DE, HL). Inlining large functions
  // causes massive register spilling that dwarfs the benefit.
  // Allow inlining for:
  //   - functions marked inlinehint (e.g. Rust iterators)
  //   - small functions (≤ 10 instructions) where call overhead dominates
  bool areInlineCompatible(const Function *Caller,
                           const Function *Callee) const override {
    if (Callee->hasFnAttribute(Attribute::InlineHint))
      return true;
    if (Callee->getInstructionCount() <= 10)
      return true;
    return false;
  }
};

} // end namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80TARGETTRANSFORMINFO_H
