//===-- Z80Subtarget.cpp - Z80 Subtarget Information ----------------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Z80 specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "Z80Subtarget.h"

#include "llvm/BinaryFormat/ELF.h"
#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/Utils.h"
#include "llvm/CodeGen/LibcallLoweringInfo.h"
#include "llvm/MC/TargetRegistry.h"

#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "SM83CallLowering.h"
#include "Z80.h"
#include "Z80CallLowering.h"
#include "Z80FrameLowering.h"
#include "Z80InstructionSelector.h"
#include "Z80LegalizerInfo.h"
#include "Z80TargetMachine.h"

#define DEBUG_TYPE "z80-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "Z80GenSubtargetInfo.inc"

using namespace llvm;

Z80Subtarget::Z80Subtarget(const Triple &TT, const std::string &CPU,
                           const std::string &FS, const Z80TargetMachine &TM)
    : Z80GenSubtargetInfo(TT, CPU, /* TuneCPU */ CPU, FS), InstrInfo(*this),
      RegInfo(), FrameLowering(),
      TLInfo(TM, initializeSubtargetDependencies(CPU, FS, TM)),
      Legalizer(*this),
      InstSelector(createZ80InstructionSelector(TM, *this, RegBankInfo)),
      InlineAsmLoweringInfo(&TLInfo) {
  // Create the appropriate CallLowering after features are parsed.
  // initializeSubtargetDependencies (called during TLInfo init) sets HasSM83.
  if (hasSM83())
    CallLoweringInfo = std::make_unique<SM83CallLowering>(&TLInfo);
  else
    CallLoweringInfo = std::make_unique<Z80CallLowering>(&TLInfo);
}

// ravn/llvm-z80 #283: at -O3 (CodeGenOptLevel::Aggressive) route the 32-bit
// multiply runtime call to the signed-magnitude fast variant __mulsi3_fast.
// That variant takes the operand magnitudes first, so when the operands fit 16
// bits (the dominant (long)i16*i16 fixed-point case, e.g. the mandelbrot inner
// loop) it hits the 32->16x16 demote fast path in the z88dk classic multiply
// core -- ~2x faster than the always-32x32 default __mulsi3.  The low 32 bits
// of a signed vs unsigned product are identical, so this is a correct drop-in
// for every 32-bit multiply; it only costs a small abs/negate overhead on
// genuine-32-bit operands (which then miss the demote), which is why it is a
// speculative speed-over-predictability tradeoff gated to -O3 only.  Every
// other opt level (-O0/-O1/-O2/-Os/-Oz) keeps the default __mulsi3, so
// production firmware (built at -Os) is untouched.  Mirrors the #244
// __*hi3_fast div/mod routing.  SM83 (gbz80) is excluded: its multiply ABI and
// runtime differ and it has no _fast bridge, matching #244.
//
// This is the GlobalISel-legalizer libcall hook (the S32 G_MUL .libcallFor
// path); the legalizer consults this LibcallLoweringInfo, NOT the TLI instance,
// so the override must live here rather than in the Z80TargetLowering ctor.
void Z80Subtarget::initLibcallLoweringInfo(LibcallLoweringInfo &Info) const {
  if (!hasSM83() &&
      getTargetLowering()->getTargetMachine().getOptLevel() ==
          CodeGenOptLevel::Aggressive)
    Info.setLibcallImpl(RTLIB::MUL_I32, RTLIB::impl___mulsi3_fast);
}

Z80Subtarget &
Z80Subtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS,
                                              const TargetMachine &TM) {
  // Parse features string.
  ParseSubtargetFeatures(CPU, /* TuneCPU */ CPU, FS);

  return *this;
}
