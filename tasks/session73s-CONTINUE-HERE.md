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

## UPDATE (post-reboot): user chose (B); (B) tried and FAILED -> reverted

Built `Z80ConstrainByteAccess` (pre-RA: narrow byte-accessed vregs + enforce declared
GR16NoIR operand classes off IX/IY).  It removed the undocumented `IYH/IYL` (0 refs in
test_04/40) BUT the tests still FAILED and it broke `test_30` (net 10 vs baseline 6).
**Diagnosis corrected:** undoc was a co-symptom; the residual is a deeper wrong-value
regalloc miscompile when IY holds byte-decomposed values — the **same RegisterCoalescer
out-of-class root as #178**.  Class-narrowing reverted (net-harmful).  `LEA_IX_FI` fix +
`-z80-unreserve-iy` flag stay (committed).

**Where #112 stands now (after the option-1 investigation):** the residual is a genuine
**regalloc/coalescer correctness bug**, NOT a documented/undocumented issue.  Confirmed:
ticks executes undocumented IXH/IYH correctly (`canixh()` true for Z80), so the IY-on test
failures are REAL wrong-value miscompiles -> **(A) +undocumented would NOT fix them** either.
(B) class-narrowing removed the undoc ops but the value stayed wrong + broke a pointer test
(test_30) -> narrow 16-bit classes under IX/IY pressure trigger regalloc miscompiles, same
family as #178's out-of-class assignment.

**SHARPENED (post-reboot drill): the residual is a loop-carried-value-in-IY miscompile.**
Earlier undoc / byte-decomposition / coalescer framing was largely a red herring.  Clean-build
test-runner (flag temp-ON) showed: test_30 PASSES (it was a (B)-pass artifact); test_04/40
FATAL = emulator TIMEOUT (loop hang) at O1/O2/O3/Os only.  Minimal 6-line repro saved at
`tasks/session73s-issue112-popcount-iy-repro.c`:
  `volatile u32 val=0xA5A5A5A5; u8 count=0; u32 v=val; while(v){count+=(v&1); v>>=1;}` -> 0 at
  -O2 IY-on (16 correct at O0/Oz).  Asm diff: v's high half is PINNED in IY across the loop
  (`push hl; pop iy`) but the shifted-back value is never written into IY -> loop-carried
  chunk not maintained.  This is the parked #14 class (loop-carried PHI value in IY +
  push/pop-copy regalloc mishandling), NOT byte-decomposition/undoc, NOT #178's tied-operand.

**Next step (assertions build IN PROGRESS, `build-macos-asserts`):** when it finishes, trace
the 6-line repro at -O2 IY-on (`-mllvm -z80-unreserve-iy -debug-only=regalloc -print-after-all`)
through PHI-elim / two-addr / coalescer / greedy; find where the loop-back COPY into IY is
dropped/misplaced.  Build cmd: `cmake -C clang/cmake/caches/Z80.cmake -DLLVM_ENABLE_ASSERTIONS=ON
-DCMAKE_MAKE_PROGRAM=<ninja> -DCMAKE_C/CXX_COMPILER_LAUNCHER=<sccache> -G Ninja -S llvm -B build-macos-asserts`.
Likely fix: keep loop-carried PHI values off IY (a targeted constraint), OR fix the IY
loop-back COPY handling.  The OLD cheap hypothesis (ADD16_tied .td=HLI) is for the separate
#178 manifestation only -- not this.

## THE ORIGINAL DECISION (now resolved: user picked B, B failed)

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
