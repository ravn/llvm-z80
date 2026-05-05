# RCA: BSS spill→PUSH/POP #74 regression

**Bisect identified:** commit `96dde0c` ([Z80] BSS spill→PUSH/POP:
cross-register-pair (#74, refines #82), 2026-05-02).

**Reported symptom:** autoload-in-c PROM hangs.  PC sample stuck at
LMA `0x02D6` = VMA `0x626E` (inside `_fdc_detect_sector_size_and_density`,
right after `call $6138` returns).  PROM display (0x7A00) shows
autoload banner, BIOS display (0xF800) stays blank — autoload never
hands off to BIOS.  rcbios standalone boot via the **assembly**
`roa375.rom` PROM is unaffected.

## Bisect verdict

- `da18ede` (parent of 96dde0c): autoload-in-c boots cleanly via
  `cd autoload-in-c && make mame` → PASS at frame=200, 4.0s emulated.
- `96dde0c` and every later commit: autoload-in-c hangs.

## Resolution

**Full revert of 96dde0c's changes to `Z80LateOptimization.cpp`**
(commit `b843d94`, 2026-05-04 evening).  da18ede's BSS-spill block
grafted in place of the post-#74 version, while keeping subsequent
commits' changes (#104 / #107 H/L liveness checks etc.) intact.

This is a TACTICAL fix — restores the working pre-#74 behavior
(same-pair-only constraint, single-pass apply-as-we-go) and gives
up #74's BIOS savings until the actual root cause is identified.

Verification per user's success criterion ("autoload-in-c boots
when both autoload-in-c AND rcbios are freshly compiled"):

  - Both freshly built with the spliced clang.
  - `make mame` in autoload-in-c: PASS at frame=200, 4s emulated.
  - rcbios bios.cim: 5961 B (was 5929 B at #74's peak; +32 B).
  - cpnos-rom: 1777 B (unchanged).
  - Z80 lit suite: 90/90 (89 PASS + 1 XFAIL).

## Earlier wrong conclusions (corrected)

Earlier in the session I committed a **conservative fix** (021d5e5)
that restricted the BSS-spill peephole to same-register-pair only
(reverting #74's cross-pair extension) but kept the LIFO collect-
and-reverse-apply refactor.  I claimed that worked.  **It did not.**

Methodology error: I conflated `make mame-test` runs (which use
whatever PROM was last installed in `mame/roms/rc702/`) with
autoload-in-c boot tests.  When the **assembly** roa375 PROM was
installed, `make mame-test` showed the rcbios BIOS banner — which I
read as "autoload-in-c works."  The assembly autoload was being
booted, NOT autoload-in-c.

I also asserted that "autoload-in-c is byte-identical between
broken and fixed compiler states; the bug is in rcbios."  The
byte-identical claim was correct (only the build-date string
differs).  But the "bug-is-in-rcbios" inference was wrong — the
conservative fix's effect on rcbios doesn't actually restore
autoload-in-c boot.  Specifically the LIFO refactor (collect-and-
reverse-apply) is implicated, not just the cross-pair extension —
but I have NOT pinned the exact mechanism.

## What's still unknown

- **Why** the post-#74 BSS-spill peephole code breaks autoload-in-c
  when it doesn't break rcbios standalone boot.
- **Which** specific transformation in the new code is wrong:
    (a) Cross-pair extension itself.
    (b) Collect-and-reverse-apply ordering.
    (c) Some interaction with #82's orphan-load handling.
- **Whether** the bug surfaces in autoload-in-c via:
    (i) A BSS-spill peephole site IN AUTOLOAD-IN-C that miscompiles
        — but autoload-in-c's prom.bin is byte-identical between
        compiler states (only timestamp differs), so this is unlikely.
    (ii) An indirect mechanism — e.g. the peephole's behavior
         affects which other passes fire upstream, changing
         autoload-in-c's CODE indirectly.  But asm-identical
         autoload-in-c rules this out.
    (iii) Some side effect via rcbios that autoload-in-c depends on
          but assembly roa375 doesn't (disk-image content, BSS
          layout assumptions, ABI register state at hand-off).

The third hypothesis is the most plausible given the byte-identical
autoload-in-c.  A/B asm diff of rcbios at the autoload's hand-off
point + register-state inspection at the BIOS entry would pin it
down.

## Lessons logged

  1. **Verify the success criterion explicitly before claiming fix.**
     "Run `make mame-test` and see BIOS banner" is NOT a verification
     of autoload-in-c — it's a verification of whatever PROM is
     installed.  Always check the PROM's MD5 + the actual driver
     of the boot path before reading the screen as a result.

  2. **A/B test with EXPLICIT artifact MD5s.**  Capture compiler
     binary, autoload PROM, rcbios bios.cim MD5s before EACH test
     run.  Stale install state from prior tests is a real source
     of confusion in iterative debugging.

  3. **autoload-in-c MAME boot test required for ANY llvm-z80
     commit.**  Already added to the lessons doc + auto-memory.
     This regression was undetected for ~3 weeks (since 2026-05-02)
     because no automated check ran the C autoload.

  4. **Bisect identifies the commit; the fix scope is independent.**
     Bisect found 96dde0c.  My initial fix scope ("revert just the
     cross-pair feature") was WRONG.  The right scope was "full
     revert of the whole commit's codegen changes" — but I assumed
     too aggressively that the smaller surgery was sufficient.

  5. **"Better than nothing" claims need qualification.**  When my
     conservative fix passed lit + size oracle but DIDN'T pass the
     value oracle (autoload-in-c boot), I committed it anyway and
     claimed success.  That violated the HARD RULE.  Better: hold
     the commit until the value oracle is green, OR commit + label
     it explicitly as a "tactical workaround pending real fix" with
     no claim of correctness.

## Carry-forward

- ravn/llvm-z80#74 (REOPENED 2026-05-05 with implementation
  instructions): re-enable BSS-spill→PUSH/POP only after the LIFO
  refactor's interaction with autoload-in-c is understood and fixed.
  Issue body now contains four investigation hypotheses, two
  restoration paths (narrower implementation vs root-cause-first),
  and the HARD-RULE verification protocol (lit + test-runner +
  autoload-in-c boot + rcbios MAME boot + cpnos-rom byte-identity).

- A/B investigation: pin down the exact mechanism by which #74's
  code path breaks autoload-in-c.  Most likely path: instrument
  rcbios's BSS-spill fire-sites with `LLVM_DEBUG` and trace what
  autoload-in-c is doing differently.

- **Rolling-walk listings as an investigation resource (2026-05-05):**
  rc700-gensmedet commits `5dbedb6..b75b7ae` capture per-step
  source-annotated `bios.clang.lis` and `prom.clang.lis` for every
  post-merge ravn-fork commit between `da18ede` and HEAD.  Each
  step's listing was generated with #74 reverted on top, so diffing
  adjacent steps shows what each commit changed in isolation.
  Step 4 has BOTH a FAIL state (#74 active) AND a FIX state (#74
  reverted) — diffing those two listings is the cleanest way to
  see what the cross-pair extension actually rewrites in rcbios.
  Useful starting point for narrowing which fire-site causes the
  autoload-in-c hang.

- ravn/llvm-z80#120 (REOPENED 2026-05-05): GISel combiner for
  `(shl 7; ashr 7)` icmp idiom is silently unsound at the IR layer
  because Z80's `BooleanContents = ZeroOrOne`.  Three migration
  paths in the issue body (post-ISel combiner / split G_ICMP
  lowering / change BooleanContents target-wide).  Park until
  regalloc cluster work lands.

- ravn/llvm-z80#123 — investigate which optimizer decisions are
  influenced by `-g`.  Adding `-g` to autoload-in-c CFLAGS to enable
  source-annotated listings shifted the PROM 1826 → 1861 B (+35 B).
  Likely cause: a Z80LateOptimization peephole walks instructions
  without skipping `DBG_VALUE`, or a GISel combiner preserves debug
  locations by suppressing some rewrites.  Low priority but a probe
  into backend correctness under `-g`.

- ravn/llvm-z80#124 (filed 2026-05-05) — workspace: cmake 4.2 +
  macOS treats third-party/benchmark HAVE_PTHREAD_AFFINITY failure
  as fatal during reconfigure; workaround is
  `-DLLVM_INCLUDE_BENCHMARKS=OFF`, persisted in current build-macos
  CMakeCache.  Possible upstream fix: add the flag to
  `clang/cmake/caches/Z80.cmake`.

## Surgical-walk procedure (recipe for future bisect/walk work)

For walking older Z80 backend commits while keeping LLVM core at HEAD
(necessary because the upstream-merge `f91102a4` brings in 3781 commits
that would full-rebuild on every revert):

1. **Configure cmake once with benchmarks disabled** (avoids #124):
   ```
   cmake -S llvm -B build-macos -DLLVM_INCLUDE_BENCHMARKS=OFF -G Ninja
   ```

2. **Per-step setup — orphan-file removal:**  Some .cpp files exist at
   HEAD but not at the older step (e.g., `Z80SplitDjnzCounters.cpp`
   added by 90687fc7, `Z80LoopRotate.cpp` added by c6e867dd).  Diff
   the file lists and `rm` files that aren't in the target:
   ```
   git ls-tree --name-only HEAD -- llvm/lib/Target/Z80/ | grep -E '\.(cpp|h|td)$' | sort > /tmp/z80_head.txt
   git ls-tree --name-only <SHA> -- llvm/lib/Target/Z80/ | grep -E '\.(cpp|h|td)$' | sort > /tmp/z80_step.txt
   comm -23 /tmp/z80_head.txt /tmp/z80_step.txt | xargs rm -f
   ```

3. **Full Z80 backend snapshot (CMakeLists included):**
   ```
   git checkout <SHA> -- llvm/lib/Target/Z80/ llvm/test/CodeGen/Z80/
   ```
   This produces a self-consistent snapshot.  Older CMakeLists won't
   reference the orphan files (already removed).

4. **Apply #74 revert if step is in [96dde0c..b843d94] window:**
   ```
   git show 96dde0c -- llvm/lib/Target/Z80/Z80LateOptimization.cpp | git apply -R
   ```
   If the step is `021d5e5` (the conservative fix that partially
   reverts 96dde0c), sequential reverse-apply is needed:
   ```
   git show 021d5e5 -- llvm/lib/Target/Z80/Z80LateOptimization.cpp | git apply -R
   git show 96dde0c -- llvm/lib/Target/Z80/Z80LateOptimization.cpp | git apply -R
   ```

5. **Build incrementally:**
   ```
   ninja -C build-macos clang llc llvm-objcopy llvm-objdump llvm-nm
   ```
   Per-step rebuild ~5-10 min on M1 (broader than expected because
   `git checkout` updates CMakeLists mtimes, triggering cmake regen
   which invalidates PCH dep info — even when content is identical).

6. **Value oracle:**
   ```
   cd rc700-gensmedet/rcbios-in-c   && make COMPILER=clang clean && make COMPILER=clang bios
   cd rc700-gensmedet/autoload-in-c && rm -f clang/*.o clang/prom.clang.* clang/prom0.ic66 && make COMPILER=clang prom
   cd rc700-gensmedet/autoload-in-c && make COMPILER=clang mame   # boot test
   ```

7. **Commit listings to rc700-gensmedet** with the SHA in the
   message so future investigators can find the per-step asm:
   ```
   git -C rc700-gensmedet add autoload-in-c/clang/prom.clang.lis rcbios-in-c/clang/bios.clang.lis
   git -C rc700-gensmedet commit -m "rolling-walk step N (<SHA>) ..."
   ```

8. **Restore HEAD when walk done:**
   ```
   git checkout HEAD -- llvm/lib/Target/Z80/ llvm/test/CodeGen/Z80/
   ```
