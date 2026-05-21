# Session 73p — three documented dead-ends, one new issue filed

Date: 2026-05-21.  Follow-on from session 73o.  All commits + value-
oracle preserved; net codegen change is zero (every attempted fix
either reverted or default-off).

## TL;DR

Three lines of investigation, each ended with a documented negative
result rather than a fix:

1. **#172 (A-pin)** — scope tightened to per-MBB-single-candidate.
   Removes the 73o segfault but pin=on still regresses (+24 to +81 B).
   Default stays OFF.  Real fix needs LiveIntervals + PHI walk to pin
   the loop carrier, not proxies.

2. **#166 (ADD_HL_rr rematerialization)** — direct attempt is
   impossible: ADD_HL_rr has no SSA output (HL defined implicitly via
   `Defs = [HL, FLAGS]`).  `isReMaterializable` flag is silently
   ignored.  Real path is `ADD16_tied`.

3. **ADD16_tied ISel wire-up at G_PTR_ADD** — two routes attempted:
   - `$dst` GR16 (default): all 13 AES configs FAIL.  Root cause: BC/DE
     fallback in expansion clobbered HL without declaring it.
   - `$dst` HLI (HL/IX/IY only, fallback removed): still miscompiles.
     13/13 AES FAIL, test-runner 173/990 FAIL (vs baseline 46), 4 lit
     regressions.  Root cause: unisolated regalloc / two-address
     interaction with tied operands on narrow physreg class.
   Both reverted; in-place comment captures diagnosis for future.

**One new issue filed**: #173 — 8-bit BSS spill via A goes through a
6 B `push af; ld a,r; ld (nn),a; pop af` shape.  PUSH/POP-pair
peephole would deliver 4 B savings per spill cycle; multiple sites
per parallel-XOR-shape function (e.g. aes_mc_inv has ~15+ instances).
Higher-yield-per-session-hour than the parked paths above.

## Commits (llvm-z80 main)

- `862321520547` — #172 A-pin: tighten scope to single-candidate-per-MBB
- `34b1732266c4` — #166 ADD_HL_rr: document why remat doesn't apply (no SSA output)
- `8400050dd2bf` — #166: document ADD16_tied wire-up dead-end at G_PTR_ADD
- (session summary commit pending)

## Value oracle (HEAD post-session)

| Oracle | Result |
|---|---|
| Z80 lit suite | 104 PASS + 3 XFAIL (unchanged) |
| AES corpus | 13/13 PASS (byte-identical to pre-session baseline) |
| z80-utils test-runner | 681/46/56/207 (matches baseline noise band) |
| AES verifier 4 cells | PASS (K&R + ANSI × clang + zsdcc) |

No regressions.  No codegen change in default settings.

## What was Easy / Hard / Painful

### Line 1 — #172 conservative scope (Easy)

~30 line rewrite of `Z80PinAluAccumulator` to bucket candidates by
unique MBB and pin only size-1 buckets.  Builds clean, fixes
segfault.  Empirical A/B on 01/05/09 confirmed regression persists
even on safe cases (+24-81 B).  Honest stopping point: the data
proves proxy-pinning isn't enough regardless of safety scope.

### Line 2 — #166 ADD_HL_rr direct attempt (Easy negative)

Marked `isAsCheapAsAMove + isReMaterializable` on `ADD_HL_rr`.
Byte-identical AES corpus across 13 configs.  Diagnosed in 5 minutes:
the pseudo has no virtual output (HL is implicit physreg def), so
the rematerializer has nothing to clone.  Clean negative result, fast.

### Line 3 — #166 ADD16_tied wire-up (Hard)

Substantial debugging investment.  First route (GR16 dst, default
.td) failed AES 13/13.  Diagnosed: BC/DE fallback `PUSH BC; POP HL;
ADD HL,rr; PUSH HL; POP BC` clobbers HL without `Defs[HL]` declaration.

Second route (HLI dst, fallback removed) — expected this to fix it
but it didn't.  Still 13/13 AES FAIL, plus 4 lit regressions and
catastrophic test-runner damage (173/990 FAIL vs 46 baseline).  The
miscompile affects values UNRELATED to the ADD16_tied output, which
points to a deeper regalloc / two-address interaction with tied
operands on narrow physreg classes — but I couldn't isolate it
within this session.

**Painful**: realizing that even the conservative-class fix didn't
solve the second route, and that the actual root cause is now
something more subtle in the regalloc pipeline.  Took the test-runner
hit (173 FAIL) to be certain this wasn't viable to land.

Both routes reverted; in-place comment captures both failure modes
+ diagnosis so future drillers don't repeat both attempts.

### Line 4 — aes_mc_inv inspection → new finding (Medium)

Reading the clang +static-stack asm for `aes_mc_inv` (the single
largest clang-vs-SDCC per-function gap) surfaced the dominant
remaining bloat: 8-bit BSS spills going through A with `push af /
ld a, r / ld (nn), a / pop af` (6 B per spill).  Z80 has no
`LD (nn), r8` except for A, so any non-A 8-bit spill needs A as
transit, requiring A-save/restore.

Filed as **#173** with proposed fix paths (peephole detecting the
pattern and converting to PUSH/POP rr when partner half is dead;
or mixed-mode BSS routing 8-bit spills through IX-indexed slots).

Estimated yield: 100-200 B at AES `09_Oz_prod_like` (4-8 %),
substantial reduction in `aes_mc_inv` / `aes_mixColumns` /
`aes_subBytes`.  This is a higher-yield-per-session-hour next-target
than continuing to drill the parked paths.

## Open levers ranked by likely yield-per-session-hour

1. **#173 — 8-bit BSS spill peephole** (NEW this session).  Concrete
   pattern, well-localized, modeled on existing BSS-spill-across-CALL
   peephole.  Estimated 100-200 B AES + cpnos benefit.
2. **#170 — Z80NarrowIV parallel-phi guard removal**.  Backend-debug
   investigation; the current single-phi guard works.  Removing it
   needs SCEV-based proofs.  Narrow scope.
3. **#166 — ADD16_tied MIR-after-two-addr diagnosis**.  Print MIR
   after two-address pass on a failing function, isolate the
   unrelated-value corruption.  Hard but the diagnosis output would
   be reusable for fixing similar tied-operand issues across the
   backend.
4. **#172 — A-pin liveness-aware version**.  ~200-400 line investment.
   Yield bounded at ~5 pp of the 19 % AES gap.  Lower priority than
   #173 / #170.
5. **#169 — LSR + backend miscompile root-cause**.  Reduction
   incomplete; only manifests in full AES pipeline.  Multi-session
   investigation.

## Difficulty: Medium

No single difficult crux — three small drills, each ending honestly.
The artifact is documentation density (3 commits, 1 new issue, 2
issue comment updates) rather than codegen change.  Future drillers
of these issues start with substantially better signposting.
