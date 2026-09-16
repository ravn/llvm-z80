# ravn/llvm-z80#331 — SP-relative-frame -> PUSH/POP peephole is unsound (multi-reload)

**Date:** 2026-09-16
**Verdict:** the attempted implementation of #331 is a **miscompile** (silent, hangs
recursion). The *optimization idea* (#331) is fine; the naive peephole is not.
Parked, not fixed. This is a bug report per `feedback_file_bugs_not_fixes` — no
fix is proposed here.

## Context

#331 asks for: in non-static-frame functions, a 16-bit callee-saved scratch value
spilled across a call via a dynamic SP-relative slot

```
ld hl,#0; add hl,sp; ld (hl),c; inc hl; ld (hl),b   ; spill BC to (SP+0)   7 B
...
ld hl,#0; add hl,sp; ld c,(hl); inc hl; ld b,(hl)   ; reload BC from (SP+0) 7 B
```

should instead use `push bc` / `pop bc` (2 B total, saving 12 B).

An implementation was drafted in `Z80PreEmitPeephole.cpp`
(`optimizeSPRelativeSpillToPushPop`, ~281 lines). It builds, its lit test passed,
and autoload-in-c booted to `A>` with it. It saved only 4 compressed bytes on the
autoload PROM. It is **unsound**.

## Minimal repro

`z80-utils/test-runner/testcases/clang/test_22_recursion.c` at `-Os`:

- Peephole ON: `test_22_recursion_Os` **HANGS** (z88dk-ticks runs to the 10^10
  cycle counter at 99% CPU; non-terminating).
- Peephole OFF (stash + rebuild): `test_22_recursion_Os` **PASS** (DE=0x000F), all
  42 tests in the file PASS.

A/B done per `feedback_ab_before_blaming_test_runner` (stash the single peephole
file, `ninja -C build-macos clang llc`, rerun `cargo run -- clang test_22`).

## Root cause (real pass output, `_ackermann`)

`ackermann(m-1, ackermann(m, n-1))` keeps `m` live across the inner recursive
call: it is read for the inner call arg **and** again (as `m-1`) after the call.

Baseline (peephole OFF) — three accesses to the SP+0 slot holding `m`:

```
4f: push af                                  ; prologue: allocate SP+0 slot
5b: ld hl,$0; add hl,sp; ld (hl),c; inc hl; ld (hl),b   ; (1) SPILL  m -> SP+0
62: ld hl,$0; add hl,sp; ld c,(hl); inc hl; ld b,(hl)   ; (2) RELOAD m  (pre-call)
6b: call _ackermann
6e: ld hl,$0; add hl,sp; ld c,(hl); inc hl; ld b,(hl)   ; (3) RELOAD m  (post-call)
80: inc sp; inc sp; ret
```

Peephole ON — it matches the spill (1) and the *first* reload (2) and rewrites
them to `push bc` / `pop bc`, but reload (3) survives unchanged:

```
5b: push bc                                  ; was (1) SPILL
5c: call _ackermann
5f: ld hl,$0; add hl,sp; ld c,(hl); inc hl; ld b,(hl)   ; (3) survives -> reads SP+0
```

`push bc` writes a *fresh* 2 bytes at SP-2; `pop bc` reads them back and
deallocates. The prologue-allocated slot at SP+0 (from `push af`) is **never
written**. Reload (3) therefore reads the uninitialised slot (stale `push af`
value) → `m` is garbage → the `m == 0` / `m-1` recursion never terminates → hang.

Note also the converted pair here isn't even "across a call": reload (2) is
*before* the call, so the rewrite is pointless as well as wrong.

## The missing guard

The peephole finds the spill and the **first** matching reload and stops. It never
verifies the spill slot is read **exactly once**. A sound version must prove the
frame slot has no other reader (and no other SP-relative access) between the spill
and end-of-liveness before collapsing to PUSH/POP — the same class of guard the
BSS-spill peepholes already carry (cross-block / orphan-access / stack-depth, see
`Z80LateOptimization.cpp`). It happened to be safe on autoload only because
`_fdc_read_result` has a single reload.

## Oracle gap

The committed lit test (`issue-331-spill-push-pop.ll`, 86203a6) only covered
single-reload cases, and the draft narrowed it further (dropped case 2), so lit
went green while the pass miscompiled recursion. The detector that *did* catch it
is the runtime suite (`test_22_recursion`). Per `feedback_audit_oracle_not_just_fix`
the lit file now documents the parked/unsound status and points at that runtime
detector; test_22 is the standing guard against a naive reintroduction.

## Disposition

- Draft peephole discarded from the working tree (uncommitted only).
- `issue-331-spill-push-pop.ll` rewritten to pin the already-correct native
  loop-counter PUSH/POP and document the parked/unsound status.
- Optimization #331 stays OPEN upstream as a density enhancement; a sound
  implementation needs the single-reader guard above. Not pursued now (4 B).
