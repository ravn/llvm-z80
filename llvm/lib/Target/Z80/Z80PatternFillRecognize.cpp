//===-- Z80PatternFillRecognize.cpp - Z80 Pattern-Fill Recogniser ---------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Recognise loops that fill a buffer with a fixed K-byte repeating pattern
// (K in {1, 2, 3, 4}) for a constant trip count N, and replace them with a
// pattern-fill intrinsic (issue #88, #205).  Renamed 2026-06-09 from
// Z80LoopIdiomFill: the recognition logic below is target-agnostic and is
// the prototype of an eventual upstream LoopIdiomRecognize extension; the
// `Z80` prefix tracks where it lives, not what it knows.
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
// Post-rewrite (in the loop preheader), with the K seed bytes assembled into
// one little-endian integer pattern:
//
//   llvm.z80.pattern.fill(base, pattern, N);
//
// The backend lowers the intrinsic to a seed store + forward LDIR (`LD HL,base
// (src); LD DE,base+K (dst); LD BC,K*(N-1); LDIR`): LDIR's forward-direction
// overlapping copy with dst = src + K propagates the K-byte seed across the
// whole buffer (a classic Z80 trick).  Using a *target* intrinsic instead of
// the old overlapping-memcpy representation matters: the overlapping memcpy
// was UB in IR (it only survived via a `volatile` marker, #136), whereas the
// intrinsic is defined and opaque to generic passes (no InstCombine
// exploitation, no PreISelIntrinsicLowering loop-expansion).  SM83 has no LDIR
// so the backend unrolls the intrinsic to N independent pattern stores.
//
// The pass is conservative: it only fires when the loop body contains
// nothing but the K bytes worth of stores plus the IV update, the trip
// count is a known constant >= 2, the base pointer is loop-invariant, and the
// loop topology lets N (the store-block execution count) be derived exactly
// from the trip count -- otherwise it leaves the loop for the generic
// optimizer (ravn/llvm-z80#205: a raw trip count overruns a non-rotated loop
// by one pattern).
//
// The pass exposes both a new-PM entry point (so clang's optimization
// pipeline picks it up via the pipeline parsing callback) and a legacy
// FunctionPass wrapper (so llc's addIRPasses pipeline picks it up).
// Both entry points share a common Function-level driver below.
//
//===----------------------------------------------------------------------===//

#include "Z80PatternFillRecognize.h"
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
#include "llvm/IR/IntrinsicsZ80.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#define DEBUG_TYPE "z80-pattern-fill-recognize"

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

  // A unique exiting block + a small constant trip count are required to size
  // the fill.  getSmallConstantTripCount counts executions of that exiting
  // block; the map to N (= store-block executions = patterns to fill) is done
  // after store collection below, once we know which block holds the stores.
  BasicBlock *Exiting = L.getExitingBlock();
  if (!Exiting)
    return false;
  unsigned TripCount = SE.getSmallConstantTripCount(&L, Exiting);
  if (TripCount < 2)
    return false;

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

  // Map the loop trip count to N = the number of times the store block runs
  // (= patterns to fill).  getSmallConstantTripCount counts executions of the
  // exiting block, which is NOT always the store-block count -- so a raw trip
  // count overruns by one pattern on a non-rotated loop (ravn/llvm-z80#205: a
  // K=2 fill wrote BC = K*N instead of K*(N-1)).  Two shapes are handled
  // precisely; anything else bails to the generic optimizer:
  //   * Rotated loop (the latch is the unique exiting block, incl. the merged
  //     single-block case): every block runs once per iteration, so the store
  //     -- in the header or the latch -- runs exactly TripCount times.
  //   * Two-block non-rotated while/for (the header is the unique exit, store
  //     in the latch): the header runs once more than the latch holding the
  //     store, so the store runs TripCount-1 times.
  // (A store in the *header* of a non-rotated loop is left to the generic
  // optimizer -- its count depends on store-vs-exit-test order.)  All stores
  // must share one block for this mapping to hold.
  BasicBlock *StoreBB = Stores.front()->getParent();
  for (StoreInst *SI : Stores)
    if (SI->getParent() != StoreBB)
      return false;
  uint64_t N;
  if (Exiting == Latch)
    N = TripCount;            // rotated: all blocks run TripCount times
  else if (Exiting == Header && StoreBB == Latch)
    N = TripCount - 1;        // non-rotated while/for; store in latch
  else
    return false;             // unrecognised topology -- leave to generic opt
  if (N < 2)
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

  LLVM_DEBUG(dbgs() << "z80-pattern-fill-recognize: matched K=" << K << " N=" << N
                    << " in " << Header->getParent()->getName() << "\n");

  // Emit one of two intrinsics depending on K:
  //   K in {1, 2, 4}: llvm.experimental.memset.pattern -- the upstream-defined
  //     intrinsic.  Pre-2026-06-09 this was untenable because PreISelIntrinsicLowering
  //     unconditionally expanded it (libcall or loop); the new TTI hook
  //     `shouldExpandExperimentalMemSetPattern` (Z80 returns false for these
  //     widths) lets the intrinsic survive to the Z80 legalizer, which emits
  //     the same seed + LDIR idiom as before.
  //   K == 3: keep the fork-local llvm.z80.pattern.fill -- a pow-of-2 container
  //     (i32) carrying an explicit K=3 lets us avoid an i24 store-decomposition
  //     in the legalizer.  Migrating this to the upstream intrinsic requires
  //     generalising the seed-store path to widen non-pow-of-2 patterns; deferred
  //     to a follow-up.
  // SM83 (no LDIR) lowers either intrinsic to unrolled stores.
  IRBuilder<> Builder(Preheader->getTerminator());
  Type *I16 = Type::getInt16Ty(Header->getContext());

  bool UseUpstream = (K == 1 || K == 2 || K == 4);

  // Assemble the K-byte pattern as a single little-endian integer.  For the
  // upstream path, use the natural iK*8 width (i8/i16/i32).  For the fork
  // intrinsic (K==3), use a pow-of-2 container (i32) and pass the real K
  // explicitly so the backend never has to emit an i24 store.
  unsigned ContBytes = UseUpstream ? K : 4;
  Type *PatTy = IntegerType::get(Header->getContext(), ContBytes * 8);
  Value *Pattern = ConstantInt::get(PatTy, 0);
  for (const Slot &S : Slots) {
    Value *V = Builder.CreateZExtOrTrunc(S.SI->getValueOperand(), PatTy);
    if (S.Offset != 0)
      V = Builder.CreateShl(V, ConstantInt::get(PatTy, (uint64_t)S.Offset * 8));
    Pattern = Builder.CreateOr(Pattern, V);
  }

  if (UseUpstream) {
    // llvm.experimental.memset.pattern(ptr dst, iN pattern, iM count, i1 vol)
    // Type args are: [dst type, pattern type, count type].  isvolatile = false.
    Type *I1 = Type::getInt1Ty(Header->getContext());
    Builder.CreateIntrinsic(
        Intrinsic::experimental_memset_pattern,
        {Base->getType(), PatTy, I16},
        {Base, Pattern, ConstantInt::get(I16, N), ConstantInt::get(I1, 0)});
  } else {
    // K == 3 stays on the fork-local intrinsic for now.
    Builder.CreateIntrinsic(
        Intrinsic::z80_pattern_fill, {PatTy},
        {Base, Pattern, ConstantInt::get(I16, K), ConstantInt::get(I16, N)});
  }

  // Erase the original stores.  deleteDeadLoop will remove the empty
  // body+IV-update plus header CFG.
  for (StoreInst *SI : Stores)
    SI->eraseFromParent();
  // deleteDeadLoop asserts L->hasDedicatedExits().  When two sequential
  // loops share a CFG edge (e.g. the deleted loop's exit IS the next
  // loop's header), that header has a self-backedge predecessor outside
  // the deleted loop, breaking the contract.  Form dedicated exits so
  // the upstream invariant holds (ravn/llvm-z80#217).
  formDedicatedExitBlocks(&L, &DT, &LI, /*MSSAU=*/nullptr,
                          /*PreserveLCSSA=*/true);
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
class Z80PatternFillRecognizeLegacyPass : public FunctionPass {
public:
  static char ID;
  Z80PatternFillRecognizeLegacyPass() : FunctionPass(ID) {
    initializeZ80PatternFillRecognizeLegacyPassPass(*PassRegistry::getPassRegistry());
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
    return "Z80 Pattern Fill Recognize (legacy)";
  }
};

} // namespace

char Z80PatternFillRecognizeLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(Z80PatternFillRecognizeLegacyPass, DEBUG_TYPE,
                      "Z80 Pattern Fill Recognize", false, false)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(Z80PatternFillRecognizeLegacyPass, DEBUG_TYPE,
                    "Z80 Pattern Fill Recognize", false, false)

FunctionPass *llvm::createZ80PatternFillRecognizeLegacyPass() {
  return new Z80PatternFillRecognizeLegacyPass();
}

PreservedAnalyses Z80PatternFillRecognize::run(Function &F,
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
