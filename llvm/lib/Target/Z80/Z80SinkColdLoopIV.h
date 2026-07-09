//===-- Z80SinkColdLoopIV.h - Sink cold-only LSR IVs -----------*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80SINKCOLDLOOPIV_H
#define LLVM_LIB_TARGET_Z80_Z80SINKCOLDLOOPIV_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class FunctionPass;
class PassRegistry;

// New-PM wrapper.
class Z80SinkColdLoopIV : public PassInfoMixin<Z80SinkColdLoopIV> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

FunctionPass *createZ80SinkColdLoopIVLegacyPass();
void initializeZ80SinkColdLoopIVLegacyPassPass(PassRegistry &);
bool isZ80SinkColdLoopIVEnabled();

} // namespace llvm

#endif // LLVM_LIB_TARGET_Z80_Z80SINKCOLDLOOPIV_H
