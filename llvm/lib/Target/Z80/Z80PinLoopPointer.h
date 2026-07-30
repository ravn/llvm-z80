//===-- Z80PinLoopPointer.h - Pin byte-array pointer-walk IV to HL -*- C++ -*-=//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80PINLOOPPOINTER_H
#define LLVM_LIB_TARGET_Z80_Z80PINLOOPPOINTER_H

namespace llvm {

class MachineFunctionPass;

MachineFunctionPass *createZ80PinLoopPointerPass();

} // end namespace llvm

#endif // LLVM_LIB_TARGET_Z80_Z80PINLOOPPOINTER_H
