# Follow-up: close #77a by handling rotation-around-CALL spills

**Status:** open.  Tracked as **ravn/llvm-z80#100**.  Filed end of
session 35 (2026-05-02).
**Gates:** ravn/llvm-z80#77 / #77a (Z80LoopRotate default-on).
**Sibling:** ravn/llvm-z80#97 (closed in session 35) and #99
(i16-counter sub-case).

## Why this note exists

Session 35 closed #97 (BC ping-pong in rotated single-BB self-loops)
via a post-RA peephole.  When `-z80-loop-rotate=true` was flipped to
the new default to verify the close, measurement showed:

- rcbios BIOS:    5920 B → **5953 B** (+33 B regression).
- cpnos-rom:      1708 B → **1712 B** (+4 B regression).
- `_netboot_mpm`: 224 B → **252 B** (+28 B alone — biggest hit).

Default flipped back off.  The peephole stayed in (it still helps
PROM0 hand-written Case 1 shapes by -1 B).  This file is the
breadcrumb that says "next time someone touches loop rotation, this
is the second gate."

## Root cause

Rotated loops that contain a CALL force the register allocator to
keep the loop carrier (counter or pointer) live across the CALL.  On
+static-stack, the carrier ends up BSS-spilled before the CALL and
reloaded after.

Concrete repro (from cpnos-rom `netboot_mpm`, 2026-05-02):

```c
const uint8_t *s = ...;
for (uint8_t i = 0; i < 23 && s[i] != 0; ++i)
    impl_conout(s[i]);          /* CALL inside the loop */
```

### Rotate-OFF body (28 B)

```
247: ld   a,c             ; head test
248: cp   $17
24a: jr   z, exit
24c: ld   hl,(base)
24f: add  hl,bc            ; pointer = base + i
250: ld   de,$ffe8
253: add  hl,de
254: ld   a,(hl)            ; s[i]
255: ld   d,a
256: or   a
257: jr   z, exit
259: push bc                ; save i across CALL
25a: call impl_conout
25d: pop  bc
25e: inc  bc
25f: jr   $247
```

`PUSH BC; CALL; POP BC` — 3 B of save/restore around the call.

### Rotate-ON body (47 B)

```
250: ld   bc,$0
253: ld   (slot1),bc        ; spill bc
257: ld   (slot2),bc        ; spill bc again??
25b: call impl_conout
25e: ld   bc,(slot2)        ; reload
262: inc  bc
263: ld   a,c
264: cp   $17
266: jr   z, exit
268: ld   hl,(base_slot)    ; reload base
26b: ld   de,(slot1)        ; reload offset
26f: add  hl,de
270: inc  hl
271: ld   a,(hl)
272: ld   d,a
273: or   a
274: ld   (slot1),bc        ; spill bc
278: jr   nz, $257
```

`LD (slot),bc` (4 B) + `LD bc,(slot)` (4 B) chain instead of a 1+1 B
PUSH/POP pair.  Rotation pushed BC's live range across more
instructions, regalloc concluded the spill cost was unavoidable, and
the BSS-spill-vs-PUSH/POP late-opt peephole (lines 4446+ in
Z80LateOptimization.cpp) didn't trigger because the spill is reused
across the back-edge, not just across a single CALL.

## Possible fixes

1. **Extend the BSS-spill→PUSH/POP peephole** (lines 4446+) to
   recognise the cross-back-edge form: a spill written before a CALL
   and reloaded after, where the slot is also written at the back-
   edge.  The single-iteration shape still works as PUSH/POP if the
   back-edge writeback mirrors the post-CALL value.  Hardest part:
   the cross-iteration safety proof.

2. **Regalloc cost-model tweak.**  When the register class is GR16
   and the live value is a cheap rematerializable form (constant /
   `LD r,(slot)` adjacent to the use), rematerialize across the CALL
   instead of spilling.  Rough sibling of #15 (rematerialise
   constants held in IX) but extended to BSS-resident counters.

3. **Pre-rotation regalloc hint.**  Before Z80LoopRotate runs, hint
   the loop carrier toward a callee-saved equivalent (Z80 has none
   under sdcccall(1), but PUSH/POP behaviour can be modeled as a hint
   that biases the allocator to short-lived non-callee-saved
   registers anyway).  Pre-RA shape change.

4. **Simplest:** keep rotation default-off, opt in per-function via
   `__attribute__((annotate("z80_rotate")))` or similar, only for
   loops where the cost model favors it.  Defers the regalloc work.

## Where to look next

- `llvm/lib/Target/Z80/Z80LoopRotate.cpp` — the gate (default
  off), with an inline comment pointing to this file.
- `llvm/lib/Target/Z80/Z80LateOptimization.cpp:4446` — the
  BSS-spill→PUSH/POP peephole that needs extending for option (1).
- `llvm/test/CodeGen/Z80/issue-77a-loop-rotate.ll` — has both ROT
  and NOROT RUN lines; both pass today behind the explicit
  `-z80-loop-rotate` flag.
- `llvm/test/CodeGen/Z80/hl-no-bc-backup.ll` — the existing #84
  control case.  Useful sanity for any regalloc tweaks.

## How to measure

```sh
# Baseline (rotate off).
cd rc700-gensmedet/cpnos-rom && rm -rf clang && make cpnos | grep payload:
cd ../rcbios-in-c && make -C clang clean && make clang-bios | grep BIOS:

# With rotation on (need EXTRA_CFLAGS scaffolding in both Makefiles
# OR temporarily flip the cl::init in Z80LoopRotate.cpp).
EXTRA_CFLAGS='-mllvm -z80-loop-rotate=true' make ...
```

cpnos-rom `_netboot_mpm` is the easiest visible canary; rcbios
spreads the regression over many functions but `bios_w_block` and the
BDOS dispatch loop are also rotated.

## Done when

- `Z80LoopRotate` `cl::init(false)` flipped to `cl::init(true)`.
- rcbios BIOS at or below 5920 B.
- cpnos-rom payload at or below 1708 B.
- `issue-77a-loop-rotate.ll` second RUN line drops the `-z80-loop-
  rotate=false` flag (default suffices) — or switches to using the
  default and explicitly passes `=false` for NOROT.
- This file deleted (or renamed to a session summary documenting
  what closed it).
