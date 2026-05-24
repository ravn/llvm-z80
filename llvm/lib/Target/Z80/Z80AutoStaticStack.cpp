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
// Safety caveat (applies to both levels): a function F is unsafe under
// static-stack iff F is called CONCURRENTLY with itself (e.g. from
// both main flow and an ISR).  This pass does NOT check that
// condition -- the typical Z80 firmware shape has ISRs calling only
// ISR-specific helpers, not arbitrary user functions.  If a user's
// ISR shares a function with main flow, they must opt out explicitly
// via "target-features"="+no-static-stack" at the source level (not
// implemented yet -- needs feature-parser additions).
//
// Per ravn/llvm-z80#176/#40.  Opt-in via -mllvm -z80-auto-static-stack=true.
// Default off until a broader empirical validation lands.
//
//===----------------------------------------------------------------------===//

#include "Z80AutoStaticStack.h"
#include "Z80.h"
#include "llvm/ADT/SCCIterator.h"
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
    "z80-auto-static-stack", cl::init(false), cl::Hidden,
    cl::desc("Z80: auto-inject +static-stack on provably-non-recursive "
             "functions (default off; opt-in via "
             "-mllvm -z80-auto-static-stack=true).  Includes leaves (Level 1) "
             "and CallGraph-SCC-non-recursive functions (Level 2)."));

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

    bool Changed = false;
    for (Function &F : M)
      Changed |= processFunction(F, NonRecursive);
    return Changed;
  }

  StringRef getPassName() const override {
    return "Z80 Auto +static-stack";
  }

private:
  bool processFunction(Function &F,
                       const SmallPtrSetImpl<const Function *> &NonRecursive) {
    // Definition only.
    if (F.empty())
      return false;
    // Respect explicit user choice.
    StringRef Existing;
    if (F.hasFnAttribute("target-features"))
      Existing = F.getFnAttribute("target-features").getValueAsString();
    if (Existing.contains("static-stack"))
      return false; // already set (or explicitly disabled via +no-static-stack).

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

INITIALIZE_PASS_BEGIN(Z80AutoStaticStack, DEBUG_TYPE,
                      "Z80 Auto +static-stack on non-recursive", false, false)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_END(Z80AutoStaticStack, DEBUG_TYPE,
                    "Z80 Auto +static-stack on non-recursive", false, false)

ModulePass *llvm::createZ80AutoStaticStackPass() {
  return new Z80AutoStaticStack();
}

bool llvm::isZ80AutoStaticStackEnabled() { return EnableAutoStaticStack; }
