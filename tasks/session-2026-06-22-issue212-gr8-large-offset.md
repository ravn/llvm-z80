# Session 2026-06-22 — Issue #212 close: GR8 large-offset HL borrow reads killed $hl

## TL;DR

Closed ravn/llvm-z80#212 by narrowing the `NeedSaveHL` gate in
`expandSpillGR8LargeOffset` and `expandReloadGR8LargeOffset`
(`Z80RegisterInfo.cpp`).  The pre-fix code blanket-saved HL whenever
the GR8 being spilled/reloaded was `H` or `L`, on the theory that the
sibling half might be live across the address-computation borrow.
After fastregalloc kills `$hl` whole (e.g. via `XOR_CMP_EQ16 killed
$hl, killed $bc`), that blanket save reads an undefined `$hl` and
trips `-verify-machineinstrs`.

The fix checks the SIBLING half specifically:

```cpp
if (DstIsHL) {
  Register OtherHalf = (DstReg == Z80::H) ? Z80::L : Z80::H;
  NeedSaveHL = isRegLiveAt(OtherHalf, MBB, NextIt, TRI);
} else {
  NeedSaveHL = isRegLiveAt(Z80::HL, MBB, NextIt, TRI);
}
```

The half being loaded/spilled is consumed by this MI itself; only the
sibling needs preservation.  When the sibling IS live (e.g. the
consecutive-RELOAD case the original comment described), `isRegLiveAt`
sees the read and returns true, so the save still fires.

## What the issue body got wrong

The 2026-05-31 issue body classified #212 as a "broad, multi-source
O0-liveness-precision problem" and said a previous "reaching-def-aware
saveBorrowHL/emitBorrowHLSave" attempt cleared zero cases, concluding
this was upstream-adjacent (fastregalloc / coarse liveness) and not
fixable at the backend level.

That classification was wrong.  The failing `PUSH_HL` is NOT emitted
by a generic borrow-save helper that consults stale liveness — it is
emitted by a Z80-specific frame-index-elimination path with a
mechanical bug:

```
bool NeedSaveHL = DstIsHL || isRegLiveAt(Z80::HL, MBB, NextIt, TRI);
```

`DstIsHL` flips the result to "save" unconditionally whenever DstReg
is H or L, regardless of whether either half has a live value at this
point.  That's not a liveness-precision issue; it's an over-eager
shortcut in a pure boolean check.

The previous attempt failed because it patched a *different* set of
borrow sites (SP-relative spill borrows, IX-frame storeBorrowHL) that
weren't the ones producing the failures.  Empirical bisection of one
failing function (test_06_i64_bitwise.c's `main`) traced the actual
emission to `expandReloadGR8LargeOffset` line 685.

## How #212 was triggered

The pre-PEI shape:
```
XOR_CMP_EQ16 killed $hl, killed $bc, defs A,B,FLAGS    ; kills $hl whole
renamable $h = RELOAD_GR8 %stack.96, 0                 ; large-offset
AND_r killed $h, ...                                   ; only $h used
```

The `RELOAD_GR8` frame index resolves to an offset outside (IX+d)
range (`__sfrend_` is the static-stack BSS endpoint; the local lives
at a negative offset > 128B from there).  PEI dispatches to
`expandReloadGR8LargeOffset`, which decides "DstIsHL = true → save
HL" and emits a `PUSH_HL` that reads the killed `$hl`.

Post-PEI:
```
PUSH_AF
PUSH_HL implicit $hl       <-- VERIFIER FAILURE: $hl is killed
PUSH_IX
POP_HL
LD_BC_nn 65191             ; large negative offset
ADD_HL_BC                  ; HL := IX + offset
LD_A_HLind                 ; A := byte at IX+offset
POP_HL                     ; restore (dead) HL
LD_H_A                     ; H := A   (overwrites POP_HL's restored garbage)
POP_AF
AND_r killed $h
```

The borrow is functionally a no-op here: `PUSH_HL`/`POP_HL` save and
restore a value that nothing reads downstream (the very next thing
overwrites `$h`, and `$l` is dead).  Removing it leaves runtime
semantics unchanged and lets the verifier pass.

## Why +static-stack still hits this

`Z80RegisterInfo::eliminateFrameIndex` has a `+static-stack && !UseFP`
short-circuit (line 1466) that handles GR8 reloads via direct
addressing (`LD A,(addr); LD r,A`) — bypassing the
large-offset expander entirely.  But `UseFP` is true whenever the
function has `AllocaInst`s (`Z80FrameLowering::hasFPImpl`), which
applies to every nontrivial C function with locals.  So at +static-
stack, large-offset GR8 reloads still flow through the
`expandReloadGR8LargeOffset` path that contained the bug.

## Why production was unaffected

Production functions (autoload `start`, BIOS service routines, cpnos
slave loop) build at `-Oz`/`-Os`/`-O2` where:
1.  Register pressure is lower (no per-temp spills like O0).
2.  Dead `RELOAD_GR8 → $h` after a killed `$hl` doesn't survive DCE.
3.  Frame sizes stay within the (IX+d) -128..127 range, so the
    large-offset expander rarely fires.

`verify-production.sh` enforces verifier-clean at the production opt
levels and has been clean throughout.  This fix also keeps it clean.

## Files changed

```
llvm/lib/Target/Z80/Z80RegisterInfo.cpp                     |  +27 -8
llvm/test/CodeGen/Z80/issue-212-gr8-reload-hl-kill.mir      |  new
```

## Verification

- **Lit**: 155 PASS + 6 XFAIL (`llvm/test/CodeGen/Z80/`), the new
  `issue-212-gr8-reload-hl-kill.mir` exercises the bug directly via
  `llc -run-pass=prologepilog -verify-machineinstrs` (the test
  reproduces the verifier abort on un-fixed `llc`).
- **O0 sweep**: all 187 testcases (`z80-utils/test-runner/testcases/
  clang/*.c`) pass `-verify-machineinstrs` at `-O0 +static-stack`.
  Pre-fix this was 57 failures (test_06/07/08/36/45/46 + the
  test_90/91/92 edge families per the original issue body).
- **verify-production.sh**: 0 failures across `-Oz/-Os/-O2
  +static-stack`.
- **Production binaries byte-identical**:
  - autoload PROM: 1660 B (matches baseline)
  - rcbios BIOS: 5462 B (matches baseline)
  - cpnos PIO PROM1 line program: 2016 B (matches baseline; the
    workspace `CLAUDE.md` value of 2015 B was off-by-one from a
    different measurement run)
- **test-runner clang suite**: 866 PASS / 0 FAIL / 0 FATAL / 256 SKIP
  (matches the baseline post-#175 commit).

## Cross-references

- Issue: ravn/llvm-z80#212
- Related earlier #197 fix that addressed the SP-relative borrow path
  (different file): `llvm/test/CodeGen/Z80/issue-197-isreglivat-skip-
  undef-use.mir`
- `verify-production.sh`: `z80-utils/test-runner/scripts/verify-
  production.sh` — keeps shipping opt levels verifier-clean
- Source for the original under-targeted fix: tasks notes
  `issue197-residual-tail-156428-2026-05-28.md` and
  `issue197-adjcallstackup-deadflag-rootcause-2026-05-28.md`
