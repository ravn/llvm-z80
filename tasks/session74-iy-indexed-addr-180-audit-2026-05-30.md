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

## State at session end
- `ravn/llvm-z80` main: `94d83af`, CI green, clean.
- Branches: `z80-27-iy-indexed-addr` (merged), `z80-loop-carrier-areg-pin`
  (#172 negative result, unmerged).
- No production size change (cpnos PROM1 2022 B unchanged; BIOS untouched).
