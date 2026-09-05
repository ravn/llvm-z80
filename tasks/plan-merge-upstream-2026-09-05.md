# Plan: Merge llvm-z80/llvm-z80 upstream into ravn/llvm-z80 main

**Oprettet:** 2026-09-05  
**Til:** Copilot / Mistral / næste Claude-session  
**Status:** IKKE STARTET — merge-branch findes ikke endnu

---

## Kontekst

`ravn/llvm-z80` er en fork af `llvm-z80/llvm-z80` (ejet af @zlfn).
Upstream har fået 12.046 nye commits siden vores divergering 2026-06-06
(`[Z80] Support fminimum_num / fmaximum_num #28`).
Vores fork har 1.011 egne commits siden da (primært Z80-optimeringer).

Workspace-rod: `/Users/ravn/z80/` (macbook) eller `/home/ravn/z80/` (sonnyboy).  
Submodul-sti: `llvm-z80/`.  
Upstream remote hedder `upstream` og peger på `https://github.com/llvm-z80/llvm-z80.git`.

---

## Hvad upstream har tilføjet (de vigtige ting)

**42 [Z80] commits fra @zlfn — alle nye features:**

```
d92665bdb423 [Z80] Add the float to int128 conversion routine
f99071c19ee4 [Z80] Reject atomic read-modify-write as unsupported
d3121dca3f86 [Z80] Drop debug uses of values the legalizer erased
b5c01ace421e [Z80] Route 128-bit float conversions through their libcalls
b90639774c00 [Z80] Lower modf through its libcall
0f17b67dffe5 [Z80] Implement frame and return address intrinsics
1c452190f72e [Z80] Set the user label prefix to match the data layout
a43cc5fea89c [Z80] Dissolve llvm.clear_cache
1a96bd0c8ebc [Z80] Lower indirect register asm outputs and reject wide asm operands
468f7671bec3 [Z80] Describe the byte register breakdown via the calling convention hooks
841d84e5e1d7 [Z80] Size every pseudo that expands after branch relaxation
921ebf0a5ef7 [Z80] Cover the SP-copy register class in the register bank
bcca8af238e6 [Z80] Keep HL intact in callee-cleanup returns that use it
9d0fa45abd97 [Z80] Legalize va_copy
e22d9f6e855c [Z80] Read aggregate varargs in place
79a072de6424 [Z80] Reject asm goto with a proper error
3a2c24a11d39 [Z80] Legalize truncating stores
95e165526788 [Z80] Legalize vector operations by scalarizing
ce3671d87b18 [Z80] Give a zero-sized byval argument a one-byte stack object
e623cd9c1832 [Z80] Clear stale liveness flags when saving HL late
7571f8f7b9f3 [Z80] Give conditionally skipped expansion tails their own blocks
38dfbd156e59 [Z80] Read the sret pointer even in functions without arguments
dab0ad00d8ac [Z80] Byte-align the fixed-point and storage-only float types
adca69a96987 [Z80] Place symbol fixups on the operand byte, not the opcode
8308086dfde2 [Z80][SM83] Declare complex types unsupported
dea42d4942d4 [Z80][SM83] Stop exporting the runtime's internal labels
2a99610c152d [Z80] Select one byte of a symbol's address as an 8-bit immediate
2ec74cf68a88 [Z80] Amortize spill slot access in the late peephole pass
ab0d79badfc0 [Z80] Keep the host's headers out of the include path
0c062bedbc69 [Z80] Refuse over-aligned stack objects with a proper error
de88c57b6541 [Z80] Run the machine verifier in every lit test
d39878e0495d [Z80] Recompute live-ins for blocks made by pseudo expansion
20a802c03c10 [Z80] Declare call stack cleanup clobbers per call site
25a61afa2cbd [Z80] Mark register reads whose value cannot matter as undef
3e66ef21596b [Z80] Give the spill pseudos operands that match what they carry
6d608b4a0545 [Z80] Clear the bits above a bool at every producer
ce3ceb2e0898 [Z80] Mark the unread halves of a one-way EX DE,HL
f2c668f78f4c [Z80] Fix build after upstream merge
4bcb904bdc6b [Z80] Emit repeated non-zero bytes as .db for sdas
e6fb1a45b146 [Z80] Register the intrinsic opcodes as legal
91cc6f54bc7e [Z80] Add 1-stage Release CMake cache and packaging script
df87408458ac [Z80] Support 8-bit atomic load and store
```

Resten (12.004 commits) er LLVM-kerneændringer + clang/libc/lld som upstream har
merget fra officielt LLVM. De fleste mergede rent — kun Z80-specifikke filer konflikter.

---

## Konfliktfiler og antal konflikter

Verificeret ved `git merge --no-commit --no-ff upstream/main` på en test-branch:

| Fil | Konflikter | Karakter |
|---|---|---|
| `llvm/lib/Target/Z80/Z80InstrInfo.cpp` | 19 | Instruktions-helpers, nye opcodes begge sider |
| `llvm/lib/Target/Z80/Z80RegisterInfo.cpp` | 9 | Register-klasser, SP-kopi |
| `llvm/lib/Target/Z80/Z80LegalizerInfo.cpp` | 9 | Nye legaliseringsregler begge sider |
| `llvm/lib/Target/Z80/Z80InstrFormats.td` | 5 | Instruktionsformat-definitioner |
| `llvm/lib/Target/Z80/Z80CallLowering.cpp` | 5 | Calling convention hooks (upstream) + vores cc-arbejde |
| `llvm/lib/Target/Z80/Z80ExpandPseudo.cpp` | 4 | Pseudo-sizing (upstream) + vores ekspansioner |
| `llvm/lib/Target/Z80/Z80FrameLowering.cpp` | 4 | Frame-håndtering |
| `llvm/lib/Target/Z80/Z80LateOptimization.cpp` | 3 | Spill-amortisering (upstream) + vores peephole |
| `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` | 2 | Symbol-adressering (upstream) + vores selektorer |
| `llvm/lib/Target/Z80/Z80InstrInfo.td` | 2 | Tabelgen-definitioner |
| `llvm/lib/Target/Z80/Z80ISelLowering.cpp` | 1 | |
| `llvm/lib/Target/Z80/Z80MCInstLower.cpp` | 1 | Symbol-fixup (upstream) |
| `clang/lib/Basic/Targets/Z80.cpp` | 1 | Z80 target info |
| `clang/include/clang/Basic/DiagnosticSemaKinds.td` | 1 | Diagnostik (vores cc281) |
| `clang/lib/Headers/CMakeLists.txt` | 1 | intrinsic.h (vores) |
| `compiler-rt/lib/builtins/z80/memchr.asm` | 2 | |
| `compiler-rt/lib/builtins/z80/memcpy.asm` | 1 | |
| `compiler-rt/lib/builtins/z80/memset.asm` | 1 | |
| `llvm/test/CodeGen/Z80/atomic.ll` | 1 | Test-cases begge sider |
| `llvm/test/CodeGen/Z80/large-frame.ll` | 1 | |
| `llvm/test/CodeGen/Z80/vararg.ll` | 1 | |
| `llvm/test/MC/Z80/lit.local.cfg` | 1 | add/add konflikt |
| `llvm/lib/Transforms/AggressiveInstCombine/TruncInstCombine.cpp` | 1 | Ikke-Z80, se note |
| `z80-utils/test-runner/scripts/verify-production.sh` | — | Upstream slettet, vores beholdes |

**Total: ~75 konflikter i 24 filer.** Alle øvrige 12.000+ upstream commits mergede rent.

---

## Generel konfliktløsningsstrategi

**Princip: Behold BEGGE siders bidrag.** Upstream tilføjer nye features (int128, atomics,
va_copy, truncating stores, frame intrinsics). Vores fork tilføjer optimeringer (peephole,
calling conventions, AutoStaticStack, math32). De er sjældent i direkte semantisk konflikt
— konflikterne skyldes at begge sider har tilføjet kode i de samme funktioner/blokke.

For hvert `<<<<<<<` / `=======` / `>>>>>>>` blok:
1. Læs hvad HEAD (vores) har tilføjet
2. Læs hvad upstream har tilføjet
3. Vurder om de er uafhængige → behold begge
4. Vurder om de løser det samme problem forskelligt → brug upstream's version
   (de kender LLVM API'et bedre) MEN tjek at vores optimering ikke går tabt
5. Vurder om de er modstridende → eskaler til ravn (Thorbjørn)

**Tommelfingerregel for Z80-backend-filer:**
- Upstream's nye legaliseringsregler (`Z80LegalizerInfo.cpp`): behold upstream
- Upstream's nye pseudo-sizing (`Z80ExpandPseudo.cpp`): behold upstream — kritisk for korrekthed
- Upstream's calling convention hooks (`Z80CallLowering.cpp`): behold upstream
- Vores peephole-optimeringer (`Z80LateOptimization.cpp`): behold vores
- Vores calling conventions (smallc, z88dk_callee, cc133): behold vores — disse eksisterer ikke i upstream
- `verify-production.sh`: behold vores (upstream har ikke den fil)

---

## Trin-for-trin procedure

### Trin 1: Opret merge-branch og start merge

```bash
cd /Users/ravn/z80/llvm-z80
git checkout main
git fetch upstream
git checkout -b merge-upstream-2026-09-05 main
git merge --no-ff upstream/main
# Merge stopper med konflikter — det er forventet
```

### Trin 2: Løs konflikterne fil for fil

Anbefalet rækkefølge (simplest først):

#### 2a. `llvm/test/MC/Z80/lit.local.cfg` (add/add, 1 konflikt)
Begge sider har tilføjet linjer. Behold begge siders tilføjelser.

#### 2b. Test-filer: `atomic.ll`, `large-frame.ll`, `vararg.ll` (1 konflikt hver)
Upstream har tilføjet test-cases for nye features. Behold begge siders test-cases.
HEAD's tests er vores regressionstests — de må ikke fjernes.

#### 2c. `z80-utils/test-runner/scripts/verify-production.sh`
Upstream har slettet denne fil (de har den ikke). Filen er allerede bevaret i
working tree (git viser den som "modify/delete"). Kør:
```bash
git add z80-utils/test-runner/scripts/verify-production.sh
```

#### 2d. `clang/lib/Headers/CMakeLists.txt` (1 konflikt)
Vores side har tilføjet `intrinsic.h` til listen. Behold vores tilføjelse plus
hvad upstream evt. har tilføjet.

#### 2e. `clang/include/clang/Basic/DiagnosticSemaKinds.td` (1 konflikt)
Vores side har diagnostikker for Z80 calling-convention-konflikter (#281).
Behold vores diagnostikker plus upstream's.

#### 2f. `clang/lib/Basic/Targets/Z80.cpp` (1 konflikt)
Sandsynligvis relateret til vores tilføjede Z80-attributter eller upstream's
label-prefix-ændring. Behold begge bidrag.

#### 2g. `compiler-rt/lib/builtins/z80/memcpy.asm`, `memset.asm`, `memchr.asm`
Tjek om upstream har ændret de samme rutiner som vi har. Vores versioner er
tilpasset z88dk-ABI'en. Behold vores logik, tag upstream's rettelser ind hvis de
ikke kolliderer med vores ABI-krav.

#### 2h. `llvm/lib/Transforms/AggressiveInstCombine/TruncInstCombine.cpp` (1 konflikt)
Dette er ikke Z80-specifik kode. Se på konflikten — sandsynligvis en LLVM-kernefil
der begge sider har rørt. Brug upstream's version medmindre vores ændring er
intentionel (tjek `git log -p -- <fil>` for at se vores commit-besked).

#### 2i. `Z80InstrInfo.h` (1 konflikt)
Header-fil. Behold alle nye metode-erklæringer fra begge sider.

#### 2j. `Z80InstrInfo.td`, `Z80InstrFormats.td` (2 og 5 konflikter)
TableGen-filer. Nye instruktionstilføjelser fra begge sider. Behold alle nye
instruktionsdefinitioner. Pas på enum-værdier der eventuelt er renummereret.

#### 2k. `Z80MCInstLower.cpp` (1 konflikt)
Relateret til upstream's "Place symbol fixups on the operand byte, not the opcode"
(`adca69a96987`). Behold upstream's rettelse.

#### 2l. `Z80ISelLowering.cpp` (1 konflikt)
Behold begge bidrag.

#### 2m. `Z80InstructionSelector.cpp` (2 konflikter)
Upstream's "Select one byte of a symbol's address as an 8-bit immediate"
(`2a99610c152d`). Vores selektorer. Behold begge — de dækker sandsynligvis
forskellige IR-mønstre.

#### 2n. `Z80LateOptimization.cpp` (3 konflikter)
Upstream: "Amortize spill slot access in the late peephole pass" (`2ec74cf68a88`).
Vores: peephole-optimeringer tilføjet siden juni 2026.
**Kritisk:** Behold begge siders peephole-regler. De er sandsynligvis uafhængige
optimeringer der begge er ønskede. Tjek at vores optimeringer kommer EFTER
upstream's nye regler i funktionen, eller flyt dem til et passende sted.

#### 2o. `Z80FrameLowering.cpp` (4 konflikter)
Upstream: frame/return address intrinsics (`0f17b67dffe5`).
Vores: `z80_critical` DI/EI frame-håndtering.
Behold begge — de dækker forskellige aspekter af frame-lowering.

#### 2p. `Z80ExpandPseudo.cpp` (4 konflikter)
Upstream: "Size every pseudo that expands after branch relaxation" (`841d84e5e1d7`),
"Give conditionally skipped expansion tails their own blocks" (`7571f8f7b9f3`),
"Recompute live-ins for blocks made by pseudo expansion" (`d39878e0495d`).
**Vigtigt:** Upstream's pseudo-sizing er en korrekthedsfiks (tidligere var det en bug
klassen der brød `jr`-range-beregning). Sørg for at ALLE vores pseudos også har
korrekt størrelse. Tjek `getInstSizeInBytes()` for alle vores egne pseudos.

#### 2q. `Z80CallLowering.cpp` (5 konflikter)
Upstream: "Describe the byte register breakdown via the calling convention hooks"
(`468f7671bec3`), "Keep HL intact in callee-cleanup returns" (`bcca8af238e6`),
"Read the sret pointer even in functions without arguments" (`38dfbd156e59`).
Vores: smallc/z88dk_callee/cc133 calling conventions.
**Kritisk:** Behold alle vores calling-convention-implementeringer. Upstream kender
ikke til z88dk_callee/smallc — de er fork-specifikke. Integrér upstream's rettelser
(sret-pointer-fix er en korrekthedsfiks der er vigtig).

#### 2r. `Z80LegalizerInfo.cpp` (9 konflikter)
Upstream: truncating stores (`3a2c24a11d39`), vector scalarization (`95e165526788`),
va_copy (`9d0fa45abd97`), varargs in place (`e22d9f6e855c`), byval zero-sized
(`ce3671d87b18`), stale liveness (`e623cd9c1832`), etc.
Vores: diverse legaliseringsregler tilføjet siden juni 2026.
Behold begge siders regler. Tjek at rækkefølgen i `setAction()` / `setLegalizeScalarToDifferentSizeStrategy()`
er konsistent.

#### 2s. `Z80RegisterInfo.cpp` (9 konflikter)
Upstream: "Cover the SP-copy register class in the register bank" (`921ebf0a5ef7`),
"Declare call stack cleanup clobbers per call site" (`20a802c03c10`), etc.
Behold begge bidrag. Pas særligt på register-klasse-definitioner der kan have
fået nye entries begge steder.

#### 2t. `Z80InstrInfo.cpp` (19 konflikter — den sværeste)
Upstream: diverse nye helper-funktioner og instruktions-queries.
Vores: vores egne helpers.
Gå metodisk igennem én konflikt ad gangen. For hver:
- Ny metode fra upstream → behold
- Ny metode fra os → behold
- Ændring i eksisterende metode → sammenlign semantisk, behold den korrekte

### Trin 3: Verificer og commit

```bash
# Tjek at ingen konfliktmarkører er tilbage
grep -r "^<<<<<<" llvm/lib/Target/Z80/ llvm/test/CodeGen/Z80/

# Kør lit-tests
cd /Users/ravn/z80/llvm-z80
# Build først (Docker eller native):
# ninja -C build-macos clang llc llvm-lit   (native macbook)
# eller cmake + ninja i Docker

# Kør Z80 lit suite
build-macos/bin/llvm-lit llvm/test/CodeGen/Z80/ -j8
# Forventet: 164 PASS + 6 XFAIL (eller bedre — upstream tilføjer nye tests)

# Commit
git add -A
git commit --no-ff -m "Merge upstream/main (llvm-z80/llvm-z80) into ravn fork

Merge-base: 273490ae5e63 (2026-06-06)
Upstream: 12046 new commits including 42 [Z80] features:
- int128 float conversions
- Atomic load/store (8-bit)
- va_copy, varargs in place
- Truncating stores, vector scalarization
- Frame and return address intrinsics
- Symbol fixup placement
- Pseudo sizing correctness
- Spill slot amortization in late peephole
- Calling convention hooks

Fork's 1011 commits preserved:
- smallc / z88dk_callee / cc133 calling conventions
- AutoStaticStack cross-TU
- math32 / float32 bridge
- Z80 peephole optimizations
- RC700 firmware-specific fixes"
```

### Trin 4: Push og opdater workspace-submodul

```bash
git push origin merge-upstream-2026-09-05
# Review — hvis tilfreds:
git checkout main
git merge --no-ff merge-upstream-2026-09-05
git push origin main

# Opdater workspace-repo
cd /Users/ravn/z80
git add llvm-z80
git commit -m "llvm-z80: merge upstream/main (llvm-z80/llvm-z80) 2026-09-05"
git push origin main
```

---

## Hvad der IKKE må gå tabt (vores unikke bidrag)

Disse features eksisterer kun i `ravn/llvm-z80` — ikke i upstream. Sørg for at de
overlever mergen:

- **`z80_smallc` / `z80_callee` / cc133-kombination** (`Z80CallLowering.cpp`, `clang/lib/Basic/Targets/Z80.cpp`)
- **`AutoStaticStack`** — automatisk cross-TU BSS-allokering (`Z80FrameLowering.cpp`, evt. andre)
- **`-z80-float-sdcccall0`** — math32-flag der router 32-bit float libcalls til sdcccall(0)
- **`__builtin_z80_di/ei/halt/nop/im2/set_i`** og **`__attribute__((z80_critical))`** (`clang/lib/Basic/Targets/Z80.cpp`, `Z80FrameLowering.cpp`)
- **`<intrinsic.h>`** header (`clang/lib/Headers/CMakeLists.txt`)
- **`z80-verify-inline-runtime-size`** diagnostik-flag
- **`z80-utils/test-runner/`** og produktionsverifikations-scripts
- **Alle lit-tests i `llvm/test/CodeGen/Z80/`** som vi har tilføjet

---

## Hvis du sidder fast

- Spørg Thorbjørn (ravn) om specifikke semantiske valg
- Se commit-beskeder: `git log --oneline upstream/main ^main | grep Z80` for upstream,
  `git log --oneline main ^upstream/main | grep Z80` for vores
- For en given konfliktfil, se begge siders commits: `git log --oneline -p <hash> -- <fil>`
- Korrekthedsprioritet: pseudo-sizing > sret-fix > register-korrekthed > optimeringer
