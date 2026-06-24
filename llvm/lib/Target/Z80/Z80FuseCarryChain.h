#ifndef LLVM_LIB_TARGET_Z80_Z80FUSECARRYCHAIN_H
#define LLVM_LIB_TARGET_Z80_Z80FUSECARRYCHAIN_H

#include "llvm/Pass.h"

namespace llvm {
class MachineFunctionPass;
void initializeZ80FuseCarryChainPass(PassRegistry &);
MachineFunctionPass *createZ80FuseCarryChain();
} // namespace llvm

#endif
