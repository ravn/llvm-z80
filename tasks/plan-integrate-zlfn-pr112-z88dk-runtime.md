# Plan: adopt zlfn's PR #112 branch AS the new ravn/main baseline

**Status:** 2026-10-11 analyse. Udarbejdet til udførelse af senere agent
(Sonnet). Branch-navn: `work/zlfn-z88dk-runtime-integration-20261011`.

**Bruger-instruks 2026-10-11:**
> "det her bliver den nye main og ny baseline at starte forfra på"

Dette er ikke en "integrer et PR i ravn/main"-opgave. Det er en
**baseline-replacement**: `upstream/users/zlfn/runtime-4-z88dk`
bliver den nye ravn/main. Alt ravn-specifikt arbejde der stadig
er relevant genlægges som patches oven på denne nye baseline.
Alt der var midlertidige shortcuts omkring det zlfn nu dækker,
droppes.

## Baggrund

Upstream `llvm-z80/llvm-z80#112` ("Call z88dk's runtime in the z88dk
environment", zlfn, 942+/-151 LOC, open, requested-reviewer @ravn) er
en substantiel refactoring af hvordan Z80-backend håndterer runtime-calls
i z88dk-miljøet. PR'en bygger på en serie `users/zlfn/runtime-N-...`
branches der tilsammen omorganiserer hele runtime-infrastrukturen.

**Base-branch** (`users/zlfn/runtime-3-z88dk-env`) er allerede del af
PR-stakken; **head-branch** (`users/zlfn/runtime-4-z88dk`) er dette PR's
tip. Lokalt fetched 2026-10-11 som
`upstream/users/zlfn/runtime-4-z88dk` i ravn/llvm-z80.

## Hvorfor den er interessant for os

PR'en adresserer **direkte** vores issue #409 (memcpy/memset IX+d
prolog) via en principiel vej: routing af runtime-calls gennem LLVM's
`RuntimeLibcalls.td`-infrastruktur + nye `CallingConv`-værdier for
z88dk's ABI'er. Og den fjerner ravn's "minimal viable product"-tilgang
(ravn/llvm-z80#396 cdf7fbabe800) som brugte hardcoded symbol-navne +
inline target-checks.

Hvis integrationen lykkes:
- Z80 backend får libcall-infrastruktur-opdeling der matcher andre
  backends (ARM, x86) — bedre fundament for upstream-filing senere
- `--target=z80-unknown-none-z88dk` lowering bliver konsistent (ikke
  spredt over inline-triple-checks i LegalizerInfo)
- #409 lever B (triple-skift giver register-ABI memset) kan evalueres
  igen med PR'en aktiv — måske autoload-regressionen 2208 B forsvinder

## Hvad PR'en gør konkret

Core-filer og linjer:

| Fil | +/- | Hvad |
|---|---|---|
| `llvm/include/llvm/IR/CallingConv.h` | +6/0 | Nye `CallingConv`-værdier for z88dk-ABI'er (SDCCCall0 varianter, __smallc, osv.) |
| `llvm/include/llvm/IR/RuntimeLibcalls.td` | +105/-1 | Ny libcall-tabel med z88dk's `l_mulu_*`, `l_f32_*`, `fcmp3`-varianter |
| `llvm/include/llvm/IR/RuntimeLibcallsImpl.td` | +5/0 | Impl-mapping |
| `Z80CallLowering.cpp` | +54/-9 | Håndterer flere CC's; alle CC's resolves eksplicit |
| `Z80InstrInfo.cpp` | +114/-5 | Nye pseudos (fused divmod, EXX-brug til 32-bit mul-paths) |
| `Z80InstrInfo.td` | +20/0 | Pseudo-definitioner |
| `Z80InstructionSelector.cpp` | +20/-70 | **-50 netto**: forenkling fordi libcall-tabel overtager |
| `Z80LegalizerInfo.cpp` | +20/-8 | FCmp lowering + libcall-opslag via `hasLibcall()` |
| `Z80AsmPrinter.cpp` | +44/0 | z88dk assembly er default-output (ikke sdas) |
| `MCTargetDesc/Z80MCTargetDesc.cpp` | +15/-7 | Streamer-valg per environment |
| `CMakeLists.txt` | +41/-3 | Byg `z80_z88dk_rt.lib` (compiler-rt bits uden C-library) |

Nye tests:
- `llvm/test/CodeGen/Z80/z80-asm-format-z88dk.ll` +5 (modifikation)
- `llvm/test/CodeGen/Z80/z88dk-runtime.ll` +160 (nyt file)
- Fire eksisterende runtime-tests markeret `-z88dk`-skip pga. kendte
  z88dk-bugs (comment i PR body)

compiler-rt stub-opdateringer:
- `sm83/divmodhi4.asm`, `sm83/modhi3.asm`, `sm83/udivhi3.asm`,
  `sm83/umodhi3.asm` — fused divmod returnerer remainder i DE
- Nye `z80/divhi3.asm`, `z80/divmodhi4.asm`, `z80/divmodsi3.asm`,
  `z80/divmodsi4.asm`, `z80/floatxisf.asm`, `z80/modhi3.asm`,
  `z80/muldi3.asm`, `z80/mulsi3.asm`, `z80/udivmodsi4.asm`,
  `z80/umodhi3.asm` — stubs der kalder z88dk's cores med riktige
  conventions

test-runner-opdateringer:
- `z80-utils/test-runner/harness/z80/crt0_z88dk.asm` +52 (ny crt0)
- `z80-utils/test-runner/src/config.rs` +34, `main.rs` +7/-4,
  `run_all.rs` +9/-7 — tilføjer `-z88dk` flag
- `src/suites/clang.rs` +113/-18 — bygger clang-tests under z88dk
- 4 testcases med `-z88dk-skip`-annotation

Kendte begrænsninger (fra PR body):
- z88dk 2.4's `l_divs_16_16x16` giver forkert fortegn på rest for
  negativ dividend
- math32 håndterer ikke infinity/NaN/subnormals
- z88dk's `memcmp`/`strcmp` sammenligner bytes som signed
- `truncf` + nogle libm-funktioner mangler linkning
- Inline assembly i sdas-syntax assemblerer ikke under
  `z88dk-z80asm`

## Konflikter med ravn/main

ravn/main har sin egen z88dk-integration (`#396`, commit
`cdf7fbabe800`) der rør samme filer:

| Fil | Ravn-side (`#396`) | zlfn-side (`#112`) |
|---|---|---|
| `clang/lib/Basic/Targets/Z80.cpp` | +17 (float-bredde, macros) | — |
| `llvm/include/llvm/TargetParser/Triple.h` | +3 (Z88DK enum-værdi) | — |
| `Z80LegalizerInfo.cpp` | asm_memset-register-ABI inline + z88dk macros | libcall-tabel + FCmp-lowering |
| `Z80InstructionSelector.cpp` | Z88DK-mappings inline (+88) | Fjerner hardcoded-paths (-70) |
| `Z80AsmPrinter.cpp` | C_LINE for z88dk (+59) | sdas default output ændret (+44) |
| `MCTargetDesc/Z80MCAsmInfo.cpp` | isZ88DK-flag (+3) | — |
| `CallingConv.h` | — | +6 nye CC-værdier |
| `RuntimeLibcalls.td` | — | +105 LOC |

Konflikt-centrum: `Z80LegalizerInfo.cpp` og
`Z80InstructionSelector.cpp` — begge forgreninger har omskrevet dem.
Zlfn's tilgang er den rene; ravn's er pragmatisk shortcut. Begge
løser dele af det samme problem.

**Vigtigt om intellektuelt ejerskab:** zlfn har bedt om at
contributors forklarer deres bidrag ([[feedback_explain_before_filing]]
2026-06-05); vi må være forsigtige med at mærke "integrere PR" som
"vores arbejde". Integration her er ikke en upstream-filing — det er
en intern merge af upstream arbejde ind i ravn/-forken.

## Plan

### Fase 0 — Setup (10 min)
1. Opret branch `work/zlfn-z88dk-runtime-integration-20261011` fra
   ravn/main.
2. Verificer `upstream/users/zlfn/runtime-4-z88dk` er fetched
   (allerede gjort 2026-10-11).
3. `git log --oneline origin/main..upstream/users/zlfn/runtime-4-z88dk
   | wc -l` giver ~361 commits (361 fordi PR'en bygger på hele
   `runtime-1/2/3`-stakken, ikke bare `runtime-4` alene).

### Fase 1 — Merge-strategi (dikteret af regel, ikke valg)

**Durable regel [[feedback_ravn_patches_rebase_onto_zlfn]] (user
2026-10-11):** ravn's arbejde lægges ALTID oven på zlfn's
beslutninger. Ikke merge-equals, ikke parallel-implementation.

**Konsekvens for denne integration:**

Zlfn's `users/zlfn/runtime-4-z88dk` er base-truth. Ravn/#396's
"minimal viable product" z88dk-integration (`cdf7fba`) var en
interrim-shortcut; den omplaceres eller droppes hvor zlfn's
infrastruktur erstatter den.

**Procedure:**

1. Start fra `upstream/users/zlfn/runtime-4-z88dk` (ikke ravn/main).
2. Identificer ravn/main-commits der er *unikke* (ikke dækket af
   zlfn's PR):
   - #410 C_LINE guard-fiks (`d081b0d`) — unik, cherry-pick
   - TTI LSR-hooks (`8d909ab`) — unik, cherry-pick
   - test_26_fixed_point.s og lign. unikke tests — cherry-pick
   - Alt der overlapper med zlfn's design (fx #396
     `cdf7fba`-inline-paths i Z80LegalizerInfo.cpp) — DROP, lad
     zlfn's version stå alene.
3. Cherry-pick de unikke commits oven på zlfn's branch.
4. Resolve små conflicts i cherry-pick ved at TILPASSE ravn's patch
   til zlfn's nye API-shape (ikke vice versa).

**Verb-check:** "cherry-pick ravn's patch ONTO zlfn's branch". Ikke
"merge zlfn's branch into ravn's main". Hvis retningen føles
omvendt, er den forkert.

### Fase 2 — Baseline replacement (60-120 min)

```bash
# Branch'en ER zlfn's branch; vi lægger ravn-patches oven på.
git checkout -b work/zlfn-z88dk-runtime-integration-20261011 \
    upstream/users/zlfn/runtime-4-z88dk

# Cherry-pick ravn's unikke commits oven på
git cherry-pick d081b0d       # #410 C_LINE guard
git cherry-pick 8d909ab       # TTI LSR hooks
# ... osv. for hver unik ravn-patch
```

Forventet conflict-overflade (filer der både ravn/#396 og
zlfn/#112 ændrer):

- `llvm/lib/Target/Z80/Z80LegalizerInfo.cpp` — stor konflikt
- `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` — stor
- `llvm/lib/Target/Z80/Z80AsmPrinter.cpp` — mellem
- `clang/lib/Basic/Targets/Z80.cpp` — mellem
- `llvm/include/llvm/TargetParser/Triple.h` — lille
- `llvm/lib/Target/Z80/MCTargetDesc/Z80MCTargetDesc.cpp` — lille

**Konflikt-opløsningsregler:**

1. **Behold zlfn's infrastruktur** (RuntimeLibcalls.td,
   CallingConv.h, libcall-tabel-opslag i LegalizerInfo). Det er
   PR'ens hovedbidrag.
2. **Behold ravn's specifikke lowerings** hvor de IKKE overlapper med
   zlfn's rute:
   - `clang/lib/Basic/Targets/Z80.cpp`: double-bredde + macros under
     Z88DK (fra #396). Zlfn har også float/double-bredde ændringer
     (`[Z80] Make double as wide as float by default` commit
     6159aaf4eb6c) — hvis identisk, OK. Ellers behold begge.
   - `clang/lib/Sema/SemaDecl.cpp`: ravn-specifik +12 (fra #396) —
     behold hvis zlfn ikke rør samme område.
3. **Behold ravn's test-runner-tilføjelser** (test_26, test_43-45
   skipped). Zlfn tilføjer `-z88dk` config men ikke de samme
   skip-annotations.
4. **Vælg zlfn's `Z80AsmPrinter`-ændringer** omkring default-output,
   men behold ravn's C_LINE-fiks fra #410 (d081b0d). zlfn har ikke
   #410-fiks.
5. **Opløs `MCAsmInfo.cpp` isZ88DK-flag**: ravn tilføjer +3; zlfn
   bruger det måske andre steder — behold ravn's definition.

Commit-besked for merge-commit:
```
Merge upstream/users/zlfn/runtime-4-z88dk — z88dk runtime infrastructure

Integrates zlfn's refactoring of Z80 runtime-call handling (upstream
llvm-z80/llvm-z80#112). Replaces ravn/#396's inline target-checks and
hardcoded symbol names with libcall-table routing + new CallingConv
values. Preserves ravn-specific lowerings (asm_memset register ABI,
C_LINE guard from #410, test-runner skip annotations) where they do
not overlap zlfn's infrastructure.
```

### Fase 3 — Build + lit (30 min)

```bash
make toolchain                              # fuld rebuild af alle værktøjer
./build-macos-asserts/bin/llvm-lit llvm/test/CodeGen/Z80/
```

**Succes-kriterie:** 156+ lit tests PASS (ravn/main har 156;
PR tilføjer `z88dk-runtime.ll` +160 LOC = sandsynligvis 2-5
nye tests → forvent 158-161 PASS).

**Hvis tests fejler:**
- Lit-failure i `z80-asm-format-z88dk.ll` → sandsynligvis PR'ens
  forventede diff, check expected output.
- Lit-failure i ravn-specifikke tests
  (fx `cp-hl-pointer-loop.ll`) → merge-konflikt-opløsning
  utilstrækkelig; undersøg konkret.
- Build-fejl → libcall-tabel-reference mangler → tjek om
  `RuntimeLibcallsImpl.td` kom med.

### Fase 4 — Runtime-tests (30 min)

```bash
cd z80-utils && BUILD_DIR=../build-macos-asserts cargo run --release -- clang
BUILD_DIR=../build-macos-asserts cargo run --release -- clang -z88dk  # ny config
```

**Succes-kriterie:** 438+ PASS på default-config (ravn's baseline).
Zlfn's PR tilføjer `-z88dk`-config der kører delmængde af tests under
z88dk-triple — forventet 50-100 tests der er compatible.

Kendte XFAIL (fra PR body):
- Division med negativ dividend (z88dk 2.4 bug)
- Float inf/NaN/subnormals (math32 limitation)
- `test_43_f32_inf_nan.c`, `test_44_f32_zero_subnorm.c`,
  `test_45_f32_round_precision.c`, `test_02_i16_signed.c`

### Fase 5 — Verificer autoload-regression (30 min)

Dette er det kritiske test af om zlfn's PR ændrer vores z88dk-triple-
regression (vi målte 2208 B = +77 B vs z80-triple 2131 B).

```bash
cd rc700-gensmedet/autoload-in-c
# Scenarie A: ravn/main (baseline, nu after integration)
make clean && make -s prom COMPILER=clang
# forventet: 2131 B (uændret, triple stadig --target=z80)

# Scenarie B: skift triple med PR'ens infrastruktur
sed -i.bak 's|--target=z80 |--target=z80-unknown-none-z88dk |' Makefile
make clean && make -s prom COMPILER=clang
mv Makefile.bak Makefile
# forventet: <2208 B (nede fra regression), ideelt ≤2131 B

make mame   # verificer boot stadig OK (A> prompt frame 175)
```

**Succes-kriterie Scenario A:** 2131 B uændret. Hvis større, har
integration brudt noget.

**Succes-kriterie Scenario B:** <2208 B (bedre end tidligere), og
MAME boot PASS. Hvis bedre end 2131 B, kan triple-skift blive den
foretrukne vej for autoload. Hvis dårligere, PR'en forbedrer ikke
dette specifikke use-case — stadig værdifuld for andre komponenter.

### Fase 6 — Beslutningspunkt

Rapporter til bruger:
- Lit PASS/FAIL med antal
- Runtime default + -z88dk PASS/FAIL med antal
- Autoload Scenario A og B tal + MAME boot-status
- Konflikt-opløsningens kompleksitet (kommentarer eller problemer)

Bruger beslutter:
1. **Merge til ravn/main?** (direkte eller via PR til review)
2. **Behold som test-branch til senere?** (hvis regression eller
   usikre resultater)
3. **Rul tilbage og vent på upstream merge?** (zlfn's PR er stadig
   open i upstream; der kan være review-feedback der ændrer form)

### Fase 7 — Dokumentation (ved godkendelse)

Hvis bruger godkender merge:
1. Opdater `tasks/memory/reference_z88dk_triple_tradeoffs.md` med nye
   tal (triple-skift effekt efter PR).
2. Opdater `MEMORY.md` autoload-sektion i CLAUDE.md hvis størrelser
   skifter.
3. Luk eller opdater issue ravn/llvm-z80#409 hvis PR'en adresserer
   det (kommentar på issue med PR-reference).
4. Push `work/zlfn-z88dk-runtime-integration-20261011` til ravn
   origin.
5. Ingen upstream-posting (reglen
   [[feedback_ravn_open_upstream_never]]).

## Risici og mitigation

| Risiko | Sandsynlighed | Mitigation |
|---|---|---|
| Merge-conflicts for komplekse til at opløse rent | Høj | Fase 1 valg C (rebase ravn-patches oven på zlfn's stak) |
| PR introducerer regressioner i ravn's eksisterende tests | Mellem | Fase 3/4 kører fulde suiter; afbryd og rul tilbage ved rød |
| Autoload z88dk-triple-regression forbliver | Mellem | Status quo (`--target=z80` + ravn's hooks); PR'en er stadig værdifuld infrastruktur |
| Upstream PR'en ændrer form før merge (zlfn re-pusher) | Lav-mellem | Kort session-ad gang; bruger beslutter om vi venter |
| Vi mister ravn's #410 C_LINE-fiks under merge | Lav | Pin som specifik check i Fase 2 regel 4 |
| Zlfn's PR har bugs der først dukker op post-merge | Lav-mellem | 361 commits — bred testning i Fase 3-5; XFAIL-liste i PR body er allerede kendt |

## Non-mål

- **Ingen upstream-posting.** Hverken til llvm-z80/llvm-z80#112
  eller videre til llvm/llvm-project.
- **Ingen ravn/#396 revert.** Zlfn's PR erstatter nogle paths; de
  rester vi behøver (asm_memset-ABI osv.) beholdes som patches
  oven på.
- **Ingen z88dk-source-ændringer.** z88dk's egne `.asm`-rutiner er
  stabile pins (`z88dk:2.4` Docker image). Kun ravn/llvm-z80 og
  rc700-gensmedet rør.
- **Ikke klar til at erstatte `--target=z80` som default.** z80-triple
  forbliver autoload's primære vej uanset udfald. Triple-switch er en
  option for komponenter der vinder på det.

## Næste agent (Sonnet) må forvente

- 2-5 timer total udførelse
- Bruger vil være interaktiv under Fase 2 (konflikt-opløsning) hvis
  der er store konflikter
- Bruger vil være interaktiv ved Fase 6 beslutningspunkt
- Alle andre faser kan køres autonomt med rapporter mellem

## Relateret

- Upstream PR: https://github.com/llvm-z80/llvm-z80/pull/112
- Ravn's #396 (konkurrerende tilgang):
  `git show cdf7fbabe8000fa55f056e64ceb42b1e6f0d868f`
- Issue ravn/llvm-z80#409 (memset/memcpy IX+d) — PR'en adresserer
  dette via infrastruktur-ændring
- Memory: `tasks/memory/reference_z88dk_triple_tradeoffs.md`
- Memory: `tasks/memory/reference_lsr_filter_prefers_wide_stride_match.md`
