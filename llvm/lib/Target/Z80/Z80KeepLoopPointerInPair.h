//===-- Z80KeepLoopPointerInPair.h - Keep i16 loop pointer out of IX/IY -*- C++ -*-=//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80KEEPLOOPPOINTERINPAIR_H
#define LLVM_LIB_TARGET_Z80_Z80KEEPLOOPPOINTERINPAIR_H

namespace llvm {

class MachineFunctionPass;

MachineFunctionPass *createZ80KeepLoopPointerInPairPass();

} // end namespace llvm

#endif // LLVM_LIB_TARGET_Z80_Z80KEEPLOOPPOINTERINPAIR_H
