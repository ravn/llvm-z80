//===-- Z80FuseCarryChain.cpp - Keep multi-byte carry in the flag ---------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Multi-byte (i16/i32/i64) add/sub is selected as a chain of carry pseudos:
//
//   ADD_HL_rr_CO  rhs0          ; HL += rhs0 ; carry -> A   (SBC A,A; AND 1)
//   ...                         ; flag-safe register shuffles
//   ADC_HL_rr_CIO rhs1, A       ; restore CF from A (LD A,r; RRCA) ; HL += rhs1+CF
//                               ;                                ; carry -> A
//
// Each limb boundary therefore pays a round-trip: the producer captures the
// carry flag into A as a 0/1 value (SBC A,A; AND 1) and the consumer restores
// it (LD A,r; RRCA) before the next add.  On a CPU with a native carry flag
// this is pure waste -- SDCC and the in-tree AVR backend simply emit
// `add hl,de; adc hl,bc`, leaving the carry in the flag across the limbs.
//
// This pass recognises a *complete* carry chain whose links are flag-safe
// (nothing between a producer and its consumer clobbers FLAGS, and the carry
// register is single-use) and whose terminal carry-out is dead, then rewrites
// the pseudos to the flag-resident real instructions:
//
//   ADD_HL_rr_CO  -> ADD_HL_BC/ADD_HL_DE          (sets CF, no capture)
//   ADC_HL_rr_CIO -> ADC_HL_BC/ADC_HL_DE          (reads + sets CF, no A)
//   SUB_HL_rr_BO  -> AND_A; SBC_HL_BC/SBC_HL_DE   (clears CF, then sub)
//   SBC_HL_rr_BIO -> SBC_HL_BC/SBC_HL_DE          (reads + sets CF, no A)
//
// Conservative by construction: if any link is not provably flag-safe, or the
// terminal carry-out / FLAGS is still live, the whole chain is left untouched
// and falls back to the existing register-carry expansion.  16-bit ADC/SBC
// HL,rr are Z80-only, so the pass is a no-op on SM83.
//
// Runs post-RA, before ExpandPostRAPseudos (so the pseudos are still intact and
// the carry register / dead-ness is concrete).
//
//===----------------------------------------------------------------------===//

#include "Z80FuseCarryChain.h"
#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80Subtarget.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "z80-fuse-carry-chain"

// NB: the flag name must differ from the pass's registration arg-name (also
// "z80-fuse-carry-chain", see INITIALIZE_PASS below). `opt` statically links the
// Z80 target and its legacy PassNameParser turns every registered pass arg-name
// into a cl option; a same-named cl::opt makes opt abort at startup with
// "Option 'z80-fuse-carry-chain' registered more than once". clang/llc use the
// new PM and are unaffected, which is why this stayed latent.
static cl::opt<bool>
    EnableFuseCarryChain("z80-enable-fuse-carry-chain", cl::init(true),
                         cl::Hidden,
                         cl::desc("Keep multi-byte add/sub carry in the flag "
                                  "instead of round-tripping through A"));

namespace {

// Roles a carry pseudo can play in a chain.
enum class CarryKind { Add, Sub };

class Z80FuseCarryChain : public MachineFunctionPass {
public:
  static char ID;
  Z80FuseCarryChain() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override { return "Z80FuseCarryChain"; }
  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  const TargetInstrInfo *TII = nullptr;

  bool processBlock(MachineBasicBlock &MBB);
};

} // namespace

char Z80FuseCarryChain::ID = 0;

INITIALIZE_PASS(Z80FuseCarryChain, DEBUG_TYPE,
                "Z80 Fuse Multi-byte Carry Chain", false, false)

/// True if MI is a chain-head producer (no carry-in): produces the carry in A.
static bool isChainHead(const MachineInstr &MI, CarryKind &K) {
  switch (MI.getOpcode()) {
  case Z80::ADD_HL_rr_CO:
    K = CarryKind::Add;
    return true;
  case Z80::SUB_HL_rr_BO:
    K = CarryKind::Sub;
    return true;
  default:
    return false;
  }
}

/// True if MI is a chain consumer (carry-in from a register, carry-out in A).
static bool isChainConsumer(const MachineInstr &MI, CarryKind &K) {
  switch (MI.getOpcode()) {
  case Z80::ADC_HL_rr_CIO:
    K = CarryKind::Add;
    return true;
  case Z80::SBC_HL_rr_BIO:
    K = CarryKind::Sub;
    return true;
  default:
    return false;
  }
}

/// Map the GR16_BCDE rhs register to the real flag-resident opcode.
static unsigned realAddOpc(Register RHS) {
  return RHS == Z80::BC ? Z80::ADC_HL_BC : Z80::ADC_HL_DE;
}
static unsigned realSubOpc(Register RHS) {
  return RHS == Z80::BC ? Z80::SBC_HL_BC : Z80::SBC_HL_DE;
}
static unsigned realHeadAddOpc(Register RHS) {
  return RHS == Z80::BC ? Z80::ADD_HL_BC : Z80::ADD_HL_DE;
}

/// Scan (P, C) exclusive for flag-safety: no instruction may define FLAGS, and
/// the carry register A must not be read or redefined between the producer and
/// its consumer.  Returns true if the interval is safe to thread carry in CF.
static bool intervalIsFlagSafe(MachineBasicBlock::iterator P,
                               MachineBasicBlock::iterator C) {
  for (auto It = std::next(P); It != C; ++It) {
    const MachineInstr &MI = *It;
    if (MI.isDebugInstr())
      continue;
    for (const MachineOperand &MO : MI.operands()) {
      if (!MO.isReg() || !MO.getReg())
        continue;
      Register R = MO.getReg();
      if (R == Z80::FLAGS && MO.isDef())
        return false; // a flag clobber breaks the CF thread
      if (R == Z80::A)
        return false; // carry value disturbed (read or redefined)
    }
    if (MI.getDesc().hasImplicitDefOfPhysReg(Z80::FLAGS))
      return false;
  }
  return true;
}

bool Z80FuseCarryChain::processBlock(MachineBasicBlock &MBB) {
  bool Changed = false;

  for (auto I = MBB.begin(), E = MBB.end(); I != E;) {
    MachineInstr &Head = *I;
    CarryKind HK;
    if (!isChainHead(Head, HK)) {
      ++I;
      continue;
    }

    // Walk the chain: head -> consumer -> consumer ...  Each link must be
    // flag-safe and the carry must be single-use (carry register A killed at
    // the consumer).  Collect the consumers.
    SmallVector<MachineInstr *, 4> Consumers;
    MachineBasicBlock::iterator Cur = I;
    bool ChainOk = true;
    while (true) {
      // Find the next chain-consumer, scanning forward past the flag-safe
      // register shuffles that the allocator inserted between the limbs.  The
      // FIRST consumer encountered is the only candidate; intervalIsFlagSafe()
      // then validates everything in between.
      MachineBasicBlock::iterator Cons = MBB.end();
      for (auto J = std::next(Cur); J != MBB.end(); ++J) {
        if (J->isDebugInstr())
          continue;
        CarryKind CK;
        if (isChainConsumer(*J, CK)) {
          // Carry-in is operand 1 (the GR8 carry register); it must read A and
          // be the same arithmetic kind (add vs sub) as the head.
          if (CK == HK && J->getNumOperands() > 1 && J->getOperand(1).isReg() &&
              J->getOperand(1).getReg() == Z80::A)
            Cons = J;
          break;
        }
      }
      if (Cons == MBB.end())
        break; // no further consumer; chain ends at Cur

      if (!intervalIsFlagSafe(Cur, Cons)) {
        ChainOk = false;
        break;
      }
      Consumers.push_back(&*Cons);
      Cur = Cons;
    }

    // Require at least one consumer and a clean, dead terminal carry/FLAGS.
    if (ChainOk && !Consumers.empty()) {
      MachineInstr *Term = Consumers.back();
      // Terminal carry-out (A) must be dead, and FLAGS must not be needed
      // downstream (the real ADC/SBC leaves different flags than SBC A,A;AND 1).
      bool TermClean = false;
      for (const MachineOperand &MO : Term->operands()) {
        if (MO.isReg() && MO.isDef() && MO.getReg() == Z80::A) {
          TermClean = MO.isDead();
          break;
        }
      }
      if (TermClean) {
        MachineBasicBlock::LivenessQueryResult FlagsLive =
            MBB.computeRegisterLiveness(
                MBB.getParent()->getSubtarget().getRegisterInfo(), Z80::FLAGS,
                Term);
        if (FlagsLive != MachineBasicBlock::LQR_Dead)
          TermClean = false;
      }

      if (TermClean) {
        // Rewrite the head.
        Register HeadRHS = Head.getOperand(0).getReg();
        DebugLoc DL = Head.getDebugLoc();
        if (HK == CarryKind::Add) {
          BuildMI(MBB, Head, DL, TII->get(realHeadAddOpc(HeadRHS)));
        } else {
          BuildMI(MBB, Head, DL, TII->get(Z80::AND_A)); // clear CF for low sub
          BuildMI(MBB, Head, DL, TII->get(realSubOpc(HeadRHS)));
        }
        MachineBasicBlock::iterator AfterHead = std::next(I);
        Head.eraseFromParent();
        I = AfterHead;

        // Rewrite each consumer to its flag-resident real instruction.
        for (MachineInstr *C : Consumers) {
          Register RHS = C->getOperand(0).getReg();
          unsigned Opc = (HK == CarryKind::Add) ? realAddOpc(RHS) : realSubOpc(RHS);
          BuildMI(MBB, *C, C->getDebugLoc(), TII->get(Opc));
          C->eraseFromParent();
        }

        LLVM_DEBUG(dbgs() << "  fused carry chain of " << Consumers.size()
                          << " link(s)\n");
        Changed = true;
        continue; // I already advanced past the (erased) head
      }
    }

    ++I;
  }

  return Changed;
}

bool Z80FuseCarryChain::runOnMachineFunction(MachineFunction &MF) {
  if (!EnableFuseCarryChain)
    return false;
  // 16-bit ADC/SBC HL,rr are Z80-only; SM83 lowers byte-wise.
  if (MF.getSubtarget<Z80Subtarget>().hasSM83())
    return false;

  TII = MF.getSubtarget().getInstrInfo();

  bool Changed = false;
  for (MachineBasicBlock &MBB : MF)
    Changed |= processBlock(MBB);
  return Changed;
}

MachineFunctionPass *llvm::createZ80FuseCarryChain() {
  return new Z80FuseCarryChain();
}
