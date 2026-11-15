//===-- Z80StorePhiSink.h - Sink G_STORE of G_PHI for CISC RMW -*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Z80 Store PHI Sinking pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80STOREPHISINK_H
#define LLVM_LIB_TARGET_Z80_Z80STOREPHISINK_H

#include "llvm/CodeGen/MachineFunctionPass.h"

namespace llvm {

MachineFunctionPass *createZ80StorePhiSinkPass();

} // namespace llvm

#endif // not LLVM_LIB_TARGET_Z80_Z80STOREPHISINK_H
