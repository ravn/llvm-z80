# Open-issue triage — 2026-05-02 (end of session 32)

State: 28 open / 30 closed.  Branch `z80-close-all-issues` is 16
commits ahead of `main`.  This document classifies every open issue
by effort + path-to-close, so the next session can plan attack order.

## Legend

- **S** — small.  Single-MBB peephole or local rewrite.  Few hours.
- **M** — medium.  New IR pass, legalizer custom case, or cross-MBB
  peephole.  Half a day to a day.
- **L** — large.  Regalloc cost model, target-aware analysis, or
  cross-pass interaction.  Multi-day.
- **X** — extra-large / infrastructure.  Calling conv, libc, lexer
  subclass.  Bigger than a session.

## Tractable now (S/M)

These are the next session's natural targets.

### #74 — Push/pop spills instead of BSS for short-lived 16-bit values [S/M]

Same family as #82 (just closed).  When a 16-bit value is BSS-spilled
across a single CALL with no other slot reads, replace the spill +
matching reload with PUSH/POP.  `Z80LateOptimization` already has a
similar peephole for 8-bit; needs extension for 16-bit + per-pair
opcode mapping.  Expected impact: a few B per call site.

### #18 — Known-value register copy (cross-MBB) [M]

Extends the per-MBB known-A peephole (#60) to track register values
across BB boundaries.  Needs a small dataflow pass or an extension to
the existing late-opt's `Known` state.  Not regalloc-touching.

### #50 — Unroll memcpy/memmove into LDI chains for speed-critical paths [S]

For small constant-length memcpy (N=2..6 typical), an unrolled chain
of `LDI` (or just `ldi; ldi; ...; ldi`) is faster than `LDIR` (LDIR
has a 5T setup penalty per repeat).  Pure speed optimization.  Needs
opt-level gating (`-O3` or `-mtune=...`).  Doesn't help -Oz.

### #53 — `+static-stack` allocates trivially-constant locals to BSS [S]

`uint8_t local = 7; use(local);` on +static-stack allocates the local
in BSS instead of folding to the immediate.  Likely an InstCombine /
SROA gap.  Should fold without backend involvement.

## Blocked on regalloc cost model / `MachineLoopInfo` [L]

These are the "DJNZ family" issues that all share the same root:
the regalloc lacks loop-aware lifetime analysis.

- **#92** Nested-loop DJNZ direction reversed (outer gets B, should
  be inner).  Needs `MachineLoopInfo` in the hint logic.
- **#94** Sequential loops: B not re-hinted between loops.
- **#89** Loop-invariant 16-bit constant rematerialized into the
  loop body instead of preserved across iterations.  Pre-regalloc
  rematerialization heuristic prevents the hint.
- **#77** 8-bit countdown loop: counter ends up in non-B reg, no
  DJNZ.  Same root as #89.
- **#95** Long-term: prevent IV rewrite from countdown to count-up
  at -Oz / target-aware.  IR-level fix that would subsume parts of
  #77 / #93's path b.

Suggested approach: a dedicated session that pulls `MachineLoopInfo`
into `Z80RegisterInfo::getRegAllocationHints`, plus tunes the
`shouldAvoidRematerialization` cost.  All five issues likely close
together.

## Blocked on calling conv / IX vs static-stack work [L]

- **#16** PUSH/POP instead of IX-indexed spills across CALLs.
- **#12** hasFP=false correct but larger (parked).
- **#27** Per-pair 16-bit register copy cost.
- **#40** Evaluate IX frame pointer vs static-stack BSS per-function.
- **#15** Loop index→pointer conversion (would help #89).

These touch the IX frame pointer subsystem.  Need a unified design
session before piecemeal changes.

## Correctness bugs [L, but each can be tackled separately]

- **#28** -O0 codegen failures in large functions.
- **#36** va_arg produces incorrect code -- printf broken.
- **#37** Undocumented `LD A,IYH` emitted for sign-extension without
  `+undocumented`.
- **#38** Large function codegen incorrect without `+undocumented`
  (IY PUSH/POP sequences).
- **#39** IX constant propagation removes setup when `+undocumented`
  sub-reg reads present.
- **#63** `bench_string` fails at -O0 only (de=045C, expect 00FF).

Each is an isolated codegen bug.  Tractable individually but each
needs reproducer + minimization + targeted fix.

## Multi-value spill (#20) [M, related to closed #82]

`fdc_write_full_cmd` spills two values across CALLs.  The peephole
that closed #82 handles single-register multi-load shapes; #20 is
when multiple register-pairs are spilled to neighbouring slots.
Could be a follow-up extension of the same peephole, but the SDCC
canonical (pack into BC, push/pop BC) is structurally different
from BSS-spill rewrites.

## Workaround-exists / low-priority [S, but close as wontfix?]

- **#4** `__critical` (DI/EI wrapper for atomic sections).
  Workaround: empty macro + manual `__asm__("di"/"ei")`.  Low
  priority -- the macro form is what cpnos uses.
- **#42** Built-in intrinsics for DI/EI/HALT/IM2/`LD I,A`.  IR
  intrinsics exist in `IntrinsicsZ80.td` but no `__builtin_z80_*`
  Clang surface.  Workaround: `static inline` wrapper functions in
  a project header.
- **#70** `-fverbose-asm` doesn't annotate.  QoL feature; not a
  correctness or size issue.
- **#35** No standard libc.  Design choice, not a bug.
- **#43** Custom calling convention for BIOS entry points.  Feature.
- **#7** Umbrella for "instruction-driven codegen".  Most child
  instructions (DJNZ, LDIR, LDDR, CP (HL), EX DE,HL, BIT, ADD HL,HL)
  are working.  CPIR/CPDR not yet done.

These could be closed as wontfix-with-workaround OR kept open as
low-priority backlog.  Decision deferred to user.

## Lexer-level [X]

- **#81** Apostrophe in `ex af,af'`.  Investigated this session;
  needs a Z80AsmLexer subclass to override LLVM's generic
  `LexSingleQuote()`.  Workaround `.byte 0x08` is mechanical and
  documented; future fix is "real but invasive".

## Closed this session (verified)

For posterity; all have lit test coverage on the branch.

- **Bug fixes (with code)**: #78, #88, #64, #91, #82, #76, #93, #86.
- **Retroactive closes**: #65, #67, #68, #69, #71, #75, #79, #83, #84,
  #85, #87, #73, #80, #60, #90.
- **New issues filed**: #91 (closed same session), #92, #93 (closed
  same session), #94, #95.

## Sizes (final)

- rcbios BIOS:    5998 → **5967 B** (-31 B, -0.52%).  Smallest yet.
- cpnos-rom payload: 1738 → **1730 B** (-8 B).
- Z80 lit suite: 65/66 + 1 XFAIL → **73/73**, no XFAILs.
