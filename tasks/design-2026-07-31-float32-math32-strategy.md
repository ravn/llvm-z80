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

## 12. Open items

- Compare + conversion bridges (`__cmpsf2/__gesf2/__gtsf2`,
  `__floatsisf/__fixsfsi`) — not yet built; needed before the existing
  `test_08_f32_*` / `test_50_f32_mathlib` run under math32.
- math.h wiring (z88dk header -> `cm32_sdcc_*` libm) — untested.
- Whether math32's div can be tuned, or a hybrid uses compiler-rt's div.
- Correct the ABI premise on upstream #34 (it repeats the wrong DEHL claim).
- zcc convenience: currently users must pass `-mllvm -z80-float-sdcccall0
  -L<z88dk>/libsrc -lmath32` by hand for every build; consider auto-injecting
  these for `-compiler=llvmz80` once this direction is confirmed with the fork
  owner, mirroring the existing `LLVMZ80RTLIB` auto-link pattern in zcc.c.
