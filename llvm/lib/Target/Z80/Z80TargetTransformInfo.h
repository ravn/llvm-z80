//===- Z80TargetTransformInfo.h - Z80 specific TTI --------------*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares a TargetTransformInfo::Concept conforming object specific
// to the Z80 target machine. It uses the target's detailed information to
// provide more precise answers to certain TTI queries, while letting the
// target-independent and default TTI implementations handle the rest.
//
// The cost-model bodies live in Z80TargetTransformInfo.cpp (matching the
// in-tree convention for targets with a non-trivial cost model); this header
// only declares the overridden hooks.
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

  bool hasDivRemOp(Type *DataType, bool IsSigned) const override;

  bool isLSRCostLess(const TargetTransformInfo::LSRCost &C1,
                     const TargetTransformInfo::LSRCost &C2) const override;

  BranchProbability getPredictableBranchThreshold() const override;

  bool isValidAddrSpaceCast(unsigned FromAS, unsigned ToAS) const override;

  unsigned getNumberOfRegisters(unsigned ClassID) const override;

  TypeSize
  getRegisterBitWidth(TargetTransformInfo::RegisterKind K) const override;

  bool areInlineCompatible(const Function *Caller,
                           const Function *Callee) const override;

  bool prefersVectorizedAddressing() const override;

  InstructionCost getArithmeticInstrCost(
      unsigned Opcode, Type *Ty, TargetTransformInfo::TargetCostKind CostKind,
      TargetTransformInfo::OperandValueInfo Op1Info = {
          TargetTransformInfo::OK_AnyValue, TargetTransformInfo::OP_None},
      TargetTransformInfo::OperandValueInfo Op2Info = {
          TargetTransformInfo::OK_AnyValue, TargetTransformInfo::OP_None},
      ArrayRef<const Value *> Args = {},
      const Instruction *CxtI = nullptr) const override;

  InstructionCost
  getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src,
                   TargetTransformInfo::CastContextHint CCH,
                   TargetTransformInfo::TargetCostKind CostKind,
                   const Instruction *I = nullptr) const override;
};

} // end namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80TARGETTRANSFORMINFO_H
