//===-- Z80AutoStaticStack.cpp - Auto-enable +static-stack on leaves ------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// IR pass that adds "target-features"="+static-stack" to leaf functions
// (functions with no CALL / INVOKE instructions, ignoring inline asm).
// Leaves cannot recurse, so static-stack is structurally safe.
//
// Safety caveat: a leaf function ALSO must not be called concurrently
// with itself (e.g. from both main flow and an ISR).  This pass does
// NOT check that condition -- the typical Z80 firmware shape has ISRs
// calling only ISR-specific helpers, not arbitrary leaf functions.
// If a user's ISR calls the same leaf function as main flow, they
// must opt out explicitly via "target-features"="+no-static-stack" or
// similar at the source level.
//
// Per ravn/llvm-z80#176/#40.  Opt-in via -mllvm -z80-auto-static-stack=true.
// Default off until a broader empirical validation lands.
//
//===----------------------------------------------------------------------===//

#include "Z80AutoStaticStack.h"
#include "Z80.h"
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
    cl::desc("Z80: auto-inject +static-stack on leaf functions "
             "(default off; opt-in via -mllvm -z80-auto-static-stack=true)"));

namespace {

class Z80AutoStaticStack : public ModulePass {
public:
  static char ID;
  Z80AutoStaticStack() : ModulePass(ID) {}

  bool runOnModule(Module &M) override {
    if (!EnableAutoStaticStack)
      return false;
    bool Changed = false;
    for (Function &F : M)
      Changed |= processFunction(F);
    return Changed;
  }

  StringRef getPassName() const override {
    return "Z80 Auto +static-stack";
  }

private:
  bool processFunction(Function &F) {
    // Definition only.
    if (F.empty())
      return false;
    // Respect explicit user choice.
    StringRef Existing;
    if (F.hasFnAttribute("target-features"))
      Existing = F.getFnAttribute("target-features").getValueAsString();
    if (Existing.contains("static-stack"))
      return false; // already set (or explicitly disabled via +no-static-stack).
    // Leaf check.
    for (BasicBlock &BB : F)
      for (Instruction &I : BB) {
        auto *CB = dyn_cast<CallBase>(&I);
        if (!CB)
          continue;
        if (CB->isInlineAsm())
          continue; // inline asm is not a CALL we worry about.
        return false; // non-leaf.
      }
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

INITIALIZE_PASS(Z80AutoStaticStack, DEBUG_TYPE,
                "Z80 Auto +static-stack on leaves", false, false)

ModulePass *llvm::createZ80AutoStaticStackPass() {
  return new Z80AutoStaticStack();
}

bool llvm::isZ80AutoStaticStackEnabled() { return EnableAutoStaticStack; }
