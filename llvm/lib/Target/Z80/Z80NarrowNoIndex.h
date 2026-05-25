//===-- Z80NarrowNoIndex.h - Keep IX/IY-incompatible values out of IX/IY -*-C++-*-=//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80NARROWNOINDEX_H
#define LLVM_LIB_TARGET_Z80_Z80NARROWNOINDEX_H

namespace llvm {
class MachineFunctionPass;

MachineFunctionPass *createZ80NarrowNoIndexPass();
} // namespace llvm

#endif
