# Z80LateOptimization liveness-guard audit (session 73s, 2026-05-25)

Audit prompted by three bugs fixed this session that shared a shape — a peephole
that erases/moves/replaces an instruction or reuses a register without the
matching safety check:
- **#189** IX/IY transfer peephole dropped a loop-carried IY update (no value-dead check).
- **#192** #173 peephole relocated `LD r,A` into a region that reads `r` (no read check).
- **#193** BSS-spill->PUSH/POP `--MII` dangled on an adjacent reload (no iterator safety).

## Method
Three failure classes: (1) value liveness, (2) iterator safety, (3) flag liveness.
For each of the ~50 peepholes, classify what it mutates and whether it has the
matching guard.  Plus an automated cross-check: run `-verify-machineinstrs` after
late-opt on representative inputs.

## Key structural finding
The pass **systematically** guards liveness through the shared primitive
`isRegDeadAfter` (Z80LateOptimization.cpp:312), used at ~25 sites for FLAGS/A/reg
deadness.  `isRegDeadAfter` was audited and is **sound/conservative**: it returns
"dead" only on a full redefine-before-use or end-of-block with no successor
live-in; its one gap (doesn't inspect `RegMask`) yields false-negatives
(over-conservative bail), never a false "dead" -> no miscompile path.

**The three bugs were the exceptions** — peepholes doing ad-hoc iterator/register
handling *instead of* using the shared guards.  All three are now fixed.

## Peepholes audited clean (no missing guard)
- `isRegDeadAfter`-guarded FLAGS/A/reg sites (~25): DEC->DJNZ, XOR->CPL, LD#0->XOR A,
  AND$1->RRCA, carry-roundtrip, ADD A,1->INC, CMP_Z16 const fold, #116 EQ/NE,
  CP/XOR->DEC/INC, etc. — sound.
- **Cross-block redundant LD A,r (#60, 3874):** proper monotone dataflow
  (meet-over-preds, fixpoint, clobber transfer over operands+implicit_defs+regmask;
  idempotent OR_A/AND_A special-cased correctly).  Safe.
- **In-block redundant LD A,r (#60, 1649):** bails on any A/reg clobber, CALL,
  regmask in its 8-instr window.  Safe.
- **BSS load forwarding (5944):** store/load tracking with clobber-invalidation
  (operands+implicit_defs), CALL clears all, volatile skipped.  Safe.
- **BSS-spill->PUSH/POP siblings:** cross-class (4633) erases the load first so the
  store iterator stays valid + the inserted PUSH keeps `--MII` valid; cross-MBB
  (4809) erases then `break`s + `RestartOuter` (no `++MII` on a dead iterator).
  Both safe.  (#193 at 4381 was the unique iterator bug; fixed.)
- **LDIR aftermath / DE reuse (1866):** count-match + `SlotClobbered` proof + the
  triple is matched tight after LDIR (DE not clobbered before reuse).  Safe but
  intricate (maintenance risk).
- **Known-immediate A tracking (1733):** invalidates on clobber, checks
  flag-consumers before removal.  Safe.

## New finding (pre-existing, latent): verify-machineinstrs fails after late-opt
`-verify-machineinstrs` **passes before** Z80LateOptimization and **fails after**
on AES `gf_log`:

```
*** Bad machine code: Using an undefined physical register ***
- function:    gf_log    basic block: %bb.2
- instruction: ADD_A_A implicit-def $a, implicit-def $flags, implicit $a
```

So a late-opt peephole leaves `$a` undefined at the `ADD_A_A` read (stale block
liveins / a removed-or-moved A def without metadata update).  Reproduces at plain
`-O2` (NOT `+static-stack`-specific; NOT from this session's #192/#193 which are
static-stack-only, nor #189 which only fires on IX/IY copies).  **Benign at runtime**
(AES is byte-correct), so it is a liveness-*metadata* staleness, not a value
miscompile — but it is exactly the latent class that bites a later pass trusting
the stale liveness.  Filed as ravn/llvm-z80#194.

**Bisected (via `-stop-before`/`-stop-after=z80-late-opt`):** the **cross-block
redundant-`LD A,r` removal (#60, ~3874)** removes gf_log bb.2's `LD_A_E` (A==E
enters bb.2 from bb.1, correctly) but does NOT add `$a` to bb.2's live-ins, so
`ADD_A_A` then reads `$a` with `$a` absent from live-ins.  A `fullyRecomputeLiveIns`
at end of late-opt fixes it but (a) grows the 2 KB-capped cpnos PROM by 2 B via
downstream block-placement, and (b) does NOT achieve module verify-clean (PEI and
other generic post-RA passes have their own pre-existing staleness, e.g.
`aes_ar_cpy` PUSH_AF).  **Fix deferred** — needs a byte-neutral surgical live-in
update in the #60 removal, or a coordinated verify-clean effort.  In-code NOTE
left at `runOnMachineFunction` end (commit `ca8938268442`).  Minor side note:
cpnos size fluxes ±1 B across clang rebuilds from comment-only changes (a peephole
iterating a pointer-keyed DenseMap — non-deterministic order).

## Recommendation
- New peepholes that erase/move instructions should use the shared `isRegDeadAfter`
  + "anchor resumption to an inserted/un-erased instruction" patterns rather than
  ad-hoc `--MII` / manual register tracking.
- Consider wiring `-verify-machineinstrs` after Z80LateOptimization in a CI lane
  (it would have caught #193 and surfaces #194).
