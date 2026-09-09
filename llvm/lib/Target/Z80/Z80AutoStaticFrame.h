//===-- Z80AutoStaticFrame.h - Auto-enable +static-frame on leaves --------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80AUTOSTATICFRAME_H
#define LLVM_LIB_TARGET_Z80_Z80AUTOSTATICFRAME_H

#include "llvm/Pass.h"

namespace llvm {

ModulePass *createZ80AutoStaticFramePass();

// Returns the cl::opt value (the pass is opt-in via -mllvm
// -z80-auto-static-frame=true).  Read at pipeline-construction time
// so the pass is omitted from the pipeline entirely when disabled --
// avoiding the small pipeline-ordering side effect of registering a
// pass that would just no-op (cf. ravn/llvm-z80#187).
bool isZ80AutoStaticFrameEnabled();

} // namespace llvm

#endif // LLVM_LIB_TARGET_Z80_Z80AUTOSTATICFRAME_H
