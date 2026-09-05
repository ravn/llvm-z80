//===-- Z80LoopRotate.h - Z80 Target-Specific Loop Rotation -----*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Z80 target-specific loop rotation pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80LOOPROTATE_H
#define LLVM_LIB_TARGET_Z80_Z80LOOPROTATE_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class FunctionPass;

// New-PM entry point.
struct Z80LoopRotate : public detail::PassInfoMixin<Z80LoopRotate> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

// Legacy-PM entry point so the pass is reachable from llc's
// addIRPasses pipeline as well as from clang's PassBuilder.
FunctionPass *createZ80LoopRotateLegacyPass();
void initializeZ80LoopRotateLegacyPassPass(PassRegistry &);

} // end namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80LOOPROTATE_H
