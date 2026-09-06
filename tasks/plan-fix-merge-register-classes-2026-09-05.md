# Plan: Fix Register Class Mismatch After Merge

**Oprettet:** 2026-09-05  
**Status:** In progress - merge reconstruction and validation
**Relateret til:** merge-upstream-2026-09-05 (commit de5aaeb411d8)

### Seneste validering (2026-09-05)

`llc` bygger efter de seneste legalizer-, sret-, selector-, frame- og
cleanupændringer. Full suite står nu på 233 PASS, 5 XFAIL og 5 FAIL
(243 tests i alt). De resterende fejl er `dangling-debug-value`,
`fixed-point`, `i1-normalize`, `trunc-global-address-byte` og
`vector-scalarize`. De fokuserede cleanup-, alignment-, carry-chain- og
sret-tests passerer. Ingen runtime- eller firmware-orakler er kørt.

De resterende fejl er registreret som selvstændige upstream-issues:

| Test | Issue |
|---|---|
| `dangling-debug-value.ll` | `ravn/llvm-z80#286` |
| `fixed-point.ll` | `ravn/llvm-z80#287` |
| `vector-scalarize.ll` | `ravn/llvm-z80#288` |
| `trunc-global-address-byte.ll` | `ravn/llvm-z80#289` |
| `i1-normalize.ll` | `ravn/llvm-z80#290` |

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

### Prioritet 1: Instruction Selector Fix (Hojt - Done)

**Fil:** `llvm/lib/Target/Z80/Z80InstructionSelector.cpp`

**Problem:** The selector created `GR16` virtual registers for pseudos requiring `GR16NoIR`.

**Lösning:** Create/constrain compare and shift operands with `GR16NoIRRegClass` at the selector sites. `getRegBankFromRegClass()` was not the failing layer and was left unchanged.

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

### Prioritet 2: Three-way merge reconstruction (Hojt - In progress)

**Filer:** `Z80InstructionSelector.cpp`, `Z80TargetMachine.cpp`,
`Z80ExpandPseudo.cpp`, `Z80LegalizerInfo.cpp`, `Z80InstrInfo.h`,
`Z80MCInstLower.cpp`

The merge result had dropped fork-specific code in several backend files, so
resolving only textual conflicts was insufficient. The affected files are
being reconstructed from fork-parent `48c1b4b461a3^1`, upstream-parent
`48c1b4b461a3^2`, and merge-base `273490ae5e63789b7daab50efb4532e5c0ea4009`.

Selector, target-machine, pseudo expansion and legalizer reconstruction now
build. `Z80InstrInfo.h` retains the fork's target flags and cost hooks while
restoring upstream's `markUndefUse`/`emitHLSavePush` helpers. Target flag
aliases keep `MO_LO`/`MO_HI` compatible with upstream's address flag names.

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

### Prioritet 4: Validering (In progress)

**Test:** `build-macos/bin/llvm-lit llvm/test/CodeGen/Z80/ -j8`

The reconstructed backend builds. Focused tests for pseudo-size verification
and `djnz.ll` pass. `vector-scalarize.ll` still fails during legalization on
`G_UNMERGE_VALUES` from `<2 x s8>`. Full-suite validation is now 215 PASS,
5 XFAIL and 23 FAIL. Runtime/value oracles remain outstanding.

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

### Current checkpoint
```
215 PASS, 5 XFAIL, 23 FAIL after the InstrInfo-header/MC-lowering
compatibility work. The remaining failures are concentrated in integer
arithmetic/support-width codegen, liveness/MIR cases, and vector ABI
legalization.
```

### Failure classification (2026-09-06)

The 23 failures are not 23 independent bugs:

| Count | Class | Tests | Observed cause |
|---:|---|---|---|
| 9 | MIR harness | `issue-197`, `issue-200`, `issue-209`, `issue-210-*`, `issue-212`, `issue-236`, `issue-237` | `-run-pass=prologepilog` is not registered in this build, so the test never reaches Z80 codegen. |
| 4 | Partial-register verifier | `arith-i32`, `i64-support`, `i128-support`, `fixed-point` | Late code emits `AND_A`/`LD_HLind_B` with an undefined physical source register. |
| 1 | Register-class verifier | `satarith` | Generated compare operands still mismatch `GR16` and `GR16NoIR`. |
| 1 | Scalar-width verifier | `i1-normalize` | The selector copies an `s1` vreg directly into the 8-bit accumulator instead of widening/materializing it first. |
| 1 | Vector legalization | `vector-scalarize` | `G_UNMERGE_VALUES` has vector source `<2 x s8>`, but the legalizer only accepts scalar pair shapes. |
| 1 | Vector construction | `dangling-debug-value` | Generic lowering creates an invalid `<2 x s128>` `G_BUILD_VECTOR` from 16-bit elements, exposing missing large-vector legalization bounds. |
| 1 | sret lowering | `sret-no-args` | The sret temporary is emitted as `G_STORE` with `$noreg`, losing pointer/type information before verification. |
| 2 | Stack alignment policy | `branch-folder-unsound-hoist-pi-cse-miscompile`, `divmod-i32-fused` | Tests create 2-byte-aligned automatic objects while Z80 SP is byte-aligned; the backend rejects them before the intended check. |
| 2 | Expected assembly drift | `ret-cleanup-hlde`, `bit-test` | Codegen reaches assembly, but current instruction selection no longer matches the tests' exact `CHECK`/`CHECK-NEXT` sequence. |
| 1 | Address-half lowering | `trunc-global-address-byte` | The emitted symbol-half syntax/target flags do not match the test's expected `z80_16hi` and full-address forms. |

The nine MIR failures are test-infrastructure/configuration failures, not
evidence that the corresponding liveness fixes are wrong. The other 14 need
backend investigation; verifier failures should be fixed before refreshing
FileCheck expectations.

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
