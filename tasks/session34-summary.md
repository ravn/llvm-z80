# Session 34 — source-cleanup audit + Z80LoopRotate + #97/#98 investigation

Date: 2026-05-02 (continuation of session 33's regalloc-cluster merge).

## TL;DR

No size win shipped this session — but **three new issues filed with thorough
investigation**, one **target-specific pass landed (gated off)**, and **two new
lit tests** (one PASS, one XFAIL) that pin the bugs the next session will
close.  Lit suite 75/75 → 77/77 (76 PASS + 1 XFAIL).  rcbios BIOS unchanged
at 5920 B, cpnos-rom payload unchanged at 1708 B.

The session opened with a source-cleanup audit (no code change needed — the
hand-written countdown idioms in cpnos-rom are still the smallest form),
took a swing at #77a (head-test loop rotation), exposed #97 (BC ping-pong)
as a gating dependency, and ended with a thorough investigation of #98
(regalloc B-dead-after-DJNZ) in service of the still-open #94.

## Issues filed

- **#96** — Investigation: regalloc-level PUSH/POP spilling for short-lived
  16-bit values (layer 3).  Documents the three sub-problems (LIFO
  StackObject modeling, cross-BB bracketing, CALL boundary balance) and
  three valid outcomes for the spike.
- **#97** — BC ping-pong: PHI'd pointer in single-BB self-loops gets
  allocated to two register pairs simultaneously.  Sibling of the closed
  #84.  Root cause confirmed at MIR level: GISel store implicit-def $hl.
- **#98** — Investigation: regalloc doesn't model B as dead between
  sequential DJNZ-eligible loops.  Sibling of #94 (the visible symptom).
  Lists facts already discovered, two sub-questions to dispatch, and
  concrete starting points (specific files + flags).

## Pass landed

**Z80LoopRotate** (`llvm/lib/Target/Z80/Z80LoopRotate.{cpp,h}`, ~170 LOC):

- Calls LLVM core's `LoopRotation()` utility directly with a non-zero
  threshold to bypass `LoopRotation.cpp:72`'s minsize gate.  At -Oz, LLVM
  core forces the rotation threshold to 0 for any function with the
  `minsize` attribute, so head-test do-while-decrement loops never get
  rotated even when the rotated form is strictly smaller on Z80.
- Modeled on `Z80LoopIdiomFill` shape (legacy `FunctionPass` + new-PM
  `PassInfoMixin` entry points).  Wired into clang's PassBuilder
  (registerVectorizerStartEPCallback) and llc's addIRPasses.
- **Gated off by default** behind `-z80-loop-rotate` CLI flag.  Reason:
  rotating exposes #97 (BC ping-pong) and currently regresses cpnos-rom
  payload by +4 B.  When #97 closes the default flips to true.

## Lit tests added

- **`issue-77a-loop-rotate.ll`** (PASS) — exercises Z80LoopRotate with
  both flag positions.  ROT prefix verifies rotation produces the
  flag-using `jr nz` shape; NOROT prefix verifies the head-test shape
  is preserved when the flag is off.
- **`issue-97-bc-pingpong-singlebb.ll`** (XFAIL) — five functions
  covering the bug shapes:
  1. canonical do-while-decrement with i16 store
  2. byte-fill variant
  3. i16 counter variant
  4. GEP-after-store IR ordering (proves IR-level fix doesn't work)
  5. positive control: head-test multi-BB form (already passes via #84)

## Investigations completed

### #77a (head-test loop rotation)

Root cause confirmed: `LoopRotation.cpp:72` gates rotation on
`Function::hasMinSize()`.  At -Oz, threshold forced to 0 → no rotation
regardless of header size.  Fix path: target-specific Z80LoopRotate that
bypasses the gate.  **Implemented and gated off pending #97.**

Sub-bug split documented on #77:
- **77a** — cross-BB redundant `or a` test.  Closed by Z80LoopRotate.
- **77b** — counter doesn't land in B.  Blocked by #97 (BC ping-pong
  occupies BC, leaving B unavailable for the counter).

### #97 (BC ping-pong) — IR-level fix path tried and disproven

Hypothesis: SSA-level reorder of the back-edge GEP to AFTER the last use
of the PHI'd pointer would break the live-range overlap and let the
coalescer fold the PHI copy.

Implementation: `sinkGEPsToAfterPHIUses` in Z80LoopRotate that moves
GEPs whose only use is the back-edge PHI past the last user of the PHI
input.  IR check: success — post-sink IR has GEP after store, no
overlap.

Result: **+4 B regression on cpnos-rom remained**.  Even with clean
non-overlapping live ranges at IR level, the regalloc still picks BC
for the long-lived pointer.

Root cause: GISel store sequences (`LD_HLind_E; INC_HL; LD_HLind_D`)
have `implicit-def $hl` on each instruction.  The physreg `$hl` is
clobbered across the stores regardless of what vreg wants to live there.
So the regalloc is forced to allocate any PHI'd-pointer vreg that's
live across the stores into a NON-HL pair (BC or DE), then COPY to/from
HL for the actual stores, then increment the non-HL pair at end of body.

Three remaining fix paths (documented on #97):
1. Rework GISel store emission so post-store HL is an explicit output.
2. Post-RA peephole that catches the post-rotation BC ping-pong shape
   (sibling of the existing #84 peephole).  **Pragmatic next step.**
3. Coalescer hint for PHI'd pointers in single-BB self-loops.

GEP-sink reverted from Z80LoopRotate.  Pass remains rotation-only.

## Source-cleanup audit (cpnos-rom)

Walked the closed-issue list against `cpnos-rom/init.c` candidate
sites.  Two countdown idioms (IVT setup and port_init dispatch) were
audited under the production flags `-Oz -mllvm -disable-lsr +static-stack`:

- IVT loop (init.c:119-125): current 14-byte body wins by 7 B vs
  idiomatic up-counter forms (which incur a BSS spill due to GEP type
  promotion).
- port_init loop (init.c:144-150): same pattern, same conclusion.

**No source change.**  The hand-written countdown comments are still
correct workarounds — but their justification has shifted: it's no
longer the LSR rewrite (which `-disable-lsr` already neutralizes), it's
the head-test shape (#77a) and the dec-via-A bug (#77b) combined.

Findings recorded in `tasks/source-cleanup-vs-closed-issues.md` so the
next pass doesn't re-walk these sites.

## Unrelated discovery: #95 isn't biting production

Filed in session 33 as the long-term path-a fix for #93 (carry-roundtrip
elimination), #95 turned out to be **already neutralized by the
production flags**.  All three project Makefiles pass `-mllvm
-disable-lsr`, which prevents LSR's countdown-to-count-up rewrite from
ever firing on cpnos-rom or rcbios.  #95 stays open for the day LSR is
re-enabled (e.g., for performance-tuned builds), but it's not on the
cpnos-rom critical path.

This corrected an earlier framing in session 33's notes: the comment
"#95 IV-rewrite blocks DJNZ even when source uses countdown" was based
on incorrect baseline (no `-disable-lsr`).  The actual production
behavior with countdown source is shape A: `dec a; ld d,a; or a; ret z`
— which is #77, not #95.

## Sizes (final)

- rcbios BIOS:    **5920 B** unchanged.
- cpnos-rom payload:  **1708 B** unchanged.
- Z80 lit suite:  75/75 → **76 PASS + 1 XFAIL** (77 total).
- Open issues:    23 → **26** (no closes; +3 new investigation/sibling
                  issues).

## Open at session end

- **#94** (sequential DJNZ) — depends on #98 spike.
- **#96** (layer 3 PUSH/POP) — investigation, no urgency.
- **#97** (BC ping-pong) — gating #77a's payoff; pragmatic fix is
  option 2 (post-RA peephole sibling of #84).  Lit test ready.
- **#98** (regalloc B-dead-after-DJNZ) — investigation; spike with
  documented commands and flags.

## Next session recommendation

**#97 fix path 2** (post-RA peephole that catches the rotated BC
ping-pong).  The lit test is ready and will validate immediately.
Closing #97 unlocks the Z80LoopRotate default flip, which closes #77a
and gives ~6 B on cpnos-rom.  The peephole is structurally analogous to
the existing #84 peephole (~110 lines), so the implementation is
well-scoped.

Alternative: **#98 spike** (1-2 hours, no code change) to dispatch the
sub-questions about regalloc B-dead modeling for sequential DJNZ loops.
This unblocks #94's implementation but doesn't ship size on its own.

## Bookkeeping

- Branch: still on main; this session's three commits already merged.
- No untracked changes at session end.
- Tasks list cleaned: only `#72` (now subsumed by #98) remains pending.
