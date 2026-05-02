//===-- Z80LoopIdiomFill.h - Z80 Loop Idiom Fill Pass -----------*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Z80 Loop Idiom Fill pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80LOOPIDIOMFILL_H
#define LLVM_LIB_TARGET_Z80_Z80LOOPIDIOMFILL_H

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class FunctionPass;

// New-PM entry point.
struct Z80LoopIdiomFill : public PassInfoMixin<Z80LoopIdiomFill> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

// Legacy-PM entry point so the pass is reachable from llc's
// addIRPasses pipeline as well as from clang's PassBuilder.
FunctionPass *createZ80LoopIdiomFillLegacyPass();
void initializeZ80LoopIdiomFillLegacyPassPass(PassRegistry &);

} // end namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80LOOPIDIOMFILL_H
