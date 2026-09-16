# Plan: Static-frame eligibility under ISR external calls & freestanding firmware

**Kontekst:** Efter upstreams omskrivning til `Z80NonReentrant` + `Z80StaticFrameAlloc` (commit `59d8fad4`) mister funktioner i firmware (som `autoload-in-c` og `rcbios-in-c`) deres statiske stak-rammer og falder tilbage til SP-relative rammer med kraftig kodevækst (+50-60%) til følge. Denne plan analyserer rodårsagen, sammenholder med upstreams designfilosofi, og opstiller løsningsmodeller.

---

## 1. Symptom og observeret adfærd

I et program med en interrupt-handler (`__attribute__((interrupt))`):
1. Hvis interrupt-handleren (eller en funktion den kalder) kalder en **ekstern deklaration** (f.eks. en port-I/O hjælper, BIOS-rutine eller runtime-builtin som `memcpy`),
2. Bliver **alle** funktioner i modulet med ekstern linkage (dvs. alle funktioner, der ikke er erklæret `static`) markeret som:
   `Reachable from multiple contexts: <funktion>`
3. Dette medfører, at `Z80NonReentrant` nægter at tildele attributten `nonreentrant`.
4. Uden `nonreentrant` afviser `Z80FrameLowering::usesStaticFrame()` statisk allokering, og koden falder tilbage til dyre stak-rammer (`add hl,sp` og `(ix+d)`).

### Konkret repro (minimal C-case)

```c
// extern helper, f.eks. I/O eller BIOS
void ext_out(unsigned char port, unsigned char val);

// En urelateret main-funktion (ekstern linkage, ikke address-taken)
int compute(int a, int b) {
    volatile int x = a + b;
    return x * 2;
}

// Interrupt-rutine der kalder en ekstern hjælper
void isr(void) __attribute__((interrupt)) {
    ext_out(0x10, 0x01);
}
```

* **Forventet:** `compute()` kaldes kun fra main-line kode, aldrig fra `isr()`. Skal have statisk ramme (`compute.frame`).
* **Faktisk adfærd:** `compute()` afvises som reentrant fordi `isr -> ext_out -> CallsExternalNode -> ExternalCallingNode -> compute`.

---

## 2. Teknisk rodårsagsanalyse i `Z80NonReentrant.cpp`

Upstreams analyse i [`Z80NonReentrant.cpp`](file:///Users/ravn/z80/llvm-z80/llvm/lib/Target/Z80/Z80NonReentrant.cpp) har to uafhængige trin:
1. **Rekursionsanalyse (SCC):** Finder ud af, om en funktion kan have to aktiveringer i samme kaldetræ.
2. **Kontekstanalyse (`visitContext`):** Finder ud af, om en funktion kan nås fra mere end én eksekveringskontekst (kontekst 0 = `ExternalCallingNode` / omverdenen; kontekst 1..N = hver enkelt interrupt-handler).

I linje 179-180 indsætter upstream en kunstig kant:
```cpp
CG.getCallsExternalNode()->addCalledFunction(nullptr,
                                             CG.getExternalCallingNode());
```

Upstreams egen begrundelse (linje 237-239):
```cpp
// The walks run with the artificial external edge still in place, so an
// indirect call inside one context conservatively reaches every
// address-taken function.
```

### Hvor uoverensstemmelsen opstår

LLVM's generiske `CallGraph` samler to uafhængige koncepter i de samme knuder:
* `CallsExternalNode` repræsenterer BÅDE:
  - Indirekte kald gennem funktionspointere (`call hl`).
  - Kald til eksterne deklarationer (`declare void @foo()`).
* `ExternalCallingNode` repræsenterer BÅDE:
  - Funktioner, hvis adresse reelt er taget (`hasAddressTaken()`).
  - **Alle** funktioner med ekstern linkage (`!hasLocalLinkage()`), dvs. alle normale C-funktioner uden `static`.

Når `CallsExternalNode -> ExternalCallingNode` holdes aktiv under kontekst-analysen:
* Et kald fra en ISR til en ekstern hjælper (f.eks. `ext_out`) fører til `CallsExternalNode`.
* `CallsExternalNode` leder til `ExternalCallingNode`.
* `ExternalCallingNode` forgrener sig ud til **alle** ikke-statiske funktioner i modulet.
* Da `ExternalCallingNode` allerede blev besøgt under Kontekst 0 (main-line), markeres samtlige disse funktioner som værende i konflikt mellem ISR og main-line!

---

## 3. Upstreams holdning og filosofi

Upstream har en stærk og velbegrundet designfilosofi:
1. **Sound by construction via CallGraph-analyse:**
   Compileren skal bevise, at en funktion ikke kan aktiveres samtidig. Det må ikke baseres på uverificerede antagelser.
2. **Afvisning af usikre manuelle attributter:**
   I linje 165-173 sletter `Z80NonReentrant` eksplicit enhver indkommende `nonreentrant`-attribut:
   > *"This pass is the attribute's only legitimate writer; drop any that arrived with the input IR so everything downstream is backed by this run's analysis."*
   Upstream ønsker altså ikke en `__attribute__((nonreentrant))`, der tillader programmøren at omgå analysen og risikere tavs datakorruption.
3. **Eksplicit opt-out findes allerede:**
   Upstream understøtter `__attribute__((target("no-static-frame")))` til funktioner i eksterne/usynlige kontekster.

---

## 4. Løsningsmodeller (rangeret efter upstream-forenelighed)

### Model 1: Præcisér kontekst-analysen til upstreams intention (Anbefalet)
* **Princip:** Hold kanten `CallsExternalNode -> ExternalCallingNode` aktiv under **rekursionsanalysen** (hvor den er nødvendig for at opdage cross-TU gensidig rekursion), men **fjern eller begræns den** under `visitContext`.
* Under `visitContext` skal et eksternt kald eller indirekte kald kun nå funktioner, der reelt har fået taget deres adresse (`F->hasAddressTaken()`), eller som eksplicit er entry points. En funktion med blot ekstern linkage kan ikke spontant kaldes fra et vilkårligt libcall/hardwarekald i en ISR, medmindre dens adresse er givet videre.
* **Fordel:** 100% automatisk, kræver ingen ændringer i kildekode eller nye driver-flags. Løser problemet direkte i analysens ånd.

### Model 2: Freestanding / Closed-world tilstand (`-ffreestanding` / `-mllvm -z80-closed-world`)
* **Princip:** I standalone firmware som `autoload` og `rcbios` køres der under `-ffreestanding`. I denne tilstand ved compileren, at der ikke findes en ekstern verden, der kalder ind i vilkårlige globale symboler.
* I `Z80NonReentrant` udelades `CallsExternalNode -> ExternalCallingNode` helt, når closed-world/freestanding er aktiv.
* **Fordel:** Ekstremt robust for firmware; matcher C-standardens semantik for fritstående miljøer.

### Model 3: Positiv target-attribut (`__attribute__((target("static-frame")))`)
* **Princip:** Det symmetriske modstykke til `no-static-frame`. Giver udvikleren mulighed for at tvinge statisk ramme på kritiske funktioner.
* **Ulempe:** Afviger fra upstreams ønske om compiler-garanteret sikkerhed.

---

## 5. Handlingsplan

- [ ] **Trin 1:** Opret issue på GitHub (`ravn/llvm-z80`), der præcist dokumenterer denne over-konservatisme i kontekstanalysen ved ISR-kald til eksterne deklarationer.
- [ ] **Trin 2:** Konstruér en minimal lit-test (`llvm/test/CodeGen/Z80/static-frames-isr-extcall.ll`), der demonstrerer fejlen (en ikke-ISR funktion mister fejlagtigt sin statiske ramme, når en ISR kalder en ekstern deklaration).
- [ ] **Trin 3:** Implementér prototype af Model 1 (begræns overførsel under `visitContext` til kun reelt adresse-tagne funktioner) og verificér mod lit-suiten og `autoload-in-c`.
- [ ] **Trin 4:** Forbered upstream-rettelse / diskussion baseret på måleresultaterne.
