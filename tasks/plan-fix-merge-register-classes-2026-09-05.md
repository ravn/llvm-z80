# Plan: Fix Register Class Mismatch After Merge

**Oprettet:** 2026-09-05  
**Status:** Draft  
**Relateret til:** merge-upstream-2026-09-05 (commit de5aaeb411d8)

---

## Problem Statement

Merge-commit de5aaeb411d8 har 113 fejlende lit-tests (46.5%) pga **machine verifier** (`-verify-machineinstrs`).

**Rodarsag:** Register class mismatch mellem fork's `GR16` (inkl. IX, IY) og upstream's pseudo-instruktioner som forventer `GR16NoIR` (uden IX, IY).

**Eksempel-fejl:**
```
*** Bad machine code: Illegal virtual register for instruction *%
- instruction: XOR_CMP_EQ16 %0:gr16, %1:gr16
- operand 0: Expected a GR16NoIR register, but got a GR16 register
```

---

## Analyse

### Register Class Definitioner (Z80RegisterInfo.td)
```td
# Fork/Upstream:
def GR16      : Z80Reg16Class<(add DE, HL, BC, IX, IY)>;  # Inkluderer IX/IY
def GR16NoIR  : Z80Reg16Class<(add DE, HL, BC)>;           # Uden IX/IY
```

### Problem-Instruktioner (Z80InstrInfo.td)
```td
# Upstream pseudos der kræver GR16NoIR:
def XOR_CMP_EQ16 : Z80Pseudo<(outs), (ins GR16NoIR:$lhs, GR16NoIR:$rhs)>
def XOR_CMP_NE16 : Z80Pseudo<(outs), (ins GR16NoIR:$lhs, GR16NoIR:$rhs)>
def XOR_CMP_Z16  : Z80Pseudo<(outs), (ins GR16NoIR:$lhs, GR16NoIR:$rhs)>
```

### Fejl-Pattern
1. Instruction selector kopierer fysisk register (HL, DE, BC) til virtuel register
2. Register bank selection tildeler **GR16** (pga fork's definition inkluderer IX/IY)
3. Pseudo-instruktion forventer **GR16NoIR** → verifier fejl

---

## Lösningsstrategi

### Prioritet 1: Register Bank Info Fix (Hojt - Blocking)

**Fil:** `llvm/lib/Target/Z80/Z80RegisterBankInfo.cpp`

**Problem:** `getRegBankFromRegClass()` mapper GR16NoIR forkert.

**Lösning:** Override `getRegBankFromRegClass()` til at return korrekt bank for GR16NoIR.

```cpp
const RegisterBank &Z80RegisterBankInfo::getRegBankFromRegClass(
    const TargetRegisterClass &RC, LLT Ty) const {
  if (&RC == &Z80::GR16NoIRRegClass)
    return getRegBank(Z80::GR16NoIRRegBankID);
  if (&RC == &Z80::GR16RegClass)
    return getRegBank(Z80::GR16RegBankID);
  // ... rest of existing logic
}
```

**Afhængighed:** Kræver at Z80GenRegisterBank.inc har korrekte definitions.

---

### Prioritet 2: Instruction Selection Fix (Middel)

**Fil:** `llvm/lib/Target/Z80/Z80InstructionSelector.cpp`

**Problem:** Instruction selector bruger GR16 i stedet for GR16NoIR.

**Lösning:** Opdater constraints i GlobalISel tabel-definitioner.

```cpp
// I Z80InstrInfo.td - skift GR16 til GR16NoIR hvor relevant
// F.eks. for XOR_CMP_EQ16:
def XOR_CMP_EQ16 : Z80Pseudo<(outs), (ins GR16NoIR:$lhs, GR16NoIR:$rhs)>
```

---

### Prioritet 3: Register Info Consolidering (Middel)

**Fil:** `llvm/lib/Target/Z80/Z80RegisterInfo.td`

**Problem:** GR16 inkluderer IX/IY, men mange instruktioner kan ikke bruge dem.

**Lösning:**
- Behold GR16 (med IX/IY) for instruktioner som kan bruge dem (PUSH/POP, LD, etc.)
- Brug GR16NoIR for instruktioner som ikke kan (arithmetik, compare, etc.)
- Opdater alle pseudo-instruktioner til korrekte register klasser

---

### Prioritet 4: Validering (Lav)

**Test:** `build-macos/bin/llvm-lit llvm/test/CodeGen/Z80/ -j8`

**Forventet resultat:**
- Passed: 243 (100%)
- Failed: 0
- XFAIL: 5 (bekendte)

---

## File List

### Krisitke (Blokerer build/test)
- [ ] llvm/lib/Target/Z80/Z80RegisterBankInfo.cpp
- [ ] llvm/lib/Target/Z80/Z80RegisterBankInfo.h

### Importante (10+ tests)
- [ ] llvm/lib/Target/Z80/Z80InstrInfo.td
- [ ] llvm/lib/Target/Z80/Z80RegisterInfo.td

### Sekundære (Få tests)
- [ ] llvm/lib/Target/Z80/Z80InstructionSelector.cpp
- [ ] llvm/lib/Target/Z80/Z80LegalizerInfo.cpp

---

## Test Validering

### Pre-fix (Current)
```
Total: 243
Passed: 125 (51.4%)
XFAIL: 5 (2.1%)
Failed: 113 (46.5%)
```

### Post-fix (Target)
```
Total: 243
Passed: 238 (98%)
XFAIL: 5 (2%)
Failed: 0
```

---

## References

- Merge commit: de5aaeb411d8
- Upstream commit med verifier: de88c57b6541
- Fork's register definitions: Z80RegisterInfo.td
- Upstream's pseudo constraints: Z80InstrInfo.td

---

## Notes

- GR16NoIR blev introduceret for at undgå IX/IY i instruktioner som ikke kan bruge dem
- Fork's AutoStaticStack og math32 er **ikke** påvirket af disse ændringer
- Machine verifier blev tilføjet i upstream for at fange netop disse typer fejl
