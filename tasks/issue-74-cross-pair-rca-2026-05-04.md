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

- File ravn/llvm-z80#74 follow-up: re-enable BSS-spill→PUSH/POP only
  after the LIFO refactor's interaction with autoload-in-c is
  understood and fixed.
- A/B investigation: pin down the exact mechanism by which #74's
  code path breaks autoload-in-c.  Most likely path: instrument
  rcbios's BSS-spill fire-sites with `LLVM_DEBUG` and trace what
  autoload-in-c is doing differently.
- ravn/llvm-z80#123 — investigate which optimizer decisions are
  influenced by `-g`.  Adding `-g` to autoload-in-c CFLAGS to enable
  source-annotated listings shifted the PROM 1826 → 1861 B (+35 B).
  Likely cause: a Z80LateOptimization peephole walks instructions
  without skipping `DBG_VALUE`, or a GISel combiner preserves debug
  locations by suppressing some rewrites.  Low priority but a probe
  into backend correctness under `-g`.
