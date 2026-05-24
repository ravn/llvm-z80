# Session 73s — #181 DAGISel vs GISel coexistence audit

**Date:** 2026-05-24
**Issue:** ravn/llvm-z80#181 ("DAGISel vs GISel coexistence — Z80ISelLowering.cpp possibly dead code").
**Outcome:** Hypothesis refuted. There is **no DAGISel path**. `Z80ISelLowering` is live, required GISel infrastructure (not dead code). One real actionable finding: two TableGen-generated ISel match tables are emitted but never compiled in.

## Question the issue asked

The roadmap Area 9 lists `Z80ISelLowering.cpp (DAGISel)` as "Coexists with GISel; status unclear" and proposes three outcomes: (1) DAGISel is dead, delete it; (2) DAGISel is a fallback, document it; (3) DAGISel runs for SM83. None of the three is correct.

## Findings

### 1. There is no SelectionDAG instruction selector at all
- No `Z80ISelDAGToDAG.cpp` exists.
- No `SelectionDAGISel` subclass anywhere.
- Zero SelectionDAG signatures in the entire backend: `grep -rE "setOperationAction|LowerOperation|SDValue|ISD::|createZ80ISelDag"` over all `*.cpp`/`*.h` returns **nothing**.
- GlobalISel is unconditional: `Z80TargetMachine` sets `setGlobalISel(true)` + `setGlobalISelAbort(GlobalISelAbortMode::Enable)` (Z80TargetMachine.cpp:119/121). With abort enabled and no DAGISel selector to fall back to, a GISel selection failure *errors* — there is no silent DAGISel route.
- `Z80PassConfig` overrides only the GISel hooks (`addIRTranslator`, `addLegalizeMachineIR`, `addRegBankSelect`, `addGlobalInstructionSelect`). There is no `addInstSelector` (the DAGISel hook).

### 2. `Z80ISelLowering.{h,cpp}` is NOT dead code — it is the live GISel `TargetLowering`
- `class Z80TargetLowering : public TargetLowering` — `TargetLowering` is the **shared base class** used by *both* DAGISel and GISel. GISel requires it for legalization queries, calling-convention setup, and target hooks.
- The file (235 LOC) defines only generic, path-agnostic queries: `getRegisterType`, `isLegalAddressingMode`, `isTruncateFree` (Type + LLT overloads), `isZExtFree` (Type + LLT overloads). The LLT overloads are GISel-specific. There is **no DAG lowering** (`LowerOperation`/`LowerFormalArguments`/`setOperationAction` never appear).
- It is actively consumed by the GISel pipeline:
  - `Z80CallLowering` / `SM83CallLowering` call `getTLI()` for `ComputeValueVTs` (Z80CallLowering.cpp:313, 1206).
  - `Z80InlineAsmLowering` is constructed from a `Z80TargetLowering *` (Z80InlineAsmLowering.cpp:19).
  - `Z80Subtarget` holds it as `TLInfo` and passes `&TLInfo` into the inline-asm and call-lowering helpers (Z80Subtarget.cpp:42–51).
- **Conclusion: deleting `Z80ISelLowering` (issue outcome 1) would break the build.** Keep it.

### 3. SM83 shares the same GISel selector (issue outcome 3 is false)
- `Z80TargetMachine` registers both `z80` and `sm83` targets onto the same machine class. SM83 differs only in `SM83CallLowering` (calling convention); it still receives the same `&TLInfo` and routes through the same hand-written GISel selector.

### 4. Git history: `Z80ISelLowering.cpp` was born GISel-only
- Earliest commit touching it is `31997a6 [Z80] Add Z80/SM83 backend with GlobalISel pipeline`.
- `git log -S "setOperationAction" -- Z80ISelLowering.cpp` returns **nothing** — the file never contained DAG lowering. The "status unclear" framing was never-investigated, not stripped-down remnant.

### 5. NEW finding — two generated ISel match tables are emitted but never compiled in
CMakeLists.txt generates both:
- line 8:  `tablegen(LLVM Z80GenDAGISel.inc -gen-dag-isel)`
- line 14: `tablegen(LLVM Z80GenGlobalISel.inc -gen-global-isel)`

Both `.inc` files are produced in the build dir, but **neither is `#include`d anywhere** in the backend (`grep -rn Z80GenDAGISel` / `Z80GenGlobalISel` over `*.cpp`/`*.h`/`*.td` → zero source references). For contrast, the combiner / regbank / instrinfo / callingconv / subtarget generated tables *are* all included.

Instruction selection is **100 % hand-written**: `Z80InstructionSelector::select()` (Z80InstructionSelector.cpp:1824) does its own dispatch and never calls the TableGen-generated `selectImpl()`. The 5777-LOC selector is the whole story.

Both tablegen lines are almost certainly vestigial from the LLVM-MOS optimization-infrastructure port (`6d1fd0b [LLVM] Port optimization infrastructure from LLVM-MOS`).

## Recommendation

- **Keep `Z80ISelLowering.{h,cpp}`** — it is required live GISel `TargetLowering`. Update roadmap Area 9 from "DAGISel; coexists, status unclear" to "GISel-only `TargetLowering`; no DAGISel selector exists."
- **Remove the `-gen-dag-isel` tablegen line** (CMakeLists.txt:8): it generates an unused table, costs build time, and is the artifact that makes the backend *look* like it has a DAGISel path. Unambiguously dead — the backend is committed to GISel.
- **`-gen-global-isel` line (CMakeLists.txt:14):** also currently unused (hand-written selector ignores `selectImpl`), but keeping it is defensible — it is the conventional GISel table and a future refactor could wire `selectImpl()` as a pre-pass fallback to shrink the hand-written selector. Flag for discussion, do not remove unilaterally.

## Validation performed

- Empirical: the issue's step 3 ("force `-global-isel` ON and verify nothing changes") is moot — GISel is already the only path, so there is nothing to A/B against. The AES corpus + lit suite + test-runner all pass under GISel today (they have no alternative).
- After removing the `-gen-dag-isel` line: clean reconfigure + `ninja clang llc` + full Z80 lit suite (see commit).

## Closes
Resolves the #181 audit. Files stay; one dead tablegen line removed.
