//===-- Z80PreLegalizerCombiner.cpp
//----------------------------------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Combines on generic MachineInstrs before legalization.
//
//===----------------------------------------------------------------------===//

#include "Z80.h"
#include "Z80Combiner.h"
#include "Z80Subtarget.h"
#include "llvm/CodeGen/GlobalISel/CSEInfo.h"
#include "llvm/CodeGen/GlobalISel/Combiner.h"
#include "llvm/CodeGen/GlobalISel/GenericMachineInstrs.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/CombinerInfo.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "llvm/CodeGen/GlobalISel/GISelValueTracking.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"

#define GET_GICOMBINER_DEPS
#include "Z80GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_DEPS

#define DEBUG_TYPE "z80-prelegalizer-combiner"

using namespace llvm;

// Emergency-off switch for the wide-copy -> block-move combine.
static cl::opt<bool> EnableWideCopyBlockMove(
    "z80-wide-copy-block-move", cl::init(true), cl::Hidden,
    cl::desc("Rewrite wide scalar load+store pairs as block moves"));

namespace {

#define GET_GICOMBINER_TYPES
#include "Z80GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_TYPES

// Wide scalar load+store pair -> block move (see Z80Combine.td).
//
// Soundness:
//  - `store (load p), q` reads all bytes before writing any, which is
//    exactly memmove semantics — G_MEMMOVE is correct even when p and q
//    overlap, so no alias proof is needed between p and q themselves.
//  - The rewrite happens at the STORE's position, i.e. the read moves
//    DOWN to the store point.  Any intervening instruction that may
//    write memory (or a call) could alias the source, so bail on those.
bool matchWideCopyToBlockMove(MachineInstr &MI, MachineRegisterInfo &MRI,
                              MachineInstr *&LoadMI) {
  if (!EnableWideCopyBlockMove)
    return false;
  auto &Store = cast<GStore>(MI);
  if (Store.isAtomic() || Store.isVolatile())
    return false;

  Register Val = Store.getValueReg();
  LLT Ty = MRI.getType(Val);
  // Wider than a register pair, whole bytes only.  i16 and below are
  // native; non-byte sizes keep their existing lowering.
  if (!Ty.isScalar() || Ty.getSizeInBits() < 32 || Ty.getSizeInBits() % 8)
    return false;
  // Full-width store of the loaded value only (no truncating store).
  if (Store.getMMO().getSizeInBits().getValue() != Ty.getSizeInBits())
    return false;
  if (!MRI.hasOneNonDBGUse(Val))
    return false;

  MachineInstr *Def = MRI.getVRegDef(Val);
  auto *Load = dyn_cast_or_null<GLoad>(Def);
  if (!Load || Load->isAtomic() || Load->isVolatile())
    return false;
  if (Load->getMMO().getSizeInBits().getValue() != Ty.getSizeInBits())
    return false;

  // Same address space on both sides (and not the I/O-port space).
  LLT DstPtrTy = MRI.getType(Store.getPointerReg());
  LLT SrcPtrTy = MRI.getType(Load->getPointerReg());
  if (DstPtrTy.getAddressSpace() != 0 || SrcPtrTy.getAddressSpace() != 0)
    return false;

  // The read moves down to the store point: nothing in between may write.
  if (Load->getParent() != Store.getParent())
    return false;
  unsigned Scanned = 0;
  for (auto It = std::next(Load->getIterator()); It != Store.getIterator();
       ++It) {
    if (++Scanned > 32)
      return false;
    if (It->mayStore() || It->isCall() || It->hasUnmodeledSideEffects())
      return false;
  }

  LoadMI = Load;
  return true;
}

void applyWideCopyToBlockMove(MachineInstr &MI, MachineRegisterInfo &MRI,
                              MachineIRBuilder &B, MachineInstr *LoadMI) {
  auto &Store = cast<GStore>(MI);
  auto &Load = cast<GLoad>(*LoadMI);

  B.setInsertPt(*MI.getParent(), MI.getIterator());
  unsigned Bytes = MRI.getType(Store.getValueReg()).getSizeInBytes();
  auto Size = B.buildConstant(LLT::scalar(16), Bytes);
  B.buildInstr(TargetOpcode::G_MEMMOVE)
      .addUse(Store.getPointerReg())
      .addUse(Load.getPointerReg())
      .addUse(Size.getReg(0))
      .addImm(0) // not a tail call
      .addMemOperand(&Store.getMMO())
      .addMemOperand(&Load.getMMO());
  MI.eraseFromParent();
  LoadMI->eraseFromParent();
}

class Z80PreLegalizerCombinerImpl : public Combiner {
protected:
  const CombinerHelper Helper;
  const Z80PreLegalizerCombinerImplRuleConfig &RuleConfig;
  const Z80Subtarget &STI;

public:
  Z80PreLegalizerCombinerImpl(
      MachineFunction &MF, CombinerInfo &CInfo, GISelValueTracking &VT,
      GISelCSEInfo *CSEInfo,
      const Z80PreLegalizerCombinerImplRuleConfig &RuleConfig,
      const Z80Subtarget &STI, MachineDominatorTree *MDT,
      const LegalizerInfo *LI);

  static const char *getName() { return "Z80PreLegalizerCombiner"; }

  bool tryCombineAll(MachineInstr &I) const override;

private:
#define GET_GICOMBINER_CLASS_MEMBERS
#include "Z80GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CLASS_MEMBERS
};

#define GET_GICOMBINER_IMPL
#include "Z80GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_IMPL

Z80PreLegalizerCombinerImpl::Z80PreLegalizerCombinerImpl(
    MachineFunction &MF, CombinerInfo &CInfo, GISelValueTracking &VT,
    GISelCSEInfo *CSEInfo,
    const Z80PreLegalizerCombinerImplRuleConfig &RuleConfig,
    const Z80Subtarget &STI, MachineDominatorTree *MDT, const LegalizerInfo *LI)
    : Combiner(MF, CInfo, &VT, CSEInfo),
      Helper(Observer, B, /*IsPreLegalize*/ true, &VT, MDT, LI),
      RuleConfig(RuleConfig), STI(STI),
#define GET_GICOMBINER_CONSTRUCTOR_INITS
#include "Z80GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CONSTRUCTOR_INITS
{
}

// Pass boilerplate
class Z80PreLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;

  Z80PreLegalizerCombiner();

  StringRef getPassName() const override { return "Z80PreLegalizerCombiner"; }

  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  Z80PreLegalizerCombinerImplRuleConfig RuleConfig;
};
} // end anonymous namespace

void Z80PreLegalizerCombiner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  getSelectionDAGFallbackAnalysisUsage(AU);
  AU.addRequired<GISelValueTrackingAnalysisLegacy>();
  AU.addPreserved<GISelValueTrackingAnalysisLegacy>();
  AU.addRequired<MachineDominatorTreeWrapperPass>();
  AU.addPreserved<MachineDominatorTreeWrapperPass>();
  AU.addRequired<GISelCSEAnalysisWrapperPass>();
  AU.addPreserved<GISelCSEAnalysisWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

Z80PreLegalizerCombiner::Z80PreLegalizerCombiner() : MachineFunctionPass(ID) {
  if (!RuleConfig.parseCommandLineOption())
    report_fatal_error("Invalid rule identifier");
}

bool Z80PreLegalizerCombiner::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getProperties().hasFailedISel())
    return false;

  GISelCSEAnalysisWrapper &Wrapper =
      getAnalysis<GISelCSEAnalysisWrapperPass>().getCSEWrapper();
  auto *CSEInfo =
      &Wrapper.get(getStandardCSEConfigForOpt(MF.getTarget().getOptLevel()));

  const Z80Subtarget &ST = MF.getSubtarget<Z80Subtarget>();
  const auto *LI = ST.getLegalizerInfo();

  const Function &F = MF.getFunction();
  bool EnableOpt =
      MF.getTarget().getOptLevel() != CodeGenOptLevel::None && !skipFunction(F);
  GISelValueTracking *VT =
      &getAnalysis<GISelValueTrackingAnalysisLegacy>().get(MF);
  MachineDominatorTree *MDT =
      &getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();
  CombinerInfo CInfo(/*AllowIllegalOps*/ true, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, EnableOpt, F.hasOptSize(),
                     F.hasMinSize());
  CInfo.MaxIterations = 1;
  CInfo.ObserverLvl = CombinerInfo::ObserverLevel::SinglePass;
  CInfo.EnableFullDCE = true;
  Z80PreLegalizerCombinerImpl Impl(MF, CInfo, *VT, CSEInfo, RuleConfig, ST, MDT,
                                   LI);
  return Impl.combineMachineInstrs();
}

char Z80PreLegalizerCombiner::ID = 0;
INITIALIZE_PASS_BEGIN(Z80PreLegalizerCombiner, DEBUG_TYPE,
                      "Combine Z80 machine instrs before legalization", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(GISelValueTrackingAnalysisLegacy)
INITIALIZE_PASS_DEPENDENCY(GISelCSEAnalysisWrapperPass)
INITIALIZE_PASS_END(Z80PreLegalizerCombiner, DEBUG_TYPE,
                    "Combine Z80 machine instrs before legalization", false,
                    false)

FunctionPass *llvm::createZ80PreLegalizerCombiner() {
  return new Z80PreLegalizerCombiner();
}
