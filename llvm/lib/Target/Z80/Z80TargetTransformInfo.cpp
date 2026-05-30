//===- Z80TargetTransformInfo.cpp - Z80 specific TTI ----------------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Z80-specific TargetTransformInfo cost hooks.  The
// guiding facts about the target:
//   * 3 allocatable 16-bit register pairs (BC/DE/HL) + IX/IY (prefixed, costly)
//     -> extreme register pressure; spills are expensive (3-4 B each).
//   * 8-bit accumulator-centric ALU; 16-bit ALU is limited to ADD HL,rr / INC /
//     DEC; there is no hardware multiply (mul/div/rem are libcalls).
//   * No branch predictor; no SIMD / vectorization.
//
// Costs are reported CostKind-aware where the byte cost (TCK_CodeSize /
// TCK_SizeAndLatency) and the throughput/latency cost (TCK_RecipThroughput /
// TCK_Latency) genuinely differ on Z80.
//
//===----------------------------------------------------------------------===//

#include "Z80TargetTransformInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

#define DEBUG_TYPE "z80tti"

// All div, rem, and divrem ops are libcalls, so any possible combination
// exists.
bool Z80TTIImpl::hasDivRemOp(Type *DataType, bool IsSigned) const {
  return true;
}

bool Z80TTIImpl::isLSRCostLess(const TargetTransformInfo::LSRCost &C1,
                               const TargetTransformInfo::LSRCost &C2) const {
  // Z80 has extreme register pressure (3 pairs). Penalize register count
  // most heavily, then instruction count.
  return std::tie(C1.NumRegs, C1.Insns, C1.AddRecCost, C1.NumIVMuls,
                  C1.NumBaseAdds, C1.ScaleCost, C1.ImmCost, C1.SetupCost) <
         std::tie(C2.NumRegs, C2.Insns, C2.AddRecCost, C2.NumIVMuls,
                  C2.NumBaseAdds, C2.ScaleCost, C2.ImmCost, C2.SetupCost);
}

BranchProbability Z80TTIImpl::getPredictableBranchThreshold() const {
  return BranchProbability(0, 1);
}

bool Z80TTIImpl::isValidAddrSpaceCast(unsigned FromAS, unsigned ToAS) const {
  return true;
}

// Z80 has extremely few registers. Tell the loop optimizer so it avoids
// creating extra induction variables that cause spills.
unsigned Z80TTIImpl::getNumberOfRegisters(unsigned ClassID) const {
  // ClassID 0 = scalar. Z80 has 3 allocatable 16-bit pairs (BC, DE, HL)
  // and 7 allocatable 8-bit regs (A, B, C, D, E, H, L), but pairs and
  // sub-registers overlap, so the effective count is very low.
  return 3;
}

TypeSize
Z80TTIImpl::getRegisterBitWidth(TargetTransformInfo::RegisterKind K) const {
  return TypeSize::getFixed(8);
}

// Z80 CALL is 3 bytes + RET 1 byte = 4 bytes overhead per call.
// Inlining large functions causes massive register spilling.
// Allow inlining for:
//   - functions marked inlinehint (e.g. Rust iterators)
//   - small functions (<= 10 instructions) where call overhead dominates
//   - single-call-site internal functions (no code duplication)
bool Z80TTIImpl::areInlineCompatible(const Function *Caller,
                                     const Function *Callee) const {
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
bool Z80TTIImpl::prefersVectorizedAddressing() const { return false; }

// "Legal add immediate" means the target can add this constant to a register
// WITHOUT first materializing it into another register.  Z80 has no
// `ADD HL,nn` / `ADD rr,nn`: a 16-bit constant add costs `LD rr,nn` (3 B) +
// `ADD HL,rr` (1 B) = 4 B, the value materialized first.  The only
// no-materialize option is a chain of `INC/DEC rr` (1 B each), strictly
// cheaper than materialize-and-add only for |Imm| <= 3 (3 B vs 4 B; |Imm|==4
// breaks even).  The TargetLoweringBase default returns true for EVERY
// immediate -- inaccurate for Z80.
//
// Effect (measured, not assumed): LSR consults this when scoring formulas.
// With the default, for a loop touching several arrays at fixed offsets from
// one base (the aes_done key-zeroing shape; see the lit test), LSR keeps a
// single base IV + folded member offsets -- a shape that does not fit Z80's 3
// register pairs, so the backend spills a pointer pair into the loop.
// Reporting the truth changes LSR's formula/IV choice so the pointers stay in
// registers and the in-loop spill disappears.  NB: the corrected model does
// NOT eliminate large-immediate adds (the strength-reduced code still has
// `ld de,nn; add hl,de` steps); the win is reduced register pressure, net
// fewer instructions.  Measured on the AES corpus: -8..-124 B and faster on
// every LSR-active config; production configs (-disable-lsr) byte-identical,
// so this cannot regress the production targets.  (ravn/llvm-z80#177.)
bool Z80TTIImpl::isLegalAddImmediate(int64_t Imm) const {
  return Imm >= -3 && Imm <= 3;
}

// ravn/llvm-z80#177 Phase B (post-B1 bisect): only the clean cases
// ship.  See `tasks/issue177-phase-b1-finding.md` and
// `tasks/issue177-phase-b2-bisect.md`.
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
//     - BIOS: 5922 -> 5916 B (-6 B)
//   Net: production cost outweighs benefits.  The i16/i32 width charge
//   cannot be CostKind-isolated either: the consumer that benefits
//   (IndVarSimplify IV-widening) queries getArithmeticInstrCost(Add) at
//   TCK_RecipThroughput -- the same bucket as any speed gain -- so a
//   per-CostKind split does not separate the win from the regression.
//   Held until the +static-stack miscompile behind #184 is root-caused
//   separately (a MIR/BSS-lowering issue, not a cost-model one).
InstructionCost Z80TTIImpl::getArithmeticInstrCost(
    unsigned Opcode, Type *Ty, TargetTransformInfo::TargetCostKind CostKind,
    TargetTransformInfo::OperandValueInfo Op1Info,
    TargetTransformInfo::OperandValueInfo Op2Info,
    ArrayRef<const Value *> Args, const Instruction *CxtI) const {
  // Z80 has no hardware multiplier; mul is a libcall.  Default
  // returns TCC_Expensive for SDiv/SRem/UDiv/URem/FDiv/FRem but
  // not Mul.  Add Mul to the libcall set.
  if (Opcode == Instruction::Mul)
    return TargetTransformInfo::TCC_Expensive;

  // Z80 has no barrel shifter.  Each bit of a shift is a separate single-bit
  // shift/rotate (SLA/SRL/SRA r = 2 B/8 T; left-shift can use ADD HL,HL for
  // i16), and a *variable* shift count compiles to a runtime loop.  The
  // BasicTTIImpl default reports legal shifts as cost 1 -- far too cheap.
  // Cost scales with BOTH the shift amount AND the width in bytes: an N-byte
  // shift moves one bit across all N limbs (RR/SLA + carry) per position, so a
  // 32-bit shift-by-1 is ~4 ops, not 1.  Undercharging wide shifts as 1 made
  // LoopUnroll fully unroll wide-int shift loops (e.g. a CRC inner loop blew up
  // ~3.4x at -O2).  A variable count compiles to a runtime loop -> expensive.
  if (Opcode == Instruction::Shl || Opcode == Instruction::LShr ||
      Opcode == Instruction::AShr) {
    unsigned Bytes = std::max(1u, (Ty->getScalarSizeInBits() + 7) / 8);
    const ConstantInt *AmtC = nullptr;
    if (Args.size() == 2)
      AmtC = dyn_cast_or_null<ConstantInt>(Args[1]);
    if (!AmtC && CxtI && CxtI->getNumOperands() == 2)
      AmtC = dyn_cast_or_null<ConstantInt>(CxtI->getOperand(1));
    if (!AmtC)
      return TargetTransformInfo::TCC_Expensive * Bytes; // variable -> loop
    unsigned Amt = AmtC->getValue().getZExtValue();
    if (Amt == 0)
      return 0;
    // ~Bytes single-bit ops per shifted position; cap the per-position amount
    // (the ShiftRotateChain pass open-codes wide constant shifts).
    return std::min(Amt, 8u) * Bytes;
  }

  return BaseT::getArithmeticInstrCost(Opcode, Ty, CostKind, Op1Info, Op2Info,
                                       Args, CxtI);
}

InstructionCost
Z80TTIImpl::getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src,
                             TargetTransformInfo::CastContextHint CCH,
                             TargetTransformInfo::TargetCostKind CostKind,
                             const Instruction *I) const {
  if (Dst->isIntegerTy() && Src->isIntegerTy()) {
    unsigned DstBits = Dst->getIntegerBitWidth();
    unsigned SrcBits = Src->getIntegerBitWidth();
    // Truncation is always free on Z80 -- the narrow value is the low bytes of
    // the wide one; the high bytes are simply dropped.  (The base implementation
    // charges 1 for e.g. i64->i32; correct that to 0 for all widths.)
    if (Opcode == Instruction::Trunc && DstBits < SrcBits)
      return 0;
    // Zero-extension clears the new high bytes (LD r, 0).  i8 -> i16 is a
    // single LD H, 0, essentially free; wider zexts fall through to the base
    // (~1), which is close enough.
    if (Opcode == Instruction::ZExt && SrcBits == 8 && DstBits == 16)
      return 0;
    // Sign-extension computes the sign (LD A,lo; RLCA; SBC A,A ~ 2 ops) then
    // splats it into each new high byte.  Cost scales with the added bytes and
    // must be monotonic in width -- the base undercharges wider sexts (it
    // reports sext i8->i32 cheaper than sext i8->i16, which is backwards).
    if (Opcode == Instruction::SExt && DstBits > SrcBits) {
      unsigned ExtraBytes = (DstBits - SrcBits + 7) / 8;
      return 1 + ExtraBytes; // i8->i16: 2; i16->i32: 3; i8->i32: 4; i32->i64: 5
    }
  }
  return BaseT::getCastInstrCost(Opcode, Dst, Src, CCH, CostKind, I);
}
