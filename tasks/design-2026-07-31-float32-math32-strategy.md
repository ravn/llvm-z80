# Design: 32-bit `double` + z88dk `math32` runtime, and the sub/div bridge choice

Date: 2026-07-31.
Status: decided (Path Y for first landing); Path X documented as the upstream-discussable alternative.
Tracking issue: ravn/llvm-z80 #277. Related: #276 (LLVM-24 merge), `plan-2026-07-10-z88dk-calling-conventions.md`.

This document records a floating-point ABI decision for the Z80 backend and, in
particular, the **Path X vs Path Y** choice for the two non-commutative float
libcalls (`sub`, `div`). It is written to be discussed upstream on the
`llvm-z80/llvm-z80` fork: the name-selection half touches
`llvm/include/llvm/IR/RuntimeLibcalls.td` (a generic-LLVM mechanism), and the
choice between a custom libcall calling convention and library-side shims is a
genuine design fork worth a second opinion.

---

## 1. Goal and decision

**Goal (user, 2026-07-31):** reuse z88dk's existing runtime library and invent as
little as possible; the compiler should emit the **fewest possible type casts**
under the hood.

**Decision:**
1. `double` and `long double` are **32-bit IEEE-754 binary32** on Z80/SM83 (same
   width and bit-format as `float`), not the C-standard 64-bit binary64.
2. Reuse z88dk's **`math32`** runtime (a 32-bit IEEE-754 float library, selected
   in z88dk by `--math32` = `-fp-mode=ieee`) instead of shipping our own 64-bit
   Berkeley-SoftFloat closure.
3. For the operand-order mismatch on the two non-commutative libcalls
   (`__subsf3`, `__divsf3`), use **Path Y** (library-side swap-shims) for the
   first landing; **Path X** (a custom libcall calling convention in the backend)
   is documented below as the zero-runtime-cost alternative for later.

The existing 64-bit softfloat work (`llvmz80-softfloat/` tree,
`softfloat_cpm_z80.lib`, the #273 int->double fix, the `%f` nanoprintf shim) is
**parked, not deleted**; it is simply no longer the default path.

---

## 2. Why 32-bit `double` satisfies "fewest typecasts" (measured)

In C the usual arithmetic conversions and the default argument promotions insert
`float`->`double` conversions in many places (variadic calls such as `printf`,
unsuffixed float constants, many `math.h` prototypes). With a 64-bit `double`
each such promotion is a real `__extendsfdf2` call, and every FP operation on a
`double` is a 64-bit `df` libcall that nothing on this target implements.

Making `double` == `float` == binary32 turns every one of those conversions into
an **identity no-op**. Measured on the `build-macos` clang (already built with the
change):

```
float  fadd(float,float)   -> call ___addsf3
double dadd(double,double) -> call ___addsf3      (same 32-bit libcall)
double f2d(float x){return x;}  -> (no call — identity)
float  d2f(double x){return x;} -> (no call — identity)
```

Zero `__extendsfdf2` / `__truncdfsf2` anywhere; every FP op is a 32-bit `sf`
libcall. The "fewest typecasts" constraint is therefore satisfied by the
frontend type change alone — the rest of this document is only about mapping the
`sf` libcalls to `math32`.

The 13-line change lives in `clang/lib/Basic/Targets/Z80.cpp`
(`DoubleWidth/DoubleFormat/LongDoubleWidth/LongDoubleFormat` -> 32 / IEEEsingle),
with test `clang/test/CodeGen/z80-double-is-float32.c`.

---

## 3. The libcall surface (measured, 2026-07-31)

clang-z80 emits the standard compiler-rt `sf` names. Full surface:

| Category | libcalls emitted |
|---|---|
| arithmetic | `__addsf3`, `__subsf3`, `__mulsf3`, `__divsf3` |
| compare | `__cmpsf2`, `__gesf2`, `__gtsf2` (all 6 predicates canonicalise to these 3) |
| int -> float | `__floatsisf`, `__floatunsisf` (16-bit int widens to SImode first) |
| float -> int | `__fixsfsi`, `__fixunssfsi` |

### Measured ABI: clang vs `math32` core

| clang libcall | clang placement | `math32` core | Match |
|---|---|---|---|
| `__addsf3(a,b)` | a in **DEHL** (D=MSB), b on **stack**, result DEHL | `m32_fsadd32x32`: x(stack)+y(dehl)->dehl | **exact** (commutative) |
| `__mulsf3(a,b)` | a DEHL, b stack | `m32_fsmul32x32`: same | **exact** (commutative) |
| `__subsf3(a,b)=a-b` | a DEHL, b stack | `m32_fssub`: **x(stack)-y(dehl)** | **reversed** -> yields b-a |
| `__divsf3(a,b)=a/b` | a DEHL, b stack | `m32_fsdiv`: reciprocal of y -> **stack/dehl** | **reversed** |
| `__floatsisf(n)` | n in **DE:HL** (DE=lo, HL=hi, int layout) | int->float core (DEHL->DEHL) | reg-order: 0-1 instr |
| `__fixsfsi(x)` | x in DEHL(float) -> DE:HL(int) | `m32_f2sint`: `DEHL float -> DEHL integer` | reg-order: 0-1 instr |
| `__cmpsf2/gesf2/gtsf2` | returns **signed int** (-1/0/1 style) | `m32_compare`: returns **flags** (Z/carry) | return-repr adapter |

Byte order confirmed: `math32`'s packed float has D = MSB (sign/exponent),
matching clang's DE=high16. That is why add/mul are exact matches.

**Consequence:** add and mul need only a name change; sub and div additionally
need the operand order reversed; conversions need at most one `ex de,hl`; the
three compares need a flags->signed-int adapter (an irreducible difference in
*return representation*, present under any approach).

---

## 4. The LLVM libcall model (where names and CC come from)

Z80/SM83 currently sit in **`LegacyDefaultSystemLibrary`** via `isDefaultLibcallArch`
in `llvm/include/llvm/IR/RuntimeLibcalls.td` (~line 3580), which supplies the
default compiler-rt names (`__addsf3`, ...). That file explicitly says:

> `// TODO: Should make every target explicit.`

Two knobs, both confirmed to be honoured by Z80's GlobalISel pipeline
(`LegalizerHelper::createLibcall` reads both `getLibcallImplName` and
`getLibcallImplCallingConv`):

- **Name** = the `RuntimeLibcallImpl` name. To emit `m32_fsadd32x32` instead of
  `__addsf3`, define `def m32_fsadd32x32 : RuntimeLibcallImpl<ADD_F32,
  "m32_fsadd32x32">;` and place it in a **dedicated Z80 `SystemRuntimeLibrary`**
  (splitting z80/sm83 out of `isDefaultLibcallArch`). This directly discharges
  the upstream TODO — it is the intended mechanism, not a workaround.
- **Calling convention** = set at runtime via `setLibcallImplCallingConv(Impl,
  CC)` (public in `RuntimeLibcalls.h`; AArch64 already uses it for
  `AArch64_VectorCall`).

The name half is identical in both Path X and Path Y. The paths differ only in
how the sub/div operand order is corrected.

---

## 5. The core problem: non-commutative operand order

clang computes `__subsf3(a,b) = a - b` and places **a in DEHL, b on the stack**.
`math32`'s `m32_fssub` computes `stack - dehl`. Feeding clang's placement in
directly yields `b - a` — the wrong sign. `__divsf3` has the same shape
(`stack / dehl` vs clang's a=dividend in DEHL). add and mul are unaffected
(commutative). So exactly one thing must swap the two operands for sub and div.

---

## 6. Path X vs Path Y

### Path X — custom libcall calling convention (in the compiler)

Define a Z80 calling convention for float libcalls that places **arg1 on the
stack and arg2 in DEHL** (the reverse of the default sdcccall(1)). Then clang
emits the operands where `math32` wants them:

```
a -> stack,  b -> DEHL
m32_fssub:  stack - dehl  =  a - b   correct
```

- **Where:** `Z80CallLowering.cpp` (the hand-written CC switch that already
  handles `Z80_SDCCCall0` / `Z80_Z88dkCallee`) + a new `CallingConv::ID` +
  `setLibcallImplCallingConv` wiring the CC onto the four F32 arith impls.
- **Runtime cost:** zero — the register allocator places operands correctly at
  compile time.
- **Cost:** ~30-60 lines of backend code + a new CC ID; a compiler change that
  must be built and lit-tested. Same custom CC also covers the conversions
  (absorbing any register-order difference at zero runtime cost).

### Path Y — library-side swap-shims (in z88dk)

Let clang emit the default placement (a in DEHL, b on stack) and interpose a
tiny asm shim ahead of `math32` that swaps the two 32-bit operands for the two
non-commutative ops:

```
__subsf3:
    <swap DEHL with the 32-bit operand on the stack>   ; ~10-14 instructions
    jp m32_fssub
```

- **Where:** a small asm bridge in z88dk (same class as the existing string /
  integer-helper bridges; cf. `libsrc/l/llvmz80/newlib/llvmz80_imath.lib`). The
  backend is untouched.
- **Runtime cost:** ~10-14 instructions per `sub`/`div` call (those two ops only;
  add/mul/conversions/compares are unaffected).
- **Cost:** two small asm shims. No compiler-CC code, no clang rebuild for this
  half.

### Comparison

| | Path X (custom CC) | Path Y (swap-shims) |
|---|---|---|
| add, mul | name-remap, 0 shim | name-remap, 0 shim |
| sub, div | 0 shim (CC places operands) | asm swap-shim x2 |
| conversions | CC-absorbed (0 instr) | 0-1 instr |
| compares | 3 flags->int adapters | 3 flags->int adapters |
| where the code lives | llvm-z80 backend | z88dk library |
| runtime cost | zero | ~10-14 instr per sub/div call |
| needs clang build + lit | yes | no (for the sub/div half) |
| reversibility | is the end state | trivially replaced by X later |

---

## 7. Decision and rationale

**Path Y for the first landing.**

- add, mul, conversions, and compares need no calling convention in either path,
  so Path Y's only delta from Path X is two small asm shims that live entirely in
  z88dk — zero compiler-CC surface and zero clang-rebuild risk for that half.
- It matches the project ethos ("reuse z88dk, invent as little as possible"): the
  shim is the same class of artefact as the string and integer bridges already in
  `libsrc/l/llvmz80/`.
- The runtime cost is confined to two operations (`sub`, `div`) that are not
  typically hot on this workload; if profiling ever shows otherwise, Path X is a
  drop-in replacement (§8).
- It de-risks the first landing: the load-bearing, harder-to-review change (the
  custom CC in `Z80CallLowering`) is deferred until the name-remap + math32
  reuse is proven end-to-end.

**Path X is the preferred *end state*** if zero runtime glue is wanted: it is the
"clang conforms to the ABI" outcome, with no per-call shim on any op. It is
documented here so the trade-off is explicit and can be discussed on the fork.

---

## 8. Migration Path Y -> X (if chosen later)

1. Add a `CallingConv::ID` for the Z80 float-libcall convention.
2. Implement arg placement (arg1->stack, arg2->DEHL, result DEHL) in
   `Z80CallLowering.cpp`, alongside the existing `Z80_SDCCCall0` handling.
3. `setLibcallImplCallingConv` for the four F32 arith impls (and the conversions
   if their register order is folded in).
4. Delete the two z88dk swap-shims; the name-remap and compare adapters are
   unchanged.

Because the name-selection and compare-adapter halves are identical in both
paths, the migration touches only the two shims and the new CC — nothing else
regresses.

---

## 9. Upstream relevance (llvm-z80/llvm-z80)

- **`RuntimeLibcalls.td` change is on the intended path:** splitting z80/sm83 into
  a dedicated `SystemRuntimeLibrary` with math32 impl names discharges the file's
  own `// TODO: Should make every target explicit.` It is a clean, reviewable
  change, not a hack.
- **The Path X/Y choice is a general pattern** for any Z80 backend that targets a
  host float library whose operand order differs from compiler-rt: either a
  target libcall CC or library shims. Documenting the measured ABI and the
  trade-off lets the fork owner weigh in before the CC surface is committed.
- **No generic-LLVM behaviour changes** for other targets: the default system
  library is untouched; only Z80/SM83's selection moves.

---

## 10. Open items (impl-time, low risk)

- Exact int byte-order in `m32_f2sint` result / `m32_sint2f` argument (determines
  whether conversions are 0 or 1 `ex de,hl`).
- `__unordsf2` / NaN compare path — did not appear at -O2; confirm whether
  NaN-aware code emits it and bridge if so.
- Exact `m32_compare` flag encoding (Z + carry) -> the -1/0/1 mapping per
  `__cmpsf2` / `__gesf2` / `__gtsf2`.
- Confirm stock ieee `printf("%f")` accepts clang's binary32 directly (both are
  IEEE-754 binary32); if so, drop the nanoprintf / `-D__LLVMZ80_IEEE_PRINTF`
  closure.

---

## 11. Verification gates

- **lit:** emitted call name + operand placement pinned per libcall (two RUN
  lines each).
- **runtime (ntvcm):** self-checking oracles for arith / compare / conversion per
  op, including sign-sensitive `sub`/`div` and NaN.
- **printf:** `printf("%f")` byte-match vs a host reference.
