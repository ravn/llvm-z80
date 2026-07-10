# Plan: native z88dk calling-convention support in ravn/llvm-z80 clang

Date: 2026-07-10.  Goal context: make llvm-z80 clang a **full** CP/M compiler
that consumes the **official** z88dk classic clib with no shims — every clib
function's declared calling convention honoured by construction, not by
accidental coincidence.

## Question asked

"Can clang support sdcccall(0), or does it not make sense?" — and, by extension,
the two z88dk conventions the classic clib actually uses.

## Verified findings (this session, from source — not assumption)

### sdcccall(0) — ALREADY SUPPORTED, nothing to do
`__attribute__((sdcccall(0)))` = `CallingConv::Z80_SDCCCall0` (128) is fully
implemented: args on stack, caller cleanup.  `include/sys/compiler.h` maps
`__smallc -> sdcccall(0)`; console I/O and the divide helpers work through it.
So "can clang support sdcccall(0)" = **yes, it already does.**  The gap for a
full CP/M compiler is the other two classic conventions.

### The two real gaps
z88dk classic clib decorates functions with three attributes.  Mapping today
(`include/sys/compiler.h`, clang branch):

| z88dk attribute     | meaning                                  | clang today            | status |
|---------------------|------------------------------------------|------------------------|--------|
| `__smallc`          | args on stack, **caller** cleanup        | `sdcccall(0)`          | ✅ works |
| `__z88dk_callee`    | args on stack, **callee** cleanup        | `z80_callee` (cc131)   | ✅ works |
| `__z88dk_fastcall`  | single arg in fixed reg (L/HL/DEHL)      | `z80_fastcall` (cc130) | ✅ works |

`__z88dk_fastcall` no-op is only safe for a single 16-bit arg (HL==HL, pinned by
`z88dk/test/clang/fastcall_abi_16.c`).  It **mismatches** for 8-bit (z88dk L vs
clang A) and 32-bit (z88dk DEHL vs clang HLDE) — verified from `rs232_put.asm`
(`ld a,l`), `swapendian.asm` (HL), `CallingConv.h:301` (clang i8->A, i32->HLDE).
`__z88dk_callee` has no clang equivalent at all.

### Feasibility — HIGH.  Infrastructure is proven; the fork did this pattern TWICE.
`Z80_SDCCCall0` (128) and `Z80_AllReg` (129) were both added the same way.  The
full plumbing chain is understood and each hook has an obvious extension point:

1. `clang/include/clang/Basic/Attr.td` — attribute spelling.
2. `clang/lib/Sema/SemaDeclAttr.cpp` (~5599, ~5887) + `SemaType.cpp` (~7826) —
   attribute -> `CallingConv` enum.
3. `clang/include/clang/Basic/Specifiers.h` (~316) — `CC_Z80*` enum.
4. `clang/lib/CodeGen/CGCall.cpp` (~127, ~347) — clang CC -> `llvm::CallingConv`.
5. `clang/lib/AST/{Itanium,Microsoft}Mangle.cpp`, `TypePrinter.cpp` — print/mangle.
6. `clang/lib/Basic/Targets/Z80.cpp` (~167) — validate CC for target.
7. `llvm/include/llvm/IR/CallingConv.h` (~300) — LLVM CC enum (add 130, 131).
8. `llvm/lib/Target/Z80/Z80CallLowering.cpp` — arg classify + return + cleanup.
9. `Z80CallLowering.h` `CallingConvRegs` + `getRegsForCC()`; `Z80CallingConv.td`
   / `Z80RegisterInfo.cpp` CSR lists + reg masks.

The lowering is **fully parameterized** by a `CallingConvRegs` struct selected in
`getRegsForCC(CC)` (Z80CallLowering.h).  Return i32 word order is a config field
(`Ret_I32_Hi/Lo`), so DEHL-vs-HLDE is a data change, not new control flow.  The
callee-stack-cleanup mechanism (`RET_CLEANUP` pseudo + `StackParamBytes` +
`ADJCALLSTACKUP` emitted right after `CALL`, Z80CallLowering.cpp:250/703/1153ff)
**already exists** and is exercised by sdcccall(1) with stack-overflow args — so
`__z88dk_callee` reuses it wholesale.

## Verdict

Supporting the full z88dk classic clib ABI natively **makes sense and is
feasible** with the existing, proven CC-plumbing pattern.  Neither gap needs new
compiler machinery — both reuse mechanisms already in the tree:
- `__z88dk_fastcall` = a swapped/byte-precise `CallingConvRegs` config for a
  single arg + matching return regs.
- `__z88dk_callee`  = sdcccall(0) arg layout + force callee cleanup (mechanism
  already present).

## Plan (incremental, each step red-green tested; issues-only upstream)

**Phase A — `__z88dk_fastcall` — DONE (2026-07-10), red-green validated.**
Implemented and merged into the working tree (uncommitted):
- `CallingConv.h`: `Z80_Z88dkFastCall = 130`.
- `Z80CallLowering.{h,cpp}`: `First_I8` field, `classifyArgFastCall` (single
  arg L/HL/DE:HL), third `CCRegsFast` config + `getRegsForCC` branch.  No CSR
  hook needed (return regs L/HL/DE volatile; IX-CSR unchanged; register arg =>
  StackArgOffset==0 => no RET_CLEANUP).
- `SM83CallLowering.cpp`: consistent but UNVERIFIED SM83 fastcall config.
- clang: `__attribute__((z80_fastcall))` -> `CC_Z80FastCall` ->
  `Z80_Z88dkFastCall`, wired across Attr.td/AttrDocs.td/Specifiers.h/
  Targets/Z80.cpp/CGCall.cpp/SemaType.cpp/SemaDeclAttr.cpp/ItaniumMangle.cpp/
  MicrosoftMangle.cpp/TypePrinter.cpp.
- z88dk `include/sys/compiler.h`: `#define __z88dk_fastcall
  __attribute__((z80_fastcall))` (was a no-op).
- Tests: `llvm/test/CodeGen/Z80/z88dk-fastcall.ll` (backend, PASS),
  `clang/test/CodeGen/z80-fastcall.c` (frontend IR, PASS),
  `z88dk/test/clang/fastcall_abi_16.c` (red-green contract).  Z80 suite 186
  PASS + 6 XFAIL; Z80 clang 10/10.  End-to-end verified through the real
  compiler.h: `__z88dk_fastcall` -> `cc130`.
Deferred (open item, not blocking): a Sema diagnostic rejecting >1 fastcall
arg (backend degrades safely to stack today); confirm z88dk's exact multi-arg
rule first.

Original Phase A checklist (for reference):
1. LLVM: add `Z80_Z88dkFastCall = 130` to CallingConv.h with an ABI comment
   (i8->L, i16->HL, i32->DEHL; single argument; return in same regs).
2. Backend: add a third `CallingConvRegs` (fastcall) — arg+return L/HL/DEHL —
   and branch it in `getRegsForCC`; restrict `classifyArg` to the single-arg
   case (2nd+ arg = error/illegal, matching z88dk).  CSR: caller-saved (no
   stack).  No callee cleanup.
3. clang: new attribute `__attribute__((z80_fastcall))` (or reuse a spelling) ->
   `CC_Z80Z88dkFastCall`; wire Sema/CGCall/Specifiers/TypePrinter/Targets.
4. Tests — see the dedicated "Test plan (fastcall)" below.
5. Header: `include/sys/compiler.h` clang branch
   `#define __z88dk_fastcall __attribute__((z80_fastcall))`; remove the
   "no-op only safe for 16-bit" caveat; extend
   `z88dk/test/clang/fastcall_abi_16.c` with i8/i32 siblings, red-green.

### Test plan (fastcall) — four shapes, each red-green (fails on unmodified clang)
Files: `llvm/test/CodeGen/Z80/z88dk-fastcall-*.ll` (+ mirror `.c` through clang).
Each CHECK is written to FAIL on today's no-op behaviour (which puts i8 in A,
i32 in HLDE) so committing the test BEFORE the impl gives a real RED.
- **(a) exact pattern** — one arg, one call, per width:
  - i8:  arg must be read from **L** (CHECK: `ld a, l` in callee / `ld l,` at call
    site), and `CHECK-NOT: ld a,` for the argument load → today's `ld a` FAILS.
  - i16: arg in **HL** (already passes today — positive control, see (c)).
  - i32: arg in **DE:HL** (DE=high, HL=low).  CHECK the word order explicitly;
    `CHECK-NOT` the HLDE order → today's HLDE FAILS.
- **(b) structural variations** — return value path (not just args):
  - return i8/i16/i32 from a fastcall fn → result in L/HL/DEHL (CHECK the same
    register discipline on the RET side; i32 return word-order pinned).
  - fastcall fn that is only declared+called (caller side) vs. defined here
    (callee side) — both directions pinned.
  - pointer arg (i16) and `enum`/`_Bool` (i8) to confirm width classification,
    not C type, drives the register.
- **(c) positive controls** — must still pass, guard against collateral damage:
  - a plain (default-CC) function in the same TU still uses A / HL / HLDE.
  - a `sdcccall(0)` (`__smallc`) function still stacks its args + caller cleanup.
  - the existing i16 fastcall coincidence keeps working.
- **(d) safety / boundary** — the CC must reject what z88dk fastcall forbids:
  - **>1 argument** on a fastcall decl → a clean diagnostic (clang Sema error),
    NOT silent wrong codegen.  (Confirm z88dk's single-arg rule first — open
    item.)  Test both the error and that a 1-arg decl compiles.
  - **0 arguments** (void) fastcall → no arg lowering, plain call.
  - struct-by-value / >32-bit arg → diagnostic or documented stack fallback,
    whichever z88dk does; pin whichever we choose so it can't drift silently.
  - a fastcall call inside a larger expression (arg is itself a call result) →
    no clobber of the fixed reg before the call.
- **clang C level**: `z88dk/test/clang/fastcall_abi_{8,16,32}.{c,sh}` red-green
  runners mirroring (a)+(b), so the header mapping is validated end-to-end, not
  just the LLVM IR path.

**Phase B — `__z88dk_callee`.  ✅ DONE (validated 2026-07-10, uncommitted).**
Backend `Z80_Z88dkCallee = 131` (CallingConv.h); `isCalleeCleanup -> true` for
cc131 (forced, independent of return size); arg layout + return regs identical to
sdcccall(0) (getRegsForCC maps cc131 -> CCRegs0).  clang `z80_callee` attr wired
across the 13 sites (mirrors `z80_fastcall`).  Header maps `__z88dk_callee ->
__attribute__((z80_callee))` (was latent-bug no-op).  Tests: `z88dk-callee.ll`
(XFAIL removed, PASS) + `z88dk-callee-controls.ll` (PASS) + clang `z80-callee.c`
(PASS).  End-to-end via `sys/compiler.h` -> cc131 confirmed.  Full Z80 suite 187
PASS + 5 XFAIL; 11 Z80 clang tests PASS.  Key invariant pinned: cc131 i32-return
STILL callee-cleans (BC-fallback pop/push) whereas cc128 i32-return caller-cleans.
1. LLVM: add `Z80_Z88dkCallee = 131`.
2. Backend: arg layout identical to sdcccall(0) (stack, same order); force
   `isCalleeCleanup -> true` for this CC (independent of return size); reuse
   `RET_CLEANUP` + `StackParamBytes`.  Verify caller emits `ADJCALLSTACKUP`
   after CALL and callee emits `RET_CLEANUP`.
3. clang attribute `z80_callee` -> `CC_Z80Z88dkCallee`; wire the chain.
4. Tests — see the dedicated "Test plan (callee)" below.
5. Header: map `__z88dk_callee`; remove its no-op; add a red-green test.

### Test plan (callee) — four shapes, each red-green
Files: `llvm/test/CodeGen/Z80/z88dk-callee-*.ll` (+ clang `.c`).  The pivotal
invariant is **exactly-once** stack cleanup: callee pops, caller does NOT.
- **(a) exact pattern** — a 1- and 2-arg callee-cleanup fn:
  - callee epilogue reclaims N bytes (CHECK `RET_CLEANUP`/pop-N sequence);
    `CHECK-NOT` a caller-side `ADJCALLSTACKUP`/`pop` that also reclaims → catches
    a DOUBLE-pop (stack corruption) which today's no-op would produce.
- **(b) structural variations**:
  - arg byte-counts: 1×i8 (2 B slot?), 1×i16, 2×i16, mixed i8+i16+i32 → CHECK
    the exact N popped matches the pushed size for each.
  - return sizes ≤16 vs >16 bits (callee cleanup here is CC-forced, independent
    of return size — verify it does NOT fall back to caller cleanup for i32
    return, unlike sdcccall(1)).
  - PUSH order matches `__smallc`/sdcccall(0) (open item — confirm vs SDCC asm).
- **(c) positive controls**:
  - a `__smallc` (sdcccall(0)) fn in the same TU still does CALLER cleanup
    (single-pop on the caller side) — proves we didn't flip the wrong CC.
  - default-CC and fastcall fns unaffected.
- **(d) safety / boundary**:
  - variadic-ish / 0-arg callee fn → no spurious pop.
  - a callee-cleanup call whose result feeds another call → SP balanced across
    both (no leak/underflow); ideally a `runtime-tests` fixture that returns a
    sentinel and would mis-return on an unbalanced stack.
  - recursion + `+static-stack` interaction is OUT (static-stack is
    non-reentrant) — document, don't test-force.

**Phase C — validate against the real clib + remove `__STDC_ABI_ONLY` gating.**
1. With both attributes real, the `#ifndef __STDC_ABI_ONLY` gates that hide
   `_callee`/`_fastcall` clib decls from clang can be dropped incrementally;
   re-expose and compile-test headers (stdio fileno/perror, fcntl readbyte,
   malloc.h Heap*) — these become correctly-lowered instead of hidden.
2. Re-run the dcc benchmark migration (sieve/e/ttt/tm) end-to-end on official
   libs; confirm no regression.  Then extend to a broader clib surface.
3. The earlier "poison tripwire" idea becomes unnecessary — the attributes now
   carry real meaning, so a mis-decl is a wrong-CC bug caught by tests, not a
   silent no-op.

## End state — this work is destined UPSTREAM (build it upstream-ready from day 1)

Goal (user, 2026-07-10): "de skal ende upstream når vi engang er færdige" — the
z88dk calling-convention support AND its tests should land upstream once
finished.  The tests are written NOW and committed to the canonical tree in an
**XFAIL (fail-but-ignored)** state so they can go upstream immediately without
breaking CI, and flip to real PASS when the feature lands (see next section).
Routing (per the staged-collaboration model + the PR-#17 lesson):

- **Target = ravn/llvm-z80 (the fork-of-record, owner @zlfn).**  These are
  Z80-target-specific calling conventions + Z80 lit tests — they belong in the
  Z80 backend, NOT in generic llvm/llvm-project.  The "issues-only, never file
  fixes" rule applies to **generic** llvm/llvm-project bugs; the fork accepts
  code contributions the contributor can fully explain (that was the PR-#17
  retraction lesson — explain root cause, don't dump code).
- **Build upstream-quality from the start**, so submission is a *packaging*
  step, not a rewrite:
  - lit tests go in the canonical tree (`llvm/test/CodeGen/Z80/`) with proper
    FileCheck, RUN lines, and ABI comments — the same files we submit.
  - clang attribute plumbing follows the exact shape of the existing
    `sdcccall`/`z80_allreg` attributes (Attr.td docs included) so it reviews
    like native code.
  - each phase is its own self-contained, explainable changeset (CC enum +
    lowering + attribute + tests) — one reviewable unit, root-cause documented.
- **Before any actual upstream submission**: explain root cause + get explicit
  per-filing go-ahead (`feedback_explain_before_filing`).  If any sub-piece
  turns out to be generic-LLVM (not Z80-specific), it routes to
  llvm/llvm-project as an ISSUE with a testcase, never a fix PR.

### Upstream failure mode = XFAIL (fail-but-ignored) — the ordering discipline
The user wants the tests to **fail upstream in an ignored way**.  Mechanism =
LLVM lit `XFAIL`, the house convention (suite is already "164 PASS + 6 XFAIL";
CLAUDE.md §5 "XFAIL lit test per bug").  Workflow per phase:
1. **Write the test first, committed as `XFAIL: z80` (or `XFAIL: *`)** with a
   comment linking the tracking issue.  Against the unmodified compiler it
   FAILS, but lit reports it as an *expected* failure → CI stays green, the gap
   is documented in-tree, and it is safe to submit upstream immediately.
   - The CHECK lines encode the FINISHED behaviour (i8→L, i32→DEHL, single-pop
     callee cleanup, …), so the XFAIL is a genuine red for the right reason.
   - This is exactly the "red-but-ignored" convention already used for the xcc
     bug repros (`reference_z88dk...` / xcc-issue-filing-process).
2. **Implement the phase.**  The now-correct codegen makes the test PASS.
3. **Remove the `XFAIL` line** in the SAME changeset as the impl.  If the impl
   works but you forget, lit emits **`XPASS` (unexpected pass) → CI FAILS**,
   which is the built-in tripwire that forces the marker off.  Net: the test can
   never silently drift — it is red-ignored before, green-required after.
4. Never weaken a CHECK to make an XFAIL "pass"; the XFAIL is removed only by
   real codegen, proven by the red→green transition (matches the mandatory
   red-green rule, just expressed in lit's own vocabulary).

Applies to BOTH test plans below (fastcall and callee): every new `.ll`/`.c`
lands XFAIL first, un-XFAILs exactly when its phase's codegen is in.

## Constraints / conventions (from user + CLAUDE.md)
- Upstream routing: Z80-specific → ravn/llvm-z80 fork (explainable code
  contribution, explain+go-ahead first).  Generic-LLVM → llvm/llvm-project,
  ISSUES-ONLY, never fixes.
- Every fix red-green validated + a permanent test (lit is the CI gate;
  `build-and-lit` + `runtime-tests`).
- Commit locally freely; push only at `--no-ff` merges.
- No benchmark currently *needs* these — this is the full-CP/M-compiler goal, so
  correctness-by-construction is the deliverable even without a failing user.

## Open items to verify at implementation time (labelled: NOT yet confirmed)
- Exact z88dk `__z88dk_fastcall` behaviour for 0-arg / >1-arg decls (assumed
  strictly single-arg; confirm against z88dk docs/asm before restricting).
- Whether `__z88dk_callee` arg PUSH order matches sdcccall(0) exactly (assumed
  yes; confirm with an SDCC `--reserve-regs-iy`/asm reference).
- Attribute spelling choice: new `z80_fastcall`/`z80_callee` vs. extending
  `sdcccall(...)` with named ABIs (naming/mangling impact).
