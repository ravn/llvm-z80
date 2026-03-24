# Z80 Complete Instruction Set Reference

A comprehensive reference for the Z80 instruction set including all undocumented instructions,
organized for compiler backend implementers. This document covers every prefix group and
focuses on what is needed to make IX/IY fully allocatable as general-purpose 16-bit registers
with optional 8-bit sub-register access (IXH/IXL, IYH/IYL).

## Sources

- Sean Young, "The Undocumented Z80 Documented" v0.91 (2005) -- http://www.z80.info/zip/z80-documented.pdf
- Grauw's Z80/R800 instruction set -- https://map.grauw.nl/resources/z80instr.php
- z80.info undocumented instructions -- http://www.z80.info/z80undoc.htm
- z80.info opcode decoding -- http://www.z80.info/decoding.htm
- ClrHome Z80 opcode table -- https://clrhome.org/table/
- CPC Wiki undocumented opcodes -- https://www.cpcwiki.eu/index.php/Z80_-_undocumented_opcodes
- Zilog Z80 User Manual (UM0080) -- https://www.zilog.com/docs/z80/um0080.pdf

## Notation

- `r` = 8-bit register: B(0), C(1), D(2), E(3), H(4), L(5), (HL)(6), A(7)
- `p` = 8-bit register with IX substitution: B(0), C(1), D(2), E(3), IXH(4), IXL(5), ---(6), A(7)
- `q` = 8-bit register with IY substitution: B(0), C(1), D(2), E(3), IYH(4), IYL(5), ---(6), A(7)
- `rr` = 16-bit register pair: BC(0), DE(1), HL(2), SP(3)
- `n` = 8-bit immediate
- `nn` = 16-bit immediate (little-endian)
- `d` = 8-bit signed displacement (-128 to +127)
- `b` = bit number (0-7)
- `cc` = condition: NZ(0), Z(1), NC(2), C(3), PO(4), PE(5), P(6), M(7)
- T = T-states (clock cycles)
- Sz = size in bytes
- (U) = Undocumented, (D) = Documented

## Prefix System Overview

The Z80 uses four prefix bytes that modify instruction interpretation:

| Prefix | Hex  | Purpose |
|--------|------|---------|
| CB     | 0xCB | Bit manipulation, rotate/shift |
| DD     | 0xDD | IX register operations (replaces HL, H, L) |
| FD     | 0xFD | IY register operations (replaces HL, H, L) |
| ED     | 0xED | Extended operations (block, I/O, 16-bit ALU) |
| DD CB  | 0xDDCB | Indexed bit operations on (IX+d) |
| FD CB  | 0xFDCB | Indexed bit operations on (IY+d) |

### DD/FD Prefix Substitution Rules

When a DD or FD prefix precedes an opcode:
1. **HL -> IX/IY**: 16-bit register pair HL becomes IX or IY
2. **(HL) -> (IX+d)/(IY+d)**: Memory indirect via HL becomes indexed with displacement byte
3. **H -> IXH/IYH, L -> IXL/IYL**: 8-bit halves become index register halves **(UNDOCUMENTED)**

**Critical exception**: Rules 2 and 3 are mutually exclusive. An opcode that uses (HL) gets rule 2 (indexed addressing), NOT rules 3 for H/L in the same instruction. For example, `LD H,(IX+d)` loads from indexed memory into the real H register, not IXH.

### Multiple Prefix Behavior

Stacking DD/FD prefixes: only the last one takes effect. `DD DD DD 21 nn nn` = `LD IX,nn`.
DD/FD followed by ED: the DD/FD is ignored, ED instruction executes normally.

---

## 1. DD-Prefixed Instructions (IX Variants)

### 1.1 Documented 16-bit IX Operations

| Mnemonic | Opcode | Sz | T | Base Instruction | Change |
|----------|--------|----|---|------------------|--------|
| ADD IX,BC | DD 09 | 2 | 15 | ADD HL,BC (09) | HL->IX |
| ADD IX,DE | DD 19 | 2 | 15 | ADD HL,DE (19) | HL->IX |
| ADD IX,IX | DD 29 | 2 | 15 | ADD HL,HL (29) | HL->IX |
| ADD IX,SP | DD 39 | 2 | 15 | ADD HL,SP (39) | HL->IX |
| LD IX,nn | DD 21 nn nn | 4 | 14 | LD HL,nn (21) | HL->IX |
| LD IX,(nn) | DD 2A nn nn | 4 | 20 | LD HL,(nn) (2A) | HL->IX |
| LD (nn),IX | DD 22 nn nn | 4 | 20 | LD (nn),HL (22) | HL->IX |
| INC IX | DD 23 | 2 | 10 | INC HL (23) | HL->IX |
| DEC IX | DD 2B | 2 | 10 | DEC HL (2B) | HL->IX |
| PUSH IX | DD E5 | 2 | 15 | PUSH HL (E5) | HL->IX |
| POP IX | DD E1 | 2 | 14 | POP HL (E1) | HL->IX |
| EX (SP),IX | DD E3 | 2 | 23 | EX (SP),HL (E3) | HL->IX |
| JP (IX) | DD E9 | 2 | 8 | JP (HL) (E9) | HL->IX |
| LD SP,IX | DD F9 | 2 | 10 | LD SP,HL (F9) | HL->IX |

### 1.2 Documented Indexed Addressing: (IX+d)

These instructions replace (HL) with (IX+d), adding a displacement byte.

| Mnemonic | Opcode | Sz | T | Base Instruction |
|----------|--------|----|---|------------------|
| LD B,(IX+d) | DD 46 d | 3 | 19 | LD B,(HL) (46) |
| LD C,(IX+d) | DD 4E d | 3 | 19 | LD C,(HL) (4E) |
| LD D,(IX+d) | DD 56 d | 3 | 19 | LD D,(HL) (56) |
| LD E,(IX+d) | DD 5E d | 3 | 19 | LD E,(HL) (5E) |
| LD H,(IX+d) | DD 66 d | 3 | 19 | LD H,(HL) (66) |
| LD L,(IX+d) | DD 6E d | 3 | 19 | LD L,(HL) (6E) |
| LD A,(IX+d) | DD 7E d | 3 | 19 | LD A,(HL) (7E) |
| LD (IX+d),B | DD 70 d | 3 | 19 | LD (HL),B (70) |
| LD (IX+d),C | DD 71 d | 3 | 19 | LD (HL),C (71) |
| LD (IX+d),D | DD 72 d | 3 | 19 | LD (HL),D (72) |
| LD (IX+d),E | DD 73 d | 3 | 19 | LD (HL),E (73) |
| LD (IX+d),H | DD 74 d | 3 | 19 | LD (HL),H (74) |
| LD (IX+d),L | DD 75 d | 3 | 19 | LD (HL),L (75) |
| LD (IX+d),A | DD 77 d | 3 | 19 | LD (HL),A (77) |
| LD (IX+d),n | DD 36 d n | 4 | 19 | LD (HL),n (36) |
| ADD A,(IX+d) | DD 86 d | 3 | 19 | ADD A,(HL) (86) |
| ADC A,(IX+d) | DD 8E d | 3 | 19 | ADC A,(HL) (8E) |
| SUB (IX+d) | DD 96 d | 3 | 19 | SUB (HL) (96) |
| SBC A,(IX+d) | DD 9E d | 3 | 19 | SBC A,(HL) (9E) |
| AND (IX+d) | DD A6 d | 3 | 19 | AND (HL) (A6) |
| XOR (IX+d) | DD AE d | 3 | 19 | XOR (HL) (AE) |
| OR (IX+d) | DD B6 d | 3 | 19 | OR (HL) (B6) |
| CP (IX+d) | DD BE d | 3 | 19 | CP (HL) (BE) |
| INC (IX+d) | DD 34 d | 3 | 23 | INC (HL) (34) |
| DEC (IX+d) | DD 35 d | 3 | 23 | DEC (HL) (35) |

**Note on LD H,(IX+d) and LD L,(IX+d)**: These load into the REAL H/L registers, not IXH/IXL. The (HL)->indexed substitution takes priority over the H/L->IXH/IXL substitution. This is important for the compiler backend -- you cannot use indexed addressing and IXH/IXL in the same instruction.

### 1.3 Undocumented 8-bit IXH/IXL Operations

These are undocumented but work reliably on all NMOS and CMOS Z80 chips. They arise because the DD prefix also substitutes H->IXH and L->IXL for opcodes that reference H or L (but not (HL)).

#### IXH/IXL Load Instructions (U)

| Mnemonic | Opcode | Sz | T | Base Instruction | Status |
|----------|--------|----|---|------------------|--------|
| LD IXH,B | DD 60 | 2 | 8 | LD H,B (60) | (U) |
| LD IXH,C | DD 61 | 2 | 8 | LD H,C (61) | (U) |
| LD IXH,D | DD 62 | 2 | 8 | LD H,D (62) | (U) |
| LD IXH,E | DD 63 | 2 | 8 | LD H,E (63) | (U) |
| LD IXH,IXH | DD 64 | 2 | 8 | LD H,H (64) | (U) |
| LD IXH,IXL | DD 65 | 2 | 8 | LD H,L (65) | (U) |
| LD IXH,A | DD 67 | 2 | 8 | LD H,A (67) | (U) |
| LD IXL,B | DD 68 | 2 | 8 | LD L,B (68) | (U) |
| LD IXL,C | DD 69 | 2 | 8 | LD L,C (69) | (U) |
| LD IXL,D | DD 6A | 2 | 8 | LD L,D (6A) | (U) |
| LD IXL,E | DD 6B | 2 | 8 | LD L,E (6B) | (U) |
| LD IXL,IXH | DD 6C | 2 | 8 | LD L,H (6C) | (U) |
| LD IXL,IXL | DD 6D | 2 | 8 | LD L,L (6D) | (U) |
| LD IXL,A | DD 6F | 2 | 8 | LD L,A (6F) | (U) |
| LD B,IXH | DD 44 | 2 | 8 | LD B,H (44) | (U) |
| LD B,IXL | DD 45 | 2 | 8 | LD B,L (45) | (U) |
| LD C,IXH | DD 4C | 2 | 8 | LD C,H (4C) | (U) |
| LD C,IXL | DD 4D | 2 | 8 | LD C,L (4D) | (U) |
| LD D,IXH | DD 54 | 2 | 8 | LD D,H (54) | (U) |
| LD D,IXL | DD 55 | 2 | 8 | LD D,L (55) | (U) |
| LD E,IXH | DD 5C | 2 | 8 | LD E,H (5C) | (U) |
| LD E,IXL | DD 5D | 2 | 8 | LD E,L (5D) | (U) |
| LD A,IXH | DD 7C | 2 | 8 | LD A,H (7C) | (U) |
| LD A,IXL | DD 7D | 2 | 8 | LD A,L (7D) | (U) |
| LD IXH,n | DD 26 n | 3 | 11 | LD H,n (26) | (U) |
| LD IXL,n | DD 2E n | 3 | 11 | LD L,n (2E) | (U) |

#### IXH/IXL Increment/Decrement (U)

| Mnemonic | Opcode | Sz | T | Base Instruction | Status |
|----------|--------|----|---|------------------|--------|
| INC IXH | DD 24 | 2 | 8 | INC H (24) | (U) |
| DEC IXH | DD 25 | 2 | 8 | DEC H (25) | (U) |
| INC IXL | DD 2C | 2 | 8 | INC L (2C) | (U) |
| DEC IXL | DD 2D | 2 | 8 | DEC L (2D) | (U) |

#### IXH/IXL Arithmetic/Logic with A (U)

| Mnemonic | Opcode | Sz | T | Base Instruction | Status |
|----------|--------|----|---|------------------|--------|
| ADD A,IXH | DD 84 | 2 | 8 | ADD A,H (84) | (U) |
| ADD A,IXL | DD 85 | 2 | 8 | ADD A,L (85) | (U) |
| ADC A,IXH | DD 8C | 2 | 8 | ADC A,H (8C) | (U) |
| ADC A,IXL | DD 8D | 2 | 8 | ADC A,L (8D) | (U) |
| SUB IXH | DD 94 | 2 | 8 | SUB H (94) | (U) |
| SUB IXL | DD 95 | 2 | 8 | SUB L (95) | (U) |
| SBC A,IXH | DD 9C | 2 | 8 | SBC A,H (9C) | (U) |
| SBC A,IXL | DD 9D | 2 | 8 | SBC A,L (9D) | (U) |
| AND IXH | DD A4 | 2 | 8 | AND H (A4) | (U) |
| AND IXL | DD A5 | 2 | 8 | AND L (A5) | (U) |
| XOR IXH | DD AC | 2 | 8 | XOR H (AC) | (U) |
| XOR IXL | DD AD | 2 | 8 | XOR L (AD) | (U) |
| OR IXH | DD B4 | 2 | 8 | OR H (B4) | (U) |
| OR IXL | DD B5 | 2 | 8 | OR L (B5) | (U) |
| CP IXH | DD BC | 2 | 8 | CP H (BC) | (U) |
| CP IXL | DD BD | 2 | 8 | CP L (BD) | (U) |

### 1.4 DD Prefix on Other Opcodes (NOPs)

Any DD-prefixed opcode not listed above executes as the base opcode (ignoring the DD prefix), taking an extra 4 T-states. The R register increments by 1 for the prefix. These are effectively expensive NOPs and should NOT be used.

---

## 2. FD-Prefixed Instructions (IY Variants)

FD-prefixed instructions are exactly parallel to DD-prefixed ones, with IY replacing IX, IYH replacing IXH, and IYL replacing IXL.

### 2.1 Documented 16-bit IY Operations

| Mnemonic | Opcode | Sz | T | Base Instruction | Change |
|----------|--------|----|---|------------------|--------|
| ADD IY,BC | FD 09 | 2 | 15 | ADD HL,BC (09) | HL->IY |
| ADD IY,DE | FD 19 | 2 | 15 | ADD HL,DE (19) | HL->IY |
| ADD IY,IY | FD 29 | 2 | 15 | ADD HL,HL (29) | HL->IY |
| ADD IY,SP | FD 39 | 2 | 15 | ADD HL,SP (39) | HL->IY |
| LD IY,nn | FD 21 nn nn | 4 | 14 | LD HL,nn (21) | HL->IY |
| LD IY,(nn) | FD 2A nn nn | 4 | 20 | LD HL,(nn) (2A) | HL->IY |
| LD (nn),IY | FD 22 nn nn | 4 | 20 | LD (nn),HL (22) | HL->IY |
| INC IY | FD 23 | 2 | 10 | INC HL (23) | HL->IY |
| DEC IY | FD 2B | 2 | 10 | DEC HL (2B) | HL->IY |
| PUSH IY | FD E5 | 2 | 15 | PUSH HL (E5) | HL->IY |
| POP IY | FD E1 | 2 | 14 | POP HL (E1) | HL->IY |
| EX (SP),IY | FD E3 | 2 | 23 | EX (SP),HL (E3) | HL->IY |
| JP (IY) | FD E9 | 2 | 8 | JP (HL) (E9) | HL->IY |
| LD SP,IY | FD F9 | 2 | 10 | LD SP,HL (F9) | HL->IY |

### 2.2 Documented Indexed Addressing: (IY+d)

| Mnemonic | Opcode | Sz | T | Base Instruction |
|----------|--------|----|---|------------------|
| LD B,(IY+d) | FD 46 d | 3 | 19 | LD B,(HL) (46) |
| LD C,(IY+d) | FD 4E d | 3 | 19 | LD C,(HL) (4E) |
| LD D,(IY+d) | FD 56 d | 3 | 19 | LD D,(HL) (56) |
| LD E,(IY+d) | FD 5E d | 3 | 19 | LD E,(HL) (5E) |
| LD H,(IY+d) | FD 66 d | 3 | 19 | LD H,(HL) (66) |
| LD L,(IY+d) | FD 6E d | 3 | 19 | LD L,(HL) (6E) |
| LD A,(IY+d) | FD 7E d | 3 | 19 | LD A,(HL) (7E) |
| LD (IY+d),B | FD 70 d | 3 | 19 | LD (HL),B (70) |
| LD (IY+d),C | FD 71 d | 3 | 19 | LD (HL),C (71) |
| LD (IY+d),D | FD 72 d | 3 | 19 | LD (HL),D (72) |
| LD (IY+d),E | FD 73 d | 3 | 19 | LD (HL),E (73) |
| LD (IY+d),H | FD 74 d | 3 | 19 | LD (HL),H (74) |
| LD (IY+d),L | FD 75 d | 3 | 19 | LD (HL),L (75) |
| LD (IY+d),A | FD 77 d | 3 | 19 | LD (HL),A (77) |
| LD (IY+d),n | FD 36 d n | 4 | 19 | LD (HL),n (36) |
| ADD A,(IY+d) | FD 86 d | 3 | 19 | ADD A,(HL) (86) |
| ADC A,(IY+d) | FD 8E d | 3 | 19 | ADC A,(HL) (8E) |
| SUB (IY+d) | FD 96 d | 3 | 19 | SUB (HL) (96) |
| SBC A,(IY+d) | FD 9E d | 3 | 19 | SBC A,(HL) (9E) |
| AND (IY+d) | FD A6 d | 3 | 19 | AND (HL) (A6) |
| XOR (IY+d) | FD AE d | 3 | 19 | XOR (HL) (AE) |
| OR (IY+d) | FD B6 d | 3 | 19 | OR (HL) (B6) |
| CP (IY+d) | FD BE d | 3 | 19 | CP (HL) (BE) |
| INC (IY+d) | FD 34 d | 3 | 23 | INC (HL) (34) |
| DEC (IY+d) | FD 35 d | 3 | 23 | DEC (HL) (35) |

### 2.3 Undocumented 8-bit IYH/IYL Operations

Exactly parallel to IXH/IXL, using FD prefix instead of DD.

#### IYH/IYL Load Instructions (U)

| Mnemonic | Opcode | Sz | T | Base Instruction | Status |
|----------|--------|----|---|------------------|--------|
| LD IYH,B | FD 60 | 2 | 8 | LD H,B (60) | (U) |
| LD IYH,C | FD 61 | 2 | 8 | LD H,C (61) | (U) |
| LD IYH,D | FD 62 | 2 | 8 | LD H,D (62) | (U) |
| LD IYH,E | FD 63 | 2 | 8 | LD H,E (63) | (U) |
| LD IYH,IYH | FD 64 | 2 | 8 | LD H,H (64) | (U) |
| LD IYH,IYL | FD 65 | 2 | 8 | LD H,L (65) | (U) |
| LD IYH,A | FD 67 | 2 | 8 | LD H,A (67) | (U) |
| LD IYL,B | FD 68 | 2 | 8 | LD L,B (68) | (U) |
| LD IYL,C | FD 69 | 2 | 8 | LD L,C (69) | (U) |
| LD IYL,D | FD 6A | 2 | 8 | LD L,D (6A) | (U) |
| LD IYL,E | FD 6B | 2 | 8 | LD L,E (6B) | (U) |
| LD IYL,IYH | FD 6C | 2 | 8 | LD L,H (6C) | (U) |
| LD IYL,IYL | FD 6D | 2 | 8 | LD L,L (6D) | (U) |
| LD IYL,A | FD 6F | 2 | 8 | LD L,A (6F) | (U) |
| LD B,IYH | FD 44 | 2 | 8 | LD B,H (44) | (U) |
| LD B,IYL | FD 45 | 2 | 8 | LD B,L (45) | (U) |
| LD C,IYH | FD 4C | 2 | 8 | LD C,H (4C) | (U) |
| LD C,IYL | FD 4D | 2 | 8 | LD C,L (4D) | (U) |
| LD D,IYH | FD 54 | 2 | 8 | LD D,H (54) | (U) |
| LD D,IYL | FD 55 | 2 | 8 | LD D,L (55) | (U) |
| LD E,IYH | FD 5C | 2 | 8 | LD E,H (5C) | (U) |
| LD E,IYL | FD 5D | 2 | 8 | LD E,L (5D) | (U) |
| LD A,IYH | FD 7C | 2 | 8 | LD A,H (7C) | (U) |
| LD A,IYL | FD 7D | 2 | 8 | LD A,L (7D) | (U) |
| LD IYH,n | FD 26 n | 3 | 11 | LD H,n (26) | (U) |
| LD IYL,n | FD 2E n | 3 | 11 | LD L,n (2E) | (U) |

#### IYH/IYL Increment/Decrement (U)

| Mnemonic | Opcode | Sz | T | Base Instruction | Status |
|----------|--------|----|---|------------------|--------|
| INC IYH | FD 24 | 2 | 8 | INC H (24) | (U) |
| DEC IYH | FD 25 | 2 | 8 | DEC H (25) | (U) |
| INC IYL | FD 2C | 2 | 8 | INC L (2C) | (U) |
| DEC IYL | FD 2D | 2 | 8 | DEC L (2D) | (U) |

#### IYH/IYL Arithmetic/Logic with A (U)

| Mnemonic | Opcode | Sz | T | Base Instruction | Status |
|----------|--------|----|---|------------------|--------|
| ADD A,IYH | FD 84 | 2 | 8 | ADD A,H (84) | (U) |
| ADD A,IYL | FD 85 | 2 | 8 | ADD A,L (85) | (U) |
| ADC A,IYH | FD 8C | 2 | 8 | ADC A,H (8C) | (U) |
| ADC A,IYL | FD 8D | 2 | 8 | ADC A,L (8D) | (U) |
| SUB IYH | FD 94 | 2 | 8 | SUB H (94) | (U) |
| SUB IYL | FD 95 | 2 | 8 | SUB L (95) | (U) |
| SBC A,IYH | FD 9C | 2 | 8 | SBC A,H (9C) | (U) |
| SBC A,IYL | FD 9D | 2 | 8 | SBC A,L (9D) | (U) |
| AND IYH | FD A4 | 2 | 8 | AND H (A4) | (U) |
| AND IYL | FD A5 | 2 | 8 | AND L (A5) | (U) |
| XOR IYH | FD AC | 2 | 8 | XOR H (AC) | (U) |
| XOR IYL | FD AD | 2 | 8 | XOR L (AD) | (U) |
| OR IYH | FD B4 | 2 | 8 | OR H (B4) | (U) |
| OR IYL | FD B5 | 2 | 8 | OR L (B5) | (U) |
| CP IYH | FD BC | 2 | 8 | CP H (BC) | (U) |
| CP IYL | FD BD | 2 | 8 | CP L (BD) | (U) |

---

## 3. CB-Prefixed Instructions (Bit Operations, Rotate/Shift)

### 3.1 Register Encoding in CB Opcodes

The low 3 bits of the second byte select the register:

| Bits 2-0 | Register |
|----------|----------|
| 000 | B |
| 001 | C |
| 010 | D |
| 011 | E |
| 100 | H |
| 101 | L |
| 110 | (HL) |
| 111 | A |

### 3.2 Documented Rotate/Shift Instructions

| Mnemonic | Opcode | Sz | T (reg) | T ((HL)) |
|----------|--------|----|---------|----------|
| RLC r | CB 00+r | 2 | 8 | 15 |
| RRC r | CB 08+r | 2 | 8 | 15 |
| RL r | CB 10+r | 2 | 8 | 15 |
| RR r | CB 18+r | 2 | 8 | 15 |
| SLA r | CB 20+r | 2 | 8 | 15 |
| SRA r | CB 28+r | 2 | 8 | 15 |
| SRL r | CB 38+r | 2 | 8 | 15 |

### 3.3 Undocumented: SLL (Shift Left Logical, aka SL1, SLI)

| Mnemonic | Opcode | Sz | T (reg) | T ((HL)) | Status |
|----------|--------|----|---------|----------|--------|
| SLL B | CB 30 | 2 | 8 | -- | (U) |
| SLL C | CB 31 | 2 | 8 | -- | (U) |
| SLL D | CB 32 | 2 | 8 | -- | (U) |
| SLL E | CB 33 | 2 | 8 | -- | (U) |
| SLL H | CB 34 | 2 | 8 | -- | (U) |
| SLL L | CB 35 | 2 | 8 | -- | (U) |
| SLL (HL) | CB 36 | 2 | -- | 15 | (U) |
| SLL A | CB 37 | 2 | 8 | -- | (U) |

**Behavior**: Like SLA, but sets bit 0 to 1 instead of 0. Effectively: result = (operand << 1) | 1, with bit 7 going to carry. This is a multiply-by-2-and-add-1 operation.

**Note**: The R800 does NOT support SLL; it executes as SLA instead. CMOS Z80s support SLL.

### 3.4 Documented Bit Test/Set/Reset Instructions

| Mnemonic | Opcode | Sz | T (reg) | T ((HL)) |
|----------|--------|----|---------|----------|
| BIT b,r | CB 40+8*b+r | 2 | 8 | 12 |
| RES b,r | CB 80+8*b+r | 2 | 8 | 15 |
| SET b,r | CB C0+8*b+r | 2 | 8 | 15 |

### 3.5 DD CB / FD CB Double-Prefixed Instructions

Format: `DD CB d xx` or `FD CB d xx` (4 bytes total).
Note: the displacement byte `d` comes BEFORE the opcode byte `xx`.

#### Documented (IX+d) Bit Operations

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| RLC (IX+d) | DD CB d 06 | 4 | 23 |
| RRC (IX+d) | DD CB d 0E | 4 | 23 |
| RL (IX+d) | DD CB d 16 | 4 | 23 |
| RR (IX+d) | DD CB d 1E | 4 | 23 |
| SLA (IX+d) | DD CB d 26 | 4 | 23 |
| SRA (IX+d) | DD CB d 2E | 4 | 23 |
| SRL (IX+d) | DD CB d 3E | 4 | 23 |
| BIT b,(IX+d) | DD CB d 46+8*b | 4 | 20 |
| RES b,(IX+d) | DD CB d 86+8*b | 4 | 23 |
| SET b,(IX+d) | DD CB d C6+8*b | 4 | 23 |

Same for IY with FD prefix.

#### Undocumented: DDCB/FDCB with Register Copy

For all non-BIT operations, the undocumented variants simultaneously write the result to a register. The register is determined by the low 3 bits of the final opcode byte (same encoding as section 3.1, but bit pattern 110 is the documented version).

**Rotate/shift with register copy (U):**

| Mnemonic | Last Byte | Sz | T | Behavior |
|----------|-----------|----|---|----------|
| RLC (IX+d),B | 00 | 4 | 23 | RLC (IX+d); LD B,result |
| RLC (IX+d),C | 01 | 4 | 23 | RLC (IX+d); LD C,result |
| RLC (IX+d),D | 02 | 4 | 23 | RLC (IX+d); LD D,result |
| RLC (IX+d),E | 03 | 4 | 23 | RLC (IX+d); LD E,result |
| RLC (IX+d),H | 04 | 4 | 23 | RLC (IX+d); LD H,result |
| RLC (IX+d),L | 05 | 4 | 23 | RLC (IX+d); LD L,result |
| RLC (IX+d) | 06 | 4 | 23 | (documented) |
| RLC (IX+d),A | 07 | 4 | 23 | RLC (IX+d); LD A,result |

The same pattern applies to all rotate/shift operations:

| Operation | Base byte (documented) | Undocumented range |
|-----------|----------------------|-------------------|
| RLC | 06 | 00-05, 07 |
| RRC | 0E | 08-0D, 0F |
| RL | 16 | 10-15, 17 |
| RR | 1E | 18-1D, 1F |
| SLA | 26 | 20-25, 27 |
| SRA | 2E | 28-2D, 2F |
| SLL (U) | 36 | 30-35, 37 |
| SRL | 3E | 38-3D, 3F |

**RES/SET with register copy (U):**

Same pattern. For `RES b,(IX+d),r` the last byte is `80+8*b+r`, and for `SET b,(IX+d),r` it is `C0+8*b+r`, where r!=6 gives the undocumented form.

| Mnemonic | Last Byte | Sz | T | Behavior |
|----------|-----------|----|---|----------|
| RES 0,(IX+d),B | 80 | 4 | 23 | RES 0,(IX+d); LD B,result |
| RES 0,(IX+d),C | 81 | 4 | 23 | RES 0,(IX+d); LD C,result |
| ... | ... | ... | ... | ... |
| RES 0,(IX+d),A | 87 | 4 | 23 | RES 0,(IX+d); LD A,result |
| SET 0,(IX+d),B | C0 | 4 | 23 | SET 0,(IX+d); LD B,result |
| SET 0,(IX+d),C | C1 | 4 | 23 | SET 0,(IX+d); LD C,result |
| ... | ... | ... | ... | ... |
| SET 7,(IX+d),A | FF | 4 | 23 | SET 7,(IX+d); LD A,result |

**BIT instructions exception**: BIT operations do NOT copy to a register. All 8 variants of `BIT b,(IX+d)` (last bytes 40-47+8*b) behave identically -- they just test the bit and set flags. The undocumented variants differ only in which bits appear in flags bits 3 and 5 (from the register encoding bits, not a meaningful register copy).

**FD CB equivalents**: All of the above apply identically with FD prefix for IY.

---

## 4. ED-Prefixed Instructions

### 4.1 Documented ED Instructions

#### 16-bit Arithmetic

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| SBC HL,BC | ED 42 | 2 | 15 |
| ADC HL,BC | ED 4A | 2 | 15 |
| SBC HL,DE | ED 52 | 2 | 15 |
| ADC HL,DE | ED 5A | 2 | 15 |
| SBC HL,HL | ED 62 | 2 | 15 |
| ADC HL,HL | ED 6A | 2 | 15 |
| SBC HL,SP | ED 72 | 2 | 15 |
| ADC HL,SP | ED 7A | 2 | 15 |

#### 16-bit Load (Direct)

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| LD (nn),BC | ED 43 nn nn | 4 | 20 |
| LD BC,(nn) | ED 4B nn nn | 4 | 20 |
| LD (nn),DE | ED 53 nn nn | 4 | 20 |
| LD DE,(nn) | ED 5B nn nn | 4 | 20 |
| LD (nn),HL | ED 63 nn nn | 4 | 20 |
| LD HL,(nn) | ED 6B nn nn | 4 | 20 |
| LD (nn),SP | ED 73 nn nn | 4 | 20 |
| LD SP,(nn) | ED 7B nn nn | 4 | 20 |

**Note**: ED 63 and ED 6B are undocumented duplicates of the non-ED instructions `LD (nn),HL` (22) and `LD HL,(nn)` (2A). They work but are redundant. Some sources list them as undocumented; Zilog includes them in the ED block for orthogonality.

#### Special Register Loads

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| LD I,A | ED 47 | 2 | 9 |
| LD R,A | ED 4F | 2 | 9 |
| LD A,I | ED 57 | 2 | 9 |
| LD A,R | ED 5F | 2 | 9 |

**LD A,I and LD A,R flag behavior**: Sets P/V flag from IFF2 (interrupt flip-flop). On NMOS Z80, if an interrupt occurs during LD A,I or LD A,R, the P/V flag is erroneously reset to 0. This bug is fixed on CMOS Z80.

#### Negate

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| NEG | ED 44 | 2 | 8 |

#### Interrupt Mode

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| IM 0 | ED 46 | 2 | 8 |
| IM 1 | ED 56 | 2 | 8 |
| IM 2 | ED 5E | 2 | 8 |

#### Return from Interrupt

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| RETN | ED 45 | 2 | 14 |
| RETI | ED 4D | 2 | 14 |

#### Rotate Digit (BCD)

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| RRD | ED 67 | 2 | 18 |
| RLD | ED 6F | 2 | 18 |

#### I/O Operations

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| IN B,(C) | ED 40 | 2 | 12 |
| OUT (C),B | ED 41 | 2 | 12 |
| IN C,(C) | ED 48 | 2 | 12 |
| OUT (C),C | ED 49 | 2 | 12 |
| IN D,(C) | ED 50 | 2 | 12 |
| OUT (C),D | ED 51 | 2 | 12 |
| IN E,(C) | ED 58 | 2 | 12 |
| OUT (C),E | ED 59 | 2 | 12 |
| IN H,(C) | ED 60 | 2 | 12 |
| OUT (C),H | ED 61 | 2 | 12 |
| IN L,(C) | ED 68 | 2 | 12 |
| OUT (C),L | ED 69 | 2 | 12 |
| IN A,(C) | ED 78 | 2 | 12 |
| OUT (C),A | ED 79 | 2 | 12 |

#### Block Transfer

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| LDI | ED A0 | 2 | 16 |
| LDIR | ED B0 | 2 | 21/16 |
| LDD | ED A8 | 2 | 16 |
| LDDR | ED B8 | 2 | 21/16 |

#### Block Search

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| CPI | ED A1 | 2 | 16 |
| CPIR | ED B1 | 2 | 21/16 |
| CPD | ED A9 | 2 | 16 |
| CPDR | ED B9 | 2 | 21/16 |

#### Block I/O

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| INI | ED A2 | 2 | 16 |
| INIR | ED B2 | 2 | 21/16 |
| IND | ED AA | 2 | 16 |
| INDR | ED BA | 2 | 21/16 |
| OUTI | ED A3 | 2 | 16 |
| OTIR | ED B3 | 2 | 21/16 |
| OUTD | ED AB | 2 | 16 |
| OTDR | ED BB | 2 | 21/16 |

### 4.2 Undocumented ED Instructions

#### NEG Duplicates (U)

All 8 opcodes ED x4 (where x = 4,5,6,7) decode as NEG:

| Opcode | Sz | T | Status |
|--------|----|---|--------|
| ED 44 | 2 | 8 | (D) -- documented NEG |
| ED 4C | 2 | 8 | (U) -- NEG duplicate |
| ED 54 | 2 | 8 | (U) -- NEG duplicate |
| ED 5C | 2 | 8 | (U) -- NEG duplicate |
| ED 64 | 2 | 8 | (U) -- NEG duplicate |
| ED 6C | 2 | 8 | (U) -- NEG duplicate |
| ED 74 | 2 | 8 | (U) -- NEG duplicate |
| ED 7C | 2 | 8 | (U) -- NEG duplicate |

#### RETN Duplicates (U)

| Opcode | Sz | T | Status |
|--------|----|---|--------|
| ED 45 | 2 | 14 | (D) -- documented RETN |
| ED 4D | 2 | 14 | (D) -- documented RETI |
| ED 55 | 2 | 14 | (U) -- RETN duplicate |
| ED 5D | 2 | 14 | (U) -- RETN duplicate |
| ED 65 | 2 | 14 | (U) -- RETN duplicate |
| ED 6D | 2 | 14 | (U) -- RETN duplicate |
| ED 75 | 2 | 14 | (U) -- RETN duplicate |
| ED 7D | 2 | 14 | (U) -- RETN duplicate |

**Note**: RETI and RETN differ only in their effect on IFF1 (RETN copies IFF2 to IFF1). The hardware distinction matters for peripherals like the Z80 PIO/CTC that monitor the RETI opcode on the bus (ED 4D specifically).

#### IM Duplicates (U)

| Opcode | Sz | T | Effect | Status |
|--------|----|---|--------|--------|
| ED 46 | 2 | 8 | IM 0 | (D) |
| ED 4E | 2 | 8 | IM 0 | (U) |
| ED 56 | 2 | 8 | IM 1 | (D) |
| ED 5E | 2 | 8 | IM 2 | (D) |
| ED 66 | 2 | 8 | IM 0 | (U) |
| ED 6E | 2 | 8 | IM 0 | (U) |
| ED 76 | 2 | 8 | IM 1 | (U) |
| ED 7E | 2 | 8 | IM 2 | (U) |

#### IN F,(C) / IN (C) (U)

| Mnemonic | Opcode | Sz | T | Status |
|----------|--------|----|---|--------|
| IN F,(C) | ED 70 | 2 | 12 | (U) |

Reads from port C, discards the result, but sets flags (S, Z, H, P/V, N) based on the value read. Also written as `IN (C)` since no destination register is written.

#### OUT (C),0 / OUT (C),255 (U)

| Mnemonic | Opcode | Sz | T | Status |
|----------|--------|----|---|--------|
| OUT (C),0 | ED 71 | 2 | 12 | (U) NMOS |
| OUT (C),255 | ED 71 | 2 | 12 | (U) CMOS |

Outputs to port C. On NMOS Z80, outputs 0x00. On CMOS Z80, outputs 0xFF. This difference is due to internal bus floating behavior.

#### ED 77 and ED 7F: NOP-like (U)

| Opcode | Sz | T | Status | Notes |
|--------|----|---|--------|-------|
| ED 77 | 2 | 8 | (U) | NOP; may affect flags like LD I,I |
| ED 7F | 2 | 8 | (U) | NOP; may affect flags like LD R,R |

These positions in the opcode map correspond to where `LD I,I` and `LD R,R` would be by the decoding pattern. They do nothing useful but are not pure NOPs -- they may set P/V from IFF2.

### 4.3 ED NOP Ranges

Any ED-prefixed opcode not in the 0x40-0x7F or 0xA0-0xBF ranges acts as a 2-byte NOP:

| Range | Behavior | T-states |
|-------|----------|----------|
| ED 00 - ED 3F | NOP (2 bytes) | 8 |
| ED 80 - ED 9F | NOP (2 bytes) | 8 |
| ED C0 - ED FF | NOP (2 bytes) | 8 |

Within ED 40-7F, unused opcodes also act as NOPs (8 T-states). The R register increments by 2 for any ED-prefixed instruction (1 for the ED prefix fetch, 1 for the second byte fetch).

### 4.4 R800-Only Instructions (not Z80)

These exist on the ASCII R800 CPU (MSX turboR) but NOT on any Z80:

| Mnemonic | Opcode | Sz | T (R800) | Notes |
|----------|--------|----|----------|-------|
| MULUB A,B | ED C1 | 2 | 14 | A*B -> HL (unsigned) |
| MULUB A,C | ED C9 | 2 | 14 | A*C -> HL (unsigned) |
| MULUB A,D | ED D1 | 2 | 14 | A*D -> HL (unsigned) |
| MULUB A,E | ED D9 | 2 | 14 | A*E -> HL (unsigned) |
| MULUW HL,BC | ED C3 | 2 | 36 | HL*BC -> DE:HL (unsigned) |
| MULUW HL,SP | ED F3 | 2 | 36 | HL*SP -> DE:HL (unsigned) |

---

## 5. Unprefixed Instructions (Base Opcode Map)

For completeness, here are the base (unprefixed) instructions that interact with HL, H, L -- these are the ones that get IX/IY variants via DD/FD prefixes.

### 5.1 8-bit Load

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| LD r,r' | 40+8*r+r' | 1 | 4 |
| LD r,(HL) | 46+8*r | 1 | 7 |
| LD (HL),r | 70+r | 1 | 7 |
| LD r,n | 06+8*r n | 2 | 7 |
| LD (HL),n | 36 n | 2 | 10 |
| LD A,(BC) | 0A | 1 | 7 |
| LD A,(DE) | 1A | 1 | 7 |
| LD A,(nn) | 3A nn nn | 3 | 13 |
| LD (BC),A | 02 | 1 | 7 |
| LD (DE),A | 12 | 1 | 7 |
| LD (nn),A | 32 nn nn | 3 | 13 |

### 5.2 16-bit Load

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| LD BC,nn | 01 nn nn | 3 | 10 |
| LD DE,nn | 11 nn nn | 3 | 10 |
| LD HL,nn | 21 nn nn | 3 | 10 |
| LD SP,nn | 31 nn nn | 3 | 10 |
| LD HL,(nn) | 2A nn nn | 3 | 16 |
| LD (nn),HL | 22 nn nn | 3 | 16 |
| LD SP,HL | F9 | 1 | 6 |
| PUSH BC | C5 | 1 | 11 |
| PUSH DE | D5 | 1 | 11 |
| PUSH HL | E5 | 1 | 11 |
| PUSH AF | F5 | 1 | 11 |
| POP BC | C1 | 1 | 10 |
| POP DE | D1 | 1 | 10 |
| POP HL | E1 | 1 | 10 |
| POP AF | F1 | 1 | 10 |

### 5.3 8-bit Arithmetic/Logic

| Mnemonic | Opcode | Sz | T (reg) | T ((HL)) |
|----------|--------|----|---------|----------|
| ADD A,r | 80+r | 1 | 4 | 7 |
| ADC A,r | 88+r | 1 | 4 | 7 |
| SUB r | 90+r | 1 | 4 | 7 |
| SBC A,r | 98+r | 1 | 4 | 7 |
| AND r | A0+r | 1 | 4 | 7 |
| XOR r | A8+r | 1 | 4 | 7 |
| OR r | B0+r | 1 | 4 | 7 |
| CP r | B8+r | 1 | 4 | 7 |
| ADD A,n | C6 n | 2 | 7 | -- |
| ADC A,n | CE n | 2 | 7 | -- |
| SUB n | D6 n | 2 | 7 | -- |
| SBC A,n | DE n | 2 | 7 | -- |
| AND n | E6 n | 2 | 7 | -- |
| XOR n | EE n | 2 | 7 | -- |
| OR n | F6 n | 2 | 7 | -- |
| CP n | FE n | 2 | 7 | -- |
| INC r | 04+8*r | 1 | 4 | 11 |
| DEC r | 05+8*r | 1 | 4 | 11 |
| DAA | 27 | 1 | 4 | -- |
| CPL | 2F | 1 | 4 | -- |
| NEG | ED 44 | 2 | 8 | -- |

### 5.4 16-bit Arithmetic

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| ADD HL,BC | 09 | 1 | 11 |
| ADD HL,DE | 19 | 1 | 11 |
| ADD HL,HL | 29 | 1 | 11 |
| ADD HL,SP | 39 | 1 | 11 |
| INC BC | 03 | 1 | 6 |
| INC DE | 13 | 1 | 6 |
| INC HL | 23 | 1 | 6 |
| INC SP | 33 | 1 | 6 |
| DEC BC | 0B | 1 | 6 |
| DEC DE | 1B | 1 | 6 |
| DEC HL | 2B | 1 | 6 |
| DEC SP | 3B | 1 | 6 |

### 5.5 Rotate (Accumulator)

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| RLCA | 07 | 1 | 4 |
| RRCA | 0F | 1 | 4 |
| RLA | 17 | 1 | 4 |
| RRA | 1F | 1 | 4 |

### 5.6 Jumps, Calls, Returns

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| JP nn | C3 nn nn | 3 | 10 |
| JP cc,nn | C2+8*cc nn nn | 3 | 10 |
| JR e | 18 e | 2 | 12 |
| JR NZ,e | 20 e | 2 | 12/7 |
| JR Z,e | 28 e | 2 | 12/7 |
| JR NC,e | 30 e | 2 | 12/7 |
| JR C,e | 38 e | 2 | 12/7 |
| JP (HL) | E9 | 1 | 4 |
| DJNZ e | 10 e | 2 | 13/8 |
| CALL nn | CD nn nn | 3 | 17 |
| CALL cc,nn | C4+8*cc nn nn | 3 | 17/10 |
| RET | C9 | 1 | 10 |
| RET cc | C0+8*cc | 1 | 11/5 |
| RST p | C7+p | 1 | 11 |

### 5.7 Exchange, Misc

| Mnemonic | Opcode | Sz | T |
|----------|--------|----|---|
| EX DE,HL | EB | 1 | 4 |
| EX AF,AF' | 08 | 1 | 4 |
| EXX | D9 | 1 | 4 |
| EX (SP),HL | E3 | 1 | 19 |
| NOP | 00 | 1 | 4 |
| HALT | 76 | 1 | 4 |
| DI | F3 | 1 | 4 |
| EI | FB | 1 | 4 |
| SCF | 37 | 1 | 4 |
| CCF | 3F | 1 | 4 |
| IN A,(n) | DB n | 2 | 11 |
| OUT (n),A | D3 n | 2 | 11 |

---

## 6. NMOS vs CMOS Z80 vs Clones

### 6.1 NMOS Z80 (original Zilog, 1976)

- All undocumented instructions work as described above
- `OUT (C),0` at ED 71 outputs 0x00
- LD A,I / LD A,R: IFF2 can be erroneously reset if interrupt arrives during execution
- SLL (CB 30-37) works as documented above
- Flags bits 3 and 5 after CCF/SCF: `(Q ^ F) | A` (where Q is internal state)

### 6.2 CMOS Z80 (Zilog Z84C00xx)

- All undocumented instructions work identically to NMOS **except**:
  - `OUT (C),0` at ED 71 outputs 0xFF instead of 0x00 (floating bus reads high)
  - LD A,I / LD A,R bug is **fixed** -- IFF2 is correctly preserved
- SLL still works
- Generally better power characteristics, same instruction behavior

### 6.3 R800 (ASCII Corporation, MSX turboR)

- **SLL does NOT work** -- CB 30-37 execute as SLA instead
- **DDCB/FDCB undocumented register copy does NOT work** -- only the (IX+d)/(IY+d) operation executes; the register copy is silently dropped
- **IXH/IXL/IYH/IYL undocumented instructions DO work**
- Adds MULUB and MULUW instructions (see section 4.4)
- Different timing for all instructions (pipelined architecture)
- OUT (C),0 behavior: outputs 0x00

### 6.4 eZ80 (Zilog)

- 24-bit address bus, additional addressing modes
- Undocumented Z80 instructions generally NOT supported
- IXH/IXL/IYH/IYL may not work reliably
- Not relevant for standard Z80 compiler targeting

### 6.5 Z180/Z8S180

- IXH/IXL/IYH/IYL DO work (confirmed by various sources)
- SLL does NOT work
- DDCB/FDCB register copy does NOT work
- Adds some new instructions (TSTIO, TST, MLT, etc.)

### 6.6 Summary of Clone Compatibility

| Feature | NMOS Z80 | CMOS Z80 | Z180 | R800 | eZ80 |
|---------|----------|----------|------|------|------|
| IXH/IXL/IYH/IYL | Yes | Yes | Yes | Yes | No |
| SLL (CB 30-37) | Yes | Yes | No | No | No |
| DDCB reg copy | Yes | Yes | No | No | No |
| OUT (C),0 = 0x00 | Yes | No (0xFF) | ? | Yes | ? |
| LD A,I/R IFF2 bug | Yes | No (fixed) | No | No | No |
| MULUB/MULUW | No | No | No | Yes | No |

---

## 7. Compiler Backend Implementation Summary

### 7.1 Instructions Required for IX/IY as Allocatable 16-bit Registers

These instructions are needed to treat IX and IY as general-purpose 16-bit registers (like HL/BC/DE):

| Category | Mnemonic | Opcode (IX) | Opcode (IY) | Sz | T | Status |
|----------|----------|-------------|-------------|----|---|--------|
| Load imm | LD IX,nn | DD 21 nn nn | FD 21 nn nn | 4 | 14 | (D) |
| Load mem | LD IX,(nn) | DD 2A nn nn | FD 2A nn nn | 4 | 20 | (D) |
| Store mem | LD (nn),IX | DD 22 nn nn | FD 22 nn nn | 4 | 20 | (D) |
| Push | PUSH IX | DD E5 | FD E5 | 2 | 15 | (D) |
| Pop | POP IX | DD E1 | FD E1 | 2 | 14 | (D) |
| Inc 16 | INC IX | DD 23 | FD 23 | 2 | 10 | (D) |
| Dec 16 | DEC IX | DD 2B | FD 2B | 2 | 10 | (D) |
| Add 16 | ADD IX,BC | DD 09 | FD 09 | 2 | 15 | (D) |
| Add 16 | ADD IX,DE | DD 19 | FD 19 | 2 | 15 | (D) |
| Add 16 | ADD IX,IX | DD 29 | FD 29 | 2 | 15 | (D) |
| Add 16 | ADD IX,SP | DD 39 | FD 39 | 2 | 15 | (D) |
| SP load | LD SP,IX | DD F9 | FD F9 | 2 | 10 | (D) |
| Exchange | EX (SP),IX | DD E3 | FD E3 | 2 | 23 | (D) |
| Jump | JP (IX) | DD E9 | FD E9 | 2 | 8 | (D) |

### 7.2 Instructions Required for IXH/IXL as Allocatable 8-bit Sub-registers

All undocumented. Required if the backend wants to split IX/IY into 8-bit halves:

| Category | Mnemonic | Opcode (IX) | Opcode (IY) | Sz | T | Status |
|----------|----------|-------------|-------------|----|---|--------|
| Load imm | LD IXH,n | DD 26 n | FD 26 n | 3 | 11 | (U) |
| Load imm | LD IXL,n | DD 2E n | FD 2E n | 3 | 11 | (U) |
| Load reg | LD IXH,r | DD 60+r | FD 60+r | 2 | 8 | (U) |
| Load reg | LD IXL,r | DD 68+r | FD 68+r | 2 | 8 | (U) |
| Load reg | LD r,IXH | DD 40+8*r+4 | FD 40+8*r+4 | 2 | 8 | (U) |
| Load reg | LD r,IXL | DD 40+8*r+5 | FD 40+8*r+5 | 2 | 8 | (U) |
| Inc | INC IXH | DD 24 | FD 24 | 2 | 8 | (U) |
| Dec | DEC IXH | DD 25 | FD 25 | 2 | 8 | (U) |
| Inc | INC IXL | DD 2C | FD 2C | 2 | 8 | (U) |
| Dec | DEC IXL | DD 2D | FD 2D | 2 | 8 | (U) |
| ADD | ADD A,IXH | DD 84 | FD 84 | 2 | 8 | (U) |
| ADD | ADD A,IXL | DD 85 | FD 85 | 2 | 8 | (U) |
| ADC | ADC A,IXH | DD 8C | FD 8C | 2 | 8 | (U) |
| ADC | ADC A,IXL | DD 8D | FD 8D | 2 | 8 | (U) |
| SUB | SUB IXH | DD 94 | FD 94 | 2 | 8 | (U) |
| SUB | SUB IXL | DD 95 | FD 95 | 2 | 8 | (U) |
| SBC | SBC A,IXH | DD 9C | FD 9C | 2 | 8 | (U) |
| SBC | SBC A,IXL | DD 9D | FD 9D | 2 | 8 | (U) |
| AND | AND IXH | DD A4 | FD A4 | 2 | 8 | (U) |
| AND | AND IXL | DD A5 | FD A5 | 2 | 8 | (U) |
| XOR | XOR IXH | DD AC | FD AC | 2 | 8 | (U) |
| XOR | XOR IXL | DD AD | FD AD | 2 | 8 | (U) |
| OR | OR IXH | DD B4 | FD B4 | 2 | 8 | (U) |
| OR | OR IXL | DD B5 | FD B5 | 2 | 8 | (U) |
| CP | CP IXH | DD BC | FD BC | 2 | 8 | (U) |
| CP | CP IXL | DD BD | FD BD | 2 | 8 | (U) |

**Important limitation**: `r` in the load instructions above refers to B, C, D, E, IXH, IXL, A -- NOT H or L. You cannot load between H/L and IXH/IXL directly (those opcodes are the indexed addressing forms instead).

### 7.3 Indexed Addressing Instructions (IX+d)/(IY+d)

These are needed for frame pointer access when IX/IY is used as a stack frame pointer:

| Category | Mnemonic | Opcode (IX) | Opcode (IY) | Sz | T |
|----------|----------|-------------|-------------|----|---|
| Load | LD r,(IX+d) | DD 46+8*r d | FD 46+8*r d | 3 | 19 |
| Store | LD (IX+d),r | DD 70+r d | FD 70+r d | 3 | 19 |
| Store imm | LD (IX+d),n | DD 36 d n | FD 36 d n | 4 | 19 |
| Inc | INC (IX+d) | DD 34 d | FD 34 d | 3 | 23 |
| Dec | DEC (IX+d) | DD 35 d | FD 35 d | 3 | 23 |
| ADD | ADD A,(IX+d) | DD 86 d | FD 86 d | 3 | 19 |
| ADC | ADC A,(IX+d) | DD 8E d | FD 8E d | 3 | 19 |
| SUB | SUB (IX+d) | DD 96 d | FD 96 d | 3 | 19 |
| SBC | SBC A,(IX+d) | DD 9E d | FD 9E d | 3 | 19 |
| AND | AND (IX+d) | DD A6 d | FD A6 d | 3 | 19 |
| XOR | XOR (IX+d) | DD AE d | FD AE d | 3 | 19 |
| OR | OR (IX+d) | DD B6 d | FD B6 d | 3 | 19 |
| CP | CP (IX+d) | DD BE d | FD BE d | 3 | 19 |

### 7.4 Key Constraints for Register Allocation

1. **IXH/IXL and indexed addressing are mutually exclusive in a single instruction**: You cannot encode `LD IXH,(IX+d)` or `LD (IX+d),IXH`. The `DD 66 d` opcode gives `LD H,(IX+d)` (loads into real H, not IXH). This means when IX is used as a frame pointer, IXH/IXL are not usable as general-purpose 8-bit registers in indexed load/store instructions.

2. **No SBC IX,rr or ADC IX,rr**: Only ADD IX,rr exists for 16-bit arithmetic. SBC and ADC are only available for HL (via ED prefix). To do 16-bit subtract with IX, you must move IX to HL first.

3. **No INC/DEC IX affecting flags**: INC IX and DEC IX (16-bit) do NOT affect flags, same as INC/DEC on other 16-bit register pairs. INC IXH/DEC IXH (8-bit, undocumented) DO affect flags.

4. **No CB-prefix operations on IXH/IXL**: The DD CB prefix gives indexed addressing `(IX+d)`, not register operations on IXH/IXL. There is no way to do RLC IXH, BIT b,IXH, SET b,IXH, etc. To perform bit operations on IXH/IXL, you must first move to a regular register.

5. **DD/FD prefix cost**: Every IX/IY instruction costs at least 1 extra byte and 4 extra T-states compared to the HL equivalent. This makes IX/IY slower for tight loops.

6. **No EX IX,IY or EX IX,DE**: Only EX (SP),IX exists. To exchange IX with another register pair, you need push/pop sequences.

7. **Undocumented instruction compatibility**: IXH/IXL/IYH/IYL work on all Z80, Z180, and R800 processors. They do NOT work on eZ80. For a compiler targeting standard Z80, they are safe to use.

### 7.5 Cost Comparison: HL vs IX/IY Operations

| Operation | HL | IX/IY | Extra bytes | Extra T-states |
|-----------|-----|-------|-------------|----------------|
| LD rr,nn | 3B/10T | 4B/14T | +1 | +4 |
| ADD rr,rr | 1B/11T | 2B/15T | +1 | +4 |
| INC rr | 1B/6T | 2B/10T | +1 | +4 |
| DEC rr | 1B/6T | 2B/10T | +1 | +4 |
| PUSH rr | 1B/11T | 2B/15T | +1 | +4 |
| POP rr | 1B/10T | 2B/14T | +1 | +4 |
| LD r,(rr) | 1B/7T | 3B/19T | +2 | +12 |
| LD (rr),r | 1B/7T | 3B/19T | +2 | +12 |
| LD (rr),n | 2B/10T | 4B/19T | +2 | +9 |
| INC (rr) | 1B/11T | 3B/23T | +2 | +12 |
| ALU A,(rr) | 1B/7T | 3B/19T | +2 | +12 |
| LD r,H/L | 1B/4T | 2B/8T | +1 | +4 |
| ALU A,H/L | 1B/4T | 2B/8T | +1 | +4 |
| INC H/L | 1B/4T | 2B/8T | +1 | +4 |
| LD H/L,n | 2B/7T | 3B/11T | +1 | +4 |
| EX (SP),rr | 1B/19T | 2B/23T | +1 | +4 |
| JP (rr) | 1B/4T | 2B/8T | +1 | +4 |
| LD SP,rr | 1B/6T | 2B/10T | +1 | +4 |

### 7.6 Missing Operations (No IX/IY Equivalent Exists)

These HL/H/L operations have NO IX/IY equivalent and require register shuffling:

| Operation | Available for HL | Available for IX/IY |
|-----------|------------------|---------------------|
| ADC HL,rr | Yes (ED prefix) | NO |
| SBC HL,rr | Yes (ED prefix) | NO |
| RLC/RRC/RL/RR H or L | Yes (CB prefix) | NO -- CB ops on IXH/IXL don't exist |
| SLA/SRA/SRL H or L | Yes (CB prefix) | NO |
| BIT b,H or BIT b,L | Yes (CB prefix) | NO |
| SET b,H or SET b,L | Yes (CB prefix) | NO |
| RES b,H or RES b,L | Yes (CB prefix) | NO |
| RLD / RRD | Uses (HL) | NO (DD prefix ignored for RLD/RRD) |
| EX DE,HL | Yes | NO |
| EXX | Uses HL | NO |
| Block ops (LDI etc.) | Uses HL | NO |
| DAA | Operates on A | N/A |

---

## 8. Complete DD Prefix Opcode Map (0x00-0xFF)

For reference, this shows what every DD xx combination does. Entries marked "--" execute the base opcode (ignoring DD prefix, wasting 4 T-states).

```
     x0     x1     x2     x3     x4     x5     x6     x7
0x:  --     --     --     --     --     --     --     --
     x8     x9     xA     xB     xC     xD     xE     xF
0x:  --     ADD IX,BC --   --     --     --     --     --

1x:  --     --     --     --     --     --     --     --
     --     ADD IX,DE --   --     --     --     --     --

2x:  --     LD IX,nn  LD(nn),IX INC IX INC IXH DEC IXH LD IXH,n --
     --     ADD IX,IX LD IX,(nn) DEC IX INC IXL DEC IXL LD IXL,n --

3x:  --     --     --     --     INC(IX+d) DEC(IX+d) LD(IX+d),n --
     --     ADD IX,SP --   --     --     --     --     --

4x:  --     --     --     --     LD B,IXH LD B,IXL LD B,(IX+d) --
     --     --     --     --     LD C,IXH LD C,IXL LD C,(IX+d) --

5x:  --     --     --     --     LD D,IXH LD D,IXL LD D,(IX+d) --
     --     --     --     --     LD E,IXH LD E,IXL LD E,(IX+d) --

6x:  LD IXH,B LD IXH,C LD IXH,D LD IXH,E LD IXH,IXH LD IXH,IXL LD H,(IX+d) LD IXH,A
     LD IXL,B LD IXL,C LD IXL,D LD IXL,E LD IXL,IXH LD IXL,IXL LD L,(IX+d) LD IXL,A

7x:  LD(IX+d),B LD(IX+d),C LD(IX+d),D LD(IX+d),E LD(IX+d),H LD(IX+d),L -- LD(IX+d),A
     --     --     --     --     LD A,IXH LD A,IXL LD A,(IX+d) --

8x:  --     --     --     --     ADD A,IXH ADD A,IXL ADD A,(IX+d) --
     --     --     --     --     ADC A,IXH ADC A,IXL ADC A,(IX+d) --

9x:  --     --     --     --     SUB IXH  SUB IXL  SUB(IX+d) --
     --     --     --     --     SBC A,IXH SBC A,IXL SBC A,(IX+d) --

Ax:  --     --     --     --     AND IXH  AND IXL  AND(IX+d) --
     --     --     --     --     XOR IXH  XOR IXL  XOR(IX+d) --

Bx:  --     --     --     --     OR IXH   OR IXL   OR(IX+d)  --
     --     --     --     --     CP IXH   CP IXL   CP(IX+d)  --

Cx:  --     --     --     --     --     --     --     --
     --     --     --     DDCB   --     --     --     --

Dx:  --     --     --     --     --     --     --     --
     --     --     --     --     --     --     --     --

Ex:  --     POP IX --     EX(SP),IX --  PUSH IX --     --
     --     JP(IX) --     --     --     --     --     --

Fx:  --     --     --     --     --     --     --     --
     --     LD SP,IX --   --     --     --     --     --
```

The FD prefix map is identical with IY replacing IX and IYH/IYL replacing IXH/IXL.
