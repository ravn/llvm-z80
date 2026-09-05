//===-- Z80TargetMachine.cpp - Define TargetMachine for Z80 ---------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the Z80 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#include "Z80TargetMachine.h"
#include "Z80KeepLoopPointerInPair.h"
#include "Z80NarrowNoIndex.h"
#include "Z80PinAluAccumulator.h"
#include "Z80PinLoopPointer.h"
#include "Z80HighByteFirstBranch.h"
#include "Z80RemoveJumpToNext.h"
#include "Z80PruneCallFrameDefs.h"
#include "Z80ReorderTestDec.h"
#include "Z80SplitDjnzCounters.h"

#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/Localizer.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"
#include "llvm/CodeGen/MachineBlockFrequencyInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/IndVarSimplify.h"
#include "llvm/Transforms/Utils.h"

#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80BranchCleanup.h"
#include "Z80Combiner.h"
#include "Z80ExpandPseudo.h"
#include "Z80FixupImplicitDefs.h"
#include "Z80FuseCarryChain.h"
#include "Z80IndexIV.h"
#include "Z80AutoStaticStack.h"
#include "Z80PatternFillRecognize.h"
#include "Z80LoopRotate.h"
#include "Z80LoopInstrFormPrep.h"
#include "Z80SinkColdLoopIV.h"
#include "Z80LateOptimization.h"
#include "Z80LowerSelect.h"
#include "Z80MachineFunctionInfo.h"
#include "Z80PostRACompareMerge.h"
#include "Z80PostRAScavenging.h"
#include "Z80ShiftRotateChain.h"
#include "Z80TargetObjectFile.h"
#include "Z80TargetTransformInfo.h"

using namespace llvm;

// ravn/llvm-z80 #156428: the Z80FixupImplicitDefs mitigation removes spurious
// super-register implicit-defs that LiveVariables added.  The LiveVariables
// root-cause fix (LiveVariables::HandlePhysRegUse, only claim a super-register
// defined when every leaf sub-register is defined) prevents those defs being
// added at all, so the mitigation's removal loop is subsumed.  CONFIRMED: with
// this flag set, the full differential oracle stays clean in both configs
// (default 799/0/50/207, +static-stack 793/0/50/213, zero divergences), so the
// MCP miscompile the mitigation guards no longer occurs.  The pass is kept on
// by default because its fullyRecomputeLiveIns tail still clears 2 verifier
// errors (test_11, test_37); this hidden flag lets a future focused session
// remove the now-redundant removal loop with proof in hand.
static cl::opt<bool> DisableFixupImplicitDefs(
    "z80-disable-fixup-implicit-defs", cl::Hidden, cl::init(false),
    cl::desc("Disable the Z80FixupImplicitDefs #156428 mitigation pass"));

// ravn/llvm-z80#23 investigation (2026-06-08): the global disable of
// MachineLICM + MachineCSE in Z80PassConfig is BOTH a size workaround
// (#128 / #198) AND a correctness guard at -O2 (MachineCSE alone
// miscompiles AES, isolated, verifier FAIL).  These two opt-in flags
// let an investigation harness (aes256-corpus/task3_licm_ab.sh and the
// new compiler-comparison-corpus benches) lift each disable
// independently to MEASURE the workaround's cost.  Both default OFF.
// CSE-enable refuses to apply at -O2 (the #198 miscompile would corrupt
// the measurement); LICM-enable applies at all opt levels.
// ravn/llvm-z80#23 resolution (2026-06-08, REVISED 2026-06-08 same-day):
// historical `disablePass(LICM + EarlyLICM + CSE)` is now PARTIALLY removed
// -- LICM enabled by default, CSE STILL DISABLED by default after a same-day
// regression sweep on compiler-comparison-corpus surfaced a MachineCSE
// miscompile on bench_pi.c at -Oz (#198 class -- "no longer reproduces" was
// AES-specific, the pass still miscompiles other code).  Three-state
// re-measurement on AES @ -Oz (aes256-corpus/probe_cse.sh):
//   LICM+CSE on (the AES-only resolution):  aes_text=2156 bin=2516 ts=16,577,307  pi FAIL
//   LICM only / CSE off (THIS DEFAULT):     aes_text=2238 bin=2595 ts=16,571,818  pi PASS
//   both off (pre-#23):                     aes_text=2226 bin=2581 ts=18,214,790  pi PASS
// Key insight: the AES -8.9% tstates win comes from LICM, NOT CSE; the
// LICM-only cell is FASTER than LICM+CSE (16.572M vs 16.577M ts) and 9% faster
// than pre-#23.  CSE only contributed size (+79 B aes_text without it).
// We keep the speedup and avoid the pi miscompile by leaving CSE disabled.
// LICM ships as default ON (the actual speedup driver).
// pi miscompile reproducer: bench_pi.c @ -Oz, CSE on -> 880 B / 58.87M ts /
//   verify FAIL; CSE off -> 887 B / 58.83M ts / verify PASS.  Not yet root-
//   caused or filed (per HARD rule explain-before-filing; needs minimisation).
static cl::opt<bool> EnableMachineLICM(
    "z80-enable-licm", cl::Hidden, cl::init(true),
    cl::desc("Z80: enable MachineLICM + EarlyMachineLICM (default TRUE; "
             "set false to restore the pre-2026-06-08 disablePass workaround)"));
static cl::opt<bool> EnableMachineCSE(
    "z80-enable-cse", cl::Hidden, cl::init(false),
    cl::desc("Z80: enable MachineCSE (default FALSE -- MachineCSE miscompiles "
             "bench_pi.c at -Oz, #198 class still active despite AES no longer "
             "tripping it; opt-in for measurement/probes only)"));

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeZ80Target() {
  // Register both Z80 and SM83 targets.
  RegisterTargetMachine<Z80TargetMachine> X(getTheZ80Target());
  RegisterTargetMachine<Z80TargetMachine> Y(getTheSM83Target());

  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeGlobalISel(PR);
  initializeZ80BranchCleanupPass(PR);
  initializeZ80PreLegalizerCombinerPass(PR);
  initializeZ80PostLegalizerCombinerPass(PR);
  initializeZ80FixupImplicitDefsPass(PR);
  initializeZ80FuseCarryChainPass(PR);
  initializeZ80LateOptimizationPass(PR);
  initializeZ80PatternFillRecognizeLegacyPassPass(PR);
  initializeZ80LoopRotateLegacyPassPass(PR);
  initializeZ80AutoStaticStackPass(PR);
  initializeZ80LowerSelectPass(PR);
  initializeZ80PostRAScavengingPass(PR);
  initializeZ80PruneCallFrameDefsPass(PR);
  initializeZ80ReorderTestDecPass(PR);
  initializeZ80ShiftRotateChainPass(PR);
  initializeZ80SplitDjnzCountersPass(PR);
  initializeZ80PinAluAccumulatorPass(PR);
  initializeZ80PinLoopPointerPass(PR);
  initializeZ80HighByteFirstBranchPass(PR);
  initializeZ80RemoveJumpToNextPass(PR);
  initializeZ80KeepLoopPointerInPairPass(PR);
  initializeZ80SinkColdLoopIVLegacyPassPass(PR);
  initializeZ80NarrowNoIndexPass(PR);
  initializeZ80PostRACompareMergePass(PR);
}

// Z80 data layout:
// e = little endian
// p:16:8 = 16-bit pointers with 8-bit alignment
// i16:8 = 16-bit integers with 8-bit alignment
// i32:8 = 32-bit integers with 8-bit alignment
// f32:8 = 32-bit floats with 8-bit alignment
// f64:8 = 64-bit floats with 8-bit alignment
// n8:16 = native integer widths are 8 and 16 bits
static const char *Z80DataLayout =
    "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-ve-n8:16";

/// Processes a CPU name.
static StringRef getCPU(StringRef CPU, const Triple &TT) {
  if (CPU.empty() || CPU == "generic")
    return TT.getArch() == Triple::sm83 ? "sm83" : "z80";
  return CPU;
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM ? *RM : Reloc::Static;
}

Z80TargetMachine::Z80TargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   std::optional<Reloc::Model> RM,
                                   std::optional<CodeModel::Model> CM,
                                   CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, Z80DataLayout, TT, getCPU(CPU, TT), FS,
                               Options, getEffectiveRelocModel(RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      SubTarget(TT, getCPU(CPU, TT).str(), FS.str(), *this) {
  this->TLOF = std::make_unique<Z80TargetObjectFile>();

  initAsmInfo();

  setGlobalISel(true);
  // Prevents fallback to SelectionDAG by allowing direct aborts.
  setGlobalISelAbort(GlobalISelAbortMode::Enable);
}

const Z80Subtarget *
Z80TargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  auto CPU = getCPU(CPUAttr.isValid() ? CPUAttr.getValueAsString()
                                      : StringRef(TargetCPU),
                    TargetTriple)
                 .str();
  auto FS = FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;

  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    I = std::make_unique<Z80Subtarget>(TargetTriple, CPU, FS, *this);
  }
  return I.get();
}

TargetTransformInfo
Z80TargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<Z80TTIImpl>(this, F));
}

void Z80TargetMachine::registerPassBuilderCallbacks(PassBuilder &PB) {
  PB.registerPipelineParsingCallback(
      [](StringRef Name, LoopPassManager &PM,
         ArrayRef<PassBuilder::PipelineElement>) {
        if (Name == "z80-indexiv") {
          // Rewrite pointer artithmetic in loops to use 8-bit IV offsets.
          PM.addPass(Z80IndexIV());
          return true;
        }
        return false;
      });
  PB.registerPipelineParsingCallback(
      [](StringRef Name, FunctionPassManager &PM,
         ArrayRef<PassBuilder::PipelineElement>) {
        if (Name == "z80-pattern-fill-recognize") {
          PM.addPass(Z80PatternFillRecognize());
          return true;
        }
        if (Name == "z80-loop-rotate") {
          PM.addPass(Z80LoopRotate());
          return true;
        }
        return false;
      });

  PB.registerLateLoopOptimizationsEPCallback(
      [](LoopPassManager &PM, OptimizationLevel Level) {
        if (Level != OptimizationLevel::O0) {
          PM.addPass(Z80IndexIV());
        }
      });
  // Pattern-fill rewrite is a Function-level pass (it walks loops itself
  // so the legacy-PM wrapper can share the body).
  PB.registerVectorizerStartEPCallback(
      [](FunctionPassManager &PM, OptimizationLevel Level) {
        if (Level != OptimizationLevel::O0) {
          PM.addPass(Z80PatternFillRecognize());
          // Re-rotate head-test loops that LLVM's LoopRotate skipped at
          // -Oz due to the minsize gate (issue #77a).
          PM.addPass(Z80LoopRotate());
        }
      });
}

MachineFunctionInfo *Z80TargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {

  return Z80FunctionInfo::create<Z80FunctionInfo>(
      Allocator, F, static_cast<const Z80Subtarget *>(STI));
}

//===----------------------------------------------------------------------===//
// Pass Pipeline Configuration
//===----------------------------------------------------------------------===//

namespace {
/// Z80 Code Generator Pass Configuration Options.
class Z80PassConfig : public TargetPassConfig {
public:
  Z80PassConfig(Z80TargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {
    // ravn/llvm-z80#128/#198/#23 -- HISTORICAL CONTEXT:
    //
    // For ~2 months the Z80 backend shipped `disablePass(LICM + EarlyLICM
    // + CSE)` unconditionally, on two grounds:
    //   (a) size pessimization on cpnos/AES at -Oz (#128, #177 Task 3)
    //   (b) correctness guard: MachineCSE miscompiled AES at -O2 (#198)
    //
    // 2026-06-08 re-measurement on clean rebuild invalidated both
    // grounds for current HEAD:
    //   - AES at -Oz: LICM+CSE on saves 13 B AND 8.9% tstates (the
    //     "+34 B" historical number no longer reproduces -- either
    //     backend movement since 2026-05 or stale-rebuild measurement)
    //   - AES at -O2: LICM+CSE on saves 118 B AND 9.2% tstates, PASS
    //     (the #198 miscompile no longer reproduces -- verifier clean,
    //     value oracle matches, test-runner 854 PASS 0 FAIL across all
    //     opt levels with LICM+CSE forced on)
    //   - autoload-in-c: +64 B raw .text / +25 B compressed (temporary
    //     size regression; user direction 2026-06-08 "don't let short-
    //     term size block structural fixes -- bytes will come back via
    //     follow-up cost-model work").
    //   - cpnos PROM1: -11 B (improves).
    //   - rcbios BIOS: +7 B (negligible).
    //
    // Decision (2026-06-08, user-directed): default LICM+CSE ON.  The
    // `EnableMachineLICM` / `EnableMachineCSE` cl::opt flags above
    // default TRUE; set them FALSE to restore the historical workaround
    // for diagnosis without rebuilding clang.
    //
    // Follow-up: `Z80InstrInfo::shouldHoist` (gated by
    // `-z80-licm-block-on-call`) is an opt-in coarse heuristic that
    // refuses to hoist out of loops whose body contains a CALL.  It
    // preserves the AES win and eliminates the autoload regression,
    // but currently un-does cpnos's improvement -- left default OFF
    // pending a count-based refinement that respects already-hoisted
    // live-across-call invariant count.  See known-suboptimal-codegen.md
    // M5 for the full picture.
    if (!EnableMachineLICM) {
      disablePass(&EarlyMachineLICMID);
      disablePass(&MachineLICMID);
    }
    if (!EnableMachineCSE) {
      disablePass(&MachineCSELegacyID);
    }
  }

  Z80TargetMachine &getZ80TargetMachine() const {
    return getTM<Z80TargetMachine>();
  }

  void addIRPasses() override;
  bool addPreISel() override;
  bool addIRTranslator() override;
  void addPreLegalizeMachineIR() override;
  bool addLegalizeMachineIR() override;
  void addPreRegBankSelect() override;
  bool addRegBankSelect() override;
  void addPreGlobalInstructionSelect() override;
  bool addGlobalInstructionSelect() override;

  // Register pressure is too high to work without optimized register
  // allocation.
  void addFastRegAlloc() override { addOptimizedRegAlloc(); }
  void addOptimizedRegAlloc() override;
  void addPreRegAlloc() override;

  void addPostRewrite() override;
  void addPreSched2() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *Z80TargetMachine::createPassConfig(PassManagerBase &PM) {
  // Disable the machine outliner.  On Z80, CALL is 3 bytes + RET is 1 byte
  // = 4 bytes overhead per outlined sequence.  Most Z80 instructions are
  // 1-3 bytes, so outlining short sequences (like port I/O) makes code
  // LARGER, not smaller.
  this->Options.EnableMachineOutliner = false;
  return new Z80PassConfig(*this, PM);
}

namespace {
/// Report IR constructs this backend does not support as proper errors.
/// asm goto has no GlobalISel lowering (the IR translator refuses inline-asm
/// callbr) and would otherwise surface as an internal backend error; the
/// callbr is replaced with its fallthrough edge so compilation reaches the
/// diagnostic cleanly.
// True when one of the constraint's alternative codes lets the operand live
// in a register.
static bool hasRegisterAlternative(const InlineAsm::ConstraintInfo &C) {
  for (const std::string &Code : C.Codes)
    if (Code == "r" || Code == "R" || Code == "X" || Code[0] == '{' ||
        (Code.size() == 1 && StringRef("abcdehl").contains(Code[0])))
      return true;
  return false;
}

static bool hasMemoryAlternative(const InlineAsm::ConstraintInfo &C) {
  for (const std::string &Code : C.Codes)
    if (Code == "m" || Code == "o" || Code == "V")
      return true;
  return false;
}

static bool isImmediateOnly(const InlineAsm::ConstraintInfo &C) {
  for (const std::string &Code : C.Codes)
    if (Code.size() != 1 || !StringRef("insEF").contains(Code[0]))
      return false;
  return !C.Codes.empty();
}

// GlobalISel's inline asm lowering does not implement register outputs that
// are stored through a pointer, which is what clang emits for "+g"- and
// "=X"-style constraints. When such an output may live in a register,
// rewrite it into a plain register output followed by an explicit store, so
// the asm call only carries operand shapes the lowering implements.
static bool rewriteIndirectAsmOutputs(Function &F) {
  SmallVector<CallInst *, 4> Worklist;
  for (BasicBlock &BB : F)
    for (Instruction &I : BB)
      if (auto *CI = dyn_cast<CallInst>(&I))
        if (CI->isInlineAsm())
          Worklist.push_back(CI);

  bool Changed = false;
  for (CallInst *CI : Worklist) {
    auto *IA = cast<InlineAsm>(CI->getCalledOperand());
    InlineAsm::ConstraintInfoVector CV = IA->ParseConstraints();

    SmallVector<bool, 8> Rewrite(CV.size(), false);
    bool Any = false;
    unsigned ArgIdx = 0;
    for (unsigned I = 0; I != CV.size(); ++I) {
      const InlineAsm::ConstraintInfo &C = CV[I];
      bool ConsumesArg = C.Type == InlineAsm::isInput ||
                         (C.Type == InlineAsm::isOutput && C.isIndirect);
      // An aggregate cannot become a direct asm result; leave it indirect
      // (a register-only aggregate is then diagnosed as unsupported).
      if (C.Type == InlineAsm::isOutput && C.isIndirect &&
          hasRegisterAlternative(C) &&
          !CI->getParamElementType(ArgIdx)->isAggregateType()) {
        Rewrite[I] = true;
        Any = true;
      }
      if (ConsumesArg)
        ++ArgIdx;
    }
    if (!Any)
      continue;

    // Constraint string segments map 1:1 to the parsed constraints.
    SmallVector<StringRef, 8> Segments;
    StringRef ConstraintStr = IA->getConstraintString();
    ConstraintStr.split(Segments, ',');
    if (Segments.size() != CV.size())
      continue;

    Type *OldRet = CI->getType();
    auto OldRetElt = [&](unsigned Idx) -> Type * {
      if (auto *ST = dyn_cast<StructType>(OldRet))
        return ST->getElementType(Idx);
      return OldRet;
    };

    unsigned OldRetIdx = 0, ArgNo = 0;
    SmallVector<Type *, 4> NewRetTypes;
    SmallVector<Value *, 8> NewArgs;
    SmallVector<AttributeSet, 8> NewArgAttrs;
    std::string NewConstraints;
    // Pointer to store through and the result index that feeds it.
    SmallVector<std::pair<Value *, unsigned>, 4> Stores;
    // New result index of each old direct output, in output order.
    SmallVector<unsigned, 4> OldToNewRet;

    for (unsigned I = 0; I != CV.size(); ++I) {
      const InlineAsm::ConstraintInfo &C = CV[I];
      if (!NewConstraints.empty())
        NewConstraints += ',';
      if (Rewrite[I]) {
        Stores.push_back({CI->getArgOperand(ArgNo), NewRetTypes.size()});
        NewRetTypes.push_back(CI->getParamElementType(ArgNo));
        ++ArgNo;
        NewConstraints += C.isEarlyClobber ? "=&r" : "=r";
        continue;
      }
      NewConstraints += Segments[I];
      if (C.Type == InlineAsm::isOutput && !C.isIndirect) {
        OldToNewRet.push_back(NewRetTypes.size());
        NewRetTypes.push_back(OldRetElt(OldRetIdx++));
      }
      if (C.Type == InlineAsm::isInput ||
          (C.Type == InlineAsm::isOutput && C.isIndirect)) {
        NewArgs.push_back(CI->getArgOperand(ArgNo));
        NewArgAttrs.push_back(CI->getAttributes().getParamAttrs(ArgNo));
        ++ArgNo;
      }
    }

    LLVMContext &Ctx = F.getContext();
    Type *NewRet = NewRetTypes.empty()      ? Type::getVoidTy(Ctx)
                   : NewRetTypes.size() == 1 ? NewRetTypes[0]
                                             : StructType::get(Ctx, NewRetTypes);
    SmallVector<Type *, 8> ParamTys;
    for (Value *V : NewArgs)
      ParamTys.push_back(V->getType());
    FunctionType *NewFTy = FunctionType::get(NewRet, ParamTys, false);
    InlineAsm *NewIA =
        InlineAsm::get(NewFTy, IA->getAsmString(), NewConstraints,
                       IA->hasSideEffects(), IA->isAlignStack(),
                       IA->getDialect(), IA->canThrow());
    CallInst *NewCall =
        CallInst::Create(NewFTy, NewIA, NewArgs, "", CI->getIterator());
    NewCall->copyMetadata(*CI);
    NewCall->setAttributes(AttributeList::get(
        Ctx, CI->getAttributes().getFnAttrs(), AttributeSet(), NewArgAttrs));

    auto ExtractRet = [&](unsigned Idx) -> Value * {
      if (NewRetTypes.size() == 1)
        return NewCall;
      return ExtractValueInst::Create(NewCall, {Idx}, "", CI->getIterator());
    };

    for (const auto &[Ptr, Idx] : Stores)
      new StoreInst(ExtractRet(Idx), Ptr, CI->getIterator());

    if (!OldRet->isVoidTy()) {
      Value *Repl;
      if (auto *ST = dyn_cast<StructType>(OldRet)) {
        Repl = PoisonValue::get(ST);
        for (unsigned I = 0; I != OldToNewRet.size(); ++I)
          Repl = InsertValueInst::Create(Repl, ExtractRet(OldToNewRet[I]), {I},
                                         "", CI->getIterator());
      } else {
        Repl = ExtractRet(OldToNewRet[0]);
      }
      CI->replaceAllUsesWith(Repl);
    }
    CI->eraseFromParent();
    Changed = true;
  }
  return Changed;
}

// A value wider than a 16-bit register pair cannot be placed in registers,
// and the lowering also has no way to split a wide direct output. Wide
// operands are only viable through memory.
static bool hasWideDirectOperand(const CallBase &CB, const DataLayout &DL) {
  const auto *IA = cast<InlineAsm>(CB.getCalledOperand());
  Type *Ret = CB.getType();
  unsigned RetIdx = 0, ArgNo = 0;
  for (const InlineAsm::ConstraintInfo &C : IA->ParseConstraints()) {
    if (C.Type == InlineAsm::isClobber || C.Type == InlineAsm::isLabel)
      continue;
    if (C.Type == InlineAsm::isOutput) {
      if (C.isIndirect) {
        // Register-only indirect outputs survive the rewrite only when the
        // pointee is an aggregate, which no register sequence can carry.
        if (hasRegisterAlternative(C) && !hasMemoryAlternative(C))
          return true;
        ++ArgNo;
        continue;
      }
      Type *Ty = isa<StructType>(Ret)
                     ? cast<StructType>(Ret)->getElementType(RetIdx)
                     : Ret;
      ++RetIdx;
      if (DL.getTypeSizeInBits(Ty) > 16)
        return true;
      continue;
    }
    Value *Op = CB.getArgOperand(ArgNo++);
    if (C.isIndirect) {
      // Same for indirect inputs: only memory can carry them.
      if (hasRegisterAlternative(C) && !hasMemoryAlternative(C))
        return true;
      continue;
    }
    if (DL.getTypeSizeInBits(Op->getType()) <= 16)
      continue;
    // A tied input mirrors its output, which was already checked.
    if (!C.Codes.empty() && isDigit(C.Codes[0][0]))
      continue;
    // The lowering spills these to a stack slot itself.
    if (hasMemoryAlternative(C))
      continue;
    if (isImmediateOnly(C) && isa<Constant>(Op))
      continue;
    return true;
  }
  return false;
}

// The legalizer can erase the last definition of a wide value (a scalarized
// vector, a split integer) while a debug operand still refers to it.
// RegBankSelect maps debug operands like any other, and no 8/16-bit bank can
// carry the wide dangling type. The value no longer exists, so mark it as
// unavailable instead.
class Z80DanglingDebugCleanup : public MachineFunctionPass {
public:
  static char ID;
  Z80DanglingDebugCleanup() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override {
    return "Z80 dangling debug value cleanup";
  }
  bool runOnMachineFunction(MachineFunction &MF) override {
    MachineRegisterInfo &MRI = MF.getRegInfo();
    bool Changed = false;
    auto IsDangling = [&](const MachineOperand &MO) {
      return MO.isReg() && MO.getReg().isVirtual() &&
             !MRI.getVRegDef(MO.getReg());
    };
    for (MachineBasicBlock &MBB : MF)
      for (MachineInstr &MI : MBB) {
        if (!MI.isDebugInstr() || none_of(MI.debug_operands(), IsDangling))
          continue;
        // A location with an unavailable operand cannot be evaluated at all,
        // so the canonical form marks the whole value undef.
        if (MI.isDebugValue())
          MI.setDebugValueUndef();
        else
          for (MachineOperand &MO : MI.debug_operands())
            if (IsDangling(MO))
              MO.setReg(Register());
        Changed = true;
      }
    return Changed;
  }
};
char Z80DanglingDebugCleanup::ID = 0;

class Z80CheckUnsupported : public FunctionPass {
public:
  static char ID;
  Z80CheckUnsupported() : FunctionPass(ID) {}
  StringRef getPassName() const override {
    return "Z80 unsupported construct check";
  }
  bool runOnFunction(Function &F) override {
    // Atomic read-modify-write has no honest implementation here: nothing
    // stops an interrupt handler between the load and the store, and the
    // interrupt state cannot be reliably saved and restored to close that
    // window (IFF1 is unreadable without the NMI erratum, SM83's IME is
    // unreadable entirely). Refuse rather than pretend.
    SmallVector<Instruction *, 2> Atomics;
    for (BasicBlock &BB : F)
      for (Instruction &I : BB)
        if (isa<AtomicRMWInst>(I) || isa<AtomicCmpXchgInst>(I))
          Atomics.push_back(&I);
    for (Instruction *I : Atomics) {
      F.getContext().diagnose(DiagnosticInfoUnsupported(
          F, "atomic read-modify-write operations are not supported",
          I->getDebugLoc()));
      if (!I->getType()->isVoidTy())
        I->replaceAllUsesWith(PoisonValue::get(I->getType()));
      I->eraseFromParent();
    }

    SmallVector<CallBrInst *, 2> AsmGotos;
    for (BasicBlock &BB : F)
      if (auto *CBR = dyn_cast<CallBrInst>(BB.getTerminator()))
        if (CBR->isInlineAsm())
          AsmGotos.push_back(CBR);

    for (CallBrInst *CBR : AsmGotos) {
      F.getContext().diagnose(DiagnosticInfoUnsupported(
          F, "asm goto is not supported", CBR->getDebugLoc()));

      BasicBlock *Parent = CBR->getParent();
      BasicBlock *DefaultDest = CBR->getDefaultDest();
      // Every entry in the indirect list is its own edge with its own PHI
      // entry, even when it repeats a block or the default destination.
      // The replacing branch keeps exactly one edge (the default), so drop
      // one PHI entry per indirect entry.
      for (BasicBlock *Ind : CBR->getIndirectDests())
        Ind->removePredecessor(Parent);
      if (!CBR->getType()->isVoidTy())
        CBR->replaceAllUsesWith(PoisonValue::get(CBR->getType()));
      UncondBrInst::Create(DefaultDest, CBR->getIterator());
      CBR->eraseFromParent();
    }

    bool Changed = !Atomics.empty() || !AsmGotos.empty();
    Changed |= rewriteIndirectAsmOutputs(F);

    const DataLayout &DL = F.getParent()->getDataLayout();
    SmallVector<CallInst *, 2> WideAsm;
    for (BasicBlock &BB : F)
      for (Instruction &I : BB)
        if (auto *CI = dyn_cast<CallInst>(&I))
          if (CI->isInlineAsm() && hasWideDirectOperand(*CI, DL))
            WideAsm.push_back(CI);
    for (CallInst *CI : WideAsm) {
      F.getContext().diagnose(DiagnosticInfoUnsupported(
          F, "unsupported inline asm operand: value wider than 16 bits",
          CI->getDebugLoc()));
      if (!CI->getType()->isVoidTy())
        CI->replaceAllUsesWith(PoisonValue::get(CI->getType()));
      CI->eraseFromParent();
      Changed = true;
    }
    return Changed;
  }
};
char Z80CheckUnsupported::ID = 0;
} // namespace

void Z80PassConfig::addIRPasses() {
  addPass(new Z80CheckUnsupported());
  addPass(createAtomicExpandLegacyPass());

  // ravn/llvm-z80#176/#40: auto-inject +static-stack on provably-non-recursive
  // functions (default on; global opt-out via -mllvm -z80-auto-static-stack=
  // false).  Gate the *registration* on the flag rather than early-returning
  // inside the pass when disabled: a registered no-op pass still shifts
  // downstream behavior (#187 pipeline-ordering side effect, ~2 B on cpnos
  // PROM1), so the opt-out path must omit the pass entirely.
  if (isZ80AutoStaticStackEnabled())
    addPass(createZ80AutoStaticStackPass());

  TargetPassConfig::addIRPasses();
  if (getOptLevel() != CodeGenOptLevel::None) {
    addPass(createInstructionCombiningPass());
    // Pattern-fill rewrite (issue #88).  Runs from llc's IR pipeline
    // here; clang's pipeline picks it up via PassBuilder hook.
    addPass(createZ80PatternFillRecognizeLegacyPass());
    // Re-rotate head-test loops that LLVM's LoopRotate skipped at -Oz
    // due to the minsize gate (issue #77a).
    addPass(createZ80LoopRotateLegacyPass());
    // Sink enclosing-loop IVs that only seed a nested loop's cold path back
    // into an on-demand recompute (ravn/llvm-z80#250, sieve scan loop).  Must
    // run after the base addIRPasses() (LSR) so it can undo LSR's hoist.
    // Experimental / opt-in.
    // Auto-on at -O2 only (ravn/llvm-z80#250); the pass skips -Os/-Oz functions
    // internally, and an explicit -mllvm flag overrides.
    if (isZ80SinkColdLoopIVEnabled(getOptLevel()))
      addPass(createZ80SinkColdLoopIVLegacyPass());
    // Pointer-IV strength reduction for scale-1 byte-array loops
    // (ravn/llvm-z80#250).  MUST run after the base addIRPasses() call
    // above (which runs LSR) -- see Z80LoopInstrFormPrep.cpp's file
    // comment for why an earlier placement would be re-undone by LSR.
    // Auto-on at -O2 only (opt-size functions skipped inside the pass).
    if (isZ80LoopInstrFormPrepEnabled(getOptLevel()))
      addPass(createZ80LoopInstrFormPrepLegacyPass());
  }
}

bool Z80PassConfig::addPreISel() { return false; }

bool Z80PassConfig::addIRTranslator() {
  addPass(new IRTranslatorLegacy(getOptLevel()));
  return false;
}

void Z80PassConfig::addPreLegalizeMachineIR() {
  if (getOptLevel() != CodeGenOptLevel::None) {
    addPass(createZ80PreLegalizerCombiner());
    addPass(createZ80ShiftRotateChainPass());
  }
}

bool Z80PassConfig::addLegalizeMachineIR() {
  addPass(new LegalizerLegacy());
  return false;
}

void Z80PassConfig::addPreRegBankSelect() {
  // Post-legalization combiner must run at all optimization levels.
  // It contains correctness rules (z80_cross_size_copy, merge_combines)
  // that are required for instruction selection to succeed.
  addPass(createZ80PostLegalizerCombiner());
  addPass(createZ80LowerSelectPass());
  addPass(new Z80DanglingDebugCleanup());
}

bool Z80PassConfig::addRegBankSelect() {
  addPass(new RegBankSelectLegacy());
  return false;
}

void Z80PassConfig::addPreGlobalInstructionSelect() {
  // This pass helps reduce the live ranges of constants to within a basic
  // block, which can greatly improve machine scheduling, as they can now be
  // moved around to keep register pressure low.
  addPass(new LocalizerLegacy());
}

bool Z80PassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelectLegacy());
  return false;
}

void Z80PassConfig::addPreRegAlloc() {
  // Prune each ADJCALLSTACKUP's worst-case implicit-def $hl/$a down to what its
  // own AdjAmount actually clobbers.  Must run BEFORE the regalloc sequence's
  // first liveness computation (LiveVariables/LiveIntervals), otherwise the
  // conservative defs cause a float call's DE:HL result $hl to be marked dead
  // and the consuming COPY reads undef once the (AdjAmount==0) ADJCALLSTACKUP
  // is erased -- the dominant -verify-machineinstrs failure (ravn/llvm-z80#197).
  addPass(createZ80PruneCallFrameDefsPass());
}

void Z80PassConfig::addOptimizedRegAlloc() {
  if (getOptLevel() != CodeGenOptLevel::None) {
    // Run the coalescer twice to coalesce RMW patterns revealed by the first
    // coalesce.
    insertPass(&llvm::TwoAddressInstructionPassID, &llvm::RegisterCoalescerID);

    // Z80ReorderTestDec runs BEFORE register allocation but AFTER
    // instruction selection.  It rewrites the post-ISel "DEC_A;
    // RELOAD; OR_A; JR_Z" pattern to "SUB_n 1; JR_C" -- closes the
    // dominant gf_log/gf_alog inner-loop redundant-reload pattern
    // identified in ravn/llvm-z80#179 + #174.  Same lifecycle as
    // Z80SplitDjnzCounters below (insert after MachineScheduler,
    // before the LiveIntervals re-run).
    insertPass(&llvm::MachineSchedulerID,
               createZ80ReorderTestDecPass());

    // Z80SplitDjnzCounters must run BEFORE the LiveIntervals re-run
    // (inserted just below) so the per-loop counter COPYs the pass
    // creates are present when LiveIntervals computes its data for
    // greedy.  insertPass appends "after MachineScheduler" in
    // insertion order, so this call must precede the LiveIntervals
    // insertPass.  See tasks/regalloc-sequential-djnz-investigation.md
    // (#94 / #98).
    insertPass(&llvm::MachineSchedulerID,
               createZ80SplitDjnzCountersPass());

    // Pin 8-bit ALU accumulator vregs to A by class.  Same lifecycle
    // requirements as Z80SplitDjnzCounters above -- must run BEFORE
    // the LiveIntervals re-run so the per-loop COPYs the pass creates
    // are present when LiveIntervals recomputes for greedy.  See
    // ravn/llvm-z80#172.
    insertPass(&llvm::MachineSchedulerID,
               createZ80PinAluAccumulatorPass());

    // Z80PinLoopPointer: pin a byte-array pointer-walk induction variable to
    // HL (companion to Z80LoopInstrFormPrep, ravn/llvm-z80#250).  Same
    // lifecycle as the pins above -- pre-RA, before the LiveIntervals re-run,
    // so the HLReg class constraint is present when greedy runs.  Gated behind
    // -z80-pin-loop-pointer (default off).
    insertPass(&llvm::MachineSchedulerID,
               createZ80PinLoopPointerPass());

    // Z80KeepLoopPointerInPair: keep the loop-carried pointer of an i16 `*p++`
    // store loop out of IX/IY by constraining it to GR16NoIR (sibling of the
    // pin above for the WORD walk -- ravn/llvm-z80#249 / #251, where pinning to
    // HL is impossible because the 2-byte store walks HL).  Same lifecycle --
    // pre-RA, before the LiveIntervals re-run.  Gated behind
    // -z80-enable-keep-loop-pointer-in-pair (default off).
    insertPass(&llvm::MachineSchedulerID,
               createZ80KeepLoopPointerInPairPass());

    // Keep IX/IY-incompatible GR16 values out of IX/IY (only when IY is
    // allocatable -- gated internally on -z80-unreserve-iy).  Narrows plain
    // GR16 vregs that are byte-decomposed (sub_lo/sub_hi) or used where
    // GR16NoIR is required, so the allocator/spiller cannot park them in IY
    // and emit a push/pop shuttle or an undocumented IYH/IYL op
    // (ravn/llvm-z80#189 / #27 / #112).  Pre-RA, must precede the LiveIntervals
    // re-run.  See tasks/issue189-27-regalloc-cost-model-drill-2026-05-25.md.
    insertPass(&llvm::MachineSchedulerID, createZ80NarrowNoIndexPass());

    // Re-run Live Intervals after coalescing to renumber the contained values.
    // This can allow constant rematerialization after aggressive coalescing.
    insertPass(&llvm::MachineSchedulerID, &llvm::LiveIntervalsID);
  }
  TargetPassConfig::addOptimizedRegAlloc();
}

void Z80PassConfig::addPostRewrite() {
  // Mitigation for https://github.com/llvm/llvm-project/issues/156428
  // Remove spurious super-register implicit-defs added by LiveVariables.
  // Must run before MachineCopyPropagation to prevent incorrect dead-copy
  // elimination.  See Z80FixupImplicitDefs.cpp for full explanation.
  if (!DisableFixupImplicitDefs)
    addPass(createZ80FixupImplicitDefsPass());

  // Keep multi-byte add/sub carry in the flag across limbs instead of
  // round-tripping it through A (SBC A,A; AND 1 / LD A,r; RRCA).  Runs while
  // the carry pseudos are still intact, before ExpandPostRAPseudos.
  addPass(createZ80FuseCarryChain());
}

void Z80PassConfig::addPreSched2() {
  // Lower control flow pseudos.
  addPass(&FinalizeISelID);
  // Lower pseudos produced by control flow pseudos.
  addPass(&ExpandPostRAPseudosID);
  addPass(createZ80PostRAScavengingPass());

  // This is currently mandatory, since it lowers CMPTermZ.
  addPass(createZ80LateOptimizationPass());

  // Remove redundant OR A / AND A when the Z flag is already valid
  // from a preceding ALU instruction.
  addPass(createZ80PostRACompareMerge());
}

void Z80PassConfig::addPreEmitPass() {
  addPass(&BranchRelaxationPassID);
  // Collapse JR_CC+JP trampolines from BranchRelaxation into JP_CC.
  addPass(createZ80BranchCleanupPass());
  // High-byte-first rewrite of hot 16-bit loop exit tests (ravn/llvm-z80#250
  // lever 2).  Runs after branch cleanup (conditional exit is a documented
  // JP_cc) and before ExpandPseudo (CMP16_FLAGS still a single pseudo to
  // match).  Emits only absolute JPs, so post-relaxation placement is safe.
  // Gated behind -z80-enable-hbf-branch (default off; size-negative).
  addPass(createZ80HighByteFirstBranchPass());
  // Expand pseudos that split MBBs (variable shift loops) after branch
  // relaxation. The generated JR/DJNZ branches are always short-range.
  addPass(createZ80ExpandPseudoPass());
  // General late peephole: drop any unconditional branch to the fall-through
  // block (a jump-to-next-address).  Runs LAST so it catches redundant jumps
  // from ANY source (hbf split blocks, pseudo expansion, cleanup) -- BranchFold
  // ran too early to see them.
  addPass(createZ80RemoveJumpToNextPass());
}
