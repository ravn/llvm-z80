# Why autoload fit under 2 KB before PR #40 and is 23 B over after — 2026-09-16

Answers "how was it under 2048 and what PR #40 broke." Evidence from committed
artifacts (no rebuild): baseline = rc700-gensmedet `d092732` (2026-07-12, pre-PR#40,
already has the full ROA327 font); current = HEAD (post-recovery, peephole-clean).

## Headline

| | pre-PR#40 (`d092732`) | current HEAD |
|---|---|---|
| compressed payload (`text_compressed.zx0`) | **1915 B** | **1952 B** (+37) |
| PROM (`prom.clang.bin`) | ~2025 B (**fit** 2 KB) | **2071 B** (23 B over) |
| raw code (sum of function sizes, incl font) | 3406 B | 3445 B (+39) |
| `add hl,sp` (SP-relative frame accesses) | **0** | 2 |

NB: the "1643 B / 405 B free" figure in CLAUDE.md predates the full ROA327 font.
The correct like-for-like pre-PR#40 baseline (with the font) is **1915 B / ~2025 B
PROM**, which fit the 2 KB cap. The recovery has clawed all but **~37 compressed B
/ +39 raw B** back; that residual is the 23-over.

## Two regression classes came in with the 23.1.0 merge (PR #40)

**Class 1 — static-frame went inert (the big one, ~+975 B). RECOVERED.**
`Z80NonReentrant.cpp`'s reachability walk routed inline-asm calls (the ISR-epilogue
`__asm__ volatile("ei")`) through the same "opaque call reaches everything"
escalation as indirect calls, poisoning the `"nonreentrant"` attribute on every
non-static function. `usesStaticFrame()` needs that attribute, so static frames
fired on 0 functions and register-pressured functions built SP-relative frames
(+57.5 % code, 2618 B PROM). Fixed by `147d6f43` (exclude inline-asm from the walk)
+ antigravity's reintroduced peepholes (#326-#330, cross-class BSS spill, carry
roundtrip). `add hl,sp` 128 -> 2. See `analysis-static-frame-regression-2026-09-11.md`.

**Class 2 — regalloc spills across-call loop values instead of keeping them
callee-saved. NOT fully recovered — this is the residual 23-over.**

The +39 residual raw B is concentrated in the FDC helpers, dominated by
`_fdc_read_result` **+19 B (36 -> 55)**. Same C source, same error path — a pure
codegen difference:

```
pre-PR#40 (36 B): counter stays in callee-saved DE, cheap push/pop across the call
    ld de,#0 ... push de ; call ... ; pop de ... inc de ; or a ; jr nz   (no frame)

current  (55 B): counter spilled to a dynamic SP-relative frame slot
    push af                          ; frame allocation
    ld hl,#0 ; add hl,sp ; ld (hl),c ; inc hl ; ld (hl),b   ; spill  (7 B)
    call ...
    ld hl,#0 ; add hl,sp ; ld c,(hl) ; inc hl ; ld b,(hl)   ; reload (7 B)
    ... inc sp ; inc sp ; ret
```

Same pattern (a value live across one call inside a loop, spilled to SP instead of
held in DE/BC with push/pop) accounts for the other growers:
`_floppy_legacy_boot +13`, `_lookup_sectors_and_gap3 +9`, `_check_fdc_result +6`,
`_verify_seek_result +6`, `_fdc_select_drive_cylinder_head +6`. (Several functions
IMPROVED post-recovery: `_fdc_write_full_cmd -8`, `_main_relocated -6`,
`_fdc_write_when_ready -7` — so net is +39, not the per-function sum.)

Note these functions do NOT get a static BSS frame (they were register-resident
pre-PR#40, no `.frame` symbol); the SP frame is a fresh dynamic allocation. So
Class 2 is a **register-allocation/spill-placement** regression, distinct from the
Class-1 static-frame inertness.

## Relation to #331

ravn/llvm-z80#331 (SP-relative spill -> PUSH/POP) was an attempt to paper over
Class 2 *post-RA*: rewrite the 14-byte SP spill/reload back to `push bc`/`pop bc`
(what pre-PR#40 regalloc produced directly). It is PARKED as unsound (drops later
reloads of the same slot; hangs recursion — see
`issue331-sprelative-pushpop-unsound-2026-09-16.md`). On autoload it recovered only
4 compressed B anyway (`_fdc_read_result` alone).

## Sound recovery options (fix at the source layer, not a post-RA band-aid)

1. **Regalloc: prefer a callee-saved register (DE/BC) + push/pop for a value live
   across a single call in a loop**, restoring the pre-PR#40 shape directly. This
   is the root layer and would fix all Class-2 growers at once. Investigate what in
   23.1.0 changed the spill-vs-callee-saved decision here.
2. **Extend static-frame coverage** so these FDC helpers place the spill in a fixed
   BSS `.frame` slot (3-4 B/access) instead of a dynamic SP frame — cheaper than the
   SP form, though still worse than option 1's register-resident push/pop.
3. **A sound #331** (single-reader-guarded push/pop conversion) — narrowest, only
   recovers the exact `_fdc_read_result` shape (~4 compressed B); does not address
   the class.

Recommended: option 1 (biggest, root-layer, recovers the whole class). Measure each
against the pre-PR#40 per-function sizes above; boot-gate every change in MAME.

## Update 2026-09-17: option 1 investigated + first attempt falsified

Framing correction: this is not an upstream regression -- it is fork-local code
that did not survive our PR#40 merge. The many "Restore ... after PR #40"
commits (BSS-spill->PUSH/POP, cross-block/cross-MBB, JP->JR, tail-call,
direct 8-bit BSS addressing, ...) reconstruct one piece at a time; Class 2's
SP-frame-spill-around-call is one of the pieces **not yet restored**.

### Fase 1 (root-cause investigation) — key findings

- `Z80InstrInfo::storeRegToStackSlot` / `loadRegFromStackSlot` are UNCHANGED
  pre-PR#40 vs HEAD (only added a `SPILL_ANY16` variant). Frame-slot lowering
  is intact -- not the source.
- `Z80RegisterInfo.cpp` lost `z80IsIYAllocatable` (was gated on
  `optSize + staticStack` -> now returns unconditionally `false`) and the
  `z80-preserves-regs` attribute machinery. Both are pre-PR#40 fork-local
  code that did not come back after the merge.
- `Z80LateOptimization.cpp` (6490 lines pre-PR#40) -> `Z80PreEmitPeephole.cpp`
  (5587 lines current). ~900 lines' worth of peepholes not yet restored:
  IX constant propagation + unused IX/IY setup removal, tail-call CALL->JP
  (pre + cross-MBB), LDIR aftermath DE reuse, u8 switch range narrowing,
  SBC A,A identity-mask roundtrip, ADD A,1->INC r, IX/IY transfer elimination
  (Forms 1/2), bare BSS store + 4-instr A-preserving reload, ADD HL,rr
  commutativity, dead HL copy in pre-compare narrowed loop, and others.
  Some pieces ARE restored (BSS spill->PUSH/POP incl. cross-block/cross-MBB,
  cross-class, JP->JR, several ISel folds).

### Fase 2a (IY-unreserve attempt) — HYPOTHESIS FALSIFIED

Reinstated pre-PR#40's `z80IsIYAllocatable` gate (optSize + hasStaticFrame,
plus `-z80-unreserve-iy` bring-up flag) + conditional `Reserved.set(Z80::IY)`.

Measurement on the Class-2 repro (`/tmp/331-pressure.c`, `-Oz +static-frame`):
**identical asm** before/after. RA still emits SP-frame spills, still does not
use IY.

Root of non-effect: `CALL_nn` has `implicit-def $iy` (call clobbers IY). With
only IX in the CSR list, RA sees IY as caller-saved -- any value in IY across
a CALL still needs spill/reload, so RA correctly declines to use IY over
BC/DE/HL. Un-reservation alone does not restore pre-PR#40 behaviour; there
was some OTHER mechanism in the pre-PR#40 tree that emitted `push de; call;
pop de` for this shape.

Additional cost: un-reservation re-introduced the parked #189 byte-decompose
verifier crash on 2 lit tests (`GR16NoIR.sub_hi cannot be used for GR8
operands` on `issue-189-iy-bcde-widen-leak.ll` and `issue-332-oz-stackarg-
offset.ll`) plus 2 codegen-shape regressions (`iy-late-opt-missing-uses-
miscompile.ll`, `static-stack-addr-taken-spill-195.ll`). `Z80NarrowNoIndex`
does not cover all paths in the current tree, unlike pre-PR#40.

Change reverted. Working tree clean.

### Revised recovery plan

**Fase 2b (recommended next):** restore the missing "SP-frame-spill around
CALL -> PUSH/POP" fork-local peephole. This is the same conversion #331
tried before and parked as unsound; the sound version needs a strict
single-reader guard so it fires only when the spill has exactly one reload
and no other SP-touch/aliasing between them (Ackermann's multi-reload shape
must remain safe). Bounded 1-session try with clear go/no-go on lit +
`test_22_recursion.c`.

**Fase 2c (if 2b insufficient):** investigate what LLVM 22->23.1.0 changed
in generic RegAllocGreedy's split-around-call heuristic, and either restore
the pre-PR#40 spill-weight tuning or add a Z80-specific hook. 2-3 sessions,
possible upstream-fork patch.

**Not on the path right now:** un-reserving IY. Blocked by the byte-decompose
safety machinery (`Z80NarrowNoIndex` coverage + IX/IY sub-register handling)
that itself is pre-PR#40 fork-local code we haven't fully restored.

## Update 2026-09-17 (later): Fase 2b landed, Fase 2c falsified, #331 closed

**Fase 2b (peephole)** landed in `7c363405d297` -- sound SP-frame-spill ->
PUSH/POP with strict single-reader guard.  autoload PROM 2120 -> 2111 B
(9 B saved raw), 918 runtime PASS, test_22_recursion PASS all opts, MAME
floppy-boot-test PASS.

**Fase 2c (restore `getRegAllocationHints`) -- FALSIFIED.**  Reinstating the
pre-PR#40 hook in upstream style (all three strategies: GR16_BCDE op-hint,
DEC_rr self-loop counter hint, DJNZ innermost-loop hint) *lost* 5 B on
autoload (2111 -> 2116) and broke 21 lit tests with CHECK-line shape drift
(no verifier / miscompile).  The GR16_BCDE hint fires broadly and shifts
register pressure globally in ways that spill more than they save on the
Class-2 shape.  Reverted; the peephole alone carries the win.

**Root cause for the original +23-39 B Class-2 loss: NOT a single missing
component.**  Fase 2b recovers a defined subset; the remainder is spread
across shapes no single peephole/hook cleanly targets.  #331 CLOSED
2026-09-17 -- reopen if a production target becomes cap-tight and the
residual matters.
