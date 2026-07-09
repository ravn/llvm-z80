//===-- Z80SinkColdLoopIV.cpp - Sink cold-only LSR IVs -------------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Undo one specific LSR transform that is net-negative on Z80's 3-pair
// register file: maintaining an inner loop's affine *seed* value as an
// induction variable in the enclosing loop.
//
// Motivating case -- the Byte "sieve" benchmark scan loop
// (`for i: if (flags[i]) { prime = 2*i+3; for (k = 3*i+3; k<=SIZE; k+=prime)
//    flags[k]=0; count++; }`):
//
//   LSR strength-reduces `2*i+3` (the kill-loop stride) and `3*i+3` (the
//   kill-loop start) into two loop-carried IVs of the SCAN loop, advanced
//   EVERY scan iteration (`add %ivA,3` / `add %ivB,2`), even though both are
//   consumed ONLY inside the `if (flags[i])` branch (~2 % of scans, the prime
//   density).  On Z80 that parks two extra 16-bit pairs live across the whole
//   scan loop, which over-subscribes the file and spills the scan counter `i`
//   to BSS (`ld bc,(nn); inc bc; ld (nn),bc`) every iteration.  dcc keeps `i`
//   in a frame slot and recomputes `2*i+3` / `3*i+3` on demand only when the
//   flag is set -- ~2x fewer scan-loop cycles.
//
// This pass rewrites such an IV back to an on-demand recompute.  For a header
// phi PN of loop L that
//   * is an affine IV: PN = phi [Init, preheader], [add PN, Step, latch]
//     with Init loop-invariant and Step a constant,
//   * is NOT the canonical IV, and
//   * has EVERY use on a cold path (no use block dominates L's latch),
// and where L has a canonical `{0,+,1}` IV `i`, the value of PN at iteration k
// is exactly `Init + Step*i`.  We materialise that expression once in the
// (cold) nearest-common-dominator of the uses -- hoisted to a sub-loop
// preheader when the uses sit inside a nested loop so it stays loop-invariant
// there -- RAUW the uses, and delete the now-dead phi + its step add.  The two
// freed pairs let regalloc keep `i` (and the kill-loop pointers) in registers.
//
// Opt-in via -z80-sink-cold-loop-iv (default off) while it is measured.
// Runs post-LSR, same pipeline slot as Z80LoopInstrFormPrep, so LSR cannot
// re-hoist what we sink.
//
//===----------------------------------------------------------------------===//

#include "Z80SinkColdLoopIV.h"
#include "Z80.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "z80-sink-cold-loop-iv"

static cl::opt<bool> EnableZ80SinkColdLoopIV(
    "z80-sink-cold-loop-iv", cl::init(false), cl::Hidden,
    cl::desc("Z80: rewrite enclosing-loop IVs that only seed a nested loop's "
             "cold path back into an on-demand recompute (ravn/llvm-z80#250)."));

bool llvm::isZ80SinkColdLoopIVEnabled() { return EnableZ80SinkColdLoopIV; }

namespace {

// A header phi we can sink, plus everything needed to rewrite it.
struct Candidate {
  PHINode *PN;             // the cold-only IV
  Instruction *StepInst;   // add PN, Step (in latch)
  Value *Init;             // loop-invariant start value
  int64_t Step;            // constant increment
};

// The latch-incoming value of PN, if it is `add PN, C` / `add C, PN`.
static Instruction *matchAffineStep(PHINode *PN, Loop *L, int64_t &Step) {
  BasicBlock *Latch = L->getLoopLatch();
  Value *Inc = PN->getIncomingValueForBlock(Latch);
  auto *BO = dyn_cast<BinaryOperator>(Inc);
  if (!BO || BO->getOpcode() != Instruction::Add)
    return nullptr;
  Value *Other = nullptr;
  if (BO->getOperand(0) == PN)
    Other = BO->getOperand(1);
  else if (BO->getOperand(1) == PN)
    Other = BO->getOperand(0);
  else
    return nullptr;
  auto *CI = dyn_cast<ConstantInt>(Other);
  if (!CI)
    return nullptr;
  // Only PN may consume the step add, otherwise the "next" value is live too.
  if (!BO->hasOneUse())
    return nullptr;
  Step = CI->getSExtValue();
  return BO;
}

// Block from which `U` (a use of some value) actually reads it: for a phi the
// use lives on the incoming edge, so return that predecessor.
static BasicBlock *useBlock(const Use &U) {
  auto *UserI = cast<Instruction>(U.getUser());
  if (auto *Phi = dyn_cast<PHINode>(UserI))
    return Phi->getIncomingBlock(U);
  return UserI->getParent();
}

// Every use of PN (other than its own step add) must be cold: its use block
// must not dominate the loop latch, and must sit inside L.  Returns false if
// any use is hot or escapes L.
static bool allUsesCold(PHINode *PN, Instruction *StepInst, Loop *L,
                        DominatorTree &DT,
                        SmallVectorImpl<BasicBlock *> &UseBlocks) {
  BasicBlock *Latch = L->getLoopLatch();
  for (const Use &U : PN->uses()) {
    if (U.getUser() == StepInst)
      continue;
    BasicBlock *B = useBlock(U);
    if (!L->contains(B))
      return false;
    if (DT.dominates(B, Latch))
      return false; // executed every iteration -> not cold
    UseBlocks.push_back(B);
  }
  return !UseBlocks.empty();
}

static bool runOnLoop(Loop *L, ScalarEvolution &SE, LoopInfo &LI,
                     DominatorTree &DT) {
  BasicBlock *Preheader = L->getLoopPreheader();
  BasicBlock *Latch = L->getLoopLatch();
  BasicBlock *Header = L->getHeader();
  if (!Preheader || !Latch)
    return false;

  PHINode *CanonIV = L->getCanonicalInductionVariable();
  if (!CanonIV)
    return false; // need `i = {0,+,1}` to reconstruct Init + Step*i

  SmallVector<Candidate, 4> Cands;
  for (PHINode &PN : Header->phis()) {
    if (&PN == CanonIV)
      continue;
    if (!PN.getType()->isIntegerTy())
      continue;
    int64_t Step;
    Instruction *StepInst = matchAffineStep(&PN, L, Step);
    if (!StepInst)
      continue;
    Value *Init = PN.getIncomingValueForBlock(Preheader);
    if (!L->isLoopInvariant(Init))
      continue;
    Cands.push_back({&PN, StepInst, Init, Step});
  }
  if (Cands.empty())
    return false;

  bool Changed = false;
  for (Candidate &C : Cands) {
    SmallVector<BasicBlock *, 4> UseBlocks;
    if (!allUsesCold(C.PN, C.StepInst, L, DT, UseBlocks))
      continue;

    // Nearest common dominator of all cold uses.
    BasicBlock *Ins = UseBlocks[0];
    for (BasicBlock *B : drop_begin(UseBlocks))
      Ins = DT.findNearestCommonDominator(Ins, B);
    if (!Ins)
      continue;

    // If the insertion point sits inside a nested loop, hoist to that
    // sub-loop's preheader so the recompute stays loop-invariant there
    // (otherwise we would recompute it every inner-loop iteration).
    if (Loop *UL = LI.getLoopFor(Ins)) {
      if (UL != L && L->contains(UL)) {
        Loop *Top = UL;
        while (Top->getParentLoop() != L)
          Top = Top->getParentLoop();
        if (BasicBlock *PH = Top->getLoopPreheader())
          if (L->contains(PH))
            Ins = PH;
      }
    }

    // Safety: insertion point must be dominated by the header (so the
    // canonical IV is available), stay cold, and dominate every use.
    if (!DT.dominates(Header, Ins) || DT.dominates(Ins, Latch))
      continue;
    bool DominatesAll = true;
    for (BasicBlock *B : UseBlocks)
      if (!DT.dominates(Ins, B)) {
        DominatesAll = false;
        break;
      }
    if (!DominatesAll)
      continue;

    // Materialise Init + Step*i at the cold insertion point.  Insert at the
    // block's first non-phi point (not the terminator): a use may live IN this
    // block (e.g. the guard's `icmp %ivA`), and the canonical IV is a header
    // phi already live at block entry, so this dominates every in-block use and
    // every successor/phi-edge use.
    IRBuilder<> B(&*Ins->getFirstInsertionPt());
    Type *Ty = C.PN->getType();
    Value *IV = CanonIV;
    if (IV->getType() != Ty)
      IV = B.CreateZExtOrTrunc(IV, Ty);
    Value *Val = IV;
    if (C.Step != 1)
      Val = B.CreateMul(IV, ConstantInt::get(Ty, C.Step));
    Value *ZeroInit = dyn_cast<ConstantInt>(C.Init);
    bool InitIsZero = ZeroInit && cast<ConstantInt>(C.Init)->isZero();
    if (!InitIsZero)
      Val = B.CreateAdd(Val, C.Init);

    // Rewrite the cold uses, then delete the dead phi + step add.
    C.PN->replaceAllUsesWith(Val);
    C.PN->eraseFromParent();
    if (C.StepInst->use_empty())
      C.StepInst->eraseFromParent();

    LLVM_DEBUG(dbgs() << "z80-sink-cold-loop-iv: sank IV (step " << C.Step
                      << ") in " << Ins->getName() << "\n");
    Changed = true;
  }
  return Changed;
}

static bool runImpl(Function &F, ScalarEvolution &SE, LoopInfo &LI,
                   DominatorTree &DT) {
  bool Changed = false;
  SmallVector<Loop *, 8> Loops;
  for (Loop *Outer : LI)
    for (Loop *L : depth_first(Outer))
      Loops.push_back(L);
  for (Loop *L : Loops)
    Changed |= runOnLoop(L, SE, LI, DT);
  return Changed;
}

class Z80SinkColdLoopIVLegacyPass : public FunctionPass {
public:
  static char ID;
  Z80SinkColdLoopIVLegacyPass() : FunctionPass(ID) {
    initializeZ80SinkColdLoopIVLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;
    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    return runImpl(F, SE, LI, DT);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
  }

  StringRef getPassName() const override {
    return "Z80 Sink Cold Loop IV (legacy)";
  }
};

} // namespace

char Z80SinkColdLoopIVLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(Z80SinkColdLoopIVLegacyPass, DEBUG_TYPE,
                     "Z80 Sink Cold Loop IV", false, false)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(Z80SinkColdLoopIVLegacyPass, DEBUG_TYPE,
                   "Z80 Sink Cold Loop IV", false, false)

FunctionPass *llvm::createZ80SinkColdLoopIVLegacyPass() {
  return new Z80SinkColdLoopIVLegacyPass();
}

PreservedAnalyses Z80SinkColdLoopIV::run(Function &F,
                                        FunctionAnalysisManager &AM) {
  auto &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  if (!runImpl(F, SE, LI, DT))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
