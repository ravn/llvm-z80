# Session 74 — #27 IX/IY-indexed addressing shipped + #180 re-audit (2026-05-30)

Recovered from a crashed session (ninja died mid-build), then advanced three
threads to clean conclusions.  All work on `ravn/llvm-z80` `main` (CI green) or
documented; no production codegen change (the one feature is flag-gated OFF).

## 1. Crash recovery
The prior session died with "ninja crashing" + an API 400.  Root cause: `ninja`
is not on PATH — it lives in CLion's bundle at
`/Applications/CLion.app/Contents/bin/ninja/mac/aarch64/ninja` (cmake's
`CMAKE_MAKE_PROGRAM`).  Disk was at 97 % but incremental builds fit.  Rebuilt
clang; the in-flight `Z80PinAluAccumulator` rewrite was intact.

## 2. #172 A-pin accumulator — PARKED (negative result)
The in-flight rewrite was the 3rd approach to #172 (connected-component
loop-carrier pin).  Measured: first *correct* one (parallel-accumulator + call
gates → AES 13/13 PASS, no segfault/miscompile) but still a **wash** (+3 B /
+100–532 ts).  Root cause: A is the universal contended 8-bit resource; pinning
the carrier just relocates the shuttle (conservation).  Five approaches now all
net-negative.  Close-out comment posted on #172; committed on branch
`z80-loop-carrier-areg-pin` (`76125a9`, not merged — negative result).

## 3. #27 IX/IY-displacement addressing — SHIPPED (flag-gated)
Drill found #27 is **not** a copy-cost-model problem — it's a phase-ordering
gap: under `-Oz +static-stack` (production model, IY allocatable) regalloc parks
pointers in IY but ISel already chose the `add hl,bc; ld (hl)` deref form, so
every access pays `push iy; pop hl; add; (hl)` instead of `ld r,(iy+d)`.

Built the deferred-addressing pseudo (approach B), merged to main (`fbff23e`,
`--no-ff`, CI green):
- `LOAD_IDX8` / `STORE_IDX8` pseudos (base constrained to `IR16` → lands in
  IX/IY), expanded in `Z80ExpandPseudo` to `LD r,(IX/IY+d)` / `LD (IX/IY+d),r`
  (with the index reg added as an explicit implicit use — the indexed defs
  don't model it).
- Two gates: **call-free** (correctness — IY is caller-saved, `Z80_CSR`=IX only;
  a base forced to IY can't cross a call) and **≥2 const-offset sites**
  (profitability — the one-time `push hl;pop iy` setup must amortise; counting
  *sites* not mem-users is selection-order-stable).
- Flag `-mllvm -z80-idx-addr`, **default OFF** → production byte-identical.
- lit test `issue-27-iy-indexed-addr.ll`; lit suite **139 + 4**.

Measured (flag ON): AES prod config **2190 → 2054 B (−136 B / −6.2 %)**,
t-states −0.11 %, enc+dec PASS (kr+ansi).

**Production verdict (key):** cpnos PROM1 with the flag = **2022 B,
byte-identical** — the feature fires on zero cpnos functions (call-heavy) and
the cpnos payload has only 6 `push ix/iy` (vs 64 in AES): cpnos/BIOS lack the
multi-site-pointer-deref pattern (they use direct BSS addressing).  So the win
is intrinsic to array/crypto code; **Stage 3 (cross-call) is NOT worth building**
(nothing to capture).  Kept flag-gated for array workloads.

Design + measurements: `tasks/issue27-iy-indexed-addr-design-2026-05-30.md`.

## 4. #180 peephole-audit re-audit — tracker is ~half stale
`tasks/issue180-drill-2026-05-30.md` (pushed `94d83af`).  Criterion: a peephole
can move to a pre-RA combiner/ISel pattern only if it doesn't depend on post-RA
info (physreg/FLAGS liveness).  Findings:
- **7 of the 16 "migrate" peepholes are already gone** (#2,#6,#9,#11,#15,#23,#24
  — sessions 73q/73s + #6 migrated to ISel).
- **~5 of the remainder are correctly Keep** (post-RA-liveness-gated: #7 FLAGS,
  #13 H/L-dead [self-corrected], #20 physreg, #12/#18/#21).
- **Only ~3–5 are genuine** (#8,#10,#17,#19,#25), and each is a pre-RA
  *infra build* (ISel pseudo/combiner — the #6/#27 model), not a port; all are
  load-bearing (removing #8 = +18 B AES, #10 = +4 B cpnos), and produce **no
  codegen win** (the peephole already optimises) — pure upstream cleanliness.

Net: the "~2300 LOC stand-in" alarm is largely resolved; realistic remaining
migration ≈ 600 LOC / ~3–5 focused sessions, gated on #177/#178/#179.

## 5. Continuation (same session) — exhausted the remaining levers

After merging #27 and re-auditing #180, drilled the two candidate next-levers to
honest negative/marginal conclusions:

- **#211 (#8 `A-via-(HL)` migration) — NOT worth building.**  Investigated:
  zero codegen win (peephole #8 already captures every winnable A-dead case; the
  A-live case can't be improved), plus ISA-split (`LD r,(BC/DE)` doesn't exist)
  + clobber-tension.  Commented WONTFIX-ish on #211; deprioritised the #180
  genuine migrations generally (cleanliness-only).
- **Production-density regalloc — TAPPED OUT.**  Built BIOS (5897 B, beats SDCC
  6091) + cpnos; instrumented.  Dominant waste is ISA-fundamental: 324 BSS-via-A
  (8-bit memory is A-only), ~245 A-shuttle moves (irreducible #172 class), ~65
  pair-copies; **zero IX-stash** in BIOS; cpnos near-optimal; ~0 recoverable
  redundancy (the 11 "redundant reloads" are A-shuttle restores).  #178 remat
  targets IX-stash/spill-across-CALL which the production targets don't have
  (it's an AES lever).  Commented #178; full note
  `tasks/production-density-regalloc-drill-2026-05-30.md`.

**Strategic upshot:** clang beats SDCC on all production targets; the cheap and
the regalloc levers are exhausted.  The only high-value remaining engineering is
**upstream-submission packaging** (both Tier A gates #180/#181 now resolved).
U-LLVM queue tracked in #186; Z80-backend packaging per `tasks/execution-plan-2026-05-22.md`.

## State at session end
- `ravn/llvm-z80` main: `ab1065c`, CI green, clean working tree.
- Branches: `z80-27-iy-indexed-addr` (merged), `z80-loop-carrier-areg-pin`
  (#172 negative result, unmerged); `z80-178-remat-drill` merged (docs) + deleted.
- No production size change (cpnos PROM1 2022 B unchanged; BIOS 5897 B untouched).
- GitHub: #211 filed+commented (WONTFIX-ish); #27/#180/#178 commented.
- Entry point for next session: `tasks/NEXT-SESSION-2026-05-30.md` (top lever:
  upstream packaging; production-density marked tapped-out).
