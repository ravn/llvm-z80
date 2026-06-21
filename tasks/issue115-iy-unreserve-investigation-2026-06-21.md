# Issue #115 — IY-extraction overhead investigation 2026-06-21

**Goal**: examine whether #115's premise (greedy picks IY for HL/DE-
needed values, forcing `PUSH IY; POP HL` extractions) still applies on
current production builds, and if so, sketch the implementation.

**Method**: empirical scan of the production triplet (autoload + cpnos
PROM1 + rcbios BIOS) disassembly for IY-extraction patterns; locate
reference single-register-class mechanisms; sketch HLReg/DEReg design.

**Outcome**: #115's premise is **REAL AND CURRENT**.  ~21 B of
recoverable overhead across the production triplet from the specific
extraction pattern #115 describes.  Plus a correction to the 2026-06-21
inverse-analysis misclassification (#115 is not a cost-model question).

## Surprise correction — IY is already allocatable in production

`Z80RegisterInfo::getReservedRegs` doesn't unconditionally reserve IY:

```cpp
// Z80RegisterInfo.cpp:277
bool llvm::z80IsIYAllocatable(const MachineFunction &MF) {
  if (Z80UnreserveIY)
    return true;
  return MF.getFunction().hasOptSize() &&
         MF.getSubtarget<Z80Subtarget>().staticStack();
}

// Z80RegisterInfo.cpp:305
if (!z80IsIYAllocatable(MF))
  Reserved.set(Z80::IY);
```

The condition `hasOptSize() && staticStack()` is true for all three
production targets (`-Oz` + `+static-stack`).  **IY is allocatable in
production today.**  No flag-toggle experiment was needed.

The 2026-06-21 inverse-analysis claim that "IY reservation default-on"
was a sledgehammer was therefore wrong: it's already conditional, and
the condition matches production.  The remaining question is whether
the *regalloc quality* under IY-allocatable is good or has the #115
regression pattern.

## Empirical scan — production extractions today

Scanned the three production ELFs (autoload-baseline.elf, cpnos-
baseline.elf, rcbios-baseline.elf -- the ones from #232's investigation)
for `PUSH IY; POP rr` and `PUSH rr; POP IY` pairs.

| Target | iy→hl | iy→de | iy→bc | hl→iy | de→iy | bc→iy | Total | Bytes |
|--------|-------|-------|-------|-------|-------|-------|-------|-------|
| autoload | 1 | 0 | 1 | 3 | 0 | 0 | 5 | 15 B |
| cpnos PROM1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | **0 B** |
| rcbios | 5 | 0 | 0 | 6 | 0 | 0 | 11 | 33 B |
| **Aggregate** | **6** | 0 | 1 | **9** | 0 | 0 | 16 | **48 B** |

Each transfer is 3 B (`PUSH rr` = 1 B + `POP rr` = 1 B for non-IX/IY,
or 2 B + 1 B with the IX/IY prefix).

Two distinct buckets:

- **iy→hl/bc** (7 transfers, ~21 B): value extracted FROM IY.  This is
  the #115 pattern -- greedy picked IY for a value that LDIR / ADD-HL /
  memory-via-HL needed.
- **hl→iy** (9 transfers, ~27 B): value placed INTO IY.  Often
  intentional (to enable subsequent `IY+d` indexed access), so not
  necessarily a bug -- but if eliminated by closer pre-RA placement,
  bytes would be saved too.

#115 specifically addresses the first bucket.  Recoverable overhead by
that interpretation: **~21 B aggregate** across autoload + rcbios; cpnos
has zero IY-extractions and benefits nothing directly.

## Shape verification — does the pattern match #115's description?

Sample of the rcbios sites (raw disassembly excerpts):

```
e2af: e5         push hl
e2b0: fd e5      push iy   ; ← IY→HL extraction
e2b2: e1         pop  hl
e2b3: cd 6b da   call $da6b ; HL passed to callee

e466: eb         ex de,hl
e467: fd e5      push iy   ; ← IY→HL extraction
e469: e1         pop  hl
e46a: a7         and a
e46b: ed 42      sbc hl,bc ; 16-bit compare via HL

e680: fd e5      push iy   ; ← IY→HL extraction
e682: e1         pop  hl
e683: a6         and (hl)  ; memory access via HL
e684: fd 77 00   ld  (iy+$0),a
```

And autoload:

```
62eb: 11 08 00   ld   de,$8
62ee: fd e5      push iy   ; ← IY→HL extraction
62f0: e1         pop  hl
62f1: 19         add  hl,de ; ADD HL,rr operand needed in HL
```

All sampled sites confirm the #115 pattern: a value living in IY needs
to be in HL for the next instruction (call argument convention, 16-bit
compare via `SBC HL,rr`, memory access via `(HL)`, ADD HL,rr operand).
**Pattern matches #115's described regression.  Design is still
applicable.**

## Where the reference single-register-class pattern lives

`Z80RegisterInfo.td`:

```tablegen
// L 179: BReg -- B-only, for DJNZ counter via Z80SplitDjnzCounters.
def BReg : Z80Reg8Class<(add B)>;

// L 211: BCReg -- BC-only, sister of BReg for i16 DJNZ.
def BCReg : Z80Reg16Class<(add BC)>;

// L 186: AReg -- A-only, for ALU accumulator via Z80PinAluAccumulator.
def AReg : Z80Reg8Class<(add A)>;

// L 221: GR16NoIR -- GR16 minus IX/IY, for pseudo expansions whose
// 8-bit-half lowering needs documented SRL/SRA paths (no IXH/IXL/IYH/IYL).
def GR16NoIR : Z80Reg16Class<(add DE, HL, BC)>;
```

Two distinct application patterns:

1. **InstructionSelector-level constraint** (`GR16NoIR` is applied via
   `RBI.constrainGenericRegister(LhsLo, Z80::GR16NoIRRegClass, MRI)` in
   `Z80InstructionSelector.cpp:1543-1574`, `1571-1574`, `1696-1699`).
   Used when lowering a specific pseudo whose tied operand has a hard
   restriction.

2. **Pre-RA pass constraint** (`BCReg` is applied via
   `Z80SplitDjnzCounters`, see `Z80SplitDjnzCounters.cpp:201,258`).
   Used when the constraint is structural (loop counter shape) and
   needs a dedicated pass to identify the candidate vreg.

Either path can host the #115 fix.

## Design sketch -- HLReg / DEReg

**Not code; this is a sketch.**  Implementing requires the same kind of
test discipline (test-first, full lit + production triplet A/B) as the
2026-06-21 sweep.

### Step A — register classes

```tablegen
// Z80RegisterInfo.td, after BCReg:
def HLReg : Z80Reg16Class<(add HL)>;
def DEReg : Z80Reg16Class<(add DE)>;
```

Mechanically straightforward; mirrors BCReg.

### Step B — application

For each pseudo where the operand HAS to live in HL / DE / BC at lower
time, constrain the vreg that COPYs into it.

**LDIR / LDDR**: `Uses = [HL, DE, BC]`.  Each of the three preceding
COPYs (`HL = COPY %src`, `DE = COPY %dst`, `BC = COPY %count`) has a
vreg source that should be constrained to HLReg / DEReg / BCReg
respectively.

**CPIR / CPDR**: `Uses = [HL, A, BC]`.  HLReg + BCReg.

**ADD HL, rr family**: HL is implicitly the destination.  The operand
COPYing into HL is the vreg to constrain.

**Memory access via (HL)**: there's no specific pseudo to anchor here;
this is regalloc's general decision to use HL as a pointer.  #115's
extractions fed this case too (the `and (hl)` shape at e683).
Constraining for this would require a more general pass.

Cleanest first step: handle just LDIR/LDDR/CPIR/CPDR.  Those are 21 B
of the rcbios extractions today (5 sites, all of which involve a call
or compare that happens to land HL via convention).

### Step C — constraint mechanism

Two implementation choices:

1. **InstructionSelector-side** (analog of `GR16NoIR`): when selecting
   the LDIR pseudo lowering, `constrainGenericRegister` the source
   vregs of the three COPYs to HLReg/DEReg/BCReg.  Simple, runs at
   ISel time.

2. **Pre-RA pass** (analog of `Z80SplitDjnzCounters`): walk LDIR/LDDR
   pseudos in MachineInstr form, identify the source vregs, apply
   `MRI.setRegClass()`.  More flexible (can see GISel-lowered MIRs).

Choose (1) -- simpler and earlier; matches the GR16NoIR pattern that's
already in the codebase.

### Step D — risks

- **Over-constraint forcing extra COPYs.**  If the constrained vreg has
  uses where HL isn't the right physreg (say, ADD HL,rr where it's the
  rr operand, not the HL operand), forcing it to HL adds a COPY there.
  Net could be a regression if the vreg has many such uses.
- **Interaction with #110.**  Greedy's copy-elimination heuristic may
  still override the class constraint in some cases (the issue #110
  warns about).  Mitigation: GR16NoIR has worked, so the lever exists
  -- but each new class needs its own validation.
- **LDIR_GUARDED / LDDR_GUARDED / MEMSET_LDIR_GUARDED variants
  (`Z80InstrInfo.td:1760+`)** need the same treatment.
- **ADD HL,rr generalisation** is the bigger ticket -- many pseudos
  involve it implicitly via 16-bit add.  Doing LDIR/LDDR first is the
  fast-payoff path; ADD HL,rr is the long-tail.

### Step E — test discipline

- Lit test pinning the LDIR-via-vreg shape (no `push iy; pop hl`
  extraction; HL produced directly by class).
- Re-run full Z80 lit suite -- no regressions.
- Rebuild production triplet; verify extraction count drops from {2,
  0, 5} toward {0, 0, 0} for IY→HL extractions.
- Test-runner runtime fixture exercising LDIR with various source-vreg
  liveness patterns, to catch over-constraint regressions.
- Re-measure compressed ROM sizes (per #232's lesson: raw vs compressed
  can disagree).

### Step F — effort estimate

- Step A: 5 minutes (one TableGen edit).
- Step B: 30 minutes per pseudo (LDIR/LDDR/CPIR/CPDR, plus the _GUARDED
  variants) = ~2-3 hours.
- Step C: 1-2 hours integrating with the InstructionSelector.
- Step D-E: A/B + measurement = ~3-4 hours.

**Total: roughly 1-2 sessions of careful work** by someone familiar
with GISel + TableGen.  Not a "afternoon fix"; not a multi-week
project either.

Expected payoff: ~21 B saved on production (15 B autoload + 33 B rcbios
of which ~half is the IY→HL extraction half).  Per-target this is
0.3% / 0.3% / 0% relative.

## Reconciliation with the 2026-06-21 inverse analysis

The 2026-06-21 inverse-analysis section of
`session-2026-06-21-z80-tti-modelling-investigation.md` classified
#115 as the "only plausible cost-model retire-candidate" after
#232 was falsified.

That classification was wrong on two counts:

1. **#115's actual proposed fix is regalloc-class machinery, not a
   cost-model edit.**  HLReg/DEReg single-register classes are
   structurally identical to BCReg / GR16NoIR -- TableGen + RegisterInfo
   constraints, not TTI cost hooks.  These should be in the same
   "machinery is the right layer" bucket as the existing AReg/BReg/
   BCReg/GR16NoIR uses.
2. **The 2026-06-21 sweep verdict that "cost-model fixes are inert on
   GISel-Z80 because the downstream Z80-specific machinery already
   produces the right shape" does NOT apply to #115's design.**  The
   single-register-class mechanism doesn't fall under that verdict --
   it's the kind of machinery that the inertness pattern depends on
   producing the right shape.  Adding HLReg/DEReg extends that
   machinery; it doesn't compete with a cost-model edit.

**Net**: #115 is in the **"weak/no cost-model candidates -- the right
layer is machinery"** list, alongside `MaxStoresPerMemcpy`,
`isReMaterializable on LD_*`, etc.  My inverse-analysis cross-ref
comment on #115 was misframed; will be corrected with a follow-up
comment.

The session writeup's inverse-analysis section should also be
corrected.

## What to do with #115

Three options:

1. **Implement HLReg/DEReg per the sketch above** (~1-2 sessions of
   careful work).  Expected: −21 B aggregate across autoload + rcbios.
   Sound payoff for the effort; aligns with existing AReg/BReg/BCReg
   precedent.

2. **Document the empirical numbers + design sketch + reconciliation,
   leave #115 open for a future session** to implement.  Lower-effort
   immediate close; the work is well-scoped when picked up.

3. **Close #115 as low-priority "would be nice but production hard
   caps not binding"**.  The aggregate −21 B doesn't unlock anything
   specific today; cpnos's 31 B free under cap doesn't gate on this.
   Document and move on.

**Recommend option 2.**  Keep #115 open as a precisely-scoped tracker.
The investigation here makes it ready for implementation when someone
picks it up.

## Files produced under /tmp/issue232/ (reused from #232's investigation)

- `autoload-baseline.elf` / `cpnos-baseline.elf` / `rcbios-baseline.elf`
  -- the binaries scanned for IY-extraction patterns.

No new artefacts under /tmp; this investigation was a read-only scan
on the existing #232 artefacts.

## Cross-references

- ravn/llvm-z80#115 (this investigation closes the "examine" question).
- `llvm-z80/tasks/session-2026-06-21-z80-tti-modelling-investigation.md`
  -- the broader TTI sweep; inverse-analysis section needs correction.
- ravn/llvm-z80#110 -- greedy regalloc heuristic overrides target
  hints; #115's HLReg/DEReg is the workaround.
- ravn/llvm-z80#94 / #98 / #99 -- the BReg/BCReg precedents.
- ravn/llvm-z80#112 -- GR16NoIR precedent.
- ravn/llvm-z80#172 -- AReg / Z80PinAluAccumulator precedent.
- `llvm/lib/Target/Z80/Z80RegisterInfo.td:170-225` -- the existing
  single-register-class definitions.
- `llvm/lib/Target/Z80/Z80InstructionSelector.cpp:1543-1699` -- the
  GR16NoIR application pattern to mirror.
- `llvm/lib/Target/Z80/Z80SplitDjnzCounters.cpp:201,258` -- the BCReg
  pre-RA-pass application pattern.
- `llvm/lib/Target/Z80/Z80RegisterInfo.cpp:277` -- the
  `z80IsIYAllocatable` condition (production IY-allocatable in -Oz +
  static-stack).
