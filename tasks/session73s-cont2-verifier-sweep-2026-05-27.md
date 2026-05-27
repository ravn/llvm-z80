# Session 73s (cont-2) — Cluster 3 verifier-surface sweep (#197) — 2026-05-27

Continued the issue-closeout Cluster 3 (verifier/#197).  Fixed **4 root-cause
classes** of the `-O2 -verify-machineinstrs` red surface, each fully gated.
All four committed to `main` (NOT pushed).

## Closed / fixed this session

| # | what | commit | byte impact |
|---|------|--------|-------------|
| #200 | SPILL/RELOAD FI pseudos kept 2-operand form after PEI -> restore folded `$offset` as 0 placeholder | `47db108` | neutral |
| #194 | 2 Z80LateOptimization peepholes left stale `$a` liveness (cross-block `LD A,r` removal -> addLiveIn; `LD A,#0`->`XOR A` -> undef use) | `46fbafb` | neutral |
| #209 | stack-reservation `PUSH AF` reads don't-care `$a`/`$flags` (frame lowering) -> mark undef | `99ee190` | neutral |
| #209 | `EX DE,HL` one-way-copy dest read is don't-care (`copyPhysReg`) -> mark dest use undef | `8ff4208` | **cpnos 2028->2022 B (-6 B)** |

#200 **CLOSED on GitHub**.  #209 **filed** (covers the PUSH AF + EX DE,HL
don't-care-read pair).  #194 fixed (its exact scope — late-opt-introduced —
is closed; can close on GitHub).

Lit: **123+5 -> 127+5** (4 new tests: issue-200, issue-194, issue-209 ×2).
Value oracle on every commit: test-runner `-diff-opt -native-oracle` default
799/0 + static-stack 793/0 (no divergences) throughout; cpnos byte-identical
for the 3 neutral fixes; for the EX DE,HL win cpnos polypascal-test PASS
(50.65 s) + AES enc/dec PASS (ts 11516046 unchanged).

## The systemic picture (zoom-out)

The #197 red surface is **two families**, not isolated bugs:

1. **Stale-def liveness** — a peephole removes/moves a physreg def without
   updating live-ins.  (#194 cross-block `LD A,r` removal.)  Fix: `addLiveIn`.
2. **Don't-care reads** — an instruction nominally reads a physreg whose value
   is irrelevant, emitted without `undef`.  Members found: `XOR A` (zeroing,
   late-opt), `PUSH AF` (stack reserve, frame lowering), `EX DE,HL` one-way
   copy (`copyPhysReg`).  Fix: `setIsUndef` on the don't-care use.

## REMAINING surface — pick up here

After the 4 fixes, `clang -O2 -ffreestanding -mllvm -verify-machineinstrs -c
aes256.c` still fails (verifier aborts at first function):

- **`aes_mixColumns`: `PUSH_HL implicit $hl` reads undef `$hl`.**  THIRD
  family, *not* a clean don't-care read.  **ROOT-CAUSED + filed as #210**;
  deliberately NOT fixed (non-trivial + high-risk frame-lowering, non-production
  codepath).  **Confirmed root cause:** SP-relative frame spill borrows HL
  (`PUSH_HL; LD HL,nn; ADD HL,SP; LD (HL),r; POP_HL`) gated by
  `NeedSaveHL = isRegLiveAt(Z80::HL, ...)`; that forward scan **over-reports HL
  live** — it hits the next SP-relative access (which references HL but
  *redefines* it before use) and concludes HL is live, cascading so every spill
  saves a dead HL and reads it undef.  Confirmed in bb.8 (`liveins: $a`, HL
  killed by a preceding `LOAD8_IND` then read by `PUSH_HL`).  **Verifier-metadata
  only / runtime-correct** (diff-oracle green; AES byte-correct).  **Does NOT
  affect production** — cpnos/BIOS/autoload all `+static-stack` (BSS direct
  addressing, no SP-relative HL borrow).  Fix needs robust HL-liveness (precompute
  per-position over the original block, or a provably-dead scratch) + full MAME
  oracle; a wrong HL-dead call skips a needed save -> miscompile.  Earlier
  findings (kept for context):
  - It is an **SP-relative frame-access bracket** that borrows HL for the
    address: `PUSH_HL; LD HL,nn; ADD HL,SP; LD (HL),r; [INC HL; LD (HL),r;]
    POP_HL` (the dump shows e.g. spilling `$e` at SP+7).  `aes_mixColumns` has
    `framePointerPolicy: none` and is NOT `+static-stack`, so every frame slot
    is reached SP-relative, borrowing HL.
  - The bracket appears **after ExpandPostRAPseudos** (`-print-after=postrapseudos`
    shows the PUSH_HL; `-stop-after=prologepilog` still shows the unexpanded
    `SPILL_GR8/16`).  So the emission is in `Z80InstrInfo::expandPostRAPseudo`
    (or a helper it calls) for the no-FP SP-relative case — NOT the
    `eliminateFrameIndex` SP-relative helpers in `Z80RegisterInfo.cpp` I first
    suspected.  (Caveat: the SPILL_GR8 case I read at Z80InstrInfo.cpp:950
    emits `LD (IX+d),r`, which doesn't match the observed SP-relative PUSH_HL
    bracket — so the exact path is still unconfirmed; reconcile the pass
    boundary first.)
  - Root mechanism: the PUSH_HL saves HL purely to borrow it as the
    address-scratch register, guarded by a liveness check.  The verifier sees
    HL undef at the PUSH, so either the guard **over-saves** (saves HL when it
    is actually dead -> reads undef; fixing it = elide the PUSH = a size win)
    or the block live-ins are **stale**.
  - **UPDATE 2026-05-27 (cont-2, FULL fix designed): #210 is TWO root causes;
    the complete fix is specified on the issue.**  Cause A (cascade
    over-report) is fixable by sound per-unit liveness + PUSH/POP bracket
    transparency in `isRegLiveAt` (prototyped: removed 8/48 unnecessary borrow
    PUSH_HLs in `aes_mixColumns`, oracle-clean, cpnos byte-identical; reverted
    as insufficient alone).  **Cause B is the real blocker: a partial-undef-pair
    borrow** — the surviving `PUSH_HL` is a *correct* save (`$h` live across it)
    but reads the whole pair while `$l` is undef.  Fix B: insert
    `$<deadhalf> = IMPLICIT_DEF` (zero code) before the borrow PUSH for any dead
    half, relying on Cause-A's sound per-unit liveness to identify it (an
    IMPLICIT_DEF on a *live* half would corrupt it → the soundness hinge).
    Complete fix = A + B, full oracle + MAME, focused session.  Also clears the
    bench `PUSH_AF` (`$a`-dead/flags-live) analogue.  Three reverted attempts
    this session proved the partial fixes sound-but-insufficient; the issue now
    carries the full implementable design.  See #210.
  - (earlier framing, superseded:)  Tried two
    sound, oracle-clean (799/0), production-byte-identical `isRegLiveAt`
    refinements (sub-register-redef-aware successor fallback; then full per-unit
    forward liveness keeping ADJCALLSTACK handling) — BOTH insufficient; reverted.
    Why: PEI `replaceFrameIndices` walks the block **backward**, so a spill's
    later siblings are already expanded into SP-relative borrow brackets
    (`PUSH_HL; LD HL,nn; ADD HL,SP; LD r,(HL); POP_HL`) when the earlier spill
    queries liveness.  The borrow `PUSH_HL` is a **genuine `$hl` use** (before
    its in-bracket `LD HL,nn` redef), so any forward scan correctly sees HL
    "used" → save → its own borrow PUSH → cascade.  Fixing `isRegLiveAt` can't
    help.  **Real fix:** make the borrow bracket liveness-transparent — mark the
    borrow `PUSH_HL`'s `$hl` use `undef` at emission (guarded for the genuinely-
    HL-live case to avoid DCE dropping HL's def), or precompute HL liveness over
    the pre-expansion stream.  Full root cause on #210.  Needs the full value
    oracle + MAME; non-production (`+static-stack` never hits this path).
- Whatever surfaces after that (the verifier aborts at the first function, so
  the full remaining count is unknown until `aes_mixColumns` clears).

## Also closed (Cluster 6 tooling, zero codegen risk)
- **#124** CLOSED — `set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "")` in
  `clang/cmake/caches/Z80.cmake`.  The `third-party/benchmark`
  HAVE_PTHREAD_AFFINITY probe fails on macOS and cmake 4.2 (CLion) treats it as
  fatal, breaking fresh configures / bisect-walks; baked in the proven
  workaround.  Verified: fresh configure with cmake 4.2.2 exits 0.

## Production-target check (this session, safe measurements)
- **BIOS clang = 5897 B — UNCHANGED** (was 5897).  The 4 fixes are byte-neutral
  for the BIOS (no EX DE,HL DCE opportunity there); no regression, no re-boot
  needed (byte-identical to last-verified).
- **cpnos PROM1 = 2022 B** (was 2028; -6 B from the EX DE,HL fix), polypascal PASS.
- **AES** enc/dec PASS, size + ts unchanged.

Plus the known **#112/#189** illegal-vreg class (GR16NoIR) — the first of the
three #197 classes named in #200's text — is gated separately (IY-unreserve
cost model).

## #197 status
3 of the originally-named classes (#200, #194, and the PUSH AF/EX DE,HL
don't-care reads) cleared.  #197 (flip `-verify` to a blocking CI lane) still
gated on: the `aes_mixColumns` PUSH_HL liveness-reconciliation class above +
#112/#189.  Once those clear, wire `-verify-machineinstrs` into a CI lane.

---

## #210 RESOLVED 2026-05-27 (cont-3) — root cause was MISDIAGNOSED above

The "backward-PEI cascade" theory in the #210 section above is **WRONG**.
Z80 sets `eliminateFrameIndicesBackwards() = false` (Z80RegisterInfo.h:55), so
PEI eliminates frame indices **FORWARD** — when a spill is expanded, the
instructions after it are still pristine pseudos (no borrow brackets exist
ahead of the `isRegLiveAt` forward scan).  There is no cascade.  Reading the
actual post-PEI MIR of `aes_mixColumns`/`aes_mc_inv` showed three real defects:

1. **`isRegLiveAt(HL)` over-reported.**  It concluded "dead" only on a single
   *full* HL def; when HL's halves are redefined by *separate* later defs
   (`$h = ...`; `$l = ...`) it never saw a full def and fell through to the
   successor live-ins of the *new* halves → reported the stale pair live.
   Fix: track liveness per register **unit** (a def retires the units it
   covers; only still-pending units fall back to successor live-ins).
2. **GR8 half spill/reload forced the HL save** (`NeedSaveHL = SrcIsHL || …`).
   Fix: save HL only when the *other* half is live; IMPLICIT_DEF a dead
   destination half when the pair must still be saved.
3. **`emitSPRelativeAddr` flag-preserving `PUSH_AF`** (#209 family): A is not
   modified in the PUSH_AF/POP_AF bracket (the save carries FLAGS over
   ADD_HL_SP), so when A is dead its read is don't-care → mark `$a` undef
   (the live `$flags` read keeps the pair from DCE).

Commit `4feddfec517e` (llvm-z80 main).  Default-config-only; production
`+static-stack` byte-identical.  aes256.c verify 0 errors (was ~24); `-verify
-full` suite A/B 757→753 fatal (−4, no new failures); diff-oracle default
799/0/50/207 + `+static-stack` 793/0/50/213 (exact baselines, 0 divergences);
AES 13/13 PASS; cpnos PROM1 2022 B + polypascal PASS 50.97 s; lit 127+5 → 129+5
(two new MIR tests).  **Methodology note:** the verifier-only A/B and reading
MIR (not trusting the filed theory) was what corrected the diagnosis —
`feedback_state_certainty` + `feedback_audit_oracle_not_just_fix`.

**#197 still open:** the broad O0 `ADD_HL_HL`/`DJNZ`/`PUSH` undef surface across
many functions remains (753 fatal under `-verify -full`); this session cleared
only the aes frame-spill class.
