# XFAIL inventory — main (2026-09-16)

61 XFAIL-tests i `llvm/test/CodeGen/Z80/`. Empirisk verificeret: alle 61 fejler stadig når `XFAIL: *` fjernes — ingen er stale. Alle 61 er fork-tilføjede (upstream/main har 0 XFAIL).

Kategorier:
- **CORR** — korrekthedsbug: fix er typisk lille (regalloc/liveness/metadata gate). Anbefaling: FIX.
- **CORR-HARD** — korrekthedsbug men kræver design eller ISel/legalizer-arbejde. Anbefaling: FIX hvis tid, ellers PARK.
- **DENSITY** — density-optimering (user-direktiv: ingen mere density-arbejde nu). Anbefaling: SKIP.
- **FEATURE** — parked/gated feature, XFAIL pinner opførsel med flag. Anbefaling: SKIP (kræver un-parking).
- **UPSTREAM** — bug i `llvm/llvm-project`, filed der. Anbefaling: SKIP (forkert repo).
- **META** — drift-guard, tester at invariant holder. Anbefaling: FIX (typisk lille metadata-rettelse).
- **SHAPE-DRIFT** — test kræver et kodegen-shape der ikke længere produceres; skal enten opdateres eller slettes.

| # | Fil | Kat | Issue | Verdict | Kort |
|---:|---|---|---|---|---|
| 1 | `add16-acc.ll` | FEATURE | #178 | SKIP | Default-OFF `-z80-add16-acc`; parked ADD16_acc pseudo path (size regression under IX/IY-reserved). |
| 2 | `bss-self-clear.ll` | CORR | #51 | FIX | BSS-self-clear via memcpy(p+1,p,n-1) må ikke clobber dst-pointer i +static-frame. |
| 3 | `dead-bss-store-back.mir` | DENSITY | – | SKIP | Dead-store elimination (backward scan). Density peephole. |
| 4 | `fcmp-libcall-return-i16.ll` | CORR | – | FIX | fcmp-libcall retur behandles som i16 (ikke i32) i legalizer. |
| 5 | `high-byte-first-loop-exit.ll` | FEATURE | – | SKIP | Pinner HBF-branch flag opførsel. |
| 6 | `hl-no-bc-backup.ll` | SHAPE-DRIFT | #180 | UPDATE | Peephole fjernet per #180 C2 audit; shape produceres ikke længere. Test er en anti-check. |
| 7 | `inline-runtime-size-verify.ll` | META | #240 | FIX | Drift-guard for MUL16/UDIV16/UMOD16/SDIV16/SMOD16 pseudo-størrelser. |
| 8 | `issue-105-ldir-guarded.ll` | CORR | #105 | FIX | LDIR med BC=0 → 65536 iter → 64KB scramble. Pseudo `LDIR_GUARDED` findes på main; expansion-check kan mismatche. |
| 9 | `issue-114-exx-bracket-candidate.ll` | DENSITY | #114 | SKIP | EXX-bracket prototype for outer-counter parkering. Density. |
| 10 | `issue-12-auto-static-frame-cross-tu-recursion.ll` | FEATURE | #12 | SKIP | Auto-static-frame cross-TU cycle safety; parked hasFP=false family. |
| 11 | `issue-140-cross-mbb-edge-split.mir` | CORR | #140 | FIX | Cross-MBB BSS-spill peephole (#132 follow-up). |
| 12 | `issue-156428-livevars-independent-subregs.mir` | UPSTREAM | llvm/llvm-project#156428 | SKIP | Generic LiveVariables bug; upstream. |
| 13 | `issue-167-branchless-conditional-xor.ll` | CORR-HARD | #167 | SKIP | SimplifyCFG foldTwoEntryPHINode + backend select-lowering. Mid-end design. |
| 14 | `issue-175-alu-ixd.mir` | DENSITY | #175 | SKIP | LD r,(IX+d); ALU_r → `ALU a,(IX+d)` fusion. -1B/-4T per site. |
| 15 | `issue-176-auto-static-frame.ll` | FEATURE | #176 | SKIP | Auto-static-frame safety gate (leaf/ISR/address-taken). Parked. |
| 16 | `issue-194-late-opt-liveness.mir` | CORR | #194 | FIX | Late-opt peephole efterlader stale liveness ($a bliver undefined use). |
| 17 | `issue-197-isreglivat-skip-undef-use.mir` | CORR | #197 | FIX | isRegLiveAt tæller undef-brug som live; PUSH_HL over-save. |
| 18 | `issue-200-spill-gr16-offset-operand-count.mir` | CORR | #200 | FIX | SPILL_GR16 3-op erklæret, 2-op efter PEI; -verify-machineinstrs "Too few operands". |
| 19 | `issue-205-reverse-fill-seed.ll` | FEATURE | #205 | SKIP | Default-OFF `-z80-reverse-fill-seed`; -1B per site. |
| 20 | `issue-209-stack-reserve-push-af-undef.ll` | CORR | #209 | FIX | PUSH_AF for stack reserve læser undef $a; skal have implicit undef. |
| 21 | `issue-210-prologue-large-frame-push-hl-undef.ll` | CORR | #210 | FIX | Large-frame prologue PUSH_HL for save læser undef $hl når HL ikke live-in. |
| 22 | `issue-210-sprelative-flag-push-af-dead-a.mir` | CORR | #210 | FIX | SP-relative frame emit skal markere $a undef i PUSH_AF når A død. |
| 23 | `issue-210-sprelative-spill-dead-hl-half.mir` | CORR | #210 | FIX | Per-register-unit liveness ved SP-relative spill; skip PUSH_HL når HL dead. |
| 24 | `issue-212-add-iy-peephole-hl-live.mir` | CORR | #212 | FIX | ADD IX/IY peephole må kun folde når $hl er killed i closing copy. |
| 25 | `issue-212-gr8-reload-hl-kill.mir` | CORR | #212 | FIX | -O0 +static-frame fastregalloc kill markering + reload offset out-of-range. |
| 26 | `issue-216-cp-sbc-and.ll` | DENSITY | #216 | SKIP | `(x<imm)?v:0` → cp/sbc/and idiom (5B vs 8B). |
| 27 | `issue-236-spill-hl-liveness-gate.mir` | CORR | #236 | FIX | expandSpillGR16* bruger `isKill()` (usikker) i stedet for reel liveness → drop af restore → miscompile. |
| 28 | `issue-237-add-hl-fi-implicit-def.mir` | CORR | #237 | FIX | ADD_HL_FI PUSH_HL læser undef half. |
| 29 | `issue-239-site1-zext-iy-hl-partial.mir` | CORR | #239 | FIX | ZEXT_GR8_GR16 IY-dest: partial-undef $hl i PUSH_HL. |
| 30 | `issue-239-site1b-sext-iy-hl-partial.mir` | CORR | #239 | FIX | SEXT_GR8_GR16 IY-dest sibling af site 1. |
| 31 | `issue-239-site2-spill-gr16-iy-hl-partial.mir` | CORR | #239 | FIX | SPILL_GR16 med IX/IY source, partial-undef PUSH_HL. |
| 32 | `issue-239-site3-reload-gr16-iy-hl-partial.mir` | CORR | #239 | FIX | RELOAD_GR16 med IX/IY dest, partial-undef PUSH_HL. |
| 33 | `issue-239-site4-sext16-iy-src-hl-partial.mir` | CORR | #239 | FIX | SEXT16 med IX/IY src (SrcIsIR=true). |
| 34 | `issue-239-site5-sext16-iy-dst-hl-partial.mir` | CORR | #239 | FIX | SEXT16 med IX/IY dst (DstIsIR=true). |
| 35 | `issue-239-site5a-copy-ixh-to-gr8-hl-partial.mir` | CORR | #239 | FIX | copyPhysReg IXH/IXL/IYH/IYL → GR8. |
| 36 | `issue-239-site5c-copy-gr8-to-ixh-hl-partial.mir` | CORR | #239 | FIX | copyPhysReg GR8 → IXH/IXL/IYH/IYL. |
| 37 | `issue-239-site6-copy-sp-bc-hl-undef.mir` | CORR | #239 | FIX | copyPhysReg SP→BC|DE unconditional PUSH_HL med undef. |
| 38 | `issue-241-nodbg-add-commutativity.mir` | CORR | #241 | FIX | ADD peephole bruger raw std::next; DBG_VALUE bail. |
| 39 | `issue-241-nodbg-cmp-swap.mir` | CORR | #241 | FIX | CP swap peephole samme bug. |
| 40 | `issue-241-nodbg-incdec-mem.mir` | CORR | #241 | FIX | INC/DEC-mem peephole samme bug. |
| 41 | `issue-244-div-fast-o3.ll` | FEATURE | #244 | SKIP | `-z80-fast-div` opt-in flag. |
| 42 | `issue-249-word-pointer-iy-park.ll` | FEATURE | #249, #251 | SKIP | Z80KeepLoopPointerInPair pass, gated flag. |
| 43 | `issue-267-pseudo-size-drift-guard.ll` | META | #267, #240 | FIX | Drift-guard for MUL8/DIV8/MOD8 + saturating-i8 pseudos. |
| 44 | `issue-27-iy-indexed-addr.ll` | DENSITY | #27 | SKIP | IX/IY-displacement (ld r,d(i?)) vs base+add. |
| 45 | `issue-28-large-offset-iy-spill.ll` | FEATURE | #28, #263 | SKIP | Guards large-offset IX/IY spill; #263 direct-addr flag defeats det. |
| 46 | `issue-331-spill-push-pop.ll` | FEATURE | #331 | SKIP | Parked SP-relative frame → PUSH/POP peephole (unsound). |
| 47 | `issue-97-bc-pingpong-singlebb.ll` | SHAPE-DRIFT | #97, #180 | UPDATE | Peephole fjernet; shape produceres ikke længere; test forbyder shape. |
| 48 | `issue-97a-bc-pingpong-i16-counter.ll` | SHAPE-DRIFT | #99, #97 | UPDATE | i16-counter subcase; peephole bortfaldet. |
| 49 | `issue-ascii-octal-bytedrop.ll` | CORR | – | FIX | Assembler-side octal-escape ambiguitet: 0x04 efterfulgt af '0' → bytedrop. Ren korrekthed. |
| 50 | `iy-hl-mirror-fold-to-hlind.mir` | DENSITY | #243 | SKIP | (IX/IY+0) → (HL) fold. |
| 51 | `iy-loop-carried-112.ll` | CORR | #112, #14, #189 | FIX | i32-popcount loop-carried IY update kan droppes af peephole; guard mangler. |
| 52 | `iy-no-static-stack-miscompile-189.ll` | CORR | #189, #27 | FIX | IY-as-GPR miscompile default IX-frame; -z80-unreserve-iy. |
| 53 | `late-opt-bitset-mcsymbol-offset.ll` | CORR | #264 | FIX | RMW → bit-set peephole builder mister MO_MCSymbol offset. |
| 54 | `lea-fi-iy-112.ll` | CORR | #112 | FIX | LEA_IX_FI eliminateFrameIndex mangler IY-dest case → llvm_unreachable no-op i Release. |
| 55 | `outliner.ll` | FEATURE | #322 | SKIP | MachineOutliner eksplicit disabled; -1B/CALL for stort. |
| 56 | `pointer-iv-strength-reduce.ll` | FEATURE | #250 | SKIP | Byte-array loops med non-const stride; feature-gated. |
| 57 | `postra-compare-merge-pop-af.mir` | CORR | #265 | FIX | Z80PostRACompareMerge fjerner OR A som redundant hvis foregående def er POP_AF (som restaurerer flags fra tidligere). |
| 58 | `quad-init-split-27.ll` | CORR | z88dk#27 | FIX | 64-bit global initializer skal splittes i to .long på ELF/GNU tekstpath. Direkte zcc-behov. |
| 59 | `regalloc-hint-aes-shape.ll` | DENSITY | #115, #27 | SKIP | AES-mc-inv shape regalloc hint wiring test. |
| 60 | `sink-cold-loop-iv.ll` | FEATURE | #250 | SKIP | Sieve scan cold-loop IV sink; feature. |
| 61 | `static-stack-fp-direct-addr.ll` | FEATURE | #263 | SKIP | `-z80-static-frame-fp-direct-addr` flag. |

## Tallier

| Verdict | Antal |
|---|---:|
| **FIX** | **28** |
| SKIP (DENSITY) | 9 |
| SKIP (FEATURE) | 14 |
| SKIP (UPSTREAM) | 1 |
| SKIP (CORR-HARD) | 1 |
| UPDATE (SHAPE-DRIFT) | 4 |
| SKIP (andre) | 4 |

De 28 med verdict **FIX** er dem "der med rimelighed kan fikses". De fordeler sig på:

**Regalloc/liveness metadata (16):**
- #197 (isRegLiveAt undef-use)
- #200 (SPILL_GR16 operand count)
- #209 (PUSH_AF stack reserve undef)
- #210 x3 (prologue PUSH_HL undef, SP-relative PUSH_AF, per-regunit HL liveness)
- #212 x2 (ADD IY peephole HL live, GR8 reload HL kill)
- #236 (spill HL isKill unreliable)
- #237 (ADD_HL_FI implicit-def)
- #239 x7 (partial-undef HL PUSH sites)

**Peephole liveness metadata (4):**
- #194 (late-opt liveness)
- #241 x3 (nodbg adjacency walks)

**Meta / drift-guard (2):**
- #240 (inline-runtime pseudo size)
- #267 (pseudo-size drift for MUL8/DIV8 etc.)

**Backend/emit-side correctness (5):**
- #51 (BSS self-clear via memcpy)
- #105 (LDIR_GUARDED for BC=0)
- #264 (RMW bit-set MCSymbol offset)
- #265 (POP_AF FLAGS in compare-merge)
- z88dk#27 (`.quad` split)

**GISel/frame-index (2):**
- #112 site (LEA_IX_FI IY dest)
- #189 (IY-as-GPR miscompile)

**Legalizer/lowering (1):**
- fcmp-libcall retur (i16)

**Assembler-side (1):**
- ascii-octal bytedrop

## SHAPE-DRIFT (4 filer) — særskilt behandling

- `hl-no-bc-backup.ll`, `issue-97-bc-pingpong-singlebb.ll`, `issue-97a-bc-pingpong-i16-counter.ll` — den forbudte shape produceres ikke længere; testen kan omskrives til at *verificere* den gode shape og un-XFAIL'es. Eller slettes hvis den nye codegen har egen dedikeret test.
- `hl-no-bc-backup.ll` — beskrivelse siger explicit "Un-XFAIL when re-shaping happens or the peephole is revived".

## Foreslået arbejdsrækkefølge

Start med de mindste, mest isolerede metadata-rettelser (én pass, én linje ændring):
1. #200 (SPILL_GR16 operand count) — restaurer 0 placeholder-op
2. #209 (PUSH_AF undef mark) — implicit undef på $a
3. #210 prologue/SP-relative (3 tests) — samme mønster: mark $hl/$a undef gate
4. #197 (isRegLiveAt skip undef) — tilføj undef-check
5. #194 + #241 (peephole liveness) — kill/def-metadata i peepholes
6. #239 sites (7 tests) — per-half-liveness gate
7. #236, #237, #212 — spill gates
8. #240, #267 — drift-guards (typisk kræver getInstSizeInBytes opdatering)
9. #105 (LDIR_GUARDED) — verify expansion
10. #51 (BSS self-clear), #264 (RMW MCSymbol offset), #265 (POP_AF FLAGS), z88dk#27 (.quad split), #189, #112, ascii-octal, fcmp-libcall

Én fix ad gangen: implementer → kør lit → verificér 251+1 pass / 60 XFAIL → vis dig diff + resultat → næste.
