# Plan: Direkte z88dk-runtime-integration i llvm-z80

**Dato:** 2026-10-02
**Ophav:** Opus 4 analyse
**Mål:** `z80-unknown-none-z88dk`-triplen linker mod z88dk-runtime med
triple-styret CC-valg. Eksisterende z88dk-adaptere må bruges; der skrives
ingen nye bridges/wrappere i llvm-z80 eller z88dk.

## Historisk status quo (kortlagt, 2026-10-02)

- `upstream-z88dk-native-runtime` blev oprettet fra `upstream/main` (`24afb830878c`). Baseline før integration: **134/134 PASS**.
- `z80-unknown-none-z88dk`-triplen er **ikke** i upstream/main endnu — den lever kun i vores `upstream-triple-z88dk`-branch.
- `z88dk-native-libcalls` har native libcall-navne (`cm32_sdcc_fsadd`, `l_mulu_16_16x16`, `asm_memmove` m.fl.) men er lavet mod et ældre upstream (før Ac-regclass + Z80NarrowMemAccess + CP_Ac_r + #58).
- PR #59 og PR #60 er flag-only og forældede — CC-valg skal styres af triplen direkte.
- `upstream-triple-z88dk` indeholdt triple-implementeringen, men var ikke ajour med upstream/main.

## 1. Branch-strategi

`upstream-z88dk-native-runtime` er oprettet fra `upstream/main`, og integrationen
er allerede landet på branchen. De historiske cherry-pick-kommandoer nedenfor
er ikke længere en udførelsesplan. Afsnit 7 beskriver den daværende restliste;
implementering og endelige resultater står i afsnit 9.

## 2. Triple-styret float — z88dk math32

**Valgt retning (2026-10-03):** kald z88dk's eksisterende math32 ABI-indgange
`cm32_sdcc_*` med `CallingConv::Z80_SDCCCall0`. Der må ikke tilføjes
nye bridge-/wrapper-filer for at få dette til at virke — hverken i llvm-z80
eller z88dk. Adaptere, der allerede findes i z88dk's almindelige runtime, er
acceptable, også når de internt tilpasser ABI'et til math32-kernerne. Direkte
EXX-lowering til `m32_*`-kernerne er ikke et krav. Hvis der mangler en
eksisterende runtime-indgang, registreres det som en åben gap; der skrives
ikke en ny adapter som workaround.

**Implementeret og end-to-end-verificeret; resultater i afsnit 9:**
- G_FADD → `cm32_sdcc_fsadd`, G_FSUB → `cm32_sdcc_fssub`, G_FMUL → `cm32_sdcc_fsmul`, G_FDIV → `cm32_sdcc_fsdiv`
- G_FPTOSI → `cm32_sdcc___fs2sint`, G_FPTOUI → `cm32_sdcc___fs2uint`
- G_SITOFP → `cm32_sdcc___slong2fs`, G_UITOFP → `cm32_sdcc___ulong2fs`

**Float compare følger foreløbig en finite-only-testpolicy:** NaN er uden for
runtime-inputdomænet efter brugerens valg. Math32' faktiske NaN-adfærd er ikke
verificeret; tests må hverken forudsætte den eller hævde IEEE NaN-semantik.
Compilerens mapping bruger eksisterende `cm32_sdcc___fseq`, `___fsneq`,
`___fslt` og `___fsgt`-indgange til finite værdier. `ORD`/`UNO` sættes til
konstanter inden for denne begrænsede kontrakt. Der må ikke tilføjes nye
runtime- eller compiler-bridge-/wrapper-filer.

Fjern `__cmpsf2.asm`-afhængigheden først, når alle relevante predicates er
korrekt håndteret, og runtime-testen linker og passerer uden den fil.

PR #59 lukkes — flagets funktion er absorberet i triplen.

## 3. Native libcall-navne

**Symbolencoding (2026-10-03):** z80asm-navne med punktummer length-encodes
efter LLVM's symbolpræfikser: `_test.counter` -> `L5__test7_counter` og
`L_.str.1` -> `L2_L_3_str1_1`. Navne uden punktummer ændres ikke, så
almindelige C-/runtime-navne bevarer ABI'et. Længder tæller bytes og
bevarer tomme dele. Kollisionsgarantien omfatter almindelige C-symboler,
ikke brugerdefinerede eksplicitte asm-navne.

Mekanisme: rå `MCSymbol` via `MCContext::getOrCreateSymbol(StringRef)` — IKKE `GetExternalSymbolSymbol` (Mach-O tilføjer `_`-præfiks).

| LLVM-navn | z88dk-navn | ABI |
|---|---|---|
| `__divhi3`/`__modhi3` | `l_divs_16_16x16` | Register: HL÷DE → HL=kvotient, DE=rest |
| `__udivhi3`/`__umodhi3` | `l_divu_16_16x16` | Register: HL÷DE → HL=kvotient, DE=rest |
| `__mulhi3` | `l_mulu_16_16x16` | Register: HL×DE → HL=produkt |
| `__udivqi3`/`__umodqi3` | `l_fast_divu_8_8x8` | Register: direkte (se kilde) |
| `__mulsi3` | `l_mulu_32_32x32` | EXX-protokol |
| `__divsi3`/`__udivsi3` | `l_div[su]_32_32x32` | EXX-protokol |
| `__addsf3`/`__subsf3`/`__mulsf3`/`__divsf3` | `cm32_sdcc_fsadd`/`fssub`/`fsmul`/`fsdiv` | SDCCCall0 f32 |
| `__fixsfsi`/`__fixunssfsi` | `cm32_sdcc___fs2sint`/`___fs2uint` | SDCCCall0 |
| `__floatsisf`/`__floatunsisf` | `cm32_sdcc___slong2fs`/`___ulong2fs` | SDCCCall0 |
| memmove (G_MEMMOVE) | `asm_memmove` (HL=src, DE=dst, BC=count) | fast |
| memset (G_MEMSET) | `asm_memset` (HL=dst, E=val, BC=count) | fast |

**Float compare status:** `cm32_sdcc___fseq`, `___fsneq`, `___fslt` og
`___fsgt` findes i z88dk's runtime og bruges til finite predicates. NaN er
uden for testkontrakten; den faktiske NaN-adfærd er fortsat uafklaret. Fast-
math og alle fire eksisterende predicate-symboler har compile/runtime-
dækning.

## 4. Runtime library-aware optimization CC (printf→puts)

**Beslutning 2026-10-03:** ingen target-specifik CC-stamping i BuildLibCalls.
Den tidligere stamping ændrede eksisterende deklarationer uden at opdatere
tidligere kald og er fjernet. SimplifyLibCalls' konservative ABI-gate beholdes:
headerens sdcccall(0)-printf optimeres ikke til SmallC-puts.
`z88dk/test/clang/runtime_printf_puts.c` og `.sh` kontrollerer fraværet af
folding og korrekt output ved O2/O3/Oz. Almindelige C-konventionsdeklarationer
beholder LLVM's almindelige optimeringsadfærd.
Frontendens accept af eksplicitte headerkonventioner er fortsat nødvendig;
den gør ikke disse konventioner C-kompatible i LLVM's libcall-simplifier.

## 5. C_LINE direktiver

Implementeret på branchen:
- `a32a96076aea` — C_LINE emission (DebugLoc + isZ88DK())
- `f5a4ad5300e1` — kun scope-form (partial cherry-pick; `-z80-assume-no-callbacks` udelades)

Guard: `MAI.isZ88DK()` — den nye metode fra #58 er allerede på plads.

## 6. Testplan

**Lit-tests (`llvm/test/CodeGen/Z80/`):**
- `z88dk-triple-defaults.ll` — format z80asm, `__addsf3`→`cm32_sdcc_fsadd`, `printf`→`puts` med Z80_SmallC
- `z88dk-float-arith.ll` — fadd/fsub/fmul/fdiv/fptosi/sitofp og FCMP;
  check faktiske runtime-symboler og `sdcccall(0)`-ABI
- `z88dk-i16-mul-div.ll` — `l_mulu_16_16x16`, `l_divs_16_16x16`, resultat HL/rest DE
- `z88dk-i32-mul-div.ll` — `l_mulu_32_32x32`, EXX-protokol
- `z88dk-mem-intrinsics.ll` — `asm_memmove`/`asm_memset`, register-opsætning
- `z88dk-classic-libc-cc.ll` — printf("x\n")→puts, Z80_SmallC push-rækkefølge
- `z88dk-c-line.ll` — `-g`, `C_LINE N, "file"` emitteres og dedupliceres (ingen test for `-z80-assume-no-callbacks`)
- **Negative tests**: samme IR mod `z80-unknown-elf` giver compiler-rt-navne, default C ABI, DWARF

**Runtime-tests (`z80-utils/test-runner/testcases/clang/`):**
- `z88dk_float_arith.c` — bit-exact IEEE-754 mod z88dk math32 for
  arithmetic/conversions
- Float compare runtime-matrix — finite ordered predicates og fast-math;
  NaN er eksplicit uden for runtime-testens understøttede inputdomæne
- `z88dk_printf_puts.c` — printf("hello\n") → puts under ntvcm
- `z88dk_memmove_overlap.c` — overlappende memmove korrekthed
- `z88dk_i32_div.c` — stor divisor+kvotient, verificer mod host
- Link/runtime check uden `__cmpsf2.asm` eller andre llvmz80 bridge-/wrapper-
  filer; behold positiv kontrol mod almindelig `z80-unknown-elf`.

## 7. Baseline før rettelser (historisk; aktuel status i afsnit 9)

**Historisk baseline:** `upstream-z88dk-native-runtime` på `upstream/main`,
134/134 PASS før integrationen.

**Aktiv branch: `upstream-z88dk-native-runtime`.** Triple-understøttelse,
calling conventions, i16/memory mapping, float arithmetic/conversion mapping
og C_LINE-emission findes her, men i16 div/rem-resultatmappingen er forkert.
Direkte i32 div/rem-mapping findes på `z88dk-native-libcalls`, ikke på denne
branch; den aktive lowering bruger stadig `__divsi3`-familien. Det tidligere
udsagn om at hele integer-integrationen allerede var implementeret her var
for bredt. Den historiske baseline verificerer ikke den nuværende branch.

**Verificeret 2026-10-03:** 142/142 Z80 lit-tests passerer; root
`run-llvmz80-tests.sh` passerer (432 runtime: 426 PASS/0 FAIL/6 SKIP; lit:
149 PASS/0 FAIL). Begge `runtime_fcmp` finite-only tests passerer via
zcc/ntvcm. `z88dk/libsrc/l/llvmz80.lst` udelader lokalt `__cmpsf2.asm`, og
compare-tests linker og kører uden den. `run-z88dk-tests.sh` gennemførte alle
68 tests: 44 PASS, 13 FAIL, 11 XFAIL. Fejlene er i benchmark, stdio/FILE*,
integer-div/rem, long, printf-return og qsort-tests; ingen FCMP-test fejlede.
Der er ikke kørt en før-baseline for de øvrige fejl, så de kan ikke kaldes
præ-eksisterende eller tilskrives denne ændring. NaN-semantikken er ikke testet.

Resterende:
1. Triage er afsluttet for de 13 FAILs, se nedenfor; implementation og
   uændret-suite-verifikation af årsagerne mangler stadig.
2. Fortsæt øvrig z88dk runtime-verifikation uden nye bridges/wrappers.
3. Ingen PR/ekstern filing uden eksplicit go-ahead.

## 8. FAIL-triage (2026-10-03)

Alle 13 FAILs er reproduceret med eksplicit
`LLVMZ80EXE=llvm-z80/build-macos-asserts/bin/clang`, classic clib og ntvcm.
Ingen compilerkode eller eksisterende expected-værdier blev ændret. Efter
brugerens anmodning er den eksisterende qsort-test udvidet med en uafhængig
fixed-data callback-assertion; den oprindelige LCG-assertion er bevaret.
Reproer, faktisk IR/asm og diagnostiske logs ligger i
`scratch/tmp/z88dk-fails-20261003/` under workspace-roden. Ingen patch,
merge, commit, push eller ekstern filing er foretaget.

| Gruppe | FAILs | Observation |
|---|---|---|
| Libc builtin ABI | `issue22_stdio_abi`, `runtime_fileio_ferror_feof`, `runtime_fileio_qsort`, `runtime_fileio_rbplus`, `runtime_fileio_status`, `runtime_fileio_update`, `runtime_printf_ret` | Alle syv kontrol-builds får korrekte runtime-resultater med diagnostisk `-Cg-fno-builtin`; de almindelige scripts er uændrede |
| i16 quotient/remainder | `xfail_signed_mod`, `runtime_qsort` | Division og modulo henter hinandens resultatregistre; qsort-fixturens LCG-data er forkerte før callbacken |
| Manglende i32 integration | `runtime_intdiv`, `runtime_long` | Link fejler på `__divsi3`, `__modsi3`, `__udivsi3`, `__umodsi3`, samt fused helpers |
| Forældede benchmarks | `bench_math32_vs_compilerrt`, `bench_math32_vs_compilerrt_size` | Fjernet backend-flag afvises; derefter mangler deres eksplicitte bridge-inputs |

```text
=== Bug analysis: classic libc builtin calling conventions ===

Smallest repro:
  scratch/tmp/z88dk-fails-20261003/stdio.c
  zcc +cpm -compiler=llvmz80 -O2

Pass output (what the named pass actually produces):
  Preprocessed declaration:
    extern FILE *fopen(...) __attribute__((smallc));
  Sema AST, builtins enabled:
    fopen 'FILE *(const char *, const char *)'
  Sema diagnostic with -Wsystem-headers:
    smallc calling convention is not supported on builtin function
  Frontend IR at -O0, builtins enabled:
    %call = call ptr @fopen(...)
    %call3 = call i16 @fwrite(...)
    %call7 = call i16 (ptr, i16, ptr, ...) @snprintf(...)
  Frontend IR with -fno-builtin:
    call cc129 ptr @fopen(...)
    call cc129 i16 @fwrite(...)
    call z80_sdcccall0 i16 (...) @snprintf(...)

Current behavior:
  The recognized builtin declarations replace explicit header ABIs with C.
  The actual assembly passes fopen arguments in HL/DE, and reads fwrite's
  return from DE instead of the classic worker's HL.
  Minimal runtime: error=1 write=7586 seek=1, versus error=0 write=3 seek=0
  in the no-builtin control. snprintf is folded in this minimal program;
  runtime_printf_ret separately proves the unfurled return-value failure.

Expected behavior:
  Calls into classic clib must match its actual argument and return ABI,
  regardless of whether Clang recognizes the source function name.

Root cause:
  clang/lib/Sema/SemaDecl.cpp:3910-3921 ignores an explicit calling convention
  when redeclaring a builtin and resets it to the previous builtin CC.
  z88dk/include/sys/proto.h declares natural-order smallc workers;
  z88dk/include/sys/compiler.h:147-150 pins variadic workers to sdcccall(0).
  These attributes disappear in the enabled-builtin AST before optimization.

Evidence it's wrong (not just suboptimal):
  Header contract, AST, IR, actual worker assembly and runtime disagree.
  All seven controlled no-builtin builds produce the required fixture outputs.

Contaminated:
  - This is the local LLVM-Z80 fork and current classic z88dk library.
  - The Sema rule is intentional generic Clang behavior; the demonstrated
    defect is the Z88DK integration contract, not a proven generic LLVM bug.
  - Controls use an extra diagnostic flag, not an unchanged-suite pass.
  - No AVR comparison: this failure is specific to the Z88DK library ABI.

Doubt:
  Other independently failing ABI surfaces may remain after this cause is addressed.

Verdict:
  REAL-BUG in the current Z88DK integration; no generic-upstream verdict.

Recommended posture:
  HOLD AS FORK KNOWLEDGE.

Rules-checked: feedback_file_bugs_not_fixes, feedback_verdict_after_real_pass_output,
  feedback_minimal_repro_before_source_dive, feedback_state_certainty

=== Bug analysis: i16 quotient/remainder mapping and qsort data ===

Smallest repro:
  scratch/tmp/z88dk-fails-20261003/div16.c
  Independent qsort data probe: scratch/tmp/z88dk-fails-20261003/lcg.c

Pass output (what the named pass actually produces):
  Instruction selection / final assembly for division:
    _quotient:
      call l_divs_16_16x16
      ret
  For modulo:
    _signed_rem:
      call l_divs_16_16x16
      ex de,hl
      ret
  Caller consumes both i16 results from DE.

Current behavior:
  Runtime for -30000 and 7: quotient=-5, remainder=-4285.
  Runtime for unsigned 50000 and 7: quotient=6, remainder=7142.
  LCG probe WITHOUT qsort: first=48,27,55,46,6 min=0 max=65.

Expected behavior:
  C truncates signed division toward zero: quotient=-4285, remainder=-5.
  Unsigned quotient=7142, remainder=6.
  Independent Python arithmetic, wrapping each LCG step to 16 bits:
    first=846,775,876,765,26 min=5 max=991 sum=93884.

Root cause:
  llvm/lib/Target/Z80/Z80InstructionSelector.cpp:333-337 maps quotient to DE
  and remainder to HL, but the z88dk core returns HL=quotient, DE=remainder.
  The fused selection at :3983-4005 repeats the reversed mapping.
  z88dk/libsrc/math/integer/l_divs_16_16x16.asm:9-10 documents the real ABI;
  the linked map resolves it to l_small_divs_16_16x16 with the same contract.

Evidence it's wrong (not just suboptimal):
  Both signed and unsigned runtime results are exchanged exactly.
  The isolated LCG computes quotients (0..65) where it requires modulo (0..999).
  The original qsort assembly calls l_divu_16_16x16 in this data-generation loop.

Contaminated:
  - Local fork, Z88DK target; default ELF tests do not exercise this ABI.
  - Current i16 lit checks assert symbol names, not correct result registers.
  - This does not prove every qsort input/callback shape correct.
  - No AVR comparison is applicable to a Z88DK register-contract mismatch.

Doubt:
  Additional clobber/liveness defects are not ruled out by these small probes.

Verdict:
  REAL-BUG in i16 mapping; the reported qsort min/max failure is data generation,
  not evidence of a comparator calling-convention failure.

Recommended posture:
  HOLD AS FORK KNOWLEDGE.

Rules-checked: feedback_file_bugs_not_fixes, feedback_verdict_after_real_pass_output,
  feedback_minimal_repro_before_source_dive, feedback_state_certainty

=== Bug analysis: i32 integration absent from active branch ===

Smallest repro:
  scratch/tmp/z88dk-fails-20261003/div32.ll
  define i32 @q(i32 %a, i32 %b) { %v = sdiv i32 %a, %b; ret i32 %v }

Pass output (what the named pass actually produces):
  llc -mtriple=z80-unknown-none-z88dk:
    push hl ... (four argument words)
    call __divsi3
  The remainder function calls __modsi3.

Current behavior:
  runtime_long fails to link on __divsi3/__modsi3/__udivsi3/__umodsi3.
  runtime_intdiv also fails on __divmodsi4/__udivmodsi4.

Expected behavior:
  The selected runtime entry points must resolve in the existing linked runtime;
  a successful llc-only symbol check does not establish library integration.

Root cause:
  llvm/lib/Target/Z80/Z80LegalizerInfo.cpp:948-960 and :981-987 explicitly
  choose these names with SDCCCall0.
  The direct EXX implementations exist as 93f1fe8db779 and 3fc7ad58d9e0
  on z88dk-native-libcalls; neither is an ancestor of the active HEAD.
  llvm/test/CodeGen/Z80/z88dk-i32-libcalls.ll deliberately expects the
  unresolved names on the active branch.

Evidence it's wrong (not just suboptimal):
  Actual generated assembly and both link logs agree on unresolved symbols.
  The active branch lacks the direct mappings that the old plan claimed present.

Contaminated:
  - Dirty local worktrees were preserved; no branch switching or merge occurred.
  - The old implementation was inspected, not built or validated here.
  - Existing i32 implementations have frame/IX assumptions; their mere existence
    is not evidence they are safe for this branch's dynamic-frame programs.
  - No AVR comparison is applicable to missing Z88DK runtime integration.

Doubt:
  Integrating existing work may expose additional frame/register problems.

Verdict:
  REAL-BUG / incomplete integration on the active branch.

Recommended posture:
  HOLD AS FORK KNOWLEDGE.

Rules-checked: feedback_file_bugs_not_fixes, feedback_verdict_after_real_pass_output,
  feedback_revalidate_historical_compiler_claims
```

**Qsort callback cross-check (user's specific concern):** The actual generated
`cmp_asc`/`cmp_desc` functions read the left operand at SP+2 and right at SP+4,
and return in HL. `z88dk/libsrc/classic/stdlib/_qsort.asm:69-95`
(`l_cmp_sdcc`) pushes right then left and consumes the HL result; these
contracts match. `qsort-fixed.c` bypasses the LCG completely, uses fixed
`{9,1,7,3,5}` data and the same annotated comparator bodies, and prints
`asc=1,3,5,7,9 desc=9,7,5,3,1`. No wrapper was introduced. This is positive
evidence for this callback shape, not a universal qsort ABI certification.

**Tilføjet til den permanente testcase efter brugerens anmodning:**
`z88dk/test/clang/runtime_qsort.c` sorterer nu også to faste arrays med
`{7,-3,1,7,0}` gennem de samme `__z88dk_callback`-comparatorer. Scriptet
kræver den præcise linje
`callback asc=-3,0,1,7,7 desc=7,7,1,0,-3` før den oprindelige LCG-check.
Dette dækker negative værdier, dubletter og begge operandretninger uden
integer-div/rem-afhængighed.

Den nye assertion blev først observeret rød før C-fixturen blev udvidet.
Efter tilføjelsen passerer callback-checken, mens den uændrede LCG-check
stadig fejler med `qsort 200 0 65 OK`. Negative kontroller i scratch:
fjernelse af callback-attributten afvises med `incompatible function pointer
types`; byttede operandsemantikker i ascending-comparatoren giver descending
data og afvises af den nye præcise output-check. Ingen af disse mutationer
er anvendt på den permanente testcase.

**Benchmark diagnosis:** A workspace-local reproduction of their compiler
flags fails with `Unknown command line argument '-z80-float-sdcccall0'`.
Both scripts also explicitly pass removed `__addsf3.asm`; the timing script
additionally passes removed `__cmpsf2.asm` and `__floatsisf.asm`. A separate
attempt to pass `__addsf3.asm` reports `No such file or directory`. Thus the
earlier shorthand "missing bridges" was incomplete: the removed flag is the
first blocker. Timing/size measurements have not run successfully, and no
performance verdict is inferred from these failures.

**Oracle gap:** The active lit suite stays green because i16 tests pin names
without asserting result-register semantics, while i32 tests explicitly pin
helpers that do not link. The default-target runtime suite is not a correctness
oracle for Z88DK-specific lowering. These 13 failures must not be converted to
XFAIL or made green by changing expected values.

Regler: [[feedback_file_bugs_not_fixes]] og
[[feedback_verdict_after_real_pass_output]].

## 9. Implementation efter brugerens "start" (2026-10-03)

De fem aftalte trin er implementeret lokalt uden nye bridges/wrappere:

1. i16 separate/fused quotient hentes fra HL, remainder fra DE. De nye
   lit- og runtime-assertions var observeret røde før rettelsen og består nu;
   `runtime_i16_divrem` dækker O0/O2/O3/Oz og negative/boundary-værdier.
   Qsorts nye callback-check og oprindelige LCG-check består begge.
2. i32 kalder eksisterende `l_mulu_32_32x32`/`l_div[su]_32_32x32`
   med EXX og PUSH/POP IX omkring kaldet. EXX har eksplicitte register-use/defs,
   så dividend-opsætningen ikke slettes som død kode. Både default small-kerner
   og eksisterende fast-kerner består `runtime_i32_frames` ved O0/O2/O3/Oz;
   fast-kernerne vælges med z88dk's eksisterende symbol-redirect, ikke nye
   adaptere. Testen tvinger frame-pointer og verificerer levende stacklocals
   og stack-passed output-pointere efter kaldet.
3. Frontend bevarer eksplicit header-CC, når en implicit library-builtin
   deklareres på Z88DK-triplen. Ingen blanket `-fno-builtin`. O0/O2-IR-testen,
   default-target-kontrollen og builtin `strlen("abcd")`-foldingen består.
   Testen er tilføjet både test-runnerens lit-paths og CI-workflowen.
4. Timing- og size-benchmarks bruger eksisterende math32-indgange på
   Z88DK-triplen og default-target compiler-rt som separat sammenligning.
   Retired flag og slettede wrapper-inputs er fjernet; begge benchmarks består.
5. Samlet endelig launcher-verifikation kører; se nedenfor for målt mellemstatus.

Den uændrede Oz integer-oracle afslørede også en resterende i8-wrapper-sti:
den kalder nu eksisterende `l_fast_divu_8_8x8` med L/E og læser L/E som
quotient/remainder. Ny lit-test var rød før rettelsen; integer-runtime består
nu inklusive den oprindelige native i8 symbol-assertion.

**Mellemstatus:** default-target runtime 426 PASS/0 FAIL/6 SKIP, lit 151
PASS/0 FAIL. Z88DK suite 68 PASS/1 FAIL/1 XFAIL, hvor alle 13 oprindelige
fejl er væk og den eneste FAIL er survey-watchdoggen. NaN er fortsat uden
for runtime-testkontrakten; faktisk NaN-semantik er ikke verificeret.

**Watchdog-fejl, målt:** `stdlib_coverage.sh` består alene med 81 LINK og
10 LINK_ERROR på 62,3s. Det er en survey, ikke et correctness-gate; dens
LINK_ERROR-resultater må ikke omtales som runtime-PASS. Runneren tvang 60s
uanset `TEST_TIMEOUT` og rapporterede fejlagtigt den generelle 25/120s-grænse.
Surveyen får nu mindst 120s, større overrides respekteres, og fejlmeldingen
viser det faktiske budget. Ingen oracle, compilerflag, SKIP eller XFAIL er
ændret for at skjule en failure.

Ingen commit, push, merge, PR eller issue er foretaget.

**Endelig standalone-verifikation:** `run-llvmz80-tests.sh` exit 0:
432 runtime-tests (426 PASS, 0 FAIL, 0 Fatal, 6 SKIP) og 151 lit PASS,
0 FAIL. Log: `scratch/tmp/z88dk-fails-20261003/full-llvm-final.log`.
**Endelig Z88DK-verifikation:** `run-z88dk-tests.sh` exit 0 med
standardindstillinger (`TEST_TIMEOUT=25`): 69 PASS, 0 FAIL, 0 SKIP,
1 XFAIL (`tmpfile`, eksisterende classic CP/M-gap). Surveyen afsluttede på
65s; alle 13 oprindelige FAILs er væk, og de to nye integer-runtime-tests
indgår i suite-resultatet. Log:
`scratch/tmp/z88dk-fails-20261003/full-z88dk-final.log`.

**Commit-review efter brugerens anmodning:** compiler-diff gennemgået for
resultatregistre, EXX-liveness, IX-frame-preservation og frontendens
target-afgrænsning. CALL_nn har allerede implicitte clobbers for A, BC, DE,
HL, IY og FLAGS; i32-kald tilføjer IX. Ingen yderligere bevist korrekthedsfejl
fundet i gennemgangen. Det er ikke en garanti for alle ABI/inputformer:
NaN er fortsat uverificeret, SM83 er ikke dækket af native Z80-kerner, og
link-surveyens 10 LINK_ERROR er ikke runtime-correctness-resultater.
Lokale commits forberedes i compiler-, runtime- og workspace-repo; ingen push.

## Risici

- **Ac-regclass-konflikt**: `selectRuntimeLibCall16` i `Z80InstructionSelector.cpp` er signifikant ændret. Resultat-reg-udtræk skal bruge `Ac`-klassen, ikke `AReg`.
- **Mach-O `_`-præfiks-fælden**: brug ALTID `MCContext::getOrCreateSymbol(StringRef)` direkte.
- **`__memcpy_chk`-sporet**: kan omgå `getOrInsertLibFunc` — verificer via lit at kæden `memcpy_chk → memcpy → memmove` ender med `Z80_SmallC` CC.

## Nøglefiler

- `llvm/lib/Target/Z80/Z80LegalizerInfo.cpp` — f32 libcall CC + memmove/memset lowering
- `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` — i8/i16 libcall-mapping
- `llvm/lib/Target/Z80/Z80AsmPrinter.cpp` — MCInstLower f32-remapping + C_LINE
- `llvm/lib/Target/Z80/MCTargetDesc/Z80MCAsmInfo.cpp` — `isZ88DK()`
- `llvm/lib/Transforms/Utils/BuildLibCalls.cpp` — classic-libc CC stamping
- `clang/lib/Basic/Targets/Z80.cpp` — CC-kompatibilitetscheck, double=32bit
- `clang/lib/Driver/ToolChains/Z80.cpp` — driver toolchain
