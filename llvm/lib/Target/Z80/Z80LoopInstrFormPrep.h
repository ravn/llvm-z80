//===-- Z80LoopInstrFormPrep.h - Z80 pointer-IV strength reduction *-C++-*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Z80 loop pointer-IV strength-reduction pass
// (ravn/llvm-z80#250).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80LOOPINSTRFORMPREP_H
#define LLVM_LIB_TARGET_Z80_Z80LOOPINSTRFORMPREP_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/CodeGen.h"

namespace llvm {

class FunctionPass;

// New-PM entry point.
struct Z80LoopInstrFormPrep : public PassInfoMixin<Z80LoopInstrFormPrep> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

// Legacy-PM entry point so the pass is reachable from llc's/clang's codegen
// addIRPasses pipeline.
FunctionPass *createZ80LoopInstrFormPrepLegacyPass();
void initializeZ80LoopInstrFormPrepLegacyPassPass(PassRegistry &);

// addPass-time gate: auto-on at -O2 (== Default opt level), off elsewhere,
// overridable with -mllvm -z80-enable-loop-instr-form-prep[=false]
// (ravn/llvm-z80#250).  Per-function -Os/-Oz exclusion is inside the pass.
bool isZ80LoopInstrFormPrepEnabled(CodeGenOptLevel OptLevel);

} // end namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80LOOPINSTRFORMPREP_H
