# Session 73s (cont.) — WRAP + fresh-session entry point (2026-05-27)

Long session across three closeout-plan clusters.  **9 issues closed with fixes**,
3 filed, all pushed.  Start a fresh session from the "Next session" section below.

## Closed with fixes this session (9)

| # | cluster | what |
|---|---------|------|
| #18 | 1 | `LD r,n -> LD r,A` known-constant peephole |
| #151 | 1 | verify-close (sext(icmp) already clean) |
| #152 | 1 | verify-close (SET/RES via `LD A,(HL)` already done) |
| #42 | 4 | compiler ships `<intrinsic.h>` + `__builtin_z80_di/ei/halt/nop/im2/set_i` |
| #4 | 4 | `__attribute__((z80_critical))` -> DI/EI; rcbios `__critical` now real |
| #133 | 4 | callee-side `z80_preserves_regs` save/restore (verify-close, Part A) |
| #203 | 2 | guard unification steps 1-3 (predicates, UsedElsewhere, SP-write) — PARTIAL |
| #139 | 2 | verify-close (stale cpnos-rom diagnostic) |
| #146 | 1 | reclassified (not a small win — HL live at epilog) |

Filed: **#206** (#18 non-A copy generalization), **#207** (#133 Part B advisory
warning), **#208** (gate im2/set_i off SM83).

## Production state (all MAME-verified)
- rcbios BIOS clang **5897 B** (was 5922), SDCC 6091 B — same source both compilers,
  no `#ifdef`, via the compiler-shipped `<intrinsic.h>`.  Boots to A>, disk ERR=0.
- cpnos PROM1 ~2026 B (timestamp/hash makes the compressed size wobble 2026<->2027;
  uncompressed payload is the stable figure).  polypascal PASS.
- Z80 lit **123+5**.  Differential oracles (`-diff-opt` + `-native-oracle`) **0/0** in
  default + static-stack — the standing value-oracle gate, green throughout.

## #203 — substantially done, ONE piece left (the fresh-session candidate)
Steps 1-3 unified every historically drift-prone SHARED guard of the spill->PUSH/POP
family into file-scope helpers (`z80IsAnyPush/Pop`, `z80SameBssAddr`,
`z80SlotUsedElsewhere`, `z80IsExplicitSPWrite`, + loop-carried/address-taken from
earlier).  −80 net lines, all behavior-preserving (cpnos byte-identical, oracles 0/0).

**Remaining:** the forward-scan **orphan-load + stack-depth** tracking is still
per-peephole because it's interleaved with each peephole's load-collection +
byte-accounting (peepholes #1 and #2 have near-identical forward scans that could be
unified into a shared scan helper returning {conflict, loads, stack-ok}).  This is a
forward-scan *restructure* — behavior-sensitive, NOT a mechanical de-dup — so it was
deliberately deferred per the peephole-safety discipline.  Gate any attempt on
byte-identical cpnos + differential oracles + lit, exactly as steps 1-3 were.

## Verified-benign findings (no action)
Three pre-existing `-Wunused-but-set-variable` in Z80LateOptimization.cpp:
`Saved`@2112 + `SeenCall`@5423 are used only in `LLVM_DEBUG` (benign in release);
`EndIsPseudo`@4314 is redundant dead code (the I5 iterator already captures the
pseudo-vs-POP distinction).  None is a dropped guard.  Optional 1-line hygiene
removal of `EndIsPseudo`; not worth a build cycle on its own.

## Next session — pick up here
Per `issue-closeout-plan-2026-05-27.md`, clusters 1/4 DONE, 2 substantially done.
Remaining:
1. **#203 forward-scan restructure** (finish Cluster 2) — the one delicate piece above.
2. **Cluster 3** (verifier/#197): test_48 build FIXED + #200 root-caused (2026-05-27);
   remaining: #200 fix (option A expand-at-PEI / B variadic-decl), #194 (stale liveness,
   delicate), #125 (-O0 +shadow-regs crash), #190 deep part (IY-unreserve).
3. **Cluster 5** (memmove/fill): #126, #127, #205, #50.
4. **Cluster 6** (tooling): #124, #137, #70.
5. Cluster-4 follow-ups: #207 (advisory warning), #208 (SM83 gating), + the rcbios
   set_i_reg-shim->builtin cleanup task.

All repos pushed + in sync at session end (llvm-z80 `4bdeea1`, rc700 `6086d82`,
workspace `4fa24d3`).
