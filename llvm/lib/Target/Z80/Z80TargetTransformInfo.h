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
  unsigned getNumberOfRegisters(unsigned ClassID) const override {
    // ClassID 0 = scalar. Z80 has 3 allocatable 16-bit pairs (BC, DE, HL)
    // and 7 allocatable 8-bit regs (A, B, C, D, E, H, L), but pairs and
    // sub-registers overlap, so the effective count is very low.
    return 3;
  }

  TypeSize getRegisterBitWidth(TargetTransformInfo::RegisterKind K) const override {
    return TypeSize::getFixed(8);
  }

  // Z80 CALL is 3 bytes + RET 1 byte = 4 bytes overhead per call.
  // Inlining large functions causes massive register spilling.
  // Allow inlining for:
  //   - functions marked inlinehint (e.g. Rust iterators)
  //   - small functions (≤ 10 instructions) where call overhead dominates
  //   - single-call-site internal functions (no code duplication)
  bool areInlineCompatible(const Function *Caller,
                           const Function *Callee) const override {
    if (Callee->hasFnAttribute(Attribute::InlineHint))
      return true;
    if (Callee->getInstructionCount() <= 10)
      return true;
    if (Callee->hasInternalLinkage() && Callee->hasOneUse())
      return true;
    return false;
  }

  // Z80 has no vector / SIMD instructions and no auto-vectorization
  // story.  The target-independent default returns true, which
  // encourages passes (vectorize / LSR) to treat vector-style
  // addressing as cheap.  On Z80 it would just be dead-code paths.
  bool prefersVectorizedAddressing() const override { return false; }

  // ravn/llvm-z80#177 Phase B (post-B1 bisect): only the clean cases
  // ship.  See `tasks/issue177-phase-b1-finding.md` and
  // `tasks/issue177-phase-b2-bisect.md`.
  //
  // Cases that ship:
  //   getArithmeticInstrCost: Mul -> TCC_Expensive (Z80 has no mul).
  //   getCastInstrCost:       i16->i8 trunc, i8->i16 zext free;
  //                           i8->i16 sext = 2.
  //
  // Case held back (correctness-safe post #184 + #185 fixes, but
  // production-target cost > savings):
  //   getArithmeticInstrCost: i16 = 2 / i32 = 4 / i64+ = expensive.
  //   With both #184 (peephole #148 fall-through MBB check) and
  //   #185 (DJNZ peephole B-clobber safety) fixed, AES corpus
  //   13/13 PASS with i16=2 applied.  But production targets:
  //     - cpnos PROM1: 2028 -> 2037 B (+9 B; eats into 2 KB hard cap)
  //     - autoload PROM: 1652 -> 1668 B (+16 B)
  //     - AES `09_Oz_prod_like`: 2562 -> 2606 B (+44 B)
  //     - BIOS: 5922 -> 5916 B (−6 B)
  //   Net: production cost outweighs benefits.  Skipping the i16
  //   width charge keeps the IR pipeline's existing IV decisions,
  //   which LSR canonicalizes acceptably for Z80 (see also #169/#170/
  //   #171: Z80NarrowIV removed in session 73q because LSR's choice
  //   under these TTI hooks already produces the desired narrowed
  //   form on the documented inputs).

  InstructionCost getArithmeticInstrCost(
      unsigned Opcode, Type *Ty, TargetTransformInfo::TargetCostKind CostKind,
      TargetTransformInfo::OperandValueInfo Op1Info = {
          TargetTransformInfo::OK_AnyValue, TargetTransformInfo::OP_None},
      TargetTransformInfo::OperandValueInfo Op2Info = {
          TargetTransformInfo::OK_AnyValue, TargetTransformInfo::OP_None},
      ArrayRef<const Value *> Args = {},
      const Instruction *CxtI = nullptr) const override {
    // Z80 has no hardware multiplier; mul is a libcall.  Default
    // returns TCC_Expensive for SDiv/SRem/UDiv/URem/FDiv/FRem but
    // not Mul.  Add Mul to the libcall set.
    if (Opcode == Instruction::Mul)
      return TargetTransformInfo::TCC_Expensive;
    return BaseT::getArithmeticInstrCost(Opcode, Ty, CostKind, Op1Info,
                                         Op2Info, Args, CxtI);
  }

  InstructionCost
  getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src,
                   TargetTransformInfo::CastContextHint CCH,
                   TargetTransformInfo::TargetCostKind CostKind,
                   const Instruction *I = nullptr) const override {
    if (Dst->isIntegerTy() && Src->isIntegerTy()) {
      unsigned DstBits = Dst->getIntegerBitWidth();
      unsigned SrcBits = Src->getIntegerBitWidth();
      // Trunc i16 -> i8 is free on Z80: just use the low byte of the pair.
      if (Opcode == Instruction::Trunc && SrcBits == 16 && DstBits == 8)
        return 0;
      // Zext i8 -> i16 is essentially free: LD H, 0.
      if (Opcode == Instruction::ZExt && SrcBits == 8 && DstBits == 16)
        return 0;
      // Sext i8 -> i16 needs a sign-bit test sequence (~3 bytes:
      // LD A, L; RLCA; SBC A, A; LD H, A).
      if (Opcode == Instruction::SExt && SrcBits == 8 && DstBits == 16)
        return 2;
    }
    return BaseT::getCastInstrCost(Opcode, Dst, Src, CCH, CostKind, I);
  }
};

} // end namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80TARGETTRANSFORMINFO_H
