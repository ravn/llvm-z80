# Upstream submission triage (ravn/llvm-z80 fork → llvm-z80/llvm-z80)

Generated 2026-09-16 fra `upstream/main..HEAD` (efter merge af `dcdd3e6a17ef`).

**Mål:** få hele det færdige arbejde på fork'en ind i upstream `llvm-z80/llvm-z80` — korrekthedsrettelser, de optimeringer der allerede ER lavet, og zcc/z88dk-understøttelse. Ikke: ny density-optimering. Lit-suite skal fortsat passere på samme niveau som upstream (251 pass / 61 XFAIL / 0 fail).

## Divergens

- **1127 commits** foran upstream/main (961 non-merge + 164 merges + 2 = 1127; en tælleforskel skyldes at `dcdd3e6a17ef` er den seneste merge). 
- **Diff-fodaftryk:** 741 filer, +78 879 / -844 linjer — men langt størstedelen er `tasks/` (272 filer med noter).
- **Reelle kode-filer der er rørt** (kun rigtig kildekode):

| Område                    | Filer |
| ------------------------- | ----- |
| `llvm/lib/Target/Z80/`    | 64    |
| `compiler-rt/`            | 20    |
| `clang/lib/` + `include/` | 14    |
| `llvm/lib/Transforms/`    | 4     |
| `llvm/lib/CodeGen/`       | 3     |
| `llvm/include/`           | 3     |
| `lld/`                    | 3     |
| `clang/cmake/caches/`     | 1     |

Backend (`llvm/lib/Target/Z80/`) diff alene: **64 filer, +11 259 / -620 linjer** — inkl. flere helt nye `.cpp/.h`-passes (`Z80PatternFillRecognize`, `Z80PinAluAccumulator`, `Z80NarrowNoIndex`, `Z80SplitDjnzCounters`, `Z80KeepLoopPointerInPair`, `Z80PinLoopPointer`, `Z80LoopRotate`, `Z80LoopInstrFormPrep`, `Z80IndexIV`, `Z80SinkColdLoopIV`, `Z80RemoveJumpToNext`, `Z80ReorderTestDec`, `Z80NonReentrant`, `Z80HighByteFirstBranch`, `Z80DanglingDebugCleanup`, `Z80AutoStaticFrame`, `Z80FixupImplicitDefs`, `Z80FuseCarryChain`).

## Klassifikation (961 non-merge commits)

Rå fordeling (mekanisk match på fil-sti + subject-regex):

| Bucket                          | Antal | Sæt-op                                                                                          |
| ------------------------------- | ----: | ----------------------------------------------------------------------------------------------- |
| `NOISE` (tasks/, docs/, .md)    |   322 | **Drop**                                                                                        |
| `UNCLASSIFIED` (subject-regex greb ikke) |   272 | **Keep** — spot-check viser alle rører `llvm/lib/Target/Z80/`; er backend-optimeringer          |
| `z80-utils/*`                   |    53 | **Keep** — men *ikke* egen PR (se note nedenfor); ændringerne følger den kode-PR de tester      |
| `peephole-bss-pushpop`          |    32 | Keep                                                                                            |
| `tests-only`                    |    29 | Keep (bundles med sin sag)                                                                      |
| `generic-llvm-transforms`       |    25 | Keep — AggressiveInstCombine (delvist videreført af upstream `llvm/llvm-project`), SimplifyCFG  |
| `static-stack-bss`              |    24 | Keep                                                                                            |
| `z88dk-calling-conv`            |    20 | Keep — kritisk for zcc                                                                          |
| `peephole-djnz`                 |    18 | Keep                                                                                            |
| `block-mem-ops` (LDIR/LDDR/memset/…) |    18 | Keep                                                                                       |
| `LOCAL-CI`                      |    16 | **Drop** — hostspecifikke GitHub Actions-tweaks                                                 |
| `clang-frontend-attrs`          |    12 | Keep — `address_space(2)` port I/O, `z80_critical`, intrinsics                                  |
| `isel`                          |    11 | Keep                                                                                            |
| `ix-iy-regalloc`                |    11 | Keep                                                                                            |
| `inline-asm`                    |     9 | Keep — pair-constraint rewrite (`"hl"` → braced)                                                |
| `undocumented-insns`            |     9 | Keep (gated bag `+undocumented`)                                                                |
| `isel-addrs`                    |     8 | Keep — ptrtoint(GV+const), direkte BSS-adr.                                                     |
| `ix-remat`                      |     7 | Keep                                                                                            |
| `peephole-inc-dec`              |     7 | Keep                                                                                            |
| `tti-cost-model`                |     6 | Keep — `isLegalAddImmediate`, exp-flag'et cost-model                                            |
| `peephole-carry-rotate`         |     6 | Keep                                                                                            |
| `sm83`                          |     6 | Keep                                                                                            |
| `branch-range`                  |     5 | Keep — inkl. #267 pseudo-size fix                                                               |
| `peephole-tailcall`             |     5 | Keep                                                                                            |
| `shadow-regs`                   |     4 | Keep (feature gated)                                                                            |
| `MERGE-INLINE`                  |     4 | Drop (metadata)                                                                                 |
| `lld-relocs`                    |     3 | Keep — `R_Z80_ADDR16` wrap fix (#47)                                                            |
| `ACCIDENTAL-BUILD-ARTIFACT`     |     3 | **Revert** — filerne findes stadig: `build/lib/z80/{z80_rt.a,putchar.o,cpm_putchar.o,cpm_crt0.o}` |
| `REVERT`                        |     3 | Keep (paret med sit reverted-target)                                                            |
| `sdcccall`                      |     3 | Keep                                                                                            |
| `legalizer`                     |     2 | Keep                                                                                            |
| `f32-math32-abi`                |     2 | Keep — `-z80-float-sdcccall0` (#277) kritisk for zcc                                            |
| `compiler-rt`                   |     2 | Keep                                                                                            |
| `sret`                          |     2 | Keep — korrekthedsrettelser #268/#274                                                           |
| `pseudo-size`                   |     1 | Keep — #267                                                                                     |
| `ACCIDENTAL-BUGS-DIR`           |     1 | **Drop `bugs/switchbug.c`** fra commit, behold codegen-fix                                      |
| `quad-init-split`               |     1 | Keep — `.quad` → to `.long` for z88dk-asm (z88dk#27)                                            |

**Netto:** ca. 322 + 16 + 4 = **342 commits kan droppes helt** (noise + local-CI + tomme merges). Resten (~619) er reel kode + tests der skal med.

Rå per-tema-lister ligger i `tasks/_triage_themes.txt`, per-bucket i `tasks/_triage_buckets.txt`, commit-list `tasks/_triage_commits.tsv`, per-commit filliste `tasks/_triage_files.txt`.

## Klart oprydningsarbejde (uafhængigt af upstream-plan)

1. `git rm build/lib/z80/{z80_rt.a,cpm_crt0.o,cpm_putchar.o,putchar.o}` — pr-committed build-artefakter der aldrig burde have været tracket. Tilføj `build/` til `.gitignore`.
2. `git rm bugs/switchbug.c` — sample reproducer der er lækket ind i træet.
3. Overvej at fjerne hele `tasks/` fra upstream-submission (ikke fra fork'en) — det er projektets egen dokumentation.

## Strategi for upstream-submission

Kombination anbefales:

**A. Squash-by-theme til `submit/*` grene** — for hver tema-branch:
1. Start fra `upstream/main`.
2. `git checkout upstream/main -- <de-filer-temaet-rører>` fra vores tree state (dvs. den *færdige* form).
3. Skriv én commit-besked der forklarer *hvad* og *hvorfor* (per zlfn's krav, jf. `feedback_explain_before_filing`).
4. Kør lit → 251 pass / 61 XFAIL.
5. PR til `llvm-z80/llvm-z80`.

Fordel: en PR pr. sammenhængende koncept. Ulempe: mister granularitet i historik.

**B. Bevar-historik for det der *skal* forstås trin-for-trin** (typisk: korrekthedsrettelser der har flere revs, eller den generiske LLVM-transforms serie hvor upstream `llvm/llvm-project` PR'erne (#204915, #204920) allerede sætter kontekst).

### Foreslået rækkefølge

1. **Foundation, uafhængige, små:**
   - `submit/quad-init-split` (z88dk#27)
   - `submit/inline-asm-pair-constraints`
   - `submit/lld-r-z80-addr16-wrap` (#47)
   - `submit/pseudo-size-and-drift-guard` (#267)
   - `submit/sret-fixes` (#268, #274)
   - `submit/branch-range-fixes`
2. **z88dk/zcc essentials:**
   - `submit/z88dk-calling-conv` (cc130–133)
   - `submit/classic-libc-cc` (`-z80-classic-libc-cc`)
   - `submit/f32-sdcccall0` (`-z80-float-sdcccall0`, #277)
   - `submit/clang-frontend-attrs` (`z80_critical`, `address_space(2)`, `<intrinsic.h>`)
   - `submit/compiler-rt-additions` (helpers zcc bridge kalder)
3. **Codegen/peephole klynge** (kan splittes yderligere for reviewbarhed):
   - `submit/peephole-djnz`
   - `submit/peephole-bss-pushpop`
   - `submit/peephole-cp-hl`
   - `submit/peephole-tailcall`
   - `submit/peephole-inc-dec-carry-rotate`
   - `submit/isel-direct-bss-and-ptr-fold`
   - `submit/block-mem-ops` (LDIR/LDDR memset/memcpy inlining)
4. **Regalloc / TTI / passes:**
   - `submit/ix-iy-allocatable` (inkl. `COPY16_PUSHPOP`)
   - `submit/ix-remat`
   - `submit/tti-cost-model` (kun `isLegalAddImmediate` + Mul=Expensive ikke-gated; resten under exp-flag)
   - `submit/shadow-regs-feature`
   - `submit/static-stack-feature`
   - `submit/undocumented-insns-feature`
5. **Nye passes** (én PR pr. pass):
   - `Z80SplitDjnzCounters`, `Z80NarrowNoIndex`, `Z80PatternFillRecognize`, `Z80PinAluAccumulator`, `Z80KeepLoopPointerInPair`, `Z80LoopRotate`, `Z80LoopInstrFormPrep`, `Z80IndexIV`, `Z80SinkColdLoopIV`, `Z80PinLoopPointer`, `Z80ReorderTestDec`, `Z80RemoveJumpToNext`, `Z80AutoStaticFrame`, `Z80HighByteFirstBranch`, `Z80FuseCarryChain`, `Z80FixupImplicitDefs`, `Z80DanglingDebugCleanup`, `Z80NonReentrant`, `Z80PruneCallFrameDefs`
6. **Generic LLVM transforms** (hvis ikke allerede landet i `llvm/llvm-project`):
   - AggressiveInstCombine udvidelser (icmp-narrow, and-mask outside-graph)
   - `SimplifyCFG` `getPredictableBranchThreshold` gating
   - `LoopUtils::deleteDeadLoop` SSA-fix (#182)
   - `InstCombine` narrow i16 EQ/NE af byte sign-ext
   
   Route disse til `llvm/llvm-project` (jf. `feedback_upstream_routing_two_targets`), ikke fork'en.

## Om `z80-utils/`

`z80-utils/` er **allerede upstream** — skabt af @zlfn i marts 2026 (oprindeligt som `z80_test`, omdøbt i `3ce1745f64fd`). Det er en Rust-workspace *som implementations­sprog* for testværktøjerne (test-runner, elf2rel, rel2elf); det har intet med Rust-som-target-for-Z80 at gøre. Vores 96 rørte filer / 53 commits er altså ændringer *ovenpå* eksisterende upstream-kode.

Konsekvens for submission: **ingen separat z80-utils PR.** I stedet:

- **Runtime-fixtures** (`z80-utils/test-runner/testcases/{clang,llc,sdcc}/*`) bundtes med den kode-PR de verificerer — samme regel som lit-tests (jf. `CLAUDE.md` "Discipline for compiler changes").
- **Ændringer i harnessen selv** (`test-runner/src/…`, `elf2rel/`, `rel2elf/`) grupperes tematisk med den kode-ændring de understøtter (fx: `-z80-float-sdcccall0` bringer `.rel` float-test-support med sig), eller — hvis en ændring er ren infra der ikke hører til én kode-PR — samles i en lille "test-infra" PR sidst.
- **`Cargo.lock`-drift** rebase'es væk hvor muligt.

## Åbne beslutninger til dig

1. **Squash-alle-per-tema** vs. **cherry-pick-historik**: A) er hurtigere at reviewe men mister rationale; B) bevarer WHY men kræver at hver commit står forsvarligt alene. Anbefaling: **A) for peepholes og passes, B) for zcc-CC-arbejdet + korrekthedsrettelser** hvor rationale er tungt.
2. **Rækkefølge**: alt-på-én-gang som mange PR'er, eller trinvis feedback fra @zlfn per klynge? Anbefaling: **trinvis** — start med klynge 1 (Foundation) på én arbejdsdag, vent på reaktion, juster form for resten.

## Næste konkrete skridt (venter på din go-ahead)

- [ ] Rens træet for de committed build-artefakter (`build/lib/z80/*`, `bugs/switchbug.c`).
- [ ] Bekræft strategi A/B pr. klynge.
- [ ] Bekræft rækkefølge (trinvis start med Foundation).
- [ ] Bekræft submission-target: `llvm-z80/llvm-z80` for alt fork-specifikt; `llvm/llvm-project` for generiske LLVM-transforms.
- [ ] Så bygger jeg første `submit/*` branch og viser dig commit-teksten før PR sendes.
