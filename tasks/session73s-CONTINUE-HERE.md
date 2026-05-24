# Session 73s — CONTINUE HERE (saved for reboot 2026-05-24)

State at save: clean. All source committed; baseline build verified (AES `make
clang.ram` PASS 11 516 046 ts; Z80 lit 111 PASS + 5 XFAIL). Production byte-identical
(AES09 .text 2228 B). Nothing pushed to origin (per standing instruction).

## What got done this session (all committed)

1. **#178 / #166 — RESOLVED.** Root-caused the tied-operand coalescer miscompile (5-line
   repro); built non-tied `ADD16_acc` (correct, default-off `-z80-add16-acc`); measured
   lever exhausted (FLAGS-clobber blocks remat + HL-pinning +9.8%); safety-audited (no
   latent risk); fundamental conclusion: 16-bit ADD/SUB un-rematerializable on Z80.
   Writeup: `session73s-issue178-add16-tied-rootcause.md`.

2. **#112 (un-reserve IY) — dominant bug FIXED + shipped, residual scoped.**
   - Encoder opcode-0 crash (session-40's 387 FATALs): already fixed (GR16NoIR/GR16_BCDE).
   - **FIXED** `LEA_IX_FI` missing IY case (silent no-op in Release -> IY undefined),
     commit `27be55e2569d`. Added hidden `-z80-unreserve-iy` flag (default OFF; production
     byte-identical) + lit `lea-fi-iy-112.ll`. Test-runner IY-on **622/85/76 -> 684/42/57**
     (baseline 690/37/56): **63 of 70 regressions cleared**.
   - Residual (6 i32 tests) drilled: PERVASIVE undocumented IYH/IYL via byte-decomposition
     of IY values (40+ sub-register sites). Tried a `G_UNMERGE_VALUES` constraint, reverted
     (folds bypass it). Writeup: `session73s-issue112-iy-unreserve-scope.md`.

## THE OPEN DECISION (ask the user first when resuming)

#112's remaining win is gated on a **strategic choice**, not more drilling:

- **(A) Enable `+undocumented` in production.** IY's byte halves (IYH/IYL) are undocumented;
  real Z80 + MAME both execute them correctly. The only blocker is the project rule
  `feedback_no_undocumented_default`. If the user OKs the policy, un-reserve IY *with*
  `+undocumented` -> IY fully allocatable and legal. **Cheapest path to the −33 B AES win +
  3-pair-pressure relief (helps #27/#110/#115/#178).**
- **(B) Keep `!undocumented`.** Then IY is only safe for pure-16-bit values (pointers,
  `ADD IY,rr`, load/store — the `LEA_IX_FI` class, already fixed). Using it for byte-accessed
  values needs a regalloc constraint (no IX/IY for vregs with sub-register uses) or guarding
  40+ extraction sites. Large/fragile.

**First action on resume:** ask the user (A) vs (B). Most leverage is (A).
- If (A): set `+undocumented` for the production builds, flip `-z80-unreserve-iy` on (or
  un-reserve by default under +undocumented), run the FULL oracle: test-runner A/B (expect
  ~0 IY regressions), AES corpus 13/13, cpnos/autoload/BIOS sizes + **MAME boots** (#14 was a
  MAME runtime crash — boots mandatory), then decide whether to also un-reserve IX.
- If (B): design the regalloc/class constraint; bigger effort.

## Other open items (lower priority, all documented)
- **i32-undoc residual** is the #112 residual above (same thing).
- **#172** (8-bit ALU accumulator A-pin): mechanism-blocked, needs liveness-aware selector
  (~200-400 LOC), drilled to default-off in 73o. Untouched this session.
- The `-z80-unreserve-iy` and `-z80-add16-acc` flags are both default-OFF bring-up infra.

## Build/verify quickref
- Build: `NINJA=/Applications/CLion.app/Contents/bin/ninja/mac/aarch64/ninja; $NINJA -C build-macos clang llc`
- AES runtime oracle: `cd rc700-gensmedet/tasks/aes256-corpus && rm -f clang.{bin,elf,ram} aes256_clang.o test_main_clang.o && make clang.ram` then check ram[0xC010]=01, [0xC021]=01, [0xC022]=0xA5.
- test-runner: `cd llvm-z80/z80-utils/test-runner; PATH=…/z88dk/bin:$PATH BUILD_DIR=…/build-macos cargo run --release -- clang`. Baseline 690/37/56/207. Inject IY via `-z80-unreserve-iy` (lit/llc) — the runner itself doesn't pass `-mllvm`, so for full-suite IY-on use a clang wrapper or temporarily re-add the env hook.
- Disk was at 45 GB / 90% — watch it.

Last commits (workspace): bump chain ends at the #112 LEA_IX_FI + residual work; `git log --oneline -8` to see.
