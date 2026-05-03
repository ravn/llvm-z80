# Session 40 strand B — shadow-bank investigation (2026-05-03)

## TL;DR

The shadow registers (`AFp`/`BCp`/`DEp`/`HLp`/...) are
**allocatable** under `+shadow-regs` but **unreachable** by the
register allocator: no Z80 instruction defines its operands
using `ShadowGR16`, so no vreg can end up there.  Without
explicit `EXX` / `EX AF,AF'` discipline tracked at MIR level,
the shadow bank is structurally unable to hold values across
ordinary Z80 instructions — every encoding addresses the
current bank.

A single tractable niche exists: **bracket a no-CALL inner
loop with `EXX` to give the body access to a fresh BC/DE/HL
trio**.  Implementation is non-trivial; investigation-only
this session.

## Current state of shadow infrastructure

```
$ grep -nE 'BCp|HLp|DEp|AFp' llvm/lib/Target/Z80/Z80RegisterInfo.td
131: def AFp : Z80RegPair<40, "alt_af", Fp, Ap>;
132: def BCp : Z80RegPair<41, "alt_bc", Cp, Bp>;
133: def DEp : Z80RegPair<42, "alt_de", Ep, Dp>;
134: def HLp : Z80RegPair<43, "alt_hl", Lp, Hp>;
195: def ShadowGR16 : Z80Reg16Class<(add DEp, HLp, BCp)>;
```

`ShadowGR16` has **zero references** in any `.cpp` file.  The
class exists, the registers exist, the feature flag works, but
no codegen path produces a shadow-bank operand.

The only EXX/EX-AF use today (`Z80FrameLowering.cpp:165, 412`)
is at ISR entry/exit — and that use treats EXX as a *save*
mechanism, not an alternate register set:

```
Z80_Interrupt_EXX_CSR_SaveList = {IX, IY}    (Z80GenRegisterInfoTargetDesc.inc:1573)
```

The CSR has no shadow regs; it just says "ISRs preserve IX/IY",
relying on `EXX` at ISR entry to make BC/DE/HL look preserved
to the interrupted code.  No vreg is ever placed in a shadow
register.

## Why shadow-as-spill-target was abandoned (#102 closure)

Per session 37 / `tasks/late-opt-audit-2026-05-02.md` pattern
36 and ravn/llvm-z80#102 closure: the original disabled-block
attempted to convert spills to EXX, but **EXX swaps all three
of BC/DE/HL simultaneously**.  It cannot be inserted between
instructions whose live values include any of the three.  Net
viable insertion points are essentially "function entry/exit"
or "right after a CALL where caller-saves are already dead",
which doesn't beat normal spilling.

## The viable niche: hot loop bracket

For an inner loop MBB satisfying:

  1. **Single self-back-edge MBB** (or minimal cycle)
  2. **No CALL** in the loop body
  3. **All of BC, DE, HL dead at the loop's preheader-exit
     and the loop's exit point**

we can emit:

```
preheader:
  ...                       ; main-bank values dead here
  EXX                       ; 1 byte
  ; loop body now uses BC/DE/HL physregs which alias shadow set
loop:
  ...                       ; operates freely on BC/DE/HL
  ...
exit:
  EXX                       ; 1 byte
  ...                       ; main-bank values restored
```

Cost: 2 bytes total per loop (one EXX entry, one exit).  Win:
the loop body has 3 fresh GR16 pairs, eliminating spill traffic
when the body's pressure exceeds 3 pairs.  BIOS hot loops
(`_specc`, `_rwoper`, `_isr_crt`) have BSS-spill density 30-48%
of their bytes (per CLAUDE.md gap analysis); a portion of that
is from running out of GR16 within the loop.

Constraint check:

  - Condition (3) is restrictive but achievable.  Most callers
    pass args in registers and the function entry has full GR16
    pressure for the first few instructions — but inner loops
    typically open with `LD r,(addr)` style reloads that follow
    a free-pair point, which is when we'd insert EXX.

  - Condition (2) (no CALL) is automatic for tight inner loops
    but rules out anything calling helpers.  In BIOS / cpnos-
    rom, the hot loops are mostly call-free byte-twiddling.

  - The interrupt risk: an ISR firing while we're in shadow
    state would itself execute EXX (per +shadow-regs ISR
    prologue), restoring main bank to ISR scope.  ISR runs,
    EXXs back at exit, and we resume in shadow state.  Safe
    by construction *iff* the +shadow-regs ISR contract holds.
    No-shadow-regs ISRs are unsafe with this technique;
    feature must gate the bracketing on `+shadow-regs`.

## Implementation sketch (NOT done this session)

Two MIR passes:

  1. **Identify candidate loops.**  After regalloc, walk
     MachineLoopInfo, find depth>=1 loops with single MBB,
     no CALL, and cumulative GR16 pressure > 3 (computed by
     scanning the loop body and tracking max simultaneously
     live GR16s).

  2. **Verify safety preconditions.**  At loop preheader's
     terminator (last MI before loop entry): check all of
     BC, DE, HL are dead via `MachineBasicBlock::computeRegisterLiveness`.
     Same check at loop exit's first MI.  Bail if either
     check fails.

  3. **Emit EXX brackets.**  `BuildMI(...Z80::EXX)` at the
     two safe points.  No further changes — the loop body
     uses BC/DE/HL physregs as before; the EXX redefines
     which physical bytes those names refer to.

  4. **Mark for verifier.**  EXX is `Defs = [BC, DE, HL,
     BCp, DEp, HLp]` (already declared in Z80InstrInfo.td);
     the MachineVerifier should already reject mid-region
     reads of values defined before EXX.

Acceptance criteria for landing:

  - At least one rcbios or cpnos-rom function shows ≥ 3-byte
    code-size win after enabling.  (Otherwise the EXX bracket
    is parity-or-loss.)
  - clang test runner: zero new FAILs.
  - No interaction with `+shadow-regs` ISR save/restore (i.e.
    ISRs that fire mid-bracket still execute correctly).

Estimated effort: 1-2 sessions for prototype + correctness;
+1 for tuning.  Investigation should precede implementation
with a target-loop survey.

## What this session did

  - Confirmed shadow registers are allocatable but unreachable.
  - Confirmed the only viable niche is loop bracketing
    (bank-flip whole functions and bank-flip per-spill are
    blocked by the same EXX-swaps-three constraint).
  - Documented the design path.
  - Did NOT implement.

## Carry-forward

To activate this strand:

  1. Survey hot loops in cpnos-rom + rcbios for the three
     candidacy conditions.  Quantify the pressure-pressure-
     pressure niche.
  2. If 3+ candidates exist with > 3 byte savings each, the
     prototype is justified.
  3. Otherwise: park.  The EXX bracket adds 2 bytes per fired
     loop; if the loop body savings don't dominate, this is
     a no-op or regression.

## Survey results (this session, brief)

`tasks/find_loops.py` (in /tmp/, not committed) walks an
objdump'd disassembly and emits "tight back-edge loops" —
forward instructions ending in `djnz`/`jr`/`jp <local>` where
target < source and (source-target) < 200 bytes.

cpnos-rom (1777 non-padding bytes total): only 5 tight loops,
all small (≤18 B body).  None hot.  Shadow-bank niche is not
present here.

rcbios (5933 B, BIOS): ~25 tight no-CALL loops.  The
non-trivial ones (>15 B body):

| Function          | Body | Insns | Notes                              |
| ----------------- | ---: | ----: | ---------------------------------- |
| `_specc`          |  37B |    21 | display char dispatch loop         |
| `_cursor_left`    |  34B |    15 | cursor backstep loop               |
| `_scroll`         |  34B |    17 | scroll-region copy loop            |
| `_bios_conin`     |  30B |    16 | console input wait loop            |
| `_bios_hw_init`   |  24B |    15 | hw-init poll loop                  |

Spot-checked `_specc` 0xde19-0xde3c:

```
de19: ld   hl,$f078
de1c: ld   ($f3a6),bc        ; <-- BC spill ENTERING loop body
de20: add  hl,bc
de21: ld   c,$80
de23: ld   a,c               ; <-- inner-most byte loop bottom
...
de32: jr   $de23             ; inner back-edge
de34: ld   bc,($f3a6)        ; <-- BC reload at outer back-edge
de38: inc  bc
de39: ld   a,c
de3a: cp   $fa
de3c: jr   nz,$de19          ; outer back-edge
```

The 4-byte `LD ($f3a6),BC` + 4-byte `LD BC,($f3a6)` are pure
register-pressure overhead — BC's value isn't consumed inside
the inner loop; it's parked while DE/HL/A do byte-tweaking
work.  This is the textbook shadow-bank candidate: if we EXX'd
at the preheader and again at outer-loop exit, we'd save the
8 B/iteration BC spill (over many outer iterations) at the
cost of two EXX bytes total.  Net win once outer iterations
> 1, which is the common case (`_specc` displays a glyph row
multiple bytes wide).

Similar patterns expected in `_scroll`, `_cursor_left`.  Need
to verify each before prototyping; it's possible some have
a CALL on a slow-path branch within the apparent loop.

## Implementation gate

Survey suggests 3-4 BIOS candidates with measurable wins.
Prototype is justified.  Estimated work:

  1. New target MIR pass `Z80ShadowBankBracket` (~150 LOC),
     run after RA, walks MachineLoopInfo, identifies single-
     MBB no-CALL loops with sufficient pressure, verifies
     liveness preconditions at preheader / exit, inserts EXX
     brackets.
  2. Gate by `+shadow-regs` feature flag (already exists).
  3. Lit test fixture demonstrating the bracket on a small
     synthetic loop.
  4. rcbios-byte-exact-or-smaller acceptance criterion.

Out of scope for session 40.  Filing as a Phase-5 issue when
the shadow-bank Phase opens (per roadmap).
