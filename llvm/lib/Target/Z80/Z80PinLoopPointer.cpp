//===-- Z80PinLoopPointer.cpp - Pin byte-array pointer-walk IV to HL ------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Companion to Z80LoopInstrFormPrep (ravn/llvm-z80#250).  That IR pass turns
// a `base[k] = v; k += stride` byte-array loop into a genuine pointer-walk
// (pointer PHI advanced by `add hl,stride`, exit test a pointer compare vs an
// end-pointer) -- the correct shape.  But on its own it made the sieve
// benchmark SLOWER (29.68M vs 26.25M cycles), because the register allocator
// (greedy) parks the pointer IV in BC, not HL.  That is a bad choice on Z80:
//
//   * `add hl,rr` is the ONLY 16-bit add -- with the pointer in BC the loop
//     shuttles bc<->hl every iteration (ld l,c; ld h,b; ... ; ld c,l; ld b,h).
//   * that extra working-HL makes 4 live 16-bit values (pointer, stride,
//     end-pointer, working-HL) on a 3-pair register file, so the stride gets
//     spilled to the static-stack BSS scratch and reloaded every iteration
//     (`ld de,(__sfrend_main-2)`, ~20T) and the end-pointer constant is
//     re-materialised every iteration (`ld de,_flags+8191`, 10T).
//
// dcc keeps pointer=HL, stride=DE, end=BC, so the whole body is
// `ld (hl),v; add hl,de; ld a,l; sub c; ld a,h; sbc a,b; jr c` (~39T) with
// nothing spilled.  The ONLY thing standing between clang and that code is the
// register CHOICE: everything about the pointer already wants HL
// (STORE8_IND -> `ld (hl),v`, ADD_HL_rr hardcodes HL, CMP16_FLAGS reads its
// lhs out of HL as `ld a,l`), yet greedy's copy-elimination heuristic routes
// the PHI to BC anyway.
//
// Soft allocation hints do not survive greedy here (documented in
// Z80RegisterInfo::getRegAllocationHints -- the STORE8_IND->HL hint gave zero
// byte change on the AES corpus).  The mechanism that DOES work is the same one
// Z80SplitDjnzCounters uses for the DJNZ counter (#94/#99): constrain the vreg
// to a single-register class (there BCReg, here HLReg) so greedy has no choice.
//
// This pass runs pre-RA (after the register coalescer, before the LiveIntervals
// re-run -- same slot as Z80SplitDjnzCounters).  For each self-back-edge loop
// MBB it looks for the pointer-walk signature
//
//     %ptr  = PHI [ %start, %preheader ], [ %next, %thisMBB ]
//     STORE8_IND %ptr, implicit $a           ; ld (ptr),a
//     $hl    = COPY %ptr
//     ADD_HL_rr %stride                       ; hl = ptr + stride
//     %next  = COPY $hl
//     CMP16_FLAGS %next, %end (or %ptr, %end) ; pointer compare
//     <cond branch to thisMBB>
//
// and, if HLReg is a legal subclass at every use of %ptr and %next (it is:
// their only non-COPY/PHI uses are STORE8_IND=GR16 and CMP16_FLAGS=GR16NoIR,
// both of which contain HL), constrains both to HLReg.  With the pointer pinned
// to HL the `$hl = COPY %ptr` / `%next = COPY $hl` copies coalesce away, the
// store becomes `ld (hl),a`, the add is in place, and stride+end-pointer fall
// into BC/DE with no spill or remat.
//
// Gated behind -z80-pin-loop-pointer (default OFF): only meaningful when
// Z80LoopInstrFormPrep produced the pointer-walk in the first place.
//
//===----------------------------------------------------------------------===//

#include "Z80PinLoopPointer.h"
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

#define DEBUG_TYPE "z80-pin-loop-pointer"

static cl::opt<bool> EnableZ80PinLoopPointer(
    "z80-enable-pin-loop-pointer", cl::init(false), cl::Hidden,
    cl::desc("Pin byte-array pointer-walk induction variables to HL "
             "(ravn/llvm-z80#250 companion; requires -z80-loop-instr-form-prep "
             "to have created the pointer walk)"));

namespace {

class Z80PinLoopPointer : public MachineFunctionPass {
public:
  static char ID;

  Z80PinLoopPointer() : MachineFunctionPass(ID) {
    initializeZ80PinLoopPointerPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Z80 Pin loop pointer-walk IV to HL";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

char Z80PinLoopPointer::ID = 0;

INITIALIZE_PASS(Z80PinLoopPointer, DEBUG_TYPE,
                "Z80 Pin loop pointer-walk IV to HL", false, false)

// True iff MBB has a terminator that branches back to itself (a self loop).
static bool isSelfLoop(const MachineBasicBlock &MBB) {
  for (const MachineInstr &Term : MBB.terminators())
    for (const MachineOperand &MO : Term.operands())
      if (MO.isMBB() && MO.getMBB() == &MBB)
        return true;
  return false;
}

// Every non-COPY/PHI use (and def) of Reg must accept HL, so constraining Reg
// to HLReg is legal.  COPY and PHI impose no register-class constraint on the
// operand (their constraint query returns null), so they are always fine.
static bool allUsesAcceptHL(Register Reg, const MachineRegisterInfo &MRI,
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
      if (OpRC && !OpRC->contains(Z80::HL))
        return false;
    }
  }
  return true;
}

// In MBB, find the pointer-walk pointer vreg(s).  After the register coalescer
// (which runs before this pass) the canonical shape is a single self-recurring
// vreg:
//
//     STORE8_IND %P, implicit $a       ; ld (P),a   -- P from previous iter
//     $hl   = COPY %P
//     ADD_HL_rr %stride                 ; hl = P + stride
//     %P    = COPY $hl                  ; P re-defined (loop-carried)
//     CMP16_FLAGS %P, %end
//     <self back-edge branch>
//
// but before coalescing (or when the PHI survives) the value copied into $hl
// (%P) and the value copied back out (%Q) can be two distinct vregs joined by
// a header PHI.  Match both: require `$hl = COPY %P` immediately before an
// ADD_HL_rr and `%Q = COPY $hl` immediately after, with %P (or %Q) used by a
// STORE8_IND in this MBB.  Fills Ptr/Next (equal in the coalesced case).
static bool findPointerWalk(MachineBasicBlock &MBB,
                            const MachineRegisterInfo &MRI, Register &Ptr,
                            Register &Next) {
  for (MachineInstr &MI : MBB) {
    if (MI.getOpcode() != Z80::ADD_HL_rr)
      continue;

    // `%next = COPY $hl` must follow the ADD_HL_rr (skipping meta).
    auto After = std::next(MI.getIterator());
    while (After != MBB.end() && After->isMetaInstruction())
      ++After;
    if (After == MBB.end() || !After->isCopy())
      continue;
    if (!After->getOperand(0).isReg() || !After->getOperand(1).isReg())
      continue;
    if (After->getOperand(1).getReg() != Z80::HL)
      continue;
    Register NextCand = After->getOperand(0).getReg();
    if (!NextCand.isVirtual())
      continue;

    // `$hl = COPY %ptr` must precede the ADD_HL_rr (skipping meta backwards).
    if (MI.getIterator() == MBB.begin())
      continue;
    auto Before = std::prev(MI.getIterator());
    while (Before != MBB.begin() && Before->isMetaInstruction())
      --Before;
    if (!Before->isCopy())
      continue;
    if (!Before->getOperand(0).isReg() || !Before->getOperand(1).isReg())
      continue;
    if (Before->getOperand(0).getReg() != Z80::HL)
      continue;
    Register PtrCand = Before->getOperand(1).getReg();
    if (!PtrCand.isVirtual())
      continue;

    // The pointer must be the address of a STORE8_IND in this MBB -- confirms
    // this is the store-through pointer, not some other HL-add chain.  Accept
    // the store on either the in (%ptr) or out (%next) vreg (they are the same
    // vreg after coalescing).
    bool StoredThrough = false;
    for (Register R : {PtrCand, NextCand})
      for (const MachineInstr &Use : MRI.use_nodbg_instructions(R))
        if (Use.getOpcode() == Z80::STORE8_IND && Use.getParent() == &MBB)
          StoredThrough = true;
    if (!StoredThrough)
      continue;

    Ptr = PtrCand;
    Next = NextCand;
    return true;
  }
  return false;
}

bool Z80PinLoopPointer::runOnMachineFunction(MachineFunction &MF) {
  if (!EnableZ80PinLoopPointer)
    return false;

  const auto &STI = MF.getSubtarget<Z80Subtarget>();
  if (!STI.hasZ80())
    return false;

  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetInstrInfo &TII = *STI.getInstrInfo();
  const TargetRegisterInfo &TRI = *STI.getRegisterInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    if (!isSelfLoop(MBB))
      continue;

    Register Ptr, Next;
    if (!findPointerWalk(MBB, MRI, Ptr, Next))
      continue;

    // constrainRegClass intersects each vreg's current class with HLReg; it
    // returns null (and changes nothing) if HL is not in the current class, so
    // it doubles as the legality gate.  allUsesAcceptHL guards the real uses
    // (STORE8_IND / CMP16_FLAGS) whose class constraints are not reflected in
    // the def's register class.
    if (!allUsesAcceptHL(Ptr, MRI, TII, TRI) ||
        !allUsesAcceptHL(Next, MRI, TII, TRI))
      continue;

    if (!MRI.constrainRegClass(Ptr, &Z80::HLRegRegClass))
      continue;
    if (!MRI.constrainRegClass(Next, &Z80::HLRegRegClass))
      continue;

    LLVM_DEBUG(dbgs() << "z80-pin-loop-pointer: pinned " << printReg(Ptr, &TRI)
                      << " / " << printReg(Next, &TRI) << " to HL in "
                      << MF.getName() << "\n");
    Changed = true;
  }

  return Changed;
}

MachineFunctionPass *llvm::createZ80PinLoopPointerPass() {
  return new Z80PinLoopPointer;
}
