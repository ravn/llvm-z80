# #316 static-frame regression — hvad skete der, og fix-mulighederne

Ledsager til `analysis-static-frame-regression-2026-09-11.md`. Denne fil forklarer
*hvorfor* de to mekanismer kolliderer, og gennemgår de tre fix-retninger med
korrektheds-afvejning. Skrevet som svar på "forklar mig mulighederne".

Regel: brug "upstream" / "fork-ejeren" — aldrig ejerens navn i committede filer.

---

## 1. Historikken — to mekanismer, uafhængigt udviklet

### Din oprindelige mekanisme (maj–aug 2026)
`Z80AutoStaticStack` — en clang-pass der selv laver call-graph-analyse, afgør per
funktion om den er ikke-rekursiv, og i så fald auto-injicerer `+static-stack`
(locals i BSS i stedet for stak).

- `ca2f0b0d` 2026-05-24 — Level 1 (leaves), `65c88c15` — Level 2 (SCC-ikke-rekursive non-leaves), #176
- `5247c580` 2026-05-31 — default-on + ISR-concurrency-safety-gate
- `15bb0ba0` 2026-08-11 — gjort sund på tværs af translation units
- `539d57ea` 2026-09-09 — *du* omdøbte den til `AutoStaticFrame` (for at matche upstreams navngivning i merget)

### Upstreams nye mekanisme (2026-09-06)
Tre dage før din rename tilføjede upstream **uafhængigt** deres egen static-frame-maskine:

- `59d8fad4` "Allocate non-reentrant function frames in static memory" — `Z80NonReentrant`-passet + `Z80StaticFrameAlloc`
- `f17c7b76` "Harden the static frame machinery at its edges"
- `9479e845` "Replace the ISR root list with a no-static-frame attribute"

### Begge lever nu — men upstreams vinder

| | **Din** `Z80AutoStaticFrame` | **Upstreams** `Z80NonReentrant` |
|---|---|---|
| Skriver | `+static-frame` target-feature | `"nonreentrant"` funktions-attribut |
| Analyse | leaf altid OK; non-leaf OK hvis local-linkage **eller** ikke-når-eksternt | tilføjer syntetisk `CallsExternalNode→ExternalCallingNode`-kant → strengere |
| Konservatisme | mindre (virkede for firmware) | mere (tainter ISR-via-ekstern + kryds-TU-globale) |

Forbrugeren `usesStaticFrame()` (`Z80FrameLowering.cpp:93`) kræver
`hasFnAttribute("nonreentrant")` — skrives **kun** af upstreams `Z80NonReentrant`.
Værre: upstreams pass fjerner aktivt enhver eksisterende `"nonreentrant"`
(`Z80NonReentrant.cpp:128-131`: *"This pass is the attribute's only legitimate
writer; drop any that arrived with the input IR"*) — den har gjort sig selv til
enevældig autoritet.

**Nettoeffekt:** din `+static-frame`-injektion er nu nødvendig men ikke tilstrækkelig.
Din analyses resultat kasseres; upstreams strengere analyse beslutter, og dens
ekstra konservatisme er hele densitets-regressionen (autoload ~25/27, rcbios ~74/78
funktioner taint'et som "reentrant").

---

## 2. Fundamentet — hvornår er en static frame sikker?

En static frame lægger locals på en **fast BSS-adresse**. Kun sikkert hvis funktionen
**aldrig kan have to aktiveringer i live samtidig**. To måder det sker:

1. **Rekursion** — funktionen kalder sig selv (direkte eller via cyklus).
2. **Concurrency** — den kører i main-flow, en ISR afbryder og genindtræder i den
   (delt helper), eller den *er* en ISR der genindtrædes.

Analysen skal bevise at **ingen af delene** kan ske.

## 3. Synderen — den syntetiske kant (`Z80NonReentrant.cpp:137`)

LLVMs call-graph har to specielle knuder:
- `ExternalCallingNode` = "omverdenen som kalder" — kanter til hver funktion der kan kaldes udefra (ekstern linkage / address-taken).
- `CallsExternalNode` = "kald der forlader modulet" — hver funktion der kalder en ekstern deklaration peger på den.

Upstream tilføjer **`CallsExternalNode → ExternalCallingNode`**. Det lukker en løkke:
*"kode der forlader modulet kan komme tilbage ind i enhver eksternt-synlig funktion."*
**Korrekt og nødvendigt** for kryds-TU-gensidig rekursion (TU-A's `f` kalder extern
`g`; TU-B's `g` kalder extern `f` — gensidigt rekursive, men hver ser ud som en leaf
i sin egen TU).

**Men** samme kant: enhver funktion der kalder eksternt → `CallsExternalNode` →
`ExternalCallingNode` → **hver** eksternt-synlig funktion. Så når kontekst-walken går
gennem en ISR der bare kalder én ekstern helper, "når" ISR'en hver global funktion →
alle markeres delt mellem ISR- og main-kontekst → alle reentrant. Det er regressionen.

**Bevist:** probe med ISR-kalder-eksternt → 0 nonreentrant; ISR-kalder-intet → 3.

---

## 4. De tre muligheder

### Mulighed 1 — Freestanding lukket-program

**Idé:** I en freestanding whole-program firmware findes ingen omverden der kalder ind.
Eneste entry-points er reset-vektoren og hardware-interrupt-vektorerne — de er *i*
modulet (kontekst-rødder). Ingen ekstern kode holder pointere til dine `dso_local`-
funktioner. Altså modellerer den syntetiske kant en kalder der ikke eksisterer.

**Kodeændring:** Gør tilføjelsen af kanten (linje 137) betinget — udelad når target er
freestanding / `reloc=static`, **men behold for address-taken funktioner** (dem kan en
runtime-installeret vektor / callback ægte kalde).

**Hvorfor:** Spøgelses-kanten forsvinder → ISR'er der kalder eksterne helpers taint'er
ikke alle globale; kryds-TU-kald til leaf-helpers taint'er ikke. Static frames genvindes.

**Korrektheds-risiko:** Lav. Gives `&f` ud til fremmed kode, *er* `f` address-taken →
gaten beholder kanten for den → sikker. Restrisiko: fremmed ISR i ROM der kalder tilbage
— netop hvad "freestanding lukket" udelukker.

**Slægtskab:** Genindfører *din* antagelse (freestanding = lukket) som en gate på
upstreams kant; upstreams maskineri bevares.

### Mulighed 2 — Adskil rekursion fra kontekst

**Idé:** Kanten er nødvendig for **rekursions**-detektion. Over-taint'en sker i
**kontekst/reentrancy**-walken, som bruger samme graf. En ISR der kalder en ekstern
*leaf* burde ikke tælle som at nå hver global.

**Kodeændring — lille:** I dag: tilføj kant (137) → rekursions-SCC-scan (178) →
kontekst-walk (205) → fjern kant (214). **Flyt fjernelsen (214) op til før kontekst-
walken.** Rekursions-scanet ser kanten; kontekst-walken gør ikke.

**Hvorfor:**
- Rekursions-scan (med kant): fanger stadig kryds-TU-gensidig rekursion. ✓
- Main-kontekst (`ExternalCallingNode`): når alle globale via sine *ægte* kanter. ✓
- ISR-kontekst: ISR → *ægte* callees. Kalder eksternt → `CallsExternalNode` (blindgyde) →
  når ikke globale. Ægte delt helper (rigtig kant ISR→in-module `h`) fanges stadig. ✓

**Korrektheds-risiko:** Ét sjældent hul — ISR → ekstern kode → (kalder tilbage i) en
address-taken global der også er i main. Sjælden i firmware. (Mulighed 1's
address-taken-gate lukker netop dette; Mulighed 2 gør ikke.)

**Slægtskab:** Fuldt i upstreams model — raffinerer kun *hvilken* walk der bruger den
pessimistiske kant. Mindste adfærdsændring, laveste regressions-risiko for det generelle
biblioteks-tilfælde, mest upstreambar.

### Mulighed 3 — Genindsæt din analyse som autoritet

**Idé:** Lad din `Z80AutoStaticFrame` skrive `"nonreentrant"` (eller lad `usesStaticFrame`
acceptere din feature-injektion), og slå upstreams `Z80NonReentrant` fra / stop den fra
at strippe attributten. Din analyse bestemmer igen.

**Hvorfor:** Fuld genopretning af før-merge-adfærd.

**Korrektheds-risiko:** Højest. Upstream gjorde bevidst `NonReentrant` til enevældig
skriver og tilføjede kryds-TU/ISR-hærdning (`f17c7b76`, `9479e845`). Missede din analyse
et tilfælde de rettede, genindfører du bug'en. To uenige analyser + maksimal divergens →
fremtidig merge-friktion.

**Slægtskab:** "Behold min, drop deres" — reneste konceptuelt *hvis* din analyse er mindst
lige så sund, men størst afvigelse, og du mister upstreams hærdning.

---

## 5. Sammenligning + anbefaling

| | Fix-størrelse | Korrekthed | Upstream-divergens |
|---|---|---|---|
| **1 Freestanding** | lille (gate + address-taken-undtagelse) | høj (lukker address-taken-hullet) | lav — ren, upstreambar |
| **2 Adskil walks** | mindst (flyt én linje) | god (ét sjældent hul åbent) | mindst — helt i upstreams ånd |
| **3 Genindsæt din** | mellem | risikabel (mister hærdning) | størst |

**Anbefaling:** Mulighed 2 som mindste, sikreste indgreb der beholder upstreams
rekursions-soundness — eventuelt kombineret med Mulighed 1's address-taken-gate for at
lukke det sidste hul helt.

---

## 6. BESLUTNING (bruger 2026-09-11): fokus = tilpas os upstream

Retningen er at **tilpasse os upstream**, ikke at maintaine en divergent fork.
Konsekvenser:

- **Mulighed 3 er FORKASTET** (størst divergens; genindfører vores gamle analyse som
  autoritet — det modsatte af at tilpasse os).
- Foretruk rækkefølge:
  1. **Ren build-tilpasning (nul backend-ændring) — undersøges først.** Upstreams
     analyse er *korrekt* for dens antagelser (separat kompilering, mulige eksterne
     callbacks). Regressionen opstår fordi vores firmware-build giver den en *åben*
     graf: autoload compilerer per-TU (kryds-TU-kald ser eksterne ud), og eksternt-
     synlige symboler gør at `ExternalCallingNode` når dem. Bygger vi firmwaren
     **whole-program (LTO) + internaliserer** alt undtagen de ægte entry-points
     (reset/ISR-vektorer), ser upstreams analyse den *lukkede* graf den er designet
     til: kryds-TU-kald opløses, ikke-eksporterede funktioner bliver `internal` →
     `ExternalCallingNode` når dem ikke → den syntetiske kant taint'er dem ikke.
     Dette er den mest tro "tilpas os upstream": nul backend-divergens.
     - Åbent spørgsmål: rcbios bruger allerede `-flto`, men regresserede alligevel —
       skal verificeres om LTO faktisk internaliserer (linker-script/eksporterede
       symboler kan holde alt synligt). Runtime-helpers i `z80_rt.a` forbliver
       eksterne, men en *intern* kaller havner ikke i en ekstern cyklus.
  2. **Mulighed 2 (minimal, upstreambar in-model raffinering)** hvis build-tilpasning
     ikke kan lukke det helt — og indsend den som forslag til upstream frem for at
     bære den som fork-patch.
  3. **Mulighed 1** kun hvis address-taken-hullet i praksis viser sig relevant.

---

## 7. Hvad din kode kan som upstream ikke kan (og omvendt)

Afgør præcis hvad vi *mister* ved at tilpasse os. Konklusion: din kode har reelt
**én** unik egenskab — den antager en *lukket verden*.

### Din kode kan (upstream kan ikke)
Din analyse modellerer **ikke** "ekstern kode kan kalde tilbage i vilkårlige
modul-funktioner". Derfor giver den static frames til:
- eksternt-synlige **leaves** (Level 1: leaf → altid safe), og
- funktioner selv når ISR'er kalder **eksterne/kryds-TU-helpers** (ingen syntetisk
  external-callback-kant).

Upstream kan det ikke: dens `CallsExternalNode→ExternalCallingNode`-kant antager en
åben verden. Sundt for separat-kompilerede biblioteker, pessimistisk for freestanding
firmware — hele regressionen.

### Upstream kan (din kode kan ikke)
- **Static frames til interrupt-handlere selv + deres eksklusive callees** (én
  kontekst, selv om den er en ISR). Bevist: upstream markerede `@isr` `nonreentrant`.
  Din `Unsafe`-mængde taint'er alt reachable-from-interrupt → du afviser dem blankt.
- **Mere præcis kontekst-analyse** (nået-fra-2-kontekster vs. din nået-fra-nogen-ISR).

### Pointen
Din egenskab er ikke maskineri upstream *mangler* — det er en **antagelse** (lukket
program) bagt ind i din kode. Den sandhed kan gives upstream **gennem linkage** i
stedet: internalisér alt undtagen ægte entry-points → ikke-eksporterede funktioner
bliver `internal` → `ExternalCallingNode` når dem ikke → upstreams åben-verden-model
taint'er dem ikke. Altså: whole-program + internalisering **giver egenskaben tilbage
uden din kode** — du fortæller bare upstream sandheden via linkage. Derfor er build-
vejen den rigtige "tilpas os upstream".

---

## 8. EMPIRISK VERIFIKATION 2026-09-11 — build-vejen VIRKER IKKE, og en del af regressionen er en KORREKTHEDS-fix

Testede whole-program + internalisering konkret (llvm-link af rom.c+boot_rom.c+intvec.c
uden -g, `opt -internalize` tom preserve-liste, kør analysen):

- **FØR:** 27–28 defined, **2** nonreentrant, **101** `add hl,sp`.
- **EFTER internalisering:** 28 defined, **3** nonreentrant, **99** `add hl,sp`.
- → **Build-vejen genopretter stort set intet.** Hypotesen i §6/§7 om at internalisering
  fodrer upstream den lukkede graf er **empirisk modbevist** for autoload.

**Hvorfor:** en stor del af de afviste funktioner er **ægte reentrant**. ISR-bodies viser
at floppy-completion-ISR'en kalder `fdc_read_result`, `fdc_read_when_ready`,
`fdc_sense_interrupt`, `fdc_write_when_ready` — **de samme FDC-funktioner som main-boot-
flowet kalder**. Fyrer ISR'en mens main er inde i `fdc_read_result`, har funktionen to
live aktiveringer → en fast BSS-frame ville korrumperes. **Upstream afviser dem korrekt.**

**Den GAMLE kode (din compiler) gav dem static frames alligevel:** hele gamle autoload
har **0 `add hl,sp`** (5 `.frame`-symboler; `fdc_read_result` adresserer fast BSS `$6d4d`).

**Det åbne korrektheds-spørgsmål (kun brugeren kan svare):** er interrupts *slået fra*
mens main-flowet kalder FDC-funktionerne (poll-during-DI)? I så fald er der aldrig ægte
samtidighed → de static frames var **sunde**, og upstream er blot over-konservativ (den
modellerer ikke DI-vinduer) → vi skal fortælle upstream at de er sikre. Hvis interrupts
kan fyre under FDC-kaldene, var den gamle densitet **usund** (latent ISR-korruption der
MAME-testede OK ved held) og upstreams afvisning er en ægte bug-fix vi skal beholde.

**Konsekvens for fix-valget:** afhænger af svaret ovenfor.
- Sund (DI-vindue): behøver en måde at annotere "denne ISR-delte funktion er reelt ikke
  samtidig" — fx at ISR'er kørende med interrupts-fra ikke tæller som en separat samtidig
  kontekst for funktioner de deler med main under DI. Det er en upstream-model-udvidelse.
- Usund: behold upstreams afvisning; genvind kun densitet på de funktioner der IKKE er
  ISR-delte (dem der stadig burde få frames — undersøg hvorfor fx `compare_6bytes`,
  som ISR'erne IKKE kalder, alligevel ikke er nonreentrant).

---

## 9. #2 (den rene over-konservatisme) DEFINITIVT kortlagt 2026-09-11

Tro repro (ægte clang-pipeline): eksporteret `entry` → `main_leaf` (register-pres,
IKKE ISR-kaldt); `__interrupt isr` der kalder navngivet ekstern `ext_helper`; `isr`
address-taken via en IVT-lignende tabel.

- ISR kalder navngivet ekstern → `main_leaf` er `norecurse` men **ikke** `nonreentrant`,
  **39** `add hl,sp`.
- ISR kalder INTET eksternt → `main_leaf` bliver `norecurse nonreentrant` (genoprettet).
- `main_leaf` gjort **`internal`** (static, noinline) → **stadig** afvist, stadig 39
  `add hl,sp`.

**Konklusion: #2 kan IKKE fikses build-side (internalisering hjælper ikke).** Den interne
funktion nås af ISR'en *gennem den eksporterede entry*:

```
isr → ext_helper (navngivet ekstern) → CallsExternalNode → ExternalCallingNode
    → entry (ekstern-linkage, MÅ eksporteres) → main_leaf (intern)
```

Så længe der findes ét eksporteret entry der transitivt kalder funktionen, OG én ISR der
kalder *hvad som helst* eksternt, når ISR-kontekst-walken hele programmet via den
syntetiske kant → alt markeres nået-fra-2-kontekster → reentrant. Det forklarer også
hvorfor whole-program+internalisér kun gav 2→3 (§8): de address-taken ISR'er + den
eksporterede entry er uundgåelige taint-vektorer.

**Intent vs. implementering (kernen i #2):** kommentaren (`Z80NonReentrant.cpp:194-196`)
siger konservatismen dækker *"an indirect call ... reaches every address-taken function"*
— altså **indirekte kald → address-taken**. Men implementeringen udløses også af
**navngivne eksterne kald** (reloc_zx0/runtime-helpers, der ikke kan kalde vilkårligt
tilbage) og når **hver ekstern-linkage-funktion**, ikke kun address-taken. `main_leaf`
er hverken indirekte-kaldt-mål eller address-taken → afvist alligevel.

**Sikker fix-retning for #2 (upstreambar, matcher intent):** i KONTEKST-walken skal et
*navngivet* eksternt kald ikke udløse whole-program-reachability — kun ægte *indirekte*
kald skal nå de ægte *address-taken* funktioner. Det bevarer den rigtige indirekte-kald-
sikkerhed (som Mulighed 2 alene ville svække) og genvinder al ikke-ISR-delt densitet.
Rekursions-scanet (§3) beholder den syntetiske kant uændret. #1 (ægte FDC/ISR-deling)
er orthogonal og kræver stadig programmør-assertion.

---

## 10. IMPLEMENTERET + TESTET 2026-09-11 — stramningen er USUND (rullet tilbage)

Implementerede §9's stramning: i kontekst-walken erstat `CallsExternalNode ->
ExternalCallingNode` med `CallsExternalNode -> {lokalt address-taken funktioner}`.

**Repro-genopretning så lovende ud:**
- faithful repro: `main_leaf` 39 -> 0 `add hl,sp`, nu `nonreentrant`.
- autoload rom.c: nonreentrant 2 -> 5, `add hl,sp` 101 -> 76, `compare_6bytes`
  genoprettet, FDC-funktionerne korrekt stadig afvist.

**MEN lit-suiten fangede den (25 -> 26 FAIL): `static-frames-indirect-isr.ll`.** Den test
findes præcis for dette: en ISR laver et *indirekte* kald; `main` (ekstern-linkage, IKKE
lokalt address-taken) skal alligevel afvises, fordi **en ekstern-linkage-funktion kan
være address-taken i en ANDEN TU** (separat kompilering) — modulet kan ikke se det, så et
indirekte kald kan ramme den. Stramningen til *lokalt* address-taken misser det → gav
`main` en frame den ikke må have.

**Dybt resultat:** enhver *sund* stramning på modul-niveau er `{address-taken} ∪
{ekstern-linkage}` = præcis hvad `ExternalCallingNode` allerede når. **Der findes INGEN
sund per-modul-fix** — upstreams brede reach ER nødvendig for separat kompilering.

**Rullet tilbage** (Z80NonReentrant.cpp uændret). Konklusion: densiteten kan kun genvindes
*sundt* ved **whole-program** (LTO + internalisér), hvor ikke-eksporterede ekstern-linkage-
funktioner bliver `internal`, så `hasAddressTaken()` bliver autoritativ og upstreams
STOCK-kode selv holder dem ude af den eksterne reach. Det er den eneste sunde "tilpas os
upstream". Åbent: hvorfor gav min tidligere whole-program+internalisér-test (§8) kun 2->3?
— skal genundersøges med korrekt internalisering (entry inkl.), for det er den rigtige vej.
