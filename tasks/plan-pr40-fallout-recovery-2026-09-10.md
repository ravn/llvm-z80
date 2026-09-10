# Plan: PR #40 fallout recovery (2026-09-10)

**Kontekst:** Upstream PR #40 (z88dk calling conventions + llvmz80-23.1.0-r1) blev
merged ind i ravn/llvm-z80. De umiddelbare build-brud blev lappet i commits
`a3f3f6f`–`1d7124d`, men tre legalizer-/API-regressioner er stadig åbne.

## Baseline (målt 2026-09-10, macbook, build-macos)

| Suite | PASS | XFAIL | FAIL | FATAL | SKIP |
|-------|------|-------|------|-------|------|
| Lit (`llvm/test/CodeGen/Z80/`, 285 tests) | 189 | 64 | **32** | — | — |
| Test-runner clang O1 (205 tests) | 86 | — | 0 | **68** | 51 |

Historisk baseline (før PR #40): 164 PASS + 6 XFAIL + 0 FAIL i lit.

z88dk-ticks var ikke i PATH — bygget fra `z88dk/src/ticks/` og symlinket til
`~/.local/bin/z88dk-ticks` (2026-09-10).

## Rodårsager

### R1 — `@llvm.experimental.memset.pattern` ikke legaliseret (størst scope)
- Rammer: 58 test-runner fatals + `experimental-memset-pattern.ll` + `loop-idiom-fill.ll`
- Symptom: `LLVM ERROR: unable to legalize instruction: G_INTRINSIC_W_SIDE_EFFECTS
  intrinsic(@llvm.experimental.memset.pattern)`
- Tidligere implementeret i `design-upstream-memset-pattern-target-hook-2026-06-09.md`
  (commit `6839ebc4bcbf` på en gren der ikke er med i main efter PR #40 merge).
- Sandsynlig årsag: legaliseringsreglen faldt ud ved rebase/merge.

### R2 — Z80 builtins ulegaliserede (`@llvm.z80.im2` m.fl.) — PRODUKTIONSKRITISK
- Rammer: `intrinsics.ll` lit-test FAIL
- Symptom: `LLVM ERROR: unable to legalize instruction: G_INTRINSIC_W_SIDE_EFFECTS
  intrinsic(@llvm.z80.im2)`
- rcbios bruger `__builtin_z80_im2/ei/di/halt/nop` — disse compiler-crashes i dag.
- Sandsynlig årsag: Legalizer-opslag for Z80-specifikke intrinsics brækkede i
  PR #40 merge (API-ændring i intrinsic-ID-opslag?).

### R3 — `-z80-unreserve-iy` flag fjernet/omdøbt
- Rammer: 10 test-runner fatals (test_166–174 + `test_205_reverse_fill_seed`)
- Symptom: `clang (LLVM option parsing): Unknown command line argument '-z80-unreserve-iy'`
- Compileren foreslår `--z80-enable-licm` (forkert). Det korrekte nye flag-navn
  skal søges i `Z80.td` / `Z80Subtarget.h`.

### R4 — i64/i128/arith-i32 "Found 2 machine code errors"
- Rammer: `i64-support.ll`, `i128-support.ll`, `arith-i32.ll` (3 lit-tests)
- Symptom: `LLVM ERROR: Found 2 machine code errors`
- Kan være relateret til PR #40 register-class ændringer; kræver nærmere
  undersøgelse med `-verify-machineinstrs`.

### R5 — Codegen-drift (26 lit-tests)
- FileCheck-patterns er forældet efter PR #40 ændrede instruktionsvalg.
- Korrekthedsproblem er usandsynligt; tests skal opdateres til ny output.
- Gøres SIDST så rækkefølgen ikke skjuler ægte regressioner.

## Handlingsplan

### Trin 1 — Find nyt IY-unreserve flag (R3) [hurtig, ~30 min]
```
grep -r "unreserve.iy\|UnreserveIY\|reserveIY\|ReserveIY" \
  llvm/lib/Target/Z80/ llvm/include/llvm/Target/Z80/
```
Opdater `EXTRA-FLAGS` i test_166–174 + `test_205_reverse_fill_seed`.
Verifikation: test-runner fatals for IY-tests forsvinder.

### Trin 2 — Gendan Z80 builtins legalisering (R2) [kritisk]
- Kig i `Z80LegalizerInfo.cpp` efter `z80.im2` / `z80.ei` / `z80.di` osv.
- Sammenlign med `git log --all --oneline -- llvm/lib/Target/Z80/Z80LegalizerInfo.cpp`
  for at finde seneste fungerende commit.
- Verifikation: `intrinsics.ll` PASS + rcbios smoke-kompilering.

### Trin 3 — Gendan `@llvm.experimental.memset.pattern` legalisering (R1)
- Se `design-upstream-memset-pattern-target-hook-2026-06-09.md` for den tidligere
  implementering (commit `6839ebc4bcbf`).
- `git show 6839ebc4bcbf -- llvm/lib/Target/Z80/Z80LegalizerInfo.cpp` for at
  sammenligne.
- Verifikation: test-runner fatals fra memset.pattern falder fra 58 til 0.

### Trin 4 — Undersøg i64/i128/arith-i32 machine code errors (R4)
```
llc -verify-machineinstrs -mtriple=z80 llvm/test/CodeGen/Z80/i64-support.ll 2>&1 | head -30
```
Bestem om det er regressor fra PR #40 register-class ændringer.

### Trin 5 — Opdater 26 codegen-drift tests (R5)
```
build-macos/bin/llvm-lit llvm/test/CodeGen/Z80/ \
  --filter "djnz|sat-arith|tail-call|..." --update-check-files
```
Eller manuelt for at sikre vi ikke skjuler regressioner.

## Målstate
- Lit: 0 FAIL (189+ PASS, 64 XFAIL tilladt)
- Test-runner clang O1: >150 PASS, 0 FATAL (ekskl. kendte SKIP)
- rcbios kompilerer uden backend crash
