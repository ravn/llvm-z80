//===-- Z80KeepLoopPointerInPair.cpp - Keep i16 loop pointer out of IX/IY --===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Sibling of Z80PinLoopPointer, for the OTHER shape of the loop-pointer regalloc
// problem (ravn/llvm-z80#249 / #251).  Z80PinLoopPointer handles a *byte*-array
// walk `base[k]=v; k+=stride` and pins the pointer to HL, because there the
// store is a single `ld (hl),a` and the advance is `add hl,stride` in place --
// everything wants HL and HL survives the store.
//
// This pass handles the WORD (i16) walk `*p++ = i`:
//
//     ld  iy,_buf                 ; pointer parked in IY  (the bug)
//     ld  de,512                  ; counter in DE
//   .L:
//     ld  l,e ; ld h,d ; ld a,l ; or h ; ret z     ; counter test
//     push iy ; pop bc ; inc bc ; inc bc           ; bc = p + 2   (IY->BC)
//     push iy ; pop hl ; ld (hl),e ; inc hl ; ld (hl),d   ; store  (IY->HL)
//     dec de
//     push bc ; pop iy                             ; p += 2       (BC->IY)
//     jr .L
//
// The store is 2 bytes wide, so it *walks* HL (`inc hl` between the two
// `ld (hl),.`), leaving HL at p+1.  The loop-carried next pointer is p+2, which
// therefore CANNOT live in HL alongside the store -- so the #250 "pin to HL"
// trick is impossible here.  With HL busy for the store, DE for the counter and
// BC for the p+2 advance, greedy's copy-elimination heuristic parks the
// loop-carried pointer in the only pair left -- IY -- and shuttles it
// IY<->BC<->HL with three push/pop pairs (~45-75 t-states) every iteration.
// (IR16 already carries CopyCost=3 to discourage this, but the copy-elimination
// heuristic overrides the cost -- the same failure mode documented in
// Z80PinLoopPointer and Z80SplitDjnzCounters.)
//
// The pointer does not need HL; it just must stay out of IX/IY.  Constraining
// the loop-carried pointer vreg and its advanced-next vreg to GR16NoIR
// {DE,HL,BC} forces greedy to keep them in a main pair; the per-iteration
// IY<->HL / IY<->BC push/pop shuttle collapses to a single cheap
// `ld l,c; ld h,b` copy into HL for the store:
//
//     ld  c,l ; ld b,h            ; pointer in BC
//   .L:
//     ld  l,e ; ld h,d ; ld a,l ; or h ; ret z
//     ld  l,c ; ld h,b            ; hl = p   (cheap, no push/pop)
//     inc bc ; inc bc             ; p += 2   (in place, in BC)
//     ld (hl),e ; inc hl ; ld (hl),d
//     dec de
//     jr .L
//
// Mechanism, lifecycle and legality gate mirror Z80PinLoopPointer exactly: runs
// pre-RA after the register coalescer (same slot as Z80SplitDjnzCounters), so
// the class constraint is present when LiveIntervals recomputes for greedy.
// constrainRegClass doubles as the legality gate (it returns null and changes
// nothing if GR16NoIR is not a legal subclass), and allUsesAcceptNoIR guards
// the real uses whose class constraints are not reflected in the def's class.
//
// Gated behind -z80-enable-keep-loop-pointer-in-pair (default OFF).  Corpus-only
// today (no production firmware component has the `*p++` i16 walk); tracked as
// B20 in tasks/known-suboptimal-codegen.md.
//
//===----------------------------------------------------------------------===//

#include "Z80KeepLoopPointerInPair.h"
#include "MCTargetDesc/Z80MCTargetDesc.h"
#include "Z80.h"
#include "Z80InstrInfo.h"
#include "Z80Subtarget.h"

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "z80-keep-loop-pointer-in-pair"

static cl::opt<bool> EnableZ80KeepLoopPointerInPair(
    "z80-enable-keep-loop-pointer-in-pair", cl::init(false), cl::Hidden,
    cl::desc("Keep the loop-carried pointer of an i16 `*p++` store loop out of "
             "IX/IY by constraining it to GR16NoIR (ravn/llvm-z80#249/#251)"));

namespace {

class Z80KeepLoopPointerInPair : public MachineFunctionPass {
public:
  static char ID;

  Z80KeepLoopPointerInPair() : MachineFunctionPass(ID) {
    initializeZ80KeepLoopPointerInPairPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Z80 Keep i16 loop pointer in a main register pair";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

char Z80KeepLoopPointerInPair::ID = 0;

INITIALIZE_PASS(Z80KeepLoopPointerInPair, DEBUG_TYPE,
                "Z80 Keep i16 loop pointer in a main register pair", false,
                false)

// True iff MI is a byte store through HL (`ld (hl),r` / `ld (hl),n`) -- the
// second operand of the `$hl = COPY %ptr` store idiom.
static bool isStoreThroughHL(const MachineInstr &MI) {
  switch (MI.getOpcode()) {
  case Z80::LD_HLind_A:
  case Z80::LD_HLind_B:
  case Z80::LD_HLind_C:
  case Z80::LD_HLind_D:
  case Z80::LD_HLind_E:
  case Z80::LD_HLind_H:
  case Z80::LD_HLind_L:
  case Z80::LD_HLind_n:
    return true;
  default:
    return false;
  }
}

// Every non-COPY/PHI use (and def) of Reg must be able to accept a GR16NoIR
// register, so constraining Reg to GR16NoIR is legal.  COPY and PHI impose no
// register-class constraint on the operand, so they are always fine.  Mirrors
// Z80PinLoopPointer::allUsesAcceptHL, but for the {DE,HL,BC} class.
static bool allUsesAcceptNoIR(Register Reg, const MachineRegisterInfo &MRI,
                              const TargetInstrInfo &TII,
                              const TargetRegisterInfo &TRI) {
  for (const MachineInstr &MI : MRI.reg_nodbg_instructions(Reg)) {
    if (MI.isCopy() || MI.isPHI())
      continue;
    for (unsigned I = 0, E = MI.getNumOperands(); I != E; ++I) {
      const MachineOperand &MO = MI.getOperand(I);
      if (!MO.isReg() || MO.getReg() != Reg)
        continue;
      const TargetRegisterClass *OpRC = MI.getRegClassConstraint(I, &TII, &TRI);
      if (OpRC && !TRI.getCommonSubClass(OpRC, &Z80::GR16NoIRRegClass))
        return false;
    }
  }
  return true;
}

// True iff Reg has a def in the function of the form `Reg = COPY Src` with Src
// virtual, and reports Src.  (The loop-carried update `%ptr = COPY %next`, or
// the advance seed `%next = COPY %ptr`.)
static bool hasVirtCopyDef(Register Reg, const MachineRegisterInfo &MRI,
                           Register &Src) {
  for (const MachineInstr &Def : MRI.def_instructions(Reg)) {
    if (!Def.isCopy())
      continue;
    const MachineOperand &SrcMO = Def.getOperand(1);
    if (SrcMO.isReg() && SrcMO.getReg().isVirtual()) {
      Src = SrcMO.getReg();
      return true;
    }
  }
  return false;
}

// True iff Reg has an INC16 or DEC16 def -- i.e. it is really advanced, not a
// bare copy of the pointer (guards against a degenerate ptr==next identity).
static bool hasStrideDef(Register Reg, const MachineRegisterInfo &MRI) {
  for (const MachineInstr &Def : MRI.def_instructions(Reg))
    if (Def.getOpcode() == Z80::INC16 || Def.getOpcode() == Z80::DEC16)
      return true;
  return false;
}

// Find the i16 store-pointer walk and report the pointer vreg Ptr and its
// advanced-next vreg Next.  Anchor: `$hl = COPY %Ptr` immediately followed by a
// `ld (hl),.` store.  Then require the loop-carried cycle %Ptr = COPY %Next and
// %Next = COPY %Ptr with %Next advanced by INC16/DEC16.  That def-use cycle only
// exists for a loop-carried recurrence, so no MachineLoopInfo is needed (and the
// pass stays inert -- no forced analysis -- when the flag is off).
static bool findWordPointerWalk(MachineBasicBlock &MBB,
                                const MachineRegisterInfo &MRI, Register &Ptr,
                                Register &Next) {
  for (MachineInstr &MI : MBB) {
    // `$hl = COPY %ptr` (dst physical HL, src virtual).
    if (!MI.isCopy())
      continue;
    if (!MI.getOperand(0).isReg() || MI.getOperand(0).getReg() != Z80::HL)
      continue;
    if (!MI.getOperand(1).isReg() || !MI.getOperand(1).getReg().isVirtual())
      continue;

    // The next non-meta instruction must be a store through HL.
    auto After = std::next(MI.getIterator());
    while (After != MBB.end() && After->isMetaInstruction())
      ++After;
    if (After == MBB.end() || !isStoreThroughHL(*After))
      continue;

    Register PtrCand = MI.getOperand(1).getReg();

    // Loop-carried cycle: %ptr = COPY %next  and  %next = COPY %ptr, with %next
    // actually advanced (INC16/DEC16).
    Register NextCand;
    if (!hasVirtCopyDef(PtrCand, MRI, NextCand))
      continue;
    Register BackToPtr;
    if (!hasVirtCopyDef(NextCand, MRI, BackToPtr) || BackToPtr != PtrCand)
      continue;
    if (!hasStrideDef(NextCand, MRI))
      continue;

    Ptr = PtrCand;
    Next = NextCand;
    return true;
  }
  return false;
}

bool Z80KeepLoopPointerInPair::runOnMachineFunction(MachineFunction &MF) {
  if (!EnableZ80KeepLoopPointerInPair)
    return false;

  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  if (!STI.hasZ80())
    return false;

  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetInstrInfo &TII = *STI.getInstrInfo();
  const TargetRegisterInfo &TRI = *STI.getRegisterInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    Register Ptr, Next;
    if (!findWordPointerWalk(MBB, MRI, Ptr, Next))
      continue;

    if (!allUsesAcceptNoIR(Ptr, MRI, TII, TRI) ||
        !allUsesAcceptNoIR(Next, MRI, TII, TRI))
      continue;

    // constrainRegClass intersects each vreg's class with GR16NoIR and returns
    // null (changing nothing) if that intersection is empty -- so it is also
    // the legality gate.
    if (!MRI.constrainRegClass(Ptr, &Z80::GR16NoIRRegClass))
      continue;
    if (!MRI.constrainRegClass(Next, &Z80::GR16NoIRRegClass))
      continue;

    LLVM_DEBUG(dbgs() << "z80-keep-loop-pointer-in-pair: constrained "
                      << printReg(Ptr, &TRI) << " / " << printReg(Next, &TRI)
                      << " to GR16NoIR in " << MF.getName() << "\n");
    Changed = true;
  }

  return Changed;
}

MachineFunctionPass *llvm::createZ80KeepLoopPointerInPairPass() {
  return new Z80KeepLoopPointerInPair;
}
