# Plan: submit the llvm-z80 work upstream

**Date:** 2026-09-06  
**Target:** `llvm-z80/llvm-z80`  
**Scope:** compiler work only  
**Excluded:** separate `z88dk/z88dk` pull request, RC700 firmware, MAME
integration, and any test that requires z88dk.

## Objective

Prepare a small series of reviewable pull requests that moves the useful
compiler work from `ravn/llvm-z80` into `llvm-z80/llvm-z80`. Every submitted
change must be explainable as llvm-z80 compiler functionality, have a
self-contained regression test, and build against the upstream repository
without external z88dk sources, libraries, assemblers, or runtime tools.

The immediate outcome is not to upstream the entire fork. The immediate
outcome is a focused first PR that gives the maintainer the z88dk calling
convention support needed by the compiler, followed by separate PRs for
independent correctness and optimization work.

## Current baseline

The fork has already merged the large upstream update from
`llvm-z80/llvm-z80:main`. The current fork `main` therefore contains both
upstream changes and fork-only work. It must not be submitted as one diff.

The working tree also contains an untracked `scratch/` directory with merge
artifacts and logs. It is investigation material only and must not be included
in any PR.

## Build-tree isolation

Each long-lived compiler version or PR branch must use its own build directory.
Do not reuse `build-macos` after switching branches: CMake/Ninja will otherwise
recompile a large portion of LLVM and can leave binaries that do not match the
checked-out source.

For this PR, use:

```text
llvm-z80/build-pr296/
```

The existing `build`, `build-macos`, `build-macos-asserts`, and `build-macos-b`
directories are separate caches for other configurations or historical
compiler states. Keep them intact. Configure a new tree with the repository's
Z80 cache and build only the required tools initially:

```bash
cmake -C clang/cmake/caches/Z80.cmake -G Ninja -S llvm -B build-pr296
ninja -C build-pr296 clang llc lld
```

The build directory is generated state and must not be included in the PR.

## Non-negotiable rules

1. Base each PR on the current `upstream/main`, not on the full fork history.
2. Do not include merge commits in a PR branch.
3. Keep each PR thematically coherent and independently reviewable.
4. Add the test before the implementation and observe the failure on the
   unmodified base where the change is a behavior fix.
5. LLVM lit/FileCheck and Clang tests must be self-contained.
6. Do not make z88dk a build- or test-time dependency of llvm-z80.
7. Do not include RC700 source, MAME scripts, production verifier scripts,
   local paths, generated binaries, or workspace scratch files.
8. Preserve existing upstream behavior unless the PR explicitly changes it
   and includes a test for that change.
9. Run `clang` and `llc` builds together after backend changes.
10. Do not open or push a PR until the branch has passed its stated gates and
    the maintainer-facing explanation is ready.

## PR series

### PR 0: preparation branch and inventory

Create a clean local branch from the current upstream tip:

```text
upstream/main
  |
  +-- pr/z80-calling-conventions
```

Before copying code:

- record the exact upstream base commit;
- list the fork commits that implement or test `z80_fastcall`,
  `z80_callee`, and the `smallc` + `callee` combination;
- separate compiler files from z88dk-side files;
- identify duplicated or superseded commits after the upstream merge;
- confirm that no selected commit introduces a z88dk include, library, tool, or
  path dependency.

The inventory is complete only when every selected file has a reason to be in
the PR and every omitted file has a reason to stay out.

### PR 1: z88dk calling-convention support

This is the first submission and the highest priority.

#### ABI conventions covered

The PR must describe and test these three distinct Z80 conventions:

- **`z80_allreg` / z88dk all-register:** all arguments and return values use
  registers; no arguments are passed on the stack and the callee does not
  perform stack cleanup. This is the multi-argument register ABI used by Z80
  runtime helpers.
- **`z80_fastcall`:** a single-argument register ABI. The supported argument
  and return registers depend on width: `i8` uses `L`, `i16` uses `HL`, and
  `i32` uses `DE:HL`. It is not a general multi-argument convention.
- **`z80_callee`:** arguments use the z88dk stack layout, but the callee
  removes the arguments before returning. The caller must not perform a second
  cleanup.

`z80_allreg` and `z80_fastcall` are both register conventions, but they are
not interchangeable: `z80_allreg` supports multiple register arguments,
whereas `z80_fastcall` is the specialized single-argument convention.
`z80_callee` is a stack convention with callee cleanup.

#### Current compiler use: runtime-unknown `memmove`

The current concrete use of `z80_allreg` is the runtime-unknown-direction
`llvm.memmove` fallback in `Z80LegalizerInfo.cpp`. When the compiler cannot
prove whether the copy must run forwards or backwards, it emits an internal
`__memmove_rt` call with the three 16-bit arguments in registers:

```text
dst  -> HL
src  -> DE
size -> BC
```

This is a good fit for the all-register convention because the helper is
compiler-generated, has multiple arguments, returns `void`, and does not need
the public z88dk `memmove` ABI. It avoids pushing the arguments, creating an
IX-based stack frame, and performing caller/callee cleanup; the helper can
immediately adapt the registers and tail-call the overlap-safe copy core.
The public `memmove` ABI remains unchanged, and the LLVM tests can verify the
lowering without linking z88dk. This is the reason `z80_allreg` belongs in the
compiler PR even though its current production-facing use is an internal
runtime helper rather than a general z88dk library entry point.

#### Return-value placement

For scalar integer and pointer-like values up to 32 bits, the return
registers are part of the ABI and must be documented and tested separately
from argument placement. For a 32-bit value, `high:low` means that the high
16-bit word is in the first register and the low 16-bit word is in the
second register.

| Convention | LLVM/Clang name | In `upstream/main` | 8-bit return | 16-bit return | 32-bit return |
|---|---|---:|---|---|---|
| Default Z80 C ABI / `sdcccall(1)` | `CallingConv::C` | **yes** | `A` | `DE` | `HL:DE` |
| `sdcccall(0)` | `Z80_SDCCCall0` / `CC_Z80SDCCCall0` | **yes** | `L` | `HL` | `DE:HL` |
| `z80_allreg` | `Z80_AllReg` / `CC_Z80AllReg` | **new** | `A` | `DE` | `HL:DE` |
| `z80_fastcall` | `Z80_Z88dkFastCall` / `CC_Z80FastCall` | **new** | `L` | `HL` | `DE:HL` |
| `z80_callee` | `Z80_Z88dkCallee` / `CC_Z80Callee` | **new** | `L` | `HL` | `DE:HL` |
| `z80_smallc` | `Z80_SmallC` / `CC_Z80SmallC` | **new** | `L` | `HL` | `DE:HL` |
| `z80_smallc + z80_callee` | `Z80_SmallCCallee` / `CC_Z80SmallCCallee` | **new** | `L` | `HL` | `DE:HL` |

Here “new” is relative to the exact `llvm-z80/llvm-z80:upstream/main`
baseline used for the PR branch. It does not mean that the ABI is new to
z88dk; it means that the convention representation, lowering, frontend
plumbing, and tests still have to be contributed to llvm-z80 upstream.

`z80_allreg` is the important exception to the simple “all register
conventions return in the same registers” description: its *arguments* use
the all-register allocation pools, but its return values use the default Z80
return mapping. `z80_fastcall` returns in the same fixed registers used for
its one argument. The stack-based z88dk conventions share the classic
z88dk return mapping regardless of who cleans the stack or which argument
order is selected.

Values larger than 32 bits are not returned in the scalar register pairs.
They use the existing structure-return (`sret`) lowering: a hidden pointer is
passed on the stack and the callee stores the result through that pointer.
Aggregate values of up to four bytes are packed into the convention’s
8-/16-/32-bit return registers and must preserve the same word order.

#### Intended contents

- LLVM calling-convention identifiers and ABI documentation.
- Z80 call lowering for:
  - `z80_allreg` / the z88dk all-register convention;
  - `z80_fastcall`;
  - `z80_callee`;
  - the existing `sdcccall(0)`/`__smallc` behavior;
  - the valid composed `smallc + callee` case.
- Correct argument order, argument placement, return registers, and stack
  cleanup.
- Clang attribute spelling, semantic validation, diagnostics, type propagation,
  CodeGen mapping, printing/mangling, and target validation.
- Necessary register masks and call-site information.
- Backend lit/FileCheck tests.
- Clang CodeGen and Sema tests.
- Short ABI comments in the relevant implementation and tests.

#### Required test matrix

The tests must run using only the LLVM build and lit infrastructure.

| Area | Required coverage |
|---|---|
| Fastcall arguments | i8, i16, i32, pointer, declaration and definition |
| Fastcall returns | i8, i16, i32 and explicit word order |
| All-register convention | multi-argument register placement, default Z80 return registers, and no stack arguments |
| Callee cleanup | one and multiple stack arguments, mixed widths |
| Cleanup boundary | zero arguments and larger return values |
| Composition | left-to-right push order plus exactly-once callee cleanup |
| Indirect calls | function pointers retain the ABI in the function type |
| Positive controls | default CC and `sdcccall(0)` remain unchanged |
| Diagnostics | genuinely conflicting attributes remain errors |
| Safety | unsupported or ambiguous argument shapes are diagnosed, or have an explicitly tested and documented fallback |

Every FileCheck test must pin the ABI property that would be wrong if the
implementation regressed: argument register choice, return register choice,
push order, cleanup location, or return-value word order. For each convention,
include at least one callee-side return test and one caller-side extraction
test where the register mapping differs from the default. Avoid checks that
merely prove that compilation completed.

#### Commit structure

Prefer a small sequence such as:

1. Add failing backend ABI tests.
2. Add the LLVM calling-convention representation and Z80 lowering.
3. Add failing Clang attribute/diagnostic tests.
4. Add Clang attribute and CodeGen plumbing.
5. Add composed-convention coverage and final documentation.

Squash only when the resulting commits remain easy to review. Do not preserve
experimental merge commits or intermediate generated output.

#### PR 1 acceptance gates

- `ninja -C build clang llc llvm-lit` succeeds.
- `build/bin/llvm-lit llvm/test/CodeGen/Z80/` succeeds.
- Relevant `clang/test/CodeGen` and `clang/test/Sema` selections succeed.
- The test suite passes without z88dk installed or on `PATH`.
- No test references z88dk, zcc, RC700, MAME, or a developer-local path.
- The diff contains no conflict markers, generated binaries, or scratch files.
- A clean-tree rebuild reproduces the same result.

### PR 2: independent correctness fixes

After PR 1 is accepted or the maintainer requests parallel review, submit
correctness fixes that are independent of the calling-convention design.

Candidate categories:

- pseudo sizing and branch-relaxation correctness;
- sret setup and return handling;
- varargs and legalizer correctness;
- register-bank, liveness, and spill operand correctness;
- atomic, vector, truncating-store, and intrinsic legality;
- diagnostics for unsupported operations.

Each item must be its own logical commit or small PR when it has a distinct
root cause. Each needs a minimal reproducer, a failing-first test, the fix,
and a focused validation command. Do not bundle unrelated code-size changes
into a correctness PR.

### PR 3: backend optimizations

Submit optimizations only after the correctness PRs have a stable base.
Organize them by mechanism rather than by the historical order in the fork:

- instruction selection and block-move idioms;
- DJNZ and loop-counter handling;
- late peepholes with complete liveness and CFG guards;
- register-allocation hints and cost-model changes;
- branch cleanup and tail-call opportunities;
- static-stack support, only where it is general llvm-z80 functionality.

Each optimization needs:

- a before/after instruction-level test;
- a positive control that must not change;
- boundary and liveness tests for any instruction-erasing or register-moving
  rule;
- a stated code-size or correctness rationale;
- no reliance on an RC700 binary-size result as the only oracle.

## Extraction procedure

For each PR:

1. Fetch and record the current `upstream/main`.
2. Create a new branch from that exact commit.
3. Cherry-pick only the selected logical commits, or recreate the change
   manually when the historical commit includes unrelated fork material.
4. Resolve API drift against the current upstream code, not against the old
   fork merge result.
5. Add or migrate self-contained LLVM/Clang tests.
6. Build the affected tools.
7. Run the targeted tests first, then the complete relevant lit suites.
8. Inspect the final diff and dependency paths.
9. Write the PR description with:
   - the user-visible/compiler behavior;
   - the ABI or correctness invariant;
   - why the change belongs in llvm-z80;
   - test commands and their independence from z88dk;
   - explicit non-goals.
10. Only then request maintainer review.

## Maintainer communication

The issue discussion should state that the first PR is compiler-only:
calling-convention representation, lowering, Clang plumbing, and
self-contained tests. The separate z88dk driver/library integration is deferred
and is not a dependency of the llvm-z80 tests.

The first PR should not claim that all z88dk library functions are supported.
It should claim only that the compiler can represent and lower the conventions
covered by the tests.

The convention currently easy to miss is the existing `z80_allreg` calling
convention (`CallingConv::Z80_AllReg`, LLVM CC 129). It is used by the
runtime-unknown `memmove` helper lowering described above and is also exposed
for explicit multi-argument register calls. It is separate from
`z80_fastcall`: fastcall is the single-argument z88dk register convention,
while all-register is the multi-argument register ABI. Both must be named
explicitly in the PR inventory and tested without z88dk.

## Stop criteria

Stop and split the work further if any of these occurs:

- a PR needs z88dk to compile or run its tests;
- a commit mixes ABI support with RC700 or z88dk bridge changes;
- a test checks only that code generation succeeds;
- a correctness fix cannot be reduced to an independent reproducer;
- upstream API changes make a historical cherry-pick misleading;
- the diff cannot be explained file-by-file to the maintainer.

## Definition of done

The first phase is complete when a clean branch from upstream contains a
focused calling-convention PR, its tests pass with no z88dk dependency, the
full relevant lit suites are green, and the PR description accurately states
what is and is not covered. Later correctness and optimization work remains
separate until it meets the same standard.
