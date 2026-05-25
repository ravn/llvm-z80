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

## UPDATE 2026-05-25 (later): full oracle run with IY default-ON -> NOT clean

User asked to flip `-z80-unreserve-iy` default to ON and run the full oracle.
Result: **default-on is NOT shippable** -- it introduces correctness regressions
beyond the loop-carried bug already fixed.  Reverted default to OFF (production
restored byte-identical: AES C010=01/C021=01, 11516046 ts, 3715 B; lit 112+5).

Full oracle (IY default-ON):
- **AES production target: MISCOMPILE** (deterministic C010=00, was 01).  Hard blocker.
- test-runner clang: 694/37-38/57-58 vs IY-off 696/37/56 (net -2 pass; rest of the
  FATAL/FAIL list is the known test_90/91 `edge_*` noise, #136).  Real regressions:
  test_48_dynamic_alloca FATAL at ALL opt levels, test_40_hash_crc, test_38_sort_search.
- Z80 lit: 109 PASS + 3 FAIL (codegen shifts): `add16-acc` (`add iy,de` vs `add hl,de`),
  `ldir-aftermath` (reorder), `issue-156-bss-spill-loop-header-pop` (`push hl` reappears).

Dig-in (the AES/test-runner regressions are a regalloc CLASS, not one peephole):
- Isolated reliable repros (test-runner oracle): **test_167_iy_crc32** (FATAL O2 / FAIL O3)
  and **test_168_iy_crc_inner** (FAIL O1/Os, DE=0x0044 vs 0xEF8D).  crc reduction loops:
  i32 carried, `crc >>= 1` per iter.
- `-print-after-all` on crc_one O1: the z80-late-opt copy removals here are **LEGAL**
  (they drop a redundant `iy=hl; hl=iy` round-trip where IY is provably dead, redefined
  right after without a read).  So NOT my peephole.
- Root: the allocator **uses IY to hold pieces of a split 32-bit value** then shuffles it
  through expensive `push iy`/`pop hl` round-trips and corrupts the dataflow (wrong value,
  not a crash; stack balanced).  This is the "Phase-3 regalloc cost-model work" the
  original #112 framing gated IY-unreserve on -- needs a cost model that keeps IY for
  clean 16-bit pointer-like values and OFF multi-pair/sub-register-accessed values.
  The earlier "(B) documented-only constraint" attempt aimed at this and was net-harmful;
  a proper fix is a cost-model change, not a class-narrowing pass.
- `dynamic_alloca` (test_48) is a SEPARATE class (frame-pointer interaction).

Status: the loop-carried peephole fix (below) stays shipped -- it cleared the dominant
#14 crash.  IY-unreserve default-on remains gated on the i32-split regalloc cost model
(+ dynamic_alloca).  New repros test_166/167/168 + lit `iy-loop-carried-112.ll` are the
guards/vehicles for that work.  **Filed: ravn/llvm-z80 #189** (the i32-split-through-IY
regalloc miscompile, the gate) and **#190** (dynamic_alloca FATAL, separate class).
Backlog task added under `unpark-2026-05-22.md` "IY-unreserve default-on".

## UPDATE 2026-05-25: #112/#14 loop-carried residual ROOT-CAUSED + FIXED

Commit `dfa073a23e99`.  The loop-carried-value-in-IY hang was NOT a coalescer
bug -- it was the **Z80LateOptimization "IX/IY transfer" peephole** (Form 2,
COPY16_PUSHPOP pair; Form 1 latent) collapsing `COPY16_PUSHPOP IY,rr ...
COPY16_PUSHPOP rr,IY` -> `PUSH rr ... POP rr` and **dropping the `IY <- rr`
write** because it assumed IY dead-scratch.  For a loop-carried high word that
write is the per-iteration update; IY is live-out via the back-edge.  Found by
`-print-after-all` bisection (survives postrapseudos+scavenging, gone after
z80-late-opt).  Fix: `computeRegisterLiveness(IXReg, after-closing-copy) ==
LQR_Dead` guard on BOTH forms.  Reliable repro: `test_166_iy_shiftloop`
(test-runner, NOT the DIY harness).  Lit guard `iy-loop-carried-112.ll`.

Result: IY-on suite 684->694 pass; production IY-off 696/37/56 byte-identical
(AES 11516046 ts, lit 112+5).  **#112 IY-unreserve loop-carried residual is
CLOSED.**  Remaining IY-on deltas are ~2 tests (down from 70).  Next: identify
those 2 (run `cargo run -- clang` with flag temp-on, diff vs the 696/37/56
IY-off set) -- likely a separate small class, no longer the dominant blocker.

## (historical) UPDATE (post-reboot): user chose (B); (B) tried and FAILED -> reverted

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

**Assertions toolchain BUILT and ready:** `build-macos-asserts/bin/clang` (and llc),
`-DLLVM_ENABLE_ASSERTIONS=ON`.  Reproduces the residual.  Use for `-debug-only=regalloc` /
`-verify-machineinstrs`.  (Process lesson: do NOT edit source while a background build of the
same tree runs.)

**What was tried + ruled out this session:**
- A pre-RA pass narrowing PHI 16-bit operands off IX/IY (`Z80ConstrainByteAccess`, PHI
  variant): did NOT fix the hang -> the value reaching IY is a COPY of the loop-carried
  value, not the PHI vreg itself.  Reverted.
- DIY ticks harness (`/tmp/iyrepro`) for minimal repro: UNRELIABLE (return-reg HL-vs-DE
  ambiguity, `halt;jp _done` spins to tstate limit, volatile/opt artifacts -- a bare shift
  loop even gave a wrong IY-*off* baseline).  Do not trust its numbers.

**Reliable next step (fresh session):**
1. Reduce via the TEST-RUNNER (the reliable oracle): add `testcases/clang/test_NN_i32_shift_loop.c`
   with `while(v){v>>=1;}`-style body + `/* expect */`, temp-flip `-z80-unreserve-iy` default
   to true, `cargo run -- clang test_NN`.  Confirm the minimal hang there (NOT in /tmp/iyrepro).
2. Single-step the failing loop in z88dk-ticks (pipe trace through tail -- disk!) OR
   `build-macos-asserts/bin/clang ... -mllvm -debug-only=regalloc` to watch the i32 loop
   variable (`$de`+`$iy`) across one iteration; find where it stops decreasing.
3. Suspects: `LSHR16` expansion or `COPY16_PUSHPOP` (push/pop iy) for the loop-carried `$iy`
   under this exact register assignment.  test_04 bb.3 MIR *appears* to update $iy at the
   back-edge yet hangs -> subtle expansion/flags bug.
The OLD cheap hypothesis (ADD16_tied .td=HLI) is for the separate #178 manifestation only.

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
