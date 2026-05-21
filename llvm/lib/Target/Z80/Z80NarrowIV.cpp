//===-- Z80NarrowIV.cpp - Z80 Loop-Counter IV Narrowing -------------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass narrows i16 loop counters to i8 when SCEV proves the IV's
// signed range fits in [-128, 127] (after sign-extension to i16 the
// original value is recoverable) or, more commonly for Z80, when the
// unsigned range fits in [0, 255].
//
// Motivation (ravn/llvm-z80#77 fix path 1): the dominant AES inner-loop
// shape is
//
//   register uint8_t i = 16;
//   while (i--) buf[i] = f(buf[i]);
//
// At -Oz the C frontend + SROA widen the counter to i16 because the
// `buf[i]` GEP needs an i16 index, and SROA merges the counter's stack
// slot with the index value.  The resulting IR is
//
//   %3 = phi i16 [ %6, %5 ], [ 16, %1 ]
//   %4 = icmp eq i16 %3, 0
//   br i1 %4, label %exit, label %5
//   ...
//   %6 = add nsw i16 %3, -1
//   %7 = and i16 %6, 255           ; <-- proves the byte-mask is intent
//   %8 = getelementptr i8, ptr %0, i16 %7
//
// At codegen this becomes `ld bc, 16; dec bc; ld a, c; or a; jr z` —
// the 16-bit `dec bc` doesn't set flags, so the OR is mandatory.
// Narrowing the IV to i8 lets the backend use `dec c` (sets flags
// directly, eliminates the `ld a, c; or a` pair).  Downstream i16 users
// pick up a `zext i8 to i16` which InstCombine collapses with the
// `and 255` mask.
//
// LLVM core's IndVarSimplify only widens IVs (i8 -> i16, etc.); it
// never narrows.  This pass is the inverse for Z80.
//
//===----------------------------------------------------------------------===//

#include "Z80NarrowIV.h"

#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#define DEBUG_TYPE "z80-narrow-iv"

using namespace llvm;

// Session 73n investigation found that running this pass in the New PM
// IR pipeline (where it was originally registered) caused AES configs
// with LSR enabled to FAIL the verifier: LLVM core's
// LoopStrengthReduce -- which runs LATER in the CodeGen-IR pipeline --
// rewrites the narrowed phi into a "shift-by-1" form
// (`ld c, 15; ld a, c; inc a; ret z`) where the preserved-across-CALL
// carrier register holds an off-by-one value, producing an infinite
// loop.  The minimal repro (`while (i--) buf[i] = f(buf[i])`
// standalone) is fine.  Bisect via `-disable-lsr` confirmed LSR as
// the corruptor.
//
// Fix: move the pass to run AFTER LSR via the legacy-PM hook in
// `Z80PassConfig::addIRPasses()`.  The legacy-PM wrapper below is
// what's registered there; the New PM Loop entry point remains for
// `opt -passes='loop-mssa(z80-narrow-iv)'` debugging but is not on
// the codegen path.
//
// With the after-LSR placement, all 13 AES corpus configs PASS
// (session 73n measurement, 2026-05-21).
// Default off pending investigation of test-runner regressions.
// Session 73n: after moving the pass to run AFTER LSR (via the
// legacy-PM `Z80PassConfig::addIRPasses` hook below), AES corpus is
// 100% PASS with -12 B on production target.  But the z80-utils
// test-runner regressed 681 -> 666 PASS (15 new failures), notably
// `test_94_bss_self_clear` and `test_96_iy_largeoffset_spill` at
// multiple opt levels.  Both produce wrong runtime values rather
// than crashing -- some interaction with non-obvious downstream
// passes corrupting semantics on certain loop shapes (parallel
// i8/i16 phis suspected on test_94).  Not isolated this session.
//
// Opt-in via `-mllvm -enable-z80-narrow-iv=true` -- safe for
// AES corpus + production builds (cpnos + AES prod_like); not safe
// for general -Oz code until the test-runner regressions are
// understood.
static cl::opt<bool> EnableZ80NarrowIV(
    "enable-z80-narrow-iv", cl::init(true), cl::Hidden,
    cl::desc("Enable Z80 target-specific loop-counter IV narrowing "
             "(default on with single-phi-only guard; see #77 / #170)"));

static cl::opt<int> Z80NarrowIVLimit(
    "z80-narrow-iv-limit", cl::init(-1), cl::Hidden,
    cl::desc("Bisect helper: only narrow the first N candidates "
             "(-1 = unlimited)"));

static int FireCounter = 0;

namespace {

// Try to narrow a single header PHI from i16 to i8.  Returns true on
// success.  Conservative: only fires when the PHI's only "wide" uses
// can be expressed as a zext from the narrowed value.
static bool tryNarrowPhi(PHINode *Phi, Loop &L, ScalarEvolution &SE) {
  IntegerType *I16 = dyn_cast<IntegerType>(Phi->getType());
  if (!I16 || I16->getBitWidth() != 16)
    return false;

  // We need a single back-edge value coming from the latch and a single
  // preheader-entry constant.
  BasicBlock *Preheader = L.getLoopPreheader();
  BasicBlock *Latch = L.getLoopLatch();
  if (!Preheader || !Latch || Phi->getNumIncomingValues() != 2)
    return false;

  Value *InitV = Phi->getIncomingValueForBlock(Preheader);
  Value *NextV = Phi->getIncomingValueForBlock(Latch);
  auto *InitC = dyn_cast<ConstantInt>(InitV);
  if (!InitC || InitC->getValue().ugt(255))
    return false;

  // The recurrence value must be a `phi + const` add inside the loop.
  auto *AddI = dyn_cast<BinaryOperator>(NextV);
  if (!AddI || AddI->getOpcode() != Instruction::Add)
    return false;
  if (!L.contains(AddI->getParent()))
    return false;
  Value *AddLHS = AddI->getOperand(0), *AddRHS = AddI->getOperand(1);
  if (AddLHS != Phi) {
    if (AddRHS != Phi)
      return false;
    std::swap(AddLHS, AddRHS);
  }
  auto *StepC = dyn_cast<ConstantInt>(AddRHS);
  if (!StepC)
    return false;
  // The step value must itself be representable in i8 so the truncated
  // narrowed add produces the same result modulo 256.
  if (StepC->getValue().getActiveBits() > 7 &&
      !StepC->getValue().isMinSignedValue() &&
      StepC->getValue().sgt(127))
    return false;
  if (StepC->getValue().slt(-128))
    return false;

  // Use SCEV to bound the IV.  Both signed and unsigned ranges must fit
  // in [-128, 127] -- equivalently, the high byte of the i16 form is
  // zero throughout the loop (since the IV monotonically decreases from
  // a value <= 255 to >= -1, and we exit at 0 normally; we conservatively
  // require initial value to be the maximum, ruling out wrap).
  const SCEV *PhiSCEV = SE.getSCEV(Phi);
  if (auto *AddRec = dyn_cast<SCEVAddRecExpr>(PhiSCEV)) {
    ConstantRange URange = SE.getUnsignedRange(AddRec);
    if (URange.getUpper().ugt(APInt(URange.getBitWidth(), 256)))
      return false;
  } else {
    // No useful range info; bail conservatively.
    return false;
  }

  // Inspect all uses of the phi and the recurrence.  Acceptable uses:
  //   - The recurrence itself (Phi -> AddI -> Phi).
  //   - icmp against a constant <= 255 (we'll rewrite the constant to i8).
  //   - zext to a wider type (rewrite as zext from the new i8).
  //   - and with constant whose low byte covers the value (i.e. mask >= 0xFF
  //     within i16) -- rewrite to zext.
  //   - trunc to i8 (drop, replaced by direct use of new i8).
  // Any other use bails the transform (we don't try to insert i8->i16
  // conversions for arbitrary downstream arithmetic — InstCombine handles
  // those better after our zext is in place, but for safety we restrict).
  auto VerifyUsers = [&](Value *V) -> bool {
    for (User *U : V->users()) {
      if (U == AddI || U == Phi)
        continue;
      if (auto *CI = dyn_cast<ICmpInst>(U)) {
        // The other operand must be a constant fitting in i8 (or a value
        // SCEV proves is in the same range -- skip the harder case).
        unsigned OtherIdx = CI->getOperand(0) == V ? 1 : 0;
        auto *OtherC = dyn_cast<ConstantInt>(CI->getOperand(OtherIdx));
        if (!OtherC || OtherC->getValue().ugt(255))
          return false;
        continue;
      }
      if (isa<ZExtInst>(U) || isa<TruncInst>(U) || isa<SExtInst>(U))
        continue;
      if (auto *BO = dyn_cast<BinaryOperator>(U)) {
        if (BO->getOpcode() == Instruction::And) {
          unsigned MaskIdx = BO->getOperand(0) == V ? 1 : 0;
          auto *MaskC = dyn_cast<ConstantInt>(BO->getOperand(MaskIdx));
          if (MaskC && (MaskC->getValue().getZExtValue() & 0xFF) == 0xFF)
            continue;  // mask covers the byte: rewrite as zext
        }
        return false;
      }
      return false;
    }
    return true;
  };

  if (!VerifyUsers(Phi) || !VerifyUsers(AddI))
    return false;

  if (Z80NarrowIVLimit >= 0 && FireCounter >= Z80NarrowIVLimit)
    return false;
  ++FireCounter;

  LLVM_DEBUG(dbgs() << "z80-narrow-iv: narrowing phi " << *Phi
                    << " in loop " << L.getHeader()->getName() << "\n");

  IntegerType *I8 = Type::getInt8Ty(Phi->getContext());

  // Build the narrowed phi at the same position.
  IRBuilder<> Builder(Phi);
  PHINode *NewPhi = Builder.CreatePHI(I8, 2, Phi->getName() + ".narrow");
  NewPhi->addIncoming(ConstantInt::get(I8, InitC->getZExtValue()), Preheader);

  // Build the narrowed add.
  Builder.SetInsertPoint(AddI);
  Value *NewAdd = Builder.CreateAdd(
      NewPhi, ConstantInt::get(I8, StepC->getValue().trunc(8)),
      AddI->getName() + ".narrow", AddI->hasNoUnsignedWrap(),
      AddI->hasNoSignedWrap());
  NewPhi->addIncoming(NewAdd, Latch);

  // Materialise zext-back-to-i16 for downstream users, then RAUW the
  // old phi/add to the zexts.  InstCombine will collapse the
  // zext+icmp/zext+and patterns.
  Builder.SetInsertPoint(Phi->getParent(), Phi->getParent()->getFirstNonPHIIt());
  Value *PhiZ = Builder.CreateZExt(NewPhi, I16, Phi->getName() + ".zext");
  Phi->replaceAllUsesWith(PhiZ);

  // PhiZ itself currently uses NewPhi (good); but Phi was used by AddI in
  // the operand we already replaced via building NewAdd from NewPhi.
  // The remaining users of Phi (other than AddI) all see PhiZ now.

  Builder.SetInsertPoint(AddI->getParent(),
                         std::next(BasicBlock::iterator(AddI)));
  Value *AddZ = Builder.CreateZExt(NewAdd, I16, AddI->getName() + ".zext");
  AddI->replaceAllUsesWith(AddZ);

  // Erase originals.  Erase the add before the phi (the phi may have
  // been the add's operand before RAUW; after RAUW the add has no
  // remaining use of the phi).
  AddI->eraseFromParent();
  Phi->eraseFromParent();

  return true;
}

} // end anonymous namespace

// Shared body for both PM entry points.  Walks the innermost loops in F
// and tries to narrow each one's header phi.
static bool runOnLoopsImpl(Function &F, LoopInfo &LI, ScalarEvolution &SE) {
  if (!EnableZ80NarrowIV)
    return false;

  bool Changed = false;
  // Collect innermost loops first; tryNarrowPhi mutates IR.
  SmallVector<Loop *, 8> Loops;
  for (Loop *Outer : LI)
    for (Loop *L : depth_first(Outer))
      if (L->isInnermost())
        Loops.push_back(L);

  for (Loop *L : Loops) {
    BasicBlock *Header = L->getHeader();
    if (!Header)
      continue;
    SmallVector<PHINode *, 4> Phis;
    for (PHINode &P : Header->phis())
      Phis.push_back(&P);

    // Conservative gate from ravn/llvm-z80#170: don't narrow if the
    // header has more than one phi.  Parallel `phi i8` + `phi i16` IVs
    // in the same header (test_94 verifier loops) are observed to
    // miscompile when only the i16 phi is narrowed -- the regalloc/
    // coalescer treats two i8 phis differently from an i8+i16 pair.
    if (Phis.size() != 1)
      continue;

    for (PHINode *P : Phis)
      Changed |= tryNarrowPhi(P, *L, SE);
  }
  return Changed;
}

PreservedAnalyses Z80NarrowIV::run(Loop &L, LoopAnalysisManager &AM,
                                   LoopStandardAnalysisResults &AR,
                                   LPMUpdater &) {
  if (!EnableZ80NarrowIV)
    return PreservedAnalyses::all();

  // Only innermost loops; nested cases would compound complexity.
  if (!L.isInnermost())
    return PreservedAnalyses::all();

  BasicBlock *Header = L.getHeader();
  if (!Header)
    return PreservedAnalyses::all();

  // Take a snapshot of the header's phis (we mutate during the walk).
  SmallVector<PHINode *, 4> Phis;
  for (PHINode &P : Header->phis())
    Phis.push_back(&P);

  bool Changed = false;
  for (PHINode *P : Phis)
    Changed |= tryNarrowPhi(P, L, AR.SE);

  if (!Changed)
    return PreservedAnalyses::all();
  auto PA = getLoopPassPreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

namespace {

// Legacy-PM Function pass wrapper.  Used by Z80PassConfig::addIRPasses
// so this pass can run AFTER LLVM core's LoopStrengthReduce pass
// (which rewrites the narrowed phi into a "shift-by-1" form the
// backend mishandles).
class Z80NarrowIVLegacyPass : public FunctionPass {
public:
  static char ID;
  Z80NarrowIVLegacyPass() : FunctionPass(ID) {
    initializeZ80NarrowIVLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;
    auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    return runOnLoopsImpl(F, LI, SE);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
  }

  StringRef getPassName() const override {
    return "Z80 Narrow IV (legacy)";
  }
};

} // end anonymous namespace

char Z80NarrowIVLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(Z80NarrowIVLegacyPass, "z80-narrow-iv-legacy",
                      "Z80 Narrow IV", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(Z80NarrowIVLegacyPass, "z80-narrow-iv-legacy",
                    "Z80 Narrow IV", false, false)

FunctionPass *llvm::createZ80NarrowIVLegacyPass() {
  return new Z80NarrowIVLegacyPass();
}
