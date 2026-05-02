//===-- Z80LoopIdiomFill.cpp - Z80 Pattern-Fill Loop Idiom ----------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Recognise loops that fill a buffer with a fixed K-byte repeating pattern
// (K in {1, 2, 3, 4}) for a constant trip count N, and replace them with
// a seed (K loop-invariant byte stores) followed by a memcpy that lets
// the Z80 backend lower it as a single LDIR (issue #88).
//
// Pre-rewrite shape (the canonical for-loop form):
//
//   for (i = 0; i < N; i++) {
//     base[i*K + 0] = v0;     // v0..v_{K-1} loop-invariant, often constant
//     base[i*K + 1] = v1;
//     ...
//     base[i*K + K-1] = v_{K-1};
//   }
//
// Post-rewrite (in the loop preheader):
//
//   base[0]  = v0;
//   base[1]  = v1;
//   ...
//   base[K-1] = v_{K-1};
//   memcpy(base + K, base, K * (N - 1));
//
// The Z80 backend's memcpy lowering already emits this as `LD HL,base;
// LD DE,base+K; LD BC,K*(N-1); LDIR`.  LDIR's forward-direction copy with
// dst = src + K propagates the K-byte seed across the whole buffer (a
// classic Z80 trick).
//
// The pass is conservative: it only fires when the loop body contains
// nothing but the K bytes worth of stores plus the IV update, the trip
// count is a known constant >= 2, and the base pointer is loop-invariant.
//
// The pass exposes both a new-PM entry point (so clang's optimization
// pipeline picks it up via the pipeline parsing callback) and a legacy
// FunctionPass wrapper (so llc's addIRPasses pipeline picks it up).
// Both entry points share a common Function-level driver below.
//
//===----------------------------------------------------------------------===//

#include "Z80LoopIdiomFill.h"
#include "Z80.h"
#include "Z80InstrInfo.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#define DEBUG_TYPE "z80-loop-idiom-fill"

using namespace llvm;

namespace {

// Try to match the pattern-fill idiom on \p L.  Returns true if matched
// and rewritten; false otherwise.  Conservative -- bails on anything
// non-canonical.
bool tryRewritePatternFill(Loop &L, ScalarEvolution &SE, DominatorTree &DT,
                           LoopInfo &LI) {
  // Only innermost loops with at most two blocks (header + latch, or
  // a merged header/latch).
  if (!L.isInnermost())
    return false;
  BasicBlock *Header = L.getHeader();
  BasicBlock *Latch = L.getLoopLatch();
  if (!Latch)
    return false;
  SmallVector<BasicBlock *, 2> Blocks(L.blocks());
  if (Blocks.size() > 2)
    return false;

  // Trip count must be a known constant >= 2.  Use the SE helper that
  // returns the actual number of body executions regardless of whether
  // the loop is do-while-shaped or while-shaped.
  unsigned TripCount = SE.getSmallConstantTripCount(&L);
  if (TripCount < 2)
    return false;
  uint64_t N = TripCount;

  // Walk Body (and Header if separate) collecting StoreInsts.  Bail if
  // we see anything else with side effects, or a store with a
  // non-loop-invariant value, an unusual store width, or anything that
  // could read memory the rewrite would skip.
  const DataLayout &DL = Header->getModule()->getDataLayout();
  SmallVector<StoreInst *, 4> Stores;
  for (BasicBlock *BB : Blocks) {
    for (Instruction &I : *BB) {
      if (auto *SI = dyn_cast<StoreInst>(&I)) {
        if (SI->isVolatile() || !SI->isSimple())
          return false;
        Type *VT = SI->getValueOperand()->getType();
        // Integer stores only -- bytes for the seed, plus i16/i32 for
        // word-fill / dword-fill cases that the backend will split.
        if (!VT->isIntegerTy())
          return false;
        unsigned Bits = VT->getIntegerBitWidth();
        if (Bits != 8 && Bits != 16 && Bits != 32)
          return false;
        if (!L.isLoopInvariant(SI->getValueOperand()))
          return false;
        Stores.push_back(SI);
        continue;
      }
      if (I.mayHaveSideEffects())
        return false;
      if (I.mayReadFromMemory())
        return false;
    }
  }
  if (Stores.empty())
    return false;

  // Sum store sizes -> K (total pattern width in bytes).
  uint64_t KSum = 0;
  for (StoreInst *SI : Stores) {
    Type *VT = SI->getValueOperand()->getType();
    KSum += DL.getTypeStoreSize(VT).getFixedValue();
  }
  if (KSum < 1 || KSum > 4)
    return false;
  unsigned K = (unsigned)KSum;

  // Each store address must be a SCEVAddRecExpr in this loop with a
  // constant step equal to K (the total pattern width), and a constant
  // offset from a single loop-invariant base.  Each store covers
  // [Offset, Offset + StoreSize).  The covered ranges must partition
  // [0, K).
  struct Slot {
    StoreInst *SI;
    int64_t Offset;
    unsigned Size;
  };
  SmallVector<Slot, 4> Slots;
  Slots.reserve(Stores.size());
  const SCEV *FirstStart = nullptr;

  for (StoreInst *SI : Stores) {
    const SCEV *PtrSCEV = SE.getSCEV(SI->getPointerOperand());
    auto *AR = dyn_cast<SCEVAddRecExpr>(PtrSCEV);
    if (!AR || AR->getLoop() != &L || !AR->isAffine())
      return false;
    auto *StepC = dyn_cast<SCEVConstant>(AR->getStepRecurrence(SE));
    if (!StepC)
      return false;
    if (StepC->getAPInt() != APInt(StepC->getAPInt().getBitWidth(), K))
      return false;
    const SCEV *Start = AR->getStart();
    unsigned Size =
        DL.getTypeStoreSize(SI->getValueOperand()->getType()).getFixedValue();
    if (!FirstStart) {
      FirstStart = Start;
      Slots.push_back({SI, 0, Size});
      continue;
    }
    const SCEV *Diff = SE.getMinusSCEV(Start, FirstStart);
    auto *DiffC = dyn_cast<SCEVConstant>(Diff);
    if (!DiffC)
      return false;
    int64_t Off = DiffC->getAPInt().getSExtValue();
    if (Off < 0 || Off + Size > K)
      return false;
    Slots.push_back({SI, Off, Size});
  }

  // Validate the covered ranges partition [0, K).
  uint64_t Mask = 0;
  for (const Slot &S : Slots) {
    uint64_t Bits = ((1ull << S.Size) - 1) << S.Offset;
    if (Mask & Bits)
      return false;  // overlap
    Mask |= Bits;
  }
  if (Mask != ((1ull << K) - 1))
    return false;

  // Sort slots by offset for predictable seed-store order in the IR.
  llvm::sort(Slots,
             [](const Slot &A, const Slot &B) { return A.Offset < B.Offset; });

  // Need a preheader to land the seed + memcpy in.
  BasicBlock *Preheader = L.getLoopPreheader();
  if (!Preheader)
    return false;

  // Derive Base as a Value.  If the SCEV start is a SCEVUnknown, it's
  // an existing IR Value; use it.  Otherwise bail (full SCEV expansion
  // is overkill for the cases we target).
  Value *Base = nullptr;
  if (auto *Unk = dyn_cast<SCEVUnknown>(FirstStart))
    Base = Unk->getValue();
  if (!Base)
    return false;
  if (auto *Inst = dyn_cast<Instruction>(Base))
    if (!DT.dominates(Inst, Preheader->getTerminator()))
      return false;

  LLVM_DEBUG(dbgs() << "z80-loop-idiom-fill: matched K=" << K << " N=" << N
                    << " in " << Header->getParent()->getName() << "\n");

  IRBuilder<> Builder(Preheader->getTerminator());
  Type *I8 = Type::getInt8Ty(Header->getContext());
  Type *I16 = Type::getInt16Ty(Header->getContext());

  Value *I8Base = Base;

  // Emit one seed store per slot, preserving original value type.  The
  // backend will lower wider integer stores into byte writes when
  // needed; that cost is the same we'd otherwise pay inside the loop.
  for (const Slot &S : Slots) {
    Value *Addr = I8Base;
    if (S.Offset != 0)
      Addr = Builder.CreateInBoundsGEP(
          I8, I8Base, ConstantInt::get(I16, S.Offset), "z80.fill.seed");
    Builder.CreateAlignedStore(S.SI->getValueOperand(), Addr,
                               S.SI->getAlign(), S.SI->isVolatile());
  }

  // Emit memcpy(base + K, base, K*(N-1)).
  uint64_t CopyLen = (uint64_t)K * (N - 1);
  if (CopyLen > 0) {
    Value *DstAddr = Builder.CreateInBoundsGEP(
        I8, I8Base, ConstantInt::get(I16, K), "z80.fill.dst");
    Builder.CreateMemCpy(DstAddr, MaybeAlign(1), I8Base, MaybeAlign(1),
                         ConstantInt::get(I16, CopyLen));
  }

  // Erase the original stores.  deleteDeadLoop will remove the empty
  // body+IV-update plus header CFG.
  for (StoreInst *SI : Stores)
    SI->eraseFromParent();
  deleteDeadLoop(&L, &DT, &SE, &LI);
  return true;
}

// Function-level driver: walk all innermost loops; rewrite any that
// match the pattern-fill idiom.
bool runOnFunctionImpl(Function &F, ScalarEvolution &SE, DominatorTree &DT,
                      LoopInfo &LI) {
  bool Changed = false;
  // Collect innermost loops first to avoid iterator invalidation when
  // tryRewritePatternFill deletes a loop.
  SmallVector<Loop *, 8> Loops;
  for (Loop *Outer : LI)
    for (Loop *L : depth_first(Outer))
      if (L->isInnermost())
        Loops.push_back(L);
  for (Loop *L : Loops) {
    if (tryRewritePatternFill(*L, SE, DT, LI))
      Changed = true;
  }
  return Changed;
}

// Legacy FunctionPass wrapper so llc's addIRPasses pipeline picks
// the pass up.
class Z80LoopIdiomFillLegacyPass : public FunctionPass {
public:
  static char ID;
  Z80LoopIdiomFillLegacyPass() : FunctionPass(ID) {
    initializeZ80LoopIdiomFillLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;
    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    return runOnFunctionImpl(F, SE, DT, LI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesCFG();
  }

  StringRef getPassName() const override {
    return "Z80 Loop Idiom Fill (legacy)";
  }
};

} // namespace

char Z80LoopIdiomFillLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(Z80LoopIdiomFillLegacyPass, DEBUG_TYPE,
                      "Z80 Loop Idiom Fill", false, false)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(Z80LoopIdiomFillLegacyPass, DEBUG_TYPE,
                    "Z80 Loop Idiom Fill", false, false)

FunctionPass *llvm::createZ80LoopIdiomFillLegacyPass() {
  return new Z80LoopIdiomFillLegacyPass();
}

PreservedAnalyses Z80LoopIdiomFill::run(Function &F,
                                       FunctionAnalysisManager &AM) {
  auto &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);
  if (!runOnFunctionImpl(F, SE, DT, LI))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
