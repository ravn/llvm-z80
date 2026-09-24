# Session 2026-09-24/25: z88dk + llvm-z80 post-PR#40 breakthrough — baseline established

**Kontekst:** fortsættelse af `tasks/plan-pr40-fallout-recovery-2026-09-10.md` /
`analysis-autoload-over-2kb-after-pr40-2026-09-16.md`. Mål: få `zcc
-compiler=llvmz80` til reelt at virke mod frisk `llvm-z80` main, og finde ud
af hvor meget af PR #40-fallout der reelt er tilbage. Se også workspace
`tasks/todo.md` "Plan: z88dk + llvm-z80 — bryde igennem post-PR#40" for den
fulde Trin 0-6-plan denne session gennemførte Trin 0-1 og en stor del af
Trin 3 af.

## Trin 0 — Frisk build (DONE)

Native Linux build, `llvm-z80/build-linux/` (ny — undgår macOS-specifik
`osx_xcrun`-crash i `lit.cfg.py` som den gamle `build/` (fra 29/6, FJERNET
denne session) ramte). `clang version 24.0.0git` på `origin/main` (`1f0e709`).

**ccache tilføjet til `clang/cmake/caches/Z80.cmake`** (no-op hvis ikke
installeret) — muliggør billige parallelle `build-<navn>`-træer på tværs af
grene/PR'er fremover (jf. `plan-llvm-z80-upstream-pr-series-2026-09-06.md`s
"Build-tree isolation"-konvention).

**Ny `git worktree`**: `../llvm-z80-worktrees/upstream-main/` på ren
`upstream/main` (llvm-z80/llvm-z80, `b904185`) — separat build
`build-upstream-main`, ~65-70% færdig da sessionen sluttede (IKKE fuldført,
fortsæt her). Fund undervejs: `origin/main` (jeres fork) indeholder **100%**
af `upstream/main` (merge-base == upstream/mains spids, 0 commits mangler,
1177 fork-only commits ovenpå) — I er ikke bagud upstream noget sted.

## Trin 1 — Lit-baseline (DONE, bedre end forventet)

Med komplet test-værktøjssæt (skal bygges separat — `FileCheck not count
llvm-mc llvm-objdump llvm-readelf llvm-config` er IKKE del af
`clang llc lld`-target-listen):

**284 tests: 278 PASS + 5 XPASS + 1 XFAIL. 0 FAIL.**

5 XFAIL-markeringer er forældede (testene består nu faktisk) — IKKE fjernet
endnu, kun identificeret:
- `index-iv-static-frame.ll`
- `issue-217-pattern-fill-dedicated-exits.ll`
- `issue-214-opt-mtriple-datalayout.ll`
- `issue-57-z80cc-libcall-simplify.ll`
- `issue-57-classic-libc-cc.ll`

Kun `issue-216-cp-sbc-and.ll` er reelt stadig XFAIL.

**Konklusion:** PR #40-fallout-recovery (R1-R5, Class 1/2) er reelt landet
korrekt på compiler-siden. CLAUDE.md's "164 PASS + 6 XFAIL"-baseline er
STÆRKT forældet (til det bedre) — bør opdateres når Trin 4/5 er kørt.

## Trin 3 (delvist) — z88dk end-to-end-verifikation

### Stale-binary-fælde (systemisk, ramte flere gange)
`z88dk/bin/*` var delvist fra 28. juni (før al zcc-ABI-recovery-arbejdet,
8/9-9/9). `make`s afhængighedstjek fejlvurderede flere binaries (`zcc`,
`z88dk-ticks`, `z88dk-appmake`, `z88dk-copt`, `z88dk-lib`, `z88dk-sccz80`,
`z88dk-ucpp`, `z88dk-z80asm`, `z88dk-z80nm`, `z88dk-zobjcopy`,
`z88dk-zpragma`) som "up to date" selvom kildekoden var nyere. Rettet ved
eksplicit `rm` + `make bin/<navn>` per binary. **Hvis nogen støder på
mystiske "Unknown compiler type: llvmz80" eller lignende — tjek FØRST om
`bin/zcc` er nyere end `src/zcc/zcc.c` (`find src/zcc -newer bin/zcc`).**
`z88dk-appmake` krævede desuden `libgmp-dev` (TI-8x-calculator-support).

### Case-sensitivity-bug i test/clang-suiten (kritisk for Linux-brug)
`zcc ... -create-app` navngiver CP/M-output med **store bogstaver**
(`RT.COM`, `T.COM`, `ORACLE.COM`, `RT_O2.COM` osv. — hele filnavnet, ikke
kun extension). 63 af 66 filer i `z88dk/test/clang/*.sh` tjekker for
**små bogstaver** (`rt.com` osv.) — virker på macOS (case-insensitive
filsystem), fejler næsten alt på Linux (case-sensitive ext4).

**Fix anvendt (COMMITTED denne session, se z88dk-commit):**
`sed -i 's/rt\.com/RT.COM/g; s/rt\.ihx/RT.IHX/g; s/oracle\.com/ORACLE.COM/g'
test/clang/*.sh` — 47 filer. Dette dækker de mest almindelige mønstre, men
**IKKE alle** (fx `runtime_intdiv.sh`s `$WORK/rt_$OPT.com` blev IKKE ramt af
denne sed — verificeret manuelt at det compilerer/kører korrekt alligevel,
men selve scriptet blev ikke rettet for det mønster). En fuldstændig,
robust fix ville være case-insensitiv fil-matching i scripts (`shopt -s
nocaseglob` eller `find -iname`) i stedet for literal sed-erstatning — ikke
gjort her, kun det mest almindelige mønster.

### Endeligt, race-frit facit: 48 PASS / 15 FAIL / 2 SKIP / 1 XFAIL (af 66)

De 15 resterende fejl blev alle undersøgt manuelt (bruger bad om KUN
llvm-z80 backend-fejl, ikke z88dk/runtime-fejl medmindre miscompile):

**INGEN llvm-z80 backend-bugs fundet.** Alle 15 er z88dk-side eller
test-infra:

| Test | Root cause | Kategori |
|---|---|---|
| `issue20/22/52_*_abi`, `nontrivial_demo`, `runtime_intdiv -O2`, `runtime_rodata_cstn` (alle -O-niveauer) | Samme case-sensitivity-bug i endnu et navnemønster jeg ikke sed'ede | **FALSK** — kompilerer og kører korrekt ved manuel verifikation |
| `issue23_fcntl_write` | write() returnerer stadig forkert | Kendt z88dk-gap |
| `runtime_fileio_qsort`, `runtime_fileio_update`, `runtime_fileio_status` | Kendt stdio-regression, testene refererer selv til z88dk#54 | z88dk-side |
| `runtime_fileio_ferror_feof`, `runtime_libgen` | sccz80-ORACLEN selv tom ("test assumption broken") | Test-infra, ikke llvmz80 |
| `runtime_float` (32-bit float div returnerer altid 0) | **Undersøgt til bunds**: genereret assembly for `a/b` vs `a+b` er BYTE-FOR-BYTE identisk (samme calling convention, kun symbolnavn differerer); `cm32_sdcc_fsdiv.asm` vs `cm32_sdcc_fsadd.asm` strukturelt identiske. Fejlen sidder i selve `m32_fsdiv`s Newton-Raphson-reciprok-algoritme i z88dk math32 | **z88dk math32-bibliotek, IKKE backend** |
| `runtime_printf_autoformat` (`%f` → `0.000000`) | `int(a*1000)` giver korrekt `3500` (double-aritmetik OK), kun printf's dtoa fejler under klassisk clib. Matcher `reference_llvmz80_newlib_ieee_printf_fix.md`s fix, som kun dækker newlib, ikke klassisk | z88dk classic-clib printf, se separat memory-note |
| `stdlib_coverage` (timeout) | Standalone `malloc`-probe kompilerer < 1s, intet tegn på uendelig løkke. Scriptet kører dusinvis af sekventielle kompileringer og overskred simpelthen den generiske 25s-timeout, forværret af samtidig `build-upstream-main`-ninja der spiste alle 8 kerner | Test-harness-timeout, ikke compiler-hang |

## Nyt fund: `%f`-printf broken under klassisk clib

Se `tasks/memory/finding_llvmz80_classic_printf_f_broken_2026-09-25.md` i
workspace-repoet. Kort: `--math32`-aritmetik er korrekt, men
`printf("%f",...)` printer altid `0.000000` under `+cpm` uden
`-clib=newlib_iy`. Ikke undersøgt til bunds (root cause-hypotese: samme
klasse fix som #35s `-D__LLVMZ80_IEEE_PRINTF`, men for klassisk clib).

## Sammenligning: `upstream/main` alene (efterfølgende, samme session)

Efter `build-upstream-main` blev færdig (3463/3463):

**Lit-suite: 112/112 PASS (100%)** — vs 284 på fork-`main` (278 PASS + 5
XPASS + 1 XFAIL). De 172 ekstra tests på fork-siden er alle jeres egne
regressionstests (issue-*, z88dk-cc, static-frames osv.) som ikke findes
opstrøms — forventet, ikke et problem. Ren upstream er selv 100% grøn på
sit mindre testsæt.

**z88dk-testsuite mod ren upstream: 4 PASS / 59 FAIL / 2 SKIP / 1 XFAIL**
(vs 48/15/2/1 mod fork-`main`). Rodårsag identificeret entydigt:

```
clang (LLVM option parsing): Unknown command line argument '-z80-float-sdcccall0'.
clang (LLVM option parsing): Unknown command line argument '-z80-classic-libc-cc'.
```

`zcc +cpm -compiler=llvmz80` injicerer **ubetinget** disse to `-mllvm`-flag
(z88dk-siden, `src/zcc/zcc.c`) — begge er fork-only mllvm-options fra jeres
math32/ABI-arbejde, findes slet ikke i `Z80TargetMachine.cpp`/
`Z80Subtarget.cpp` på ren `upstream/main`. **Dette er arkitektonisk
tilsigtet kobling, ikke en regression** — z88dk's `-compiler=llvmz80`-sti er
skrevet specifikt til `ravn/llvm-z80`, ikke til upstream. Svaret på "kræver
z88dk noget af fork-arbejdet" er dermed et klart, verificeret **ja** — hele
zcc-integrationen er utænkelig uden mindst disse to mllvm-flags implementeret
i backend'en.

## Åbne tråde til næste session

1. **`build-upstream-main` var ikke færdig** da sessionen sluttede — fuldfør
   builden, kør samme lit-suite + z88dk-test-suite mod den, sammenlign med
   fork-`main`s resultat. Formålet var at afgøre om z88dk kræver noget af
   jeres 1177 fork-only commits, eller virker lige så godt mod ren upstream.
2. **5 forældede XFAIL-markeringer** i lit-suiten (listet ovenfor) bør
   fjernes — lille, sikker ændring, ikke gjort endnu.
3. **z88dk test/clang case-sensitivity-fix er ufuldstændig** — kun de 3
   mest almindelige navnemønstre blev sed'et. En robust fix (case-insensitiv
   matching) er ikke lavet.
4. **`runtime_float`s m32_fsdiv-bug** og **klassisk `%f`-printf-buggen** er
   begge reelle z88dk-side fund, ikke rapporteret/filet noget sted endnu —
   kandidater til `z88dk`-issues hvis/når fokus skifter dertil (bruger sagde
   eksplicit "ikke aktuelle nu").
5. **Trin 2 (CI-genoplivning)** og **Trin 4-5 (produktions-genmåling,
   CLAUDE.md-opdatering)** fra hovedplanen er slet ikke startet.
6. **Trin 6 (Docker-image z88dk+llvm-z80)** afventer stadig Trin 0/3 grønne
   — Trin 0 er nu grøn, Trin 3 er langt (48/66), men ikke 100%. Bruger sagde
   "vent" med selve Dockerfilen.

## Sidegevinster denne session (ikke llvm-z80, men noteret for kontinuitet)

- `emu2-cpm86`: merget 82 commits fra reel upstream (`johnsonjh/emu2-cpm86`,
  ikke `dmsc/emu2` som `.gitmodules` fejlagtigt pegede på før) — P_LOAD,
  FCB double-close, DOS Debug-support m.m. `.gitmodules`/pin rettet i
  workspace.
- `dcc`: samme mønster — `.gitmodules` rettet fra `davidly/dcc` (upstream)
  til `ravn/dcc` (jeres fork, som havde 17 unikke commits ikke reflekteret
  i workspace-pinnet).
- `open-watcom-v2`: fuldt build verificeret virkende (`owcc -bcpm86` →
  Mandelbrot → `emu2` kørt korrekt, alle 3 varianter O0/O2/OWIMUL).
  `OWDOSBOX`-env-var nødvendig for docs-trinnet (nu bruger har installeret
  `dosbox`).
- `ntvcm` var ikke bygget — bygget nu (`bash m.sh`). **VIGTIGT: `ntvcm`, ikke
  `emu2` (som er CP/M-86/x86), er den rigtige emulator til klassiske Z80
  CP/M `.com`-binaries fra `zcc +cpm`.**
