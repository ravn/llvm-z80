//===-- Z80InstrInfo.td - Z80 Instruction defs --------------*- tablegen -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the Z80 instructions in TableGen format.
//
// Z80 instruction encoding:
//   - Unprefixed: 1-3 bytes
//   - CB prefix: Bit manipulation (2 bytes)
//   - DD prefix: IX operations (2-4 bytes)
//   - FD prefix: IY operations (2-4 bytes)
//   - ED prefix: Extended operations (2-4 bytes)
//   - DD CB / FD CB: Indexed bit operations (4 bytes)
//
//===----------------------------------------------------------------------===//

include "Z80InstrFormats.td"
include "Z80InstrCommon.td"

//===----------------------------------------------------------------------===//
// Z80-only instructions
// These use ED/DD/FD prefixes, exchange operations, block transfer/search,
// I/O operations, sign/parity condition codes, or opcodes reused by SM83.
//===----------------------------------------------------------------------===//

let Predicates = [HasZ80] in {

//===----------------------------------------------------------------------===//
//  8-bit Load Group (Z80-only)
//===----------------------------------------------------------------------===//

// LD A,(nn) / LD (nn),A - Direct addressing
// (SM83 reuses opcodes 0x32/0x3A for LD (HL-),A / LD A,(HL-))
let mayLoad = true, mayStore = false, hasSideEffects = false in
def LD_A_nnind : Inst24<"ld a,", 0x3A, DirectMode> { let Defs = [A]; }
let mayLoad = false, mayStore = true, hasSideEffects = false in
def LD_nnind_A : Inst24<"ld", 0x32, DirectMode> {
  let AsmString = "ld ( $imm ),a";
  let Uses = [A];
}

// LD A,I / LD A,R / LD I,A / LD R,A - Special registers (ED prefix)
def LD_A_I : InstED8<"ld a,i", 0x57>;
def LD_A_R : InstED8<"ld a,r", 0x5F>;
def LD_I_A : InstED8<"ld i,a", 0x47>;
def LD_R_A : InstED8<"ld r,a", 0x4F>;

//===----------------------------------------------------------------------===//
//  16-bit Load Group (Z80-only)
//===----------------------------------------------------------------------===//

// LD HL,(nn) - Load HL from direct address
// (SM83 reuses opcode 0x2A for LD A,(HL+))
let mayLoad = true, mayStore = false, hasSideEffects = false in
def LD_HL_nnind : Inst24<"ld hl,", 0x2A, DirectMode> { let Defs = [HL]; }

// LD (nn),HL - Store HL to direct address
// (SM83 reuses opcode 0x22 for LD (HL+),A)
let mayLoad = false, mayStore = true, hasSideEffects = false in
def LD_nnind_HL : Inst24<"ld", 0x22, DirectMode> {
  let AsmString = "ld ( $imm ),hl";
  let Uses = [HL];
}

// LD dd,(nn) / LD (nn),dd - ED prefixed (DDDirectMode: addr at offset 2)
let mayLoad = true, mayStore = false, hasSideEffects = false in {
def LD_BC_nnind : InstED24<"ld bc,", 0x4B> { let Defs = [BC]; }
def LD_DE_nnind : InstED24<"ld de,", 0x5B> { let Defs = [DE]; }
def LD_SP_nnind : InstED24<"ld sp,", 0x7B> { let Defs = [SP]; }
}

let mayLoad = false, mayStore = true, hasSideEffects = false in {
def LD_nnind_BC : InstED24<"ld", 0x43> { let AsmString = "ld ( $imm ),bc"; let Uses = [BC]; }
def LD_nnind_DE : InstED24<"ld", 0x53> { let AsmString = "ld ( $imm ),de"; let Uses = [DE]; }
def LD_nnind_SP : InstED24<"ld", 0x73> { let AsmString = "ld ( $imm ),sp"; let Uses = [SP]; }
}

//===----------------------------------------------------------------------===//
//  Exchange Group
//===----------------------------------------------------------------------===//

def EX_DE_HL : Inst8<"ex de,hl", 0xEB> { let Uses = [DE, HL]; let Defs = [DE, HL]; }
def EX_AF_AF : Inst8<"ex af,af'", 0x08> { let Uses = [AF, AFp]; let Defs = [AF, AFp]; }
def EXX : Inst8<"exx", 0xD9> { let Uses = [BC, DE, HL, BCp, DEp, HLp]; let Defs = [BC, DE, HL, BCp, DEp, HLp]; }
let mayLoad = true, mayStore = true, hasSideEffects = false in
def EX_SPind_HL : Inst8<"ex (sp),hl", 0xE3> { let Uses = [SP, HL]; let Defs = [HL]; }

//===----------------------------------------------------------------------===//
//  Block Transfer and Search Group (ED prefixed)
//===----------------------------------------------------------------------===//

// Block transfer: read (HL), write (DE), decrement BC, inc/dec HL and DE
let mayLoad = true, mayStore = true in {
def LDI  : InstED8<"ldi",  0xA0> { let Uses = [HL, DE, BC]; let Defs = [HL, DE, BC, FLAGS]; }
def LDIR : InstED8<"ldir", 0xB0> { let Uses = [HL, DE, BC]; let Defs = [HL, DE, BC, FLAGS]; }
def LDD  : InstED8<"ldd",  0xA8> { let Uses = [HL, DE, BC]; let Defs = [HL, DE, BC, FLAGS]; }
def LDDR : InstED8<"lddr", 0xB8> { let Uses = [HL, DE, BC]; let Defs = [HL, DE, BC, FLAGS]; }
}

// Block compare: read (HL), compare with A, decrement BC, inc/dec HL
let mayLoad = true, mayStore = false in {
def CPI  : InstED8<"cpi",  0xA1> { let Uses = [HL, A, BC]; let Defs = [HL, BC, FLAGS]; }
def CPIR : InstED8<"cpir", 0xB1> { let Uses = [HL, A, BC]; let Defs = [HL, BC, FLAGS]; }
def CPD  : InstED8<"cpd",  0xA9> { let Uses = [HL, A, BC]; let Defs = [HL, BC, FLAGS]; }
def CPDR : InstED8<"cpdr", 0xB9> { let Uses = [HL, A, BC]; let Defs = [HL, BC, FLAGS]; }
}

//===----------------------------------------------------------------------===//
//  General-Purpose Arithmetic and CPU Control (Z80-only)
//===----------------------------------------------------------------------===//

def NEG : InstED8<"neg", 0x44> { let Uses = [A]; let Defs = [A, FLAGS]; }

// IM instructions
def IM_0 : InstED8<"im 0", 0x46>;
def IM_1 : InstED8<"im 1", 0x56>;
def IM_2 : InstED8<"im 2", 0x5E>;

//===----------------------------------------------------------------------===//
//  16-bit Arithmetic Group (Z80-only: ED prefixed)
//===----------------------------------------------------------------------===//

// ADC HL,ss (ED prefixed) - Add with carry (reads carry flag)
def ADC_HL_BC : InstED8<"adc hl,bc", 0x4A> { let Uses = [HL, BC, FLAGS]; let Defs = [HL, FLAGS]; }
def ADC_HL_DE : InstED8<"adc hl,de", 0x5A> { let Uses = [HL, DE, FLAGS]; let Defs = [HL, FLAGS]; }
def ADC_HL_HL : InstED8<"adc hl,hl", 0x6A> { let Uses = [HL, FLAGS]; let Defs = [HL, FLAGS]; }
def ADC_HL_SP : InstED8<"adc hl,sp", 0x7A> { let Uses = [HL, SP, FLAGS]; let Defs = [HL, FLAGS]; }

// SBC HL,ss (ED prefixed) - Subtract with carry (reads carry flag)
def SBC_HL_BC : InstED8<"sbc hl,bc", 0x42> { let Uses = [HL, BC, FLAGS]; let Defs = [HL, FLAGS]; }
def SBC_HL_DE : InstED8<"sbc hl,de", 0x52> { let Uses = [HL, DE, FLAGS]; let Defs = [HL, FLAGS]; }
def SBC_HL_HL : InstED8<"sbc hl,hl", 0x62> { let Uses = [HL, FLAGS]; let Defs = [HL, FLAGS]; }
def SBC_HL_SP : InstED8<"sbc hl,sp", 0x72> { let Uses = [HL, SP, FLAGS]; let Defs = [HL, FLAGS]; }

//===----------------------------------------------------------------------===//
//  Rotate and Shift Group (Z80-only: ED prefixed)
//===----------------------------------------------------------------------===//

// RLD / RRD (ED prefixed) - set flags
let mayLoad = true, mayStore = true, hasSideEffects = false in {
def RLD : InstED8<"rld", 0x6F> { let Uses = [HL, A]; let Defs = [A, FLAGS]; }
def RRD : InstED8<"rrd", 0x67> { let Uses = [HL, A]; let Defs = [A, FLAGS]; }
}

//===----------------------------------------------------------------------===//
//  Jump Group (Z80-only: sign/parity conditions + DJNZ)
//===----------------------------------------------------------------------===//

// JP cc,nn - Sign/parity conditions (Z80-only)
def JP_PO_nn : JPcond<CC_PO>;
def JP_PE_nn : JPcond<CC_PE>;
def JP_P_nn  : JPcond<CC_P>;
def JP_M_nn  : JPcond<CC_M>;

// DJNZ e - Decrement B and jump if not zero
def DJNZ_e : Inst16<"djnz", 0x10, RelativeMode>, InstConditionalBranch {
  let Uses = [B];
  let Defs = [B, FLAGS];
}

//===----------------------------------------------------------------------===//
//  Call and Return Group (Z80-only)
//===----------------------------------------------------------------------===//

// CALL_IY - Indirect call through IY register.
// Pseudo-instruction that expands to "call __call_iy" in expandPostRAPseudo.
// The runtime __call_iy trampoline does "jp (iy)".
// IY holds the function pointer; HL/DE are free for arguments.
let isPseudo = true in
def CALL_IY : Z80Inst<"call\t__call_iy">, InstCall {
  let InOperandList = (ins);
  let Uses = [IY];
  let Defs = [A, BC, DE, HL, IY, FLAGS];
}

// CALL cc,nn - Sign/parity conditions (Z80-only)
def CALL_PO_nn : CALLcond<CC_PO>;
def CALL_PE_nn : CALLcond<CC_PE>;
def CALL_P_nn  : CALLcond<CC_P>;
def CALL_M_nn  : CALLcond<CC_M>;

// RET cc - Sign/parity conditions (Z80-only)
def RET_PO : RETcond<CC_PO>;
def RET_PE : RETcond<CC_PE>;
def RET_P  : RETcond<CC_P>;
def RET_M  : RETcond<CC_M>;

// RETI / RETN (ED prefix)
def RETI : InstED8<"reti", 0x4D>, InstReturn;
def RETN : InstED8<"retn", 0x45>, InstReturn;

//===----------------------------------------------------------------------===//
//  Input and Output Group
//===----------------------------------------------------------------------===//

// IN A,(n)
def IN_A_n : Inst16<"in a,", 0xDB, PortMode> {
  let AsmString = "in a, ( $imm )";
  let Defs = [A];
}

// OUT (n),A
def OUT_n_A : Inst16<"out", 0xD3, PortMode> {
  let AsmString = "out ( $imm ), a";
  let Uses = [A];
}

// IN r,(C) - ED prefixed (sets flags)
// These instructions read from port specified by C and store in the register
let Uses = [C] in {
  def IN_B_C : InstED8<"in b,(c)", 0x40> { let Defs = [B, FLAGS]; }
  def IN_C_C : InstED8<"in c,(c)", 0x48> { let Defs = [C, FLAGS]; }
  def IN_D_C : InstED8<"in d,(c)", 0x50> { let Defs = [D, FLAGS]; }
  def IN_E_C : InstED8<"in e,(c)", 0x58> { let Defs = [E, FLAGS]; }
  def IN_H_C : InstED8<"in h,(c)", 0x60> { let Defs = [H, FLAGS]; }
  def IN_L_C : InstED8<"in l,(c)", 0x68> { let Defs = [L, FLAGS]; }
  def IN_A_C : InstED8<"in a,(c)", 0x78> { let Defs = [A, FLAGS]; }
}

// OUT (C),r - ED prefixed
// These instructions write the register value to port specified by C
let Uses = [C] in {
  def OUT_C_B : InstED8<"out (c),b", 0x41> { let Uses = [C, B]; }
  def OUT_C_C : InstED8<"out (c),c", 0x49> { let Uses = [C]; }
  def OUT_C_D : InstED8<"out (c),d", 0x51> { let Uses = [C, D]; }
  def OUT_C_E : InstED8<"out (c),e", 0x59> { let Uses = [C, E]; }
  def OUT_C_H : InstED8<"out (c),h", 0x61> { let Uses = [C, H]; }
  def OUT_C_L : InstED8<"out (c),l", 0x69> { let Uses = [C, L]; }
  def OUT_C_A : InstED8<"out (c),a", 0x79> { let Uses = [C, A]; }
}

// Block I/O (set flags)
def INI  : InstED8<"ini",  0xA2> { let Defs = [FLAGS]; }
def INIR : InstED8<"inir", 0xB2> { let Defs = [FLAGS]; }
def IND  : InstED8<"ind",  0xAA> { let Defs = [FLAGS]; }
def INDR : InstED8<"indr", 0xBA> { let Defs = [FLAGS]; }

def OUTI : InstED8<"outi", 0xA3> { let Defs = [FLAGS]; }
def OTIR : InstED8<"otir", 0xB3> { let Defs = [FLAGS]; }
def OUTD : InstED8<"outd", 0xAB> { let Defs = [FLAGS]; }
def OTDR : InstED8<"otdr", 0xBB> { let Defs = [FLAGS]; }

} // Predicates = [HasZ80]

//===----------------------------------------------------------------------===//
//  IX/IY Instructions (DD/FD prefixed) - Selected examples
//===----------------------------------------------------------------------===//

let Predicates = [HasZ80] in {

// LD IX,nn / LD IY,nn
def LD_IX_nn : InstDD24<"ld ix,", 0x21> { let Defs = [IX]; }
def LD_IY_nn : InstFD24<"ld iy,", 0x21> { let Defs = [IY]; }

// LD IX,(nn) / LD IY,(nn) — Direct addressing load
let mayLoad = true, mayStore = false, hasSideEffects = false in {
def LD_IX_nnind : InstDD24<"ld ix,", 0x2A, DDDirectMode> { let Defs = [IX]; }
def LD_IY_nnind : InstFD24<"ld iy,", 0x2A, DDDirectMode> { let Defs = [IY]; }
}

// LD (nn),IX / LD (nn),IY — Direct addressing store
let mayLoad = false, mayStore = true, hasSideEffects = false in {
def LD_nnind_IX : InstDD24<"ld", 0x22, DDDirectMode> { let AsmString = "ld ( $imm ),ix"; let Uses = [IX]; }
def LD_nnind_IY : InstFD24<"ld", 0x22, DDDirectMode> { let AsmString = "ld ( $imm ),iy"; let Uses = [IY]; }
}

// PUSH IX / PUSH IY
def PUSH_IX : InstDD8<"push ix", 0xE5> { let Uses = [IX]; }
def PUSH_IY : InstFD8<"push iy", 0xE5> { let Uses = [IY]; }

// POP IX / POP IY
def POP_IX : InstDD8<"pop ix", 0xE1> { let Defs = [IX]; }
def POP_IY : InstFD8<"pop iy", 0xE1> { let Defs = [IY]; }

// JP (IX) / JP (IY)
def JP_IXind : InstDD8<"jp (ix)", 0xE9>, InstUnconditionalBranch {
  let isIndirectBranch = true;
  let Uses = [IX];
}
def JP_IYind : InstFD8<"jp (iy)", 0xE9>, InstUnconditionalBranch {
  let isIndirectBranch = true;
  let Uses = [IY];
}

// LD SP,IX / LD SP,IY
def LD_SP_IX : InstDD8<"ld sp,ix", 0xF9> { let Uses = [IX]; let Defs = [SP]; }
def LD_SP_IY : InstFD8<"ld sp,iy", 0xF9> { let Uses = [IY]; let Defs = [SP]; }

// INC IX / INC IY / DEC IX / DEC IY
def INC_IX : InstDD8<"inc ix", 0x23> { let Uses = [IX]; let Defs = [IX]; }
def INC_IY : InstFD8<"inc iy", 0x23> { let Uses = [IY]; let Defs = [IY]; }
def DEC_IX : InstDD8<"dec ix", 0x2B> { let Uses = [IX]; let Defs = [IX]; }
def DEC_IY : InstFD8<"dec iy", 0x2B> { let Uses = [IY]; let Defs = [IY]; }

// ADD IX,pp / ADD IY,rr (sets carry, H)
def ADD_IX_BC : InstDD8<"add ix,bc", 0x09> { let Uses = [IX, BC]; let Defs = [IX, FLAGS]; }
def ADD_IX_DE : InstDD8<"add ix,de", 0x19> { let Uses = [IX, DE]; let Defs = [IX, FLAGS]; }
def ADD_IX_IX : InstDD8<"add ix,ix", 0x29> { let Uses = [IX];     let Defs = [IX, FLAGS]; }
def ADD_IX_SP : InstDD8<"add ix,sp", 0x39> { let Uses = [IX, SP]; let Defs = [IX, FLAGS]; }

def ADD_IY_BC : InstFD8<"add iy,bc", 0x09> { let Uses = [IY, BC]; let Defs = [IY, FLAGS]; }
def ADD_IY_DE : InstFD8<"add iy,de", 0x19> { let Uses = [IY, DE]; let Defs = [IY, FLAGS]; }
def ADD_IY_IY : InstFD8<"add iy,iy", 0x29> { let Uses = [IY];     let Defs = [IY, FLAGS]; }
def ADD_IY_SP : InstFD8<"add iy,sp", 0x39> { let Uses = [IY, SP]; let Defs = [IY, FLAGS]; }

// EX (SP),IX / EX (SP),IY
let mayLoad = true, mayStore = true, hasSideEffects = false in {
def EX_SPind_IX : InstDD8<"ex (sp),ix", 0xE3> { let Uses = [SP, IX]; let Defs = [IX]; }
def EX_SPind_IY : InstFD8<"ex (sp),iy", 0xE3> { let Uses = [SP, IY]; let Defs = [IY]; }
}

//===----------------------------------------------------------------------===//
// Indexed addressing: LD r,(IX+d) / LD r,(IY+d)
// Opcode: DD [46|4E|56|5E|66|6E|7E] d for IX
// Opcode: FD [46|4E|56|5E|66|6E|7E] d for IY
//===----------------------------------------------------------------------===//

// LD r,(IX+d) — IX is the address register (Use); r is the result (Def).
// IX must be in Uses so that liveness analysis and touchesReg() in the
// IX-transfer peephole (Z80LateOptimization.cpp Form 1/2) see IX as live
// across this instruction.  Omitting IX was the root cause of ravn/llvm-z80#243.
def LD_B_IXd : InstDD16<"ld b,", 0x46, IndexedIXMode> { let Uses = [IX]; let Defs = [B]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_C_IXd : InstDD16<"ld c,", 0x4E, IndexedIXMode> { let Uses = [IX]; let Defs = [C]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_D_IXd : InstDD16<"ld d,", 0x56, IndexedIXMode> { let Uses = [IX]; let Defs = [D]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_E_IXd : InstDD16<"ld e,", 0x5E, IndexedIXMode> { let Uses = [IX]; let Defs = [E]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_H_IXd : InstDD16<"ld h,", 0x66, IndexedIXMode> { let Uses = [IX]; let Defs = [H]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_L_IXd : InstDD16<"ld l,", 0x6E, IndexedIXMode> { let Uses = [IX]; let Defs = [L]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_A_IXd : InstDD16<"ld a,", 0x7E, IndexedIXMode> { let Uses = [IX]; let Defs = [A]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }

// LD r,(IY+d) — IY is the address register (Use); r is the result (Def).
def LD_B_IYd : InstFD16<"ld b,", 0x46, IndexedIYMode> { let Uses = [IY]; let Defs = [B]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_C_IYd : InstFD16<"ld c,", 0x4E, IndexedIYMode> { let Uses = [IY]; let Defs = [C]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_D_IYd : InstFD16<"ld d,", 0x56, IndexedIYMode> { let Uses = [IY]; let Defs = [D]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_E_IYd : InstFD16<"ld e,", 0x5E, IndexedIYMode> { let Uses = [IY]; let Defs = [E]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_H_IYd : InstFD16<"ld h,", 0x66, IndexedIYMode> { let Uses = [IY]; let Defs = [H]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_L_IYd : InstFD16<"ld l,", 0x6E, IndexedIYMode> { let Uses = [IY]; let Defs = [L]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }
def LD_A_IYd : InstFD16<"ld a,", 0x7E, IndexedIYMode> { let Uses = [IY]; let Defs = [A]; let mayLoad = 1; let mayStore = 0; let hasSideEffects = 0; }

//===----------------------------------------------------------------------===//
// Indexed addressing: LD (IX+d),r / LD (IY+d),r
// Opcode: DD [70|71|72|73|74|75|77] d for IX
// Opcode: FD [70|71|72|73|74|75|77] d for IY
//===----------------------------------------------------------------------===//

// LD (IX+d),r — IX is the address register (Use); r is the data (Use).
// IX must be in Uses for the same reason as LD_r_IXd above (#243).
def LD_IXd_B : InstDD16<"ld", 0x70, IndexedIXMode> { let Uses = [B, IX]; let AsmString = "ld ( ix + $imm ), b"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IXd_C : InstDD16<"ld", 0x71, IndexedIXMode> { let Uses = [C, IX]; let AsmString = "ld ( ix + $imm ), c"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IXd_D : InstDD16<"ld", 0x72, IndexedIXMode> { let Uses = [D, IX]; let AsmString = "ld ( ix + $imm ), d"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IXd_E : InstDD16<"ld", 0x73, IndexedIXMode> { let Uses = [E, IX]; let AsmString = "ld ( ix + $imm ), e"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IXd_H : InstDD16<"ld", 0x74, IndexedIXMode> { let Uses = [H, IX]; let AsmString = "ld ( ix + $imm ), h"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IXd_L : InstDD16<"ld", 0x75, IndexedIXMode> { let Uses = [L, IX]; let AsmString = "ld ( ix + $imm ), l"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IXd_A : InstDD16<"ld", 0x77, IndexedIXMode> { let Uses = [A, IX]; let AsmString = "ld ( ix + $imm ), a"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }

// LD (IY+d),r — IY is the address register (Use); r is the data (Use).
def LD_IYd_B : InstFD16<"ld", 0x70, IndexedIYMode> { let Uses = [B, IY]; let AsmString = "ld ( iy + $imm ), b"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IYd_C : InstFD16<"ld", 0x71, IndexedIYMode> { let Uses = [C, IY]; let AsmString = "ld ( iy + $imm ), c"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IYd_D : InstFD16<"ld", 0x72, IndexedIYMode> { let Uses = [D, IY]; let AsmString = "ld ( iy + $imm ), d"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IYd_E : InstFD16<"ld", 0x73, IndexedIYMode> { let Uses = [E, IY]; let AsmString = "ld ( iy + $imm ), e"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IYd_H : InstFD16<"ld", 0x74, IndexedIYMode> { let Uses = [H, IY]; let AsmString = "ld ( iy + $imm ), h"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IYd_L : InstFD16<"ld", 0x75, IndexedIYMode> { let Uses = [L, IY]; let AsmString = "ld ( iy + $imm ), l"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IYd_A : InstFD16<"ld", 0x77, IndexedIYMode> { let Uses = [A, IY]; let AsmString = "ld ( iy + $imm ), a"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }

//===----------------------------------------------------------------------===//
// Indexed addressing: LD (IX+d),n / LD (IY+d),n - Store immediate to indexed
// Opcode: DD 36 d n for IX
// Opcode: FD 36 d n for IY
//===----------------------------------------------------------------------===//

// LD (IX+d),n / LD (IY+d),n — IX/IY is the address register (Use) (#243).
def LD_IXd_n : InstDD16Imm<"ld", 0x36> { let Uses = [IX]; let AsmString = "ld ( ix + $disp ), $val"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }
def LD_IYd_n : InstFD16Imm<"ld", 0x36> { let Uses = [IY]; let AsmString = "ld ( iy + $disp ), $val"; let mayLoad = 0; let mayStore = 1; let hasSideEffects = 0; }

//===----------------------------------------------------------------------===//
//  IX-indexed ALU instructions (DD prefix)
//===----------------------------------------------------------------------===//

let mayLoad = 1, mayStore = 0, hasSideEffects = 0 in {
  def ADD_A_IXd : InstDD16<"add a,", 0x86, IndexedIXMode> {
    let Uses = [A, IX]; let Defs = [A, FLAGS];
  }
  def ADC_A_IXd : InstDD16<"adc a,", 0x8E, IndexedIXMode> {
    let Uses = [A, IX, FLAGS]; let Defs = [A, FLAGS];
  }
  def SUB_IXd : InstDD16<"sub", 0x96, IndexedIXMode> {
    let AsmString = "sub ( ix + $imm )";
    let Uses = [A, IX]; let Defs = [A, FLAGS];
  }
  def SBC_A_IXd : InstDD16<"sbc a,", 0x9E, IndexedIXMode> {
    let Uses = [A, IX, FLAGS]; let Defs = [A, FLAGS];
  }
  def AND_IXd : InstDD16<"and", 0xA6, IndexedIXMode> {
    let AsmString = "and ( ix + $imm )";
    let Uses = [A, IX]; let Defs = [A, FLAGS];
  }
  def XOR_IXd : InstDD16<"xor", 0xAE, IndexedIXMode> {
    let AsmString = "xor ( ix + $imm )";
    let Uses = [A, IX]; let Defs = [A, FLAGS];
  }
  def OR_IXd : InstDD16<"or", 0xB6, IndexedIXMode> {
    let AsmString = "or ( ix + $imm )";
    let Uses = [A, IX]; let Defs = [A, FLAGS];
  }
  def CP_IXd : InstDD16<"cp", 0xBE, IndexedIXMode> {
    let AsmString = "cp ( ix + $imm )";
    let Uses = [A, IX]; let Defs = [FLAGS];
  }
}

//===----------------------------------------------------------------------===//
//  IY-indexed ALU instructions (FD prefix)
//===----------------------------------------------------------------------===//

let mayLoad = 1, mayStore = 0, hasSideEffects = 0 in {
  def ADD_A_IYd : InstFD16<"add a,", 0x86, IndexedIYMode> {
    let Uses = [A, IY]; let Defs = [A, FLAGS];
  }
  def ADC_A_IYd : InstFD16<"adc a,", 0x8E, IndexedIYMode> {
    let Uses = [A, IY, FLAGS]; let Defs = [A, FLAGS];
  }
  def SUB_IYd : InstFD16<"sub", 0x96, IndexedIYMode> {
    let AsmString = "sub ( iy + $imm )";
    let Uses = [A, IY]; let Defs = [A, FLAGS];
  }
  def SBC_A_IYd : InstFD16<"sbc a,", 0x9E, IndexedIYMode> {
    let Uses = [A, IY, FLAGS]; let Defs = [A, FLAGS];
  }
  def AND_IYd : InstFD16<"and", 0xA6, IndexedIYMode> {
    let AsmString = "and ( iy + $imm )";
    let Uses = [A, IY]; let Defs = [A, FLAGS];
  }
  def XOR_IYd : InstFD16<"xor", 0xAE, IndexedIYMode> {
    let AsmString = "xor ( iy + $imm )";
    let Uses = [A, IY]; let Defs = [A, FLAGS];
  }
  def OR_IYd : InstFD16<"or", 0xB6, IndexedIYMode> {
    let AsmString = "or ( iy + $imm )";
    let Uses = [A, IY]; let Defs = [A, FLAGS];
  }
  def CP_IYd : InstFD16<"cp", 0xBE, IndexedIYMode> {
    let AsmString = "cp ( iy + $imm )";
    let Uses = [A, IY]; let Defs = [FLAGS];
  }
}

//===----------------------------------------------------------------------===//
//  IX/IY-indexed INC/DEC (DD/FD prefix)
//===----------------------------------------------------------------------===//

let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def INC_IXd : InstDD16<"inc", 0x34, IndexedIXMode> {
    let AsmString = "inc ( ix + $imm )";
    let Uses = [IX]; let Defs = [FLAGS];
  }
  def DEC_IXd : InstDD16<"dec", 0x35, IndexedIXMode> {
    let AsmString = "dec ( ix + $imm )";
    let Uses = [IX]; let Defs = [FLAGS];
  }
  def INC_IYd : InstFD16<"inc", 0x34, IndexedIYMode> {
    let AsmString = "inc ( iy + $imm )";
    let Uses = [IY]; let Defs = [FLAGS];
  }
  def DEC_IYd : InstFD16<"dec", 0x35, IndexedIYMode> {
    let AsmString = "dec ( iy + $imm )";
    let Uses = [IY]; let Defs = [FLAGS];
  }
}

//===----------------------------------------------------------------------===//
//  DDCB/FDCB indexed rotate/shift instructions (4-byte)
//===----------------------------------------------------------------------===//

// Rotate/shift (IX+d) — read and write memory
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def RLC_IXd : InstDDCB<"rlc", 0x06> { let Uses = [IX]; let Defs = [FLAGS]; }
  def RRC_IXd : InstDDCB<"rrc", 0x0E> { let Uses = [IX]; let Defs = [FLAGS]; }
  def RL_IXd  : InstDDCB<"rl",  0x16> { let Uses = [IX, FLAGS]; let Defs = [FLAGS]; }
  def RR_IXd  : InstDDCB<"rr",  0x1E> { let Uses = [IX, FLAGS]; let Defs = [FLAGS]; }
  def SLA_IXd : InstDDCB<"sla", 0x26> { let Uses = [IX]; let Defs = [FLAGS]; }
  def SRA_IXd : InstDDCB<"sra", 0x2E> { let Uses = [IX]; let Defs = [FLAGS]; }
  def SRL_IXd : InstDDCB<"srl", 0x3E> { let Uses = [IX]; let Defs = [FLAGS]; }
}

// Rotate/shift (IY+d) — read and write memory
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def RLC_IYd : InstFDCB<"rlc", 0x06> { let Uses = [IY]; let Defs = [FLAGS]; }
  def RRC_IYd : InstFDCB<"rrc", 0x0E> { let Uses = [IY]; let Defs = [FLAGS]; }
  def RL_IYd  : InstFDCB<"rl",  0x16> { let Uses = [IY, FLAGS]; let Defs = [FLAGS]; }
  def RR_IYd  : InstFDCB<"rr",  0x1E> { let Uses = [IY, FLAGS]; let Defs = [FLAGS]; }
  def SLA_IYd : InstFDCB<"sla", 0x26> { let Uses = [IY]; let Defs = [FLAGS]; }
  def SRA_IYd : InstFDCB<"sra", 0x2E> { let Uses = [IY]; let Defs = [FLAGS]; }
  def SRL_IYd : InstFDCB<"srl", 0x3E> { let Uses = [IY]; let Defs = [FLAGS]; }
}

//===----------------------------------------------------------------------===//
//  DDCB/FDCB indexed BIT/SET/RES instructions (4-byte)
//===----------------------------------------------------------------------===//

// BIT b,(IX+d) — read memory only
let mayLoad = 1, mayStore = 0, hasSideEffects = 0 in {
  def BIT_0_IXd : InstDDCB<"bit 0,", 0x46> { let Uses = [IX]; let Defs = [FLAGS]; }
  def BIT_1_IXd : InstDDCB<"bit 1,", 0x4E> { let Uses = [IX]; let Defs = [FLAGS]; }
  def BIT_2_IXd : InstDDCB<"bit 2,", 0x56> { let Uses = [IX]; let Defs = [FLAGS]; }
  def BIT_3_IXd : InstDDCB<"bit 3,", 0x5E> { let Uses = [IX]; let Defs = [FLAGS]; }
  def BIT_4_IXd : InstDDCB<"bit 4,", 0x66> { let Uses = [IX]; let Defs = [FLAGS]; }
  def BIT_5_IXd : InstDDCB<"bit 5,", 0x6E> { let Uses = [IX]; let Defs = [FLAGS]; }
  def BIT_6_IXd : InstDDCB<"bit 6,", 0x76> { let Uses = [IX]; let Defs = [FLAGS]; }
  def BIT_7_IXd : InstDDCB<"bit 7,", 0x7E> { let Uses = [IX]; let Defs = [FLAGS]; }
}

// BIT b,(IY+d) — read memory only
let mayLoad = 1, mayStore = 0, hasSideEffects = 0 in {
  def BIT_0_IYd : InstFDCB<"bit 0,", 0x46> { let Uses = [IY]; let Defs = [FLAGS]; }
  def BIT_1_IYd : InstFDCB<"bit 1,", 0x4E> { let Uses = [IY]; let Defs = [FLAGS]; }
  def BIT_2_IYd : InstFDCB<"bit 2,", 0x56> { let Uses = [IY]; let Defs = [FLAGS]; }
  def BIT_3_IYd : InstFDCB<"bit 3,", 0x5E> { let Uses = [IY]; let Defs = [FLAGS]; }
  def BIT_4_IYd : InstFDCB<"bit 4,", 0x66> { let Uses = [IY]; let Defs = [FLAGS]; }
  def BIT_5_IYd : InstFDCB<"bit 5,", 0x6E> { let Uses = [IY]; let Defs = [FLAGS]; }
  def BIT_6_IYd : InstFDCB<"bit 6,", 0x76> { let Uses = [IY]; let Defs = [FLAGS]; }
  def BIT_7_IYd : InstFDCB<"bit 7,", 0x7E> { let Uses = [IY]; let Defs = [FLAGS]; }
}

// RES b,(IX+d) — read and write memory
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def RES_0_IXd : InstDDCB<"res 0,", 0x86> { let Uses = [IX]; }
  def RES_1_IXd : InstDDCB<"res 1,", 0x8E> { let Uses = [IX]; }
  def RES_2_IXd : InstDDCB<"res 2,", 0x96> { let Uses = [IX]; }
  def RES_3_IXd : InstDDCB<"res 3,", 0x9E> { let Uses = [IX]; }
  def RES_4_IXd : InstDDCB<"res 4,", 0xA6> { let Uses = [IX]; }
  def RES_5_IXd : InstDDCB<"res 5,", 0xAE> { let Uses = [IX]; }
  def RES_6_IXd : InstDDCB<"res 6,", 0xB6> { let Uses = [IX]; }
  def RES_7_IXd : InstDDCB<"res 7,", 0xBE> { let Uses = [IX]; }
}

// RES b,(IY+d) — read and write memory
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def RES_0_IYd : InstFDCB<"res 0,", 0x86> { let Uses = [IY]; }
  def RES_1_IYd : InstFDCB<"res 1,", 0x8E> { let Uses = [IY]; }
  def RES_2_IYd : InstFDCB<"res 2,", 0x96> { let Uses = [IY]; }
  def RES_3_IYd : InstFDCB<"res 3,", 0x9E> { let Uses = [IY]; }
  def RES_4_IYd : InstFDCB<"res 4,", 0xA6> { let Uses = [IY]; }
  def RES_5_IYd : InstFDCB<"res 5,", 0xAE> { let Uses = [IY]; }
  def RES_6_IYd : InstFDCB<"res 6,", 0xB6> { let Uses = [IY]; }
  def RES_7_IYd : InstFDCB<"res 7,", 0xBE> { let Uses = [IY]; }
}

// SET b,(IX+d) — read and write memory
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def SET_0_IXd : InstDDCB<"set 0,", 0xC6> { let Uses = [IX]; }
  def SET_1_IXd : InstDDCB<"set 1,", 0xCE> { let Uses = [IX]; }
  def SET_2_IXd : InstDDCB<"set 2,", 0xD6> { let Uses = [IX]; }
  def SET_3_IXd : InstDDCB<"set 3,", 0xDE> { let Uses = [IX]; }
  def SET_4_IXd : InstDDCB<"set 4,", 0xE6> { let Uses = [IX]; }
  def SET_5_IXd : InstDDCB<"set 5,", 0xEE> { let Uses = [IX]; }
  def SET_6_IXd : InstDDCB<"set 6,", 0xF6> { let Uses = [IX]; }
  def SET_7_IXd : InstDDCB<"set 7,", 0xFE> { let Uses = [IX]; }
}

// SET b,(IY+d) — read and write memory
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def SET_0_IYd : InstFDCB<"set 0,", 0xC6> { let Uses = [IY]; }
  def SET_1_IYd : InstFDCB<"set 1,", 0xCE> { let Uses = [IY]; }
  def SET_2_IYd : InstFDCB<"set 2,", 0xD6> { let Uses = [IY]; }
  def SET_3_IYd : InstFDCB<"set 3,", 0xDE> { let Uses = [IY]; }
  def SET_4_IYd : InstFDCB<"set 4,", 0xE6> { let Uses = [IY]; }
  def SET_5_IYd : InstFDCB<"set 5,", 0xEE> { let Uses = [IY]; }
  def SET_6_IYd : InstFDCB<"set 6,", 0xF6> { let Uses = [IY]; }
  def SET_7_IYd : InstFDCB<"set 7,", 0xFE> { let Uses = [IY]; }
}

} // Predicates = [HasZ80]

//===----------------------------------------------------------------------===//
//  Undocumented instructions (Z80 NMOS/CMOS only, NOT R800/eZ80)
//===----------------------------------------------------------------------===//
// Gated by FeatureUndocumented. These are real opcodes that work on all NMOS
// and CMOS Z80 variants but are not officially documented by Zilog.

//===----------------------------------------------------------------------===//
//  SLL r (Shift Left Logical / "SL1") — CB 30-37
//  Undocumented. Like SLA but shifts 1 into bit 0 instead of 0.
//  result = (operand << 1) | 1, bit 7 → carry.
//  On SM83, this opcode slot (CB 30-37) is the documented SWAP instruction.
//===----------------------------------------------------------------------===//

let Predicates = [HasZ80, HasUndocumented] in {

def SLL_B : CBRotate<0b110, REG_B, "sll"> { let Defs = [B, FLAGS]; let Uses = [B]; }
def SLL_C : CBRotate<0b110, REG_C, "sll"> { let Defs = [C, FLAGS]; let Uses = [C]; }
def SLL_D : CBRotate<0b110, REG_D, "sll"> { let Defs = [D, FLAGS]; let Uses = [D]; }
def SLL_E : CBRotate<0b110, REG_E, "sll"> { let Defs = [E, FLAGS]; let Uses = [E]; }
def SLL_H : CBRotate<0b110, REG_H, "sll"> { let Defs = [H, FLAGS]; let Uses = [H]; }
def SLL_L : CBRotate<0b110, REG_L, "sll"> { let Defs = [L, FLAGS]; let Uses = [L]; }
let mayLoad = true, mayStore = true, hasSideEffects = false in
def SLL_HLind : InstCB<"sll (hl)", 0x36> { let Uses = [HL]; let Defs = [FLAGS]; }
def SLL_A : CBRotate<0b110, REG_A, "sll"> { let Defs = [A, FLAGS]; let Uses = [A]; }

//===----------------------------------------------------------------------===//
//  SLL (IX+d) / SLL (IY+d) — DD CB d 36 / FD CB d 36
//  Undocumented indexed shift-left-logical.
//===----------------------------------------------------------------------===//

let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def SLL_IXd : InstDDCB<"sll", 0x36> { let Uses = [IX]; let Defs = [FLAGS]; }
  def SLL_IYd : InstFDCB<"sll", 0x36> { let Uses = [IY]; let Defs = [FLAGS]; }
}

//===----------------------------------------------------------------------===//
//  IN (C) — ED 70 — Input and discard, set flags only
//  Reads port (C), discards value, sets flags based on input.
//===----------------------------------------------------------------------===//

def IN_F_C : InstED8<"in (c)", 0x70> { let Uses = [C]; let Defs = [FLAGS]; }

//===----------------------------------------------------------------------===//
//  OUT (C),0 — ED 71 — Output zero to port
//  NMOS Z80 outputs 0x00, CMOS Z80 outputs 0xFF. Platform-dependent!
//===----------------------------------------------------------------------===//

def OUT_C_0 : InstED8<"out (c),0", 0x71> { let Uses = [C]; }

//===----------------------------------------------------------------------===//
//  DDCB/FDCB undocumented rotate/shift with register copy
//  These perform the indexed operation AND copy the result to a register.
//  Example: RLC (IX+d),B = DD CB d 00 — rotates (IX+d) left and copies to B.
//  The "normal" form (register field = 110 = (HL)) is the documented version.
//  All other register fields (000-101, 111) are undocumented variants.
//===----------------------------------------------------------------------===//

// Helper: indexed rotate/shift with register copy
// These 4-byte instructions do: tmp = shift/rotate(mem[IX/IY+d]); mem = tmp; reg = tmp

// --- RLC (IX+d),r ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def RLC_IXd_B : InstDDCB<"rlc", 0x00> { let Uses = [IX]; let Defs = [B, FLAGS]; let AsmString = "rlc ( ix + $imm ), b"; }
  def RLC_IXd_C : InstDDCB<"rlc", 0x01> { let Uses = [IX]; let Defs = [C, FLAGS]; let AsmString = "rlc ( ix + $imm ), c"; }
  def RLC_IXd_D : InstDDCB<"rlc", 0x02> { let Uses = [IX]; let Defs = [D, FLAGS]; let AsmString = "rlc ( ix + $imm ), d"; }
  def RLC_IXd_E : InstDDCB<"rlc", 0x03> { let Uses = [IX]; let Defs = [E, FLAGS]; let AsmString = "rlc ( ix + $imm ), e"; }
  def RLC_IXd_H : InstDDCB<"rlc", 0x04> { let Uses = [IX]; let Defs = [H, FLAGS]; let AsmString = "rlc ( ix + $imm ), h"; }
  def RLC_IXd_L : InstDDCB<"rlc", 0x05> { let Uses = [IX]; let Defs = [L, FLAGS]; let AsmString = "rlc ( ix + $imm ), l"; }
  def RLC_IXd_A : InstDDCB<"rlc", 0x07> { let Uses = [IX]; let Defs = [A, FLAGS]; let AsmString = "rlc ( ix + $imm ), a"; }
}

// --- RRC (IX+d),r ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def RRC_IXd_B : InstDDCB<"rrc", 0x08> { let Uses = [IX]; let Defs = [B, FLAGS]; let AsmString = "rrc ( ix + $imm ), b"; }
  def RRC_IXd_C : InstDDCB<"rrc", 0x09> { let Uses = [IX]; let Defs = [C, FLAGS]; let AsmString = "rrc ( ix + $imm ), c"; }
  def RRC_IXd_D : InstDDCB<"rrc", 0x0A> { let Uses = [IX]; let Defs = [D, FLAGS]; let AsmString = "rrc ( ix + $imm ), d"; }
  def RRC_IXd_E : InstDDCB<"rrc", 0x0B> { let Uses = [IX]; let Defs = [E, FLAGS]; let AsmString = "rrc ( ix + $imm ), e"; }
  def RRC_IXd_H : InstDDCB<"rrc", 0x0C> { let Uses = [IX]; let Defs = [H, FLAGS]; let AsmString = "rrc ( ix + $imm ), h"; }
  def RRC_IXd_L : InstDDCB<"rrc", 0x0D> { let Uses = [IX]; let Defs = [L, FLAGS]; let AsmString = "rrc ( ix + $imm ), l"; }
  def RRC_IXd_A : InstDDCB<"rrc", 0x0F> { let Uses = [IX]; let Defs = [A, FLAGS]; let AsmString = "rrc ( ix + $imm ), a"; }
}

// --- RL (IX+d),r ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def RL_IXd_B : InstDDCB<"rl", 0x10> { let Uses = [IX, FLAGS]; let Defs = [B, FLAGS]; let AsmString = "rl ( ix + $imm ), b"; }
  def RL_IXd_C : InstDDCB<"rl", 0x11> { let Uses = [IX, FLAGS]; let Defs = [C, FLAGS]; let AsmString = "rl ( ix + $imm ), c"; }
  def RL_IXd_D : InstDDCB<"rl", 0x12> { let Uses = [IX, FLAGS]; let Defs = [D, FLAGS]; let AsmString = "rl ( ix + $imm ), d"; }
  def RL_IXd_E : InstDDCB<"rl", 0x13> { let Uses = [IX, FLAGS]; let Defs = [E, FLAGS]; let AsmString = "rl ( ix + $imm ), e"; }
  def RL_IXd_H : InstDDCB<"rl", 0x14> { let Uses = [IX, FLAGS]; let Defs = [H, FLAGS]; let AsmString = "rl ( ix + $imm ), h"; }
  def RL_IXd_L : InstDDCB<"rl", 0x15> { let Uses = [IX, FLAGS]; let Defs = [L, FLAGS]; let AsmString = "rl ( ix + $imm ), l"; }
  def RL_IXd_A : InstDDCB<"rl", 0x17> { let Uses = [IX, FLAGS]; let Defs = [A, FLAGS]; let AsmString = "rl ( ix + $imm ), a"; }
}

// --- RR (IX+d),r ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def RR_IXd_B : InstDDCB<"rr", 0x18> { let Uses = [IX, FLAGS]; let Defs = [B, FLAGS]; let AsmString = "rr ( ix + $imm ), b"; }
  def RR_IXd_C : InstDDCB<"rr", 0x19> { let Uses = [IX, FLAGS]; let Defs = [C, FLAGS]; let AsmString = "rr ( ix + $imm ), c"; }
  def RR_IXd_D : InstDDCB<"rr", 0x1A> { let Uses = [IX, FLAGS]; let Defs = [D, FLAGS]; let AsmString = "rr ( ix + $imm ), d"; }
  def RR_IXd_E : InstDDCB<"rr", 0x1B> { let Uses = [IX, FLAGS]; let Defs = [E, FLAGS]; let AsmString = "rr ( ix + $imm ), e"; }
  def RR_IXd_H : InstDDCB<"rr", 0x1C> { let Uses = [IX, FLAGS]; let Defs = [H, FLAGS]; let AsmString = "rr ( ix + $imm ), h"; }
  def RR_IXd_L : InstDDCB<"rr", 0x1D> { let Uses = [IX, FLAGS]; let Defs = [L, FLAGS]; let AsmString = "rr ( ix + $imm ), l"; }
  def RR_IXd_A : InstDDCB<"rr", 0x1F> { let Uses = [IX, FLAGS]; let Defs = [A, FLAGS]; let AsmString = "rr ( ix + $imm ), a"; }
}

// --- SLA (IX+d),r ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def SLA_IXd_B : InstDDCB<"sla", 0x20> { let Uses = [IX]; let Defs = [B, FLAGS]; let AsmString = "sla ( ix + $imm ), b"; }
  def SLA_IXd_C : InstDDCB<"sla", 0x21> { let Uses = [IX]; let Defs = [C, FLAGS]; let AsmString = "sla ( ix + $imm ), c"; }
  def SLA_IXd_D : InstDDCB<"sla", 0x22> { let Uses = [IX]; let Defs = [D, FLAGS]; let AsmString = "sla ( ix + $imm ), d"; }
  def SLA_IXd_E : InstDDCB<"sla", 0x23> { let Uses = [IX]; let Defs = [E, FLAGS]; let AsmString = "sla ( ix + $imm ), e"; }
  def SLA_IXd_H : InstDDCB<"sla", 0x24> { let Uses = [IX]; let Defs = [H, FLAGS]; let AsmString = "sla ( ix + $imm ), h"; }
  def SLA_IXd_L : InstDDCB<"sla", 0x25> { let Uses = [IX]; let Defs = [L, FLAGS]; let AsmString = "sla ( ix + $imm ), l"; }
  def SLA_IXd_A : InstDDCB<"sla", 0x27> { let Uses = [IX]; let Defs = [A, FLAGS]; let AsmString = "sla ( ix + $imm ), a"; }
}

// --- SRA (IX+d),r ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def SRA_IXd_B : InstDDCB<"sra", 0x28> { let Uses = [IX]; let Defs = [B, FLAGS]; let AsmString = "sra ( ix + $imm ), b"; }
  def SRA_IXd_C : InstDDCB<"sra", 0x29> { let Uses = [IX]; let Defs = [C, FLAGS]; let AsmString = "sra ( ix + $imm ), c"; }
  def SRA_IXd_D : InstDDCB<"sra", 0x2A> { let Uses = [IX]; let Defs = [D, FLAGS]; let AsmString = "sra ( ix + $imm ), d"; }
  def SRA_IXd_E : InstDDCB<"sra", 0x2B> { let Uses = [IX]; let Defs = [E, FLAGS]; let AsmString = "sra ( ix + $imm ), e"; }
  def SRA_IXd_H : InstDDCB<"sra", 0x2C> { let Uses = [IX]; let Defs = [H, FLAGS]; let AsmString = "sra ( ix + $imm ), h"; }
  def SRA_IXd_L : InstDDCB<"sra", 0x2D> { let Uses = [IX]; let Defs = [L, FLAGS]; let AsmString = "sra ( ix + $imm ), l"; }
  def SRA_IXd_A : InstDDCB<"sra", 0x2F> { let Uses = [IX]; let Defs = [A, FLAGS]; let AsmString = "sra ( ix + $imm ), a"; }
}

// --- SLL (IX+d),r (undocumented shift + undocumented register copy) ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def SLL_IXd_B : InstDDCB<"sll", 0x30> { let Uses = [IX]; let Defs = [B, FLAGS]; let AsmString = "sll ( ix + $imm ), b"; }
  def SLL_IXd_C : InstDDCB<"sll", 0x31> { let Uses = [IX]; let Defs = [C, FLAGS]; let AsmString = "sll ( ix + $imm ), c"; }
  def SLL_IXd_D : InstDDCB<"sll", 0x32> { let Uses = [IX]; let Defs = [D, FLAGS]; let AsmString = "sll ( ix + $imm ), d"; }
  def SLL_IXd_E : InstDDCB<"sll", 0x33> { let Uses = [IX]; let Defs = [E, FLAGS]; let AsmString = "sll ( ix + $imm ), e"; }
  def SLL_IXd_H : InstDDCB<"sll", 0x34> { let Uses = [IX]; let Defs = [H, FLAGS]; let AsmString = "sll ( ix + $imm ), h"; }
  def SLL_IXd_L : InstDDCB<"sll", 0x35> { let Uses = [IX]; let Defs = [L, FLAGS]; let AsmString = "sll ( ix + $imm ), l"; }
  def SLL_IXd_A : InstDDCB<"sll", 0x37> { let Uses = [IX]; let Defs = [A, FLAGS]; let AsmString = "sll ( ix + $imm ), a"; }
}

// --- SRL (IX+d),r ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def SRL_IXd_B : InstDDCB<"srl", 0x38> { let Uses = [IX]; let Defs = [B, FLAGS]; let AsmString = "srl ( ix + $imm ), b"; }
  def SRL_IXd_C : InstDDCB<"srl", 0x39> { let Uses = [IX]; let Defs = [C, FLAGS]; let AsmString = "srl ( ix + $imm ), c"; }
  def SRL_IXd_D : InstDDCB<"srl", 0x3A> { let Uses = [IX]; let Defs = [D, FLAGS]; let AsmString = "srl ( ix + $imm ), d"; }
  def SRL_IXd_E : InstDDCB<"srl", 0x3B> { let Uses = [IX]; let Defs = [E, FLAGS]; let AsmString = "srl ( ix + $imm ), e"; }
  def SRL_IXd_H : InstDDCB<"srl", 0x3C> { let Uses = [IX]; let Defs = [H, FLAGS]; let AsmString = "srl ( ix + $imm ), h"; }
  def SRL_IXd_L : InstDDCB<"srl", 0x3D> { let Uses = [IX]; let Defs = [L, FLAGS]; let AsmString = "srl ( ix + $imm ), l"; }
  def SRL_IXd_A : InstDDCB<"srl", 0x3F> { let Uses = [IX]; let Defs = [A, FLAGS]; let AsmString = "srl ( ix + $imm ), a"; }
}

// --- RES b,(IX+d),r (indexed bit reset with register copy) ---
// The documented RES b,(IX+d) uses register field 110 (already defined above).
// These undocumented variants also copy the result to the specified register.
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  // RES 0,(IX+d),r
  def RES_0_IXd_B : InstDDCB<"res 0,", 0x80> { let Uses = [IX]; let Defs = [B]; let AsmString = "res 0, ( ix + $imm ), b"; }
  def RES_0_IXd_C : InstDDCB<"res 0,", 0x81> { let Uses = [IX]; let Defs = [C]; let AsmString = "res 0, ( ix + $imm ), c"; }
  def RES_0_IXd_D : InstDDCB<"res 0,", 0x82> { let Uses = [IX]; let Defs = [D]; let AsmString = "res 0, ( ix + $imm ), d"; }
  def RES_0_IXd_E : InstDDCB<"res 0,", 0x83> { let Uses = [IX]; let Defs = [E]; let AsmString = "res 0, ( ix + $imm ), e"; }
  def RES_0_IXd_H : InstDDCB<"res 0,", 0x84> { let Uses = [IX]; let Defs = [H]; let AsmString = "res 0, ( ix + $imm ), h"; }
  def RES_0_IXd_L : InstDDCB<"res 0,", 0x85> { let Uses = [IX]; let Defs = [L]; let AsmString = "res 0, ( ix + $imm ), l"; }
  def RES_0_IXd_A : InstDDCB<"res 0,", 0x87> { let Uses = [IX]; let Defs = [A]; let AsmString = "res 0, ( ix + $imm ), a"; }
  // RES 1,(IX+d),r
  def RES_1_IXd_B : InstDDCB<"res 1,", 0x88> { let Uses = [IX]; let Defs = [B]; let AsmString = "res 1, ( ix + $imm ), b"; }
  def RES_1_IXd_C : InstDDCB<"res 1,", 0x89> { let Uses = [IX]; let Defs = [C]; let AsmString = "res 1, ( ix + $imm ), c"; }
  def RES_1_IXd_D : InstDDCB<"res 1,", 0x8A> { let Uses = [IX]; let Defs = [D]; let AsmString = "res 1, ( ix + $imm ), d"; }
  def RES_1_IXd_E : InstDDCB<"res 1,", 0x8B> { let Uses = [IX]; let Defs = [E]; let AsmString = "res 1, ( ix + $imm ), e"; }
  def RES_1_IXd_H : InstDDCB<"res 1,", 0x8C> { let Uses = [IX]; let Defs = [H]; let AsmString = "res 1, ( ix + $imm ), h"; }
  def RES_1_IXd_L : InstDDCB<"res 1,", 0x8D> { let Uses = [IX]; let Defs = [L]; let AsmString = "res 1, ( ix + $imm ), l"; }
  def RES_1_IXd_A : InstDDCB<"res 1,", 0x8F> { let Uses = [IX]; let Defs = [A]; let AsmString = "res 1, ( ix + $imm ), a"; }
  // RES 2,(IX+d),r
  def RES_2_IXd_B : InstDDCB<"res 2,", 0x90> { let Uses = [IX]; let Defs = [B]; let AsmString = "res 2, ( ix + $imm ), b"; }
  def RES_2_IXd_C : InstDDCB<"res 2,", 0x91> { let Uses = [IX]; let Defs = [C]; let AsmString = "res 2, ( ix + $imm ), c"; }
  def RES_2_IXd_D : InstDDCB<"res 2,", 0x92> { let Uses = [IX]; let Defs = [D]; let AsmString = "res 2, ( ix + $imm ), d"; }
  def RES_2_IXd_E : InstDDCB<"res 2,", 0x93> { let Uses = [IX]; let Defs = [E]; let AsmString = "res 2, ( ix + $imm ), e"; }
  def RES_2_IXd_H : InstDDCB<"res 2,", 0x94> { let Uses = [IX]; let Defs = [H]; let AsmString = "res 2, ( ix + $imm ), h"; }
  def RES_2_IXd_L : InstDDCB<"res 2,", 0x95> { let Uses = [IX]; let Defs = [L]; let AsmString = "res 2, ( ix + $imm ), l"; }
  def RES_2_IXd_A : InstDDCB<"res 2,", 0x97> { let Uses = [IX]; let Defs = [A]; let AsmString = "res 2, ( ix + $imm ), a"; }
  // RES 3,(IX+d),r
  def RES_3_IXd_B : InstDDCB<"res 3,", 0x98> { let Uses = [IX]; let Defs = [B]; let AsmString = "res 3, ( ix + $imm ), b"; }
  def RES_3_IXd_C : InstDDCB<"res 3,", 0x99> { let Uses = [IX]; let Defs = [C]; let AsmString = "res 3, ( ix + $imm ), c"; }
  def RES_3_IXd_D : InstDDCB<"res 3,", 0x9A> { let Uses = [IX]; let Defs = [D]; let AsmString = "res 3, ( ix + $imm ), d"; }
  def RES_3_IXd_E : InstDDCB<"res 3,", 0x9B> { let Uses = [IX]; let Defs = [E]; let AsmString = "res 3, ( ix + $imm ), e"; }
  def RES_3_IXd_H : InstDDCB<"res 3,", 0x9C> { let Uses = [IX]; let Defs = [H]; let AsmString = "res 3, ( ix + $imm ), h"; }
  def RES_3_IXd_L : InstDDCB<"res 3,", 0x9D> { let Uses = [IX]; let Defs = [L]; let AsmString = "res 3, ( ix + $imm ), l"; }
  def RES_3_IXd_A : InstDDCB<"res 3,", 0x9F> { let Uses = [IX]; let Defs = [A]; let AsmString = "res 3, ( ix + $imm ), a"; }
  // RES 4,(IX+d),r
  def RES_4_IXd_B : InstDDCB<"res 4,", 0xA0> { let Uses = [IX]; let Defs = [B]; let AsmString = "res 4, ( ix + $imm ), b"; }
  def RES_4_IXd_C : InstDDCB<"res 4,", 0xA1> { let Uses = [IX]; let Defs = [C]; let AsmString = "res 4, ( ix + $imm ), c"; }
  def RES_4_IXd_D : InstDDCB<"res 4,", 0xA2> { let Uses = [IX]; let Defs = [D]; let AsmString = "res 4, ( ix + $imm ), d"; }
  def RES_4_IXd_E : InstDDCB<"res 4,", 0xA3> { let Uses = [IX]; let Defs = [E]; let AsmString = "res 4, ( ix + $imm ), e"; }
  def RES_4_IXd_H : InstDDCB<"res 4,", 0xA4> { let Uses = [IX]; let Defs = [H]; let AsmString = "res 4, ( ix + $imm ), h"; }
  def RES_4_IXd_L : InstDDCB<"res 4,", 0xA5> { let Uses = [IX]; let Defs = [L]; let AsmString = "res 4, ( ix + $imm ), l"; }
  def RES_4_IXd_A : InstDDCB<"res 4,", 0xA7> { let Uses = [IX]; let Defs = [A]; let AsmString = "res 4, ( ix + $imm ), a"; }
  // RES 5,(IX+d),r
  def RES_5_IXd_B : InstDDCB<"res 5,", 0xA8> { let Uses = [IX]; let Defs = [B]; let AsmString = "res 5, ( ix + $imm ), b"; }
  def RES_5_IXd_C : InstDDCB<"res 5,", 0xA9> { let Uses = [IX]; let Defs = [C]; let AsmString = "res 5, ( ix + $imm ), c"; }
  def RES_5_IXd_D : InstDDCB<"res 5,", 0xAA> { let Uses = [IX]; let Defs = [D]; let AsmString = "res 5, ( ix + $imm ), d"; }
  def RES_5_IXd_E : InstDDCB<"res 5,", 0xAB> { let Uses = [IX]; let Defs = [E]; let AsmString = "res 5, ( ix + $imm ), e"; }
  def RES_5_IXd_H : InstDDCB<"res 5,", 0xAC> { let Uses = [IX]; let Defs = [H]; let AsmString = "res 5, ( ix + $imm ), h"; }
  def RES_5_IXd_L : InstDDCB<"res 5,", 0xAD> { let Uses = [IX]; let Defs = [L]; let AsmString = "res 5, ( ix + $imm ), l"; }
  def RES_5_IXd_A : InstDDCB<"res 5,", 0xAF> { let Uses = [IX]; let Defs = [A]; let AsmString = "res 5, ( ix + $imm ), a"; }
  // RES 6,(IX+d),r
  def RES_6_IXd_B : InstDDCB<"res 6,", 0xB0> { let Uses = [IX]; let Defs = [B]; let AsmString = "res 6, ( ix + $imm ), b"; }
  def RES_6_IXd_C : InstDDCB<"res 6,", 0xB1> { let Uses = [IX]; let Defs = [C]; let AsmString = "res 6, ( ix + $imm ), c"; }
  def RES_6_IXd_D : InstDDCB<"res 6,", 0xB2> { let Uses = [IX]; let Defs = [D]; let AsmString = "res 6, ( ix + $imm ), d"; }
  def RES_6_IXd_E : InstDDCB<"res 6,", 0xB3> { let Uses = [IX]; let Defs = [E]; let AsmString = "res 6, ( ix + $imm ), e"; }
  def RES_6_IXd_H : InstDDCB<"res 6,", 0xB4> { let Uses = [IX]; let Defs = [H]; let AsmString = "res 6, ( ix + $imm ), h"; }
  def RES_6_IXd_L : InstDDCB<"res 6,", 0xB5> { let Uses = [IX]; let Defs = [L]; let AsmString = "res 6, ( ix + $imm ), l"; }
  def RES_6_IXd_A : InstDDCB<"res 6,", 0xB7> { let Uses = [IX]; let Defs = [A]; let AsmString = "res 6, ( ix + $imm ), a"; }
  // RES 7,(IX+d),r
  def RES_7_IXd_B : InstDDCB<"res 7,", 0xB8> { let Uses = [IX]; let Defs = [B]; let AsmString = "res 7, ( ix + $imm ), b"; }
  def RES_7_IXd_C : InstDDCB<"res 7,", 0xB9> { let Uses = [IX]; let Defs = [C]; let AsmString = "res 7, ( ix + $imm ), c"; }
  def RES_7_IXd_D : InstDDCB<"res 7,", 0xBA> { let Uses = [IX]; let Defs = [D]; let AsmString = "res 7, ( ix + $imm ), d"; }
  def RES_7_IXd_E : InstDDCB<"res 7,", 0xBB> { let Uses = [IX]; let Defs = [E]; let AsmString = "res 7, ( ix + $imm ), e"; }
  def RES_7_IXd_H : InstDDCB<"res 7,", 0xBC> { let Uses = [IX]; let Defs = [H]; let AsmString = "res 7, ( ix + $imm ), h"; }
  def RES_7_IXd_L : InstDDCB<"res 7,", 0xBD> { let Uses = [IX]; let Defs = [L]; let AsmString = "res 7, ( ix + $imm ), l"; }
  def RES_7_IXd_A : InstDDCB<"res 7,", 0xBF> { let Uses = [IX]; let Defs = [A]; let AsmString = "res 7, ( ix + $imm ), a"; }
}

// --- SET b,(IX+d),r (indexed bit set with register copy) ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  // SET 0,(IX+d),r
  def SET_0_IXd_B : InstDDCB<"set 0,", 0xC0> { let Uses = [IX]; let Defs = [B]; let AsmString = "set 0, ( ix + $imm ), b"; }
  def SET_0_IXd_C : InstDDCB<"set 0,", 0xC1> { let Uses = [IX]; let Defs = [C]; let AsmString = "set 0, ( ix + $imm ), c"; }
  def SET_0_IXd_D : InstDDCB<"set 0,", 0xC2> { let Uses = [IX]; let Defs = [D]; let AsmString = "set 0, ( ix + $imm ), d"; }
  def SET_0_IXd_E : InstDDCB<"set 0,", 0xC3> { let Uses = [IX]; let Defs = [E]; let AsmString = "set 0, ( ix + $imm ), e"; }
  def SET_0_IXd_H : InstDDCB<"set 0,", 0xC4> { let Uses = [IX]; let Defs = [H]; let AsmString = "set 0, ( ix + $imm ), h"; }
  def SET_0_IXd_L : InstDDCB<"set 0,", 0xC5> { let Uses = [IX]; let Defs = [L]; let AsmString = "set 0, ( ix + $imm ), l"; }
  def SET_0_IXd_A : InstDDCB<"set 0,", 0xC7> { let Uses = [IX]; let Defs = [A]; let AsmString = "set 0, ( ix + $imm ), a"; }
  // SET 1,(IX+d),r
  def SET_1_IXd_B : InstDDCB<"set 1,", 0xC8> { let Uses = [IX]; let Defs = [B]; let AsmString = "set 1, ( ix + $imm ), b"; }
  def SET_1_IXd_C : InstDDCB<"set 1,", 0xC9> { let Uses = [IX]; let Defs = [C]; let AsmString = "set 1, ( ix + $imm ), c"; }
  def SET_1_IXd_D : InstDDCB<"set 1,", 0xCA> { let Uses = [IX]; let Defs = [D]; let AsmString = "set 1, ( ix + $imm ), d"; }
  def SET_1_IXd_E : InstDDCB<"set 1,", 0xCB> { let Uses = [IX]; let Defs = [E]; let AsmString = "set 1, ( ix + $imm ), e"; }
  def SET_1_IXd_H : InstDDCB<"set 1,", 0xCC> { let Uses = [IX]; let Defs = [H]; let AsmString = "set 1, ( ix + $imm ), h"; }
  def SET_1_IXd_L : InstDDCB<"set 1,", 0xCD> { let Uses = [IX]; let Defs = [L]; let AsmString = "set 1, ( ix + $imm ), l"; }
  def SET_1_IXd_A : InstDDCB<"set 1,", 0xCF> { let Uses = [IX]; let Defs = [A]; let AsmString = "set 1, ( ix + $imm ), a"; }
  // SET 2,(IX+d),r
  def SET_2_IXd_B : InstDDCB<"set 2,", 0xD0> { let Uses = [IX]; let Defs = [B]; let AsmString = "set 2, ( ix + $imm ), b"; }
  def SET_2_IXd_C : InstDDCB<"set 2,", 0xD1> { let Uses = [IX]; let Defs = [C]; let AsmString = "set 2, ( ix + $imm ), c"; }
  def SET_2_IXd_D : InstDDCB<"set 2,", 0xD2> { let Uses = [IX]; let Defs = [D]; let AsmString = "set 2, ( ix + $imm ), d"; }
  def SET_2_IXd_E : InstDDCB<"set 2,", 0xD3> { let Uses = [IX]; let Defs = [E]; let AsmString = "set 2, ( ix + $imm ), e"; }
  def SET_2_IXd_H : InstDDCB<"set 2,", 0xD4> { let Uses = [IX]; let Defs = [H]; let AsmString = "set 2, ( ix + $imm ), h"; }
  def SET_2_IXd_L : InstDDCB<"set 2,", 0xD5> { let Uses = [IX]; let Defs = [L]; let AsmString = "set 2, ( ix + $imm ), l"; }
  def SET_2_IXd_A : InstDDCB<"set 2,", 0xD7> { let Uses = [IX]; let Defs = [A]; let AsmString = "set 2, ( ix + $imm ), a"; }
  // SET 3,(IX+d),r
  def SET_3_IXd_B : InstDDCB<"set 3,", 0xD8> { let Uses = [IX]; let Defs = [B]; let AsmString = "set 3, ( ix + $imm ), b"; }
  def SET_3_IXd_C : InstDDCB<"set 3,", 0xD9> { let Uses = [IX]; let Defs = [C]; let AsmString = "set 3, ( ix + $imm ), c"; }
  def SET_3_IXd_D : InstDDCB<"set 3,", 0xDA> { let Uses = [IX]; let Defs = [D]; let AsmString = "set 3, ( ix + $imm ), d"; }
  def SET_3_IXd_E : InstDDCB<"set 3,", 0xDB> { let Uses = [IX]; let Defs = [E]; let AsmString = "set 3, ( ix + $imm ), e"; }
  def SET_3_IXd_H : InstDDCB<"set 3,", 0xDC> { let Uses = [IX]; let Defs = [H]; let AsmString = "set 3, ( ix + $imm ), h"; }
  def SET_3_IXd_L : InstDDCB<"set 3,", 0xDD> { let Uses = [IX]; let Defs = [L]; let AsmString = "set 3, ( ix + $imm ), l"; }
  def SET_3_IXd_A : InstDDCB<"set 3,", 0xDF> { let Uses = [IX]; let Defs = [A]; let AsmString = "set 3, ( ix + $imm ), a"; }
  // SET 4,(IX+d),r
  def SET_4_IXd_B : InstDDCB<"set 4,", 0xE0> { let Uses = [IX]; let Defs = [B]; let AsmString = "set 4, ( ix + $imm ), b"; }
  def SET_4_IXd_C : InstDDCB<"set 4,", 0xE1> { let Uses = [IX]; let Defs = [C]; let AsmString = "set 4, ( ix + $imm ), c"; }
  def SET_4_IXd_D : InstDDCB<"set 4,", 0xE2> { let Uses = [IX]; let Defs = [D]; let AsmString = "set 4, ( ix + $imm ), d"; }
  def SET_4_IXd_E : InstDDCB<"set 4,", 0xE3> { let Uses = [IX]; let Defs = [E]; let AsmString = "set 4, ( ix + $imm ), e"; }
  def SET_4_IXd_H : InstDDCB<"set 4,", 0xE4> { let Uses = [IX]; let Defs = [H]; let AsmString = "set 4, ( ix + $imm ), h"; }
  def SET_4_IXd_L : InstDDCB<"set 4,", 0xE5> { let Uses = [IX]; let Defs = [L]; let AsmString = "set 4, ( ix + $imm ), l"; }
  def SET_4_IXd_A : InstDDCB<"set 4,", 0xE7> { let Uses = [IX]; let Defs = [A]; let AsmString = "set 4, ( ix + $imm ), a"; }
  // SET 5,(IX+d),r
  def SET_5_IXd_B : InstDDCB<"set 5,", 0xE8> { let Uses = [IX]; let Defs = [B]; let AsmString = "set 5, ( ix + $imm ), b"; }
  def SET_5_IXd_C : InstDDCB<"set 5,", 0xE9> { let Uses = [IX]; let Defs = [C]; let AsmString = "set 5, ( ix + $imm ), c"; }
  def SET_5_IXd_D : InstDDCB<"set 5,", 0xEA> { let Uses = [IX]; let Defs = [D]; let AsmString = "set 5, ( ix + $imm ), d"; }
  def SET_5_IXd_E : InstDDCB<"set 5,", 0xEB> { let Uses = [IX]; let Defs = [E]; let AsmString = "set 5, ( ix + $imm ), e"; }
  def SET_5_IXd_H : InstDDCB<"set 5,", 0xEC> { let Uses = [IX]; let Defs = [H]; let AsmString = "set 5, ( ix + $imm ), h"; }
  def SET_5_IXd_L : InstDDCB<"set 5,", 0xED> { let Uses = [IX]; let Defs = [L]; let AsmString = "set 5, ( ix + $imm ), l"; }
  def SET_5_IXd_A : InstDDCB<"set 5,", 0xEF> { let Uses = [IX]; let Defs = [A]; let AsmString = "set 5, ( ix + $imm ), a"; }
  // SET 6,(IX+d),r
  def SET_6_IXd_B : InstDDCB<"set 6,", 0xF0> { let Uses = [IX]; let Defs = [B]; let AsmString = "set 6, ( ix + $imm ), b"; }
  def SET_6_IXd_C : InstDDCB<"set 6,", 0xF1> { let Uses = [IX]; let Defs = [C]; let AsmString = "set 6, ( ix + $imm ), c"; }
  def SET_6_IXd_D : InstDDCB<"set 6,", 0xF2> { let Uses = [IX]; let Defs = [D]; let AsmString = "set 6, ( ix + $imm ), d"; }
  def SET_6_IXd_E : InstDDCB<"set 6,", 0xF3> { let Uses = [IX]; let Defs = [E]; let AsmString = "set 6, ( ix + $imm ), e"; }
  def SET_6_IXd_H : InstDDCB<"set 6,", 0xF4> { let Uses = [IX]; let Defs = [H]; let AsmString = "set 6, ( ix + $imm ), h"; }
  def SET_6_IXd_L : InstDDCB<"set 6,", 0xF5> { let Uses = [IX]; let Defs = [L]; let AsmString = "set 6, ( ix + $imm ), l"; }
  def SET_6_IXd_A : InstDDCB<"set 6,", 0xF7> { let Uses = [IX]; let Defs = [A]; let AsmString = "set 6, ( ix + $imm ), a"; }
  // SET 7,(IX+d),r
  def SET_7_IXd_B : InstDDCB<"set 7,", 0xF8> { let Uses = [IX]; let Defs = [B]; let AsmString = "set 7, ( ix + $imm ), b"; }
  def SET_7_IXd_C : InstDDCB<"set 7,", 0xF9> { let Uses = [IX]; let Defs = [C]; let AsmString = "set 7, ( ix + $imm ), c"; }
  def SET_7_IXd_D : InstDDCB<"set 7,", 0xFA> { let Uses = [IX]; let Defs = [D]; let AsmString = "set 7, ( ix + $imm ), d"; }
  def SET_7_IXd_E : InstDDCB<"set 7,", 0xFB> { let Uses = [IX]; let Defs = [E]; let AsmString = "set 7, ( ix + $imm ), e"; }
  def SET_7_IXd_H : InstDDCB<"set 7,", 0xFC> { let Uses = [IX]; let Defs = [H]; let AsmString = "set 7, ( ix + $imm ), h"; }
  def SET_7_IXd_L : InstDDCB<"set 7,", 0xFD> { let Uses = [IX]; let Defs = [L]; let AsmString = "set 7, ( ix + $imm ), l"; }
  def SET_7_IXd_A : InstDDCB<"set 7,", 0xFF> { let Uses = [IX]; let Defs = [A]; let AsmString = "set 7, ( ix + $imm ), a"; }
}

} // Predicates = [HasZ80, HasUndocumented]

// Now the IY equivalents (FDCB prefix) — same structure, different prefix register

let Predicates = [HasZ80, HasUndocumented] in {

// --- Rotate/Shift (IY+d),r ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  // RLC (IY+d),r
  def RLC_IYd_B : InstFDCB<"rlc", 0x00> { let Uses = [IY]; let Defs = [B, FLAGS]; let AsmString = "rlc ( iy + $imm ), b"; }
  def RLC_IYd_C : InstFDCB<"rlc", 0x01> { let Uses = [IY]; let Defs = [C, FLAGS]; let AsmString = "rlc ( iy + $imm ), c"; }
  def RLC_IYd_D : InstFDCB<"rlc", 0x02> { let Uses = [IY]; let Defs = [D, FLAGS]; let AsmString = "rlc ( iy + $imm ), d"; }
  def RLC_IYd_E : InstFDCB<"rlc", 0x03> { let Uses = [IY]; let Defs = [E, FLAGS]; let AsmString = "rlc ( iy + $imm ), e"; }
  def RLC_IYd_H : InstFDCB<"rlc", 0x04> { let Uses = [IY]; let Defs = [H, FLAGS]; let AsmString = "rlc ( iy + $imm ), h"; }
  def RLC_IYd_L : InstFDCB<"rlc", 0x05> { let Uses = [IY]; let Defs = [L, FLAGS]; let AsmString = "rlc ( iy + $imm ), l"; }
  def RLC_IYd_A : InstFDCB<"rlc", 0x07> { let Uses = [IY]; let Defs = [A, FLAGS]; let AsmString = "rlc ( iy + $imm ), a"; }
  // RRC (IY+d),r
  def RRC_IYd_B : InstFDCB<"rrc", 0x08> { let Uses = [IY]; let Defs = [B, FLAGS]; let AsmString = "rrc ( iy + $imm ), b"; }
  def RRC_IYd_C : InstFDCB<"rrc", 0x09> { let Uses = [IY]; let Defs = [C, FLAGS]; let AsmString = "rrc ( iy + $imm ), c"; }
  def RRC_IYd_D : InstFDCB<"rrc", 0x0A> { let Uses = [IY]; let Defs = [D, FLAGS]; let AsmString = "rrc ( iy + $imm ), d"; }
  def RRC_IYd_E : InstFDCB<"rrc", 0x0B> { let Uses = [IY]; let Defs = [E, FLAGS]; let AsmString = "rrc ( iy + $imm ), e"; }
  def RRC_IYd_H : InstFDCB<"rrc", 0x0C> { let Uses = [IY]; let Defs = [H, FLAGS]; let AsmString = "rrc ( iy + $imm ), h"; }
  def RRC_IYd_L : InstFDCB<"rrc", 0x0D> { let Uses = [IY]; let Defs = [L, FLAGS]; let AsmString = "rrc ( iy + $imm ), l"; }
  def RRC_IYd_A : InstFDCB<"rrc", 0x0F> { let Uses = [IY]; let Defs = [A, FLAGS]; let AsmString = "rrc ( iy + $imm ), a"; }
  // RL (IY+d),r
  def RL_IYd_B : InstFDCB<"rl", 0x10> { let Uses = [IY, FLAGS]; let Defs = [B, FLAGS]; let AsmString = "rl ( iy + $imm ), b"; }
  def RL_IYd_C : InstFDCB<"rl", 0x11> { let Uses = [IY, FLAGS]; let Defs = [C, FLAGS]; let AsmString = "rl ( iy + $imm ), c"; }
  def RL_IYd_D : InstFDCB<"rl", 0x12> { let Uses = [IY, FLAGS]; let Defs = [D, FLAGS]; let AsmString = "rl ( iy + $imm ), d"; }
  def RL_IYd_E : InstFDCB<"rl", 0x13> { let Uses = [IY, FLAGS]; let Defs = [E, FLAGS]; let AsmString = "rl ( iy + $imm ), e"; }
  def RL_IYd_H : InstFDCB<"rl", 0x14> { let Uses = [IY, FLAGS]; let Defs = [H, FLAGS]; let AsmString = "rl ( iy + $imm ), h"; }
  def RL_IYd_L : InstFDCB<"rl", 0x15> { let Uses = [IY, FLAGS]; let Defs = [L, FLAGS]; let AsmString = "rl ( iy + $imm ), l"; }
  def RL_IYd_A : InstFDCB<"rl", 0x17> { let Uses = [IY, FLAGS]; let Defs = [A, FLAGS]; let AsmString = "rl ( iy + $imm ), a"; }
  // RR (IY+d),r
  def RR_IYd_B : InstFDCB<"rr", 0x18> { let Uses = [IY, FLAGS]; let Defs = [B, FLAGS]; let AsmString = "rr ( iy + $imm ), b"; }
  def RR_IYd_C : InstFDCB<"rr", 0x19> { let Uses = [IY, FLAGS]; let Defs = [C, FLAGS]; let AsmString = "rr ( iy + $imm ), c"; }
  def RR_IYd_D : InstFDCB<"rr", 0x1A> { let Uses = [IY, FLAGS]; let Defs = [D, FLAGS]; let AsmString = "rr ( iy + $imm ), d"; }
  def RR_IYd_E : InstFDCB<"rr", 0x1B> { let Uses = [IY, FLAGS]; let Defs = [E, FLAGS]; let AsmString = "rr ( iy + $imm ), e"; }
  def RR_IYd_H : InstFDCB<"rr", 0x1C> { let Uses = [IY, FLAGS]; let Defs = [H, FLAGS]; let AsmString = "rr ( iy + $imm ), h"; }
  def RR_IYd_L : InstFDCB<"rr", 0x1D> { let Uses = [IY, FLAGS]; let Defs = [L, FLAGS]; let AsmString = "rr ( iy + $imm ), l"; }
  def RR_IYd_A : InstFDCB<"rr", 0x1F> { let Uses = [IY, FLAGS]; let Defs = [A, FLAGS]; let AsmString = "rr ( iy + $imm ), a"; }
  // SLA (IY+d),r
  def SLA_IYd_B : InstFDCB<"sla", 0x20> { let Uses = [IY]; let Defs = [B, FLAGS]; let AsmString = "sla ( iy + $imm ), b"; }
  def SLA_IYd_C : InstFDCB<"sla", 0x21> { let Uses = [IY]; let Defs = [C, FLAGS]; let AsmString = "sla ( iy + $imm ), c"; }
  def SLA_IYd_D : InstFDCB<"sla", 0x22> { let Uses = [IY]; let Defs = [D, FLAGS]; let AsmString = "sla ( iy + $imm ), d"; }
  def SLA_IYd_E : InstFDCB<"sla", 0x23> { let Uses = [IY]; let Defs = [E, FLAGS]; let AsmString = "sla ( iy + $imm ), e"; }
  def SLA_IYd_H : InstFDCB<"sla", 0x24> { let Uses = [IY]; let Defs = [H, FLAGS]; let AsmString = "sla ( iy + $imm ), h"; }
  def SLA_IYd_L : InstFDCB<"sla", 0x25> { let Uses = [IY]; let Defs = [L, FLAGS]; let AsmString = "sla ( iy + $imm ), l"; }
  def SLA_IYd_A : InstFDCB<"sla", 0x27> { let Uses = [IY]; let Defs = [A, FLAGS]; let AsmString = "sla ( iy + $imm ), a"; }
  // SRA (IY+d),r
  def SRA_IYd_B : InstFDCB<"sra", 0x28> { let Uses = [IY]; let Defs = [B, FLAGS]; let AsmString = "sra ( iy + $imm ), b"; }
  def SRA_IYd_C : InstFDCB<"sra", 0x29> { let Uses = [IY]; let Defs = [C, FLAGS]; let AsmString = "sra ( iy + $imm ), c"; }
  def SRA_IYd_D : InstFDCB<"sra", 0x2A> { let Uses = [IY]; let Defs = [D, FLAGS]; let AsmString = "sra ( iy + $imm ), d"; }
  def SRA_IYd_E : InstFDCB<"sra", 0x2B> { let Uses = [IY]; let Defs = [E, FLAGS]; let AsmString = "sra ( iy + $imm ), e"; }
  def SRA_IYd_H : InstFDCB<"sra", 0x2C> { let Uses = [IY]; let Defs = [H, FLAGS]; let AsmString = "sra ( iy + $imm ), h"; }
  def SRA_IYd_L : InstFDCB<"sra", 0x2D> { let Uses = [IY]; let Defs = [L, FLAGS]; let AsmString = "sra ( iy + $imm ), l"; }
  def SRA_IYd_A : InstFDCB<"sra", 0x2F> { let Uses = [IY]; let Defs = [A, FLAGS]; let AsmString = "sra ( iy + $imm ), a"; }
  // SLL (IY+d),r
  def SLL_IYd_B : InstFDCB<"sll", 0x30> { let Uses = [IY]; let Defs = [B, FLAGS]; let AsmString = "sll ( iy + $imm ), b"; }
  def SLL_IYd_C : InstFDCB<"sll", 0x31> { let Uses = [IY]; let Defs = [C, FLAGS]; let AsmString = "sll ( iy + $imm ), c"; }
  def SLL_IYd_D : InstFDCB<"sll", 0x32> { let Uses = [IY]; let Defs = [D, FLAGS]; let AsmString = "sll ( iy + $imm ), d"; }
  def SLL_IYd_E : InstFDCB<"sll", 0x33> { let Uses = [IY]; let Defs = [E, FLAGS]; let AsmString = "sll ( iy + $imm ), e"; }
  def SLL_IYd_H : InstFDCB<"sll", 0x34> { let Uses = [IY]; let Defs = [H, FLAGS]; let AsmString = "sll ( iy + $imm ), h"; }
  def SLL_IYd_L : InstFDCB<"sll", 0x35> { let Uses = [IY]; let Defs = [L, FLAGS]; let AsmString = "sll ( iy + $imm ), l"; }
  def SLL_IYd_A : InstFDCB<"sll", 0x37> { let Uses = [IY]; let Defs = [A, FLAGS]; let AsmString = "sll ( iy + $imm ), a"; }
  // SRL (IY+d),r
  def SRL_IYd_B : InstFDCB<"srl", 0x38> { let Uses = [IY]; let Defs = [B, FLAGS]; let AsmString = "srl ( iy + $imm ), b"; }
  def SRL_IYd_C : InstFDCB<"srl", 0x39> { let Uses = [IY]; let Defs = [C, FLAGS]; let AsmString = "srl ( iy + $imm ), c"; }
  def SRL_IYd_D : InstFDCB<"srl", 0x3A> { let Uses = [IY]; let Defs = [D, FLAGS]; let AsmString = "srl ( iy + $imm ), d"; }
  def SRL_IYd_E : InstFDCB<"srl", 0x3B> { let Uses = [IY]; let Defs = [E, FLAGS]; let AsmString = "srl ( iy + $imm ), e"; }
  def SRL_IYd_H : InstFDCB<"srl", 0x3C> { let Uses = [IY]; let Defs = [H, FLAGS]; let AsmString = "srl ( iy + $imm ), h"; }
  def SRL_IYd_L : InstFDCB<"srl", 0x3D> { let Uses = [IY]; let Defs = [L, FLAGS]; let AsmString = "srl ( iy + $imm ), l"; }
  def SRL_IYd_A : InstFDCB<"srl", 0x3F> { let Uses = [IY]; let Defs = [A, FLAGS]; let AsmString = "srl ( iy + $imm ), a"; }
}

// --- RES b,(IY+d),r ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def RES_0_IYd_B : InstFDCB<"res 0,", 0x80> { let Uses = [IY]; let Defs = [B]; let AsmString = "res 0, ( iy + $imm ), b"; }
  def RES_0_IYd_C : InstFDCB<"res 0,", 0x81> { let Uses = [IY]; let Defs = [C]; let AsmString = "res 0, ( iy + $imm ), c"; }
  def RES_0_IYd_D : InstFDCB<"res 0,", 0x82> { let Uses = [IY]; let Defs = [D]; let AsmString = "res 0, ( iy + $imm ), d"; }
  def RES_0_IYd_E : InstFDCB<"res 0,", 0x83> { let Uses = [IY]; let Defs = [E]; let AsmString = "res 0, ( iy + $imm ), e"; }
  def RES_0_IYd_H : InstFDCB<"res 0,", 0x84> { let Uses = [IY]; let Defs = [H]; let AsmString = "res 0, ( iy + $imm ), h"; }
  def RES_0_IYd_L : InstFDCB<"res 0,", 0x85> { let Uses = [IY]; let Defs = [L]; let AsmString = "res 0, ( iy + $imm ), l"; }
  def RES_0_IYd_A : InstFDCB<"res 0,", 0x87> { let Uses = [IY]; let Defs = [A]; let AsmString = "res 0, ( iy + $imm ), a"; }
  def RES_1_IYd_B : InstFDCB<"res 1,", 0x88> { let Uses = [IY]; let Defs = [B]; let AsmString = "res 1, ( iy + $imm ), b"; }
  def RES_1_IYd_C : InstFDCB<"res 1,", 0x89> { let Uses = [IY]; let Defs = [C]; let AsmString = "res 1, ( iy + $imm ), c"; }
  def RES_1_IYd_D : InstFDCB<"res 1,", 0x8A> { let Uses = [IY]; let Defs = [D]; let AsmString = "res 1, ( iy + $imm ), d"; }
  def RES_1_IYd_E : InstFDCB<"res 1,", 0x8B> { let Uses = [IY]; let Defs = [E]; let AsmString = "res 1, ( iy + $imm ), e"; }
  def RES_1_IYd_H : InstFDCB<"res 1,", 0x8C> { let Uses = [IY]; let Defs = [H]; let AsmString = "res 1, ( iy + $imm ), h"; }
  def RES_1_IYd_L : InstFDCB<"res 1,", 0x8D> { let Uses = [IY]; let Defs = [L]; let AsmString = "res 1, ( iy + $imm ), l"; }
  def RES_1_IYd_A : InstFDCB<"res 1,", 0x8F> { let Uses = [IY]; let Defs = [A]; let AsmString = "res 1, ( iy + $imm ), a"; }
  def RES_2_IYd_B : InstFDCB<"res 2,", 0x90> { let Uses = [IY]; let Defs = [B]; let AsmString = "res 2, ( iy + $imm ), b"; }
  def RES_2_IYd_C : InstFDCB<"res 2,", 0x91> { let Uses = [IY]; let Defs = [C]; let AsmString = "res 2, ( iy + $imm ), c"; }
  def RES_2_IYd_D : InstFDCB<"res 2,", 0x92> { let Uses = [IY]; let Defs = [D]; let AsmString = "res 2, ( iy + $imm ), d"; }
  def RES_2_IYd_E : InstFDCB<"res 2,", 0x93> { let Uses = [IY]; let Defs = [E]; let AsmString = "res 2, ( iy + $imm ), e"; }
  def RES_2_IYd_H : InstFDCB<"res 2,", 0x94> { let Uses = [IY]; let Defs = [H]; let AsmString = "res 2, ( iy + $imm ), h"; }
  def RES_2_IYd_L : InstFDCB<"res 2,", 0x95> { let Uses = [IY]; let Defs = [L]; let AsmString = "res 2, ( iy + $imm ), l"; }
  def RES_2_IYd_A : InstFDCB<"res 2,", 0x97> { let Uses = [IY]; let Defs = [A]; let AsmString = "res 2, ( iy + $imm ), a"; }
  def RES_3_IYd_B : InstFDCB<"res 3,", 0x98> { let Uses = [IY]; let Defs = [B]; let AsmString = "res 3, ( iy + $imm ), b"; }
  def RES_3_IYd_C : InstFDCB<"res 3,", 0x99> { let Uses = [IY]; let Defs = [C]; let AsmString = "res 3, ( iy + $imm ), c"; }
  def RES_3_IYd_D : InstFDCB<"res 3,", 0x9A> { let Uses = [IY]; let Defs = [D]; let AsmString = "res 3, ( iy + $imm ), d"; }
  def RES_3_IYd_E : InstFDCB<"res 3,", 0x9B> { let Uses = [IY]; let Defs = [E]; let AsmString = "res 3, ( iy + $imm ), e"; }
  def RES_3_IYd_H : InstFDCB<"res 3,", 0x9C> { let Uses = [IY]; let Defs = [H]; let AsmString = "res 3, ( iy + $imm ), h"; }
  def RES_3_IYd_L : InstFDCB<"res 3,", 0x9D> { let Uses = [IY]; let Defs = [L]; let AsmString = "res 3, ( iy + $imm ), l"; }
  def RES_3_IYd_A : InstFDCB<"res 3,", 0x9F> { let Uses = [IY]; let Defs = [A]; let AsmString = "res 3, ( iy + $imm ), a"; }
  def RES_4_IYd_B : InstFDCB<"res 4,", 0xA0> { let Uses = [IY]; let Defs = [B]; let AsmString = "res 4, ( iy + $imm ), b"; }
  def RES_4_IYd_C : InstFDCB<"res 4,", 0xA1> { let Uses = [IY]; let Defs = [C]; let AsmString = "res 4, ( iy + $imm ), c"; }
  def RES_4_IYd_D : InstFDCB<"res 4,", 0xA2> { let Uses = [IY]; let Defs = [D]; let AsmString = "res 4, ( iy + $imm ), d"; }
  def RES_4_IYd_E : InstFDCB<"res 4,", 0xA3> { let Uses = [IY]; let Defs = [E]; let AsmString = "res 4, ( iy + $imm ), e"; }
  def RES_4_IYd_H : InstFDCB<"res 4,", 0xA4> { let Uses = [IY]; let Defs = [H]; let AsmString = "res 4, ( iy + $imm ), h"; }
  def RES_4_IYd_L : InstFDCB<"res 4,", 0xA5> { let Uses = [IY]; let Defs = [L]; let AsmString = "res 4, ( iy + $imm ), l"; }
  def RES_4_IYd_A : InstFDCB<"res 4,", 0xA7> { let Uses = [IY]; let Defs = [A]; let AsmString = "res 4, ( iy + $imm ), a"; }
  def RES_5_IYd_B : InstFDCB<"res 5,", 0xA8> { let Uses = [IY]; let Defs = [B]; let AsmString = "res 5, ( iy + $imm ), b"; }
  def RES_5_IYd_C : InstFDCB<"res 5,", 0xA9> { let Uses = [IY]; let Defs = [C]; let AsmString = "res 5, ( iy + $imm ), c"; }
  def RES_5_IYd_D : InstFDCB<"res 5,", 0xAA> { let Uses = [IY]; let Defs = [D]; let AsmString = "res 5, ( iy + $imm ), d"; }
  def RES_5_IYd_E : InstFDCB<"res 5,", 0xAB> { let Uses = [IY]; let Defs = [E]; let AsmString = "res 5, ( iy + $imm ), e"; }
  def RES_5_IYd_H : InstFDCB<"res 5,", 0xAC> { let Uses = [IY]; let Defs = [H]; let AsmString = "res 5, ( iy + $imm ), h"; }
  def RES_5_IYd_L : InstFDCB<"res 5,", 0xAD> { let Uses = [IY]; let Defs = [L]; let AsmString = "res 5, ( iy + $imm ), l"; }
  def RES_5_IYd_A : InstFDCB<"res 5,", 0xAF> { let Uses = [IY]; let Defs = [A]; let AsmString = "res 5, ( iy + $imm ), a"; }
  def RES_6_IYd_B : InstFDCB<"res 6,", 0xB0> { let Uses = [IY]; let Defs = [B]; let AsmString = "res 6, ( iy + $imm ), b"; }
  def RES_6_IYd_C : InstFDCB<"res 6,", 0xB1> { let Uses = [IY]; let Defs = [C]; let AsmString = "res 6, ( iy + $imm ), c"; }
  def RES_6_IYd_D : InstFDCB<"res 6,", 0xB2> { let Uses = [IY]; let Defs = [D]; let AsmString = "res 6, ( iy + $imm ), d"; }
  def RES_6_IYd_E : InstFDCB<"res 6,", 0xB3> { let Uses = [IY]; let Defs = [E]; let AsmString = "res 6, ( iy + $imm ), e"; }
  def RES_6_IYd_H : InstFDCB<"res 6,", 0xB4> { let Uses = [IY]; let Defs = [H]; let AsmString = "res 6, ( iy + $imm ), h"; }
  def RES_6_IYd_L : InstFDCB<"res 6,", 0xB5> { let Uses = [IY]; let Defs = [L]; let AsmString = "res 6, ( iy + $imm ), l"; }
  def RES_6_IYd_A : InstFDCB<"res 6,", 0xB7> { let Uses = [IY]; let Defs = [A]; let AsmString = "res 6, ( iy + $imm ), a"; }
  def RES_7_IYd_B : InstFDCB<"res 7,", 0xB8> { let Uses = [IY]; let Defs = [B]; let AsmString = "res 7, ( iy + $imm ), b"; }
  def RES_7_IYd_C : InstFDCB<"res 7,", 0xB9> { let Uses = [IY]; let Defs = [C]; let AsmString = "res 7, ( iy + $imm ), c"; }
  def RES_7_IYd_D : InstFDCB<"res 7,", 0xBA> { let Uses = [IY]; let Defs = [D]; let AsmString = "res 7, ( iy + $imm ), d"; }
  def RES_7_IYd_E : InstFDCB<"res 7,", 0xBB> { let Uses = [IY]; let Defs = [E]; let AsmString = "res 7, ( iy + $imm ), e"; }
  def RES_7_IYd_H : InstFDCB<"res 7,", 0xBC> { let Uses = [IY]; let Defs = [H]; let AsmString = "res 7, ( iy + $imm ), h"; }
  def RES_7_IYd_L : InstFDCB<"res 7,", 0xBD> { let Uses = [IY]; let Defs = [L]; let AsmString = "res 7, ( iy + $imm ), l"; }
  def RES_7_IYd_A : InstFDCB<"res 7,", 0xBF> { let Uses = [IY]; let Defs = [A]; let AsmString = "res 7, ( iy + $imm ), a"; }
}

// --- SET b,(IY+d),r ---
let mayLoad = 1, mayStore = 1, hasSideEffects = 0 in {
  def SET_0_IYd_B : InstFDCB<"set 0,", 0xC0> { let Uses = [IY]; let Defs = [B]; let AsmString = "set 0, ( iy + $imm ), b"; }
  def SET_0_IYd_C : InstFDCB<"set 0,", 0xC1> { let Uses = [IY]; let Defs = [C]; let AsmString = "set 0, ( iy + $imm ), c"; }
  def SET_0_IYd_D : InstFDCB<"set 0,", 0xC2> { let Uses = [IY]; let Defs = [D]; let AsmString = "set 0, ( iy + $imm ), d"; }
  def SET_0_IYd_E : InstFDCB<"set 0,", 0xC3> { let Uses = [IY]; let Defs = [E]; let AsmString = "set 0, ( iy + $imm ), e"; }
  def SET_0_IYd_H : InstFDCB<"set 0,", 0xC4> { let Uses = [IY]; let Defs = [H]; let AsmString = "set 0, ( iy + $imm ), h"; }
  def SET_0_IYd_L : InstFDCB<"set 0,", 0xC5> { let Uses = [IY]; let Defs = [L]; let AsmString = "set 0, ( iy + $imm ), l"; }
  def SET_0_IYd_A : InstFDCB<"set 0,", 0xC7> { let Uses = [IY]; let Defs = [A]; let AsmString = "set 0, ( iy + $imm ), a"; }
  def SET_1_IYd_B : InstFDCB<"set 1,", 0xC8> { let Uses = [IY]; let Defs = [B]; let AsmString = "set 1, ( iy + $imm ), b"; }
  def SET_1_IYd_C : InstFDCB<"set 1,", 0xC9> { let Uses = [IY]; let Defs = [C]; let AsmString = "set 1, ( iy + $imm ), c"; }
  def SET_1_IYd_D : InstFDCB<"set 1,", 0xCA> { let Uses = [IY]; let Defs = [D]; let AsmString = "set 1, ( iy + $imm ), d"; }
  def SET_1_IYd_E : InstFDCB<"set 1,", 0xCB> { let Uses = [IY]; let Defs = [E]; let AsmString = "set 1, ( iy + $imm ), e"; }
  def SET_1_IYd_H : InstFDCB<"set 1,", 0xCC> { let Uses = [IY]; let Defs = [H]; let AsmString = "set 1, ( iy + $imm ), h"; }
  def SET_1_IYd_L : InstFDCB<"set 1,", 0xCD> { let Uses = [IY]; let Defs = [L]; let AsmString = "set 1, ( iy + $imm ), l"; }
  def SET_1_IYd_A : InstFDCB<"set 1,", 0xCF> { let Uses = [IY]; let Defs = [A]; let AsmString = "set 1, ( iy + $imm ), a"; }
  def SET_2_IYd_B : InstFDCB<"set 2,", 0xD0> { let Uses = [IY]; let Defs = [B]; let AsmString = "set 2, ( iy + $imm ), b"; }
  def SET_2_IYd_C : InstFDCB<"set 2,", 0xD1> { let Uses = [IY]; let Defs = [C]; let AsmString = "set 2, ( iy + $imm ), c"; }
  def SET_2_IYd_D : InstFDCB<"set 2,", 0xD2> { let Uses = [IY]; let Defs = [D]; let AsmString = "set 2, ( iy + $imm ), d"; }
  def SET_2_IYd_E : InstFDCB<"set 2,", 0xD3> { let Uses = [IY]; let Defs = [E]; let AsmString = "set 2, ( iy + $imm ), e"; }
  def SET_2_IYd_H : InstFDCB<"set 2,", 0xD4> { let Uses = [IY]; let Defs = [H]; let AsmString = "set 2, ( iy + $imm ), h"; }
  def SET_2_IYd_L : InstFDCB<"set 2,", 0xD5> { let Uses = [IY]; let Defs = [L]; let AsmString = "set 2, ( iy + $imm ), l"; }
  def SET_2_IYd_A : InstFDCB<"set 2,", 0xD7> { let Uses = [IY]; let Defs = [A]; let AsmString = "set 2, ( iy + $imm ), a"; }
  def SET_3_IYd_B : InstFDCB<"set 3,", 0xD8> { let Uses = [IY]; let Defs = [B]; let AsmString = "set 3, ( iy + $imm ), b"; }
  def SET_3_IYd_C : InstFDCB<"set 3,", 0xD9> { let Uses = [IY]; let Defs = [C]; let AsmString = "set 3, ( iy + $imm ), c"; }
  def SET_3_IYd_D : InstFDCB<"set 3,", 0xDA> { let Uses = [IY]; let Defs = [D]; let AsmString = "set 3, ( iy + $imm ), d"; }
  def SET_3_IYd_E : InstFDCB<"set 3,", 0xDB> { let Uses = [IY]; let Defs = [E]; let AsmString = "set 3, ( iy + $imm ), e"; }
  def SET_3_IYd_H : InstFDCB<"set 3,", 0xDC> { let Uses = [IY]; let Defs = [H]; let AsmString = "set 3, ( iy + $imm ), h"; }
  def SET_3_IYd_L : InstFDCB<"set 3,", 0xDD> { let Uses = [IY]; let Defs = [L]; let AsmString = "set 3, ( iy + $imm ), l"; }
  def SET_3_IYd_A : InstFDCB<"set 3,", 0xDF> { let Uses = [IY]; let Defs = [A]; let AsmString = "set 3, ( iy + $imm ), a"; }
  def SET_4_IYd_B : InstFDCB<"set 4,", 0xE0> { let Uses = [IY]; let Defs = [B]; let AsmString = "set 4, ( iy + $imm ), b"; }
  def SET_4_IYd_C : InstFDCB<"set 4,", 0xE1> { let Uses = [IY]; let Defs = [C]; let AsmString = "set 4, ( iy + $imm ), c"; }
  def SET_4_IYd_D : InstFDCB<"set 4,", 0xE2> { let Uses = [IY]; let Defs = [D]; let AsmString = "set 4, ( iy + $imm ), d"; }
  def SET_4_IYd_E : InstFDCB<"set 4,", 0xE3> { let Uses = [IY]; let Defs = [E]; let AsmString = "set 4, ( iy + $imm ), e"; }
  def SET_4_IYd_H : InstFDCB<"set 4,", 0xE4> { let Uses = [IY]; let Defs = [H]; let AsmString = "set 4, ( iy + $imm ), h"; }
  def SET_4_IYd_L : InstFDCB<"set 4,", 0xE5> { let Uses = [IY]; let Defs = [L]; let AsmString = "set 4, ( iy + $imm ), l"; }
  def SET_4_IYd_A : InstFDCB<"set 4,", 0xE7> { let Uses = [IY]; let Defs = [A]; let AsmString = "set 4, ( iy + $imm ), a"; }
  def SET_5_IYd_B : InstFDCB<"set 5,", 0xE8> { let Uses = [IY]; let Defs = [B]; let AsmString = "set 5, ( iy + $imm ), b"; }
  def SET_5_IYd_C : InstFDCB<"set 5,", 0xE9> { let Uses = [IY]; let Defs = [C]; let AsmString = "set 5, ( iy + $imm ), c"; }
  def SET_5_IYd_D : InstFDCB<"set 5,", 0xEA> { let Uses = [IY]; let Defs = [D]; let AsmString = "set 5, ( iy + $imm ), d"; }
  def SET_5_IYd_E : InstFDCB<"set 5,", 0xEB> { let Uses = [IY]; let Defs = [E]; let AsmString = "set 5, ( iy + $imm ), e"; }
  def SET_5_IYd_H : InstFDCB<"set 5,", 0xEC> { let Uses = [IY]; let Defs = [H]; let AsmString = "set 5, ( iy + $imm ), h"; }
  def SET_5_IYd_L : InstFDCB<"set 5,", 0xED> { let Uses = [IY]; let Defs = [L]; let AsmString = "set 5, ( iy + $imm ), l"; }
  def SET_5_IYd_A : InstFDCB<"set 5,", 0xEF> { let Uses = [IY]; let Defs = [A]; let AsmString = "set 5, ( iy + $imm ), a"; }
  def SET_6_IYd_B : InstFDCB<"set 6,", 0xF0> { let Uses = [IY]; let Defs = [B]; let AsmString = "set 6, ( iy + $imm ), b"; }
  def SET_6_IYd_C : InstFDCB<"set 6,", 0xF1> { let Uses = [IY]; let Defs = [C]; let AsmString = "set 6, ( iy + $imm ), c"; }
  def SET_6_IYd_D : InstFDCB<"set 6,", 0xF2> { let Uses = [IY]; let Defs = [D]; let AsmString = "set 6, ( iy + $imm ), d"; }
  def SET_6_IYd_E : InstFDCB<"set 6,", 0xF3> { let Uses = [IY]; let Defs = [E]; let AsmString = "set 6, ( iy + $imm ), e"; }
  def SET_6_IYd_H : InstFDCB<"set 6,", 0xF4> { let Uses = [IY]; let Defs = [H]; let AsmString = "set 6, ( iy + $imm ), h"; }
  def SET_6_IYd_L : InstFDCB<"set 6,", 0xF5> { let Uses = [IY]; let Defs = [L]; let AsmString = "set 6, ( iy + $imm ), l"; }
  def SET_6_IYd_A : InstFDCB<"set 6,", 0xF7> { let Uses = [IY]; let Defs = [A]; let AsmString = "set 6, ( iy + $imm ), a"; }
  def SET_7_IYd_B : InstFDCB<"set 7,", 0xF8> { let Uses = [IY]; let Defs = [B]; let AsmString = "set 7, ( iy + $imm ), b"; }
  def SET_7_IYd_C : InstFDCB<"set 7,", 0xF9> { let Uses = [IY]; let Defs = [C]; let AsmString = "set 7, ( iy + $imm ), c"; }
  def SET_7_IYd_D : InstFDCB<"set 7,", 0xFA> { let Uses = [IY]; let Defs = [D]; let AsmString = "set 7, ( iy + $imm ), d"; }
  def SET_7_IYd_E : InstFDCB<"set 7,", 0xFB> { let Uses = [IY]; let Defs = [E]; let AsmString = "set 7, ( iy + $imm ), e"; }
  def SET_7_IYd_H : InstFDCB<"set 7,", 0xFC> { let Uses = [IY]; let Defs = [H]; let AsmString = "set 7, ( iy + $imm ), h"; }
  def SET_7_IYd_L : InstFDCB<"set 7,", 0xFD> { let Uses = [IY]; let Defs = [L]; let AsmString = "set 7, ( iy + $imm ), l"; }
  def SET_7_IYd_A : InstFDCB<"set 7,", 0xFF> { let Uses = [IY]; let Defs = [A]; let AsmString = "set 7, ( iy + $imm ), a"; }
}

} // Predicates = [HasZ80, HasUndocumented]

//===----------------------------------------------------------------------===//
//  Undocumented IXH/IXL/IYH/IYL 8-bit Instructions (DD/FD prefixed)
//===----------------------------------------------------------------------===//
// These are real opcodes on NMOS and CMOS Z80 but not officially documented
// by Zilog. They work by the DD/FD prefix substituting H→IXH/IYH and L→IXL/IYL
// in the register encoding. Gated by FeatureUndocumented.
//
// NOT supported on: R800 (MSX TurboR), eZ80
// Supported on: all NMOS Z80, CMOS Z80 (Zilog, Toshiba, NEC, ST, etc.)

let Predicates = [HasZ80, HasUndocumented] in {

// --- LD IXH/IXL,r and LD r,IXH/IXL (8-bit half-register reg-reg moves) ---
// Register-only operation -- override conservative Z80Inst defaults.
// See ravn/llvm-z80#154.
let mayLoad = 0, mayStore = 0, hasSideEffects = 0 in {
def LD_IXH_A : InstDD8<"ld ixh,a", 0x67> { let Uses = [A]; let Defs = [IXH]; }
def LD_IXH_B : InstDD8<"ld ixh,b", 0x60> { let Uses = [B]; let Defs = [IXH]; }
def LD_IXH_C : InstDD8<"ld ixh,c", 0x61> { let Uses = [C]; let Defs = [IXH]; }
def LD_IXH_D : InstDD8<"ld ixh,d", 0x62> { let Uses = [D]; let Defs = [IXH]; }
def LD_IXH_E : InstDD8<"ld ixh,e", 0x63> { let Uses = [E]; let Defs = [IXH]; }
def LD_IXH_IXH : InstDD8<"ld ixh,ixh", 0x64> { let Uses = [IXH]; let Defs = [IXH]; }
def LD_IXH_IXL : InstDD8<"ld ixh,ixl", 0x65> { let Uses = [IXL]; let Defs = [IXH]; }

def LD_IXL_A : InstDD8<"ld ixl,a", 0x6F> { let Uses = [A]; let Defs = [IXL]; }
def LD_IXL_B : InstDD8<"ld ixl,b", 0x68> { let Uses = [B]; let Defs = [IXL]; }
def LD_IXL_C : InstDD8<"ld ixl,c", 0x69> { let Uses = [C]; let Defs = [IXL]; }
def LD_IXL_D : InstDD8<"ld ixl,d", 0x6A> { let Uses = [D]; let Defs = [IXL]; }
def LD_IXL_E : InstDD8<"ld ixl,e", 0x6B> { let Uses = [E]; let Defs = [IXL]; }
def LD_IXL_IXH : InstDD8<"ld ixl,ixh", 0x6C> { let Uses = [IXH]; let Defs = [IXL]; }
def LD_IXL_IXL : InstDD8<"ld ixl,ixl", 0x6D> { let Uses = [IXL]; let Defs = [IXL]; }

// --- LD r,IXH/IXL (load IX half into register) ---
def LD_A_IXH : InstDD8<"ld a,ixh", 0x7C> { let Uses = [IXH]; let Defs = [A]; }
def LD_A_IXL : InstDD8<"ld a,ixl", 0x7D> { let Uses = [IXL]; let Defs = [A]; }
def LD_B_IXH : InstDD8<"ld b,ixh", 0x44> { let Uses = [IXH]; let Defs = [B]; }
def LD_B_IXL : InstDD8<"ld b,ixl", 0x45> { let Uses = [IXL]; let Defs = [B]; }
def LD_C_IXH : InstDD8<"ld c,ixh", 0x4C> { let Uses = [IXH]; let Defs = [C]; }
def LD_C_IXL : InstDD8<"ld c,ixl", 0x4D> { let Uses = [IXL]; let Defs = [C]; }
def LD_D_IXH : InstDD8<"ld d,ixh", 0x54> { let Uses = [IXH]; let Defs = [D]; }
def LD_D_IXL : InstDD8<"ld d,ixl", 0x55> { let Uses = [IXL]; let Defs = [D]; }
def LD_E_IXH : InstDD8<"ld e,ixh", 0x5C> { let Uses = [IXH]; let Defs = [E]; }
def LD_E_IXL : InstDD8<"ld e,ixl", 0x5D> { let Uses = [IXL]; let Defs = [E]; }
} // mayLoad = 0, mayStore = 0, hasSideEffects = 0 (LD IX{H,L} reg-reg)

// --- LD IXH/IXL,n (load immediate) ---
def LD_IXH_n : InstDD16<"ld ixh,", 0x26, Imm8Mode> { let Defs = [IXH]; }
def LD_IXL_n : InstDD16<"ld ixl,", 0x2E, Imm8Mode> { let Defs = [IXL]; }

// --- INC/DEC IXH/IXL ---
def INC_IXH : InstDD8<"inc ixh", 0x24> { let Uses = [IXH]; let Defs = [IXH, FLAGS]; }
def DEC_IXH : InstDD8<"dec ixh", 0x25> { let Uses = [IXH]; let Defs = [IXH, FLAGS]; }
def INC_IXL : InstDD8<"inc ixl", 0x2C> { let Uses = [IXL]; let Defs = [IXL, FLAGS]; }
def DEC_IXL : InstDD8<"dec ixl", 0x2D> { let Uses = [IXL]; let Defs = [IXL, FLAGS]; }

// --- ALU A,IXH/IXL (arithmetic/logic with accumulator) ---
def ADD_A_IXH : InstDD8<"add a,ixh", 0x84> { let Uses = [A, IXH]; let Defs = [A, FLAGS]; }
def ADD_A_IXL : InstDD8<"add a,ixl", 0x85> { let Uses = [A, IXL]; let Defs = [A, FLAGS]; }
def ADC_A_IXH : InstDD8<"adc a,ixh", 0x8C> { let Uses = [A, IXH, FLAGS]; let Defs = [A, FLAGS]; }
def ADC_A_IXL : InstDD8<"adc a,ixl", 0x8D> { let Uses = [A, IXL, FLAGS]; let Defs = [A, FLAGS]; }
def SUB_IXH : InstDD8<"sub ixh", 0x94> { let Uses = [A, IXH]; let Defs = [A, FLAGS]; }
def SUB_IXL : InstDD8<"sub ixl", 0x95> { let Uses = [A, IXL]; let Defs = [A, FLAGS]; }
def SBC_A_IXH : InstDD8<"sbc a,ixh", 0x9C> { let Uses = [A, IXH, FLAGS]; let Defs = [A, FLAGS]; }
def SBC_A_IXL : InstDD8<"sbc a,ixl", 0x9D> { let Uses = [A, IXL, FLAGS]; let Defs = [A, FLAGS]; }
def AND_IXH : InstDD8<"and ixh", 0xA4> { let Uses = [A, IXH]; let Defs = [A, FLAGS]; }
def AND_IXL : InstDD8<"and ixl", 0xA5> { let Uses = [A, IXL]; let Defs = [A, FLAGS]; }
def XOR_IXH : InstDD8<"xor ixh", 0xAC> { let Uses = [A, IXH]; let Defs = [A, FLAGS]; }
def XOR_IXL : InstDD8<"xor ixl", 0xAD> { let Uses = [A, IXL]; let Defs = [A, FLAGS]; }
def OR_IXH : InstDD8<"or ixh", 0xB4> { let Uses = [A, IXH]; let Defs = [A, FLAGS]; }
def OR_IXL : InstDD8<"or ixl", 0xB5> { let Uses = [A, IXL]; let Defs = [A, FLAGS]; }
def CP_IXH : InstDD8<"cp ixh", 0xBC> { let Uses = [A, IXH]; let Defs = [FLAGS]; }
def CP_IXL : InstDD8<"cp ixl", 0xBD> { let Uses = [A, IXL]; let Defs = [FLAGS]; }

// --- IYH/IYL equivalents (FD prefix) ---
// Register-only moves -- override conservative Z80Inst defaults.
// See ravn/llvm-z80#154.
let mayLoad = 0, mayStore = 0, hasSideEffects = 0 in {
def LD_IYH_A : InstFD8<"ld iyh,a", 0x67> { let Uses = [A]; let Defs = [IYH]; }
def LD_IYH_B : InstFD8<"ld iyh,b", 0x60> { let Uses = [B]; let Defs = [IYH]; }
def LD_IYH_C : InstFD8<"ld iyh,c", 0x61> { let Uses = [C]; let Defs = [IYH]; }
def LD_IYH_D : InstFD8<"ld iyh,d", 0x62> { let Uses = [D]; let Defs = [IYH]; }
def LD_IYH_E : InstFD8<"ld iyh,e", 0x63> { let Uses = [E]; let Defs = [IYH]; }
def LD_IYH_IYH : InstFD8<"ld iyh,iyh", 0x64> { let Uses = [IYH]; let Defs = [IYH]; }
def LD_IYH_IYL : InstFD8<"ld iyh,iyl", 0x65> { let Uses = [IYL]; let Defs = [IYH]; }

def LD_IYL_A : InstFD8<"ld iyl,a", 0x6F> { let Uses = [A]; let Defs = [IYL]; }
def LD_IYL_B : InstFD8<"ld iyl,b", 0x68> { let Uses = [B]; let Defs = [IYL]; }
def LD_IYL_C : InstFD8<"ld iyl,c", 0x69> { let Uses = [C]; let Defs = [IYL]; }
def LD_IYL_D : InstFD8<"ld iyl,d", 0x6A> { let Uses = [D]; let Defs = [IYL]; }
def LD_IYL_E : InstFD8<"ld iyl,e", 0x6B> { let Uses = [E]; let Defs = [IYL]; }
def LD_IYL_IYH : InstFD8<"ld iyl,iyh", 0x6C> { let Uses = [IYH]; let Defs = [IYL]; }
def LD_IYL_IYL : InstFD8<"ld iyl,iyl", 0x6D> { let Uses = [IYL]; let Defs = [IYL]; }

def LD_A_IYH : InstFD8<"ld a,iyh", 0x7C> { let Uses = [IYH]; let Defs = [A]; }
def LD_A_IYL : InstFD8<"ld a,iyl", 0x7D> { let Uses = [IYL]; let Defs = [A]; }
def LD_B_IYH : InstFD8<"ld b,iyh", 0x44> { let Uses = [IYH]; let Defs = [B]; }
def LD_B_IYL : InstFD8<"ld b,iyl", 0x45> { let Uses = [IYL]; let Defs = [B]; }
def LD_C_IYH : InstFD8<"ld c,iyh", 0x4C> { let Uses = [IYH]; let Defs = [C]; }
def LD_C_IYL : InstFD8<"ld c,iyl", 0x4D> { let Uses = [IYL]; let Defs = [C]; }
def LD_D_IYH : InstFD8<"ld d,iyh", 0x54> { let Uses = [IYH]; let Defs = [D]; }
def LD_D_IYL : InstFD8<"ld d,iyl", 0x55> { let Uses = [IYL]; let Defs = [D]; }
def LD_E_IYH : InstFD8<"ld e,iyh", 0x5C> { let Uses = [IYH]; let Defs = [E]; }
def LD_E_IYL : InstFD8<"ld e,iyl", 0x5D> { let Uses = [IYL]; let Defs = [E]; }
} // mayLoad = 0, mayStore = 0, hasSideEffects = 0 (LD IY{H,L} reg-reg)

def LD_IYH_n : InstFD16<"ld iyh,", 0x26, Imm8Mode> { let Defs = [IYH]; }
def LD_IYL_n : InstFD16<"ld iyl,", 0x2E, Imm8Mode> { let Defs = [IYL]; }

def INC_IYH : InstFD8<"inc iyh", 0x24> { let Uses = [IYH]; let Defs = [IYH, FLAGS]; }
def DEC_IYH : InstFD8<"dec iyh", 0x25> { let Uses = [IYH]; let Defs = [IYH, FLAGS]; }
def INC_IYL : InstFD8<"inc iyl", 0x2C> { let Uses = [IYL]; let Defs = [IYL, FLAGS]; }
def DEC_IYL : InstFD8<"dec iyl", 0x2D> { let Uses = [IYL]; let Defs = [IYL, FLAGS]; }

def ADD_A_IYH : InstFD8<"add a,iyh", 0x84> { let Uses = [A, IYH]; let Defs = [A, FLAGS]; }
def ADD_A_IYL : InstFD8<"add a,iyl", 0x85> { let Uses = [A, IYL]; let Defs = [A, FLAGS]; }
def ADC_A_IYH : InstFD8<"adc a,iyh", 0x8C> { let Uses = [A, IYH, FLAGS]; let Defs = [A, FLAGS]; }
def ADC_A_IYL : InstFD8<"adc a,iyl", 0x8D> { let Uses = [A, IYL, FLAGS]; let Defs = [A, FLAGS]; }
def SUB_IYH : InstFD8<"sub iyh", 0x94> { let Uses = [A, IYH]; let Defs = [A, FLAGS]; }
def SUB_IYL : InstFD8<"sub iyl", 0x95> { let Uses = [A, IYL]; let Defs = [A, FLAGS]; }
def SBC_A_IYH : InstFD8<"sbc a,iyh", 0x9C> { let Uses = [A, IYH, FLAGS]; let Defs = [A, FLAGS]; }
def SBC_A_IYL : InstFD8<"sbc a,iyl", 0x9D> { let Uses = [A, IYL, FLAGS]; let Defs = [A, FLAGS]; }
def AND_IYH : InstFD8<"and iyh", 0xA4> { let Uses = [A, IYH]; let Defs = [A, FLAGS]; }
def AND_IYL : InstFD8<"and iyl", 0xA5> { let Uses = [A, IYL]; let Defs = [A, FLAGS]; }
def XOR_IYH : InstFD8<"xor iyh", 0xAC> { let Uses = [A, IYH]; let Defs = [A, FLAGS]; }
def XOR_IYL : InstFD8<"xor iyl", 0xAD> { let Uses = [A, IYL]; let Defs = [A, FLAGS]; }
def OR_IYH : InstFD8<"or iyh", 0xB4> { let Uses = [A, IYH]; let Defs = [A, FLAGS]; }
def OR_IYL : InstFD8<"or iyl", 0xB5> { let Uses = [A, IYL]; let Defs = [A, FLAGS]; }
def CP_IYH : InstFD8<"cp iyh", 0xBC> { let Uses = [A, IYH]; let Defs = [FLAGS]; }
def CP_IYL : InstFD8<"cp iyl", 0xBD> { let Uses = [A, IYL]; let Defs = [FLAGS]; }

} // Predicates = [HasZ80, HasUndocumented]

//===----------------------------------------------------------------------===//
// Z80 Spill Pseudos
// These are expanded in expandPostRAPseudo after physical registers are assigned
//===----------------------------------------------------------------------===//

// Base class for Z80 pseudo instructions
class Z80Pseudo<dag outs, dag ins> : Instruction {
  let Namespace = "Z80";
  dag OutOperandList = outs;
  dag InOperandList = ins;
  let hasSideEffects = false;
  let isPseudo = true;
  let isCodeGenOnly = true;
}

// 16-bit register copy via PUSH src; POP dst for IX/IY without +undocumented.
// Emitted as a single MI by copyPhysReg to prevent optimization passes from
// inserting between PUSH and POP, which causes SP-relative addressing
// corruption (issue #32). Expanded to adjacent PUSH/POP in Z80ExpandPseudo
// (which runs after Z80LateOptimization).
def COPY16_PUSHPOP : Z80Pseudo<(outs GR16:$dst), (ins GR16:$src)> {
  let mayStore = true;
  let mayLoad = true;
}

// #27: deferred IX/IY-displacement 8-bit load.  The base is constrained to the
// index class (IR16 = {IX,IY}) at instruction selection so register allocation
// must place the pointer in IX/IY; Z80ExpandPseudo then emits
// `LD r,(IX/IY+disp)`.  This avoids the IX/IY->HL copy + `add hl,bc` + `ld (hl)`
// sequence that arises because the deref form is chosen pre-RA, before the
// IX/IY assignment exists (phase-ordering gap; see issue27 design note).
let mayLoad = 1, mayStore = 0, hasSideEffects = 0 in
def LOAD_IDX8 : Z80Pseudo<(outs GR8:$dst), (ins IR16:$base, i8imm:$disp)>;

// #27: deferred IX/IY-displacement 8-bit store (mirror of LOAD_IDX8).  Together
// they let a read-modify-write (buf[i] = f(buf[i])) drop the address
// computation entirely: `ld r,(iy+d)` ... `ld (iy+d),r`.
let mayLoad = 0, mayStore = 1, hasSideEffects = 0 in
def STORE_IDX8 : Z80Pseudo<(outs), (ins GR8:$src, IR16:$base, i8imm:$disp)>;

// Spill/Reload pseudo instructions for stack slots (frame index).
// No static Defs: small offsets expand to IX+d (no clobbers), large offsets
// are expanded in eliminateFrameIndex with register scavenging.
// Accept Anyi16 (not just GR16) — the allocator may widen the register
// class to Anyi16 via getLargestLegalSuperClass or class intersection.
// The expansion in eliminateFrameIndex handles all physical registers
// in Anyi16 (DE, HL, BC, IX, IY, SP).  See issue #51.
def SPILL_GR16 : Z80Pseudo<(outs), (ins Anyi16:$src, i32imm:$fi, i32imm:$offset)> {
  let mayStore = true;
  let mayLoad = false;
}
def RELOAD_GR16 : Z80Pseudo<(outs Anyi16:$dst), (ins i32imm:$fi, i32imm:$offset)> {
  let mayLoad = true;
  let mayStore = false;
  // Allow rematerialization for loads from immutable stack slots (e.g. incoming
  // arguments). isReMaterializableImpl checks isImmutableObjectIndex, so only
  // fixed-stack args are actually rematerialized — spill slot reloads are not.
  let isReMaterializable = true;
}

// 16-bit add/sub from frame slot via 8-bit IX-indexed ALU.
// ADD_HL_FI expands to: LD A,L; ADD A,(IX+d); LD L,A; LD A,H; ADC A,(IX+d+1); LD H,A
// SUB_HL_FI expands to: LD A,L; SUB (IX+d); LD L,A; LD A,H; SBC A,(IX+d+1); LD H,A
// These avoid allocating a GR16_BCDE register pair, reducing register pressure.
def ADD_HL_FI : Z80Pseudo<(outs), (ins i32imm:$fi)> {
  let Uses = [HL];
  let Defs = [HL, A, FLAGS];
  let mayLoad = true;
  let mayStore = false;
  let Predicates = [HasZ80];
}
def SUB_HL_FI : Z80Pseudo<(outs), (ins i32imm:$fi)> {
  let Uses = [HL];
  let Defs = [HL, A, FLAGS];
  let mayLoad = true;
  let mayStore = false;
  let Predicates = [HasZ80];
}
def SPILL_GR8 : Z80Pseudo<(outs), (ins GR8:$src, i32imm:$fi, i32imm:$offset)> {
  let mayStore = true;
  let mayLoad = false;
}
def SPILL_IMM8 : Z80Pseudo<(outs), (ins i8imm:$val, i32imm:$fi, i32imm:$offset)> {
  let mayStore = true;
  let mayLoad = false;
}
def RELOAD_GR8 : Z80Pseudo<(outs GR8:$dst), (ins i32imm:$fi, i32imm:$offset)> {
  let mayLoad = true;
  let mayStore = false;
  let isReMaterializable = true;
}

// LEA_IX_FI: Compute the effective address of a frame index into a GR16.
// Expanded in eliminateFrameIndex using register scavenging to find
// available scratch registers. No static Defs — temporaries are managed
// dynamically via the RegScavenger (PUSH/POP if live).
def LEA_IX_FI : Z80Pseudo<(outs GR16:$dst), (ins i32imm:$fi)> {
  let mayLoad = false;
  let mayStore = false;
}

// 8-bit indirect load/store via any register pair (BC, DE, or HL).
// Regalloc chooses which pair; expandPostRAPseudo selects the instruction:
//   BC → LD A,(BC) / LD (BC),A
//   DE → LD A,(DE) / LD (DE),A
//   HL → LD A,(HL) / LD (HL),A
// This avoids forcing the address into HL, reducing register pressure.
def LOAD8_IND : Z80Pseudo<(outs), (ins GR16:$addr)> {
  let Defs = [A];
  let mayLoad = true;
  let mayStore = false;
}
def STORE8_IND : Z80Pseudo<(outs), (ins GR16:$addr)> {
  let Uses = [A];
  let mayLoad = false;
  let mayStore = true;
}

// LD_r8_n: Load 8-bit immediate into any GR8 register.
// Expanded in expandPostRAPseudo to the concrete LD_r_n opcode.
def LD_r8_n : Z80Pseudo<(outs GR8:$dst), (ins i8imm:$imm)> {
  let mayLoad = false;
  let mayStore = false;
  let isAsCheapAsAMove = true;
  let isMoveImm = true;
  let isReMaterializable = true;
}

// LD_r16_nn: Load 16-bit immediate into any GR16 register.
// Expanded in expandPostRAPseudo to the concrete LD_rr_nn opcode.
def LD_r16_nn : Z80Pseudo<(outs GR16:$dst), (ins i16imm:$imm)> {
  let mayLoad = false;
  let mayStore = false;
  let isAsCheapAsAMove = true;
  let isMoveImm = true;
  let isReMaterializable = true;
}

// INC/DEC 16-bit pseudo instructions.
// Expanded in expandPostRAPseudo to the concrete INC/DEC opcode.
// Real Z80 INC/DEC rr does NOT touch flags — marking them
// isAsCheapAsAMove + isReMaterializable lets the regalloc recompute
// `base + small_const` pointer chains at the use site instead of
// spilling each intermediate.  S3' of #115/#27 (session 73b).
def INC16 : Z80Pseudo<(outs GR16:$dst), (ins GR16:$src)> {
  let Constraints = "$dst = $src";
  let mayLoad = false;
  let mayStore = false;
  let isAsCheapAsAMove = true;
  let isReMaterializable = true;
}
def DEC16 : Z80Pseudo<(outs GR16:$dst), (ins GR16:$src)> {
  let Constraints = "$dst = $src";
  let mayLoad = false;
  let mayStore = false;
  let isAsCheapAsAMove = true;
  let isReMaterializable = true;
}

// SUB_HL_xx: 16-bit subtraction pseudo instructions.
// Generalized 16-bit arithmetic pseudo instructions.
// The second operand (rr) is a GR16_BCDE virtual register — the register
// allocator is free to assign BC or DE.  expandPostRAPseudo inspects the
// physical register and emits the correct real instruction.
//
// HL is always the implicit first operand (hardware constraint: ADD/ADC/SBC
// HL,rr only supports HL as destination).

let isPseudo = true, isCodeGenOnly = true in {

// ADD HL,rr (plain addition, no carry capture)
//
// #166 attempted to mark this isAsCheapAsAMove + isReMaterializable
// (session 73p).  Byte-identical AES corpus result -- the flag is
// silently ignored because ADD_HL_rr has no virtual output: HL is
// implicitly defined via `Defs = [HL, FLAGS]`, not as an SSA vreg in
// OutOperandList.  Greedy's rematerializer only clones instructions
// that DEFINE a vreg it can rewire; physreg-only defs have no vreg
// to remat.  ADD16_tied (below) is the SSA-shaped sibling but is
// "not yet used by ISel" -- wiring that up is the real #166 path.
def ADD_HL_rr : Z80Inst<"add_hl_rr"> {
  let InOperandList = (ins GR16_BCDE:$rhs);
  let OutOperandList = (outs);
  let Uses = [HL];
  let Defs = [HL, FLAGS];
}

// ADD16_tied: generalized 16-bit add with explicit accumulator register.
// Unlike ADD_HL_rr (which hardcodes HL), this allows the register allocator
// to place the accumulator in any GR16 register including IX/IY.
// expandPostRAPseudo selects the real instruction:
//   HL → ADD HL,rr (1B)
//   IX → ADD IX,rr (2B)
//   IY → ADD IY,rr (2B)
//   BC/DE → COPY to HL, ADD HL,rr, COPY back (fallback)
// RHS is constrained to GR16_BCDE (BC or DE) by the ISel.
def ADD16_tied : Z80Pseudo<(outs GR16:$dst), (ins GR16:$src, GR16_BCDE:$rhs)> {
  let Constraints = "$dst = $src";
  let Defs = [FLAGS];
  let mayLoad = false;
  let mayStore = false;
}

// ADD16_acc (#178): NON-tied accumulator add.  $dst = $base + $rhs.
//
// Unlike ADD16_tied this has NO tie between $dst and the base operand, so
// the RegisterCoalescer has no SSA copy to fold the (often-surviving) base
// into the HLI-classed result -- which is what made ADD16_tied assign an
// out-of-class physreg (BC) to $dst and miscompile (session 73s
// root-cause).  $dst stays in HLI (-> HL, since IX/IY are reserved) and the
// base lives independently in GR16.  expandPostRAPseudo materialises it as
//   COPY $dst <- $base   (skipped if regalloc already placed them together)
//   ADD  $dst, $rhs      (ADD HL/IX/IY,rr)
// $dst is a real SSA vreg, so the pseudo can be marked isReMaterializable
// to let greedy clone "$dst = base + off" at a use point instead of
// spilling -- the #166/#178 win, attacking the base-reuse copy traffic #27
// measured.
def ADD16_acc : Z80Pseudo<(outs HLI:$dst), (ins GR16:$base, GR16_BCDE:$rhs)> {
  let Defs = [FLAGS];
  let mayLoad = false;
  let mayStore = false;
  // SSA $dst is the prerequisite for remat (ADD_HL_rr's physreg output is
  // not).  HOWEVER session-73s A/B showed remat-on and remat-off produce
  // byte-identical AES: the implicit FLAGS def blocks LLVM's rematerializer
  // in practice (16-bit ADD always clobbers H/N/C; dropping the FLAGS def
  // would miscompile flag-dependent code).  Kept true to document intent;
  // it currently never fires.  See session73s-issue178-add16-tied-rootcause.md.
  let isReMaterializable = true;
  let isAsCheapAsAMove = true;
}

// AND A; SBC HL,rr (subtraction without borrow)
def SUB_HL_rr : Z80Inst<"sub_hl_rr"> {
  let InOperandList = (ins GR16_BCDE:$rhs);
  let OutOperandList = (outs);
  let Uses = [HL];
  let Defs = [HL, A, FLAGS];
}

// AND A; ADC HL,rr (signed addition — sets P/V for overflow detection)
def SADD_HL_rr : Z80Inst<"sadd_hl_rr"> {
  let InOperandList = (ins GR16_BCDE:$rhs);
  let OutOperandList = (outs);
  let Uses = [HL];
  let Defs = [HL, A, FLAGS];
}

// ADD HL,rr; SBC A,A; AND 1 — carry out in A
def ADD_HL_rr_CO : Z80Inst<"add_hl_rr_co"> {
  let InOperandList = (ins GR16_BCDE:$rhs);
  let OutOperandList = (outs);
  let Uses = [HL];
  let Defs = [HL, A, FLAGS];
}

// AND A; SBC HL,rr; SBC A,A; AND 1 — borrow out in A
def SUB_HL_rr_BO : Z80Inst<"sub_hl_rr_bo"> {
  let InOperandList = (ins GR16_BCDE:$rhs);
  let OutOperandList = (outs);
  let Uses = [HL];
  let Defs = [HL, A, FLAGS];
}

// LD A,carry; RRCA; ADC HL,rr; SBC A,A; AND 1
def ADC_HL_rr_CIO : Z80Inst<"adc_hl_rr_cio"> {
  let InOperandList = (ins GR16_BCDE:$rhs, GR8:$carry_in);
  let OutOperandList = (outs);
  let Uses = [HL];
  let Defs = [HL, A, FLAGS];
}

// LD A,borrow; RRCA; SBC HL,rr; SBC A,A; AND 1
def SBC_HL_rr_BIO : Z80Inst<"sbc_hl_rr_bio"> {
  let InOperandList = (ins GR16_BCDE:$rhs, GR8:$borrow_in);
  let OutOperandList = (outs);
  let Uses = [HL];
  let Defs = [HL, A, FLAGS];
}

// Generalized 8-bit ALU pseudo instructions.
// The second operand is a GR8 virtual register — the register allocator is
// free to assign any of {A,B,C,D,E,H,L}.  expandPostRAPseudo inspects the
// physical register and emits the correct real instruction (ADD_A_A/ADD_A_B/
// ADD_A_C/etc).
//
// A is always the implicit first operand (hardware constraint: all 8-bit ALU
// ops use A as the accumulator).

def ADD_A_r : Z80Inst<"add_a_r"> {
  let InOperandList = (ins GR8:$rhs);
  let OutOperandList = (outs);
  let Uses = [A];
  let Defs = [A, FLAGS];
}

def SUB_r : Z80Inst<"sub_r"> {
  let InOperandList = (ins GR8:$rhs);
  let OutOperandList = (outs);
  let Uses = [A];
  let Defs = [A, FLAGS];
}

def AND_r : Z80Inst<"and_r"> {
  let InOperandList = (ins GR8:$rhs);
  let OutOperandList = (outs);
  let Uses = [A];
  let Defs = [A, FLAGS];
}

def OR_r : Z80Inst<"or_r"> {
  let InOperandList = (ins GR8:$rhs);
  let OutOperandList = (outs);
  let Uses = [A];
  let Defs = [A, FLAGS];
}

def XOR_r : Z80Inst<"xor_r"> {
  let InOperandList = (ins GR8:$rhs);
  let OutOperandList = (outs);
  let Uses = [A];
  let Defs = [A, FLAGS];
}

def CP_r : Z80Inst<"cp_r"> {
  let InOperandList = (ins GR8:$rhs);
  let OutOperandList = (outs);
  let Uses = [A];
  let Defs = [FLAGS];
}

} // isPseudo, isCodeGenOnly

// CAPTURE_PV: Read the P/V flag (bit 2 of F register) into A as 0 or 1.
// Expanded in expandPostRAPseudo to: PUSH AF; POP HL; LD A,L; RRCA; RRCA; AND 1
// Must be atomic (single pseudo) because the RA must not insert stack-modifying
// instructions between the PUSH AF and POP HL.
let isPseudo = true, isCodeGenOnly = true in {
def CAPTURE_PV : Z80Inst<"capture_pv"> {
  let InOperandList = (ins);
  let OutOperandList = (outs);
  let Uses = [FLAGS];
  let Defs = [A, HL, FLAGS];
}
}

// Zero extend 8-bit to 16-bit.
// Expanded in expandPostRAPseudo: LD lo,src; LD hi,0
def ZEXT_GR8_GR16 : Z80Pseudo<(outs GR16:$dst), (ins GR8:$src)> {
  let mayLoad = false;
  let mayStore = false;
}

// Sign extend 8-bit to 16-bit.
// Expanded in expandPostRAPseudo: LD A,src; LD lo,A; RLCA; SBC A,A; LD hi,A
def SEXT_GR8_GR16 : Z80Pseudo<(outs GR16:$dst), (ins GR8:$src)> {
  let Defs = [A, FLAGS];
  let mayLoad = false;
  let mayStore = false;
}

// 16-bit unsigned comparison pseudo-instructions using 8-bit SUB/SBC chain.
// LD A,lhs_lo; SUB rhs_lo; LD A,lhs_hi; SBC A,rhs_hi
// Carry flag = unsigned less-than. Does NOT clobber HL/DE (unlike SBC HL,DE).
// Both operands can be any GR16 (no GR16_BCDE restriction).

// CMP16_FLAGS: sets carry flag for unsigned comparison. Used for fused branch.
// GR16NoIR (#113): expansion uses `getLD8RegOpcode(A, lhs_half)` and
// `getSUBOpcode(rhs_half)` which would emit undocumented IXH/IXL/IYH/IYL
// ops if IX/IY were allocated.  No PUSH/POP fallback exists for these,
// so the only safe path is to forbid IX/IY operands by class.
def CMP16_FLAGS : Z80Pseudo<(outs), (ins GR16NoIR:$lhs, GR16NoIR:$rhs)> {
  let Defs = [A, FLAGS];
  let mayLoad = false;
  let mayStore = false;
}

// CMP16_ULT: materializes unsigned less-than as 0/1 in A.
// Appends SBC A,A; AND 1 after the SUB/SBC chain.
// GR16NoIR — same rationale as CMP16_FLAGS (#113).
def CMP16_ULT : Z80Pseudo<(outs), (ins GR16NoIR:$lhs, GR16NoIR:$rhs)> {
  let Defs = [A, FLAGS];
  let mayLoad = false;
  let mayStore = false;
}

// CMP16_SBC_FLAGS: carry-chain continuation for multi-word comparison.
// LD A,lhs_lo; SBC A,rhs_lo; LD A,lhs_hi; SBC A,rhs_hi
// Uses carry-in from previous CMP16_FLAGS or CMP16_SBC_FLAGS.
// Uses = [FLAGS] ensures regalloc won't insert flag-clobbering ops in between.
// GR16NoIR — same rationale as CMP16_FLAGS (#113).
def CMP16_SBC_FLAGS : Z80Pseudo<(outs), (ins GR16NoIR:$lhs, GR16NoIR:$rhs)> {
  let Defs = [A, FLAGS];
  let Uses = [FLAGS];
  let mayLoad = false;
  let mayStore = false;
}

// 16-bit XOR-based equality comparison pseudo-instructions.
// These compare two 16-bit registers using byte-level XOR and produce 0/1 in A.
// Expanded in expandPostRAPseudo after register allocation, when physical
// registers are known and sub-register extraction is safe.
// Uses A, B as scratch. Does NOT clobber HL/DE (unlike SBC HL,DE).
// GR16NoIR (#113): expansion calls `getLD8RegOpcode(A, lhs_hi)` and
// `getXOROpcode(rhs_hi)` which would emit undocumented IXH/IXL/IYH/IYL
// XOR ops if IX/IY were allocated.  Force regalloc to land operands in
// BC/DE/HL so the standard expansion path is the only path.
def XOR_CMP_EQ16 : Z80Pseudo<(outs), (ins GR16NoIR:$lhs, GR16NoIR:$rhs)> {
  let Defs = [A, B, FLAGS];
  let mayLoad = false;
  let mayStore = false;
}
def XOR_CMP_NE16 : Z80Pseudo<(outs), (ins GR16NoIR:$lhs, GR16NoIR:$rhs)> {
  let Defs = [A, B, FLAGS];
  let mayLoad = false;
  let mayStore = false;
}

// 16-bit XOR-based equality comparison — sets Z flag directly.
// XORs both bytes, ORs results: Z=1 if equal, Z=0 if not.
// Same as SM83_CMP_Z16 but available on Z80.
// Used in fused compare-and-branch to avoid 0/1 normalize overhead.
// 6 bytes: LD A,hi; XOR hi'; LD B,A; LD A,lo; XOR lo'; OR B
// GR16NoIR — same rationale as XOR_CMP_EQ16 (#113).
def XOR_CMP_Z16 : Z80Pseudo<(outs), (ins GR16NoIR:$lhs, GR16NoIR:$rhs)> {
  let Defs = [A, B, FLAGS];
  let mayLoad = false;
  let mayStore = false;
}

// 16-bit shift right pseudo-instructions.
// Expanded in expandPostRAPseudo to SRL/SRA hi + RR lo.
// GR16NoIR (#112): operand cannot be IX/IY because the expansion calls
// `getSRLOpcode(IYH)` / `getRROpcode(IYL)` which return 0 for the index
// halves, producing bare opcode-0 MIs that the encoder rejects.
def LSHR16 : Z80Pseudo<(outs GR16NoIR:$dst), (ins GR16NoIR:$src)> {
  let Constraints = "$dst = $src";
  let Defs = [FLAGS];
  let mayLoad = false;
  let mayStore = false;
}
def ASHR16 : Z80Pseudo<(outs GR16NoIR:$dst), (ins GR16NoIR:$src)> {
  let Constraints = "$dst = $src";
  let Defs = [FLAGS];
  let mayLoad = false;
  let mayStore = false;
}

// 16-bit sign extension: result = 0x0000 (positive) or 0xFFFF (negative)
// Expanded to: LD A,src_hi; ADD A,A; SBC A,A; LD dst_lo,A; LD dst_hi,A
// Used for G_ASHR by type_size-1 (e.g., ASHR i16, 15 for sext i16->i32)
def SEXT16 : Z80Pseudo<(outs GR16:$dst), (ins GR16:$src)> {
  let Defs = [A, FLAGS];
  let mayLoad = false;
  let mayStore = false;
}

// Variable shift pseudos - expanded to DJNZ loops in expandPostRAPseudo.
// Pattern: INC B; DEC B; JR Z,done; .loop: <shift>; DJNZ .loop; .done:
// 8-bit: value in A, count in B, result in A. B is decremented to 0.
let mayLoad = false, mayStore = false in {
def SHL8_VAR : Z80Pseudo<(outs), (ins)> {
  let Uses = [A, B];
  let Defs = [A, B, FLAGS];
}
def LSHR8_VAR : Z80Pseudo<(outs), (ins)> {
  let Uses = [A, B];
  let Defs = [A, B, FLAGS];
}
def ASHR8_VAR : Z80Pseudo<(outs), (ins)> {
  let Uses = [A, B];
  let Defs = [A, B, FLAGS];
}
def ROTL8_VAR : Z80Pseudo<(outs), (ins)> {
  let Uses = [A, B];
  let Defs = [A, B, FLAGS];
}
def ROTR8_VAR : Z80Pseudo<(outs), (ins)> {
  let Uses = [A, B];
  let Defs = [A, B, FLAGS];
}
// 16-bit: value in HL, count in B, result in HL. B is decremented to 0.
def SHL16_VAR : Z80Pseudo<(outs), (ins)> {
  let Uses = [HL, B];
  let Defs = [HL, B, FLAGS];
}
def LSHR16_VAR : Z80Pseudo<(outs), (ins)> {
  let Uses = [HL, B];
  let Defs = [HL, B, FLAGS];
}
def ASHR16_VAR : Z80Pseudo<(outs), (ins)> {
  let Uses = [HL, B];
  let Defs = [HL, B, FLAGS];
}
// 8-bit multiply pseudo - expanded to DJNZ shift-add loop.
// Input: A = multiplier, E = multiplicand. Output: A = result (low 8 bits).
// Loop body: ADD A,A; RL D; JR NC,skip; ADD A,E; DJNZ loop
def MUL8 : Z80Pseudo<(outs), (ins)> {
  let Uses = [A, E];
  let Defs = [A, B, D, FLAGS];
}
// 8-bit unsigned division pseudo - expanded to DJNZ restoring division loop.
// Input: A = dividend, E = divisor. Output: A = quotient.
// After loop: D = quotient, A = remainder; UDIV8 returns D, UMOD8 returns A.
def UDIV8 : Z80Pseudo<(outs), (ins)> {
  let Uses = [A, E];
  let Defs = [A, B, D, FLAGS];
}
def UMOD8 : Z80Pseudo<(outs), (ins)> {
  let Uses = [A, E];
  let Defs = [A, B, D, FLAGS];
}
// 8-bit signed division pseudo - sign-handles operands then restoring division.
// Input: A = dividend, E = divisor. Output: A = quotient (SDIV8) or remainder (SMOD8).
def SDIV8 : Z80Pseudo<(outs), (ins)> {
  let Uses = [A, E];
  let Defs = [A, B, C, D, E, FLAGS];
}
def SMOD8 : Z80Pseudo<(outs), (ins)> {
  let Uses = [A, E];
  let Defs = [A, B, C, D, E, FLAGS];
}
// 16-bit multiply pseudo (for +inline-i16-runtime mode).
// Input: HL = multiplicand, DE = multiplier. Output: DE = result.
// Expanded to 16-iteration shift-add loop in Z80ExpandPseudo.
def MUL16 : Z80Pseudo<(outs), (ins)> {
  let Uses = [HL, DE];
  let Defs = [A, B, C, DE, HL, FLAGS];
}
// 16-bit unsigned division pseudo (for +inline-i16-runtime mode).
// Input: HL = dividend, DE = divisor. Output: DE = quotient.
def UDIV16 : Z80Pseudo<(outs), (ins)> {
  let Uses = [HL, DE];
  let Defs = [A, B, C, DE, HL, FLAGS];
}
// 16-bit unsigned modulo pseudo. Output: DE = remainder.
def UMOD16 : Z80Pseudo<(outs), (ins)> {
  let Uses = [HL, DE];
  let Defs = [A, B, C, DE, HL, FLAGS];
}
// 16-bit signed division pseudo. Output: DE = quotient.
def SDIV16 : Z80Pseudo<(outs), (ins)> {
  let Uses = [HL, DE];
  let Defs = [A, B, C, DE, HL, FLAGS];
}
// 16-bit signed modulo pseudo. Output: DE = remainder.
def SMOD16 : Z80Pseudo<(outs), (ins)> {
  let Uses = [HL, DE];
  let Defs = [A, B, C, DE, HL, FLAGS];
}
// LDIR_GUARDED / LDDR_GUARDED / MEMSET_LDIR_GUARDED — runtime BC==0
// guard around block-move / block-fill instructions (issue #105).
//
// Z80 LDIR / LDDR test BC AFTER decrement: with BC=0 they run 65536
// iterations and trash 64 KB.  When the legalizer can't statically
// prove BC > 0 (variable-size memcpy / memmove / memset), it must
// emit a guarded form that skips the block instruction at run-time.
// Expansion at post-RA in Z80ExpandPseudo splits the MBB and emits:
//
//   head:        LD A,B; OR C; JR Z, tail
//   body:        LDIR    (or LDDR; or LD (HL),A + DE setup + LDIR for memset)
//   tail:        ...
//
// LDIR_GUARDED: HL=src, DE=dst, BC=count.  Skips LDIR when count==0.
// LDDR_GUARDED: HL=src+count-1, DE=dst+count-1, BC=count.  Skips LDDR
//               when count==0.
// MEMSET_LDIR_GUARDED: HL=dst, E=val, BC=size.  Skips entire fill
//   (leading store + LDIR) when size==0.  Body for size>=1:
//     LD (HL),E ; first byte
//     DEC BC    ; BC=size-1
//     LD A,B; OR C; JR Z, tail   ; size==1 case: LDIR with BC=0 forbidden
//     LD D,H; LD E,L; INC DE     ; DE = HL + 1
//     LDIR
//   Note: E carries val; we deliberately do not put val in A so the
//   BC test (LD A,B; OR C) does not clobber it.
def LDIR_GUARDED : Z80Pseudo<(outs), (ins)> {
  let Uses = [HL, DE, BC];
  let Defs = [A, HL, DE, BC, FLAGS];
  let mayLoad = true;
  let mayStore = true;
}
def LDDR_GUARDED : Z80Pseudo<(outs), (ins)> {
  let Uses = [HL, DE, BC];
  let Defs = [A, HL, DE, BC, FLAGS];
  let mayLoad = true;
  let mayStore = true;
}
def MEMSET_LDIR_GUARDED : Z80Pseudo<(outs), (ins)> {
  let Uses = [HL, E, BC];
  let Defs = [A, HL, DE, BC, FLAGS];
  let mayLoad = true;
  let mayStore = true;
}

// 8-bit unsigned saturating add pseudo.
// ADD A,src; JR NC,.done; LD A,#0xFF; .done:
def UADDSAT8 : Z80Pseudo<(outs), (ins GR8:$src)> {
  let Uses = [A];
  let Defs = [A, FLAGS];
}
// 8-bit unsigned saturating subtract pseudo.
// SUB src; JR NC,.done; XOR A; .done:
def USUBSAT8 : Z80Pseudo<(outs), (ins GR8:$src)> {
  let Uses = [A];
  let Defs = [A, FLAGS];
}
// 8-bit signed saturating add pseudo (Z80 only — uses P/V overflow flag).
// ADD A,src; JP PO,.done; RLCA; SBC A,A; XOR #0x80; .done:
def SADDSAT8 : Z80Pseudo<(outs), (ins GR8:$src)> {
  let Uses = [A];
  let Defs = [A, FLAGS];
}
// 8-bit signed saturating subtract pseudo (Z80 only — uses P/V overflow flag).
// SUB src; JP PO,.done; RLCA; SBC A,A; XOR #0x80; .done:
def SSUBSAT8 : Z80Pseudo<(outs), (ins GR8:$src)> {
  let Uses = [A];
  let Defs = [A, FLAGS];
}
} // mayLoad = false, mayStore = false

// CALL_HL - SM83 indirect call through HL register.
// Pseudo-instruction that expands to "call __call_hl" in expandPostRAPseudo.
// The runtime __call_hl trampoline is just "jp (hl)" (1 byte).
// No register shift needed: SM83 args are in DE/BC, so HL is free for
// the function pointer.
let isPseudo = true in
def CALL_HL : Z80Inst<"call\t__call_hl">, InstCall {
  let InOperandList = (ins);
  let Uses = [HL];
  let Defs = [A, BC, DE, HL, FLAGS];
}

// Call frame setup/destroy pseudos for stack argument passing.
// These bracket call sequences where arguments are pushed onto the stack.
// ADJCALLSTACKDOWN is erased (PUSHes adjust SP), ADJCALLSTACKUP emits
// SP cleanup code in eliminateCallFramePseudoInstr.
let Defs = [SP], Uses = [SP], hasSideEffects = true,
    mayLoad = false, mayStore = false in {
def ADJCALLSTACKDOWN : Z80Pseudo<(outs), (ins i16imm:$bytes, i16imm:$prior)>;
}
// ADJCALLSTACKUP may clobber HL (via LD HL,N; ADD HL,SP; LD SP,HL for large
// amounts) and A (via POP AF for small amounts).
let Defs = [SP, HL, A], Uses = [SP], hasSideEffects = true,
    mayLoad = false, mayStore = false in {
def ADJCALLSTACKUP : Z80Pseudo<(outs), (ins i16imm:$bytes, i16imm:$prior)>;
}

// Callee-cleanup return: pops return address, adjusts SP by $bytes, then
// returns via indirect jump. Used by __sdcccall(1) callee-cleanup and
// future __z88dk_callee convention.
let isPseudo = true, isReturn = true, isTerminator = true, isBarrier = true,
    Defs = [SP, HL, BC], Uses = [SP], hasSideEffects = true,
    mayLoad = false, mayStore = false in {
def RET_CLEANUP : Z80Pseudo<(outs), (ins i16imm:$bytes)>;
}


// Tail call: JP to an external function as a return.
// NOT marked isBranch — getBranchDestBlock/BranchCleanup don't process it.
// Lowered to JP_nn in MCInstLowering (Z80AsmPrinter).
let isPseudo = true, isReturn = true, isTerminator = true, isBarrier = true,
    isCall = true, mayLoad = false, mayStore = false in {
def TAILJMP : Z80Pseudo<(outs), (ins i16imm:$target)>;
}

// SM83-only instructions and pseudos
include "SM83InstrInfo.td"

// GlobalISel pseudo-instructions for code generation
include "Z80InstrGISel.td"
