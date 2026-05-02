# Session 37 — Phase 1 foundation + Phase 2 first two correctness items

Date: 2026-05-02 (immediate continuation of session 36).
Branch: `session-37-phase-1-foundation` (off `main`).

## TL;DR

Roadmap approved.  Phase 1 (Foundation) deliverables landed.  Phase 2
correctness sweep made progress on the first two of five items: #81
fixed, #36 verified-already-fixed and closed.  Two new issues filed
(#102, #103) capturing janitorial follow-ups identified during the
work.

End-of-session sizes: BIOS **5920 B** byte-exact vs session 36;
cpnos-rom payload 1759 B / relocator 128 B byte-exact.  Lit suite:
**78 PASS + 1 XFAIL** (was 77 + 1 — net +1 from the new
`issue-81-ex-af-prime.ll`).

## Roadmap state

- Phase 0 (research + planning) — complete (session 36).
- **Phase 1 (Foundation) — complete this session.**
- **Phase 2 (Correctness sweep) — 2 of 5 items closed.**
  - #81 closed by codegen fix.
  - #36 closed by verification (no codegen change needed).
  - #28 + #63 + #38 remaining.

## Commits (this branch, oldest first)

1. `4142853d` — `test/CodeGen/Z80: extend vararg.ll, document #36
   verification` (pushed to origin; referenced from #36's close).
2. `9940e7c4` — `[Z80] add missing override markers on TTI (#101)`.
3. `95d2cd71` — `[Z80] AsmParser: accept "ex af, af'" inline asm (#81)`.
4. `c97738016ccc` — `Phase 1 infra: Z80 CI workflow + per-function
   size baseline tracker`.
5. `1296275b7acb` — `tasks: Phase 1 audit deliverables + master-plan
   pointer`.

Only commit (1) is pushed to origin so far.  Commits (2)–(5) are
local on `session-37-phase-1-foundation`, awaiting an explicit user
decision to push.

## Phase 1 deliverables

### #101 — `Z80TargetTransformInfo.h` `override` markers (closed)

Two-line fix.  Build clean, lit unchanged, sizes byte-exact.  Closes
the only open item that came out of session 36's upstream sync.

### `.github/workflows/z80-ci.yml`

Path-filtered to `llvm/lib/Target/Z80/**`, `llvm/test/CodeGen/Z80/**`,
`llvm/test/MC/Z80/**`, `clang/lib/Driver/ToolChains/SDCC.cpp`,
`clang/cmake/caches/Z80.cmake`, and the workflow file itself.  Builds
clang+llc with ccache (key `z80-ci-${runner.os}`, 2 GB cap) and runs
the Z80 lit suite.  Workspace-mode for now; a promotion candidate
for `llvm-z80/llvm-z80` once engagement mode opens.

### `tasks/size-baseline.py` + `tasks/size-baseline.json`

`record` walks the BIOS / cpnos-rom payload / cpnos-rom relocator
ELFs and writes per-function code-symbol sizes (via `llvm-nm
--print-size --size-sort`, T/t symbols only) to JSON.  `check`
diffs the current build against the baseline and exits non-zero on
regression.  Baseline locked at the session-36 sizes:

```
rcbios-in-c/clang/bios.elf:  5107 B  (98 fns)
cpnos-rom/clang/payload.elf: 1759 B  (132 fns)
cpnos-rom/clang/relocator.elf: 128 B  (4 fns)
```

(Sums of T/t symbols only; the BIOS ELF total of 5920 B includes
section padding and pure-data sections not in 't'.)

### `tasks/late-opt-audit-2026-05-02.md`

Per-peephole keep / migrate / delete classification of
`Z80LateOptimization.cpp` (5272 LOC).  37 distinct patterns; 18
keep / 16 migrate / 3 delete.  Phase 8 ceiling estimate: ~2450 LOC
removed (~46%).

Findings worth highlighting:

- The `#if 0`-disabled EXX shadow-register-conversion block (lines
  ~4036–4192, ~150 LOC) is unsalvageable — `EXX` swaps all three of
  BC/DE/HL atomically and cannot be inserted between live values.
  → filed as **ravn/llvm-z80#102**.
- The SM83-specific peepholes cluster heavily (lines 2900–3920, ~6
  patterns, ~900 LOC).  Worth considering a separate SM83-only
  late-opt pass guarded by `STI.hasSM83()` if the SM83 backend keeps
  growing.  Logged in the audit; no issue filed yet.
- Patterns #26 (mask-roundtrip after SBC A,A) and #27/#28 (carry
  roundtrip) become delete-safe after the GISel combiner patches for
  #79 and #93 land.  Tightly coupled — delete as a unit.

### `tasks/source-cleanup-vs-closed-issues.md` verification append

13 closed issues cross-checked against the current backend; only the
two cpnos-rom #95-blocked workarounds remain as outstanding source-
side workarounds.  Caveat preserved: this was a targeted spot-check,
not an exhaustive grep across all sources; deeper sweep deferred.

### `tasks/fix-plan.md` master-plan pointer

Banner at the top redirects readers to
`tasks/roadmap-to-maturity.md` as the master plan; `fix-plan.md`
remains the per-cluster engineering doc.

## Phase 2 deliverables

### #81 — `ex af, af'` inline-asm parsing (closed by fix)

The Z80 ISA's only register-position operand that uses an apostrophe
is the shadow accumulator-flags register `AF'` in `EX AF, AF'`.  The
AsmLexer treats `'` as a single-quote string opener, so `af'\n...`
was mis-tokenised as identifier `af` plus an unterminated string.
The only workaround was `.byte 0x08`.

**Fix** (`Z80AsmParser.cpp`, ~30 LOC + 2 includes):
in `tryParseRegisterOperand`, when the current token is identifier
`af` (case-insensitive) and the very next byte in the source buffer
is `'`, push a `Z80Operand::CreateToken("af'", ...)` operand
(matching the auto-generated `MCK_af_39_` class for `EX_AF_AF`'s
second operand) and reposition the AsmLexer past the apostrophe via
`setBuffer` so the caller's `Parser.Lex()` does not invoke
`LexSingleQuote`.

Scope kept narrow: only `af'` (bc'/de'/hl' are not standalone Z80
operands; `EXX` swaps all three implicitly).  Lit test
`issue-81-ex-af-prime.ll` covers four shapes (lower / upper /
no-space / followed-by-other-instructions) at -O0 and -O2.

### #36 — `va_arg` correctness (closed by verification)

Verified fixed on trunk.  No backend change.

- Codegen: existing `vararg.ll` lit test passes; this commit extends
  it with an `-O2` RUN line and records the verification in the file
  header.
- Runtime: `test_25_vararg.c` returns `DE = 0x00FF` (all 8 sub-tests
  pass) at every opt level (-O0/O1/O2/O3/Os/Oz) when linked with the
  ELF crt0 from `compiler-rt/lib/builtins/z80/crt0.asm` and run under
  `z88dk-ticks`.

The original "v always 0" symptom did not reproduce; consistent with
the 2026-04-07 follow-up comments, it was almost certainly downstream
of `va_arg` itself (a stub `putchar` returning zero, or a SDCC-built
`printf` called with the wrong calling convention).

GitHub closed via the `4142853d` commit reference.

## Issues filed this session

- **#101** — file by session 36; closed this session.
- **#102** — Remove disabled EXX shadow-register-conversion block in
  `Z80LateOptimization.cpp` (~150 LOC dead code).  Pure janitorial.
- **#103** — z80-utils test runner can't find `_halt` because the
  clang link path lacks crt0 + linker script.  Discovered while
  verifying #36 — manual link with `compiler-rt/lib/builtins/z80/
  crt0.asm` + `z80.ld` works.  Fix sketch included in the issue body.

## Issue inventory at end of session 37

Open: 28 - 2 (closed #81, #36) + 2 (filed #102, #103) = **28 open**.
(#101 closed in commit `9940e7c4`; will sync to GitHub on push.)

## Verification gates passed

- `ninja -C build-macos clang llc` — clean (only pre-existing
  -Wunused-but-set-variable warnings in `Z80LateOptimization.cpp`,
  unrelated to this session's changes).
- `llvm-lit llvm/test/CodeGen/Z80/` — 78 PASS + 1 XFAIL.
- `make -C rc700-gensmedet/rcbios-in-c bios` — BIOS = 5920 B.
- `python3 tasks/size-baseline.py check` — zero per-function deltas
  vs locked baseline.
- Manual #81 repro — emits `ex af,af'` and `exx` cleanly.
- Manual #36 repro — DE=0x00FF at all six opt levels.

## What this session did NOT do

- Did not push commits 2–5 to origin.  Awaiting explicit user
  authorisation; #36 commit (1) was pushed because closing the issue
  required a visible reference.
- Did not start on #28+#63+#38 (Phase 2 items 3–5).
- Did not attempt the EXX cleanup (#102) or runner fix (#103) —
  filed as follow-up tasks and issues.
- Did not modify any source-side workarounds in rcbios / cpnos-rom
  (out of scope per roadmap §16).

## Pickup for session 38

1. Decide whether to push commits 2–5 of this branch.  If yes,
   `git push origin session-37-phase-1-foundation` is enough; the
   branch already tracks origin after the #36 push.
2. Phase 2 next: **#28 + #63** investigation.  Roadmap suggests they
   share a FastRegAlloc / spill-slot root cause at -O0; design a
   shared fix before coding.
3. Optional fold-ins:
   - **#102** is a 5-minute janitorial PR (delete a `#if 0` block).
   - **#103** unblocks the runtime test suite for future correctness
     verification (helps Phase 2 efficiency materially).
4. After all of Phase 2 lands: optional merge of
   `session-37-phase-1-foundation` into `main` with `--no-ff` per
   the project's session-merge convention.
