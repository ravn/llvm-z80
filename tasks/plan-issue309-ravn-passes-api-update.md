# Plan: Fix ravn-local passes after upstream PR #40 merge (issue #309)

Status: IN PROGRESS — do NOT start from scratch next session; read this first.

## Situation (as of 2026-09-09)

After merging llvm-z80/llvm-z80 PR #40 (the fork owner's z88dk CC refactoring), the
build was falsely green: ninja was using STALE `.o` files from before the merge.
When we forced a real rebuild (rm libLLVMZ80CodeGen.a), real errors surfaced.

**REVISED 2026-09-09 (build-verified).** The first pass of this plan (below) was
written from a partial rebuild that stopped at the first errors. A full
`rm build-macos/lib/libLLVMZ80CodeGen.a && ninja -C build-macos -k 0 llc`
(keep-going, collect ALL errors) now gives the authoritative picture:
**45 errors across 9 files, not 5.** See "Authoritative error inventory" below —
it supersedes the older "Remaining compilation errors" table. Full log:
`/tmp/z80_build.log`.

## What we have already fixed (COMMITTED — 321242de50a2, working tree clean)

These changes ARE committed (commit `321242de50a2`, "[Z80] Partial fixes after
PR #40 merge — ravn passes API update WIP"). The Z80-target working tree is
clean; do NOT expect a dirty tree next session. All the `PassInfoMixin →
OptionalPassInfoMixin` conversions are done and verified (incl. `Z80IndexIV.h`,
a 17th ravn-authored file not on the provenance list below).

| File | Fix |
|------|-----|
| `llvm/include/llvm/IR/CallingConv.h` | Removed duplicate Z80_AllReg=129 enum block (merge artifact) |
| `llvm/lib/Analysis/TargetLibraryInfo.cpp` | Removed Z80_AllReg case (CC no longer exists) |
| `llvm/lib/Target/Z80/CMakeLists.txt` | Added all 16 ravn-local .cpp files (required by LLVM cmake check) |
| `llvm/lib/Target/Z80/SM83CallLowering.cpp` | Folded 3rd CallingConvRegs into 2nd (constructor takes 2, not 3) |
| `llvm/lib/Target/Z80/SM83InstrInfo.td` | GR16NoIR → GR16 (register class renamed upstream in PR #40) |
| `llvm/lib/Target/Z80/Z80AsmPrinter.cpp` | STI.staticStack() → STI.hasStaticFrame() |
| `llvm/lib/Target/Z80/Z80CallingConv.td` | Removed unused Z80_AllReg_CSR definition |
| `llvm/lib/Target/Z80/Z80InstrInfo.td` | Restored TAILJMP pseudo (ravn tail-call optimization, lost in merge) |
| `llvm/lib/Target/Z80/Z80LoopInstrFormPrep.h` | PassInfoMixin → OptionalPassInfoMixin |
| `llvm/lib/Target/Z80/Z80LoopRotate.h` | PassInfoMixin → OptionalPassInfoMixin |
| `llvm/lib/Target/Z80/Z80PatternFillRecognize.h` | PassInfoMixin → OptionalPassInfoMixin |
| `llvm/lib/Target/Z80/Z80SinkColdLoopIV.h` | PassInfoMixin → OptionalPassInfoMixin |
| `llvm/test/CodeGen/Z80/remove-jump-to-next.mir` | XFAIL removed (should pass once build is green) |

## Authoritative error inventory (build-verified 2026-09-09) — 45 errors / 9 files

Full keep-going build: **9 files fail, in 4 distinct breakage categories.** The
older 5-file table was incomplete (missed 4 files) and mis-mapped two fixes;
this replaces it. Files marked ★ were NOT in the original table.

### Category A — deleted single-register RegisterClasses (root cause, 5 files)
PR #40 **deleted** 5 single-register classes from `Z80RegisterInfo.td` that the
ravn passes constrain against. They are NOT renames — `A`/`B`/`HL` are *registers*,
not RegisterClasses, so `ARegClass`/`HLRegClass` are never generated. The clean
fix is to **re-add the 4 class defs** (they were 1 line each, purely additive,
known-good from before the merge — see snippet under "Recommended plan"):

| Pre-merge def (deleted) | Members | Referenced in |
|---|---|---|
| `def AReg`     | A       | `Z80PinAluAccumulator.cpp` (`ARegRegClass`) |
| `def BReg`     | B       | ★`Z80SplitDjnzCounters.cpp` (`BRegRegClass`) |
| `def BCReg`    | BC      | ★`Z80SplitDjnzCounters.cpp` (`BCRegRegClass`) |
| `def HLReg`    | HL      | `Z80PinLoopPointer.cpp` (`HLRegRegClass`) |
| `def GR16NoIR` | DE,HL,BC | `Z80NarrowNoIndex.cpp`, ★`Z80KeepLoopPointerInPair.cpp` |

**IMPORTANT — do NOT follow the old table's `HLRegRegClass → HLIRegClass` advice.**
`HLI` = {HL,IX,IY} broadens the constraint; `Z80PinLoopPointer` must pin to HL
*only*. Same for `AReg` (pins to A only). Re-add the single-reg classes instead.
`GR16NoIR (DE,HL,BC)` == new `GR16 (BC,DE,HL)` (identical members), so that one
*could* be rewritten to `GR16RegClass`, but re-adding `GR16NoIR` keeps all call
sites untouched and is the smaller diff.

### Category B — instruction defs restructured to parameterized register-forms
PR #40 replaced per-register/mnemonic instr defs with register-operand forms.
These need real code changes (add an operand / use the new form), not renames:

| Old (gone) | New form | Referenced in |
|---|---|---|
| `ADC_HL_BC/DE`, `SBC_HL_BC/DE`, `ADD_HL_BC/DE` | `ADC_HL_rr`/`SBC_HL_rr`/`ADD_HL_rr` (reg operand) | `Z80FuseCarryChain.cpp` |
| `AND_A` | (verify new clear-CF form) | `Z80FuseCarryChain.cpp` |
| `LD_HLind_A..L` (7), `INC16`, `DEC16` | reg-form store / `INC_rr`/`DEC_rr` | ★`Z80KeepLoopPointerInPair.cpp` |
| `OR_A`, `DEC_A`, `ADD_A_A` | (verify; `DEC_A`→`DEC_r`) | ★`Z80ReorderTestDec.cpp` |
| `DEC16`, `DEC_A` | as above | ★`Z80SplitDjnzCounters.cpp` |

### Category C — opcode-helper API changed (★`Z80HighByteFirstBranch.cpp`, 11 errors, worst-hit)
- `getLD8RegOpcode`, `getSUBOpcode`, `getSBCOpcode`, `getCPOpcode` — **removed**.
- `getAluRegOpcode(unsigned Op)` now takes **one** arg; the pass calls it with two.
This file needs the most rework — retarget every call against the new
`Z80InstrInfo.h` opcode-helper surface.

### Category D — Subtarget (`Z80Subtarget.cpp` / .h / Features.td)
- **ShadowRegs is only HALF-removed** (old table said "gone"). `Z80Features.td:69`
  still defines `FeatureShadowRegs` (field `"ShadowRegs"`) and generated
  `Z80GenSubtargetInfo.inc:240` does `ShadowRegs = true;`, but the `bool ShadowRegs`
  field was dropped from `Z80Subtarget.h`. Fix = re-add the field to the header
  (keeps `+shadow-regs` working) OR delete the feature from `Z80Features.td`
  (only if shadow-regs is truly being retired — check callers first).
- `initLibcallLoweringInfo` has **no declaration** in `Z80Subtarget.h` (the merge
  dropped it); the `.cpp` out-of-line def matches nothing. Re-add the decl (the
  body already uses `hasStaticFrame()`, which exists).

## Root cause

The 16 (now 17, incl. Z80IndexIV) ravn-local files are ALL authored by
tra@ravnand.dk (2026-05-02 → 2026-07-13). The fork owner never had them. PR #40 overwrote
TD files, headers, and shared .cpp files. Breakage is **broader than "renames"** —
four distinct classes (A–D above): deleted register classes, restructured
instruction defs, a changed opcode-helper API, and a half-removed subtarget feature.

## Recommended plan for next session

**Superseded recommendation:** the old "Option A — move 15 passes to `ravn/`
subdir" does NOT address root cause and is now clearly insufficient: 4 of the 9
failing files break on instruction-def/opcode-helper API (categories B/C) that a
directory move does nothing for. Fix the API breakage directly, in this order
(cheapest/safest first, so the build goes green in stages):

### Step 1 — re-add the 4 deleted single-register classes (fixes Category A, 5 sites)
Add to `Z80RegisterInfo.td` (verbatim from pre-merge `cbaa9835043a~1`):
```
def BReg      : Z80Reg8Class<(add B)>;
def AReg      : Z80Reg8Class<(add A)>;
def BCReg     : Z80Reg16Class<(add BC)>;
def HLReg     : Z80Reg16Class<(add HL)>;
```
This makes `ARegRegClass`/`BRegRegClass`/`BCRegRegClass`/`HLRegRegClass` resolve
again with the correct (single-register) semantics. For `GR16NoIRRegClass`
(NarrowNoIndex + KeepLoopPointerInPair) either re-add `def GR16NoIR :
Z80Reg16Class<(add DE, HL, BC)>;` or sed the 5 call sites to `GR16RegClass`
(identical members). Rebuild and re-check — regclass additions can shift tablegen
allocation; confirm lit still green.

### Step 2 — Subtarget (Category D)
- Re-add `bool ShadowRegs = false;` to `Z80Subtarget.h` (keeps `+shadow-regs`),
  OR remove `FeatureShadowRegs` from `Z80Features.td` if retiring it (check callers).
- Re-add the `initLibcallLoweringInfo` declaration to `Z80Subtarget.h`.

### Step 3 — instruction-def references (Category B)
Rewrite `Z80FuseCarryChain`, `Z80KeepLoopPointerInPair`, `Z80ReorderTestDec`,
`Z80SplitDjnzCounters` to the new register-operand instruction forms
(`ADC_HL_rr`/`SBC_HL_rr`/`ADD_HL_rr`, `INC_rr`/`DEC_rr`, `DEC_r`, reg-form
`LD (HL),r`). These are behavioural — verify emitted asm per site.

### Step 4 — opcode-helper API (Category C, worst-hit)
`Z80HighByteFirstBranch` — retarget all calls to the new `Z80InstrInfo.h`
surface (`getAluRegOpcode(Op)` single-arg; find replacements for the removed
`getLD8RegOpcode`/`getSUBOpcode`/`getSBCOpcode`/`getCPOpcode`).

### Then
5. Rebuild green, run lit + `cargo run -- clang`, re-verify MAME boot on production.
6. Close issue #309 (its direct goal, `Z80RemoveJumpToNext` registration, already compiles).

### Fix for ninja archive not updating:
The archive `libLLVMZ80CodeGen.a` gets stuck at old timestamp. The only reliable
fix is `rm build-macos/lib/libLLVMZ80CodeGen.a` before `ninja llc`. Consider
adding this to the build workflow. This is a macOS ninja/ar interaction bug:
when new .o files are added to a cmake target, the archive's dependency list
in build.ninja is updated but the archive itself isn't seen as stale.

### Before touching anything:
1. `rm build-macos/lib/libLLVMZ80CodeGen.a` first
2. `ninja -C build-macos clang llc lld` — see ACTUAL errors
3. Fix errors one file at a time; do NOT `touch *.o`

## Key API changes to know (for fixing ravn passes)

| Old API | New API | Where |
|---------|---------|-------|
| `Z80::GR16NoIRRegClass` | re-add `def GR16NoIR` OR use `Z80::GR16RegClass` (same members BC,DE,HL) | Register class |
| `Z80::HLRegRegClass` | **re-add `def HLReg` (add HL)** — NOT `HLIRegClass` (HLI={HL,IX,IY} broadens; pass must pin HL) | Register class |
| `Z80::ARegRegClass` | **re-add `def AReg` (add A)** — no `ARegClass` is generated (A is a register, not a class) | Register class |
| `Z80::BRegRegClass` / `BCRegRegClass` | **re-add `def BReg` / `def BCReg`** | Register class |
| `STI.staticStack()` | `STI.hasStaticFrame()` | Z80Subtarget method (DONE) |
| `PassInfoMixin<T>` | `OptionalPassInfoMixin<T>` | llvm/IR/PassManager.h (DONE, all 6 files) |
| `Z80_AllReg` CC | gone | CallingConv.h (DONE) |
| `TAILJMP` pseudo | restored in Z80InstrInfo.td | Tail call (DONE) |
| `ADC_HL_BC/DE`, `SBC_HL_BC/DE`, `ADD_HL_BC/DE` | `ADC_HL_rr`/`SBC_HL_rr`/`ADD_HL_rr` (register operand) | Instruction defs |
| `INC16`/`DEC16` | `INC_rr`/`DEC_rr` (verify) | Instruction defs |
| `DEC_A` | `DEC_r` (register operand) | Instruction defs |
| `OR_A`/`ADD_A_A`/`AND_A`/`LD_HLind_r` | verify new reg-form in Z80InstrInfo.td | Instruction defs |
| `getLD8RegOpcode`/`getSUBOpcode`/`getSBCOpcode`/`getCPOpcode` | removed — retarget to new `Z80InstrInfo.h` helpers | Opcode helpers |
| `getAluRegOpcode(Op, x)` | `getAluRegOpcode(Op)` — single arg now | Opcode helper signature |

## Provenance of the 16 ravn-local files
All introduced by tra@ravnand.dk, none in llvm-z80/llvm-z80 upstream:
- Z80AutoStaticStack (2026-05-24), Z80FuseCarryChain (2026-06-24)
- Z80HighByteFirstBranch (2026-07-12), Z80KeepLoopPointerInPair (2026-07-12)
- Z80LoopInstrFormPrep (2026-07-05), Z80LoopRotate (2026-05-02)
- Z80NarrowNoIndex (2026-05-26), Z80PatternFillRecognize (2026-05-02)
- Z80PinAluAccumulator (2026-05-21), Z80PinLoopPointer (2026-07-09)
- Z80PruneCallFrameDefs (2026-05-28), Z80RemoveJumpToNext (2026-07-13)
- Z80ReorderTestDec (2026-05-21), Z80SinkColdLoopIV (2026-07-09)
- Z80SplitDjnzCounters (2026-05-03), Z80TargetTransformInfo (2026-05-30)

## Issues tracking this work
- ravn/llvm-z80 #309: z80-remove-jump-to-next not registered (direct goal)
- ravn/llvm-z80 #301: 76 lit tests XFAIL after PR #40 merge (context)
- ravn/llvm-z80 #302-#311: categorized XFAIL groups
