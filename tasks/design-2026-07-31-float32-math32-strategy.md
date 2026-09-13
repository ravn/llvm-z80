# Design: 32-bit `double` + z88dk math32 float runtime (measured)

Date: 2026-07-31, substantially revised 2026-08-01 with **measured** ABI facts,
a working Path X prototype, and a cycle benchmark.
Status: Phase 0 committed; Path X implemented, gated, and verified end-to-end
(both the compiler CC gate and the z88dk bridge now committed).
Tracking: ravn/llvm-z80 #277. Upstream discussion: llvm-z80/llvm-z80 #34.
Related: #276 (LLVM-24 merge), `plan-2026-07-10-z88dk-calling-conventions.md`.

> NOTE: an earlier draft of this document stated clang passes 32-bit floats in
> "DEHL (D=MSB)". That was wrong. Disassembly of a real call site (2026-08-01)
> shows clang uses **HLDE (HL = high word)**. All ABI claims below are measured,
> not assumed.

---

## 1. Goal and decision

Make `double`/`long double` 32-bit IEEE-754 binary32 and reuse z88dk's **math32**
runtime, inventing as little as possible. The decisive reason (clarified
2026-08-01): **math32 provides the full libm** (sqrt/sin/cos/exp/log/pow/...),
which llvm-z80's own float runtime does NOT. See §3.

Decision:
1. `double` = `long double` = 32-bit binary32 (Phase 0, committed).
2. Reuse math32 for the z88dk-classic path.
3. **Path X** (a calling-convention change) is the chosen mechanism: emit the
   f32 arithmetic libcalls with `CallingConv::Z80_SDCCCall0`, which already
   matches math32's ABI, so the z88dk bridge is a pure alias with zero glue.
   **Path Y** (a library-side shim, §8) is a working fallback needing no
   compiler change.

---

## 2. Why 32-bit `double` gives the fewest typecasts (measured)

C's usual conversions insert `float`->`double` promotions (variadic `printf`,
unsuffixed constants, `math.h`). With 64-bit `double` each is a real
`__extendsfdf2`. Making `double` == `float` == binary32 turns them into identity
no-ops. Measured on `build-macos` clang (Phase 0):

```
float  f + float  f -> ___addsf3
double d + double d -> ___addsf3          (same 32-bit libcall)
(double)aFloat       -> (no call)         identity
```

Zero `__extendsfdf2`/`__truncdfsf2`; every FP op is a 32-bit `sf` libcall. The
13-line change is in `clang/lib/Basic/Targets/Z80.cpp` (`DoubleWidth`/
`DoubleFormat`/`LongDoubleWidth`/`LongDoubleFormat`), test
`clang/test/CodeGen/z80-double-is-float32.c`. **Committed** on branch
`float32-math32` (`dafa0a1a79c0`).

---

## 3. Two float runtimes exist — and only math32 has libm

| | llvm-z80 compiler-rt (ELF path) | z88dk math32 (z88dk path) |
|---|---|---|
| where | `compiler-rt/lib/builtins/z80/*.asm`, ships with clang-z80 | z88dk `libsrc/math/float/math32/` |
| ABI | register (sdcccall(1)): arg1 HLDE, arg2 stack, result HLDE, callee-clean | stack (sdcccall(0)): both args stack, result DEHL |
| arithmetic (`+ - * /`, compare, convert) | yes | yes |
| **libm / math.h** (sqrt/sin/cos/exp/log/pow) | **NO** | **YES (full)** |
| used by | standalone `--target=z80` (ld.lld ELF) | `zcc +cpm -compiler=llvmz80` |

The ELF path's `__addsf3` is zlfn's own hand-written IEEE-754 binary32 runtime,
written to match clang's default sdcccall(1) ABI (`compiler-rt/.../addsf3.asm`
header: `Input: HLDE = a, stack = b; Output: HLDE = a + b; callee-cleanup`).

**The gap:** `zcc` links z88dk's libraries, not llvm-z80's compiler-rt, so
`__addsf3` is unresolved in a z88dk build. Reusing math32 fills the gap AND
brings libm for free. Writing libm from scratch is infeasible; this is what
justifies the whole integration.

---

## 4. Measured ABI (ground truth, disassembly 2026-08-01)

clang emits the standard compiler-rt `sf` names. Full arithmetic surface:
`__addsf3/__subsf3/__mulsf3/__divsf3`; compares canonicalise to three
(`__cmpsf2/__gesf2/__gtsf2`); conversions `__floatsisf/__floatunsisf/__fixsfsi/
__fixunssfsi`.

**clang's default C ABI (sdcccall(1)) for a 32-bit value:**
- arg1 in **HLDE**: **HL = high word (MSB byte in H), DE = low word**.
- arg2 on the stack.
- result in **HLDE** (HL = high word). callee cleans.

Confirmed by disassembling `float g(void){return va+vb;}` — at `call ___addsf3`,
`HL = va_hi`, `DE = va_lo`, `vb` on the stack.

**z88dk math32 cores (`m32_fsadd` etc.):** operand on the stack + operand in
**DEHL (D = MSB)**, result **DEHL**, entered by `jp` so the stack operand sits
directly above the core's return address. This is the sccz80/SmallC HL-centric
heritage (see `plan-2026-07-10` and §7).

So clang's HLDE (HL=high) is the **word-swap** of math32's DEHL (DE=high) — the
same half-swap the existing integer bridges already handle with `ex de,hl`
(`z88dk/libsrc/l/llvmz80/__divsi3.asm`: *"clang ABI: HL:DE (HL=high); core ABI:
DE:HL; one `ex de,hl`"*).

---

## 5. The backend already has sdcccall(0) — and it matches math32

`Z80CallLowering.cpp` defines per-convention register tables:

| | sdcccall(1) (C default) | sdcccall(0) (`Z80_SDCCCall0`) |
|---|---|---|
| args | arg1 HLDE, arg2 stack | both on stack |
| 16-bit return | DE | **HL** |
| 32-bit return | `Ret_I32_Hi=HL` (HL=high) | **`Ret_I32_Hi=DE` (DE=high)** |
| cleanup | callee | caller |

`Z80_SDCCCall0` already returns 32-bit in **DEHL (DE=high)** and 16-bit in HL —
exactly math32's convention. So no new calling convention is needed: the f32
libcalls just need to be emitted with `Z80_SDCCCall0`, and the z88dk bridge
becomes a pure alias.

---

## 6. Path X (chosen): the calling-convention change + pure alias

**Backend (one call site).** The f32 arithmetic is legalized by *custom* code in
`Z80LegalizerInfo.cpp` (not `.libcallFor`), which hardcoded `CallingConv::C`.
Changing that one `createLibcall(...)` to `CallingConv::Z80_SDCCCall0` makes
clang push both float operands on the stack and read the result in DEHL.
(Verified: after the change, a call site emits 4 `push` + `call ___addsf3` +
`pop af`x4.) **This is why setting the CC in `RuntimeLibcalls.cpp` had no
effect** — the custom legalizer bypasses the RuntimeLibcalls CC.

**z88dk bridge (pure alias).** With the sdcccall(0) frame, `__addsf3` is a plain
alias to z88dk's **SDCC** float wrappers:
```
___addsf3: jp cm32_sdcc_fsadd
___subsf3: jp cm32_sdcc_fssub
___mulsf3: jp cm32_sdcc_fsmul
___divsf3: jp cm32_sdcc_fsdiv
```
Result: **ALL PASS** on the thorough arithmetic test (`z88dk/test/clang/
runtime_float.c`, bit-exact, order-sensitive sub/div + signed-zero) at
`-O2`/`-O3`/`-Os`. Zero register-order glue.

Use the **`cm32_sdcc_*`** wrappers, NOT `cm32_sccz80_*`: the sccz80 wrappers do a
`switch_arg` (SmallC->SDCC order) which double-swaps clang's already-SDCC-order
operands, breaking sub/div. The SDCC wrappers match sdcccall(0) directly.

---

## 7. Why math32 needs the packed-float / jp contract (hard-won detail)

- `m32_fsadd32x32` is NOT the packed-float entry (it reads the sign from `C`, a
  pre-unpacked operand). The packed-float public entries are
  `m32_fsadd/fssub/fsmul/fsdiv` — they do `ex de,hl; ld b,h; add hl,hl` to unpack
  a packed float from DEHL.
- The core reads the second operand from the stack **above its own return
  address**, so it must be entered by `jp` (as the wrappers do), not `call` — a
  `call` inserts an extra return address and the core reads the wrong bytes.
- Result byte order: math32 returns DEHL (D=MSB); clang's HLDE return needs the
  word-swap (handled by the sdcccall(0) DE-high return in Path X, or by an
  `ex de,hl` trampoline in Path Y).

---

## 8. Path Y (working fallback): library-side shim, no compiler change

If the backend is not changed, clang emits `__addsf3` with the default
sdcccall(1) frame (arg1 HLDE, arg2 stack). The z88dk bridge then:
1. `ex de,hl` (HLDE arg1 -> DEHL for the core);
2. swaps the DEHL operand with the 32-bit stack operand (so a lands on the stack,
   b in DEHL -> correct order for non-commutative sub/div);
3. `jp` the core via a trampoline that `ex de,hl`s the DEHL result back to HLDE.
~15 instructions/op, all in z88dk. Also **ALL PASS** on `runtime_float.c` when
prototyped this way (2026-08-01).

Path X is strictly cleaner (pure alias, zero runtime glue) and is now the
**committed** implementation (§11); this swap-shim variant of `__addsf3.asm`
was superseded and is not what ships. It remains documented here as the
fallback if `-z80-float-sdcccall0` (§10) is ever reverted or deferred.

---

## 9. Cycle benchmark: the "math32 is faster" assumption is only half true

Per-op cycles, cycle-accurate z88dk-ticks, N-difference (1000 vs 3000
iterations, cancels startup), single operand set (3.14159 / 2.71828):

| op | math32 | compiler-rt | winner |
|---|---|---|---|
| add | ~949 | ~1867 | **math32 ~2.0x** |
| mul | ~2325 | ~8921 | **math32 ~3.8x** |
| div | ~24567 | ~10787 | **compiler-rt ~2.3x** |

- math32 is clearly faster for **add/mul** (dominant in typical code).
- **compiler-rt is ~2.3x faster for division** — math32 divides via a
  Newton-Raphson reciprocal (`a/b = a*(1/b)`, iterative, ~24.5 k cycles; this is
  also the source of the 1-ULP `-3/3` result), while compiler-rt divides
  directly (~10.8 k).
- Per-op figures include loop + arg-marshalling overhead (fair: real call cost).
  Data-dependent paths mean these are representative, not worst/best case.

**Implication:** "math32 is faster" holds for add/mul, not div. But math32 is
still required for libm regardless, so it is not either/or; a hybrid (math32 +
compiler-rt's faster div) is possible but couples two runtimes.

### 9a. Isolated ABI-only overhead (2026-08-01) — would porting compiler-rt to sdcccall(0) pay off?

§9 measures *whole algorithms* (different bodies, different ABI) and cannot
answer the narrower question raised while discussing this doc: is the
`Z80_SDCCCall0` shape itself cheaper or more expensive to marshal than the
default `CallingConv::C` shape, holding the arithmetic body constant? If
sdcccall(0) marshalling were *cheaper*, there would be a real argument for
porting compiler-rt to it upstream (removing the shim/flag entirely). If it
is *more expensive*, the current flag + math32-glue design is strictly
better on every axis (correct AND already faster, §9), and there is no
performance case for an upstream ABI change.

Measured directly with a hand-written raw-Z80 microbenchmark (`z88dk z80asm
-b`, run under the cycle-accurate `z88dk-ticks -pc <start> -end <halt-addr>`
emulator — the working invocation, found in `test/suites/bench.sh` /
`src/z80asm/tools/get_emul_ticks.pl`; the earlier `-l file,addr -counter N`
combination silently produced a constant, code-independent 131072 and was
discarded), N=2000 calls, identical 1-instruction (`nop`) body in both call
shapes:

| ABI shape | T-states / 2000 calls | T-states / call |
|---|---|---|
| default `CallingConv::C` (arg1 in HLDE, arg2 on stack, callee-cleanup) | 330 005 | **165.0** |
| `Z80_SDCCCall0` non-destructive-peek shim (both args on stack, caller-cleanup) | 614 005 | **307.0** |

Delta: **+142 T-states per call for the sdcccall(0) shape** (≈ +86%
marshalling overhead), cross-checked by hand-counting the emitted
instructions for both loops (165 and ~311 T-states respectively, matching
the measured figures to within the last-iteration `jr nz` not-taken
correction).

**Conclusion:** porting compiler-rt's arithmetic to `Z80_SDCCCall0` would
make compiler-rt *slower*, not faster (e.g. add: ~1867 T-states today ->
roughly ~1867 - 165 + 307 ≈ **2009 T-states**, i.e. ~7-8% worse), and would
still be nowhere near math32's ~949 T-states, because math32's advantage
over compiler-rt (§9) is algorithmic, not ABI-shape-related. There is
therefore **no performance argument** for an upstream compiler-rt ABI
change here — per the project's own decision rule (change the compiler only
if it demonstrably pays off; otherwise prefer glue code), the current
`-z80-float-sdcccall0` flag + math32-glue design is confirmed as the right
call, not just the pragmatic one.

### 9b. All 7 float ops, math32 vs compiler-rt (2026-08-01, reproducible) — is "route everything to math32" actually optimal?

§9 only covered add/mul/div. This extends the comparison to all 7 f32
libcalls clang can emit (add/sub/mul/div, `<` compare, float->int, int->float),
to check whether the flag's current "always bridge to math32" behaviour is
uniformly the right choice, or only right for some ops.

**Reproducible**: `z88dk/test/clang/bench_math32_vs_compilerrt.sh`
regenerates this whole table (both sides, all 7 ops, plus the
`-ffast-math` compare row in §9c) deterministically — nothing here is
hand-copied from a since-deleted `/tmp` script.

**Method (both sides cycle-accurate via `z88dk-ticks`, N=2000 loop, fixed
operands every iteration):**

- **compiler-rt side**: a standalone freestanding binary (no CP/M CRT) —
  plain portable C compiled `--target=z80 -Os` with no z88dk/sdcccall0
  flag, so clang emits the libcall under its own default ABI; linked
  directly against the prebuilt compiler-rt `.o` for that op;
  `z88dk-ticks -pc <_start> -end <_bench_halt>`. Operands 3.14159f/2.71828f
  (12345.678f for f2i, 12345 for i2f).
- **math32 side**: real `zcc +cpm -compiler=llvmz80 -mllvm
  -z80-float-sdcccall0 -lmath32` build (the exact production pipeline,
  already correctness-verified by `runtime_float.sh`/`runtime_fcmp.sh`/
  `runtime_fconv.sh`), same loop body, `z88dk-ticks` on the resulting
  `.com`.

**Two pitfalls hit while building the script (documented so they aren't
rediscovered):**
- `z88dk-ticks` requires the input file to be *named* `*.com` — it
  string-matches the filename (`ticks_main.c`) to decide whether to
  install the CP/M warm-boot/BDOS trap vectors at addresses 0/5/8/11 *and*
  load the binary at 0x100. Without it, the same bytes load at address 0
  instead (a 0x100 offset error), corrupting the emulated environment from
  the first instruction and reliably segfaulting the *host* `z88dk-ticks`
  process itself (confirmed via `lldb`: a NULL string reaches
  `strcasecmp_l` a few calls deep in the resulting garbage execution). This
  was the earlier session's "math32 side crashes z88dk-ticks" blocker.
- On the standalone compiler-rt side, a `noreturn`-only halt function gets
  **inlined** at `-Os`, so its out-of-line copy (whose address `-end`
  looks up) is dead code the loop never reaches — `z88dk-ticks` then hits
  its internal 200M-cycle safety timeout instead of stopping at the real
  exit point. Fix: also mark it `noinline`.

| op | math32 (T/call) | compiler-rt (T/call) | winner |
|---|---|---|---|
| add | 978.2 | 1877.0 | **math32 ~1.9x** |
| sub | 1202.4 | 2254.0 | **math32 ~1.9x** |
| mul | 2354.2 | 8931.0 | **math32 ~3.8x** |
| div | 24596.3 | 10797.0 | **compiler-rt ~2.3x** |
| compare (`<`) | 1115.4 | 663.0 | **compiler-rt ~1.7x** |
| float->int | 1425.4 | 799.0 | **compiler-rt ~1.8x** |
| int->float | 583.9 | 615.0 | math32 ~1.05x (near tie) |

**Conclusion:** the flag's current "always route to math32" behaviour is
**not uniformly optimal** — it is a clear win for add/sub/mul (the
overwhelmingly common ops in real code) and a clear loss for div, compare,
and f2i, with i2f close to a wash either way. math32's `m32_compare`
(sign/magnitude bit-fiddling plus this session's NaN short-circuit, see
`__cmpsf2.asm`) costs more than compiler-rt's direct IEEE-754 bit-pattern
compare. A future per-op hybrid (bridge add/sub/mul to math32, leave
div/compare/f2i on compiler-rt) is possible in principle, but — per the
project's stated decision rule — is not proposed as a change here without
separate user sign-off; this section only answers the measurement
question asked.

**Why div specifically loses, and a possible upstream angle:** this is
architectural, not glue overhead (unlike compare above). math32 computes
`a/b` as `a * (1/b)` via a **Newton-Raphson reciprocal iteration**
(`m32_fsinv_fastcall` in `f32_fsdiv.asm`: a degree-2 polynomial seed plus
two `X := X + X*(1 - D'*X)` refinement steps) — roughly 8 full float
multiplies + 7 float adds worth of sub-calls. compiler-rt's `___divsf3`
instead does **24-bit restoring binary long division** directly on the
unpacked mantissas — a 24-iteration compare/subtract/shift loop, no
multiply at all. NR needs fewer iterations, but each iteration is an
expensive Z80 shift-and-add float multiply; direct long division needs
more iterations, each a cheap single compare-subtract-shift. On this ISA
(no hardware multiplier) the many-cheap-steps approach wins by ~2.3x.
This is also the source of the 1-ULP `-3/3` rounding discrepancy (§9,
NR's accumulated rounding vs. a direct divider's exact guard/round/sticky
tracking). Full algorithmic writeup with source excerpts:
`z88dk/libsrc/l/llvmz80/MATH32_BRIDGE.md` §5 ("Why div is ~2.3x slower in
math32"). A trade-off report to the z88dk maintainers (compiler-rt's
divider as an alternative/option for `m32_fsdiv` on z80/z180/z80n) is
worth considering but **not filed yet** — it would need to be framed
honestly as a trade-off (NR is a legitimate, correct design choice that
simply loses on this specific ISA), not as a math32 bug, per the
project's explain-before-filing discipline.

### 9c. Isolating the compare NaN-check glue, and closing it under `-ffast-math` (2026-08-01)

Follow-up question: is compare's ~1.7x gap (§9b) the NaN-check glue in
`__cmpsf2.asm`, or `m32_compare` itself? Isolated with matched raw-asm
loop shapes (same driver structure, `z88dk-ticks`): `m32_compare` called
directly (via the nested-return-address wrapper its own contract
requires, see `f32_fscompare.asm`) = **619 T-states/call**; the full
`___cmpsf2` bridge (two `CheckNaN` calls + Z/C-to-tri-state translation)
= **946 T-states/call**. The glue itself costs **327 T-states/call**, most
of the gap to compiler-rt.

math32 has no NaN awareness at all (§4 of the design, `f32_fscompare.asm`
never inspects the exponent/mantissa for a NaN pattern) — the NaN check
exists purely in the bridge, to give correct IEEE semantics under plain
C. LLVM's existing convention for "caller doesn't need NaN correctness"
is `-ffast-math`'s `nnan` flag, not an opt-level gate (opt level must
never change FP semantics). Z80's GlobalISel legalizer already had a
`hasAllFastFlags`-gated `__cmpsf2_fast` dispatch for exactly this case
(pre-existing, `Z80LegalizerInfo.cpp`, zlfn, commit `31997a65c57fe`,
2026-03-12) and compiler-rt already implements `__cmpsf2_fast` for the
default ABI — but the z88dk/math32 sdcccall(0) bridge did not, so
`-ffast-math` builds against math32 failed to link
(`undefined symbol: ___cmpsf2_fast`).

Fixed: `z88dk/libsrc/l/llvmz80/__cmpsf2.asm` gained `___cmpsf2_fast`
(same entry contract as `___cmpsf2`, `m32_compare` direct, no `CheckNaN`
calls). Verified red (undefined-symbol link failure pre-fix) / green
(links + correct on all six ordered predicates, non-NaN operands) via
`z88dk/test/clang/runtime_fcmp_fast.{c,sh}`, and measured reproducibly via
`bench_math32_vs_compilerrt.sh`'s `compare`/`compare_fast` rows: math32
1115.4 -> 811.4 T-states/call = **304 T-states/call** saved, matching the
327 T-state isolated estimate within ~7% (compiler-rt also gains a little,
663.0 -> 602.0, from its own `__cmpsf2_fast`, so the gap narrows from
~1.7x to ~1.3x rather than closing fully). Full writeup:
`z88dk/libsrc/l/llvmz80/MATH32_BRIDGE.md` §5a.

---

## 10. Path X caveat, RESOLVED: the conditional-CC gate

The Path X legalizer change, if unconditional, would also change the **ELF
path**'s f32 libcalls to sdcccall(0) -- but the ELF compiler-rt `__addsf3`
expects the register ABI, so the standalone `--target=z80` float runtime +
the z80-utils f32 tests would break.

**Fixed (2026-08-01):** the CC selection is gated behind a new opt-in
`cl::opt<bool>` flag, `-z80-float-sdcccall0` (default OFF), in
`Z80LegalizerInfo.cpp`. z88dk's zcc must pass `-mllvm -z80-float-sdcccall0`
for `-compiler=llvmz80` builds (see `test/clang/runtime_float.sh` in z88dk);
the default (flag absent) ELF path is unchanged -- confirmed by a lit
`DEFAULT` check-prefix run with no flag
(`llvm/test/CodeGen/Z80/issue-277-f32-libcall-sdcccall0.ll`) plus the full
Z80 lit suite (208 PASS + 5 XFAIL, no regressions).

math.h then follows for free: z88dk's `math.h` declares `sqrtf`/`sinf`/... which
resolve to the `cm32_sdcc_*` libm wrappers under the same convention -> no
boundary swaps between `a+b` and `sqrtf(...)`.

---

## 11. Status

- **Committed** (llvm-z80 `float32-math32`): Phase 0 double=32 + lit test
  (`dafa0a1a79c0`); this design doc (`d6bec6aa613a`, updated 2026-08-01).
- **Committed** (llvm-z80 `float32-math32`): the `Z80LegalizerInfo.cpp` CC
  change (Path X), gated behind `-z80-float-sdcccall0` (default OFF), with lit
  test `issue-277-f32-libcall-sdcccall0.ll` pinning both the flag-on stack-arg
  behavior and the flag-off (default/ELF) behavior.
- **Committed** (z88dk `llvmz80-float32-math32`): the pure-alias bridge
  `libsrc/l/llvmz80/__addsf3.asm` + thorough runtime test
  `test/clang/runtime_float.c` + harness `test/clang/runtime_float.sh`.
  Verified end-to-end under ntvcm: `ALL PASS` (order-sensitive sub/div and
  signed-zero cases included); confirmed the SAME build WITHOUT the flag
  produces garbage on every case (proves the gate is load-bearing, not a
  no-op).
- **Committed (2026-08-01, llvm-z80 `float32-math32`):** compares
  (`__cmpsf2`/`__gtsf2`/`__gesf2`/`__unordsf2`, G_FCMP) gated behind the same
  flag via a local `F32LibcallCC`. Conversions (`__fixsfsi`/`__fixunssfsi`/
  `__floatsisf`/`__floatunsisf`) moved from the generic
  `.libcallForCartesianProduct` (whose CC comes from a table Z80 never
  populates) to `.customFor({{S32,S32}})` + a new `legalizeCustom` case, so
  they can be gated the same way; the `{S32,S64}`/`{S64,S64}` pairs are
  untouched. New lit test `issue-277-f32-cmp-conv-sdcccall0.ll`
  (CHECK/DEFAULT pairs); full suite 209 PASS + 5 XFAIL, no regressions.
- **Committed (2026-08-01, z88dk `llvmz80-float32-math32`):** bridges
  `libsrc/l/llvmz80/__floatsisf.asm` (pure aliases -- `__fixsfsi`/
  `__fixunssfsi` -> `cm32_sdcc___fs2sint`/`__fs2uint`; `__floatsisf`/
  `__floatunsisf` -> `cm32_sdcc___slong2fs`/`__ulong2fs`, NOT the
  16-bit-named `__sint2fs`/`__uint2fs`, which would drop half of clang's
  32-bit-widened argument) and `libsrc/l/llvmz80/__cmpsf2.asm` (a real
  adapter, not an alias: math32's raw `m32_compare` core has no NaN
  awareness at all, so each entry point runs its own NaN check on both
  stack operands before translating `m32_compare`'s Z/C flags to GCC's
  -1/0/+1 tri-state). New runtime tests `test/clang/runtime_fconv.{c,sh}` /
  `runtime_fcmp.{c,sh}` (boundary values, all 6 comparison predicates, NaN
  in both operand positions, `__builtin_islessgreater`/`isunordered`):
  `ALL PASS` under ntvcm. Negative control (same builds without the flag)
  confirms the gate is load-bearing here too: conversions fail
  deterministically with wrong values, compares hang -- both are the
  expected symptom of an ABI mismatch, not silent wrong-but-plausible
  behavior. `runtime_float.sh` (arithmetic) re-verified still PASS, so the
  legalizer refactor this depends on caused no regression.

## 12. Open items

- math.h wiring (z88dk header -> `cm32_sdcc_*` libm) — untested.
- Whether math32's div can be tuned, or a hybrid uses compiler-rt's div.
- Correct the ABI premise on upstream #34 (it repeats the wrong DEHL claim).
- zcc convenience: currently users must pass `-mllvm -z80-float-sdcccall0
  -L<z88dk>/libsrc -lmath32` by hand for every build; consider auto-injecting
  these for `-compiler=llvmz80` once this direction is confirmed with the fork
  owner, mirroring the existing `LLVMZ80RTLIB` auto-link pattern in zcc.c.
