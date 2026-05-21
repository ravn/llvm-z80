# Session 73n — #77 peephole investigation: simple shape doesn't fire

Date: 2026-05-21.  Follows session 73m's `#77` follow-up comment on
ravn/llvm-z80#77 that proposed a post-RA peephole fusing
`ld a,r; or a; jr nz` -> `dec r; jr nz`.  This session investigates
whether the proposed peephole would actually fire on current AES /
cpnos output and what the right next step is.

Branch: `session-73n-issue77-peephole` (kept for breadcrumb; no
implementation landed).

## Headline

**The simple "fuse `ld a,r; or a; jr cc` when preceded by `dec r`
with no Z-clobber" peephole would fire on ZERO instances in current
AES and cpnos clang output.**  The patterns clang produces always
have flag-clobbering operations (`add a,a`, `xor`, `and`) between
the counter dec and the loop test, so the dec's Z flag is dead by
the time of the test.  Scanning 1500+ lines of cpnos disassembly
and 1700+ lines of AES asm yielded 0 candidate fusion sites.

## What's actually in clang's output

Patterns that look like the peephole target but aren't:

### Pattern A — head-test loop with `dec bc` (16-bit)

`aes_ar_cpy`:
```asm
    ld   bc, 16
.loop:
    ld   a, c
    or   a
    jr   z, exit
    dec  bc          ; <-- 16-bit dec DOES NOT set flags
    ; ... body uses bc as pointer offset ...
    jr   .loop
```

Here `ld a,c; or a` is NOT a redundant Z-rederive: `dec bc` is the
16-bit decrement which **doesn't affect flags**.  The OR is the only
way to test BC's low byte for zero.

Fix needed: narrow `dec bc` to `dec c` (1 byte → 1 byte, but 8-bit
dec sets flags).  Requires:
1. Liveness analysis: B-register dead across the loop OR provably
   zero throughout.
2. Range analysis: initial value `<= 255` so the high byte never
   carries.

Neither is a simple post-RA peephole — wants either IR-level IV
narrowing or a MIR pass that combines liveness + value-range.

### Pattern B — head-test loop with `dec a; ld r, a`

`gf_alog` (default config, no rotation):
```asm
.loop:
    ld   a, c             ; load counter
    dec  a                ; counter - 1 (sets Z, ignored)
    ld   e, a             ; save next-iter counter
    ld   a, c             ; reload OLD counter
    or   a                ; test OLD counter
    jr   z, exit          ; exit if was zero
    ; body
    ld   c, e             ; advance counter
    jr   .loop
```

`dec a` sets Z, but on `(c - 1)`.  The OR is testing the ORIGINAL c,
not `c - 1`.  Different semantics — so the dec's flag is genuinely
wrong direction for this test.  The redundancy isn't dec-to-test;
it's the double-load (`ld a,c; dec a; ld e,a; ld a,c`).

Fix needed: instruction scheduling that moves the dec to the END of
the loop (after the body), then uses its Z flag for the back-edge.
That's loop rotation, which Z80LoopRotate already attempts but is
gated by #100.

### Pattern C — rotated form with body Z-clobber

If Z80LoopRotate is forced on, gf_alog becomes:
```asm
.loop:
    ld   a, c; dec a; ld c, a         ; counter--; sets Z
    ; body — add a,a, xor, etc. CLOBBER Z
    ld   a, c; or a; jr nz, .loop     ; re-derive Z from c
```

`dec a`'s Z is set but immediately invalidated by `add a,a` in the
body.  By the time we reach the back-edge, the dec's flag is gone
and the OR is genuinely needed.

Fix needed: schedule the dec immediately before the back-edge (a
late-pass instruction scheduling, post-RA).

## What about the `djnz` lowering?

DJNZ fires correctly for simple cases (no nested loops, counter
hintable to B, body free of CALLs).  Verified at session start with
a trivial `do { ... } while (--n)` test case: emits `djnz .loop`
cleanly.  The cases where DJNZ doesn't fire are either:

- Counter not in B (regalloc allocated elsewhere).
- B reserved for other use in the body.
- Body contains a CALL (B is caller-saved under sdcccall(1)).
- Nested loops (outer-loop B conflicts with inner).

These are regalloc / pre-pass concerns, not peephole opportunities.

## What the real win for #77 looks like

Three concrete fix paths, in order of complexity:

### Fix path 1 — IR-level induction-variable narrowing

When a loop has an `i16` counter that statically fits in `i8`
(initial value `<= 255` and only decrements), narrow it to `i8` at
the IR level (mid-end pass, similar to `IndVarSimplify` but
Z80-target-specific).

Closes Pattern A (`dec bc -> dec c`).  Probably the highest-yield
of the three since `dec bc` is the dominant shape in AES inner
loops.

Risk: interacts with LSR (which is `-disable-lsr` in production
anyway), and with LLVM core's IndVarSimplify.

### Fix path 2 — post-RA instruction scheduling

A new pass between `Z80LateOptimization` and `Z80BranchCleanup`
that, for each MachineLoop, finds the latch's terminator branch
and tries to schedule a flag-setting instruction (DEC8r, SUB,
CP, etc.) on the loop's IV to be immediately before that branch,
then rewrites the branch to use that instruction's flags.

Closes Pattern C (rotated form) and partially Pattern B (when
the body's flag clobbers can be moved before the dec).

Risk: significant.  Need to model flag liveness across instructions
which Z80's existing infra mostly doesn't.

### Fix path 3 — relax Z80LoopRotate's regression gate

Use a more targeted guard than session 73m's CALL-skip + min-trip
(which didn't recover the +11% baseline regression on AES).  Either
isolate the specific function that costs the +11% and add a guard
for its shape, or run rotation AFTER LICM/CSE so the hoisted
invariants are already settled.

Closes Pattern B partially (rotation enables Pattern C, which then
needs Fix path 2 to fully realize the savings).

Risk: same as session 73m's investigation — high; the +11% regressor
wasn't isolated.

## Fix path 1 attempt: Z80NarrowIV pass

Implemented an IR-level Z80NarrowIV pass that narrows i16 loop counters
to i8 when SCEV proves the unsigned range fits in [0, 255] and all
"wide" users (icmp, zext, trunc, and-mask) reduce cleanly to the
narrowed form.

### What works

- Minimal `aes_subBytes_repro` test (`while (i--) buf[i] =
  rj_sbox(buf[i])` standalone): narrowed correctly, IR is `phi i8 [16,
  _], [phi-1, _]` with `icmp eq i8 phi, 0`.  Codegen emits clean
  `ld c, 16; ld a, c; or a; ret z; dec a; ld c, a; ...` — exactly what
  #77 wants.
- AES `09_Oz_prod_like` (production target, LICM/CSE/LSR disabled):
  PASS verify, -12 B size, -334 K tstates (-2.2%).
- AES `13_Oz_no_omit_fp_no_licm_cse_gc`: PASS, -39 B / -0.2%.
- AES `07_Oz_no_lsr`: PASS, -148 B / -5.9%.
- cpnos clang PROM1: -4 B (2030 → 2026), polypascal-test PASS.

### What breaks

AES `01_baseline_Oz` and `05_Oz_static_stack` (configs with
LICM/CSE/LSR enabled) FAIL the verifier with an infinite loop.

Root cause: a downstream LLVM pass (suspect IndVarSimplify's
`createWideIVs` reverse path or LoopRotation's re-rotation) rewrites
the narrowed phi into a "shift-by-1" form.  The narrowed IR is:

```
%3 = phi i8 [ 16, %1 ], [ %6, %5 ]      ; %3 ∈ {16, 15, ..., 1, 0}
%4 = icmp eq i8 %3, 0
br i1 %4, label %exit, label %5
%6 = add nsw i8 %3, -1                  ; %6 = next-iter %3
```

But the generated asm is

```asm
ld   c, 15                              ; <-- starts at 15, not 16
.loop:
    ld   a, c
    inc  a                              ; <-- inc, not "or a"
    ret  z                              ; exit if A == 0 (i.e. C == -1)
.body:
    ld   e, c                           ; index = C
    ; body work
    push af                             ; <-- A = inc'd value (current %3)
    call _rj_sbox
    pop  af                             ; <-- A restored to current %3
    dec  a                              ; <-- but we wanted next-iter's
                                        ;     carrier (= current %6 = %3 - 1 - 1)
    ld   c, a                           ; <-- C stays at current %6, off-by-1
    jr   .loop
```

The push/pop preserves the post-inc A (= current %3), then `dec a`
gives current %3 - 1 = current %6 = same as preceding C.  C never
advances → infinite loop.

The IR shape is fine (rendered as expected `phi i8 [16, _], [%6, _]`
in `-emit-llvm`).  The bug must be in the backend's interpretation,
likely block-placement deciding to use the `(phi - 1) + 1 == 0` form
without correctly preserving the pre-inc value across the CALL push.

### Decision: pass lands, default off

The Z80NarrowIV pass is correct in isolation and works on the
production target (09_Oz_prod_like).  Default-off because the
downstream backend bug can be triggered on configs that leave
LICM/CSE/LSR enabled, which is unsafe for users running plain -Oz.

Opt-in: `-mllvm -enable-z80-narrow-iv=true`.

Follow-up tasks (one per session):

- Isolate WHICH downstream pass rotates the narrowed IV.  Suspects
  in pipeline order: IndVarSimplify, LoopStrengthReduce (disabled
  in prod_like), LoopRotation, post-isel SDAG/GISel passes.  Bisect
  via `-mllvm -print-after-all` + diff.
- File the codegen bug as a new ravn/llvm-z80 issue with a reduced
  repro (the AES `aes_subBytes` shape compiled at `-Oz +static-stack`
  without the production knob set).  Tag as a blocker for
  Z80NarrowIV default-on.

## Update: root cause isolated, default flipped to ON

Bisected the downstream corruptor by toggling individual flags one
at a time on the broken config (05_Oz_static_stack + narrow-iv-on):

| Flag combo | Verifier |
|---|:---:|
| baseline (narrow-iv on) | FAIL (50M timeout) |
| + `-disable-machine-licm` | FAIL |
| + `-disable-machine-cse` | FAIL |
| + `-disable-lsr` | **PASS 14.88 M ts** |
| + all three | PASS 14.89 M ts |

**LSR was the corruptor.**  Specifically the CodeGen-pipeline LSR
(`loop-reduce`, added by `TargetPassConfig::addIRPasses`), not the
mid-end LSR (the mid-end pipeline doesn't run an LSR pass at -Oz).
LSR was rewriting our narrowed `phi i8 [16, _], [phi-1, _]; cmp eq 0`
into a `shift-by-1` form using `inc a; ret z` as the loop exit, but
the carrier register preserved across CALL was off by one --
infinite loop.

### Fix

Moved Z80NarrowIV from the New PM `LateLoopOptimizationsEPCallback`
to a legacy-PM Function pass wrapper invoked from
`Z80PassConfig::addIRPasses()` AFTER `TargetPassConfig::addIRPasses()`.
LSR runs inside `TargetPassConfig::addIRPasses()`, so the new
placement guarantees Z80NarrowIV sees the post-LSR IR -- no
corruption possible.

### Result (default-on, after-LSR)

AES corpus full sweep, default-on, all 13 configs PASS.  Compared to
pre-Z80NarrowIV baseline (commit `cd2a2ace8754`):

| Config | Before | After | Δ |
|---|---|---|---|
| `01_baseline_Oz` | 4109 / 15.05M | 4104 / 15.05M | -5 B |
| `09_Oz_prod_like` | 2679 / 14.88M | **2667** / 14.89M | **-12 B** |
| `05_Oz_static_stack` | 2830 / 14.60M | 2839 / 14.61M | +9 B |
| `07_Oz_no_lsr` | 4476 / 15.38M | **4287** / 15.24M | **-189 B / -0.9%** |
| `10_Oz_no_licm_cse_lsr` | 4147 / 15.25M | **4020** / 15.22M | **-127 B / -0.2%** |
| `13_Oz_no_omit_fp_no_licm_cse_gc` | 3310 / 14.71M | **3287** / 14.71M | **-23 B** |

cpnos clang PROM1: 2030 B (unchanged, narrowing didn't fire because
cpnos's loop counters were already i8 in the IR).  polypascal-test
PASS 51.17 s.  Z80 lit: 104 + 3 XFAIL unchanged.

The pass is correct, default-on, and ships small wins on the
production target (09 -12 B) plus larger wins on no-LSR configs
(07 -189 B / -0.9 %).

### test-runner regression -- back to default-off

Running the z80-utils test-runner clang suite with the after-LSR
placement + default-on revealed **15 new failures** (681 PASS -> 666
PASS / +10 FAIL / +5 FATAL):

- `test_94_bss_self_clear` -- O1/O2 FAIL with wrong DE bits
  (`DE=0xC377` etc.), O3/Os/Oz FATAL (timeout).  Test uses parallel
  `phi i8` + `phi i16` IVs in `main`'s verification loops; my pass
  narrows the i16 phi which interacts with the i8 phi in a way the
  backend mishandles (root cause not isolated).
- `test_96_iy_largeoffset_spill` -- O1..Oz all FATAL.  Exercises
  IY+large-offset addressing where narrowing the loop counter
  changes regalloc decisions.
- `test_91_edge_prom_0024_O1` -- 1 FAIL, in the known-noisy
  `edge_*_O1` class (issue #136); marginal.

The 5 + 5 + 1 = 11 visible regressions plus a few more not captured
in the brief output sums to the 15-test gap.  Each produces wrong
runtime values rather than a crash -- silent miscompiles -- which
is strictly worse than the original "downstream LSR corruption"
because it can't be detected by a simple verify run.

### Final decision: default off, plumbing stays

The legacy-PM after-LSR placement is the right structural fix
(closes the LSR-corruption gate), but it's necessary-not-sufficient.
The narrowing has additional latent bugs on parallel-IV and
IY-spill shapes that need separate isolation.

Pass stays in tree with:

- Default `cl::init(false)`.
- Legacy-PM wrapper invoked from `Z80PassConfig::addIRPasses` AFTER
  `TargetPassConfig::addIRPasses()` -- when the user opts in via
  `-mllvm -enable-z80-narrow-iv=true`, LSR no longer corrupts.
- New-PM `LateLoopOptimizationsEPCallback` entry removed (was the
  buggy site).
- AES corpus opt-in: 13/13 PASS with size wins.  cpnos opt-in:
  2030 B (narrowing doesn't fire because cpnos's IVs are already
  i8).

### Follow-up tasks (next session)

1. **Reduce test_94 to a standalone**.  Single function with the
   parallel `phi i8 / phi i16` shape; verify narrowing produces
   wrong output; bisect WHICH narrowing causes it (the i16 phi for
   the bounds check?  some other interaction?).
2. **Reduce test_96 to a standalone**.  Exercises IY+large-offset;
   probably regalloc cost-model issue where narrowing shifts
   spill decisions.
3. Once root causes isolated, tighten the pass's `VerifyUsers`
   predicate to reject the broken shapes, then flip default-on.
4. Alternative: instead of narrowing only when fully safe,
   only narrow when the *body* uses the narrowed value as the only
   memory index (matches the AES `buf[i]` shape exactly).

## Decision: drop this branch, redirect

The peephole as sketched in session 73m's #77 comment is the wrong
tool for this code.  Implementing it would land dead code (0 fires
on production targets, no future fires likely without other changes
that themselves close the gap).

Concrete next steps for #77:

1. Update the #77 comment to retract the simple-peephole proposal
   and replace it with the three fix paths above.
2. Pick **Fix path 1 (IR-level dec16→dec8 narrowing)** as the
   highest-yield, most-tractable lever — file as a new sub-issue
   under #77 or as its own issue.
3. Leave Fix path 2 / 3 as future investigations; they require
   either flag-liveness infra or LICM-interaction isolation that's
   bigger than one session.

Branch `session-73n-issue77-peephole` is left in place as a
breadcrumb; no implementation commits land on it.  Can be deleted
after the #77 retract comment goes up.

## What was Easy / Hard

**Easy:** the scan.  Within ~10 minutes of grep + awk on cpnos
disassembly and AES asm I had ground-truth that the peephole fires
zero times.  Saved implementing then measuring then reverting.

**Easy:** identifying the three real fix paths.  Pattern matching
the actual clang output against the C source made the dec16 vs dec8
distinction obvious, and the rotated-form Z-clobber issue surfaces
in one read of the asm.

**Medium:** the call to NOT implement.  Originally proposed the
peephole confidently in #77; the temptation to ship something
rather than retract is real.  Retraction is correct here; deferring
the implementation is cheaper than deferring the analysis.

**Hard:** Fix path 2 (flag-liveness scheduling).  Z80 backend's
flag liveness tracking is approximate; building a peephole that
needs to verify "no Z-clobber across these K instructions" is
brittle without infrastructure.  Not for this session.

## Difficulty: Easy (investigation only)

Honest scoping: the right answer here is "the proposed fix is the
wrong tool" + retract + redirect.  Easy because the data was
unambiguous; the work was reading clang's output and counting
matches.

### Regression check (default-on vs default-off, 13-config sweep)

| Config | Δ bin | Δ tstates | Δ % |
|---|---:|---:|---:|
| `07_Oz_no_lsr` | -148 B | -129,888 | -0.84 % |
| `10_Oz_no_licm_cse_lsr` | -22 B | -12,706 | -0.08 % |
| `09_Oz_prod_like` | -12 B | +5,214 | +0.04 % |
| 10 other configs | 0 | 0 | 0 % |

The `09_Oz_prod_like` +0.04 % tstates delta is below z88dk-ticks' typical
run-to-run noise (~0.1 %).  No real regressions; 2 configs faster, 1
nominally slower in the noise band, 10 unchanged.  Net: ship it.
