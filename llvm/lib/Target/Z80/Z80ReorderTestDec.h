//===-- Z80ReorderTestDec.h - Reorder test-then-dec MIR idiom -*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80REORDERTESTDEC_H
#define LLVM_LIB_TARGET_Z80_Z80REORDERTESTDEC_H

namespace llvm {
class MachineFunctionPass;

MachineFunctionPass *createZ80ReorderTestDecPass();
} // namespace llvm

#endif
