//===-- Z80AutoStaticStack.cpp - Auto-enable +static-stack on leaves ------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// IR pass that adds "target-features"="+static-stack" to functions that
// are provably non-recursive.  Two safety levels:
//
//   Level 1 (always on):  Leaf functions (no CALL / INVOKE).  Trivially
//   non-recursive; static-stack is structurally safe.
//
//   Level 2 (always on):  Non-leaf functions in a CallGraph SCC of size 1
//   AND not in their own SCC's self-edge.  No recursion cycle reachable
//   from F to itself.
//
// Safety gate (applies to both levels): a function F is unsafe under
// static-stack iff F is called CONCURRENTLY with itself (e.g. a helper
// shared between main flow and a preemptive ISR re-enters and clobbers
// its own fixed BSS slots).  runOnModule builds an "unsafe" taint set --
// everything reachable from an "interrupt"-attributed function, plus every
// address-taken function (an opaque indirect-call / runtime-vector target) --
// and processFunction refuses +static-stack on it.
//
// Per-function opt-out: a user disables static-stack on one function with
// __attribute__((target("no-static-stack"))), which clang lowers to
// "target-features"="...,-static-stack"; the substring check below skips it,
// and the feature parser clears the bit even if +static-stack were present.
//
// Per ravn/llvm-z80#176/#40.  Default on; global opt-out via
// -mllvm -z80-auto-static-stack=false.
//
//===----------------------------------------------------------------------===//

#include "Z80AutoStaticStack.h"
#include "Z80.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "z80-auto-static-stack"

static cl::opt<bool> EnableAutoStaticStack(
    "z80-enable-auto-static-stack", cl::init(true), cl::Hidden,
    cl::desc("Z80: auto-inject +static-stack on provably-non-recursive "
             "functions (default on; global opt-out via "
             "-mllvm -z80-auto-static-stack=false).  Includes leaves (Level 1) "
             "and CallGraph-SCC-non-recursive functions (Level 2), minus the "
             "ISR-concurrency / address-taken safety gate."));

namespace {

class Z80AutoStaticStack : public ModulePass {
public:
  static char ID;
  Z80AutoStaticStack() : ModulePass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<CallGraphWrapperPass>();
    AU.setPreservesCFG();
  }

  bool runOnModule(Module &M) override {
    if (!EnableAutoStaticStack)
      return false;

    // Build the set of non-recursive functions.
    SmallPtrSet<const Function *, 16> NonRecursive;
    CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();
    for (auto SCCIt = scc_begin(&CG); !SCCIt.isAtEnd(); ++SCCIt) {
      const std::vector<CallGraphNode *> &SCC = *SCCIt;
      // Multi-node SCC: every function in it is recursive.  Skip.
      if (SCC.size() != 1)
        continue;
      CallGraphNode *N = SCC.front();
      Function *F = N->getFunction();
      if (!F || F->empty())
        continue;
      // Self-edge in single-node SCC = direct recursion.  Skip.
      bool SelfEdge = false;
      for (const auto &Edge : *N)
        if (Edge.second == N) {
          SelfEdge = true;
          break;
        }
      if (SelfEdge)
        continue;
      NonRecursive.insert(F);
    }

    // ISR-concurrency / opaque-target safety gate (ravn/llvm-z80#176).
    // +static-stack puts locals in FIXED BSS slots, so a function that can run
    // CONCURRENTLY WITH ITSELF (it is executing when an interrupt fires and the
    // ISR path re-enters the same function, or a helper shared by main flow and
    // an ISR) would clobber its own slots.  Conservatively refuse +static-stack
    // on two ORed predicates (kept separate so the address-taken half can be
    // relaxed later via a main-reachability analysis without touching the ISR
    // half):
    //
    //   (1) reachable from any "interrupt"-attributed function (transitively,
    //       over direct CallGraph edges) -- a shared helper may re-enter; and
    //   (2) address-taken -- an opaque indirect call (CallsExternalNode) or a
    //       runtime-installed interrupt vector could dispatch to it, neither of
    //       which the CallGraph can resolve.
    //
    // Seeding the address-taken set directly covers the opaque-edge case: the
    // set of functions an opaque call can reach is exactly the address-taken
    // set, so a null callee on a tainted node's edge needs no special handling.
    SmallPtrSet<const Function *, 16> Unsafe;
    SmallVector<const Function *, 8> Work;
    for (Function &F : M)
      if (F.hasFnAttribute("interrupt") || F.hasAddressTaken())
        if (Unsafe.insert(&F).second)
          Work.push_back(&F);
    while (!Work.empty()) {
      const Function *F = Work.pop_back_val();
      const CallGraphNode *N = CG[F];
      if (!N)
        continue;
      for (const auto &Edge : *N) {
        const Function *Callee = Edge.second->getFunction();
        if (!Callee) // CallsExternalNode: opaque target, covered by (2).
          continue;
        if (Unsafe.insert(Callee).second)
          Work.push_back(Callee);
      }
    }

    bool Changed = false;
    for (Function &F : M)
      Changed |= processFunction(F, NonRecursive, Unsafe);
    return Changed;
  }

  StringRef getPassName() const override {
    return "Z80 Auto +static-stack";
  }

private:
  bool processFunction(Function &F,
                       const SmallPtrSetImpl<const Function *> &NonRecursive,
                       const SmallPtrSetImpl<const Function *> &Unsafe) {
    // Definition only.
    if (F.empty())
      return false;
    // Respect explicit user choice.
    StringRef Existing;
    if (F.hasFnAttribute("target-features"))
      Existing = F.getFnAttribute("target-features").getValueAsString();
    if (Existing.contains("static-stack"))
      return false; // already set (or explicitly disabled via -static-stack).

    // ISR-concurrency / opaque-target safety gate (see runOnModule).  Refuse
    // BEFORE the leaf scan: even a leaf ISR helper can run concurrently with
    // itself, so being a leaf does not make it safe.
    if (Unsafe.contains(&F))
      return false;

    // Level 1: leaf (no CALL / INVOKE).
    bool IsLeaf = true;
    for (BasicBlock &BB : F)
      for (Instruction &I : BB) {
        auto *CB = dyn_cast<CallBase>(&I);
        if (!CB)
          continue;
        if (CB->isInlineAsm())
          continue;
        IsLeaf = false;
        break;
      }
    bool Safe = IsLeaf;
    // Level 2: non-leaf but CallGraph-SCC-non-recursive.
    if (!Safe && NonRecursive.contains(&F))
      Safe = true;
    if (!Safe)
      return false;

    // Apply.
    std::string NewFeatures;
    if (Existing.empty())
      NewFeatures = "+static-stack";
    else
      NewFeatures = (Existing + ",+static-stack").str();
    F.addFnAttr("target-features", NewFeatures);
    return true;
  }
};

} // end anonymous namespace

char Z80AutoStaticStack::ID = 0;

// NB: the pass registration name must differ from the cl::opt flag name
// ("z80-auto-static-stack", == DEBUG_TYPE).  `opt` builds a PassNameParser
// that registers every pass's name as a CLI literal option; if it equals an
// existing cl::opt the CommandLine layer aborts with "registered more than
// once" (crashes `opt -mtriple=z80` outright -- only `opt`, since llc/clang
// don't build that parser).  Use a distinct "-pass" suffix here; DEBUG_TYPE
// (for -debug-only) and the user-facing flag are unchanged.
INITIALIZE_PASS_BEGIN(Z80AutoStaticStack, DEBUG_TYPE "-pass",
                      "Z80 Auto +static-stack on non-recursive", false, false)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_END(Z80AutoStaticStack, DEBUG_TYPE "-pass",
                    "Z80 Auto +static-stack on non-recursive", false, false)

ModulePass *llvm::createZ80AutoStaticStackPass() {
  return new Z80AutoStaticStack();
}

bool llvm::isZ80AutoStaticStackEnabled() { return EnableAutoStaticStack; }
