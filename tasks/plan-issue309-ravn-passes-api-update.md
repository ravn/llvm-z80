# Plan: Fix ravn-local passes after upstream PR #40 merge (issue #309)

Status: IN PROGRESS — do NOT start from scratch next session; read this first.

## Situation (as of 2026-09-09)

After merging llvm-z80/llvm-z80 PR #40 (@zlfn's z88dk CC refactoring), the
build was falsely green: ninja was using STALE `.o` files from before the merge.
When we forced a real rebuild (rm libLLVMZ80CodeGen.a), real errors surfaced.

## What we have already fixed (NOT YET COMMITTED — dirty working tree)

All changes are in `llvm-z80/` working tree, uncommitted:

| File | Fix |
|------|-----|
| `llvm/include/llvm/IR/CallingConv.h` | Removed duplicate Z80_AllReg=129 enum block (merge artifact) |
| `llvm/lib/Analysis/TargetLibraryInfo.cpp` | Removed Z80_AllReg case (CC no longer exists) |
| `llvm/lib/Target/Z80/CMakeLists.txt` | Added all 16 ravn-local .cpp files (required by LLVM cmake check) |
| `llvm/lib/Target/Z80/SM83CallLowering.cpp` | Folded 3rd CallingConvRegs into 2nd (constructor takes 2, not 3) |
| `llvm/lib/Target/Z80/SM83InstrInfo.td` | GR16NoIR → GR16 (register class renamed by @zlfn) |
| `llvm/lib/Target/Z80/Z80AsmPrinter.cpp` | STI.staticStack() → STI.hasStaticFrame() |
| `llvm/lib/Target/Z80/Z80CallingConv.td` | Removed unused Z80_AllReg_CSR definition |
| `llvm/lib/Target/Z80/Z80InstrInfo.td` | Restored TAILJMP pseudo (ravn tail-call optimization, lost in merge) |
| `llvm/lib/Target/Z80/Z80LoopInstrFormPrep.h` | PassInfoMixin → OptionalPassInfoMixin |
| `llvm/lib/Target/Z80/Z80LoopRotate.h` | PassInfoMixin → OptionalPassInfoMixin |
| `llvm/lib/Target/Z80/Z80PatternFillRecognize.h` | PassInfoMixin → OptionalPassInfoMixin |
| `llvm/lib/Target/Z80/Z80SinkColdLoopIV.h` | PassInfoMixin → OptionalPassInfoMixin |
| `llvm/test/CodeGen/Z80/remove-jump-to-next.mir` | XFAIL removed (should pass once build is green) |

## Remaining compilation errors (as of session end)

After `rm build-macos/lib/libLLVMZ80CodeGen.a && ninja llc`, these errors remain
in ravn-local passes that reference old API names:

| File | Error |
|------|-------|
| `Z80NarrowNoIndex.cpp` | `GR16NoIRRegClass` no longer exists (use GR16RegClass) |
| `Z80PinAluAccumulator.cpp` | `ARegRegClass` no longer exists |
| `Z80PinLoopPointer.cpp` | `HLRegRegClass` → `HLIRegClass` |
| `Z80Subtarget.cpp` | `ShadowRegs` gone; `initLibcallLoweringInfo` out-of-line mismatch |
| `Z80FuseCarryChain.cpp` | `ADC_HL_BC`, `ADC_HL_DE` instruction names changed |

There are likely more errors in other ravn-local passes that haven't been
reached yet.

## Root cause

The 16 ravn-local `.cpp` files are ALL authored by tra@ravnand.dk (introduced
2026-05-02 through 2026-07-13). @zlfn never had them. His PR #40 merge
overwrote TD files, headers, and a few shared .cpp files, changing:
- Register class names (GR16NoIR→GR16, HLReg→HLI, AReg→?)
- Instruction names (ADC_HL_BC, ADC_HL_DE → new scheme)
- Subtarget API (staticStack()→hasStaticFrame(), ShadowRegs removed)
- Calling convention API (PassInfoMixin moved to detail::, constructor args changed)

## Recommended plan for next session

### Option A (surgical — recommended):
1. Move 15 problematic ravn passes to `llvm/lib/Target/Z80/ravn/` with own CMakeLists.txt
   - This breaks the LLVM cmake check problem (only files in main dir need to be listed)
   - Only `Z80RemoveJumpToNext.cpp` stays in main dir (it compiles cleanly)
   - Commit this state cleanly
2. Fix Z80RemoveJumpToNext registration issue (ninja archive problem) — see below
3. Close issue #309
4. Fix ravn passes in ravn/ one by one in later sessions

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
| `Z80::GR16NoIRRegClass` | `Z80::GR16RegClass` | Register class (IX/IY not in GR16 anymore) |
| `Z80::HLRegRegClass` | `Z80::HLIRegClass` | Register class rename |
| `Z80::ARegRegClass` | unknown — check Z80RegisterInfo.td | Register class rename |
| `STI.staticStack()` | `STI.hasStaticFrame()` | Z80Subtarget method |
| `PassInfoMixin<T>` | `OptionalPassInfoMixin<T>` | llvm/IR/PassManager.h |
| `Z80_AllReg` CC | gone | CallingConv.h |
| `TAILJMP` pseudo | now restored in Z80InstrInfo.td | Tail call |
| `ADC_HL_BC/DE` | check Z80InstrInfo.td for new names | Instruction names |

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
