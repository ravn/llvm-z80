# Session 73s — #112 IY-unreserve: peephole fix shipped, default-on blocked (2026-05-25)

## One-line

Root-caused + fixed the dominant #14 loop-carried-IY crash (a peephole liveness
bug); ran the full oracle with IY default-on, found it still miscompiles
(regalloc-class), reverted to default-off, filed #189/#190.

## What shipped (committed + pushed)

**`dfa073a` — Z80LateOptimization IX/IY-transfer peephole liveness guard.**
The peephole collapsed `COPY16_PUSHPOP IY,rr ... COPY16_PUSHPOP rr,IY` ->
`PUSH rr ... POP rr`, dropping the `IY <- rr` write on the assumption IY is dead
scratch.  For a loop-carried IY value (live-out via back-edge) that write is the
per-iteration update -> loop froze (the #14 symptom).  Fix: gate both forms on
`computeRegisterLiveness(IXReg, after-closing-copy) == LQR_Dead`.  Genuine scratch
transfers still fire, so production (IY reserved) is byte-identical.
- Found by `-print-after-all` bisection (survives postrapseudos+scavenging, gone
  after z80-late-opt).
- Oracle: IY-on suite 684->694 pass; IY-off production 696/37/56 byte-identical
  (AES 11516046 ts, lit 112+5).  Lit guard `iy-loop-carried-112.ll`.

## Full oracle with IY default-on -> NOT clean (commit `ecf6e39`, reverted)

| Oracle | IY-off (shipped) | IY-on |
|---|---|---|
| AES `09_Oz_prod_like` | PASS C010=01, 11.52M ts, 3715 B | **MISCOMPILE** C010=00 (deterministic) |
| test-runner clang | 696 / 37 / 56 | 694 / ~38 / ~57 |
| Z80 lit | 112 PASS + 5 XFAIL | 109 PASS + 3 FAIL |

Real test-runner regressions (rest is known `test_90/91` edge noise, #136):
`test_48_dynamic_alloca` FATAL all opt levels, `test_40_hash_crc`,
`test_38_sort_search`.  Lit shifts: `add16-acc` (`add iy,de`), `ldir-aftermath`
(reorder), `issue-156-bss-spill-loop-header-pop` (`push hl` reappears).

## Dig-in conclusion — TWO distinct bugs (corrected via instruction-level trace)

Reliable repros `test_167_iy_crc32` / `test_168_iy_crc_inner` (crc i32 reduction
loops); controls `test_169` (`(crc>>1)^const`) and `test_170` (cond-xor only)
both PASS, so the bug needs the select+shift+xor combination.

**Important:** the test-runner builds **without `+static-stack`** (SP-relative
frame), while production/AES use `+static-stack` (BSS locals).  These are
different codegen paths, so there are TWO distinct IY-unreserve bugs:

1. **`test_168` (non-`+static-stack`): SP-relative store aliases a pushed
   register.**  `z88dk-ticks -trace` of `crc_one(0xFF)` O1: the carried HIGH
   half `0xEDB8` is saved by `push hl`, but before its `pop hl` there is a second
   `push hl` plus an SP-relative store `ld hl,4; add hl,sp; ld (hl),a` whose
   offset lands on the saved HIGH half — clobbering its high byte `0xED -> 0x44`.
   The corrupted HIGH (`0x41B8`) shifts into IY as `0x20DC` (not `0x76DC`) and the
   error propagates to the wrong result `0x0044`.  Root cause: frame-index ->
   SP-offset computation does not account for the live pushed value; IY-unreserve
   triggers the nested-push spill shape.  This is frame-lowering, NOT a cost model.
   (My earlier `+static-stack`-asm "cost-model" reading was wrong for this path;
   corrected on #189.)

2. **AES (`+static-stack`): a separate, still-unreduced bug.**  No SP-relative
   frame (locals in BSS), so it cannot be bug #1.  Mechanism TBD.

`dynamic_alloca` (#190) is a third, frame-pointer class.

## Filed / tracked

- **#189** — IY-unreserve split-32-bit-in-IY regalloc miscompile (gates default-on).
- **#190** — IY-unreserve dynamic alloca FATAL (frame-pointer class).
- Backlog: `unpark-2026-05-22.md` "IY-unreserve default-on" with first-drill steps.
- Repros retained: `test_166/167/168` (test-runner), `iy-loop-carried-112.ll` (lit).

## Acceptance for the default-on flip (future)

`test_166/167/168` pass all opt levels + AES byte-correct + no new test-runner
FATAL/FAIL beyond `test_90/91` (#136).  The `-z80-unreserve-iy` flag stays as the
A/B switch.
