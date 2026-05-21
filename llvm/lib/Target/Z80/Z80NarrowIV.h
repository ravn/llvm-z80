//===-- Z80NarrowIV.h - Z80 Loop-Counter IV Narrowing -----------*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Z80 loop-counter IV narrowing pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80NARROWIV_H
#define LLVM_LIB_TARGET_Z80_Z80NARROWIV_H

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

namespace llvm {

struct Z80NarrowIV : public PassInfoMixin<Z80NarrowIV> {
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM,
                        LoopStandardAnalysisResults &AR, LPMUpdater &U);
};

class FunctionPass;

// Legacy-PM wrapper so the pass can run AFTER LLVM core's LSR pass
// from `Z80PassConfig::addIRPasses`.  LSR rewrites the narrowed phi
// into a "shift-by-1" form that the backend mishandles (see issue
// #77 follow-up); running Z80NarrowIV after LSR avoids the
// interaction.
FunctionPass *createZ80NarrowIVLegacyPass();
void initializeZ80NarrowIVLegacyPassPass(PassRegistry &);

} // end namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80NARROWIV_H
