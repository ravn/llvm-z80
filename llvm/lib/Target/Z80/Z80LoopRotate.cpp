//===-- Z80LoopRotate.cpp - Z80 Target-Specific Loop Rotation -------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// LLVM core's LoopRotate is gated on Function::hasMinSize() at -Oz: when the
// minsize attribute is set, the rotation threshold is forced to 0 so no
// rotation happens regardless of header size.  See LoopRotation.cpp:72.
//
// On Z80 that gate is the wrong call.  Rotating a head-test
// do-while-decrement loop into tail-test form is a strict size win, because
// the rotated form lets the codegen use the Z flag set by the IV's `dec`
// directly for the back-edge branch (1 byte: jr nz) instead of routing the
// counter through A and re-testing with `or a; ret z` (3 bytes plus the
// implicit unconditional back-jump = 5 bytes).  Issue ravn/llvm-z80#77a.
//
// This pass calls LoopRotation() directly on innermost loops, bypassing the
// minsize gate.  IsUtilMode=true tells the rotator to skip its own
// profitability heuristic; we already know rotation is profitable for the
// shapes we care about on Z80.  Threshold is set to LLVM's
// DefaultRotationThreshold (16) so we don't rotate huge loops where the
// duplicated header would itself bloat the code.
//
// The pass is intentionally minimal — it just re-runs LoopRotation with
// Z80-friendly parameters.  It runs in clang's IR pipeline (via PassBuilder
// hook in Z80TargetMachine.cpp) AND in llc's addIRPasses pipeline (via the
// legacy FunctionPass wrapper at the bottom of this file).
//
//===----------------------------------------------------------------------===//

#include "Z80LoopRotate.h"
#include "Z80.h"

#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/SimplifyQuery.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/MemorySSAUpdater.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/LoopRotationUtils.h"

#define DEBUG_TYPE "z80-loop-rotate"

using namespace llvm;

// Off by default.  The original gate was ravn/llvm-z80#97 (BC ping-pong
// in rotated single-BB self-loops); that's now fixed by the post-RA
// peephole in Z80LateOptimization.cpp covering Cases 1 (param→BC),
// 2 (constant in both HL and BC), 3 (constant in BC only), with both
// orderings (LD L,C first or last in body).  But measurement on
// 2026-05-02 still shows rcbios BIOS +33 B and cpnos-rom payload +4 B
// when rotation runs by default — rotated loops with a CALL inside
// force regalloc to BSS-spill the loop counter / pointer across the
// CALL, which more than offsets the head-test `or a` savings #77a is
// after.  Closing #77a productively needs either (a) a peephole that
// rewrites the spill-around-CALL shape, or (b) a regalloc cost-model
// tweak to rematerialize cheap loop carriers across the call.  Until
// then the pass exists for opt-in (-mllvm -z80-loop-rotate=true) and
// gates #77a for testing; the default stays off.
static cl::opt<bool>
    EnableZ80LoopRotate("z80-loop-rotate", cl::init(false), cl::Hidden,
                        cl::desc("Enable Z80 target-specific loop rotation "
                                 "(off by default — see comment for the open "
                                 "rotation-around-CALL spill regression)"));

namespace {

// Threshold for the size of the loop header — copies the value LLVM core
// uses as DefaultRotationThreshold.  Loops with headers larger than this
// won't be rotated (the duplicated header would offset the win).
static constexpr unsigned Z80LoopRotateThreshold = 16;

bool runOnFunctionImpl(Function &F, AssumptionCache &AC, DominatorTree &DT,
                       LoopInfo &LI, ScalarEvolution &SE,
                       const TargetTransformInfo &TTI,
                       MemorySSAUpdater *MSSAU) {
  if (!EnableZ80LoopRotate)
    return false;
  bool Changed = false;
  const DataLayout &DL = F.getDataLayout();
  const SimplifyQuery SQ(DL, &DT, &AC);

  // Walk innermost loops.  Rotate each one if LoopRotation accepts; the
  // utility takes care of all the structural checks (single latch, well-
  // formed PHIs, etc.) and inserts the entry-side guard when needed.
  SmallVector<Loop *, 8> Loops;
  for (Loop *Outer : LI)
    for (Loop *L : depth_first(Outer))
      if (L->isInnermost())
        Loops.push_back(L);

  for (Loop *L : Loops) {
    bool Rotated =
        LoopRotation(L, &LI, &TTI, &AC, &DT, &SE, MSSAU, SQ,
                     /*RotationOnly=*/true, Z80LoopRotateThreshold,
                     /*IsUtilMode=*/true, /*PrepareForLTO=*/false,
                     /*CheckExitCount=*/false);
    if (Rotated) {
      LLVM_DEBUG(dbgs() << "z80-loop-rotate: rotated loop in "
                        << F.getName() << "\n");
      Changed = true;
    }
  }
  return Changed;
}

class Z80LoopRotateLegacyPass : public FunctionPass {
public:
  static char ID;
  Z80LoopRotateLegacyPass() : FunctionPass(ID) {
    initializeZ80LoopRotateLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;
    auto &AC = getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
    auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    auto &TTI = getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
    return runOnFunctionImpl(F, AC, DT, LI, SE, TTI, /*MSSAU=*/nullptr);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
  }

  StringRef getPassName() const override {
    return "Z80 Loop Rotate (legacy)";
  }
};

} // namespace

char Z80LoopRotateLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(Z80LoopRotateLegacyPass, DEBUG_TYPE, "Z80 Loop Rotate",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(Z80LoopRotateLegacyPass, DEBUG_TYPE, "Z80 Loop Rotate",
                    false, false)

FunctionPass *llvm::createZ80LoopRotateLegacyPass() {
  return new Z80LoopRotateLegacyPass();
}

PreservedAnalyses Z80LoopRotate::run(Function &F,
                                     FunctionAnalysisManager &AM) {
  auto &AC = AM.getResult<AssumptionAnalysis>(F);
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);
  auto &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
  auto &TTI = AM.getResult<TargetIRAnalysis>(F);
  if (!runOnFunctionImpl(F, AC, DT, LI, SE, TTI, /*MSSAU=*/nullptr))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
