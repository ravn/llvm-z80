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

2. **`+static-stack` `-O1`/`-Os`: i32 select+shift+xor loop miscompiles,
   INDEPENDENT of IY (ravn/llvm-z80#192).**  Reliable memory-dump harness
   (validated: trivial i32 +static-stack correct; non-static crc correct):
   `crc_one(0xFF)` returns `0xB6662D3D` instead of `0x2D02EF8D` at O1/Os with IY
   reserved.  Same select+shift+xor combination requirement as bug #1; same
   i32-loop-carried-spill defect, but via the BSS spill path under `+static-stack`
   (bug #1 is the SP-relative path under IY-on).  **This is the most important: it
   affects the shipped compiler in the production config (`+static-stack -Os`),
   no IY work required.**  The test-runner never caught it because it built only
   without `+static-stack` -- now fixed by the new `cargo run -- clang -static-stack`
   mode (commit `2d6ccd4`), which reproduces #192 via `test_168` (`_ss`, FAIL O1/Os).

`dynamic_alloca` (#190) is a fourth, frame-pointer class.

## Issues filed this session
- **#189** IY-unreserve SP-relative-store-vs-push (test_168 non-static, IY-on).
- **#190** IY-unreserve dynamic_alloca FATAL.
- **#191** llvm-objdump can't auto-detect Z80 ELFs (ELFObjectFile::getArch has no
  EM_Z80 case; e_machine=8080 IS the correct EM_Z80 per LLVM ELF.h -- my initial
  "220" claim was wrong).
- **#192** +static-stack -O1/-Os i32 select+shift+xor loop miscompile (production
  config, IY-independent) -- the headline finding.

## #192 ROOT-CAUSED (instruction trace + per-pass MIR bisection)

`crc_one(0xFF)` +static-stack -O1/-Os IY-off: the `select(crc&1 ? CONST : 0)`
**always takes the xor path** (bc=EDB8 in all 8 iters; iters 1-6 happen to be odd
so correct, 7-8 even so wrong -> 0xB6662D3D).

The i32 `icmp eq (and crc,1),0` is selected as two `XOR_CMP_EQ16` (per 16-bit half)
AND-combined.  Bisection: post-RA CORRECT; z80-late-opt INPUT correct; z80-late-opt
OUTPUT broken.  **A z80-late-opt BSS store-to-load forwarding peephole eliminates
flag1's BSS spill/reload and relocates its write `LD_D_A` (D := flag1) to BEFORE
compare2.  But D is compare2's hi-byte input (set 0 by LD_D_n 0); the clobber makes
compare2 compute `(flag1==0)=NOT(flag1)`, and the i32-== AND becomes
`NOT(flag1) AND flag1 = 0` -> branch always falls to the CONST block.**  Same family
as #189: a late-opt transform moving an instruction without a liveness guard on the
destination register.  (CMP_Z16-const-fold's `LD_B_D` only changes surface form.)

**FIXED** (commit `25656201a41d`).  The exact culprit (via asserts-build
`-debug-only=z80-late-opt`) was the **#173 peephole** ("bare BSS store + 4-instr
A-preserving reload -> LD r,A; PUSH/POP rr"): it relocated flag1's `LD D,A` write
to the store site and bracketed with PUSH/POP DE to preserve D AFTER the region,
but never checked that D is READ in the region (compare2 reads D as its zero input).
Fix: track 8-bit regs READ in the [store, reload) interval (mirroring DefinedRegs)
and bail if the dest register is read.  Same family as #189 (late-opt move without
a liveness guard).

Oracle (full): test_167/168 -static-stack now PASS all opt levels; AES corpus
byte-identical (C010=01, 11516046 ts, 3715 B); cpnos PROM1 byte-identical (2028 B)
-> MAME boot unaffected; Z80 lit 113+5; test-runner default 720/37/56 (no
regression -- #173 is +static-stack-only).  Lit guard `static-stack-i32-select-192.ll`.

Separate finding while validating: a PRE-EXISTING `Z80LateOptimization` **segfault**
on test_40's `xorshift16` +static-stack (the "BSS spill->PUSH/POP" peephole on a
16-bit DE spill), confirmed via the assertions build (predates this fix).  Filed as
**ravn/llvm-z80#193**.

## Test-runner additions
test_166 (popcount), test_167 (crc32), test_168 (crc_inner), test_169 (uncond
xor control), test_170 (cond-xor control); new `-static-stack` run mode.

## Filed / tracked

- **#189** — IY-unreserve split-32-bit-in-IY regalloc miscompile (gates default-on).
- **#190** — IY-unreserve dynamic alloca FATAL (frame-pointer class).
- Backlog: `unpark-2026-05-22.md` "IY-unreserve default-on" with first-drill steps.
- Repros retained: `test_166/167/168` (test-runner), `iy-loop-carried-112.ll` (lit).

## Acceptance for the default-on flip (future)

`test_166/167/168` pass all opt levels + AES byte-correct + no new test-runner
FATAL/FAIL beyond `test_90/91` (#136).  The `-z80-unreserve-iy` flag stays as the
A/B switch.
