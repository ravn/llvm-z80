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
#include "Z80NarrowNoIndex.h"
#include "Z80PinAluAccumulator.h"
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
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/IndVarSimplify.h"
#include "llvm/Transforms/Utils.h"

#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80BranchCleanup.h"
#include "Z80Combiner.h"
#include "Z80ExpandPseudo.h"
#include "Z80FixupImplicitDefs.h"
#include "Z80IndexIV.h"
#include "Z80AutoStaticStack.h"
#include "Z80LoopIdiomFill.h"
#include "Z80LoopRotate.h"
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
  initializeZ80LateOptimizationPass(PR);
  initializeZ80LoopIdiomFillLegacyPassPass(PR);
  initializeZ80LoopRotateLegacyPassPass(PR);
  initializeZ80AutoStaticStackPass(PR);
  initializeZ80LowerSelectPass(PR);
  initializeZ80PostRAScavengingPass(PR);
  initializeZ80PruneCallFrameDefsPass(PR);
  initializeZ80ReorderTestDecPass(PR);
  initializeZ80ShiftRotateChainPass(PR);
  initializeZ80SplitDjnzCountersPass(PR);
  initializeZ80PinAluAccumulatorPass(PR);
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
    "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16";

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
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
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
        if (Name == "z80-loop-idiom-fill") {
          PM.addPass(Z80LoopIdiomFill());
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
          PM.addPass(Z80LoopIdiomFill());
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
    // ravn/llvm-z80#128: MachineLICM and MachineCSE consistently
    // pessimize Z80 code because the 3-pair register file (DE/HL/BC)
    // cannot hold the loop-invariants they want to hoist or the
    // common subexpressions they want to share.  The hoisted/shared
    // values get BSS-spilled across CALLs and reloaded each use,
    // costing more bytes + tstates than the redundant computes they
    // were meant to eliminate.  Measured on AES corpus at -Oz:
    // disabling these two saves ~280 B per config at <0.3% tstate
    // cost; on cpnos-rom snios_c.o: -141 B / -16% size.
    //
    // Disable globally pending #177 (Z80 TTI) which would let us
    // gate this on per-function optsize/minsize attributes for a
    // proper opt-level-sensitive decision.
    //
    // #177 Task 3 (2026-05-26) measured whether this should be gated by
    // opt-level (keep LICM/CSE at -O2 for speed).  It should NOT.  With
    // the disable lifted and LICM/CSE on, AES-256 measured:
    //   -Oz: +34 B .text, +144 B bin, +0.22% tstates -- pessimizes, PASS.
    //   -O2: MachineCSE MISCOMPILES (verifier FAIL, isolated to CSE; LICM
    //        alone PASSes).  So the disable is also a CORRECTNESS guard,
    //        not merely a size knob.  Tracked: ravn/llvm-z80#198.
    // There is no opt level where enabling these helps Z80, so the disable
    // stays UNCONDITIONAL (no gate).  Data: aes256-corpus/task3_licm_ab.sh.
    disablePass(&EarlyMachineLICMID);
    disablePass(&MachineLICMID);
    disablePass(&MachineCSELegacyID);
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

void Z80PassConfig::addIRPasses() {
  // Z80 is single-threaded: lower all atomic operations to plain
  // non-atomic load/store/rmw sequences at the IR level.
  addPass(createLowerAtomicPass());

  // ravn/llvm-z80#176/#40: auto-inject +static-stack on leaf functions
  // (opt-in via -mllvm -z80-auto-static-stack=true).  Only register
  // the pass in the pipeline when enabled -- otherwise its mere
  // presence as a no-op pass shifts downstream behavior (#187
  // pipeline-ordering side effect), costing ~2 B on cpnos PROM1
  // for users who don't opt in.
  if (isZ80AutoStaticStackEnabled())
    addPass(createZ80AutoStaticStackPass());

  TargetPassConfig::addIRPasses();
  if (getOptLevel() != CodeGenOptLevel::None) {
    addPass(createInstructionCombiningPass());
    // Pattern-fill rewrite (issue #88).  Runs from llc's IR pipeline
    // here; clang's pipeline picks it up via PassBuilder hook.
    addPass(createZ80LoopIdiomFillLegacyPass());
    // Re-rotate head-test loops that LLVM's LoopRotate skipped at -Oz
    // due to the minsize gate (issue #77a).
    addPass(createZ80LoopRotateLegacyPass());
  }
}

bool Z80PassConfig::addPreISel() { return false; }

bool Z80PassConfig::addIRTranslator() {
  addPass(new IRTranslator(getOptLevel()));
  return false;
}

void Z80PassConfig::addPreLegalizeMachineIR() {
  if (getOptLevel() != CodeGenOptLevel::None) {
    addPass(createZ80PreLegalizerCombiner());
    addPass(createZ80ShiftRotateChainPass());
  }
}

bool Z80PassConfig::addLegalizeMachineIR() {
  addPass(new Legalizer());
  return false;
}

void Z80PassConfig::addPreRegBankSelect() {
  // Post-legalization combiner must run at all optimization levels.
  // It contains correctness rules (z80_cross_size_copy, merge_combines)
  // that are required for instruction selection to succeed.
  addPass(createZ80PostLegalizerCombiner());
  addPass(createZ80LowerSelectPass());
}

bool Z80PassConfig::addRegBankSelect() {
  addPass(new RegBankSelect());
  return false;
}

void Z80PassConfig::addPreGlobalInstructionSelect() {
  // This pass helps reduce the live ranges of constants to within a basic
  // block, which can greatly improve machine scheduling, as they can now be
  // moved around to keep register pressure low.
  addPass(new Localizer());
}

bool Z80PassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelect());
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
  // Expand pseudos that split MBBs (variable shift loops) after branch
  // relaxation. The generated JR/DJNZ branches are always short-range.
  addPass(createZ80ExpandPseudoPass());
}
