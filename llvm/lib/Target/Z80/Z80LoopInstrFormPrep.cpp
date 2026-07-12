//===-- Z80LoopInstrFormPrep.cpp - Z80 pointer-IV strength reduction -----===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Rewrite scale-1 byte-array loop address computations into a genuine
// pointer induction variable (ravn/llvm-z80#250).
//
// LLVM's canonical IV form for `array[i]` loops is `offset-phi + gep(base,
// offset)`: a plain integer IV plus a fresh `getelementptr` computed every
// iteration.  That's free on targets with `[base+reg]` addressing (x86, ARM,
// MSP430's `symbol(reg)` form), but on Z80 `BaseGV + reg` is not a legal
// addressing mode at all (`Z80TargetLowering::isLegalAddressingMode` already
// says so) -- so the backend has to re-derive `base+offset` from scratch
// every iteration (`ld hl,base; add hl,offset`), where a hand-written loop
// just walks a running pointer (`add hl,stride`).  See issue #250 for the
// full repro, the AVR/MSP430 cross-target evidence, and why the cheaper
// fixes (TTI `getPreferredAddressingMode`, `isLSRCostLess` tiebreak) don't
// help: Z80 has no post-increment addressing mode for LSR to prefer, and
// the register-pressure tiebreak in `isLSRCostLess` is exactly the tradeoff
// this pass makes explicit and Z80-tuned (see registerPressureOK below).
//
// Before:
//   for (k = start; k < N; k += stride)
//     ... = *(i8*)(base + k);        // gep i8, base, k -- recomputed every iter
// After (conceptually):
//   ptr = base + start;              // computed once, in the preheader
//   for (k = start; k < N; k += stride) {
//     ... = *ptr;
//     ptr += stride;                 // single pointer add per iteration
//   }
// The original integer IV `k` is left untouched -- it's still needed for the
// loop's exit test -- only the per-iteration address GEPs are replaced with
// the new pointer PHI.  This mirrors PowerPC's `PPCLoopInstrFormPrep` "update
// form" case (`PPCLoopInstrFormPrep.cpp`, using `SCEVExpander` to build the
// same shape), but is much simpler: Z80 has no scaled/offset addressing mode
// worth preparing for, so only the single scale-1/single-base case is
// handled -- no DS/DQ-offset forms, no multi-address bucket/commoning.
//
// Pipeline placement matters more than IR-vs-MIR: this MUST run strictly
// after LoopStrengthReduce, or LSR's own canonicalization would simply
// re-derive the offset-IV form we just rewrote away.  It is registered only
// in `Z80PassConfig::addIRPasses()`, in the same slot already used by
// `Z80PatternFillRecognize`/`Z80LoopRotate` -- immediately after the base
// `TargetPassConfig::addIRPasses()` call, which is where LSR itself runs
// (`TargetPassConfig.cpp`).  That slot is part of the codegen backend
// pipeline that clang invokes internally after its own middle-end optimizer
// runs, so it fires uniformly for `clang --target=z80` and `opt | llc`.
// Unlike `Z80PatternFillRecognize`, this pass is deliberately NOT also
// registered via a middle-end `PassBuilder` EP callback (e.g.
// `registerVectorizerStartEPCallback`): pattern-fill's rewrite produces an
// opaque intrinsic call that LSR can't touch, so an early middle-end copy is
// harmless there.  Ours produces a plain PHI+GEP pointer IV that backend LSR
// *would* re-canonicalize back to the offset form if it ran again after an
// early rewrite -- so registering this pass before backend LSR would be
// actively counterproductive, not just redundant.
//
//===----------------------------------------------------------------------===//

#include "Z80LoopInstrFormPrep.h"
#include "Z80.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

#define DEBUG_TYPE "z80-loop-instr-form-prep"

using namespace llvm;

// Register-pressure gate.  Z80 only has BC/DE/HL as general-purpose pairs;
// adding a pointer IV per rewritten group costs one more live register pair
// for the life of the loop.  If the loop already carries this many (or more)
// PHI-resident values across the backedge, decline -- the extra pointer
// would likely force a spill and net negative, a cost LSR's own
// `isLSRCostLess` NumRegs tiebreak can't see either (ravn/llvm-z80#250).
static cl::opt<unsigned> Z80MaxLoopCarriedPtrs(
   "z80-loop-instr-form-prep-max-carried", cl::Hidden, cl::init(2),
   cl::desc("Max pre-existing loop-carried PHIs before "
            "Z80LoopInstrFormPrep declines to add another pointer IV"));

// Default OFF: the register-pressure gate above (registerPressureOK) is a
// coarse IR-level PHI count, not a real post-RA pressure estimate, and the
// case that matters (a non-constant stride, so the old integer IV survives
// solely for the exit test) leaves 3 live 16-bit values through the loop
// (new pointer, old IV, stride) on a target with exactly 3 GP pairs and no
// spare -- initial testing on the #250 repro showed the allocator spilling
// to the static-stack scratch area rather than keeping all 3 in registers,
// which can net WORSE than the un-rewritten form.  Keep opt-in
// (-mllvm -z80-loop-instr-form-prep) until the register-pressure gate is
// tightened or IndVarSimplify-style exit-test rewriting (converting the
// exit test to compare pointers, eliminating the old IV entirely) is added.
static cl::opt<bool> EnableZ80LoopInstrFormPrep(
   "z80-enable-loop-instr-form-prep", cl::init(false), cl::Hidden,
   cl::desc("Enable Z80 pointer-IV strength reduction for scale-1 "
            "byte-array loops (ravn/llvm-z80#250, experimental)"));

bool llvm::isZ80LoopInstrFormPrepEnabled() {
 return EnableZ80LoopInstrFormPrep;
}

namespace {

// One recognised "base[k]"-shaped address computation: every Inst in
// \c Addrs shares the same (Start, Step) SCEV pair, so they can all be
// replaced by a single new pointer PHI.
struct AddrGroup {
 const SCEV *Start;
 const SCEV *Step;
 SmallVector<GetElementPtrInst *, 4> Addrs;
};

// Collect scale-1 byte-array address computations in \p L: GEPs with a
// single index into an i8 source element type, whose SCEV is an affine
// AddRec for \p L with a loop-invariant step and loop-invariant start, and
// whose only use is the load/store that consumes them (never escapes the
// loop, so replacing them with the header-resident PHI value -- the
// address at the START of the current iteration -- is always sound).
static bool collectAddrGroups(Loop &L, ScalarEvolution &SE,
                             SmallVectorImpl<AddrGroup> &Groups) {
 for (BasicBlock *BB : L.blocks()) {
   for (Instruction &I : *BB) {
     Value *PtrOp;
     if (auto *LI = dyn_cast<LoadInst>(&I))
       PtrOp = LI->getPointerOperand();
     else if (auto *SI = dyn_cast<StoreInst>(&I))
       PtrOp = SI->getPointerOperand();
     else
       continue;

     auto *GEP = dyn_cast<GetElementPtrInst>(PtrOp);
     // Only rewrite GEPs computed fresh in this block, used exactly once
     // (by the load/store found above) -- guards against the address
     // escaping the loop via some other user we'd silently mis-replace.
     if (!GEP || GEP->getParent() != BB || !GEP->hasOneUse())
       continue;
     if (GEP->getNumIndices() != 1 ||
         !GEP->getSourceElementType()->isIntegerTy(8))
       continue;
     if (!SE.isSCEVable(GEP->getType()))
       continue;
     auto *AR = dyn_cast<SCEVAddRecExpr>(SE.getSCEV(GEP));
     if (!AR || AR->getLoop() != &L || !AR->isAffine())
       continue;
     const SCEV *Step = AR->getStepRecurrence(SE);
     const SCEV *Start = AR->getStart();
     if (!SE.isLoopInvariant(Step, &L) || !SE.isLoopInvariant(Start, &L))
       continue;

     bool Found = false;
     for (AddrGroup &G : Groups) {
       if (G.Start == Start && G.Step == Step) {
         G.Addrs.push_back(GEP);
         Found = true;
         break;
       }
     }
     if (!Found)
       Groups.push_back({Start, Step, {GEP}});
   }
 }
 return !Groups.empty();
}

static cl::opt<bool> Z80AllowNestedLoopInstrFormPrep(
   "z80-loop-instr-form-prep-allow-nested", cl::Hidden, cl::init(false),
   cl::desc("Allow Z80LoopInstrFormPrep to rewrite loops nested inside "
            "another loop (default off: nested rewrites add a 3rd live "
            "16-bit value and empirically regress the sieve kill loop)"));

static cl::opt<bool> Z80LoopInstrFormPrepNoCostGate(
   "z80-loop-instr-form-prep-no-cost-gate", cl::Hidden, cl::init(false),
   cl::desc("Disable the profitability gate that only rewrites loops whose "
            "old integer IV can be eliminated (for experiments only: without "
            "it the pass regresses real loops, see ravn/llvm-z80#250)"));

static bool registerPressureOK(Loop &L, unsigned NewGroups) {
 // Nesting gate (ravn/llvm-z80#250).  A loop nested inside another loop
 // shares BC/DE/HL with the enclosing loop's live values; adding a pointer
 // IV here is what regressed the sieve KILL loop
 // (`for k in i_sq..SIZE step i: flags[k]`, nested in the `i` loop) by
 // +1.31M T-states in prior measurements -- the new pointer starves the
 // outer loop's IV and forces a spill.  Non-nested loops (e.g. sieve's
 // `for i: flags[i] = 0` init loop, or a flat guarded scan) have the whole
 // register file to themselves and are the safe, profitable case.  This is
 // a coarse structural proxy, not a real post-RA pressure estimate; the
 // escape hatch re-enables nested rewriting for experiments.
 if (L.getParentLoop() && !Z80AllowNestedLoopInstrFormPrep)
   return false;

 unsigned ExistingPHIs = 0;
 for (PHINode &PN : L.getHeader()->phis()) {
   (void)PN;
   ++ExistingPHIs;
 }
 return ExistingPHIs + NewGroups <= Z80MaxLoopCarriedPtrs;
}

// Try to eliminate the old integer IV entirely by rewriting the loop's
// exit test to compare pointers instead of integers (an LFTR-style
// transform, scoped to the one shape we need).
//
// WHY THIS STEP IS NOT OPTIONAL ON Z80: leaving the old IV alive (as a
// first cut of this pass did) keeps 3 live 16-bit values through the loop
// -- the new pointer, the loop-invariant stride, and the old IV chain used
// only for the compare.  Z80 has exactly 3 GP register pairs (BC/DE/HL, no
// spare with IX/IY reserved by default) and empirically this 3rd value
// forced the allocator to spill to the static-stack scratch area, turning
// the #250 repro from 508 to 680 bytes -- a regression, not a win.  This
// step drops back to 2 live values (pointer, stride) by deleting the old
// IV once nothing but the compare depends on it.
//
// Loop shape assumed: the loop's unique exiting block ends in
// `br i1 %c, ...` where %c = `icmp pred %oldIV, %bound` (either operand
// order), %oldIV is an AddRec for this loop sharing the SAME step as our
// new pointer IV (\p G.Step) -- e.g. `%kn = %k + %prime` is one iteration
// ahead of the GEP's own index `%k`, so its SCEV is
// {G.Start + G.Step, +, G.Step}, matching \p IncGEP's phase exactly -- and
// %bound is loop-invariant.  Bails (leaving the old IV alive, no regression
// beyond the address rewrite itself) on anything that doesn't match this
// precisely: non-relational predicates, an old-IV value used anywhere else,
// or a phase that lines up with neither \p NewPHI's nor \p IncGEP's start.
//
// Worked example (continuing #250's `flags[k] = 0` for `k += prime`,
// bound 8191): %kn's SCEV start is G.Start + G.Step, matching \p IncGEP, so
// `icmp ult i16 %kn, 8191` becomes `icmp ult ptr %z80.ivptr.next, %endptr`
// where `%endptr = getelementptr i8, ptr @flags, i16 8191` is materialised
// once in the preheader.  `%kn` and the header phi `%k` are then dead (no
// remaining uses) and are erased.
// Pieces of the loop's exit test that matched an address group -- what
// tryEliminateOldIV needs to rewrite the compare into pointer form, and what
// canEliminateOldIV inspects (without mutating) as the profitability gate.
struct OldIVMatch {
 CondBrInst *Br;
 ICmpInst *Cmp;
 Instruction *OldIVInst; // old integer IV chain value (`%kn`)
 PHINode *OldPHI;        // its header phi (`%k`)
 Value *BoundVal;        // loop-invariant compare bound
 bool OldIVIsLHS;        // old IV is the compare's LHS operand
 bool MatchesInc;        // old IV lines up with IncGEP (next iter) vs NewPHI
};

// Match \p L's exit test against address group \p G without mutating IR: does
// the unique exiting block's `icmp` compare an integer IV chain that shares
// G.Step and is dead once the address GEPs are gone?  \p BaseVal is the array
// base (passed in, since the GEPs may already be erased by the time the
// mutating tryEliminateOldIV runs).  Being pure, it doubles as the pass's
// profitability gate (canEliminateOldIV) run BEFORE any rewrite.
static bool matchOldIV(Loop &L, ScalarEvolution &SE, const AddrGroup &G,
                      Value *BaseVal, OldIVMatch &M) {
 BasicBlock *Exiting = L.getExitingBlock();
 if (!Exiting)
   return false;
 auto *Br = dyn_cast<CondBrInst>(Exiting->getTerminator());
 if (!Br)
   return false;
 auto *Cmp = dyn_cast<ICmpInst>(Br->getCondition());
 if (!Cmp || !Cmp->hasOneUse() || !Cmp->isRelational())
   return false;

 // Find the operand that is our old-IV chain: an AddRec for this loop
 // sharing G.Step (SCEV nodes are uniqued by ScalarEvolution, so pointer
 // equality is a valid identity check here, same as in collectAddrGroups).
 Value *OldIVUse = nullptr;
 bool OldIVIsLHS = false;
 for (unsigned Idx = 0; Idx < 2; ++Idx) {
   Value *Op = Cmp->getOperand(Idx);
   if (!SE.isSCEVable(Op->getType()))
     continue;
   auto *AR = dyn_cast<SCEVAddRecExpr>(SE.getSCEV(Op));
   if (AR && AR->getLoop() == &L && AR->getStepRecurrence(SE) == G.Step) {
     OldIVUse = Op;
     OldIVIsLHS = (Idx == 0);
     break;
   }
 }
 if (!OldIVUse)
   return false;
 Value *BoundVal = Cmp->getOperand(OldIVIsLHS ? 1 : 0);
 if (!SE.isLoopInvariant(SE.getSCEV(BoundVal), &L))
   return false;
 auto *OldIVUseInst = dyn_cast<Instruction>(OldIVUse);
 if (!OldIVUseInst)
   return false;

 // Resolve the old IV into its header phi (`%k`) and its increment
 // (`%kn = %k + step`), whichever operand the compare used:
 //   * exit test on the INCREMENT (`icmp %kn, N`, LSR's post-inc form) --
 //     OldIVUse is `%kn`, and `%k` is its header-phi operand;
 //   * exit test on the PHI itself (`icmp %k, N`, the canonical
 //     `for (i…) base[i]` form under -disable-lsr) -- OldIVUse is `%k`, and
 //     the increment is `%k`'s backedge value.
 // Both are eliminable: once the address GEPs and the compare are gone the
 // phi/increment pair is dead.  (The first cut of this pass only handled the
 // post-inc form, which is why guarded `base[i]` loops -- ravn/llvm-z80#250's
 // own shape -- were declined by the cost gate and stayed un-strength-reduced.)
 PHINode *OldPHI = nullptr;
 Instruction *OldInc = nullptr;
 if (auto *PN = dyn_cast<PHINode>(OldIVUseInst)) {
   if (PN->getParent() != L.getHeader())
     return false;
   OldPHI = PN;
   OldInc = dyn_cast<Instruction>(PN->getIncomingValueForBlock(L.getLoopLatch()));
   if (!OldInc)
     return false;
 } else {
   OldInc = OldIVUseInst;
   for (Use &U : OldInc->operands())
     if (auto *PN = dyn_cast<PHINode>(U.get()))
       if (PN->getParent() == L.getHeader())
         OldPHI = PN;
   if (!OldPHI)
     return false;
 }

 // The phi/increment pair must be dead once the address GEPs and the compare
 // are gone -- otherwise the old IV survives regardless and rewriting the
 // exit test wouldn't free a register.  `%kn` (OldInc) may only be used by
 // `%k` (its backedge) and the compare; `%k` (OldPHI) may only be used by
 // `%kn`, the compare, and the address GEPs we are about to erase (G.Addrs).
 auto isGroupAddr = [&](const User *U) {
   for (const GetElementPtrInst *A : G.Addrs)
     if (A == U)
       return true;
   return false;
 };
 for (User *U : OldInc->users())
   if (U != OldPHI && U != Cmp)
     return false;
 for (User *U : OldPHI->users())
   if (U != OldInc && U != Cmp && !isGroupAddr(U))
     return false;

 // Which phase does the old IV correspond to: the current-iteration
 // pointer (NewPHI) or the next-iteration one (IncGEP)?  Bail if neither
 // matches -- rewriting against the wrong phase would be a miscompile.
 //
 // The old IV is an INTEGER recurrence (e.g. {%start,+,%prime} for `%k` or
 // {%start+%prime,+,%prime} for `%kn`); the pointer IVs are POINTER
 // recurrences offset from the array base (NewPHI's start is
 // G.Start = {@flags+%start}, IncGEP's is G.Start+G.Step).  So we can't
 // compare the two starts directly -- they differ by the base.  Subtract
 // the base SCEV from each pointer phase to get the integer offset it
 // represents, and match THAT against the old IV's start:
 //   NewPHI  <-> integer offset (G.Start        - base)  [= %start]
 //   IncGEP  <-> integer offset (G.Start+G.Step - base)  [= %start+%prime]
 // (getMinusSCEV of two pointer-typed SCEVs yields an integer SCEV.)
 const SCEV *OldIVStart =
     cast<SCEVAddRecExpr>(SE.getSCEV(OldIVUse))->getStart();
 const SCEV *BaseSCEV = SE.getSCEV(BaseVal);
 const SCEV *NewPhiOffset = SE.getMinusSCEV(G.Start, BaseSCEV);
 const SCEV *IncGepOffset =
     SE.getMinusSCEV(SE.getAddExpr(G.Start, G.Step), BaseSCEV);
 bool MatchesInc;
 if (OldIVStart == NewPhiOffset)
   MatchesInc = false;
 else if (OldIVStart == IncGepOffset)
   MatchesInc = true;
 else
   return false;

 M = {Br, Cmp, OldInc, OldPHI, BoundVal, OldIVIsLHS, MatchesInc};
 return true;
}

// Non-mutating profitability gate: can the old integer IV be eliminated
// entirely if we rewrite \p G?  If not, the rewrite would leave the old IV
// live alongside the new pointer (+ stride), and on Z80's 3 GP pairs that
// added pressure costs setup bytes or a spill in every measured real loop
// while never producing a win -- so the pass declines the group.
static bool canEliminateOldIV(Loop &L, ScalarEvolution &SE,
                             const AddrGroup &G) {
 OldIVMatch M;
 return matchOldIV(L, SE, G, G.Addrs.front()->getPointerOperand(), M);
}

// Rewrite the loop's exit test to compare pointers, deleting the old integer
// IV.  Assumes matchOldIV succeeds (the pass only rewrites groups that passed
// canEliminateOldIV); returns false defensively if it no longer matches.
static bool tryEliminateOldIV(Loop &L, ScalarEvolution &SE,
                             const AddrGroup &G, PHINode *NewPHI,
                             Instruction *IncGEP, Value *BaseVal) {
 OldIVMatch M;
 if (!matchOldIV(L, SE, G, BaseVal, M))
   return false;

 Value *MatchingPtr = M.MatchesInc ? IncGEP : NewPHI;
 BasicBlock *Preheader = L.getLoopPreheader();
 Type *I8Ty = Type::getInt8Ty(L.getHeader()->getContext());
 // BoundVal already dominates the preheader (any def used inside the loop
 // must dominate the whole loop, hence its unique predecessor) -- no SCEV
 // re-expansion needed, just reuse the existing Value as the GEP index.
 IRBuilder<> PHBuilder(Preheader->getTerminator());
 Value *EndPtr =
     PHBuilder.CreateGEP(I8Ty, BaseVal, M.BoundVal, "z80.ivptr.end");

 // Place the new compare right before the branch terminator: when we match
 // against IncGEP (the next-iteration pointer, defined near the end of the
 // latch) the compare must be dominated by it, so it can't sit at the old
 // compare's (earlier) position.  Just before the terminator is dominated by
 // both NewPHI and IncGEP.
 IRBuilder<> CmpBuilder(M.Br);
 Value *NewCmp = CmpBuilder.CreateICmp(
     M.Cmp->getPredicate(), M.OldIVIsLHS ? MatchingPtr : EndPtr,
     M.OldIVIsLHS ? EndPtr : MatchingPtr, "z80.ivptr.cmp");
 M.Cmp->replaceAllUsesWith(NewCmp);
 M.Cmp->eraseFromParent();
 M.OldIVInst->eraseFromParent();
 if (M.OldPHI && M.OldPHI->use_empty())
   M.OldPHI->eraseFromParent();
 return true;
}

// Rewrite one recognised address group: materialise a start pointer once in
// the preheader, thread a pointer PHI through the header, advance it once
// per iteration in the latch, and replace every GEP in the group with the
// PHI directly.  The original integer IV is left alone -- it's still needed
// for the loop's exit test.
//
// Worked example (the ravn/llvm-z80#250 repro, `flags[k] = 0` for
// `k += prime`): Start = SCEVUnknown(@flags) + <loop-invariant start>,
// Step = SCEVUnknown(%prime) (not a compile-time constant -- confirmed
// handled since we only require loop-invariance, not a SCEVConstant).  The
// rewrite produces, in the preheader: `%ptr.init = getelementptr i8, ptr
// @flags, i16 %start`; in the header: `%ptr = phi ptr [%ptr.init, %ph],
// [%ptr.next, %latch]`; in the latch: `%ptr.next = getelementptr i8, ptr
// %ptr, i16 %prime`.  Every `getelementptr i8, ptr @flags, i16 %k` in the
// loop body is replaced by `%ptr`.
static bool rewriteAddrGroup(Loop &L, SCEVExpander &SCEVE,
                            const AddrGroup &G) {
 BasicBlock *Preheader = L.getLoopPreheader();
 BasicBlock *Header = L.getHeader();
 BasicBlock *Latch = L.getLoopLatch();
 if (!Preheader || !Header || !Latch)
   return false;

 LLVMContext &Ctx = Header->getContext();
 Type *I8Ty = Type::getInt8Ty(Ctx);
 PointerType *PtrTy = cast<PointerType>(G.Addrs.front()->getType());
 // Capture the original base pointer Value before the GEPs that reference
 // it are erased below -- needed both here (unused) and by
 // tryEliminateOldIV to rebuild an end-pointer for the rewritten exit test.
 Value *BaseVal = G.Addrs.front()->getPointerOperand();

 Value *StartPtr =
     SCEVE.expandCodeFor(G.Start, PtrTy, Preheader->getTerminator());
 Value *StepVal =
     SCEVE.expandCodeFor(G.Step, G.Step->getType(), Preheader->getTerminator());

 PHINode *NewPHI =
     PHINode::Create(PtrTy, 2, "z80.ivptr", Header->getFirstNonPHIIt());
 NewPHI->addIncoming(StartPtr, Preheader);

 auto *IncGEP = GetElementPtrInst::Create(
     I8Ty, NewPHI, StepVal, "z80.ivptr.next",
     Latch->getTerminator()->getIterator());
 NewPHI->addIncoming(IncGEP, Latch);

 LLVM_DEBUG(dbgs() << "z80-loop-instr-form-prep: rewrote " << G.Addrs.size()
                   << " address(es) in " << Header->getParent()->getName()
                   << " to pointer IV " << *NewPHI << "\n");

 for (GetElementPtrInst *GEP : G.Addrs)
   GEP->replaceAllUsesWith(NewPHI);
 for (GetElementPtrInst *GEP : G.Addrs)
   GEP->eraseFromParent();

 if (tryEliminateOldIV(L, *SCEVE.getSE(), G, NewPHI, IncGEP, BaseVal))
   LLVM_DEBUG(dbgs() << "z80-loop-instr-form-prep: eliminated old IV, exit "
                        "test now compares pointers\n");

 return true;
}

static bool runOnFunctionImpl(Function &F, ScalarEvolution &SE, LoopInfo &LI,
                             DominatorTree &DT) {
 bool Changed = false;
 SmallVector<Loop *, 8> Loops;
 for (Loop *Outer : LI)
   for (Loop *L : depth_first(Outer))
     if (L->isInnermost())
       Loops.push_back(L);

 for (Loop *L : Loops) {
   // A single latch is required (we advance the pointer IV in it); a missing
   // preheader is NOT fatal -- we insert one on demand below, but only after
   // the loop has passed every gate, so loops we decline (and functions made
   // entirely of them, e.g. the nested sieve kill loop) are left untouched.
   if (!L->getLoopLatch())
     continue;
   SmallVector<AddrGroup, 4> Groups;
   if (!collectAddrGroups(*L, SE, Groups))
     continue;
   if (!registerPressureOK(*L, Groups.size()))
     continue;
   // COST GATE (ravn/llvm-z80#250).  Keep only groups whose rewrite also lets
   // the old integer IV die (canEliminateOldIV).  A rewrite that leaves the
   // old IV alive adds a 2nd/3rd loop-carried 16-bit value (new pointer +
   // surviving counter [+ stride]) on a target with exactly BC/DE/HL and no
   // spare -- measured to add setup bytes (cpnos init.c:603) or force the
   // pointer to spill to the static-stack scratch (cpnos init.c:488), and to
   // slow sieve at -O2, while NEVER shrinking any real corpus/production loop.
   // Only the fully-eliminable shape (exit test rephrased as a pointer
   // compare, dropping back to {pointer[,stride]}) is profitable, so it is the
   // only shape we rewrite.  This is what makes the pass safe to run
   // unconditionally; -z80-loop-instr-form-prep-no-cost-gate bypasses it for
   // experiments (and re-exposes the regressions above).
   if (!Z80LoopInstrFormPrepNoCostGate)
     llvm::erase_if(Groups, [&](const AddrGroup &G) {
       return !canEliminateOldIV(*L, SE, G);
     });
   if (Groups.empty())
     continue;
   // Insert a dedicated preheader for THIS loop only if it lacks one.  A
   // zero-trip-guarded loop (`if (c==0) skip`) enters via a conditional
   // branch and has none until now (ravn/llvm-z80#250).  Doing it here --
   // per rewritten loop, rather than requiring whole-function LoopSimplify --
   // keeps declined functions byte-identical (whole-function LoopSimplify
   // perturbed sieve's register allocation by +1 spill slot even though the
   // pass rewrote nothing there).
   if (!L->getLoopPreheader() &&
       !InsertPreheaderForLoop(L, &DT, &LI, /*MSSAU=*/nullptr,
                               /*PreserveLCSSA=*/false))
     continue;
   SCEVExpander SCEVE(SE, "z80-loop-instr-form-prep");
   for (AddrGroup &G : Groups)
     if (rewriteAddrGroup(*L, SCEVE, G))
       Changed = true;
 }
 return Changed;
}

// Legacy FunctionPass wrapper so llc's/clang's codegen addIRPasses pipeline
// picks the pass up.
class Z80LoopInstrFormPrepLegacyPass : public FunctionPass {
public:
 static char ID;
 Z80LoopInstrFormPrepLegacyPass() : FunctionPass(ID) {
   initializeZ80LoopInstrFormPrepLegacyPassPass(*PassRegistry::getPassRegistry());
 }

 bool runOnFunction(Function &F) override {
   if (skipFunction(F))
     return false;
   auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
   auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
   auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
   return runOnFunctionImpl(F, SE, LI, DT);
 }

 void getAnalysisUsage(AnalysisUsage &AU) const override {
   // We insert a dedicated preheader on demand for the loops we rewrite
   // (see runOnFunctionImpl), so we do NOT require whole-function
   // LoopSimplify -- that perturbed the register allocation of functions we
   // decline (ravn/llvm-z80#250, sieve).  On-demand insertion mutates the
   // CFG, so this pass does NOT preserve it.
   AU.addRequired<ScalarEvolutionWrapperPass>();
   AU.addRequired<DominatorTreeWrapperPass>();
   AU.addRequired<LoopInfoWrapperPass>();
 }

 StringRef getPassName() const override {
   return "Z80 Loop Instruction Form Prep (legacy)";
 }
};

} // namespace

char Z80LoopInstrFormPrepLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(Z80LoopInstrFormPrepLegacyPass, DEBUG_TYPE,
                     "Z80 Loop Instruction Form Prep", false, false)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(Z80LoopInstrFormPrepLegacyPass, DEBUG_TYPE,
                   "Z80 Loop Instruction Form Prep", false, false)

FunctionPass *llvm::createZ80LoopInstrFormPrepLegacyPass() {
 return new Z80LoopInstrFormPrepLegacyPass();
}

PreservedAnalyses Z80LoopInstrFormPrep::run(Function &F,
                                          FunctionAnalysisManager &AM) {
 auto &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
 auto &LI = AM.getResult<LoopAnalysis>(F);
 auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
 if (!runOnFunctionImpl(F, SE, LI, DT))
   return PreservedAnalyses::all();
 PreservedAnalyses PA;
 return PA;
}
