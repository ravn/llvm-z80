# Plan: composable calling-convention modifiers for z88dk (ravn/llvm-z80 #282)

Date: 2026-08-07.  Supersedes the "one new CC per combination" framing.

## User directive (2026-08-07)

> "Jeg vil gerne have en mekanisme der kan samle flere konventioner i stedet
>  for at lave en ny for hver kombination af konventioner."

i.e. **compose** orthogonal convention modifiers, not hand-author a monolithic
`CallingConv` for every combination (smallc+callee, stdc+callee, callee-alone,
smallc-alone, callee+preserves_regs, …).

## Why #282 exists

`<graphics.h>` and much of the classic clib decorate functions
`__smallc __z88dk_callee` = **left-to-right push (from `__smallc`) + callee
stack cleanup (from `__z88dk_callee`)**.  clang-z80 has `z80_smallc`
(L→R, caller-clean) and `z80_callee` (R→L, callee-clean) as **separate whole
conventions**; neither is the combination, and (post #281) stacking both now
*errors*.  So the real z88dk convention cannot be expressed.

## The conventions are orthogonal axes, not points

z88dk decorations decompose into independent axes (verified from
`z88dk/include/sys/compiler.h` + usage survey 2026-08-07):

| Axis            | Values (z88dk spelling)                              | Set by |
|-----------------|-----------------------------------------------------|--------|
| **arg order**   | left-to-right (`__smallc`) / right-to-left (`__stdc`, sdcccall default) | order modifier |
| **stack cleanup** | caller (default) / callee (`__z88dk_callee`)      | cleanup modifier |
| **arg location** | stack (smallc/stdc) / registers (`__z88dk_fastcall`, sdcccall1, `__z88dk_allreg`) | reg modifier |
| **preserved regs** | default clobber / `__preserves_regs(...)`         | opt modifier (already composable) |

Usage survey (`z88dk/include`): `__smallc __z88dk_callee` appears **677×**;
`__z88dk_callee` **1504×** total (so ~827 with a *different* base, incl.
standalone `intrinsic_ldi(...) __z88dk_callee`, `fmin_callee(...) __z88dk_callee`,
and reversed `__z88dk_callee __smallc`).  A monolithic CC per reachable point
would be ~6-8 hand-authored conventions and grows with every new pairing — the
thing the user is rejecting.

## Hard constraint: ABI axes must live in the function TYPE

`preserves_regs` (the existing composable modifier, `Z80PreservesRegsAttr`) can be
a pure **decl** attribute → IR fn-attribute string `"z80-preserves-regs"="d,e"`,
read at the call site via `Info.CB->getCalledFunction()` (Z80CallLowering.cpp
~1108).  It works because it is an **optimization** (RegMask narrowing): an
unknown/indirect callee just falls back to the conservative mask — still correct.

**Arg order and cleanup are ABI correctness, not optimization.**  The caller must
know them at *every* call site, including **indirect calls through function
pointers** (e.g. `qsort`'s `__smallc` comparator, and any `_callee` fn taken by
address).  There is no `getCalledFunction()` there.  Therefore order+cleanup MUST
be carried in the **function type**, which in LLVM means the type's
`CallingConv::ID` (the only ABI channel in `FunctionType`'s ExtInfo).

**Consequence:** true "compose to an open-ended set" is impossible for the ABI
axes within LLVM's single-enum-per-type model.  What IS achievable — and is what
the directive really needs — is **composition that resolves to a finite,
axis-structured CC space**, with *no per-combination hand-authoring*.

## Recommended design: axis-encoded CC + clang composition layer

Two complementary pieces.

### A. ABI axes → a bitfield-structured Z80 CC range (backend decodes axes)

Reserve a contiguous `CallingConv::ID` block for Z80 whose low bits encode the
ABI axes, e.g.:

```
bit 0: order   0=right-to-left        1=left-to-right (smallc)
bit 1: cleanup 0=caller               1=callee
bit 2: args    0=stack                1=registers(sdcccall1-style)   // future
... (fastcall/allreg handled by their existing distinct configs)
```

- The backend's CallLowering is **already axis-parameterized**: `getRegsForCC()`
  (reg config), `isCalleeCleanup()` (cleanup), and the `IsSmallC`/`PushForward`
  gates (order, Z80CallLowering.cpp:616/947).  Today they `==`-match named enum
  values; they change to **decode the axis bits**.  So the backend gains NO new
  per-combination code — it reads bits.
- Existing named CCs (`Z80_SDCCCall0`, `Z80_SmallC`, `Z80_Z88dkCallee`,
  `Z80_Z88dkFastCall`, `Z80_AllReg`) are re-expressed as specific bit patterns
  (or kept as aliases for source/back-compat).  `__smallc __z88dk_callee` =
  `order=L2R | cleanup=callee` — a value that already "exists" in the encoding,
  authored by nobody.

### B. clang: orthogonal modifier attributes that COMPOSE (not conflict)

Today `z80_smallc`/`z80_callee` are each a full `CC` attribute → they compete for
the type's single CC → (post #281) they conflict.  Change them to **axis
modifiers**:

- Each attribute sets its own axis bit on the type's CC, starting from a
  base (default sdcccall1, or sdcccall0 for the stack conventions).
- Composition happens when finalizing the function type's CC: collect the axis
  attributes present and compute the resulting CC value.
- **Same-axis conflicts still error** (via the #281 machinery, generalized):
  `__smallc __stdc` (two orders) or `__z88dk_callee` + a caller-clean order that
  fixes cleanup the other way → diagnosable.  **Different-axis attributes compose
  silently** (the intended `__smallc __z88dk_callee`).

This is the crux that makes #281 and #282 coherent: **#281 = conflicting attrs on
the SAME axis error; #282 = attrs on DIFFERENT axes compose.**  The composition
layer is what distinguishes the two.

### C. Keep the decl-attr modifier pattern for non-ABI axes

`preserves_regs` stays exactly as it is (decl attr → fn-attr string, optimization
only).  It already composes with any CC.  It is the precedent that shows the
"modifier not a whole convention" shape is welcome in this codebase — we extend
the idea to the ABI axes via (A)+(B) because those additionally must be
type-carried.

## Header integration (z88dk side)

With (B), `include/sys/compiler.h` maps each z88dk keyword to its single axis
attribute (clang branch):
- `__smallc`        → order=L2R attribute
- `__stdc`          → order=R2L attribute (if needed)
- `__z88dk_callee`  → cleanup=callee attribute
- `__z88dk_fastcall`→ (unchanged, distinct reg config)

Then the *existing* source `__smallc __z88dk_callee` composes with **no header
churn** and **no per-call-site edits** — exactly the goal.  sccz80/sdcc keep
composing the real keywords as before (they are unaffected; this is all in the
`#if __clang__`/`__LLVMZ80` branch).

## Interaction with #281 (already fixed)

#281 registered the Z80/SDCC CC kinds in `AttributedType::isCallingConv()` so
conflicting **whole-CC** attributes error.  Under this plan the z88dk keywords
become **axis modifiers**, so `__smallc __z88dk_callee` no longer trips that error
(different axes).  The #281 error is retargeted to genuine **same-axis**
conflicts.  Net: #281's safety net stays; #282 makes the legitimate combination
compose instead of error.

## Affected hooks (extends the 9-hook chain from plan-2026-07-10)

1. `clang/include/clang/Basic/Attr.td` — keep spellings; semantics become axis
   modifiers (may add `z80_stdc`/order spelling if needed).
2. `clang/lib/Sema/SemaDeclAttr.cpp` + `SemaType.cpp` — **the composition layer**:
   compute the type CC from the set of axis attributes; same-axis conflict
   diagnostic (reuse #281 path); different-axis compose.
3. `clang/include/clang/Basic/Specifiers.h` — axis-structured `CC_Z80*` values (or
   keep names as encoded aliases).
4. `clang/lib/CodeGen/CGCall.cpp` — clang CC → `llvm::CallingConv` (bit mapping).
5. `clang/lib/AST/*Mangle.cpp`, `TypePrinter.cpp`, `Type.cpp`
   `getNameForCallConv`/`isCallingConv` (touched by #281) — names for the encoded
   values.
6. `clang/lib/Basic/Targets/Z80.cpp` — validate the CC range.
7. `llvm/include/llvm/IR/CallingConv.h` — reserve the axis-encoded Z80 block.
8. `llvm/lib/Target/Z80/Z80CallLowering.cpp` — `getRegsForCC` / `isCalleeCleanup`
   / `IsSmallC`/`PushForward` **decode axis bits** instead of name-matching.
9. `Z80CallingConv.td` / `Z80RegisterInfo.cpp` — CSR/regmask per reg-axis (mostly
   unchanged; stack conventions share CCRegs0).

## Phasing (each step red-green; issues-only upstream, no fix-PRs)

- **Phase 0 — spike/validate the encoding.** Decide the bit layout + which named
  CCs alias which patterns.  Pure design note + a couple of `.ll` CallLowering
  lit tests pinned to the *current* named CCs (regression guard before refactor).
- **Phase 1 — backend axis-decode.** Rewrite `getRegsForCC`/`isCalleeCleanup`/
  order gates to decode bits; prove byte-identical output for existing
  `Z80_SmallC`/`Z80_Z88dkCallee`/`Z80_SDCCCall0` via existing lit tests
  (`z88dk-callee.ll`, `z88dk-callee-controls.ll`, etc.) — no behavior change yet.
- **Phase 2 — the composed value.** Wire `order=L2R | cleanup=callee` end to end;
  new lit test: a `__smallc __z88dk_callee` callee (a) compiles (no #281 error),
  (b) call site pushes x-first/left-to-right, (c) callee cleans. Backend `.ll` +
  `clang/test/CodeGen` frontend test.
- **Phase 3 — clang composition layer.** Make `z80_smallc`/`z80_callee` axis
  modifiers; same-axis conflict still errors (extend
  `clang/test/Sema/z80-conflicting-callconv.c`), different-axis composes (new
  positive test).
- **Phase 4 — header + end-to-end.** `include/sys/compiler.h` maps keywords to
  axis attrs; rebuild `rc700.lib`; the rc700 `<graphics.h>` demos
  (`examples/rc700/{sine,mandelbrot}.c`) build under `-compiler=llvmz80` and
  render correctly in MAME rc702 (the closed loop for the whole thread:
  `scratch/sine-demo/snap/gfxscc.png` is the sccz80 oracle to match).

## Open questions to resolve in Phase 0

- Exact bit layout and whether to keep the 5 existing named CCs as aliases or
  renumber (renumber = cleaner, but touches more of the 9 hooks + any hardcoded
  `130/131` in the tree).
- Whether the **reg axis** (sdcccall0-stack vs sdcccall1-registers) folds into the
  same bitfield or stays selected by `getRegsForCC` config objects (likely the
  latter — it's a richer struct, not one bit).
- Return-register mapping (sdcccall0 L vs sdcccall1 A; DEHL vs HLDE) is already a
  `CallingConvRegs` data field — confirm it rides along with the reg axis, not the
  order/cleanup bits.
- Diagnostic wording for same-axis conflict ("two argument-order attributes",
  "conflicting stack-cleanup") — nicer than the generic "not compatible".

## Non-goals

- No change to sccz80/sdcc behavior (all in the clang/`__LLVMZ80` branch).
- No general LLVM "composable CC" framework — scoped to the Z80 CC block.
- No fix-PR upstream; #282 stays an issue, work lands in ravn/llvm-z80 per
  [[feedback_no_pull_requests]] / fork policy.
```
