//===-- Z80PatternFillRecognize.h - Z80 Pattern-Fill Recogniser -*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Z80 multi-byte pattern-fill loop recogniser.
// (Was Z80LoopIdiomFill until 2026-06-09: the recognition logic is target-
// agnostic and is the prototype of an eventual upstream LoopIdiomRecognize
// extension; the `Z80` prefix tracks where it lives, not what it knows.)
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80PATTERNFILLRECOGNIZE_H
#define LLVM_LIB_TARGET_Z80_Z80PATTERNFILLRECOGNIZE_H

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class FunctionPass;

// New-PM entry point.
struct Z80PatternFillRecognize : public detail::PassInfoMixin<Z80PatternFillRecognize> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

// Legacy-PM entry point so the pass is reachable from llc's
// addIRPasses pipeline as well as from clang's PassBuilder.
FunctionPass *createZ80PatternFillRecognizeLegacyPass();
void initializeZ80PatternFillRecognizeLegacyPassPass(PassRegistry &);

} // end namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80PATTERNFILLRECOGNIZE_H
